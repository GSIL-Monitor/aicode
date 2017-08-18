#ifndef _USER_MANAGER_
#define _USER_MANAGER_

#include "boost/noncopyable.hpp"
#include "ControlCenter.h"
#include "InteractiveProtoHandler.h"
#include <unordered_map>
#include <string>
#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "NetComm.h"
#include "DBInfoCacheManager.h"
#include "boost/atomic.hpp"
#include "SessionMgr.h"
#include "ClusterAccessCollector.h"

class MysqlImpl;
class InterProcessHandler;

/************************************************************************/
/* 用户管理类，提供了管理用户所有相关的方法接口和实现。
 * 该类提供的方法会注册到到ControlCenter类中。
 * Author：尹宾
 * Date：2016-12-1*/
/************************************************************************/
class PassengerFlowManager : public boost::noncopyable
{
public:
    
    static const unsigned int UNUSED_INPUT_UINT = 0xFFFFFFFF;
    
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
        std::string m_strDevSessionTimeoutCountThreshold;
        std::string m_strLTUserSite;
        std::string m_strLTUserSiteRC4Key;
        std::string m_strFileServerURL;
        std::string m_strGetIpInfoSite;
        std::string m_strMemAddressGlobal;
        std::string m_strMemPortGlobal;
        std::string m_strUserLoginMutex;
        std::string m_strUserAllowDiffTerminal;
        std::string m_strUserKickoutType;
        std::string m_strMasterNode;
    } ParamInfo;
    
    typedef struct _AccessDomainInfo
    {
        std::string strDomainName;
        unsigned int uiLease;
    } AccessDomainInfo;
    
    inline void SetParamInfo(const ParamInfo &pinfo)
    {
        m_ParamInfo = pinfo;
    };

    PassengerFlowManager(const ParamInfo &pinfo);
    ~PassengerFlowManager();
    bool Init();

    bool GetMsgType(const std::string &strMsg, int &iMsgType);

    bool PreCommonHandler(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);
    
private:
    

private:
    ParamInfo m_ParamInfo;

    Runner m_DBRuner;
    boost::shared_ptr<InteractiveProtoHandler> m_pProtoHandler;
    
    MysqlImpl *m_pMysql;
    DBInfoCacheManager m_DBCache;
    
    boost::atomic_uint64_t m_uiMsgSeq;
    
    SessionMgr m_SessionMgr;

    TimeOutHandler m_DBTimer;

    boost::shared_ptr<InterProcessHandler> m_MsgSender;
    boost::shared_ptr<InterProcessHandler> m_MsgReceiver;

};



#endif