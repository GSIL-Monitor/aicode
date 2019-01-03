#include <stdlib.h>
#include <string.h>
#include <string>
#include <boost/filesystem.hpp> 
#include "fcgiapp.h"
#include "FCGIManager.h"
#include "HttpMsgHandler.h"
#include "ManagementAgent.h"
#include "ConfigSt.h"
#include "LogRLD.h"
#include "boost/lexical_cast.hpp"
#include "CommMsgHandler.h"
#include "CacheMgr.h"

#define CONFIG_FILE_NAME "http_server.ini"
#define PROCESS_NAME     "access_cgi"
#define FILE_VERSION     "v[1.0.0] "

void InitLog(ConfigSt& cfg)
{
    std::string strHost = "access_cgi(127.0.0.1)";
    std::string strLogPath = "./logs/";
    std::string strLogInnerShowName = PROCESS_NAME;
    int iLoglevel = LogRLD::INFO_LOG_LEVEL;
    int iSchedule = LogRLD::DAILY_LOG_SCHEDULE;
    int iMaxLogFileBackupNum = 10;

    //初始化配置信息
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
        else
        {

        }
    }

    std::string strLogInnerShowNameCfg = cfg.GetItem("Log.FileName");
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

    boost::filesystem::path LogPath(strLogPath);
    std::string strFileName = strLogInnerShowName + ".log";
    LogPath = LogPath / strFileName;

    LogRLD::GetInstance().Init(iLoglevel, strHost, strLogInnerShowName, LogPath.string(), iSchedule, iMaxLogFileBackupNum, FILE_VERSION);
}


