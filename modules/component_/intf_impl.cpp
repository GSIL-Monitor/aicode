
#include "intf.h"
#include "fdfs_impl.h"
#include "md5_helper.h"
#include "LogRLD.h"
#include <boost/format.hpp>
#include <boost/timer.hpp>
#include <boost/scope_exit.hpp>
#include <iostream>
#include <string>
#include <set>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "mysql_impl.h"
#include <uuid/uuid.h>

std::string ConvertCharValueToLex(unsigned char *pInValue, const boost::uint32_t uiSize);
std::string GetStorageIDFromFileID(const std::string &strFileID)
{
    //const std::string strFileID(fileid);
    std::string::size_type pos = strFileID.find('/');
    if (std::string::npos == pos)
    {
        LOG_ERROR_RLD("File id incorrect: " << strFileID);
        return "";
    }

    std::string strRealFileID;

    //这里要考虑兼容，数据库中原来的文件id也需要支持能够下载（类似 group3/M03/7A/C4/ChQPDVbL-oyANQ-cAAaPBriXo3M579.tar）
    const std::string &strID = strFileID.substr(pos + 1);
    if (strID.substr(0, 5) == "group") //表示为新的文件id格式
    {
        strRealFileID = strID;
    }
    else
    {
        strRealFileID = strFileID;
    }

    return strRealFileID;
}

enum RecordStatus
{
    Active = 1,
    Deleted = 2,
    NeedDelete = 3,
    Deleting = 4,
};

/**
static const char insert_sql_fmt[] = "insert into t_file_info("
                                 "id,devid,areaid, fileid, fileurl, filename, filesize, "
                                 "filemd5, extend, businessid, status, createdate) values(uuid(),"
                                 "'%s',%d,'%s','%s','%s', %d,'%s','%s',%d,1,sysdate())";
**/

static const char insert_sql_fmt[] = "insert into t_file_info("
                                 "id,devid,areaid, fileid, fileurl, filename, filesize, "
                                 "filemd5, extend, businessid, status, createdate) values(uuid(),"
                                 "'%s',%d,'%s','%s','%s', %d,'%s','%s',%d,1,'%s')";


static const char update_sql_fmt[] = "INSERT INTO t_file_log (id,statistic_uuid,devid,"
                                     "areaid,fileid,filename,visittimes) VALUES (uuid(),"
                                     "'%s','%s',%d,'%s','%s',1) ON DUPLICATE KEY UPDATE "
                                     "visittimes=visittimes+1";

DataAccessManager::DataAccessManager() : pRunner(NULL), pThread(NULL), is_init(false), m_MysqlImpl(new MysqlImpl()), m_pFdfsImpl(new FdfsImpl())
{
    
}

DataAccessManager::~DataAccessManager()
{
    //UnInit();
}


bool DataAccessManager::QuerySql(const std::string &strSql, ColumnCB ccb)
{
    return m_MysqlImpl->QueryExec(strSql, ccb);
}

bool DataAccessManager::QuerySql(const std::string& sql)
{
    return m_MysqlImpl->QueryExec(sql);
}

bool DataAccessManager::UpdateDownloadInfo2Db(const std::string devid, int areaid, const std::string fileid, const std::string filename)
{
    boost::format fmt1("%s,%d,%s");
    std::string data = boost::str(fmt1 % devid % areaid % fileid);
    std::string key = Md5Helper::Md5buf(data.c_str(), data.size());

    boost::format fmt(update_sql_fmt);
    std::string sql = boost::str(fmt % key % devid % areaid % fileid % filename);

    return m_MysqlImpl->QueryExec(sql);
}

bool DataAccessManager::Init(const char* cfgfilename, const char* dbhost, const char* dbuser, const char* dbpass, const char* dbname, int myhelpervalid)
{
    if (!cfgfilename || !dbhost || !dbuser || !dbpass || !dbname)
    {
        LOG_ERROR_RLD("DataAccessManager init failed, init arg contain null value");
        return false;
    }

    if(is_init) return true;

    if (!m_pFdfsImpl->Init(cfgfilename))
    {
        LOG_ERROR_RLD("Fdfs init failed, cfg file name is " << cfgfilename);
        return false;
    }


    bool bResult = m_MysqlImpl->Init(dbhost, dbuser, dbpass, dbname, myhelpervalid);
    if (!bResult)
    {
        LOG_ERROR_RLD("MysqlInit failed, dbhost is " << dbhost << ", dbuser is " << dbuser << ", dbpasswd is " << dbpass  << ", dbname is " << dbname);
        return false;
    }

    pRunner = new Runner(1);
    pRunner->Run();

    pThread = new boost::thread(boost::bind(&DataAccessManager::PostThread, this));

    is_init = true;

    return true;
}

