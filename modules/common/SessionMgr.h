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
/*�Ự�����������ж���Ự�ļ�ʱ����
 *�ṩ�Ự��ʱ���������ýӿڣ��ڲ�ʵ���Ͻ��Ự��Ϣ�洢��memcached
 *Author������
 *Date��2016-12-7*/
/************************************************************************/
class SessionMgr : public boost::noncopyable
{
public:
    SessionMgr();
    ~SessionMgr();

    typedef boost::function<void(const std::string &strSessionID, const unsigned int uiType)> SessionTimeoutCB;

    void SetSessionTimeoutCB(SessionTimeoutCB scb);

    void SetMemCacheAddRess(const std::string &strMemAddress, const std::string &strMemPort);

    void SetGlobalMemCacheAddRess(const std::string &strMemAddress, const std::string &strMemPort);

    bool Init();

    void Run();

    void Stop();

    typedef boost::function<void(const std::string &)> TMOUT_CB;

    bool Create(const std::string &strSessionID, const std::string &strValue, const unsigned int uiThreshold, TMOUT_CB tcb, const unsigned int uiType = 0,
        const std::string &strID = "");

    bool Exist(const std::string &strSessionID);
    
    bool ExistID(const std::string &strID); //�û�id�����豸id���жϸ��û������豸�Ƿ�����

    bool Reset(const std::string &strSessionID);

    bool ResetID(const std::string &strID);

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

    MemcacheClient *m_pMemClGlobal;
    std::string m_strMemAddressGlobal;
    std::string m_strMemPortGlobal;

};


#endif


