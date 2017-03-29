#ifndef NETDISK_SESSIONMGR
#define NETDISK_SESSIONMGR

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <string>
#include <unordered_map>
#include <boost/shared_ptr.hpp>
#include "NetComm.h"


class MemcacheClient;

/************************************************************************/
/*会话管理器，进行多个会话的计时处理。
 *提供会话超时处理函数设置接口，内部实现上将会话信息存储到memcached
 *Author：尹宾
 *Date：2016-12-7*/
/************************************************************************/
class SessionMgr : public boost::noncopyable
{
public:
    SessionMgr();
    ~SessionMgr();

    typedef boost::function<void(const std::string &strSessionID, const unsigned int uiType)> SessionTimeoutCB;

    void SetSessionTimeoutCB(SessionTimeoutCB scb);

    void SetMemCacheAddRess(const std::string &strMemAddress, const std::string &strMemPort);

    bool Init();

    void Run();

    void Stop();

    typedef boost::function<void(const std::string &)> TMOUT_CB;

    bool Create(const std::string &strSessionID, const std::string &strValue, const unsigned int uiThreshold, TMOUT_CB tcb, const unsigned int uiType = 0);

    bool Exist(const std::string &strSessionID);
    
    bool Reset(const std::string &strSessionID);

    bool Remove(const std::string &strSessionID);

private:
    void TimeoutCB (const boost::system::error_code &ec);

    void TimoutProcess(const std::string &strSessionID, TMOUT_CB tcb);

    bool MemCacheRemove(const std::string &strKey);

    bool MemCacheExist(const std::string &strKey);

    bool MemCacheCreate(const std::string &strKey, const std::string &strValue, const unsigned int uiThreshold);

    bool MemCacheReset(const std::string &strKey, const std::string &strValue, const unsigned int uiThreshold);

    bool MemCacheGet(const std::string &strKey, std::string &strValue);

    void RemoveSTMap(const std::string &strSessionID);

private:

    SessionTimeoutCB m_scb;

    typedef boost::function<bool(const std::string &strSessionID)> SessionExist;
    typedef boost::function<void(const std::string &strSessionID)> SessionRemove;
    
    struct SessionTimer
    {
        SessionTimer();
        ~SessionTimer();
        boost::shared_ptr<TimeOutHandler> m_pTimer;
        std::string m_strSid;
        unsigned int m_uiType;
        
        void TimeOutCB(const boost::system::error_code& e, SessionExist se, SessionRemove sr, SessionTimeoutCB scb);
    };

    boost::mutex m_SessionTimerMutex;
    std::unordered_map<std::string, boost::shared_ptr<SessionTimer> > m_STMap;

    std::string m_strMemAddress;
    std::string m_strMemPort;

    boost::mutex m_MemcachedMutex;
    MemcacheClient *m_pMemCl;

    TimeOutHandlerEx m_TimeOutObj;

};


#endif


