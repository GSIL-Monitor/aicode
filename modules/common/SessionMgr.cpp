#include "SessionMgr.h"
#include "LogRLD.h"
#include <boost/date_time/posix_time/posix_time.hpp>

SessionMgr::SessionMgr() : m_TickTM(boost::bind(&SessionMgr::TimeoutCB, this, _1), 1), m_TMRunner(1)
{

}


SessionMgr::~SessionMgr()
{

}

void SessionMgr::Run()
{
    m_TMRunner.Run();
    m_TickTM.Run();
}


void SessionMgr::Stop()
{
    m_TickTM.Stop();
    m_TMRunner.Stop();
}

bool SessionMgr::Create(const std::string &strSessionID, const unsigned int uiThreshold, TMOUT_CB tcb)
{
    if (0 == uiThreshold)
    {
        LOG_ERROR_RLD("Create session failed becasue threshold value is zero, sessid is " << strSessionID);
        return false;
    }

    {
        boost::shared_lock<boost::shared_mutex> lock(m_SessionnMapMutex);
        auto itFind = m_SessionMap.find(strSessionID);
        if (m_SessionMap.end() != itFind)
        {
            LOG_ERROR_RLD("Create session failed becasue session id already exists, sessid is " << strSessionID);
            return false;
        }
    }

    {
        boost::unique_lock<boost::shared_mutex> lock(m_SessionnMapMutex);
        auto itFind = m_SessionMap.find(strSessionID);
        if (m_SessionMap.end() != itFind)
        {
            LOG_ERROR_RLD("Create session failed becasue session id already exists, sessid is " << strSessionID);
            return false;
        }

        boost::shared_ptr<Session> pSession(new Session);
        pSession->m_strSessionID = strSessionID;
        pSession->m_uiTickNum = 0;
        pSession->m_uiThreshold = uiThreshold;
        pSession->m_TimeoutCB = tcb;

        m_SessionMap.insert(std::make_pair(strSessionID, pSession));
    }

    LOG_INFO_RLD("Session was created and session id is " << strSessionID);

    return true;

}

bool SessionMgr::Exist(const std::string &strSessionID)
{
    boost::shared_lock<boost::shared_mutex> lock(m_SessionnMapMutex);
    auto itFind = m_SessionMap.find(strSessionID);
    if (m_SessionMap.end() == itFind)
    {        
        return false;
    }

    LOG_INFO_RLD("Session already exist and session id is " << strSessionID);
    return true;
}

bool SessionMgr::Reset(const std::string &strSessionID)
{
    boost::shared_lock<boost::shared_mutex> lock(m_SessionnMapMutex);
    auto itFind = m_SessionMap.find(strSessionID);
    if (m_SessionMap.end() == itFind)
    {
        LOG_ERROR_RLD("Rest session failed becasue session id not exists, sessid is " << strSessionID);
        return false;
    }

    itFind->second->m_uiTickNum = 0;
    
    LOG_INFO_RLD("Session was reseted and session id is " << strSessionID);
    return true;
}

bool SessionMgr::Remove(const std::string &strSessionID)
{
    {
        boost::shared_lock<boost::shared_mutex> lock(m_SessionnMapMutex);
        auto itFind = m_SessionMap.find(strSessionID);
        if (m_SessionMap.end() == itFind)
        {
            LOG_ERROR_RLD("Remove session failed becasue session id not exists, sessid is " << strSessionID);
            return false;
        }
    }

    {
        boost::unique_lock<boost::shared_mutex> lock(m_SessionnMapMutex);
        auto itFind = m_SessionMap.find(strSessionID);
        if (m_SessionMap.end() == itFind)
        {
            LOG_ERROR_RLD("Remove session failed becasue session id not exists, sessid is " << strSessionID);
            return false;
        }

        m_SessionMap.erase(strSessionID);
    }

    LOG_INFO_RLD("Session was removed and session id is " << strSessionID);
    return true;
}

void SessionMgr::TimeoutCB(const boost::system::error_code &ec)
{
    //LOG_INFO_RLD("Session mgr check all session status");

    boost::unique_lock<boost::shared_mutex> lock(m_SessionnMapMutex);
    auto itBegin = m_SessionMap.begin();
    auto itEnd = m_SessionMap.end();
    while (itBegin != itEnd)
    {
        ++(itBegin->second->m_uiTickNum);

        if (itBegin->second->m_uiThreshold < itBegin->second->m_uiTickNum)
        {
            LOG_INFO_RLD("Timout was reached and session id is " << itBegin->second->m_strSessionID << " and threshold is " <<
                itBegin->second->m_uiThreshold << " and tick number is " << itBegin->second->m_uiTickNum);

            m_TMRunner.Post(boost::bind(&SessionMgr::TimoutProcess, this, itBegin->second->m_strSessionID, itBegin->second->m_TimeoutCB));
            m_SessionMap.erase(itBegin++);
            
            continue;
        }

        ++itBegin;
    }
        
}

void SessionMgr::TimoutProcess(const std::string &strSessionID, TMOUT_CB tcb)
{
    tcb(strSessionID);
    LOG_INFO_RLD("Timout processed and session id is " << strSessionID);
}

