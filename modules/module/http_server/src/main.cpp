#include <stdlib.h>
#include <string.h>
#include <string>
#include <boost/filesystem.hpp> 
#include "fcgiapp.h"
#include "FCGIManager.h"
#include "FileHandler.h"
#include "ConfigSt.h"
#include "LogRLD.h"

std::string g_sUploadFilePath;

#define PROCESS_NAME     "file_cgi"
#define FILE_VERSION     "v[1.0.6] "

void InitLog(ConfigSt& cfg)
{
    std::string strHost = "http_server(127.0.0.1)";
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
    if( 2 > argc )
    {
        return -1;
    }

    boost::filesystem::path currentPath(argv[1]);
    if (!boost::filesystem::exists(currentPath))
    {
        return -1;
    }

    ConfigSt cfg(currentPath.string());

    InitLog(cfg);
    LOG_INFO_RLD("http_server begin runing...");

    FileHandler filehdr(cfg);
    if (!filehdr.Init())
    {
        LOG_ERROR_RLD("FileHandler Init failed...");
        return -1;
    }

    unsigned int thread_count = 2;
    std::string strThreadCount = cfg.GetItem("sys.thread_count");
    if (!strThreadCount.empty())
    {
        thread_count = atoi(strThreadCount.c_str());
        if (thread_count > 128)
        {
            thread_count = 2;
        }
    }

    FCGIManager fcgimgr(cfg, thread_count);
    fcgimgr.SetUploadMsgHandler(boost::bind(&FileHandler::UploadHandler, &filehdr, _1, _2));
    fcgimgr.SetSpeedUploadMsgHandler(boost::bind(&FileHandler::SpeedUploadHandler, &filehdr, _1, _2));
    fcgimgr.SetDeleteMsgHandler(boost::bind(&FileHandler::DeleteHandler, &filehdr, _1, _2));
    fcgimgr.SetDownloadMsgHandler(boost::bind(&FileHandler::DownloadHandler, &filehdr, _1, _2));

    fcgimgr.Run(true);
    return 0;
}



