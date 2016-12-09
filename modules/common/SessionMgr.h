#ifndef NETDISK_SESSIONMGR
#define NETDISK_SESSIONMGR

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <string>
#include <unordered_map>
#include <boost/shared_ptr.hpp>
#include "NetComm.h"
#include <unordered_map>

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

    void SetMemCacheAddRess(const std::string &strMemAddress, const std::string &strMemPort);

    bool Init();

    void Run();

    void Stop();

    typedef boost::function<void(const std::string &)> TMOUT_CB;

    bool Create(const std::string &strSessionID, const std::string &strValue, const unsigned int uiThreshold, TMOUT_CB tcb);

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

private:

    struct Session 
    {
        std::string m_strSessionID;
        unsigned int m_uiThreshold;
        std::string m_strValue;
        boost::atomic_uint64_t m_uiTickNum;
        TMOUT_CB m_TimeoutCB;
    };

    typedef std::unordered_map<std::string, boost::shared_ptr<Session> > SessionMap;

    SessionMap m_SessionMap;
    boost::shared_mutex m_SessionnMapMutex;

    TimeOutHandler m_TickTM;

    Runner m_TMRunner;

    std::string m_strMemAddress;
    std::string m_strMemPort;

    boost::mutex m_MemcachedMutex;
    MemcacheClient *m_pMemCl;

};


#endif


