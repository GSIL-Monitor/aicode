#include "ForkChild.h"
#include "NetComm.h"
#include <list>
#include <string>
#include <map>
#include "LogRLD.h"
#include "ConfigSt.h"
#include <boost/filesystem.hpp> 
#include <boost/lexical_cast.hpp>
#include "ManagementCenter.h"
#include "ControlCenter.h"

#define CONFIG_FILE_NAME "management_center.ini"
#define VERSION "[v1.0.0] "

#ifdef _MSC_VER
#define strcasecmp _stricmp
#define strncasecmp  _strnicmp//strnicmp 
#endif

std::string ConvertCharValueToLex(unsigned char *pInValue, const boost::uint32_t uiSize)
{
    char cTmp[32] = { 0 };
    std::string strTmp;
    for (boost::uint32_t i = 0; i < uiSize; ++i)
    {
#ifdef _WIN32
        _snprintf(cTmp, sizeof(cTmp), "%02x", pInValue[i]);
#else
        snprintf(cTmp, sizeof(cTmp), "%02x", pInValue[i]);
#endif
        strTmp += cTmp;
        memset(cTmp, 0, sizeof(cTmp));
    }

    return strTmp;
}

static void InitLog()
{
    std::string strHost = "ManagementCenter(127.0.0.1)";
    std::string strLogPath = "./logs/";
    std::string strLogInnerShowName = "ManagementCenter";
    int iLoglevel = LogRLD::INFO_LOG_LEVEL;
    int iSchedule = LogRLD::DAILY_LOG_SCHEDULE;
    int iMaxLogFileBackupNum = 10;

    boost::filesystem::path currentPath = boost::filesystem::current_path() / CONFIG_FILE_NAME;

    //判断配置文件是否存在
    if (boost::filesystem::exists(currentPath))
    {
        //初始化配置信息
        ConfigSt cfg(currentPath.string());
        std::string strHostCfg = cfg.GetItem("Log.Host");
        if (!strHostCfg.empty())
        {
            strHost = strHostCfg;
        }

        std::string strLogPathCfg = cfg.GetItem("Log.LogPath");
        if (!strLogPathCfg.empty())
        {
            strLogPath = strLogPathCfg;
        }

        std::string strLogLevelCfg = cfg.GetItem("Log.Level");
        if (!strLogLevelCfg.empty())
        {
            if (strncasecmp("TRACE", strLogLevelCfg.c_str(), 5) == 0)
            {
                iLoglevel = LogRLD::TRACE_LOG_LEVEL;
            }
            else if (strncasecmp("DEBUG", strLogLevelCfg.c_str(), 5) == 0)
            {
                iLoglevel = LogRLD::DEBUG_LOG_LEVEL;
            }
            else if (strncasecmp("INFO", strLogLevelCfg.c_str(), 4) == 0)
            {
                iLoglevel = LogRLD::INFO_LOG_LEVEL;
            }
            else if (strncasecmp("WARN", strLogLevelCfg.c_str(), 4) == 0)
            {
                iLoglevel = LogRLD::WARN_LOG_LEVEL;
            }
            else if (strncasecmp("ERROR", strLogLevelCfg.c_str(), 5) == 0)
            {
                iLoglevel = LogRLD::ERROR_LOG_LEVEL;
            }
            else if (strncasecmp("FATAL", strLogLevelCfg.c_str(), 5) == 0)
            {
                iLoglevel = LogRLD::FATAL_LOG_LEVEL;
            }
        }

        std::string strLogInnerShowNameCfg = cfg.GetItem("Log.LogFileName");
        if (!strLogInnerShowNameCfg.empty())
        {
            strLogInnerShowName = strLogInnerShowNameCfg;
        }

        std::string strScheduleCfg = cfg.GetItem("Log.Schedule");
        if (!strScheduleCfg.empty())
        {
            if (strncasecmp("HOURLY", strLogLevelCfg.c_str(), 6) == 0)
            {
                iSchedule = LogRLD::HOURLY_LOG_SCHEDULE;
            }
            else if (strncasecmp("DAILY", strLogLevelCfg.c_str(), 5) == 0)
            {
                iSchedule = LogRLD::DAILY_LOG_SCHEDULE;
            }
            else
            {

            }

        }

        std::string strMaxLogFileBackupNumCfg = cfg.GetItem("Log.FileNum");
        if (!strMaxLogFileBackupNumCfg.empty())
        {
            iMaxLogFileBackupNum = atoi(strMaxLogFileBackupNumCfg.c_str());
        }


    }

    boost::filesystem::path LogPath(strLogPath);
    std::string strFileName = strLogInnerShowName + ".log";
    LogPath = LogPath / strFileName;

    LogRLD::GetInstance().Init(iLoglevel, strHost, strLogInnerShowName, LogPath.string(), iSchedule, iMaxLogFileBackupNum, VERSION);
}