void DataAccessManager::UnInit()
{
    if(!is_init) return;
    
    m_pFdfsImpl->UnInit();

    m_MysqlImpl->UnInit();

    if (m_MysqlImpl)
    {
        delete m_MysqlImpl;
        m_MysqlImpl = NULL;
    }

    pRunner->Stop();
    delete pRunner;
    pRunner = NULL;

    pThread->join();
    delete pThread;
    pThread = NULL;

    is_init = false;
}

bool DataAccessManager::UploadFile(const char* localfilename, const char* origfilename, const char* devid, char fid[MAX_FILE_ID_LEN], std::string &strStorageFileID,
                                int* errcode, int areaid, const char* filemd5, const char* extend, unsigned int businessid, std::string &strMd5Out)
{
    if(!errcode) return NULL;
    *errcode = E_SERVER_LOGDB_FAILED;

    if(!is_init)
    {
        *errcode = E_SERVER_LOGDB_FAILED ;
        return false;
    }
    if(!localfilename)
    {
        *errcode = E_SERVER_STORAGEDB_FAILED ;
        return false;
    }

    struct stat st;
    if(stat(localfilename, &st) != 0)
    {
        *errcode = E_SERVER_LOCAL_FILE_FAILED;
        //printf("ERROR: Get file [%s] info fail. %s\n", localfilename, strerror(errno));
        LOG_ERROR_RLD("Get local file " << localfilename << " info failed and error info is " << strerror(errno));
        return false;
    }

    //valid local file md5
    const std::string &strMD5 = Md5Helper::Md5file(localfilename);
    if (filemd5 && strcmp(filemd5, "") != 0)
    {
        if (std::string(filemd5) != strMD5)
        {
            *errcode = E_SERVER_LOCAL_FILE_FAILED;
            LOG_ERROR_RLD("Local file md5 is not equal to original md5, loca md5 is " << strMD5 << " and original md5 is " << filemd5);
            return false;
        }
    }
    
    strMd5Out = strMD5;

    // here, we must load file to fdfs first
    std::string fileid;
    std::string fileurl;
    if (!m_pFdfsImpl->Upload(localfilename, fileid, fileurl, origfilename))
    {
        *errcode = E_SERVER_STORAGEDB_FAILED;
        return false;
    }

    strStorageFileID = fileid;
    
    // insert file information to table: t_file_info
        
    std::string ext;
    if(extend)
    {
        ext = extend;
    }

	std::string sCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
	std::string::size_type pos = sCurrentTime.find('T');
	sCurrentTime.replace(pos,1,std::string(" "));

	LOG_DEBUG_RLD("current time[" << sCurrentTime << "]");

    //先生成唯一的文件ID值
    GenerateUniqueFileID(fileid.c_str(), fid);

    boost::format fmt(insert_sql_fmt);
    //std::string sql = boost::str(fmt % std::string(devid) % areaid % fileid % fileurl % std::string(origfilename) % (int)st.st_size % md5 % ext % businessid);
	std::string sql = boost::str(fmt % std::string(devid) % areaid % std::string(fid) % fileurl % std::string(origfilename) % (int)st.st_size % strMD5 % ext % businessid % sCurrentTime);
    pRunner->Post(boost::bind(&DataAccessManager::QuerySql, this, sql));
    
    //strncpy(fid, fileid.c_str(), MAX_FILE_ID_LEN);
    //snprintf(fid, MAX_FILE_ID_LEN, "%llu/%s", ++m_SeqNum, fileid.c_str());
    
    *errcode = 0;
    return true;
}

void DataAccessManager::GenerateUniqueFileID(const char *pFileIDIn, char *pFileIDOut)
{
    uuid_t uu;
    uuid_generate(uu);

    //strFileGUID.assign((const char*)uu, sizeof(uu));
    const std::string &strUUID = ConvertCharValueToLex((unsigned char*)uu, sizeof(uu));

    snprintf(pFileIDOut, MAX_FILE_ID_LEN, "%s/%s", strUUID.c_str(), pFileIDIn);

}

