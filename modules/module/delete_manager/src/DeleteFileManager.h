#ifndef __DELETE_FILE_MSG__
#define __DELETE_FILE_MSG__
#include <string>
#include "NetComm.h"
#include "mysql_impl.h"


class DeleteFileManager
{
public:
    typedef struct _ParamInfo
    {
        std::string m_strDBHost;
        std::string m_strDBPort;
        std::string m_strDBUser;
        std::string m_strDBPassword;
        std::string m_strDBName;
        std::string m_strMemAddress;
        std::string m_strMemPort;
        std::string m_strSessionTimeoutCountThreshold;
        unsigned int m_uiTimeInterval;
        std::string m_strEndTime;
        std::string m_strHttpIp;

    } ParamInfo;

    DeleteFileManager(const ParamInfo &pinfo);
    ~DeleteFileManager();
    
    bool Run();

private:
    void TimerHandler(const boost::system::error_code& e);

private:
    bool m_Init;

private:
    ParamInfo m_ParamInfo;
    MysqlImpl *m_pMysql;
    TimeOutHandler m_Tmh;
    
};

#endif
