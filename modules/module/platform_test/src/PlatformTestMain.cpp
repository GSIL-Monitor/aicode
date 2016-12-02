#include "ForkChild.h"
#include "NetComm.h"
#include <list>
#include <string>
#include <map>
#include "LogRLD.h"
#include "ConfigSt.h"
#include <boost/filesystem.hpp> 
#include <boost/lexical_cast.hpp>
#include "MsgBusClientFanout.h"
#include "InteractiveProtoHandler.h"
#include "UserTest.h"

#define CONFIG_FILE_NAME "platform_test.ini"
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
    std::string strHost = "PlatformTest(127.0.0.1)";
    std::string strLogPath = "./logs/";
    std::string strLogInnerShowName = "PlatformTest";
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

void Send();
void Receive();

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

    LOG_INFO_RLD("PlatformTest begin runing and daemon status is " << IsNeedDaemonRun);

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

    const std::string &strSyncShakehandTimeoutCount = GetConfig("General.SyncShakehandTimeoutCount");
    if (strSyncShakehandTimeoutCount.empty())
    {
        LOG_ERROR_RLD("SyncShakehandTimeoutCount config item not found.");
        return 0;
    }

    const std::string &strSyncShakehandTimeout = GetConfig("General.SyncShakehandTimeout");
    if (strSyncShakehandTimeout.empty())
    {

        LOG_ERROR_RLD("SyncShakehandTimeout config item not found.");
        return 0;
    }

    const std::string &strSyncAddressRspInvalidTimeout = GetConfig("General.SyncAddressRspInvalidTimeout");
    if (strSyncAddressRspInvalidTimeout.empty())
    {
        LOG_ERROR_RLD("SyncAddressRspInvalidTimeout config item not found.");
        return 0;
    }

    const std::string &strThreadOfWorking = GetConfig("General.ThreadOfWorking");
    if (strThreadOfWorking.empty())
    {
        LOG_ERROR_RLD("ThreadOfWorking config item not found.");
        return 0;
    }

    /************************************************************************/
    /* 测试协议组件序列化和反序列化                                                     */
    /************************************************************************/
    //用户注册请求
    InteractiveProtoHandler::RegisterUserReq_USR RegUsrReq;
    RegUsrReq.m_MsgType = InteractiveProtoHandler::MsgType::RegisterUserReq_USR_T;
    RegUsrReq.m_uiMsgSeq = 1;
    RegUsrReq.m_strSID = "ffffeeee";
    RegUsrReq.m_strValue = "value";
    RegUsrReq.m_userInfo.m_strUserID = "uid_test";
    RegUsrReq.m_userInfo.m_strUserName = "yinbin";
    RegUsrReq.m_userInfo.m_strUserPassword = "testpwd";
    RegUsrReq.m_userInfo.m_uiTypeInfo = 2;
    RegUsrReq.m_userInfo.m_strCreatedate = "2016-11-30";

    for (int j = 0; j < 2; ++j)
    {
        RegUsrReq.m_userInfo.m_strItemsList.push_back(std::move(std::move(boost::lexical_cast<std::string>(j))));

        for (int i = 0; i < 2; ++i)
        {
            InteractiveProtoHandler::Device devInfo;
            devInfo.m_strDevID = "did_test";
            devInfo.m_strDevName = "test_device_name";
            devInfo.m_strDevPassword = "test_dev_pwd";
            devInfo.m_uiTypeInfo = 3;
            devInfo.m_strCreatedate = "2016-11-30";
            devInfo.m_strInnerinfo = "dev_inner_info";
            devInfo.m_strOwnerUserID = "uid_test";
            for (int k = 0; k < 2; ++k)
            {
                devInfo.m_sharingUserIDList.push_back(std::move(boost::lexical_cast<std::string>(k)));
                devInfo.m_sharedUserIDList.push_back(std::move(boost::lexical_cast<std::string>(k)));
                devInfo.m_strItemsList.push_back(std::move(boost::lexical_cast<std::string>(k)));
            }
            
            if (0 == j)
            {
                RegUsrReq.m_userInfo.m_ownerDevInfoList.push_back(std::move(devInfo));                
            }
            else if (1 == j)
            {
                RegUsrReq.m_userInfo.m_sharingDevInfoList.push_back(std::move(devInfo));
            }
            else if (2 == j)
            {
                RegUsrReq.m_userInfo.m_sharedDevInfoList.push_back(std::move(devInfo));
            }
            
        }
    }

    std::string strSerializeOutPut;
    
    InteractiveProtoHandler iphander;
    if (!iphander.SerializeReq(RegUsrReq, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Register user req serialize failed.");
        return 0;
    }

    InteractiveProtoHandler::RegisterUserReq_USR RegUsrReqTmp;
    if (!iphander.UnSerializeReq(strSerializeOutPut, RegUsrReqTmp))
    {
        LOG_ERROR_RLD("Register user req unserialize failed.");
        return 0;
    }
    
    LOG_INFO_RLD("Original m_strValue " << RegUsrReq.m_strValue  << " unserialize m_strValue " << RegUsrReqTmp.m_strValue);
    LOG_INFO_RLD("Original m_MsgType " << RegUsrReq.m_MsgType << " unserialize m_MsgType " << RegUsrReqTmp.m_MsgType);
    LOG_INFO_RLD("Original m_strSID " << RegUsrReq.m_strSID << " unserialize m_strSID " << RegUsrReqTmp.m_strSID);
    LOG_INFO_RLD("Original m_uiMsgSeq " << RegUsrReq.m_uiMsgSeq << " unserialize m_uiMsgSeq " << RegUsrReqTmp.m_uiMsgSeq);
    LOG_INFO_RLD("Original m_userInfo.m_strCreatedate " << RegUsrReq.m_userInfo.m_strCreatedate << 
        " unserialize m_userInfo.m_strCreatedate " << RegUsrReqTmp.m_userInfo.m_strCreatedate);

    bool blTestResult = RegUsrReq.m_strValue == RegUsrReqTmp.m_strValue &&
        RegUsrReq.m_MsgType == RegUsrReqTmp.m_MsgType &&
        RegUsrReq.m_strSID == RegUsrReqTmp.m_strSID &&
        RegUsrReq.m_uiMsgSeq == RegUsrReqTmp.m_uiMsgSeq &&
        RegUsrReq.m_userInfo.m_strCreatedate == RegUsrReqTmp.m_userInfo.m_strCreatedate &&
        RegUsrReq.m_userInfo.m_strUserID == RegUsrReqTmp.m_userInfo.m_strUserID &&
        RegUsrReq.m_userInfo.m_strUserName == RegUsrReqTmp.m_userInfo.m_strUserName &&
        RegUsrReq.m_userInfo.m_strUserPassword == RegUsrReqTmp.m_userInfo.m_strUserPassword &&
        RegUsrReq.m_userInfo.m_uiTypeInfo == RegUsrReqTmp.m_userInfo.m_uiTypeInfo &&
        //
        RegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_strCreatedate == RegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strCreatedate &&
        RegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_strDevID == RegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strDevID &&
        RegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_strDevName == RegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strDevName &&
        RegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_strDevPassword == RegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strDevPassword &&
        RegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_strInnerinfo == RegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strInnerinfo &&
        RegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_strOwnerUserID == RegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strOwnerUserID &&
        RegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_uiTypeInfo == RegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_uiTypeInfo &&
        RegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_sharedUserIDList.front() == RegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_sharedUserIDList.front() &&
        RegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_sharingUserIDList.front() == RegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_sharingUserIDList.front() &&
        RegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_strItemsList.front() == RegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strItemsList.front();

    if (blTestResult)
    {
        LOG_INFO_RLD("Register user req serialize and unserialize compare success.");
    }
    else
    {
        LOG_ERROR_RLD("Register user req serialize and unserialize compare failure.");
    }

    //用户注册响应
    InteractiveProtoHandler::RegisterUserRsp_USR RegUsrRsp;
    RegUsrRsp.m_MsgType = InteractiveProtoHandler::MsgType::RegisterUserRsp_USR_T;
    RegUsrRsp.m_uiMsgSeq = 1;
    RegUsrRsp.m_strSID = "ffffeeee";
    RegUsrRsp.m_iRetcode = 1;
    RegUsrRsp.m_strRetMsg = "test_msg";
    RegUsrRsp.m_strUserID = "test_id";
    RegUsrRsp.m_strValue = "value";

    //std::string strSerializeOutPut;
    //InteractiveProtoHandler iphander;
    if (!iphander.SerializeReq(RegUsrRsp, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Register user rsp serialize failed.");
        return 0;
    }

    InteractiveProtoHandler::RegisterUserRsp_USR RegUsrRspTmp;
    if (!iphander.UnSerializeReq(strSerializeOutPut, RegUsrRspTmp))
    {
        LOG_ERROR_RLD("Register user rsp unserialize failed.");
        return 0;
    }

    blTestResult = RegUsrRsp.m_strUserID == RegUsrRspTmp.m_strUserID &&
        RegUsrRsp.m_strValue == RegUsrRspTmp.m_strValue &&
        RegUsrRsp.m_iRetcode == RegUsrRspTmp.m_iRetcode &&
        RegUsrRsp.m_MsgType == RegUsrRspTmp.m_MsgType &&
        RegUsrRsp.m_strRetMsg == RegUsrRspTmp.m_strRetMsg &&
        RegUsrRsp.m_strSID == RegUsrRspTmp.m_strSID;

    if (blTestResult)
    {
        LOG_INFO_RLD("Register user rsp serialize and unserialize compare success.");
    }
    else
    {
        LOG_ERROR_RLD("Register user rsp serialize and unserialize compare failure.");
    }
    
    //用户注销请求
    InteractiveProtoHandler::UnRegisterUserReq_USR UnRegUsrReq;
    UnRegUsrReq.m_MsgType = InteractiveProtoHandler::MsgType::UnRegisterUserReq_USR_T;
    UnRegUsrReq.m_uiMsgSeq = 1;
    UnRegUsrReq.m_strSID = "ffffeeee";
    UnRegUsrReq.m_strValue = "value";
    UnRegUsrReq.m_userInfo.m_strUserID = "uid_test";
    UnRegUsrReq.m_userInfo.m_strUserName = "yinbin";
    UnRegUsrReq.m_userInfo.m_strUserPassword = "testpwd";
    UnRegUsrReq.m_userInfo.m_uiTypeInfo = 2;
    UnRegUsrReq.m_userInfo.m_strCreatedate = "2016-11-30";

    for (int j = 0; j < 2; ++j)
    {
        UnRegUsrReq.m_userInfo.m_strItemsList.push_back(std::move(std::move(boost::lexical_cast<std::string>(j))));

        for (int i = 0; i < 2; ++i)
        {
            InteractiveProtoHandler::Device devInfo;
            devInfo.m_strDevID = "did_test";
            devInfo.m_strDevName = "test_device_name";
            devInfo.m_strDevPassword = "test_dev_pwd";
            devInfo.m_uiTypeInfo = 3;
            devInfo.m_strCreatedate = "2016-11-30";
            devInfo.m_strInnerinfo = "dev_inner_info";
            devInfo.m_strOwnerUserID = "uid_test";
            for (int k = 0; k < 2; ++k)
            {
                devInfo.m_sharingUserIDList.push_back(std::move(boost::lexical_cast<std::string>(k)));
                devInfo.m_sharedUserIDList.push_back(std::move(boost::lexical_cast<std::string>(k)));
                devInfo.m_strItemsList.push_back(std::move(boost::lexical_cast<std::string>(k)));
            }

            if (0 == j)
            {
                UnRegUsrReq.m_userInfo.m_ownerDevInfoList.push_back(std::move(devInfo));
            }
            else if (1 == j)
            {
                UnRegUsrReq.m_userInfo.m_sharingDevInfoList.push_back(std::move(devInfo));
            }
            else if (2 == j)
            {
                UnRegUsrReq.m_userInfo.m_sharedDevInfoList.push_back(std::move(devInfo));
            }

        }
    }

    if (!iphander.SerializeReq(UnRegUsrReq, strSerializeOutPut))
    {
        LOG_ERROR_RLD("UnRegister user req serialize failed.");
        return 0;
    }

    InteractiveProtoHandler::UnRegisterUserReq_USR UnRegUsrReqTmp;
    if (!iphander.UnSerializeReq(strSerializeOutPut, UnRegUsrReqTmp))
    {
        LOG_ERROR_RLD("UnRegister user req unserialize failed.");
        return 0;
    }

    LOG_INFO_RLD("Original m_strValue " << UnRegUsrReq.m_strValue << " unserialize m_strValue " << UnRegUsrReqTmp.m_strValue);
    LOG_INFO_RLD("Original m_MsgType " << UnRegUsrReq.m_MsgType << " unserialize m_MsgType " << UnRegUsrReqTmp.m_MsgType);
    LOG_INFO_RLD("Original m_strSID " << UnRegUsrReq.m_strSID << " unserialize m_strSID " << UnRegUsrReqTmp.m_strSID);
    LOG_INFO_RLD("Original m_uiMsgSeq " << UnRegUsrReq.m_uiMsgSeq << " unserialize m_uiMsgSeq " << UnRegUsrReqTmp.m_uiMsgSeq);
    LOG_INFO_RLD("Original m_userInfo.m_strCreatedate " << UnRegUsrReq.m_userInfo.m_strCreatedate <<
        " unserialize m_userInfo.m_strCreatedate " << UnRegUsrReqTmp.m_userInfo.m_strCreatedate);

    blTestResult = UnRegUsrReq.m_strValue == UnRegUsrReqTmp.m_strValue &&
        UnRegUsrReq.m_MsgType == UnRegUsrReqTmp.m_MsgType &&
        UnRegUsrReq.m_strSID == UnRegUsrReqTmp.m_strSID &&
        UnRegUsrReq.m_uiMsgSeq == UnRegUsrReqTmp.m_uiMsgSeq &&
        UnRegUsrReq.m_userInfo.m_strCreatedate == UnRegUsrReqTmp.m_userInfo.m_strCreatedate &&
        UnRegUsrReq.m_userInfo.m_strUserID == UnRegUsrReqTmp.m_userInfo.m_strUserID &&
        UnRegUsrReq.m_userInfo.m_strUserName == UnRegUsrReqTmp.m_userInfo.m_strUserName &&
        UnRegUsrReq.m_userInfo.m_strUserPassword == UnRegUsrReqTmp.m_userInfo.m_strUserPassword &&
        UnRegUsrReq.m_userInfo.m_uiTypeInfo == UnRegUsrReqTmp.m_userInfo.m_uiTypeInfo &&
        //
        UnRegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_strCreatedate == UnRegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strCreatedate &&
        UnRegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_strDevID == UnRegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strDevID &&
        UnRegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_strDevName == UnRegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strDevName &&
        UnRegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_strDevPassword == UnRegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strDevPassword &&
        UnRegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_strInnerinfo == UnRegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strInnerinfo &&
        UnRegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_strOwnerUserID == UnRegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strOwnerUserID &&
        UnRegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_uiTypeInfo == UnRegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_uiTypeInfo &&
        UnRegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_sharedUserIDList.front() == UnRegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_sharedUserIDList.front() &&
        UnRegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_sharingUserIDList.front() == UnRegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_sharingUserIDList.front() &&
        UnRegUsrReq.m_userInfo.m_ownerDevInfoList.front().m_strItemsList.front() == UnRegUsrReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strItemsList.front();

    if (blTestResult)
    {
        LOG_INFO_RLD("UnRegister user req serialize and unserialize compare success.");
    }
    else
    {
        LOG_ERROR_RLD("UnRegister user req serialize and unserialize compare failure.");
    }

    //用户注销响应
    InteractiveProtoHandler::UnRegisterUserRsp_USR UnRegUsrRsp;
    UnRegUsrRsp.m_MsgType = InteractiveProtoHandler::MsgType::UnRegisterUserRsp_USR_T;
    UnRegUsrRsp.m_uiMsgSeq = 1;
    UnRegUsrRsp.m_strSID = "ffffeeee";
    UnRegUsrRsp.m_iRetcode = 1;
    UnRegUsrRsp.m_strRetMsg = "test_msg";
    UnRegUsrRsp.m_strUserID = "test_id";
    UnRegUsrRsp.m_strValue = "value";

    //std::string strSerializeOutPut;
    //InteractiveProtoHandler iphander;
    if (!iphander.SerializeReq(UnRegUsrRsp, strSerializeOutPut))
    {
        LOG_ERROR_RLD("UnRegister user rsp serialize failed.");
        return 0;
    }

    InteractiveProtoHandler::UnRegisterUserRsp_USR UnRegUsrRspTmp;
    if (!iphander.UnSerializeReq(strSerializeOutPut, UnRegUsrRspTmp))
    {
        LOG_ERROR_RLD("UnRegister user rsp unserialize failed.");
        return 0;
    }

    blTestResult = UnRegUsrRsp.m_strUserID == UnRegUsrRspTmp.m_strUserID &&
        UnRegUsrRsp.m_strValue == UnRegUsrRspTmp.m_strValue &&
        UnRegUsrRsp.m_iRetcode == UnRegUsrRspTmp.m_iRetcode &&
        UnRegUsrRsp.m_MsgType == UnRegUsrRspTmp.m_MsgType &&
        UnRegUsrRsp.m_strRetMsg == UnRegUsrRspTmp.m_strRetMsg &&
        UnRegUsrRsp.m_strSID == UnRegUsrRspTmp.m_strSID;

    if (blTestResult)
    {
        LOG_INFO_RLD("UnRegister user rsp serialize and unserialize compare success.");
    }
    else
    {
        LOG_ERROR_RLD("UnRegister user rsp serialize and unserialize compare failure.");
    }

    //用户登录请求
    InteractiveProtoHandler::LoginReq_USR LoginReq;
    LoginReq.m_MsgType = InteractiveProtoHandler::MsgType::LoginReq_USR_T;
    LoginReq.m_uiMsgSeq = 1;
    LoginReq.m_strSID = "ffffeeee";
    LoginReq.m_strValue = "value";
    LoginReq.m_userInfo.m_strUserID = "uid_test";
    LoginReq.m_userInfo.m_strUserName = "yinbin";
    LoginReq.m_userInfo.m_strUserPassword = "testpwd";
    LoginReq.m_userInfo.m_uiTypeInfo = 2;
    LoginReq.m_userInfo.m_strCreatedate = "2016-11-30";

    for (int j = 0; j < 2; ++j)
    {
        LoginReq.m_userInfo.m_strItemsList.push_back(std::move(std::move(boost::lexical_cast<std::string>(j))));

        for (int i = 0; i < 2; ++i)
        {
            InteractiveProtoHandler::Device devInfo;
            devInfo.m_strDevID = "did_test";
            devInfo.m_strDevName = "test_device_name";
            devInfo.m_strDevPassword = "test_dev_pwd";
            devInfo.m_uiTypeInfo = 3;
            devInfo.m_strCreatedate = "2016-11-30";
            devInfo.m_strInnerinfo = "dev_inner_info";
            devInfo.m_strOwnerUserID = "uid_test";
            for (int k = 0; k < 2; ++k)
            {
                devInfo.m_sharingUserIDList.push_back(std::move(boost::lexical_cast<std::string>(k)));
                devInfo.m_sharedUserIDList.push_back(std::move(boost::lexical_cast<std::string>(k)));
                devInfo.m_strItemsList.push_back(std::move(boost::lexical_cast<std::string>(k)));
            }

            if (0 == j)
            {
                LoginReq.m_userInfo.m_ownerDevInfoList.push_back(std::move(devInfo));
            }
            else if (1 == j)
            {
                LoginReq.m_userInfo.m_sharingDevInfoList.push_back(std::move(devInfo));
            }
            else if (2 == j)
            {
                LoginReq.m_userInfo.m_sharedDevInfoList.push_back(std::move(devInfo));
            }

        }
    }

    if (!iphander.SerializeReq(LoginReq, strSerializeOutPut))
    {
        LOG_ERROR_RLD("LoginReq user req serialize failed.");
        return 0;
    }

    InteractiveProtoHandler::LoginReq_USR LoginReqTmp;
    if (!iphander.UnSerializeReq(strSerializeOutPut, LoginReqTmp))
    {
        LOG_ERROR_RLD("LoginReq user req unserialize failed.");
        return 0;
    }

    LOG_INFO_RLD("Original m_strValue " << LoginReq.m_strValue << " unserialize m_strValue " << LoginReqTmp.m_strValue);
    LOG_INFO_RLD("Original m_MsgType " << LoginReq.m_MsgType << " unserialize m_MsgType " << LoginReqTmp.m_MsgType);
    LOG_INFO_RLD("Original m_strSID " << LoginReq.m_strSID << " unserialize m_strSID " << LoginReqTmp.m_strSID);
    LOG_INFO_RLD("Original m_uiMsgSeq " << LoginReq.m_uiMsgSeq << " unserialize m_uiMsgSeq " << LoginReqTmp.m_uiMsgSeq);
    LOG_INFO_RLD("Original m_userInfo.m_strCreatedate " << LoginReq.m_userInfo.m_strCreatedate <<
        " unserialize m_userInfo.m_strCreatedate " << LoginReqTmp.m_userInfo.m_strCreatedate);

    blTestResult = LoginReq.m_strValue == LoginReqTmp.m_strValue &&
        LoginReq.m_MsgType == LoginReqTmp.m_MsgType &&
        LoginReq.m_strSID == LoginReqTmp.m_strSID &&
        LoginReq.m_uiMsgSeq == LoginReqTmp.m_uiMsgSeq &&
        LoginReq.m_userInfo.m_strCreatedate == LoginReqTmp.m_userInfo.m_strCreatedate &&
        LoginReq.m_userInfo.m_strUserID == LoginReqTmp.m_userInfo.m_strUserID &&
        LoginReq.m_userInfo.m_strUserName == LoginReqTmp.m_userInfo.m_strUserName &&
        LoginReq.m_userInfo.m_strUserPassword == LoginReqTmp.m_userInfo.m_strUserPassword &&
        LoginReq.m_userInfo.m_uiTypeInfo == LoginReqTmp.m_userInfo.m_uiTypeInfo &&
        //
        LoginReq.m_userInfo.m_ownerDevInfoList.front().m_strCreatedate == LoginReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strCreatedate &&
        LoginReq.m_userInfo.m_ownerDevInfoList.front().m_strDevID == LoginReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strDevID &&
        LoginReq.m_userInfo.m_ownerDevInfoList.front().m_strDevName == LoginReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strDevName &&
        LoginReq.m_userInfo.m_ownerDevInfoList.front().m_strDevPassword == LoginReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strDevPassword &&
        LoginReq.m_userInfo.m_ownerDevInfoList.front().m_strInnerinfo == LoginReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strInnerinfo &&
        LoginReq.m_userInfo.m_ownerDevInfoList.front().m_strOwnerUserID == LoginReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strOwnerUserID &&
        LoginReq.m_userInfo.m_ownerDevInfoList.front().m_uiTypeInfo == LoginReqTmp.m_userInfo.m_ownerDevInfoList.front().m_uiTypeInfo &&
        LoginReq.m_userInfo.m_ownerDevInfoList.front().m_sharedUserIDList.front() == LoginReqTmp.m_userInfo.m_ownerDevInfoList.front().m_sharedUserIDList.front() &&
        LoginReq.m_userInfo.m_ownerDevInfoList.front().m_sharingUserIDList.front() == LoginReqTmp.m_userInfo.m_ownerDevInfoList.front().m_sharingUserIDList.front() &&
        LoginReq.m_userInfo.m_ownerDevInfoList.front().m_strItemsList.front() == LoginReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strItemsList.front();

    if (blTestResult)
    {
        LOG_INFO_RLD("Login user req serialize and unserialize compare success.");
    }
    else
    {
        LOG_ERROR_RLD("Login user req serialize and unserialize compare failure.");
    }
    
    //用户登录响应
    InteractiveProtoHandler::LoginRsp_USR LoginRsp;
    LoginRsp.m_MsgType = InteractiveProtoHandler::MsgType::LoginRsp_USR_T;
    LoginRsp.m_uiMsgSeq = 1;
    LoginRsp.m_strSID = "ffffeeee";
    LoginRsp.m_iRetcode = 1;
    LoginRsp.m_strRetMsg = "test_msg";
    LoginRsp.m_strValue = "value";

    for (int i = 0; i < 2; ++i)
    {
        InteractiveProtoHandler::Device devInfo;
        devInfo.m_strDevID = "did_test";
        devInfo.m_strDevName = "test_device_name";
        devInfo.m_strDevPassword = "test_dev_pwd";
        devInfo.m_uiTypeInfo = 3;
        devInfo.m_strCreatedate = "2016-11-30";
        devInfo.m_strInnerinfo = "dev_inner_info";
        devInfo.m_strOwnerUserID = "uid_test";
        for (int k = 0; k < 2; ++k)
        {
            devInfo.m_sharingUserIDList.push_back(std::move(boost::lexical_cast<std::string>(k)));
            devInfo.m_sharedUserIDList.push_back(std::move(boost::lexical_cast<std::string>(k)));
            devInfo.m_strItemsList.push_back(std::move(boost::lexical_cast<std::string>(k)));
        }

        LoginRsp.m_devInfoList.push_back(std::move(devInfo));      
    }


    if (!iphander.SerializeReq(LoginRsp, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Login user rsp serialize failed.");
        return 0;
    }

    InteractiveProtoHandler::LoginRsp_USR LoginRspTmp;
    if (!iphander.UnSerializeReq(strSerializeOutPut, LoginRspTmp))
    {
        LOG_ERROR_RLD("Login user rsp unserialize failed.");
        return 0;
    }

    LOG_INFO_RLD("Original m_strValue " << LoginRsp.m_strValue << " unserialize m_strValue " << LoginRspTmp.m_strValue);
    LOG_INFO_RLD("Original m_MsgType " << LoginRsp.m_MsgType << " unserialize m_MsgType " << LoginRspTmp.m_MsgType);
    LOG_INFO_RLD("Original m_strSID " << LoginRsp.m_strSID << " unserialize m_strSID " << LoginRspTmp.m_strSID);
    LOG_INFO_RLD("Original m_uiMsgSeq " << LoginRsp.m_uiMsgSeq << " unserialize m_uiMsgSeq " << LoginRspTmp.m_uiMsgSeq);
    LOG_INFO_RLD("Original m_strRetMsg " << LoginRsp.m_strRetMsg << " unserialize m_strRetMsg " << LoginRsp.m_strRetMsg);
    LOG_INFO_RLD("Original m_devInfoList.front().m_strCreatedate " << LoginRsp.m_devInfoList.front().m_strCreatedate <<
        " unserialize m_devInfoList.front().m_strCreatedate " << LoginRspTmp.m_devInfoList.front().m_strCreatedate);
    LOG_INFO_RLD("Original m_devInfoList.front().m_strDevID " << LoginRsp.m_devInfoList.front().m_strDevID <<
        " unserialize m_devInfoList.front().m_strDevID " << LoginRspTmp.m_devInfoList.front().m_strDevID);
    LOG_INFO_RLD("Original m_devInfoList.front().m_strDevName " << LoginRsp.m_devInfoList.front().m_strDevName <<
        " unserialize m_devInfoList.front().m_strDevName " << LoginRspTmp.m_devInfoList.front().m_strDevName);
    LOG_INFO_RLD("Original m_devInfoList.front().m_strDevPassword " << LoginRsp.m_devInfoList.front().m_strDevPassword <<
        " unserialize m_devInfoList.front().m_strDevPassword " << LoginRspTmp.m_devInfoList.front().m_strDevPassword);
    LOG_INFO_RLD("Original m_devInfoList.front().m_strInnerinfo " << LoginRsp.m_devInfoList.front().m_strInnerinfo <<
        " unserialize m_devInfoList.front().m_strInnerinfo " << LoginRspTmp.m_devInfoList.front().m_strInnerinfo);
    LOG_INFO_RLD("Original m_devInfoList.front().m_strOwnerUserID " << LoginRsp.m_devInfoList.front().m_strOwnerUserID <<
        " unserialize m_devInfoList.front().m_strOwnerUserID " << LoginRspTmp.m_devInfoList.front().m_strOwnerUserID);
    LOG_INFO_RLD("Original m_devInfoList.front().m_uiTypeInfo " << LoginRsp.m_devInfoList.front().m_uiTypeInfo <<
        " unserialize m_devInfoList.front().m_uiTypeInfo " << LoginRspTmp.m_devInfoList.front().m_uiTypeInfo);
    LOG_INFO_RLD("Original m_devInfoList.front().m_sharedUserIDList.front() " << LoginRsp.m_devInfoList.front().m_sharedUserIDList.front() <<
        " unserialize m_devInfoList.front().m_sharedUserIDList.front() " << LoginRspTmp.m_devInfoList.front().m_sharedUserIDList.front());
    LOG_INFO_RLD("Original m_devInfoList.front().m_sharingUserIDList.front() " << LoginRsp.m_devInfoList.front().m_sharingUserIDList.front() <<
        " unserialize m_devInfoList.front().m_sharingUserIDList.front() " << LoginRspTmp.m_devInfoList.front().m_sharingUserIDList.front());
    LOG_INFO_RLD("Original m_devInfoList.front().m_strItemsList.front() " << LoginRsp.m_devInfoList.front().m_strItemsList.front() <<
        " unserialize m_devInfoList.front().m_strItemsList.front() " << LoginRspTmp.m_devInfoList.front().m_strItemsList.front());

    blTestResult = LoginRsp.m_strValue == LoginRspTmp.m_strValue &&
        LoginRsp.m_iRetcode == LoginRspTmp.m_iRetcode &&
        LoginRsp.m_MsgType == LoginRspTmp.m_MsgType &&
        LoginRsp.m_strRetMsg == LoginRspTmp.m_strRetMsg &&
        LoginRsp.m_strSID == LoginRspTmp.m_strSID &&

        //
        LoginRsp.m_devInfoList.front().m_strCreatedate == LoginRspTmp.m_devInfoList.front().m_strCreatedate &&
        LoginRsp.m_devInfoList.front().m_strDevID == LoginRspTmp.m_devInfoList.front().m_strDevID &&
        LoginRsp.m_devInfoList.front().m_strDevName == LoginRspTmp.m_devInfoList.front().m_strDevName &&
        LoginRsp.m_devInfoList.front().m_strDevPassword == LoginRspTmp.m_devInfoList.front().m_strDevPassword &&
        LoginRsp.m_devInfoList.front().m_strInnerinfo == LoginRspTmp.m_devInfoList.front().m_strInnerinfo &&
        LoginRsp.m_devInfoList.front().m_strOwnerUserID == LoginRspTmp.m_devInfoList.front().m_strOwnerUserID &&
        LoginRsp.m_devInfoList.front().m_uiTypeInfo == LoginRspTmp.m_devInfoList.front().m_uiTypeInfo &&
        LoginRsp.m_devInfoList.front().m_sharedUserIDList.front() == LoginRspTmp.m_devInfoList.front().m_sharedUserIDList.front() &&
        LoginRsp.m_devInfoList.front().m_sharingUserIDList.front() == LoginRspTmp.m_devInfoList.front().m_sharingUserIDList.front() &&
        LoginRsp.m_devInfoList.front().m_strItemsList.front() == LoginRspTmp.m_devInfoList.front().m_strItemsList.front();
       
    if (blTestResult)
    {
        LOG_INFO_RLD("Login user rsp serialize and unserialize compare success.");
    }
    else
    {
        LOG_ERROR_RLD("Login user rsp serialize and unserialize compare failure.");
    }

    //用户登出请求
    InteractiveProtoHandler::LogoutReq_USR LogoutReq;
    LogoutReq.m_MsgType = InteractiveProtoHandler::MsgType::LogoutReq_USR_T;
    LogoutReq.m_uiMsgSeq = 1;
    LogoutReq.m_strSID = "ffffeeee";
    LogoutReq.m_strValue = "value";
    LogoutReq.m_userInfo.m_strUserID = "uid_test";
    LogoutReq.m_userInfo.m_strUserName = "yinbin";
    LogoutReq.m_userInfo.m_strUserPassword = "testpwd";
    LogoutReq.m_userInfo.m_uiTypeInfo = 2;
    LogoutReq.m_userInfo.m_strCreatedate = "2016-11-30";

    for (int j = 0; j < 2; ++j)
    {
        LogoutReq.m_userInfo.m_strItemsList.push_back(std::move(std::move(boost::lexical_cast<std::string>(j))));

        for (int i = 0; i < 2; ++i)
        {
            InteractiveProtoHandler::Device devInfo;
            devInfo.m_strDevID = "did_test";
            devInfo.m_strDevName = "test_device_name";
            devInfo.m_strDevPassword = "test_dev_pwd";
            devInfo.m_uiTypeInfo = 3;
            devInfo.m_strCreatedate = "2016-11-30";
            devInfo.m_strInnerinfo = "dev_inner_info";
            devInfo.m_strOwnerUserID = "uid_test";
            for (int k = 0; k < 2; ++k)
            {
                devInfo.m_sharingUserIDList.push_back(std::move(boost::lexical_cast<std::string>(k)));
                devInfo.m_sharedUserIDList.push_back(std::move(boost::lexical_cast<std::string>(k)));
                devInfo.m_strItemsList.push_back(std::move(boost::lexical_cast<std::string>(k)));
            }

            if (0 == j)
            {
                LogoutReq.m_userInfo.m_ownerDevInfoList.push_back(std::move(devInfo));
            }
            else if (1 == j)
            {
                LogoutReq.m_userInfo.m_sharingDevInfoList.push_back(std::move(devInfo));
            }
            else if (2 == j)
            {
                LogoutReq.m_userInfo.m_sharedDevInfoList.push_back(std::move(devInfo));
            }

        }
    }

    if (!iphander.SerializeReq(LogoutReq, strSerializeOutPut))
    {
        LOG_ERROR_RLD("LogoutReq user req serialize failed.");
        return 0;
    }

    InteractiveProtoHandler::LogoutReq_USR LogoutReqTmp;
    if (!iphander.UnSerializeReq(strSerializeOutPut, LogoutReqTmp))
    {
        LOG_ERROR_RLD("LogoutReq user req unserialize failed.");
        return 0;
    }

    LOG_INFO_RLD("Original m_strValue " << LogoutReq.m_strValue << " unserialize m_strValue " << LogoutReqTmp.m_strValue);
    LOG_INFO_RLD("Original m_MsgType " << LogoutReq.m_MsgType << " unserialize m_MsgType " << LogoutReqTmp.m_MsgType);
    LOG_INFO_RLD("Original m_strSID " << LogoutReq.m_strSID << " unserialize m_strSID " << LogoutReqTmp.m_strSID);
    LOG_INFO_RLD("Original m_uiMsgSeq " << LogoutReq.m_uiMsgSeq << " unserialize m_uiMsgSeq " << LogoutReqTmp.m_uiMsgSeq);
    LOG_INFO_RLD("Original m_userInfo.m_strCreatedate " << LogoutReq.m_userInfo.m_strCreatedate <<
        " unserialize m_userInfo.m_strCreatedate " << LogoutReqTmp.m_userInfo.m_strCreatedate);

    blTestResult = LogoutReq.m_strValue == LogoutReqTmp.m_strValue &&
        LogoutReq.m_MsgType == LogoutReqTmp.m_MsgType &&
        LogoutReq.m_strSID == LogoutReqTmp.m_strSID &&
        LogoutReq.m_uiMsgSeq == LogoutReqTmp.m_uiMsgSeq &&
        LogoutReq.m_userInfo.m_strCreatedate == LogoutReqTmp.m_userInfo.m_strCreatedate &&
        LogoutReq.m_userInfo.m_strUserID == LogoutReqTmp.m_userInfo.m_strUserID &&
        LogoutReq.m_userInfo.m_strUserName == LogoutReqTmp.m_userInfo.m_strUserName &&
        LogoutReq.m_userInfo.m_strUserPassword == LogoutReqTmp.m_userInfo.m_strUserPassword &&
        LogoutReq.m_userInfo.m_uiTypeInfo == LogoutReqTmp.m_userInfo.m_uiTypeInfo &&
        //
        LogoutReq.m_userInfo.m_ownerDevInfoList.front().m_strCreatedate == LogoutReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strCreatedate &&
        LogoutReq.m_userInfo.m_ownerDevInfoList.front().m_strDevID == LogoutReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strDevID &&
        LogoutReq.m_userInfo.m_ownerDevInfoList.front().m_strDevName == LogoutReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strDevName &&
        LogoutReq.m_userInfo.m_ownerDevInfoList.front().m_strDevPassword == LogoutReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strDevPassword &&
        LogoutReq.m_userInfo.m_ownerDevInfoList.front().m_strInnerinfo == LogoutReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strInnerinfo &&
        LogoutReq.m_userInfo.m_ownerDevInfoList.front().m_strOwnerUserID == LogoutReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strOwnerUserID &&
        LogoutReq.m_userInfo.m_ownerDevInfoList.front().m_uiTypeInfo == LogoutReqTmp.m_userInfo.m_ownerDevInfoList.front().m_uiTypeInfo &&
        LogoutReq.m_userInfo.m_ownerDevInfoList.front().m_sharedUserIDList.front() == LogoutReqTmp.m_userInfo.m_ownerDevInfoList.front().m_sharedUserIDList.front() &&
        LogoutReq.m_userInfo.m_ownerDevInfoList.front().m_sharingUserIDList.front() == LogoutReqTmp.m_userInfo.m_ownerDevInfoList.front().m_sharingUserIDList.front() &&
        LogoutReq.m_userInfo.m_ownerDevInfoList.front().m_strItemsList.front() == LogoutReqTmp.m_userInfo.m_ownerDevInfoList.front().m_strItemsList.front();

    if (blTestResult)
    {
        LOG_INFO_RLD("Logout user req serialize and unserialize compare success.");
    }
    else
    {
        LOG_ERROR_RLD("Logout user req serialize and unserialize compare failure.");
    }

    //用户登出响应
    InteractiveProtoHandler::LogoutRsp_USR LogoutRsp;
    LogoutRsp.m_MsgType = InteractiveProtoHandler::MsgType::LogoutRsp_USR_T;
    LogoutRsp.m_uiMsgSeq = 1;
    LogoutRsp.m_strSID = "ffffeeee";
    LogoutRsp.m_iRetcode = 1;
    LogoutRsp.m_strRetMsg = "test_msg";
    LogoutRsp.m_strValue = "value";

    //std::string strSerializeOutPut;
    //InteractiveProtoHandler iphander;
    if (!iphander.SerializeReq(LogoutRsp, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Logout user rsp serialize failed.");
        return 0;
    }

    InteractiveProtoHandler::LogoutRsp_USR LogoutRspTmp;
    if (!iphander.UnSerializeReq(strSerializeOutPut, LogoutRspTmp))
    {
        LOG_ERROR_RLD("Logout user rsp unserialize failed.");
        return 0;
    }

    blTestResult = LogoutRsp.m_strValue == LogoutRspTmp.m_strValue &&
        LogoutRsp.m_iRetcode == LogoutRspTmp.m_iRetcode &&
        LogoutRsp.m_MsgType == LogoutRspTmp.m_MsgType &&
        LogoutRsp.m_strRetMsg == LogoutRspTmp.m_strRetMsg &&
        LogoutRsp.m_strSID == LogoutRspTmp.m_strSID;

    if (blTestResult)
    {
        LOG_INFO_RLD("Logout user rsp serialize and unserialize compare success.");
    }
    else
    {
        LOG_ERROR_RLD("Logout user rsp serialize and unserialize compare failure.");
    }

    //握手请求
    InteractiveProtoHandler::ShakehandReq_USR ShakehandReq;
    ShakehandReq.m_MsgType = InteractiveProtoHandler::MsgType::ShakehandReq_USR_T;
    ShakehandReq.m_uiMsgSeq = 1;
    ShakehandReq.m_strSID = "ffffeeee";
    ShakehandReq.m_strValue = "value";
    ShakehandReq.m_strUserID = "test_userid";

    if (!iphander.SerializeReq(ShakehandReq, strSerializeOutPut))
    {
        LOG_ERROR_RLD("ShakehandReq user req serialize failed.");
        return 0;
    }

    InteractiveProtoHandler::ShakehandReq_USR ShakehandReqTmp;
    if (!iphander.UnSerializeReq(strSerializeOutPut, ShakehandReqTmp))
    {
        LOG_ERROR_RLD("ShakehandReq user req unserialize failed.");
        return 0;
    }

    blTestResult = ShakehandReq.m_strUserID == ShakehandReqTmp.m_strUserID &&
        ShakehandReq.m_strValue == ShakehandReqTmp.m_strValue &&
        ShakehandReq.m_strUserID == ShakehandReqTmp.m_strUserID &&
        ShakehandReq.m_MsgType == ShakehandReqTmp.m_MsgType &&
        ShakehandReq.m_strSID == ShakehandReqTmp.m_strSID;

    if (blTestResult)
    {
        LOG_INFO_RLD("ShakehandReq user req serialize and unserialize compare success.");
    }
    else
    {
        LOG_ERROR_RLD("ShakehandReq user req serialize and unserialize compare failure.");
    }

    //握手响应
    InteractiveProtoHandler::ShakehandRsp_USR ShakehandRsp;
    ShakehandRsp.m_MsgType = InteractiveProtoHandler::MsgType::ShakehandRsp_USR_T;
    ShakehandRsp.m_uiMsgSeq = 1;
    ShakehandRsp.m_iRetcode = 3;
    ShakehandRsp.m_strRetMsg = "test_msg";
    ShakehandRsp.m_strSID = "ffffeeee";
    ShakehandRsp.m_strValue = "value";

    if (!iphander.SerializeReq(ShakehandRsp, strSerializeOutPut))
    {
        LOG_ERROR_RLD("ShakehandRsp user req serialize failed.");
        return 0;
    }

    InteractiveProtoHandler::ShakehandRsp_USR ShakehandRspTmp;
    if (!iphander.UnSerializeReq(strSerializeOutPut, ShakehandRspTmp))
    {
        LOG_ERROR_RLD("ShakehandRsp user req unserialize failed.");
        return 0;
    }

    blTestResult = ShakehandRsp.m_iRetcode == ShakehandRspTmp.m_iRetcode &&
        ShakehandRsp.m_strRetMsg == ShakehandRspTmp.m_strRetMsg &&
        ShakehandRsp.m_strValue == ShakehandRspTmp.m_strValue &&
        ShakehandRsp.m_MsgType == ShakehandRspTmp.m_MsgType &&
        ShakehandRsp.m_strSID == ShakehandRspTmp.m_strSID;

    if (blTestResult)
    {
        LOG_INFO_RLD("ShakehandRsp user rsp serialize and unserialize compare success.");
    }
    else
    {
        LOG_ERROR_RLD("ShakehandRsp user rsp serialize and unserialize compare failure.");
    }
        
    ////test code
    //new boost::thread(Send);
    //boost::this_thread::sleep(boost::posix_time::seconds(2));
    //new boost::thread(Receive);
    //new boost::thread(Receive);
    //boost::this_thread::sleep(boost::posix_time::seconds(1000000));
    // 

    {
        boost::shared_ptr<UserTest> pCObj(new UserTest);
        pCObj->init(strRemoteAddress.c_str(), strRemotePort.c_str(), 1);
        boost::this_thread::sleep(boost::posix_time::seconds(30));
        pCObj->Close();
    }

    printf("wait 5 seconds...\n");
    boost::this_thread::sleep(boost::posix_time::seconds(5));
    printf("exit.\n");

    return 0;
}

void Send()
{
    MsgBusClientFanout msg("exange_name_test", "172.20.122.250", 5672, "admin", "admin");
    msg.Init();

    for (int i = 0; i < 300000; ++i)
    {
        msg.BroadcastMsg("", "Msg by test");
        boost::this_thread::sleep(boost::posix_time::seconds(1));
        LOG_INFO_RLD("send..." << i);
    }

    boost::this_thread::sleep(boost::posix_time::seconds(1000000));

}

void Receive()
{
    MsgBusClientFanout msg("exange_name_test", "172.20.122.250", 5672, "admin", "admin");
    msg.Init();

    for (int i = 0; i < 300000; ++i)
    {
        std::string strReceiveMsg;
        msg.ListenMsg("", strReceiveMsg);
        LOG_INFO_RLD("receive...");
    }

    boost::this_thread::sleep(boost::posix_time::seconds(1000000));
}