int main(int argc, char *argv[])
{
    boost::filesystem::path currentPath = boost::filesystem::current_path() / CONFIG_FILE_NAME;

    //判断配置文件是否存在
    if (!boost::filesystem::exists(currentPath))
    {
        printf("Log config file not found and path is %s\n", currentPath.string().c_str());
        return 0;
    }
    
    ConfigSt cfg(currentPath.string());

    InitLog(cfg);
    LOG_INFO_RLD("http_server begin runing and config file is " << currentPath.string());

    const std::string &strDBHost = cfg.GetItem("DB.DBHost");
    if (strDBHost.empty())
    {
        LOG_ERROR_RLD("DBHost config item not found.");
        return 0;
    }

    const std::string &strDBPort = cfg.GetItem("DB.DBPort");
    if (strDBPort.empty())
    {
        LOG_ERROR_RLD("DBPort config item not found.");
        return 0;
    }

    const std::string &strDBUser = cfg.GetItem("DB.DBUser");
    if (strDBUser.empty())
    {
        LOG_ERROR_RLD("DBUser config item not found.");
        return 0;
    }

    const std::string &strDBPassword = cfg.GetItem("DB.DBPassword");
    if (strDBPassword.empty())
    {
        LOG_ERROR_RLD("DBPassword config item not found.");
        return 0;
    }

    const std::string &strDBName = cfg.GetItem("DB.DBName");
    if (strDBName.empty())
    {
        LOG_ERROR_RLD("DBName config item not found.");
        return 0;
    }

    const std::string &strRemoteAddress = cfg.GetItem("Channel.RemoteAddress");
    if (strRemoteAddress.empty())
    {

        LOG_ERROR_RLD("RemoteAddress config item not found.");
        return 0;
    }

    const std::string &strRemotePort = cfg.GetItem("Channel.RemotePort");
    if (strRemotePort.empty())
    {

        LOG_ERROR_RLD("RemotePort config item not found.");
        return 0;
    }

    std::string strShakehandOfChannelInterval = cfg.GetItem("Channel.ShakehandOfChannelInterval");
    if (strShakehandOfChannelInterval.empty())
    {
        LOG_ERROR_RLD("ShakehandOfChannelInterval config item not found.");
        return 0;
    }

    //
    strShakehandOfChannelInterval = "0";

    const std::string &strSelfID = cfg.GetItem("General.SelfID");
    if (strSelfID.empty())
    {
        LOG_ERROR_RLD("SelfID config item not found.");
        return 0;
    }

    const std::string &strThreadOfWorking = cfg.GetItem("General.ThreadOfWorking");
    if (strThreadOfWorking.empty())
    {
        LOG_ERROR_RLD("ThreadOfWorking config item not found.");
        return 0;
    }

    const std::string &CallFuncTimeout = cfg.GetItem("General.CallFuncTimeout");
    if (CallFuncTimeout.empty())
    {
        LOG_ERROR_RLD("Session timout threshold config item not found.");
        return 0;
    }

    const std::string &strCollectInfoTimeout = cfg.GetItem("General.CollectInfoTimeout");
    if (strCollectInfoTimeout.empty())
    {
        LOG_ERROR_RLD("Collect info timout threshold config item not found.");
        return 0;
    }
    
    const std::string &strMemcachedAddress = cfg.GetItem("MemCached.MemAddress");
    if (strMemcachedAddress.empty())
    {
        LOG_ERROR_RLD("Memcached address config item not found.");
        return 0;
    }

    const std::string &strMemcachedPort = cfg.GetItem("MemCached.MemPort");
    if (strMemcachedPort.empty())
    {
        LOG_ERROR_RLD("Memcached port config item not found.");
        return 0;
    }

    const std::string &UploadTmpPath = cfg.GetItem("General.UploadTmpPath");
    if (UploadTmpPath.empty())
    {
        LOG_ERROR_RLD("UploadTmpPath config item not found.");
        return 0;
    }

    std::string strUseBlacklist = cfg.GetItem("General.UseBlacklist");
    if (strUseBlacklist.empty())
    {
        strUseBlacklist = "true";
    }
    

    ///////////////////////////////////////////////////
    HttpMsgHandler::ParamInfo pm;
    pm.m_strRemoteAddress = strRemoteAddress;
    pm.m_strRemotePort = strRemotePort;
    pm.m_strSelfID = strSelfID;
    pm.m_uiCallFuncTimeout = boost::lexical_cast<unsigned int>(CallFuncTimeout);
    pm.m_uiShakehandOfChannelInterval = boost::lexical_cast<unsigned int>(strShakehandOfChannelInterval);
    pm.m_uiThreadOfWorking = boost::lexical_cast<unsigned int>(strThreadOfWorking);

    CommMsgHandler::SetCommRunningThreads(pm.m_uiThreadOfWorking);
    CommMsgHandler::SetTimeoutRunningThreads(pm.m_uiThreadOfWorking);

    CacheMgr sgr;
    sgr.SetMemCacheAddRess(strMemcachedAddress, strMemcachedPort);

    if (!sgr.Init())
    {
        LOG_ERROR_RLD("Cache mgr init failed.");
        return 0;
    }

    HttpMsgHandler filehdr(pm, sgr);

    ManagementAgent::ParamInfo pm2;
    pm2.m_strRemoteAddress = strRemoteAddress;
    pm2.m_strRemotePort = strRemotePort;
    pm2.m_strSelfID = strSelfID;
    pm2.m_uiCallFuncTimeout = boost::lexical_cast<unsigned int>(CallFuncTimeout);
    pm2.m_uiShakehandOfChannelInterval = boost::lexical_cast<unsigned int>(strShakehandOfChannelInterval);
    pm2.m_uiThreadOfWorking = boost::lexical_cast<unsigned int>(strThreadOfWorking);
    pm2.m_uiCollectInfoTimeout = boost::lexical_cast<unsigned int>(strCollectInfoTimeout);
    
    ManagementAgent ma(pm2);
    ma.SetMsgWriter(boost::bind(&HttpMsgHandler::WriteMsg, &filehdr, _1, _2, _3, _4));
        
    FCGIManager fcgimgr(boost::lexical_cast<unsigned int>(strThreadOfWorking));
    fcgimgr.SetUploadTmpPath(UploadTmpPath);

    fcgimgr.SetActionName("name"); //为了处理 http://dvsinfo.dvripc.net/getdevinfo.aspx?name=7174b3
    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_DEVICE_P2PID_ASP, boost::bind(&HttpMsgHandler::QueryDeviceP2pIDAspHandler, &filehdr, _1, _2));


    fcgimgr.SetMsgPreHandler(boost::bind(&HttpMsgHandler::ParseMsgOfCompact, &filehdr, _1, _2));
    if (strUseBlacklist == "true")
    {
        fcgimgr.SetMsgPreHandler(boost::bind(&HttpMsgHandler::BlacklistHandler, &filehdr, _1, _2));
    }

    fcgimgr.SetMsgHandler(ManagementAgent::ADD_CLUSTER_ACTION, boost::bind(&ManagementAgent::AddClusterAgentHandler, &ma, _1, _2));
    fcgimgr.SetMsgHandler(ManagementAgent::CLUSTER_SHAKEHAND__ACTION, boost::bind(&ManagementAgent::ClusterAgentShakehandHandler, &ma, _1, _2));
    fcgimgr.SetMsgHandler(ManagementAgent::DELETE_CLUSTER_ACTION, boost::bind(&ManagementAgent::DeleteClusterAgentHandler, &ma, _1, _2));


    fcgimgr.SetMsgHandler(HttpMsgHandler::REGISTER_USER_ACTION, boost::bind(&HttpMsgHandler::RegisterUserHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::UNREGISTER_USER_ACTION, boost::bind(&HttpMsgHandler::UnRegisterUserHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_USER_INFO_ACTION, boost::bind(&HttpMsgHandler::QueryUserInfoHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::MODIFY_USER_INFO_ACTION, boost::bind(&HttpMsgHandler::ModifyUserInfoHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::USER_LOGIN_ACTION, boost::bind(&HttpMsgHandler::UserLoginHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::USER_LOGOUT_ACTION, boost::bind(&HttpMsgHandler::UserLogoutHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::USER_SHAKEHAND_ACTION, boost::bind(&HttpMsgHandler::ShakehandHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::ADD_DEVICE_ACTION, boost::bind(&HttpMsgHandler::AddDeviceHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::DELETE_DEVICE_ACTION, boost::bind(&HttpMsgHandler::DeleteDeviceHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::MODIFY_DEVICE_ACTION, boost::bind(&HttpMsgHandler::ModifyDeviceHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_DEVICE_INFO_ACTION, boost::bind(&HttpMsgHandler::QueryDeviceHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_DEVICE_INFO_MULTIPLE_ACTION, boost::bind(&HttpMsgHandler::QueryDeviceMultipleHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_DEVICE_OF_USER_ACTION, boost::bind(&HttpMsgHandler::QueryDevicesOfUserHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_USER_OF_DEVICE_ACTION, boost::bind(&HttpMsgHandler::QueryUsersOfDeviceHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::SHARING_DEVICE_ACTION, boost::bind(&HttpMsgHandler::SharingDeviceHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::CANCELSHARED_DEVICE_ACTION, boost::bind(&HttpMsgHandler::CancelSharedDeviceHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::ADD_FRIEND_ACTION, boost::bind(&HttpMsgHandler::AddFriendsHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::DELETE_FRIEND_ACTION, boost::bind(&HttpMsgHandler::DeleteFriendsHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_FRIEND_ACTION, boost::bind(&HttpMsgHandler::QueryFriendHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::P2P_INFO_ACTION, boost::bind(&HttpMsgHandler::P2pInfoHandler, &filehdr, _1, _2));
    
    fcgimgr.SetMsgHandler(HttpMsgHandler::DEVICE_LOGIN_ACTION, boost::bind(&HttpMsgHandler::DeviceLoginHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::DEVICE_P2P_INFO_ACTION, boost::bind(&HttpMsgHandler::DeviceP2pInfoHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::DEVICE_SHAKEHAND_ACTION, boost::bind(&HttpMsgHandler::DeviceShakehandHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::DEVICE_LOGOUT_ACTION, boost::bind(&HttpMsgHandler::DeviceLogoutHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::DEVICE_SET_PROPERTY_ACTION, boost::bind(&HttpMsgHandler::DeviceSetPropertyHandler, &filehdr, _1, _2));
    
    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_USER_FILE_ACTION, boost::bind(&HttpMsgHandler::QueryUserFileHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::DOWNLOAD_USER_FILE_ACTION, boost::bind(&HttpMsgHandler::DownloadUserFileHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::DELETE_USER_FILE_ACTION, boost::bind(&HttpMsgHandler::DeleteUserFileHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::ADD_FILE_ACTION, boost::bind(&HttpMsgHandler::AddFileHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(HttpMsgHandler::RETRIEVE_PWD_ACTION, boost::bind(&HttpMsgHandler::RetrievePwdHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::DEVICE_QUERY_TIMEZONE_ACTION, boost::bind(&HttpMsgHandler::DeviceQueryTimeZoneHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(HttpMsgHandler::DEVICE_QUERY_ACCESS_DOMAIN_ACTION, boost::bind(&HttpMsgHandler::DeviceQueryAccessDomainNameHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::USER_QUERY_ACCESS_DOMAIN_ACTION, boost::bind(&HttpMsgHandler::UserQueryAccessDomainNameHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::DEVICE_QUERY_UPDATE_SERVICE_ACTION, boost::bind(&HttpMsgHandler::DeviceQueryUpdateServiceHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_UPLOAD_URL_ACTION, boost::bind(&HttpMsgHandler::QueryUploadURLHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::ADD_CONFIG_ACTION, boost::bind(&HttpMsgHandler::AddConfigurationHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::DELETE_CONFIG_ACTION, boost::bind(&HttpMsgHandler::DeleteConfigurationHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::MOD_CONFIG_ACTION, boost::bind(&HttpMsgHandler::ModifyConfigurationHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_CONFIG_ACTION, boost::bind(&HttpMsgHandler::QueryConfigurationHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_APP_UPGRADE_ACTION, boost::bind(&HttpMsgHandler::QueryAppUpgradeHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_DEV_UPGRADE_ACTION, boost::bind(&HttpMsgHandler::QueryDevUpgradeHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_DEVICE_PARAM_ACTION, boost::bind(&HttpMsgHandler::QueryDevParamHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::CHECK_DEVICE_P2PID_ACTION, boost::bind(&HttpMsgHandler::CheckDeviceP2pidHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_PUSH_STATUS_ACTION, boost::bind(&HttpMsgHandler::QueryPushStatusHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::DEVICE_EVENT_REPORT_ACTION, boost::bind(&HttpMsgHandler::DeviceEventReportHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_DEVICE_EVENT_ACTION, boost::bind(&HttpMsgHandler::QueryDeviceEventHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::DELETE_DEVICE_EVENT_ACTION, boost::bind(&HttpMsgHandler::DeleteDeviceEventHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::MODIFY_DEVICE_EVENT_ACTION, boost::bind(&HttpMsgHandler::ModifyDeviceEventHandler, &filehdr, _1, _2));


    fcgimgr.SetMsgHandler(HttpMsgHandler::ADD_USER_SPACE_ACTION, boost::bind(&HttpMsgHandler::AddUserSpaceHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::DELETE_USER_SPACE_ACTION, boost::bind(&HttpMsgHandler::DeleteUserSpaceHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::MODIFY_USER_SPACE_ACTION, boost::bind(&HttpMsgHandler::ModifyUserSpaceHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_USER_SPACE_ACTION, boost::bind(&HttpMsgHandler::QueryUserSpaceHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_STORAGE_SPACE_ACTION, boost::bind(&HttpMsgHandler::QueryStorageSpaceHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(HttpMsgHandler::CMS_CALL_ACTION, boost::bind(&HttpMsgHandler::CmsCallHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::REGISTER_CMSCALL_ACTION, boost::bind(&HttpMsgHandler::RegisterCmsCallHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::UNREGISTER_CMSCALL_ACTION, boost::bind(&HttpMsgHandler::UnregisterCmsCallHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_SHARING_DEVICE_LIMIT_ACTION, boost::bind(&HttpMsgHandler::QuerySharingDeviceLimitHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_DEVICE_CAPACITY_ACTION, boost::bind(&HttpMsgHandler::QueryDeviceCapacityHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_ALL_DEVICE_CAPACITY_ACTION, boost::bind(&HttpMsgHandler::QueryAllDeviceCapacityHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_DEVICE_P2PID, boost::bind(&HttpMsgHandler::QueryDeviceP2pIDHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(HttpMsgHandler::UPLOAD_USR_CFG_ACTION, boost::bind(&HttpMsgHandler::UploadUserCfgHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_USER_CFG_ACTION, boost::bind(&HttpMsgHandler::QueryUserCfgHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(HttpMsgHandler::ADD_BLACK_ID, boost::bind(&HttpMsgHandler::AddBlackIDHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::REMOVE_BLACK_ID, boost::bind(&HttpMsgHandler::RemoveBlackIDHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(HttpMsgHandler::QUERY_ALL_BLACK_LIST, boost::bind(&HttpMsgHandler::QueryAllBlackListHandler, &filehdr, _1, _2));



    fcgimgr.Run(true);
    return 0;
}