HANDLE DataAccessManager::DownloadOpen(const char* fileid, const char* devid, int* errcode)
{
    if (!fileid || !devid)
    {
        return NULL;
    }

    const std::string &strRealFileID = GetStorageIDFromFileID(std::string(fileid));

    /*
    const std::string strFileID(fileid);
    std::string::size_type pos = strFileID.find('/');
    if (std::string::npos == pos)
    {
        LOG_ERROR_RLD("File id incorrect: " << strFileID);
        return NULL;
    }

    std::string strRealFileID;
    //这里要考虑兼容，数据库中原来的文件id也需要支持能够下载（类似 group3/M03/7A/C4/ChQPDVbL-oyANQ-cAAaPBriXo3M579.tar）
    const std::string &strID = strFileID.substr(pos + 1);
    if (strID.substr(0, 5) == "group") //表示为新的文件id格式
    {
        strRealFileID = strID;
    }
    else
    {
        strRealFileID = strFileID;
    }*/

    *errcode = 0;
    int areaid = 0;
    HANDLE hdl = m_pFdfsImpl->DownloadOpen(strRealFileID.c_str(), errcode); //fileid, errcode);
    if (hdl)
    {
        pRunner->Post(boost::bind(&DataAccessManager::UpdateDownloadInfo2Db, this, std::string(devid), areaid, std::string(fileid), ""));
    }
    else
    {
        *errcode = E_SERVER_STORAGEDB_FAILED;
    }
    return hdl;
}

int DataAccessManager::DownloadBlock(HANDLE handle, char* buf, int offset, int size, int* errcode)
{
    return m_pFdfsImpl->DownloadBlock(handle, buf, offset, size, errcode);
}


void DataAccessManager::DownloadClose(HANDLE handle)
{
    m_pFdfsImpl->DownloadClose(handle);
}


boost::uint32_t DataAccessManager::ProcessDeleteFile(const std::string &strFileID, const std::string &strFileMD5, const bool IsNeedDelFdfs)
{
    char cSqlValid[512] = { 0 };
    const char* pSql = "select status from t_file_info where fileid='%s'";
    snprintf(cSqlValid, sizeof(cSqlValid), pSql, strFileID.c_str());

    bool blRet = true;
    std::vector<int> ResultVct;
    if (!(blRet = m_MysqlImpl->QueryExec(cSqlValid, ResultVct)))
    {
        LOG_ERROR_RLD("QueryExec[Deleted] failed, fileid: " << strFileID);
        return E_SERVER_DELETE_FILE_FAILED;
    }

    if (ResultVct.empty())
    {
        LOG_INFO_RLD("QueryExec[Deleted] not found file");
        return E_SERVER_DELETE_FILE_NOT_EXIST;
    }

    if (Deleted == ResultVct[0])
    {
        LOG_INFO_RLD("QueryExec[Deleted] file alread deleted");
        return E_SERVER_DELETE_FILE_ALREADY_DEL;
    }

    const std::string &strRealFileID = GetStorageIDFromFileID(strFileID);

    int iRet = IsNeedDelFdfs ? m_pFdfsImpl->DeleteFdfsFileImpl(strRealFileID.c_str()) : -1;

    //无论删除文件是否成功，都需要将数据库中文件记录状态字段标记为已删除
    char sql2[512] = { 0 };
    const char* fmt2 = "update t_file_info set status=%d where fileid='%s'";
    snprintf(sql2, sizeof(sql2), fmt2, Deleted, strFileID.c_str());
    
    bool blSqlRet = true;
    if (!(blSqlRet =m_MysqlImpl->QueryExec(sql2)))
    {
        LOG_ERROR_RLD("QueryExec[Deleted] failed, fileid: " << strFileID);
        return E_SERVER_DELETE_FILE_FAILED;
    }

    LOG_INFO_RLD("Need to delete storage: " << IsNeedDelFdfs << " return " << iRet <<
        " and update database result is " << blSqlRet);

    return 0; //这里只需要关注更新数据库是否成功
}

bool DataAccessManager::DoQueryBusinessId()
{
    std::vector<unsigned int> businesses;
    const char* sql = "select businessid from t_business_type where status = 1";
    bool result = m_MysqlImpl->QueryExec(sql, businesses);
    if (!result)
    {
        LOG_ERROR_RLD("DoQueryBusinessId failed.");
        return false;
    }

    boost::mutex::scoped_lock lock(businessid_mutex);
    for (auto it=businesses.begin(); it!=businesses.end(); ++it)
    {
        businessid_set.insert(*it);
    }

    LOG_INFO_RLD("DoQueryBusinessId finish.");
    return true;
}

