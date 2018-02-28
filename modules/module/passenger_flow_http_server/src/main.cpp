#include <stdlib.h>
#include <string.h>
#include <string>
#include <boost/filesystem.hpp> 
#include "fcgiapp.h"
#include "FCGIManager.h"
#include "PassengerFlowMsgHandler.h"
#include "ManagementAgent.h"
#include "ConfigSt.h"
#include "LogRLD.h"
#include "boost/lexical_cast.hpp"
#include "CommMsgHandler.h"

#define CONFIG_FILE_NAME "http_server.ini"
#define PROCESS_NAME     "passenger_flow"
#define FILE_VERSION     "v[1.0.0] "

void InitLog(ConfigSt& cfg)
{
    std::string strHost = "passenger_flow(127.0.0.1)";
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


    ///////////////////////////////////////////////////
    PassengerFlowMsgHandler::ParamInfo pm;
    pm.m_strRemoteAddress = strRemoteAddress;
    pm.m_strRemotePort = strRemotePort;
    pm.m_strSelfID = strSelfID;
    pm.m_uiCallFuncTimeout = boost::lexical_cast<unsigned int>(CallFuncTimeout);
    pm.m_uiShakehandOfChannelInterval = boost::lexical_cast<unsigned int>(strShakehandOfChannelInterval);
    pm.m_uiThreadOfWorking = boost::lexical_cast<unsigned int>(strThreadOfWorking);

    CommMsgHandler::SetCommRunningThreads(pm.m_uiThreadOfWorking);
    CommMsgHandler::SetTimeoutRunningThreads(pm.m_uiThreadOfWorking);

    PassengerFlowMsgHandler filehdr(pm);

    ManagementAgent::ParamInfo pm2;
    pm2.m_strRemoteAddress = strRemoteAddress;
    pm2.m_strRemotePort = strRemotePort;
    pm2.m_strSelfID = strSelfID;
    pm2.m_uiCallFuncTimeout = boost::lexical_cast<unsigned int>(CallFuncTimeout);
    pm2.m_uiShakehandOfChannelInterval = boost::lexical_cast<unsigned int>(strShakehandOfChannelInterval);
    pm2.m_uiThreadOfWorking = boost::lexical_cast<unsigned int>(strThreadOfWorking);
    pm2.m_uiCollectInfoTimeout = boost::lexical_cast<unsigned int>(strCollectInfoTimeout);
    
    ManagementAgent ma(pm2);
    ma.SetMsgWriter(boost::bind(&PassengerFlowMsgHandler::WriteMsg, &filehdr, _1, _2, _3, _4));
        
    FCGIManager fcgimgr(boost::lexical_cast<unsigned int>(strThreadOfWorking));
    fcgimgr.SetUploadTmpPath(UploadTmpPath);

    fcgimgr.SetMsgPreHandler(boost::bind(&PassengerFlowMsgHandler::ParseMsgOfCompact, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(ManagementAgent::ADD_CLUSTER_ACTION, boost::bind(&ManagementAgent::AddClusterAgentHandler, &ma, _1, _2));
    fcgimgr.SetMsgHandler(ManagementAgent::CLUSTER_SHAKEHAND__ACTION, boost::bind(&ManagementAgent::ClusterAgentShakehandHandler, &ma, _1, _2));
    fcgimgr.SetMsgHandler(ManagementAgent::DELETE_CLUSTER_ACTION, boost::bind(&ManagementAgent::DeleteClusterAgentHandler, &ma, _1, _2));

    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::CREATE_DOMAIN, boost::bind(&PassengerFlowMsgHandler::CreateDomainHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::REMOVE_DOMAIN, boost::bind(&PassengerFlowMsgHandler::RemoveDomainHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::MODIFY_DOMAIN, boost::bind(&PassengerFlowMsgHandler::ModifyDomainHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_DOMAIN, boost::bind(&PassengerFlowMsgHandler::QueryDomainHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_ALL_DOMAIN, boost::bind(&PassengerFlowMsgHandler::QueryAllDomainHandler, &filehdr, _1, _2));


    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::ADD_STORE_ACTION, boost::bind(&PassengerFlowMsgHandler::AddStoreHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::DEL_STORE_ACTION, boost::bind(&PassengerFlowMsgHandler::DelStoreHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::MOD_STORE_ACTION, boost::bind(&PassengerFlowMsgHandler::ModifyStoreHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_STORE_ACTION, boost::bind(&PassengerFlowMsgHandler::QueryStoreHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_ALL_STORE_ACTION, boost::bind(&PassengerFlowMsgHandler::QueryAllStoreHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::CREATE_ENTRANCE_ACTION, boost::bind(&PassengerFlowMsgHandler::AddEntranceHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::DELETE_ENTRANCE_ACTION, boost::bind(&PassengerFlowMsgHandler::DelEntranceHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::MODIFY_ENTRANCE_ACTION, boost::bind(&PassengerFlowMsgHandler::ModifyEntranceHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::UPLOAD_PASSENGER_FLOW_ACTION, boost::bind(&PassengerFlowMsgHandler::UploadPassengerFlowHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::BIND_ENTRANCE_DEVICE, boost::bind(&PassengerFlowMsgHandler::BindEntranceDeviceHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::UNBIND_ENTRANCE_DEVICE, boost::bind(&PassengerFlowMsgHandler::UnBindEntranceDeviceHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::IMPORT_POS_DATA, boost::bind(&PassengerFlowMsgHandler::ImportPosDataHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_PASSENGER_FLOW_REPORT, boost::bind(&PassengerFlowMsgHandler::QueryPassengerFlowReportHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::REPORT_EVENT, boost::bind(&PassengerFlowMsgHandler::ReportEventHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::DELETE_EVENT, boost::bind(&PassengerFlowMsgHandler::DeleteEventHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::MODIFY_EVENT, boost::bind(&PassengerFlowMsgHandler::ModifyEventHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_EVENT, boost::bind(&PassengerFlowMsgHandler::QueryEventHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_ALL_EVENT, boost::bind(&PassengerFlowMsgHandler::QueryAllEventHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::CREATE_GUARD_PLAN, boost::bind(&PassengerFlowMsgHandler::CreateGuardStorePlanHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::DELETE_GUARD_PLAN, boost::bind(&PassengerFlowMsgHandler::DeleteGuardStorePlanHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::MODIFY_GUARD_PLAN, boost::bind(&PassengerFlowMsgHandler::ModifyGuardStorePlanHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_GUARD_PLAN, boost::bind(&PassengerFlowMsgHandler::QueryGuardStorePlanHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_ALL_GUARD_PLAN, boost::bind(&PassengerFlowMsgHandler::QueryAllGuardStorePlanHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::CREATE_PATROL_PLAN, boost::bind(&PassengerFlowMsgHandler::CreateRegularPatrolHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::DELETE_PATROL_PLAN, boost::bind(&PassengerFlowMsgHandler::DeleteRegularPatrolPlanHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::MODIFY_PATROL_PLAN, boost::bind(&PassengerFlowMsgHandler::ModifyRegularPatrolPlanHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_PATROL_PLAN, boost::bind(&PassengerFlowMsgHandler::QueryRegularPatrolPlanHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_ALL_PATROL_PLAN, boost::bind(&PassengerFlowMsgHandler::QueryAllRegularPatrolPlanHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::CREATE_VIP, boost::bind(&PassengerFlowMsgHandler::CreateVIPHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::DELETE_VIP, boost::bind(&PassengerFlowMsgHandler::DeleteVIPHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::MODIFY_VIP, boost::bind(&PassengerFlowMsgHandler::ModifyVIPHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_VIP, boost::bind(&PassengerFlowMsgHandler::QueryVIPHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_ALL_VIP, boost::bind(&PassengerFlowMsgHandler::QueryAllVIPHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::CREATE_VIP_CONSUME, boost::bind(&PassengerFlowMsgHandler::CreateVIPConsumeHistoryHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::DELETE_VIP_CONSUME, boost::bind(&PassengerFlowMsgHandler::DeleteVIPConsumeHistoryHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::MODIFY_VIP_CONSUME, boost::bind(&PassengerFlowMsgHandler::ModifyVIPConsumeHistoryHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_VIP_CONSUME, boost::bind(&PassengerFlowMsgHandler::QueryVIPConsumeHistoryHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::USER_JOIN_STORE, boost::bind(&PassengerFlowMsgHandler::UserJoinStoreHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::USER_QUIT_STORE, boost::bind(&PassengerFlowMsgHandler::UserQuitStoreHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_USER_STORE, boost::bind(&PassengerFlowMsgHandler::QueryUserOfStoreHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_ALL_USER_LIST, boost::bind(&PassengerFlowMsgHandler::QueryAllUserListHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::CREATE_EVALUATION_TEMPLATE, boost::bind(&PassengerFlowMsgHandler::CreateEvaluationTemplateHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::DELETE_EVALUATION_TEMPLATE, boost::bind(&PassengerFlowMsgHandler::DeleteEvaluationTemplateHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::MODIFY_EVALUATION_TEMPLATE, boost::bind(&PassengerFlowMsgHandler::ModifyEvaluationTemplateHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_EVALUATION_TEMPLATE, boost::bind(&PassengerFlowMsgHandler::QueryEvaluationTemplateHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::CREATE_EVALUATION_OF_STORE, boost::bind(&PassengerFlowMsgHandler::CreateEvaluationOfStoreHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::DELETE_EVALUATION_OF_STORE, boost::bind(&PassengerFlowMsgHandler::DeleteEvaluationOfStoreHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::MODIFY_EVALUATION_OF_STORE, boost::bind(&PassengerFlowMsgHandler::ModifyEvaluationOfStoreHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_EVALUATION_OF_STORE, boost::bind(&PassengerFlowMsgHandler::QueryEvaluationOfStoreHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_ALL_EVALUATION_OF_STORE, boost::bind(&PassengerFlowMsgHandler::QueryAllEvaluationOfStoreHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::CREATE_PATROL_RECORD, boost::bind(&PassengerFlowMsgHandler::CreatePatrolRecordHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::DELETE_PATROL_RECORD, boost::bind(&PassengerFlowMsgHandler::DeletePatrolRecordHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::MODIFY_PATROL_RECORD, boost::bind(&PassengerFlowMsgHandler::ModifyPatrolRecordHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_PATROL_RECORD, boost::bind(&PassengerFlowMsgHandler::QueryPatrolRecordHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_ALL_PATROL_RECORD, boost::bind(&PassengerFlowMsgHandler::QueryAllPatrolRecordHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::CREATE_STORE_SENSOR, boost::bind(&PassengerFlowMsgHandler::CreateStoreSensorHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::DELETE_STORE_SENSOR, boost::bind(&PassengerFlowMsgHandler::DeleteStoreSensorHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::MODIFY_STORE_SENSOR, boost::bind(&PassengerFlowMsgHandler::ModifyStoreSensorHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_STORE_SENSOR, boost::bind(&PassengerFlowMsgHandler::QueryStoreSensorHandler, &filehdr, _1, _2));
    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_ALL_STORE_SENSOR, boost::bind(&PassengerFlowMsgHandler::QueryAllStoreSensorHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::REPORT_STORE_SENSOR, boost::bind(&PassengerFlowMsgHandler::ReportSensorInfoHandler, &filehdr, _1, _2));

    fcgimgr.SetMsgHandler(PassengerFlowMsgHandler::QUERY_PATROL_RESULT_REPORT, boost::bind(&PassengerFlowMsgHandler::QueryPatrolResultReportHandler, &filehdr, _1, _2));

    fcgimgr.Run(true);
    return 0;
}



