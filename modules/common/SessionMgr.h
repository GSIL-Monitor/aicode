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

    void SetUserLoginMutex(const bool blEnableMutex, const bool blAllowDiffTerminal = false); //第二个参数表示是否允许不同终端同时登录
    
    static const unsigned int KICKOUT_BEFORE = 0;
    static const unsigned int KICKOUT_SELF = 1;

    void SetUerLoginKickout(const unsigned int uiKickoutType = KICKOUT_BEFORE); //默认将之前登录用户踢掉

    typedef boost::function<void(const std::string &strSessionID, const unsigned int uiType)> SessionTimeoutCB;

    void SetSessionTimeoutCB(SessionTimeoutCB scb);

    void SetMemCacheAddRess(const std::string &strMemAddress, const std::string &strMemPort);

    void SetGlobalMemCacheAddRess(const std::string &strMemAddress, const std::string &strMemPort);

    bool Init();

    void Run();

    void Stop();

    typedef boost::function<void(const std::string &)> TMOUT_CB;

    static const unsigned int TERMINAL_APP_TYPE = 0;
    static const unsigned int TERMINAL_PAD_TYPE = 1;
    static const unsigned int TERMINAL_DEV_TYPE = 0xFFFFFFFF; //设备登录时的终端类型

    static const int PARAM_ERROR = -1;
    static const int CACHE_ERROR = -2;
    static const int LOGIN_MUTEX_ERROR = -3;
    static const int CREATE_OK = 0;

    int Create(const std::string &strSessionID, const std::string &strValue, const unsigned int uiThreshold, TMOUT_CB tcb, const unsigned int uiType = 0,
        const std::string &strID = "", const unsigned int uiTerminalType = TERMINAL_APP_TYPE, const unsigned int uiLoginType = 0);

    bool GetIDBySessionID(const std::string &strSessionID, std::string &strID);

    bool GetSessionStatus(const std::string &strSessionID, int &iStatus);

    bool GetSessionLoginType(const std::string &strSessionID, unsigned int &uiLoginType);

    bool GetTerminalType(const std::string &strID, unsigned int &uiTerminalType);

    bool Exist(const std::string &strSessionID);
    
    bool ExistID(const std::string &strID); //用户id或者设备id，判断该用户或者设备是否在线

    bool Reset(const std::string &strSessionID);

    bool ResetID(const std::string &strID);

    bool Remove(const std::string &strSessionID);

    bool RemoveByID(const std::string &strID);

private:
    void TimeoutCB (const boost::system::error_code &ec);

    void TimoutProcess(const std::string &strSessionID, TMOUT_CB tcb);

    bool MemCacheRemove(const std::string &strKey);

    bool MemCacheExist(const std::string &strKey);

    bool MemCacheCreate(const std::string &strKey, const std::string &strValue, const unsigned int uiThreshold);

    bool MemCacheReset(const std::string &strKey, const std::string &strValue, const unsigned int uiThreshold);

    bool MemCacheGet(const std::string &strKey, std::string &strValue);

    void RemoveSTMap(const std::string &strSessionID);

    bool SetSessionStatus(const std::string &strID, const int iStatus);

    bool UpdateSidList(const std::string &strID, const std::string &strSessionID);

    bool ResetIDInner(const std::string &strID);

    bool ResetInner(const std::string &strSessionID);

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

    MemcacheClient *m_pMemClGlobal;
    std::string m_strMemAddressGlobal;
    std::string m_strMemPortGlobal;

    bool m_blUserLoginMutex;
    bool m_blAllowDiffTerminal;
    unsigned int m_uiKickoutType;

    Runner m_SessionCleanRunner;
    boost::mutex m_UpdateCacheMutex;

};


#endif