bool DataAccessManager::QueryRecordCount(int* pRecordCount, boost::condition* pCond)
{
    BOOST_SCOPE_EXIT(&pCond)
    {
        pCond->notify_one();
    }
    BOOST_SCOPE_EXIT_END

    *pRecordCount = 0;

    char sql[1024] = {0};
    const char* fmt = "select count(*) from t_file_info where status = %d or status = %d";
    snprintf(sql, sizeof(sql), fmt, NeedDelete, Deleting);

    std::vector<int> vResult;

    bool bResult = m_MysqlImpl->QueryExec(sql, vResult);
    if (!bResult)
    {
        LOG_ERROR_RLD("QueryRecordCount failed.");
        return false;
    }

    if (vResult.size()==1)
    {
        *pRecordCount = vResult[0];
    }

    LOG_INFO_RLD("QueryRecordCount finish.");
    return true;
}

bool DataAccessManager::GetRecordCount(int* pRecordCount)
{
    boost::mutex mtx;
    boost::condition cond;
    boost::mutex::scoped_lock lock(mtx);
    pRunner->Post(boost::bind(&DataAccessManager::QueryRecordCount, this, pRecordCount, &cond));
    cond.wait(mtx);
    return true;
}

void DataAccessManager::DoDeleteFdfsFile(const int& nRecordCount)
{
    std::vector<FileInfoList> filelist;
    char sql[1024] = {0};
    const char* fmt = "select id, fileid, filesize, createdate from t_file_info where status=%d or status=%d order by createdate limit 0,%d";
    snprintf(sql, sizeof(sql), fmt, NeedDelete, Deleting, nRecordCount);

    if (!m_MysqlImpl->QueryExec(sql, filelist))
    {
        LOG_ERROR_RLD("QueryExec return false.");
        return;
    }

    for (auto it=filelist.begin(); filelist.end()!=it; it++)
    {
        LOG_WARN_RLD("QueryExec update fileid(" << it->fileid.c_str() << ") Deleting");

        //更新状态 -- 删除中
        char sql1[1024] = {0};
        const char* fmt1 = "update t_file_info set status=%d where id='%s'";
        snprintf(sql1, sizeof(sql1), fmt1, Deleting, it->id.c_str());
        if (!m_MysqlImpl->QueryExec(sql1))
        {
            LOG_ERROR_RLD("QueryExec[Deleting] return false, fileid: " << it->fileid);
            return;
        }

        LOG_WARN_RLD("start trim fdfs fileid:" << it->fileid.c_str());

        const std::string &strRealFileID = GetStorageIDFromFileID(it->fileid);

        int result = m_pFdfsImpl->DeleteFdfsFileImpl(strRealFileID.c_str());
        if (result!=0)
        {
            LOG_ERROR_RLD("DeleteFdfsFileImpl return false, fileid: " << it->fileid);
            //return;
        }

        LOG_WARN_RLD("QueryExec update fileid(" << it->fileid.c_str() << ") Deleted");

        //更新状态 -- 删除完成
        char sql2[1024] = {0};
        const char* fmt2 = "update t_file_info set status=%d where id='%s'";
        snprintf(sql2, sizeof(sql2), fmt2, Deleted, it->id.c_str());
        if (!m_MysqlImpl->QueryExec(sql2))
        {
            LOG_ERROR_RLD("QueryExec[Deleted] return false, fileid: " << it->fileid);
            return;
        }
    }
}

void DataAccessManager::DeleteFdfsFile(const int& nRecordCount)
{
    pRunner->Post(boost::bind(&DataAccessManager::DoDeleteFdfsFile, this, nRecordCount));
}

bool DataAccessManager::IsValidBusinessid(unsigned int businessid)
{
    boost::mutex::scoped_lock lock(businessid_mutex); //lock for business_set
    return (businessid_set.find(businessid)!=businessid_set.end());
}

void DataAccessManager::PostThread()
{
    while (1)
    {
        pRunner->Post(boost::bind(&DataAccessManager::DoQueryBusinessId, this));
        sleep(3600); //暂时实现
    }
}

int DataAccessManager::GetFileSize(HANDLE handle)
{
    return m_pFdfsImpl->GetFileSize(handle);
}


