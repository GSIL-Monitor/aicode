#include "SessionMgr.h"
#include "LogRLD.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include "libcacheclient.h"
#include "time.h"

SessionMgr::SessionMgr() : m_TickTM(boost::bind(&SessionMgr::TimeoutCB, this, _1), 1), m_TMRunner(1), m_pMemCl(NULL)
{

}


SessionMgr::~SessionMgr()
{

}

void SessionMgr::SetMemCacheAddRess(const std::string &strMemAddress, const std::string &strMemPort)
{
    m_strMemAddress = strMemAddress;
    m_strMemPort = strMemPort;
}

bool SessionMgr::Init()
{
    bool blRet = false;

    m_pMemCl = MemcacheClient::create();
    if (MemcacheClient::CACHE_SUCCESS != m_pMemCl->addServer(m_strMemAddress.c_str(), boost::lexical_cast<int>(m_strMemPort)))
    {
        MemcacheClient::destoy(m_pMemCl);
        m_pMemCl = NULL;

        LOG_ERROR_RLD("memcached client init failed, remote ip: " << m_strMemAddress << ", remote port:" << m_strMemPort);

    }
    else
    {
        blRet = true;
        LOG_INFO_RLD("memcached client init succeed, remote ip: " << m_strMemAddress << ", remote port:" << m_strMemPort);
    }

    return blRet;
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

    MemcacheClient::destoy(m_pMemCl);
    m_pMemCl = NULL;
}

bool SessionMgr::Create(const std::string &strSessionID, const std::string &strValue, const unsigned int uiThreshold, TMOUT_CB tcb)
{
    if (0 == uiThreshold)
    {
        LOG_ERROR_RLD("Create session failed becasue threshold value is zero, sessid is " << strSessionID);
        return false;
    }

    if (!MemCacheCreate(strSessionID, strValue, uiThreshold))
    {
        LOG_ERROR_RLD("Create session failed because memcache error.");
        return false;
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
        pSession->m_strValue = strValue;
        pSession->m_TimeoutCB = tcb;

        m_SessionMap.insert(std::make_pair(strSessionID, pSession));
    }
    
    LOG_INFO_RLD("Session was created and session id is " << strSessionID);

    return true;

}

bool SessionMgr::Exist(const std::string &strSessionID)
{
    {

        boost::shared_lock<boost::shared_mutex> lock(m_SessionnMapMutex);
        auto itFind = m_SessionMap.find(strSessionID);
        if (m_SessionMap.end() == itFind)
        {
            LOG_ERROR_RLD("Session id not found in local and session id is " << strSessionID);
            return false;
        }
    }
        
    if (!MemCacheExist(strSessionID))
    {
        LOG_ERROR_RLD("Session id not found in memcache and session id is " << strSessionID);
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

    if (!MemCacheReset(strSessionID, itFind->second->m_strValue, itFind->second->m_uiThreshold))
    {
        LOG_ERROR_RLD("Rest session failed becasue memecache failed, sessid is " << strSessionID);
        return false;
    }
    
    LOG_INFO_RLD("Session was reseted and session id is " << strSessionID);
    return true;
}

bool SessionMgr::Remove(const std::string &strSessionID)
{
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

    if (!MemCacheRemove(strSessionID)) //删除时，先经过本地Session的过滤处理
    {
        LOG_ERROR_RLD("Remove session failed becasue memcache failed, sessid is " << strSessionID);
        return false;
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
    if (!MemCacheRemove(strSessionID))
    {
        LOG_ERROR_RLD("Timout remove memcached failed.");
    }

    if (NULL != tcb)
    {
        tcb(strSessionID);
    }
    
    LOG_INFO_RLD("Timout processed and session id is " << strSessionID);
}

bool SessionMgr::MemCacheRemove(const std::string &strKey)
{
    boost::unique_lock<boost::mutex> lock(m_MemcachedMutex);
    int iRet = 0;
    if (MemcacheClient::CACHE_SUCCESS != (iRet = m_pMemCl->exist(strKey.c_str())))
    {
        LOG_ERROR_RLD("Memcache not exist when being removed, key is " << strKey << " and result code is " << iRet);
        return false;
    }

    //将memcached中的session id信息删除掉，相当于将用户登出了系统
    if (MemcacheClient::CACHE_SUCCESS != (iRet = m_pMemCl->remove(strKey.c_str())))
    {
        LOG_ERROR_RLD("Memcache remove failed, key is " << strKey << " and result code is " << iRet);
        return false;
    }

    LOG_INFO_RLD("Memcached remove success and key is " << strKey);
    return true;
}

bool SessionMgr::MemCacheExist(const std::string &strKey)
{
    boost::unique_lock<boost::mutex> lock(m_MemcachedMutex);
    int iRet = 0;
    if (MemcacheClient::CACHE_SUCCESS != (iRet = m_pMemCl->exist(strKey.c_str())))
    {
        LOG_ERROR_RLD("Memcache not exist when being removed, key is " << strKey << " and result code is " << iRet);
        return false;
    }

    return true;
}

bool SessionMgr::MemCacheCreate(const std::string &strKey, const std::string &strValue, const unsigned int uiThreshold)
{
    boost::unique_lock<boost::mutex> lock(m_MemcachedMutex);

    time_t CurrentTimeValue = time(NULL);
    
    CurrentTimeValue = CurrentTimeValue + (10 + uiThreshold); //指定memcache中某条项目的超时时间比本地超时要稍微长一些

    int iRet = 0;
    if (MemcacheClient::CACHE_SUCCESS != (iRet = m_pMemCl->set(strKey.c_str(), strValue.c_str(), strValue.size(), CurrentTimeValue)))
    {
        LOG_ERROR_RLD("Memcache set failed, return code is " << iRet << " and key id is " << strKey);
        return false;
    }

    return true;
}

bool SessionMgr::MemCacheReset(const std::string &strKey, const std::string &strValue, const unsigned int uiThreshold)
{
    
    LOG_INFO_RLD("Mem cache reset and key is " << strKey << " and value is " << strValue << " and threshold is " << uiThreshold);

    return MemCacheCreate(strKey, strValue, uiThreshold);
}

