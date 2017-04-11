#include "ForkChild.h"
#include "NetComm.h"
#include <list>
#include <string>
#include <map>
#include "LogRLD.h"
#include "ConfigSt.h"
#include <boost/filesystem.hpp> 
#include <boost/lexical_cast.hpp>
#include "AccessManager.h"
#include "ControlCenter.h"

#define CONFIG_FILE_NAME "access_manager.ini"
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
    std::string strHost = "AccessManager(127.0.0.1)";
    std::string strLogPath = "./logs/";
    std::string strLogInnerShowName = "AccessManager";
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
    std::string strFileName =strLogInnerShowName + ".log";
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

    LOG_INFO_RLD("AccessManager begin runing and daemon status is " << IsNeedDaemonRun);

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

    const std::string &strLTUserSite = GetConfig("General.LTUserSite");
    if (strLTUserSite.empty())
    {
        LOG_ERROR_RLD("LTUserSite config item not found.");
        return 0;
    }

    const std::string &strLTUserSiteRC4Key = GetConfig("General.LTUserSiteRC4Key");
    if (strLTUserSiteRC4Key.empty())
    {
        LOG_ERROR_RLD("LTUserSiteRC4Key config item not found.");
        return 0;
    }

    const std::string &strUploadURL = GetConfig("General.UploadURL");
    if (strUploadURL.empty())
    {
        LOG_ERROR_RLD("UploadURL config item not found.");
        return 0;
    }


    ////////////////////////////////////////////////////////////////////////////

    AccessManager::ParamInfo UmgParam;
    UmgParam.m_strDBHost = strDBHost;
    UmgParam.m_strDBName = strDBName;
    UmgParam.m_strDBPassword = strDBPassword;
    UmgParam.m_strDBPort = strDBPort;
    UmgParam.m_strDBUser = strDBUser;
    UmgParam.m_strMemAddress = strMemcachedAddress;
    UmgParam.m_strMemPort = strMemcachedPort;
    UmgParam.m_strSessionTimeoutCountThreshold = strSessionTimeoutCountThreshold;
    UmgParam.m_strLTUserSite = strLTUserSite;
    UmgParam.m_strLTUserSiteRC4Key = strLTUserSiteRC4Key;
    UmgParam.m_strUploadURL = strUploadURL;


    AccessManager Umg(UmgParam);
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

    ccenter.SetupMsgPreHandler(boost::bind(&AccessManager::PreCommonHandler, &Umg, _1, _2, _3));

    ccenter.SetupMsgTypeParseHandler(boost::bind(&AccessManager::GetMsgType, &Umg, _1, _2));

    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::RegisterUserReq_USR_T, boost::bind(&AccessManager::RegisterUserReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::UnRegisterUserReq_USR_T, boost::bind(&AccessManager::UnRegisterUserReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::QueryUsrInfoReq_USR_T, boost::bind(&AccessManager::QueryUsrInfoReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::ModifyUserInfoReq_USR_T, boost::bind(&AccessManager::ModifyUsrInfoReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::LoginReq_USR_T, boost::bind(&AccessManager::LoginReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::LogoutReq_USR_T, boost::bind(&AccessManager::LogoutReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::ShakehandReq_USR_T, boost::bind(&AccessManager::ShakehandReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::AddDevReq_USR_T, boost::bind(&AccessManager::AddDeviceReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::DelDevReq_USR_T, boost::bind(&AccessManager::DelDeviceReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::ModifyDevReq_USR_T, boost::bind(&AccessManager::ModDeviceReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::QueryDevInfoReq_USR_T, boost::bind(&AccessManager::QueryDevInfoReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::QueryDevReq_USR_T, boost::bind(&AccessManager::QueryDeviceReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::QueryUserReq_USR_T, boost::bind(&AccessManager::QueryUserReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::SharingDevReq_USR_T, boost::bind(&AccessManager::SharingDeviceReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::CancelSharedDevReq_USR_T, boost::bind(&AccessManager::CancelSharedDeviceReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::AddFriendsReq_USR_T, boost::bind(&AccessManager::AddFriendsReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::DelFriendsReq_USR_T, boost::bind(&AccessManager::DelFriendsReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::QueryFriendsReq_USR_T, boost::bind(&AccessManager::QueryFriendsReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::DeleteFileReq_USR_T, boost::bind(&AccessManager::DeleteFileReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::DownloadFileReq_USR_T, boost::bind(&AccessManager::DownloadFileReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::QueryFileReq_USR_T, boost::bind(&AccessManager::QueryFileReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::AddFileReq_DEV_T, boost::bind(&AccessManager::AddFileReqDevice, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::P2pInfoReq_USR_T, boost::bind(&AccessManager::P2pInfoReqUser, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::RetrievePwdReq_USR_T, boost::bind(&AccessManager::RetrievePwdReqUser, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::QueryTimeZoneReq_DEV_T, boost::bind(&AccessManager::QueryTimeZoneReqDevice, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::QueryAccessDomainNameReq_USR_T, boost::bind(&AccessManager::QueryAccessDomainNameReqUser, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::QueryAccessDomainNameReq_DEV_T, boost::bind(&AccessManager::QueryAccessDomainNameReqDevice, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::QueryUpgradeSiteReq_DEV_T, boost::bind(&AccessManager::QueryUpgradeSiteReqDevice, &Umg, _1, _2, _3));

    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::LoginReq_DEV_T, boost::bind(&AccessManager::LoginReqDevice, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::P2pInfoReq_DEV_T, boost::bind(&AccessManager::P2pInfoReqDevice, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::ShakehandReq_DEV_T, boost::bind(&AccessManager::ShakehandReqDevice, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::LogoutReq_DEV_T, boost::bind(&AccessManager::LogoutReqDevice, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::GetDeviceAccessRecordReq_INNER_T, boost::bind(&AccessManager::GetDeviceAccessRecordReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::GetUserAccessRecordReq_INNER_T, boost::bind(&AccessManager::GetUserAccessRecordReq, &Umg, _1, _2, _3));

    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::QueryAppUpgradeReq_USR_T, boost::bind(&AccessManager::QueryAppUpgradeReqUser, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::QueryFirmwareUpgradeReq_DEV_T, boost::bind(&AccessManager::QueryFirmwareUpgradeReqDevice, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::AddConfigurationReq_MGR_T, boost::bind(&AccessManager::AddConfigurationReqMgr, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::DeleteConfigurationReq_MGR_T, boost::bind(&AccessManager::DeleteConfigurationReqMgr, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::ModifyConfigurationReq_MGR_T, boost::bind(&AccessManager::ModifyConfigurationReqMgr, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(InteractiveProtoHandler::MsgType::QueryAllConfigurationReq_MGR_T, boost::bind(&AccessManager::QueryAllConfigurationReqMgr, &Umg, _1, _2, _3));


    ccenter.Run(true);

    return 0;
}
