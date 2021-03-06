#include "ForkChild.h"
#include "NetComm.h"
#include <list>
#include <string>
#include <map>
#include "LogRLD.h"
#include "ConfigSt.h"
#include <boost/filesystem.hpp> 
#include <boost/lexical_cast.hpp>
#include "ProductManager.h"
#include "ProductServer.h"

#define CONFIG_FILE_NAME "product_manager.ini"
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
    std::string strHost = "product_manager(127.0.0.1)";
    std::string strLogPath = "./logs/";
    std::string strLogInnerShowName = "product_manager";
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

    LOG_INFO_RLD("Begin runing and daemon status is " << IsNeedDaemonRun);

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

    const std::string &strDevSessionTimeoutCountThreshold = GetConfig("General.DeviceSessionTimeoutCountThreshold");
    if (strDevSessionTimeoutCountThreshold.empty())
    {
        LOG_ERROR_RLD("Device session timout threshold config item not found.");
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

    const std::string &strFileServerURL = GetConfig("General.FileServerURL");
    if (strFileServerURL.empty())
    {
        LOG_ERROR_RLD("FileServerURL config item not found.");
        return 0;
    }

    const std::string &strGetIpInfoSite = GetConfig("General.GetIpInfoSite");
    if (strGetIpInfoSite.empty())
    {
        LOG_ERROR_RLD("GetIpInfoSite config item not found.");
        return 0;
    }

    const std::string &strMemcachedAddressGlobal = GetConfig("MemCached.MemAddressGlobal");
    if (strMemcachedAddressGlobal.empty())
    {
        LOG_INFO_RLD("Memcached of global address config item not found.");
    }

    const std::string &strMemcachedPortGlobal = GetConfig("MemCached.MemPortGlobal");
    if (strMemcachedPortGlobal.empty())
    {
        LOG_INFO_RLD("Memcached of global port config item not found.");
    }

    const std::string &strUserLoginMutex = GetConfig("General.UserLoginMutex");
    if (strUserLoginMutex.empty())
    {
        LOG_ERROR_RLD("UserLoginMutex config item not found.");
        return 0;
    }

    const std::string &strUserAllowDiffTerminal = GetConfig("General.UserAllowDiffTerminal");
    if (strUserAllowDiffTerminal.empty())
    {
        LOG_ERROR_RLD("UserAllowDiffTerminal config item not found.");
        return 0;
    }

    const std::string &strUserKickoutType = GetConfig("General.UserKickoutType");
    if (strUserKickoutType.empty())
    {
        LOG_ERROR_RLD("UserKickoutType config item not found.");
        return 0;
    }

    const std::string &strMasterNode = GetConfig("General.MasterNode");
    if (strMasterNode.empty())
    {
        LOG_ERROR_RLD("MasterNode config item not found.");
    }

    const std::string &strServerUrl = GetConfig("Push.ServerUrl");
    if (strServerUrl.empty())
    {
        LOG_ERROR_RLD("ServerUrl config item not found.");
    }

    const std::string &strAuthExpire = GetConfig("Push.AuthExpire");
    if (strAuthExpire.empty())
    {
        LOG_ERROR_RLD("AuthExpire config item not found.");
    }

    const std::string &strAndroidAppID = GetConfig("Push.AndroidAppID");
    if (strAndroidAppID.empty())
    {
        LOG_ERROR_RLD("AndroidAppID config item not found.");
    }

    const std::string &strAndroidAppKey = GetConfig("Push.AndroidAppKey");
    if (strAndroidAppKey.empty())
    {
        LOG_ERROR_RLD("AndroidAppKey config item not found.");
    }

    const std::string &strAndroidMasterSecret = GetConfig("Push.AndroidMasterSecret");
    if (strAndroidMasterSecret.empty())
    {
        LOG_ERROR_RLD("AndroidMasterSecret config item not found.");
    }

    const std::string &strIOSAppID = GetConfig("Push.IOSAppID");
    if (strIOSAppID.empty())
    {
        LOG_ERROR_RLD("IOSAppID config item not found.");
    }

    const std::string &strIOSAppKey = GetConfig("Push.IOSAppKey");
    if (strIOSAppKey.empty())
    {
        LOG_ERROR_RLD("IOSAppKey config item not found.");
    }

    const std::string &strIOSMasterSecret = GetConfig("Push.IOSMasterSecret");
    if (strIOSMasterSecret.empty())
    {
        LOG_ERROR_RLD("IOSMasterSecret config item not found.");
    }

    const std::string &strMessageTitle = GetConfig("Push.MessageTitle");
    if (strMessageTitle.empty())
    {
        LOG_ERROR_RLD("MessageTitle config item not found.");
    }

    const std::string &strMessageContentRegularPatrol = GetConfig("Push.MessageContentRegularPatrol");
    if (strMessageContentRegularPatrol.empty())
    {
        LOG_ERROR_RLD("MessageContentRegularPatrol config item not found.");
    }

    const std::string &strMessageContentRemotePatrol = GetConfig("Push.MessageContentRemotePatrol");
    if (strMessageContentRemotePatrol.empty())
    {
        LOG_ERROR_RLD("MessageContentRemotePatrol config item not found.");
    }

    const std::string &strMessageContentEvaluation = GetConfig("Push.MessageContentEvaluation");
    if (strMessageContentEvaluation.empty())
    {
        LOG_ERROR_RLD("MessageContentEvaluation config item not found.");
    }

    const std::string &strAlarmMessageTitle = GetConfig("Push.AlarmMessageTitle");
    if (strAlarmMessageTitle.empty())
    {
        LOG_ERROR_RLD("AlarmMessageTitle config item not found.");
    }

    const std::string &strAlarmMessageContentCreated = GetConfig("Push.AlarmMessageContentCreated");
    if (strAlarmMessageContentCreated.empty())
    {
        LOG_ERROR_RLD("AlarmMessageContentCreated config item not found.");
    }


    ////////////////////////////////////////////////////////////////////////////

    ProductManager::ParamInfo UmgParam;
    UmgParam.m_strDBHost = strDBHost;
    UmgParam.m_strDBName = strDBName;
    UmgParam.m_strDBPassword = strDBPassword;
    UmgParam.m_strDBPort = strDBPort;
    UmgParam.m_strDBUser = strDBUser;
    UmgParam.m_strMemAddress = strMemcachedAddress;
    UmgParam.m_strMemPort = strMemcachedPort;
    UmgParam.m_strSessionTimeoutCountThreshold = strSessionTimeoutCountThreshold;
    UmgParam.m_strDevSessionTimeoutCountThreshold = strDevSessionTimeoutCountThreshold;
    UmgParam.m_strFileServerURL = strFileServerURL;
    UmgParam.m_strGetIpInfoSite = strGetIpInfoSite;
    UmgParam.m_strMemAddressGlobal = strMemcachedAddressGlobal;
    UmgParam.m_strMemPortGlobal = strMemcachedPortGlobal;
    UmgParam.m_strUserLoginMutex = strUserLoginMutex;
    UmgParam.m_strUserAllowDiffTerminal = strUserAllowDiffTerminal;
    UmgParam.m_strUserKickoutType = strUserKickoutType;
    UmgParam.m_strMasterNode = strMasterNode;
    UmgParam.m_strPushServerUrl = strServerUrl;
    UmgParam.m_iAuthExpire = boost::lexical_cast<int>(strAuthExpire);
    UmgParam.m_strAndroidAppID = strAndroidAppID;
    UmgParam.m_strAndroidAppKey = strAndroidAppKey;
    UmgParam.m_strAndroidMasterSecret = strAndroidMasterSecret;
    UmgParam.m_strIOSAppID = strIOSAppID;
    UmgParam.m_strIOSAppKey = strIOSAppKey;
    UmgParam.m_strIOSMasterSecret = strIOSMasterSecret;
    UmgParam.m_strMessageTitle = strMessageTitle;
    UmgParam.m_strMessageContentRegularPatrol = strMessageContentRegularPatrol;
    UmgParam.m_strMessageContentRemotePatrol = strMessageContentRemotePatrol;
    UmgParam.m_strMessageContentEvaluation = strMessageContentEvaluation;
    UmgParam.m_strAlarmMessageTitle = strAlarmMessageTitle;
    UmgParam.m_strAlarmMessageContentCreated = strAlarmMessageContentCreated;
        
    boost::shared_ptr<ProductManager> pPm(new ProductManager(UmgParam));
    if (!pPm->Init())
    {
        LOG_ERROR_RLD("Failed to init product manager.");
        return 1;
    }

    ProductServer::Param pam;
    pam.m_iServerPort = boost::lexical_cast<int>(strRemotePort);
    pam.m_uiThreadNum = boost::lexical_cast<unsigned int>(strThreadOfWorking);

    ProductServer ps(pPm);
    ps.Init(pam);

    ps.GetOrderServerHandler()->SetBeforeHandler(boost::bind(&ProductManager::PreCommonHandler, pPm, _1, _2, _3));
    ps.GetProductServerHandler()->SetBeforeHandler(boost::bind(&ProductManager::PreCommonHandler, pPm, _1, _2, _3));
    
    ps.Run();

    return 0;
}
