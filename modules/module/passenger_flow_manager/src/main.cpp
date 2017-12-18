#include "ForkChild.h"
#include "NetComm.h"
#include <list>
#include <string>
#include <map>
#include "LogRLD.h"
#include "ConfigSt.h"
#include <boost/filesystem.hpp> 
#include <boost/lexical_cast.hpp>
#include "PassengerFlowManager.h"
#include "ControlCenter.h"

#define CONFIG_FILE_NAME "passenger_flow_manager.ini"
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
    std::string strHost = "passenger_flow_manager(127.0.0.1)";
    std::string strLogPath = "./logs/";
    std::string strLogInnerShowName = "passenger_flow_manager";
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

    ////////////////////////////////////////////////////////////////////////////

    PassengerFlowManager::ParamInfo UmgParam;
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

    PassengerFlowManager Umg(UmgParam);
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

    ccenter.SetupMsgPreHandler(boost::bind(&PassengerFlowManager::PreCommonHandler, &Umg, _1, _2, _3));

    ccenter.SetupMsgTypeParseHandler(boost::bind(&PassengerFlowManager::GetMsgType, &Umg, _1, _2));

    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::AddAreaReq_T, boost::bind(&PassengerFlowManager::AddAreaReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteAreaReq_T, boost::bind(&PassengerFlowManager::DeleteAreaReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyAreaReq_T, boost::bind(&PassengerFlowManager::ModifyAreaReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllAreaReq_T, boost::bind(&PassengerFlowManager::QueryAllAreaReq, &Umg, _1, _2, _3));

    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::BindPushClientIDReq_T, boost::bind(&PassengerFlowManager::BindPushClientIDReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::UnbindPushClientIDReq_T, boost::bind(&PassengerFlowManager::UnbindPushClientIDReq, &Umg, _1, _2, _3));

    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::AddStoreReq_T, boost::bind(&PassengerFlowManager::AddStoreReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteStoreReq_T, boost::bind(&PassengerFlowManager::DeleteStoreReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyStoreReq_T, boost::bind(&PassengerFlowManager::ModifyStoreReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QueryStoreInfoReq_T, boost::bind(&PassengerFlowManager::QueryStoreInfoReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllStoreReq_T, boost::bind(&PassengerFlowManager::QueryAllStoreReq, &Umg, _1, _2, _3));

    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::AddEntranceReq_T, boost::bind(&PassengerFlowManager::AddEntranceReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteEntranceReq_T, boost::bind(&PassengerFlowManager::DeleteEntranceReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyEntranceReq_T, boost::bind(&PassengerFlowManager::ModifyEntranceReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::AddEntranceDeviceReq_T, boost::bind(&PassengerFlowManager::AddEntranceDeviceReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteEntranceDeviceReq_T, boost::bind(&PassengerFlowManager::DeleteEntranceDeviceReq, &Umg, _1, _2, _3));

    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::AddEventReq_T, boost::bind(&PassengerFlowManager::AddEventReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteEventReq_T, boost::bind(&PassengerFlowManager::DeleteEventReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyEventReq_T, boost::bind(&PassengerFlowManager::ModifyEventReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QueryEventInfoReq_T, boost::bind(&PassengerFlowManager::QueryEventInfoReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllEventReq_T, boost::bind(&PassengerFlowManager::QueryAllEventReq, &Umg, _1, _2, _3));

    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::AddSmartGuardStoreReq_T, boost::bind(&PassengerFlowManager::AddSmartGuardStoreReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteSmartGuardStoreReq_T, boost::bind(&PassengerFlowManager::DeleteSmartGuardStoreReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::ModifySmartGuardStoreReq_T, boost::bind(&PassengerFlowManager::ModifySmartGuardStoreReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QuerySmartGuardStoreInfoReq_T, boost::bind(&PassengerFlowManager::QuerySmartGuardStoreInfoReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllSmartGuardStoreReq_T, boost::bind(&PassengerFlowManager::QueryAllSmartGuardStoreReq, &Umg, _1, _2, _3));

    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::AddRegularPatrolReq_T, boost::bind(&PassengerFlowManager::AddRegularPatrolReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteRegularPatrolReq_T, boost::bind(&PassengerFlowManager::DeleteRegularPatrolReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyRegularPatrolReq_T, boost::bind(&PassengerFlowManager::ModifyRegularPatrolReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QueryRegularPatrolInfoReq_T, boost::bind(&PassengerFlowManager::QueryRegularPatrolInfoReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllRegularPatrolReq_T, boost::bind(&PassengerFlowManager::QueryAllRegularPatrolReq, &Umg, _1, _2, _3));

    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::UserJoinStoreReq_T, boost::bind(&PassengerFlowManager::UserJoinStoreReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::UserQuitStoreReq_T, boost::bind(&PassengerFlowManager::UserQuitStoreReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QueryStoreAllUserReq_T, boost::bind(&PassengerFlowManager::QueryStoreAllUserReq, &Umg, _1, _2, _3));

    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::AddVIPCustomerReq_T, boost::bind(&PassengerFlowManager::AddVIPCustomerReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteVIPCustomerReq_T, boost::bind(&PassengerFlowManager::DeleteVIPCustomerReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyVIPCustomerReq_T, boost::bind(&PassengerFlowManager::ModifyVIPCustomerReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QueryVIPCustomerInfoReq_T, boost::bind(&PassengerFlowManager::QueryVIPCustomerInfoReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllVIPCustomerReq_T, boost::bind(&PassengerFlowManager::QueryAllVIPCustomerReq, &Umg, _1, _2, _3));

    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::AddVIPConsumeHistoryReq_T, boost::bind(&PassengerFlowManager::AddVIPConsumeHistoryReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteVIPConsumeHistoryReq_T, boost::bind(&PassengerFlowManager::DeleteVIPConsumeHistoryReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyVIPConsumeHistoryReq_T, boost::bind(&PassengerFlowManager::ModifyVIPConsumeHistoryReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllVIPConsumeHistoryReq_T, boost::bind(&PassengerFlowManager::QueryAllVIPConsumeHistoryReq, &Umg, _1, _2, _3));

    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::AddEvaluationTemplateReq_T, boost::bind(&PassengerFlowManager::AddEvaluationTemplateReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteEvaluationTemplateReq_T, boost::bind(&PassengerFlowManager::DeleteEvaluationTemplateReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyEvaluationTemplateReq_T, boost::bind(&PassengerFlowManager::ModifyEvaluationTemplateReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllEvaluationTemplateReq_T, boost::bind(&PassengerFlowManager::QueryAllEvaluationTemplateReq, &Umg, _1, _2, _3));

    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::AddStoreEvaluationReq_T, boost::bind(&PassengerFlowManager::AddStoreEvaluationReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteStoreEvaluationReq_T, boost::bind(&PassengerFlowManager::DeleteStoreEvaluationReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyStoreEvaluationReq_T, boost::bind(&PassengerFlowManager::ModifyStoreEvaluationReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QueryStoreEvaluationInfoReq_T, boost::bind(&PassengerFlowManager::QueryStoreEvaluationInfoReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllStoreEvaluationReq_T, boost::bind(&PassengerFlowManager::QueryAllStoreEvaluationReq, &Umg, _1, _2, _3));

    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::AddRemotePatrolStoreReq_T, boost::bind(&PassengerFlowManager::AddRemotePatrolStoreReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteRemotePatrolStoreReq_T, boost::bind(&PassengerFlowManager::DeleteRemotePatrolStoreReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyRemotePatrolStoreReq_T, boost::bind(&PassengerFlowManager::ModifyRemotePatrolStoreReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QueryRemotePatrolStoreInfoReq_T, boost::bind(&PassengerFlowManager::QueryRemotePatrolStoreInfoReq, &Umg, _1, _2, _3));
    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllRemotePatrolStoreReq_T, boost::bind(&PassengerFlowManager::QueryAllRemotePatrolStoreReq, &Umg, _1, _2, _3));

    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::ImportPOSDataReq_T, boost::bind(&PassengerFlowManager::ImportPOSDataReq, &Umg, _1, _2, _3));

    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::QueryCustomerFlowStatisticReq_T, boost::bind(&PassengerFlowManager::QueryCustomerFlowStatisticReq, &Umg, _1, _2, _3));

    ccenter.SetupMsgHandler(PassengerFlowProtoHandler::CustomerFlowMsgType::ReportCustomerFlowDataReq_T, boost::bind(&PassengerFlowManager::ReportCustomerFlowDataReq, &Umg, _1, _2, _3));


    ccenter.Run(true);

    return 0;
}
