#ifndef __INTF_H__
#define __INTF_H__

#include <stdio.h>
#include <set>
#include <boost/thread/mutex.hpp>
#include <boost/thread/detail/singleton.hpp>
#include <boost/thread/condition.hpp>
#include "ErrorCode.h"
#include "NetComm.h"

#define MAX_FILE_ID_LEN 256

class MysqlImpl;
class FdfsImpl;
typedef void* HANDLE;

class DataAccessManager
{
public:
    DataAccessManager();
    ~DataAccessManager();

    typedef boost::function<void(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)> ColumnCB;

	static const int MYSQL_HELPER_VALID = 0;

    bool QuerySql(const std::string &strSql, ColumnCB ccb);
    
    bool QuerySql(const std::string& sql);

    bool UpdateDownloadInfo2Db(const std::string devid, int areaid, const std::string fileid, const std::string filename);

    bool Init(const char* cfgfilename, const char* dbhost, const char* dbuser, const char* dbpass, const char* dbname, int myhelpervalid);
    void UnInit();

    bool UploadFile(const char* localfilename, const char* origfilename, const char* devid, char fid[MAX_FILE_ID_LEN], std::string &strStorageFileID,
        int* errcode, int areaid, const char* filemd5, const char* extend, unsigned int businessid, std::string &strMd5Out);

    void GenerateUniqueFileID(const char *pFileIDIn, char *pFileIDOut);

    HANDLE DownloadOpen(const char* fileid, const char* devid, int* errcode);

    int    DownloadBlock(HANDLE handle, char* buf, int offset, int size, int* errcode);

    void   DownloadClose(HANDLE handle);

    boost::uint32_t ProcessDeleteFile(const std::string &strFileID, const std::string &strFileMD5, const bool IsNeedDelFdfs);
        
    bool DoQueryBusinessId();

    bool QueryRecordCount(int* pRecordCount, boost::condition* pCond);

    bool GetRecordCount(int* pRecordCount);

    void DoDeleteFdfsFile(const int& nRecordCount);

    void DeleteFdfsFile(const int& nRecordCount);

    bool IsValidBusinessid(unsigned int businessid);

    void PostThread();

    int GetFileSize(HANDLE handle);
    
private:
    boost::mutex businessid_mutex;
    std::set<unsigned int> businessid_set;

    Runner* pRunner;
    boost::thread* pThread;

    bool is_init;

    MysqlImpl* m_MysqlImpl;

    FdfsImpl *m_pFdfsImpl;
};



typedef boost::detail::thread::singleton<DataAccessManager> DataAccessInstance;
//DataAccessManager& DataAccessObj = DataAccessInstance::instance();

#endif
