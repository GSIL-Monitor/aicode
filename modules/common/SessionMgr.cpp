#include "SessionMgr.h"
#include "LogRLD.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include "libcacheclient.h"
#include "time.h"
#include "json/json.h"

SessionMgr::SessionMgr() : m_pMemCl(NULL)
{

}


SessionMgr::~SessionMgr()
{

}

void SessionMgr::SetSessionTimeoutCB(SessionTimeoutCB scb)
{
    m_scb = scb;
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
    m_TimeOutObj.Run(1);
}


void SessionMgr::Stop()
{
    
    MemcacheClient::destoy(m_pMemCl);
    m_pMemCl = NULL;

    m_TimeOutObj.Stop();
}

bool SessionMgr::Create(const std::string &strSessionID, const std::string &strValue, const unsigned int uiThreshold, TMOUT_CB tcb, const unsigned int uiType)
{
    if (0 == uiThreshold)
    {
        LOG_ERROR_RLD("Create session failed becasue threshold value is zero, sessid is " << strSessionID);
        return false;
    }

    Json::Value jsBody;
    jsBody["sid"] = strSessionID;
    jsBody["threshold"] = uiThreshold;
    jsBody["value"] = strValue;
    jsBody["type"] = uiType;
    Json::FastWriter fastwriter;
    const std::string &strBody = fastwriter.write(jsBody); //jsBody.toStyledString();

    if (!MemCacheCreate(strSessionID, strBody, uiThreshold))
    {
        LOG_ERROR_RLD("Create session failed because memcache error.");
        return false;
    }

    boost::shared_ptr<SessionTimer> pSessionTimer(new SessionTimer);

    {
        boost::unique_lock<boost::mutex> lock(m_SessionTimerMutex);
        m_STMap.insert(std::make_pair(strSessionID, pSessionTimer));
    }

    SessionExist se = boost::bind(&SessionMgr::Exist, this, _1);
    SessionRemove sr = boost::bind(&SessionMgr::RemoveSTMap, this, _1);

    unsigned int uiTime = uiThreshold + 15; //考虑到memcached设置为uiThreshold + 10超时，所以这里为了避免检测误差，设置为递增15秒

    pSessionTimer->m_uiType = uiType;
    pSessionTimer->m_strSid = strSessionID;
    pSessionTimer->m_pTimer = m_TimeOutObj.Create(boost::bind(&SessionTimer::TimeOutCB, pSessionTimer, _1, se, sr, m_scb), uiTime);
    pSessionTimer->m_pTimer->Begin();
            
    LOG_INFO_RLD("Session was created and session id is " << strSessionID << " and vaule is " << strValue << " and timeout is " << uiTime
        << " and threshold is " << uiThreshold << " and type is " << uiType);

    return true;

}

bool SessionMgr::Exist(const std::string &strSessionID)
{
    
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
    std::string strValue;
    if (!MemCacheGet(strSessionID, strValue))
    {
        LOG_ERROR_RLD("Reset session failed beacuse key not found from memcached and key is " << strSessionID);
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(strValue, root, false))
    {
        LOG_ERROR_RLD("Reset session failed beacuse value parsed failed and key is " << strSessionID << " and value is " << strValue);
        return false;
    }

    if (!root.isObject())
    {
        LOG_ERROR_RLD("Reset session failed beacuse json root parsed failed and key is " << strSessionID << " and value is " << strValue);
        return false;
    }

    Json::Value jThreshold = root["threshold"];

    if (jThreshold.isNull())
    {
        LOG_ERROR_RLD("Reset session failed beacuse json threshold  json value is null and key is " << strSessionID << " and value is " << strValue);
        return false;
    }

    ////
    //if (!jThreshold.isUInt())
    //{
    //    LOG_ERROR_RLD("Reset session failed beacuse json threshold parsed as uint is invalid and key is " << strSessionID << " and value is " << strValue);
    //    return false;
    //}
    

    if (!MemCacheReset(strSessionID, strValue, jThreshold.asUInt()))
    {
        LOG_ERROR_RLD("Rest session failed becasue memecache failed, sessid is " << strSessionID);
        return false;
    }
    
    LOG_INFO_RLD("Session was reseted and session id is " << strSessionID << " and threshold is " << jThreshold.asUInt());
    return true;
}

bool SessionMgr::Remove(const std::string &strSessionID)
{
    RemoveSTMap(strSessionID);

    if (!MemCacheRemove(strSessionID))
    {
        LOG_ERROR_RLD("Remove session failed becasue memcache failed, sessid is " << strSessionID);
        return false;
    }

    LOG_INFO_RLD("Session was removed and session id is " << strSessionID);
    return true;
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

bool SessionMgr::MemCacheGet(const std::string &strKey, std::string &strValue)
{
    boost::unique_lock<boost::mutex> lock(m_MemcachedMutex);

    int iRet = 0;
    if (MemcacheClient::CACHE_SUCCESS != (iRet = m_pMemCl->get(strKey.c_str(), strValue)))
    {
        LOG_ERROR_RLD("Memcache get failed, key is " << strKey << " and result code is " << iRet);
        return false;
    }

    return true;
}

void SessionMgr::RemoveSTMap(const std::string &strSessionID)
{
    boost::unique_lock<boost::mutex> lock(m_SessionTimerMutex);
    auto itFind = m_STMap.find(strSessionID);
    
    if (m_STMap.end() != itFind)
    {
        itFind->second->m_pTimer->End();
        m_STMap.erase(itFind);

        LOG_INFO_RLD("Session was removed from timer map and session is " << strSessionID);
    }
}

SessionMgr::SessionTimer::SessionTimer()
{
    LOG_INFO_RLD("Session timer created.");
}

SessionMgr::SessionTimer::~SessionTimer()
{
    LOG_INFO_RLD("Session timer destroyed.");
}

void SessionMgr::SessionTimer::TimeOutCB(const boost::system::error_code& e, SessionExist se, SessionRemove sr, SessionTimeoutCB scb)
{
    if (e)
    {
        LOG_ERROR_RLD("Session time out error and msg is " << e.message() << " and session id is " << m_strSid);

        //sr(m_strSid);
        //m_pTimer->End();
        m_pTimer.reset();
        return;
    }

    LOG_INFO_RLD("Session time out called and session id is " << m_strSid);

    if (!se(m_strSid))
    {
        LOG_ERROR_RLD("Session was not existed and session id is " << m_strSid);

        if (NULL != scb)
        {
            scb(m_strSid, m_uiType);
        }
        
        sr(m_strSid);
        m_pTimer->End();
        m_pTimer.reset();
    }
    
}