static std::string GetConfig(const std::string &strItem)
{
    boost::filesystem::path currentPath = boost::filesystem::current_path() / CONFIG_FILE_NAME;

    //判断配置文件是否存在
    if (boost::filesystem::exists(currentPath))
    {
        //初始化配置信息
        ConfigSt cfg(currentPath.string());
        return cfg.GetItem(strItem);
    }
    return "";
}

int main(int argc, char* argv[])
{
    bool IsNeedDaemonRun = false;
    if (2 <= argc && std::string("-d") == argv[1])
    {
        IsNeedDaemonRun = true;
    }

    if (IsNeedDaemonRun)
    {
        ForkChild();
    }

    InitLog();

    LOG_INFO_RLD("ManagementCenter begin runing and daemon status is " << IsNeedDaemonRun);

    const std::string &strDBHost = GetConfig("DB.DBHost");
    if (strDBHost.empty())
    {
        LOG_ERROR_RLD("DBHost config item not found.");
        return 0;
    }

    const std::string &strDBPort = GetConfig("DB.DBPort");
    if (strDBPort.empty())
    {
        LOG_ERROR_RLD("DBPort config item not found.");
        return 0;
    }

    const std::string &strDBUser = GetConfig("DB.DBUser");
    if (strDBUser.empty())
    {
        LOG_ERROR_RLD("DBUser config item not found.");
        return 0;
    }

    const std::string &strDBPassword = GetConfig("DB.DBPassword");
    if (strDBPassword.empty())
    {
        LOG_ERROR_RLD("DBPassword config item not found.");
        return 0;
    }

    const std::string &strDBName = GetConfig("DB.DBName");
    if (strDBName.empty())
    {
        LOG_ERROR_RLD("DBName config item not found.");
        return 0;
    }

    const std::string &strRemoteAddress = GetConfig("Channel.RemoteAddress");
    if (strRemoteAddress.empty())
    {

        LOG_ERROR_RLD("RemoteAddress config item not found.");
        return 0;
    }

    const std::string &strRemotePort = GetConfig("Channel.RemotePort");
    if (strRemotePort.empty())
    {

        LOG_ERROR_RLD("RemotePort config item not found.");
        return 0;
    }

    const std::string &strShakehandOfChannelInterval = GetConfig("Channel.ShakehandOfChannelInterval");
    if (strShakehandOfChannelInterval.empty())
    {
        LOG_ERROR_RLD("ShakehandOfChannelInterval config item not found.");
        return 0;
    }

    const std::string &strSelfID = GetConfig("General.SelfID");
    if (strSelfID.empty())
    {
        LOG_ERROR_RLD("SelfID config item not found.");
        return 0;
    }

    const std::string &strThreadOfWorking = GetConfig("General.ThreadOfWorking");
    if (strThreadOfWorking.empty())
    {
        LOG_ERROR_RLD("ThreadOfWorking config item not found.");
        return 0;
    }

    const std::string &strSessionTimeoutCountThreshold = GetConfig("General.SessionTimeoutCountThreshold");
    if (strSessionTimeoutCountThreshold.empty())
    {
        LOG_ERROR_RLD("Session timout threshold config item not found.");
        return 0;
    }

    const std::string &strManagementAddress = GetConfig("General.ManagementAddress");
    if (strManagementAddress.empty())
    {
        LOG_ERROR_RLD("Management address config item not found.");
        return 0;
    }

    const std::string &strMemcachedAddress = GetConfig("MemCached.MemAddress");
    if (strMemcachedAddress.empty())
    {
        LOG_ERROR_RLD("Memcached address config item not found.");
        return 0;
    }

    const std::string &strMemcachedPort = GetConfig("MemCached.MemPort");
    if (strMemcachedPort.empty())
    {
        LOG_ERROR_RLD("Memcached port config item not found.");
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////

    ManagementCenter::ParamInfo UmgParam;
    UmgParam.m_strDBHost = strDBHost;
    UmgParam.m_strDBName = strDBName;
    UmgParam.m_strDBPassword = strDBPassword;
    UmgParam.m_strDBPort = strDBPort;
    UmgParam.m_strDBUser = strDBUser;
    UmgParam.m_strMemAddress = strMemcachedAddress;
    UmgParam.m_strMemPort = strMemcachedPort;
    UmgParam.m_strSessionTimeoutCountThreshold = strSessionTimeoutCountThreshold;
    UmgParam.m_strManagementAddress = strManagementAddress;

    ManagementCenter Umg(UmgParam);
    if (!Umg.Init())
    {
        LOG_ERROR_RLD("Failed to init user manager.");
        return 1;
    }

    ControlCenter::ParamInfo pinfo;
    pinfo.strRemoteAddress = strRemoteAddress;
    pinfo.strRemotePort = strRemotePort;
    pinfo.uiShakehandOfChannelInterval = boost::lexical_cast<unsigned int>(strShakehandOfChannelInterval);

    pinfo.strSelfID = strSelfID;
    pinfo.uiThreadOfWorking = boost::lexical_cast<unsigned int>(strThreadOfWorking);

    ControlCenter ccenter(pinfo);

    //ccenter.SetupMsgPreHandler(boost::bind(&ManagementCenter::PreCommonHandler, &Umg, _1, _2, _3));

    ccenter.SetupMsgTypeParseHandler(boost::bind(&ManagementCenter::GetMsgType, &Umg, _1, _2));

    ccenter.SetupMsgHandler(InteractiveProtoManagementHandler::ManagementMsgType::AddClusterReq_T, boost::bind(&ManagementCenter::AddClusterReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoManagementHandler::ManagementMsgType::DeleteClusterReq_T, boost::bind(&ManagementCenter::DeleteClusterReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoManagementHandler::ManagementMsgType::ModifyClusterReq_T, boost::bind(&ManagementCenter::ModifyClusterReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoManagementHandler::ManagementMsgType::QueryClusterInfoReq_T, boost::bind(&ManagementCenter::QueryClusterInfoReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoManagementHandler::ManagementMsgType::ShakehandClusterReq_T, boost::bind(&ManagementCenter::ShakehandClusterReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoManagementHandler::ManagementMsgType::QueryAllClusterReq_T, boost::bind(&ManagementCenter::QueryAllClusterReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoManagementHandler::ManagementMsgType::QueryClusterDeviceReq_T, boost::bind(&ManagementCenter::QueryClusterDeviceReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoManagementHandler::ManagementMsgType::QueryClusterUserReq_T, boost::bind(&ManagementCenter::QueryClusterUserReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoManagementHandler::ManagementMsgType::PushClusterDeviceReq_T, boost::bind(&ManagementCenter::PushClusterDeviceReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoManagementHandler::ManagementMsgType::PushClusterUserReq_T, boost::bind(&ManagementCenter::PushClusterUserReq, &Umg, _1, _2, _3));



    ccenter.Run(true);

    return 0;
}
