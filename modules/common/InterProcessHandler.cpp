#include "InterProcessHandler.h"
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include "LogRLD.h"

typedef boost::interprocess::allocator<char, boost::interprocess::managed_shared_memory::segment_manager> StdStringAllocator;
typedef boost::interprocess::basic_string<char, std::char_traits<char>, StdStringAllocator> StdString;


InterProcessHandler::InterProcessHandler(const unsigned int uiMode, const std::string &strID, const unsigned int uiRunTdNum /*= 2*/, const unsigned int uiMemSize /*= 4096*/) :
m_strID(strID), m_strMutexID(strID + "_mutex"), m_strCondID(strID + "_cond"), m_uiMemSize(uiMemSize), m_uiRunTdNum(uiRunTdNum),
m_MsgHandleRunner(uiRunTdNum), m_ReceiveMsgRunner(1), m_SendMsgRunner(1), m_uiMode(uiMode), m_uiReceiveMsgFlag(1)
{
}

InterProcessHandler::~InterProcessHandler()
{
    if (RECEIVE_MODE == m_uiMode)
    {
        m_uiReceiveMsgFlag = 0;
        m_ReceiveMsgRunner.Stop();
        m_SendMsgRunner.Stop();

        m_MsgHandleRunner.Stop();

        boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(*m_pMsgMemMutex);
        m_pMsgMemCond->notify_all();
        boost::interprocess::shared_memory_object::remove(m_strID.c_str());
        boost::interprocess::named_mutex::remove(m_strMutexID.c_str());
        boost::interprocess::named_condition::remove(m_strCondID.c_str());
    }
    
}

bool InterProcessHandler::Init()
{
    try
    {
        m_pMsgMem.reset(new boost::interprocess::shared_memory_object(boost::interprocess::open_or_create, m_strID.c_str(), boost::interprocess::read_write));
        m_pMsgMem->truncate(m_uiMemSize);
        m_pMsgMemRegion.reset(new boost::interprocess::mapped_region(*m_pMsgMem, boost::interprocess::read_write));

        m_pMsgMemMutex.reset(new boost::interprocess::named_mutex(boost::interprocess::open_or_create, m_strMutexID.c_str()));
        m_pMsgMemCond.reset(new boost::interprocess::named_condition(boost::interprocess::open_or_create, m_strCondID.c_str()));
    }
    catch (boost::interprocess::interprocess_exception &e)
    {
        LOG_ERROR_RLD("InterProcessHandler init failed and exception is " << e.what());
        return false;
    }
    catch (...)
    {
        LOG_ERROR_RLD("InterProcessHandler init failed and unknown error occur");
        return false;
    }
    return true;
}

void InterProcessHandler::SendMsg(const std::string &strMsg)
{
    m_SendMsgRunner.Post(boost::bind(&InterProcessHandler::SendMsgInner, this, strMsg));
}

bool InterProcessHandler::SendMsgInner(const std::string &strMsg)
{
    if (SEND_MODE != m_uiMode)
    {
        LOG_ERROR_RLD("Current mode is not send mode and mode is " << m_uiMode);
        return false;
    }

    boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(*m_pMsgMemMutex);

    if (strMsg.size() > m_uiMemSize)
    {
        LOG_ERROR_RLD("Msg size of send is too large and msg size is " << strMsg.size() << " and mem size is " << m_uiMemSize);
        return false;
    }

    unsigned int *pMsgLen = static_cast<unsigned int*>(m_pMsgMemRegion->get_address());
    *pMsgLen = strMsg.size();

    char *pMsgContent = static_cast<char *>(m_pMsgMemRegion->get_address());
    pMsgContent += sizeof(unsigned int);

    memcpy(pMsgContent, strMsg.data(), strMsg.size());
        
    LOG_INFO_RLD("Send interprocess msg is " << strMsg);

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

void InterProcessHandler::RunSendMsg(const bool isWaitRunFinished /*= false*/)
{
    if (SEND_MODE == m_uiMode)
    {        
        m_SendMsgRunner.Run(isWaitRunFinished);
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
    bool blFirst = true;
    while (m_uiReceiveMsgFlag)
    {
        boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(*m_pMsgMemMutex);

        if (blFirst)
        {
            blFirst = false;

            LOG_INFO_RLD("Receive interprocess msg is begin");

            m_pMsgMemCond->notify_all();
            m_pMsgMemCond->wait(lock);
            continue;
        }

        unsigned int *pMsgLen = static_cast<unsigned int*>(m_pMsgMemRegion->get_address());
        
        char *pMsgContentSrc = static_cast<char *>(m_pMsgMemRegion->get_address());
        pMsgContentSrc += sizeof(unsigned int);

        std::string strMsgReceived(pMsgContentSrc, *pMsgLen);
        
        LOG_INFO_RLD("Receive interprocess msg is " << strMsgReceived);

        m_MsgHandleRunner.Post(boost::bind(&InterProcessHandler::MsgHandler, this, strMsgReceived));

        m_pMsgMemCond->notify_all();
        m_pMsgMemCond->wait(lock);

        LOG_INFO_RLD("Receive msg was notified");
    }
}

