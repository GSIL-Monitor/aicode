#include "CacheMgr.h"
#include "LogRLD.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include "libcacheclient.h"
#include "time.h"

CacheMgr::CacheMgr() : m_pMemCl(NULL)
{

}


CacheMgr::~CacheMgr()
{
    MemcacheClient::destoy(m_pMemCl);
    m_pMemCl = NULL;
}

void CacheMgr::SetMemCacheAddRess(const std::string &strMemAddress, const std::string &strMemPort)
{
    m_strMemAddress = strMemAddress;
    m_strMemPort = strMemPort;
}

bool CacheMgr::Init()
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

bool CacheMgr::MemCacheRemove(const std::string &strKey)
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

bool CacheMgr::MemCacheExist(const std::string &strKey)
{
    boost::unique_lock<boost::mutex> lock(m_MemcachedMutex);
    int iRet = 0;
    if (MemcacheClient::CACHE_SUCCESS != (iRet = m_pMemCl->exist(strKey.c_str())))
    {        
        LOG_ERROR_RLD("Memcache not exist and key is " << strKey << " and result code is " << iRet);
        return false;
    }

    return true;
}

bool CacheMgr::MemCacheCreate(const std::string &strKey, const std::string &strValue, const unsigned int uiThreshold)
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

bool CacheMgr::MemCacheReset(const std::string &strKey, const std::string &strValue, const unsigned int uiThreshold)
{
    
    LOG_INFO_RLD("Mem cache reset and key is " << strKey << " and value is " << strValue << " and threshold is " << uiThreshold);

    return MemCacheCreate(strKey, strValue, uiThreshold);
}

bool CacheMgr::MemCacheGet(const std::string &strKey, std::string &strValue)
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
