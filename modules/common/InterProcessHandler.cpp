#include "InterProcessHandler.h"
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include "LogRLD.h"

typedef boost::interprocess::allocator<char, boost::interprocess::managed_shared_memory::segment_manager> StdStringAllocator;
typedef boost::interprocess::basic_string<char, std::char_traits<char>, StdStringAllocator> StdString;


InterProcessHandler::InterProcessHandler(const unsigned int uiMode, const std::string &strID, const unsigned int uiRunTdNum /*= 2*/, const unsigned int uiMemSize /*= 4096*/) :
m_strID(strID), m_strMutexID(strID + "_mutex"), m_strCondID(strID + "_cond"), m_uiMemSize(uiMemSize), m_uiRunTdNum(uiRunTdNum),
m_MsgHandleRunner(uiRunTdNum), m_ReceiveMsgRunner(1), m_uiMode(uiMode), m_uiReceiveMsgFlag(1)
{
    m_pMsgMem.reset(new boost::interprocess::managed_shared_memory(boost::interprocess::open_or_create, m_strID.c_str(), uiMemSize));
    m_pMsgMemMutex.reset(new boost::interprocess::named_mutex(boost::interprocess::open_or_create, m_strMutexID.c_str()));
    m_pMsgMemCond.reset(new boost::interprocess::named_condition(boost::interprocess::open_or_create, m_strCondID.c_str()));

}

InterProcessHandler::~InterProcessHandler()
{
    if (RECEIVE_MODE == m_uiMode)
    {
        m_uiReceiveMsgFlag = 0;
        m_ReceiveMsgRunner.Stop();

        m_MsgHandleRunner.Stop();

        boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(*m_pMsgMemMutex);
        m_pMsgMemCond->notify_all();
        boost::interprocess::shared_memory_object::remove(m_strID.c_str());
        boost::interprocess::named_mutex::remove(m_strMutexID.c_str());
        boost::interprocess::named_condition::remove(m_strCondID.c_str());
    }
    
}

bool InterProcessHandler::SendMsg(const std::string &strMsg)
{
    if (SEND_MODE != m_uiMode)
    {
        LOG_ERROR_RLD("Current mode is not send mode and mode is " << m_uiMode);
        return false;
    }

    boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(*m_pMsgMemMutex);

    StdString *pStrMsg = NULL;
    try
    {
        pStrMsg = m_pMsgMem->find_or_construct<StdString>("String")(strMsg.c_str(), m_pMsgMem->get_segment_manager());
    }
    catch (boost::interprocess::bad_alloc &ex)
    {
        LOG_ERROR_RLD("Send interprocess msg failed and error is " << ex.what() << " and msg is " << strMsg);
        return false;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Send interprocess msg failed and msg is " << strMsg);
        return false;
    }

    if (NULL == pStrMsg)
    {
        LOG_ERROR_RLD("Get string from shared mem failed");
        return false;
    }

    pStrMsg->clear();
    pStrMsg->assign(strMsg.data(), strMsg.size());

    LOG_INFO_RLD("Send interprocess msg is " << *pStrMsg);

    m_pMsgMemCond->notify_all();
    m_pMsgMemCond->wait(lock);

    LOG_INFO_RLD("Send msg was notified");

    return true;
}

void InterProcessHandler::SetMsgOfReceivedHandler(InterPsMsgHandler ipmsghdr)
{
    m_IpMsgHdr = ipmsghdr;
}

void InterProcessHandler::RunReceivedMsg(const bool isWaitRunFinished)
{
    if (RECEIVE_MODE == m_uiMode)
    {
        m_MsgHandleRunner.Run(false);
        
        m_ReceiveMsgRunner.Post(boost::bind(&InterProcessHandler::ReceiveMsg, this));
        m_ReceiveMsgRunner.Run(isWaitRunFinished);
    }
}

void InterProcessHandler::MsgHandler(const std::string &strMsg)
{
    if (NULL != m_IpMsgHdr)
    {
        m_IpMsgHdr(strMsg);
    }
    else
    {
        LOG_INFO_RLD("Msg handler is null");
    }
}

void InterProcessHandler::ReceiveMsg()
{
    while (m_uiReceiveMsgFlag)
    {
        boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(*m_pMsgMemMutex);

        StdString *pStrMsg = NULL;
        try
        {
            pStrMsg = m_pMsgMem->find_or_construct<StdString>("String")("empty", m_pMsgMem->get_segment_manager());
        }
        catch (boost::interprocess::bad_alloc &ex)
        {
            LOG_ERROR_RLD("Send interprocess msg failed and error is " << ex.what());
            return;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Send interprocess msg failed");
            return;
        }

        if (NULL == pStrMsg)
        {
            LOG_ERROR_RLD("Get string from shared mem failed");
            return;
        }

        if (*pStrMsg == "empty")
        {
            LOG_INFO_RLD("Receive interprocess msg is empty");

            m_pMsgMemCond->notify_all();
            m_pMsgMemCond->wait(lock);
            continue;
        }

        std::string strMsgReceived(pStrMsg->c_str(), pStrMsg->size());
        
        LOG_INFO_RLD("Receive interprocess msg is " << strMsgReceived);

        m_MsgHandleRunner.Post(boost::bind(&InterProcessHandler::MsgHandler, this, strMsgReceived));

        m_pMsgMemCond->notify_all();
        m_pMsgMemCond->wait(lock);

        LOG_INFO_RLD("Receive msg was notified");
    }
}

