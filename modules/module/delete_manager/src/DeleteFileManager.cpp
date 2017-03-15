#include "DeleteFileManager.h"
#include "mysql_impl.h"
#include "LogRLD.h"
#include "HttpClient.h"

DeleteFileManager::DeleteFileManager(const ParamInfo &pinfo) : m_Init(false), m_ParamInfo(pinfo), m_pMysql(new MysqlImpl),
m_Tmh(boost::bind(&DeleteFileManager::Run, this, _1), pinfo.m_uiTimeInterval)
{
    if (!m_pMysql->Init(m_ParamInfo.m_strDBHost.c_str(), m_ParamInfo.m_strDBUser.c_str(),
        m_ParamInfo.m_strDBPassword.c_str(), m_ParamInfo.m_strDBName.c_str()))
    {
        LOG_ERROR_RLD("Init db failed, db host is " << m_ParamInfo.m_strDBHost << " db user is " << m_ParamInfo.m_strDBUser << " db pwd is " <<
            m_ParamInfo.m_strDBPassword << " db name is " << m_ParamInfo.m_strDBName);
        return;
    }
    
    m_Init = true;

    m_Tmh.Run();

}


DeleteFileManager::~DeleteFileManager()
{
}

bool DeleteFileManager::Run()
{
    if (!m_Init)
    {
        LOG_ERROR_RLD("DeleteFileManager status is error.");
        return false;
    }

    LOG_INFO_RLD("DeleteFileManager begin running.");

    char sql[1024] = { 0 };
    const char* sqlfmt = "select devid, fileid from t_file_info where filename like '%.tar' and createdate <= '%s' and status = 3 order by createdate asc limit 1";
    snprintf(sql, sizeof(sql), sqlfmt, m_ParamInfo.m_strEndTime.c_str());
    std::string strSql = sql;

    std::string strDevID;
    std::string strFileID;

    auto FuncTmp = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)
    {
        switch (uiColumnNum)
        {
        case 0:
            strDevID = strColumn;
            break;
        case 1:
            strFileID = strColumn;
            break;       
        default:
            LOG_ERROR_RLD("sql callback error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }
    };

    if (!m_pMysql->QueryExec(strSql, FuncTmp))
    {
        LOG_ERROR_RLD("Query file info failed");
        return false;
    }

    if (strDevID.empty() || strFileID.empty())
    {
        LOG_ERROR_RLD("Device id is empty or file id is empty and device id is " << strDevID << " and file id is " << strFileID);
        return false;
    }

    //"http://10.0.0.20/manage_file.cgi?action=delete_file&devid=111&fid=2943bc5b53d047a4aec2559f7d4729a3/group1/M00/00/00/CgAAEFgYAe2ANOXlAeLjCN3R95s3283391"

    char url[1024] = { 0 };
    const char* urlfmt = "http://%s/manage_file.cgi?action=delete_file&devid=%s&fid=%s";
    snprintf(url, sizeof(url), urlfmt, m_ParamInfo.m_strHttpIp.c_str(), strDevID.c_str(), strFileID.c_str());
    std::string strUrl = url;

    std::string strOutMsg;
    HttpClient httpClient;
    if (CURLE_OK != httpClient.Get(strUrl, strOutMsg))
    {
        LOG_ERROR_RLD("Send delete file http req failed, url is " << strUrl);
        return false;
    }

    LOG_INFO_RLD("Send delete file http req success and url is " << strUrl << " and rsp msg is " << strOutMsg);


    {
        char sql[1024] = { 0 };
        const char *sqlfmt = "delete from t_file_info where devid = '%s' and fileid = '%s'";
        snprintf(sql, sizeof(sql), sqlfmt, strDevID.c_str(), strFileID.c_str());

        if (!m_pMysql->QueryExec(std::string(sql)))
        {
            LOG_ERROR_RLD("Delete t_file_info sql exec failed, sql is " << sql);
            return false;
        }

        LOG_INFO_RLD("Delete file info to db is success.");
    }

    return true;

}

void DeleteFileManager::TimerHandler(const boost::system::error_code& e)
{
    if (e)
    {
        LOG_ERROR_RLD("Error: " << e.message());
        return;
    }

    Run();

}

