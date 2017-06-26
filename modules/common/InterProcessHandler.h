#ifndef __INTER_PROCESS_HANDLER__
#define __INTER_PROCESS_HANDLER__
#include <string>
#include <boost/interprocess/managed_shared_memory.hpp> 
#include <boost/interprocess/sync/named_mutex.hpp> 
#include <boost/interprocess/sync/named_condition.hpp> 
#include <boost/interprocess/sync/scoped_lock.hpp>
#include "boost/atomic.hpp"
#include "NetComm.h"

class InterProcessHandler
{
public:

    static const unsigned int SEND_MODE = 0;
    static const unsigned int RECEIVE_MODE = 1;

    typedef boost::function<void(const std::string &)> InterPsMsgHandler;

    InterProcessHandler(const unsigned int uiMode, const std::string &strID, const unsigned int uiRunTdNum = 2, const unsigned int uiMemSize = 4096);
    ~InterProcessHandler();
    
    bool SendMsg(const std::string &strMsg);

    void SetMsgOfReceivedHandler(InterPsMsgHandler ipmsghdr);

    void RunReceivedMsg();

private:
    void MsgHandler(const std::string &strMsg);

    void ReceiveMsg();

private:
    std::string m_strID;
    std::string m_strMutexID;
    std::string m_strCondID;
    unsigned int m_uiMemSize;

    InterPsMsgHandler m_IpMsgHdr;
    
    boost::shared_ptr<boost::interprocess::managed_shared_memory> m_pMsgMem;
    boost::shared_ptr<boost::interprocess::named_mutex> m_pMsgMemMutex;
    boost::shared_ptr<boost::interprocess::named_condition> m_pMsgMemCond;

    unsigned int m_uiRunTdNum;
    Runner m_MsgHandleRunner;
    Runner m_ReceiveMsgRunner;

    unsigned int m_uiMode;

    boost::atomic_uint32_t m_uiReceiveMsgFlag;
};

#endif
