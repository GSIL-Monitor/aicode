#include "ForkChild.h"
#include "NetComm.h"
#include <list>
#include <string>
#include <map>
#include "LogRLD.h"
#include "ConfigSt.h"
#include <boost/filesystem.hpp> 
#include <boost/lexical_cast.hpp>
#include "ProtoHandler.h"
#include "ControlCenter.h"
#include "MsgBusClientFanout.h"

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
    std::string strHost = "TDFS_CENTER(127.0.0.1)";
    std::string strLogPath = "./logs/";
    std::string strLogInnerShowName = "TDFS_CENTER";
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

    LOG_INFO_RLD("TDFS_Center begin runing and daemon status is " << IsNeedDaemonRun);

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

    ProtoHandler::LoginReq loginreq;
    loginreq.m_MsgType = ProtoHandler::MsgType::LoginReq_T;
    loginreq.m_uiMsgSeq = 1;
    loginreq.m_strSID = "sssssseeee";
    loginreq.m_strPassword = "pwd";
    loginreq.m_strSyncServiceName = "TestName";

    std::string strOutput;
    ProtoHandler phr;
    if (!phr.SerializeReq(loginreq, strOutput))
    {
        LOG_ERROR_RLD("Serialize login req failed.");
        return 0;
    }
    ProtoHandler::LoginReq loginreq2;
    if (!phr.UnSerializeReq(strOutput, loginreq2))
    {
        LOG_ERROR_RLD("Unserialize login req failed.");
        return 0;
    }

    bool blResult = loginreq.m_strPassword == loginreq2.m_strPassword && loginreq.m_strSyncServiceName == loginreq2.m_strSyncServiceName
        && loginreq.m_strSID == loginreq2.m_strSID && loginreq.m_uiMsgSeq && loginreq2.m_uiMsgSeq;

    LOG_INFO_RLD("Test LoginReq serialize and unserialize result is " << (blResult ? "true" : "false"));

    {
        {
            std::string strCrc = "21212";
            std::string strContentEncoded = "bodybody=";
            const char *pContentBufferEncoded = strContentEncoded.data();
            const unsigned int uiContentBufferLenEncoded = strContentEncoded.size();
            std::string strPartEndLen = "30";
            std::string strSrcID = "111";
            std::string strDstID = "222";
            std::string strType = "1";
            unsigned int uiAllLen = 200;
            char *pAllBuffer = new char[uiAllLen + 1];
            memset(pAllBuffer, 0, (uiAllLen + 1));
            snprintf(pAllBuffer, uiAllLen + 1, "RG,%s,%s,%s,%s,", strPartEndLen.c_str(), strSrcID.c_str(), strDstID.c_str(), strType.c_str());

            unsigned int uiPos1 = 7 + strPartEndLen.size() + strSrcID.size() + strDstID.size() + strType.size();
            memcpy(pAllBuffer + uiPos1, pContentBufferEncoded, uiContentBufferLenEncoded);
            std::string strTmp(",");
            memcpy(pAllBuffer + uiPos1 + uiContentBufferLenEncoded, strTmp.data(), strTmp.size());
            memcpy(pAllBuffer + uiPos1 + uiContentBufferLenEncoded + 1, strCrc.data(), strCrc.size());


            LOG_INFO_RLD("Test msg is " << std::string(pAllBuffer, uiAllLen + 1));
        }

        {
            ProtoHandler::SyncFileListPendingRsp syncfilelistrsp;
            syncfilelistrsp.m_iRetcode = 0;
            syncfilelistrsp.m_strRetMsg = "ok";
            syncfilelistrsp.m_uiMsgSeq = 1;
            syncfilelistrsp.m_strSID = "sslkkla";
            syncfilelistrsp.m_MsgType = ProtoHandler::MsgType::SyncFileListPendingRsp_T;

            std::string strOutputSync;
            ProtoHandler phd;
            if (!phd.SerializeRsp(syncfilelistrsp, strOutputSync))
            {
                LOG_ERROR_RLD("Serialize SyncFileListPendingRsp req failed.");
                return 0;
            }

            ProtoHandler::SyncFileListPendingRsp syncfilelistrsp2;
            if (!phd.UnSerializeRsp(strOutputSync, syncfilelistrsp2))
            {
                LOG_ERROR_RLD("UnSerialize SyncFileListPendingRsp req failed.");
                return 0;
            }

            bool blRet = syncfilelistrsp.m_iRetcode == syncfilelistrsp2.m_iRetcode && syncfilelistrsp.m_strRetMsg == syncfilelistrsp2.m_strRetMsg &&
                syncfilelistrsp.m_uiMsgSeq == syncfilelistrsp2.m_uiMsgSeq && syncfilelistrsp.m_strSID == syncfilelistrsp2.m_strSID && 
                syncfilelistrsp.m_MsgType == syncfilelistrsp2.m_MsgType;
            LOG_INFO_RLD("Test SyncFileListPendingRsp serialize and unserialize result is " << (blRet ? "true" : "false"));

        }

        ProtoHandler::SyncFileListPendingReq syncfilelistreq;
        syncfilelistreq.m_MsgType = ProtoHandler::MsgType::SyncFileListPendingReq_T;
        syncfilelistreq.m_uiMsgSeq = 22;
        syncfilelistreq.m_strSID = "siddd";

        ProtoHandler::FileInfo ff;
        ff.strCreatedate = "xxx";
        ff.strFileID = "ssaa";
        ff.strFileMD5 = "ewwa";
        ff.strFileName = "name";
        ff.uiFileSize = 10;
        syncfilelistreq.m_FileInfoList.push_back(ff);

        ff.strCreatedate = "djkajk";
        ff.strFileID = "ewq";
        ff.strFileMD5 = "fsfgga";
        ff.strFileName = "e4lkak";
        ff.uiFileSize = 30;
        syncfilelistreq.m_FileInfoList.push_back(ff);

        std::string strOutputSync;
        ProtoHandler phd;
        if (!phd.SerializeReq(syncfilelistreq, strOutputSync))
        {
            LOG_ERROR_RLD("Serialize SyncFileListPendingReq req failed.");
            return 0;
        }

        ProtoHandler::SyncFileListPendingReq syncfilelistreq2;
        if (!phd.UnSerializeReq(strOutputSync, syncfilelistreq2))
        {
            LOG_ERROR_RLD("Unserialize SyncFileListPendingReq req failed.");
            return 0;
        }

        bool blRet = syncfilelistreq.m_FileInfoList.size() == syncfilelistreq2.m_FileInfoList.size();
        if (!blRet)
        {
            LOG_ERROR_RLD("Compare SyncFileListPendingReq size failed.");
            return 0;
        }

        auto itBegin2 = syncfilelistreq2.m_FileInfoList.begin();
        //auto itEnd2 = syncfilelistreq2.m_FileInfoList.end();

        auto itBegin = syncfilelistreq.m_FileInfoList.begin();
        auto itEnd = syncfilelistreq.m_FileInfoList.end();
        while (itBegin != itEnd)
        {
            blRet = itBegin->strCreatedate == itBegin2->strCreatedate && itBegin->strFileID == itBegin2->strFileID
                && itBegin->strFileMD5 == itBegin2->strFileMD5 && itBegin->strFileName == itBegin2->strFileName
                && itBegin->uiFileSize == itBegin2->uiFileSize;
            if (!blRet)
            {
                LOG_ERROR_RLD("Compare SyncFileListPendingReq info failed.");
                return 0;
            }

            ++itBegin2;
            ++itBegin;
        }

        LOG_INFO_RLD("Compare SyncFileListPendingReq success.");
    }
    
    //////////////////////////////////////////////////////////////////////////

    ControlCenter::ParamInfo pinfo;
    pinfo.strDBHost = strDBHost;
    pinfo.strDBName = strDBName;
    pinfo.strDBPassword = strDBPassword;
    pinfo.strDBPort = strDBPort;
    pinfo.strDBUser = strDBUser;

    pinfo.strRemoteAddress = strRemoteAddress;
    pinfo.strRemotePort = strRemotePort;
    pinfo.uiShakehandOfChannelInterval = boost::lexical_cast<unsigned int>(strShakehandOfChannelInterval);

    pinfo.strSelfID = strSelfID;
    pinfo.uiSyncShakehandTimeoutCount = boost::lexical_cast<unsigned int>(strSyncShakehandTimeoutCount);
    pinfo.uiSyncShakehandTimeout = boost::lexical_cast<unsigned int>(strSyncShakehandTimeout);
    pinfo.uiSyncAddressRspInvalidTimeout = boost::lexical_cast<unsigned int>(strSyncAddressRspInvalidTimeout);
    pinfo.uiThreadOfWorking = boost::lexical_cast<unsigned int>(strThreadOfWorking);

    ControlCenter ccenter(pinfo);
    //ccenter.Run(true);


    //test code
    new boost::thread(Send);
    boost::this_thread::sleep(boost::posix_time::seconds(2));
    new boost::thread(Receive);
    new boost::thread(Receive);


    boost::this_thread::sleep(boost::posix_time::seconds(1000000));

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