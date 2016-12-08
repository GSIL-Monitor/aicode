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


/************************************************************************/
/*�Ự�����������ж���Ự�ļ�ʱ������
 *�ṩ�Ự��ʱ�����������ýӿ�
 *Author������
 *Date��2016-12-7*/
/************************************************************************/
class SessionMgr : public boost::noncopyable
{
public:
    SessionMgr();
    ~SessionMgr();

    void Run();

    void Stop();

    typedef boost::function<void(const std::string &)> TMOUT_CB;

    bool Create(const std::string &strSessionID, const unsigned int uiThreshold, TMOUT_CB tcb);

    bool Exist(const std::string &strSessionID);
    
    bool Reset(const std::string &strSessionID);

    bool Remove(const std::string &strSessionID);

private:
    void TimeoutCB (const boost::system::error_code &ec);

    void TimoutProcess(const std::string &strSessionID, TMOUT_CB tcb);

private:

    struct Session 
    {
        std::string m_strSessionID;
        unsigned int m_uiThreshold;
        boost::atomic_uint64_t m_uiTickNum;
        TMOUT_CB m_TimeoutCB;
    };

    typedef std::unordered_map<std::string, boost::shared_ptr<Session> > SessionMap;

    SessionMap m_SessionMap;
    boost::shared_mutex m_SessionnMapMutex;

    TimeOutHandler m_TickTM;

    Runner m_TMRunner;

};


#endif

