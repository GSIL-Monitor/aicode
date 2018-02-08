#include "SessionMgr.h"
#include "LogRLD.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include "libcacheclient.h"
#include "time.h"
#include "json/json.h"

SessionMgr::SessionMgr() : m_pMemCl(NULL), m_pMemClGlobal(NULL), m_blUserLoginMutex(false), m_blAllowDiffTerminal(false), m_uiKickoutType(KICKOUT_BEFORE),
m_SessionCleanRunner(1)
{

}


SessionMgr::~SessionMgr()
{

}

void SessionMgr::SetUserLoginMutex(const bool blEnableMutex, const bool blAllowDiffTerminal /*= false*/)
{
    m_blUserLoginMutex = blEnableMutex;
    m_blAllowDiffTerminal = blAllowDiffTerminal;
}

void SessionMgr::SetUerLoginKickout(const unsigned int uiKickoutType /*= KICKOUT_BEFORE*/)
{
    m_uiKickoutType = uiKickoutType;
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

void SessionMgr::SetGlobalMemCacheAddRess(const std::string &strMemAddress, const std::string &strMemPort)
{
    m_strMemAddressGlobal = strMemAddress;
    m_strMemPortGlobal = strMemPort;
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

    if (!m_strMemAddressGlobal.empty() && !m_strMemPortGlobal.empty())
    {
        blRet = false;
        m_pMemClGlobal = MemcacheClient::create();
        if (MemcacheClient::CACHE_SUCCESS != m_pMemClGlobal->addServer(m_strMemAddressGlobal.c_str(), boost::lexical_cast<int>(m_strMemPortGlobal)))
        {
            MemcacheClient::destoy(m_pMemClGlobal);
            m_pMemClGlobal = NULL;

            LOG_ERROR_RLD("memcached client init failed, remote ip: " << m_strMemAddressGlobal << ", remote port:" << m_strMemPortGlobal);

        }
        else
        {
            blRet = true;
            LOG_INFO_RLD("memcached client init succeed, remote ip: " << m_strMemAddressGlobal << ", remote port:" << m_strMemPortGlobal);
        }
    }

    m_SessionCleanRunner.Run();

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

    if (m_pMemClGlobal)
    {
        MemcacheClient::destoy(m_pMemClGlobal);
        m_pMemClGlobal = NULL;
    }

    m_TimeOutObj.Stop();
}

int SessionMgr::Create(const std::string &strSessionID, const std::string &strValue, const unsigned int uiThreshold, TMOUT_CB tcb, const unsigned int uiType,
    const std::string &strID, const unsigned int uiTerminalType, const unsigned int uiLoginType)
{
    if (0 == uiThreshold)
    {
        LOG_ERROR_RLD("Create session failed becasue threshold value is zero, sessid is " << strSessionID);
        return PARAM_ERROR;
    }

    if (!strID.empty())
    {
        boost::unique_lock<boost::mutex> lock(m_UpdateCacheMutex);

        if (!MemCacheExist(strID))
        {
            Json::Value jsBody;
            jsBody["id"] = strID;
            jsBody["threshold"] = uiThreshold;
            jsBody["terminaltype"] = uiTerminalType;
            Json::Value jsSid;
            jsSid[(unsigned int)0] = strSessionID;
            jsBody["sid"] = jsSid;
            //jsBody["sid"] = strSessionID;
            
            Json::FastWriter fastwriter;
            const std::string &strBody = fastwriter.write(jsBody); //jsBody.toStyledString();

            if (!MemCacheCreate(strID, strBody, uiThreshold))
            {
                LOG_ERROR_RLD("Create id failed because memcache error.");
                return CACHE_ERROR;
            }

            LOG_INFO_RLD("ID info is " << strBody);
        }
        else
        {
            //更新缓存中sid信息
            if (!UpdateSidList(strID, strSessionID))
            {
                LOG_ERROR_RLD("Update session id list failed and id is " << strID << " and session id is " << strSessionID);
                return CACHE_ERROR;
            }

            //该id已经有会话登录
            if (m_blUserLoginMutex) //启用会话互斥
            {
                bool blNeedVaild = true;
                if (m_blAllowDiffTerminal) //启用多终端共存
                {
                    unsigned int uiTerminalTypeAlreadyLogin;
                    if (!GetTerminalType(strID, uiTerminalTypeAlreadyLogin))
                    {
                        LOG_ERROR_RLD("Get terminal type failed because memcache error.");
                        return CACHE_ERROR;
                    }

                    blNeedVaild = uiTerminalType == uiTerminalTypeAlreadyLogin; //终端类型相同，说明不满足多终端共存条件，需要进行后续的会话校验
                }

                //最终决定是否需要校验会话
                if (blNeedVaild)
                {
                    int iRet = 0;
                    if (KICKOUT_BEFORE == m_uiKickoutType) //踢掉之前登录的用户会话，自身继续走下去创建会话
                    {
                        //Remove(strSessionID);
                        if (!SetSessionStatus(strID, LOGIN_MUTEX_ERROR))
                        {
                            LOG_ERROR_RLD("Set session status failed and id is " << strID);
                            return CACHE_ERROR;
                        }
                    }
                    else //踢掉自身，不在创建自身会话
                    {
                        iRet = LOGIN_MUTEX_ERROR;
                    }

                    LOG_ERROR_RLD("Multi session login was found and kickout type is " << m_uiKickoutType << " and result is " << iRet);

                    if (LOGIN_MUTEX_ERROR == iRet)
                    {
                        return iRet;
                    }
                }
            }
        }
    }

    Json::Value jsBody;
    jsBody["sid"] = strSessionID;
    jsBody["threshold"] = uiThreshold;
    jsBody["value"] = strValue;
    jsBody["type"] = uiType;
    jsBody["terminaltype"] = uiTerminalType;
    jsBody["status"] = CREATE_OK;
    jsBody["logintype"] = uiLoginType;
    
    if (!strID.empty())
    {
        jsBody["id"] = strID;
    }

    Json::FastWriter fastwriter;
    const std::string &strBody = fastwriter.write(jsBody); //jsBody.toStyledString();

    if (!MemCacheCreate(strSessionID, strBody, uiThreshold))
    {
        LOG_ERROR_RLD("Create session failed because memcache error.");
        return CACHE_ERROR;
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
        << " and threshold is " << uiThreshold << " and type is " << uiType << " and id is " << strID << " and terminal type is " << uiTerminalType);

    return CREATE_OK;

}

bool SessionMgr::GetIDBySessionID(const std::string &strSessionID, std::string &strID)
{
    std::string strValue;
    if (!MemCacheGet(strSessionID, strValue))
    {
        LOG_ERROR_RLD("Get session info failed beacuse key not found from memcached and key is " << strSessionID);
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(strValue, root, false))
    {
        LOG_ERROR_RLD("Get session info failed beacuse value parsed failed and key is " << strSessionID << " and value is " << strValue);
        return false;
    }

    if (!root.isObject())
    {
        LOG_ERROR_RLD("Get session info failed beacuse json root parsed failed and key is " << strSessionID << " and value is " << strValue);
        return false;
    }

    Json::Value jTerm = root["id"];

    if (jTerm.isNull() || !jTerm.isString())
    {
        LOG_ERROR_RLD("Get id by session id failed and key is " << strSessionID << " and value is " << strValue);
        return false;
    }

    strID = jTerm.asString();

    return true;
}

bool SessionMgr::GetSessionStatus(const std::string &strSessionID, int &iStatus)
{
    std::string strValue;
    if (!MemCacheGet(strSessionID, strValue))
    {
        LOG_ERROR_RLD("Get session info failed beacuse key not found from memcached and key is " << strSessionID);
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(strValue, root, false))
    {
        LOG_ERROR_RLD("Get session info failed beacuse value parsed failed and key is " << strSessionID << " and value is " << strValue);
        return false;
    }

    if (!root.isObject())
    {
        LOG_ERROR_RLD("Get session info failed beacuse json root parsed failed and key is " << strSessionID << " and value is " << strValue);
        return false;
    }

    Json::Value jTerm = root["status"];

    if (jTerm.isNull())
    {
        LOG_ERROR_RLD("Get session info failed beacuse json threshold  json value is null and key is " << strSessionID << " and value is " << strValue);
        return false;
    }

    iStatus = jTerm.asInt();

    return true;
}

bool SessionMgr::GetSessionLoginType(const std::string &strSessionID, unsigned int &uiLoginType)
{
    std::string strValue;
    if (!MemCacheGet(strSessionID, strValue))
    {
        LOG_ERROR_RLD("Get session info failed beacuse key not found from memcached and key is " << strSessionID);
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(strValue, root, false))
    {
        LOG_ERROR_RLD("Get session info failed beacuse value parsed failed and key is " << strSessionID << " and value is " << strValue);
        return false;
    }

    if (!root.isObject())
    {
        LOG_ERROR_RLD("Get session info failed beacuse json root parsed failed and key is " << strSessionID << " and value is " << strValue);
        return false;
    }

    Json::Value jLoginType = root["logintype"];

    if (jLoginType.isNull())
    {
        LOG_ERROR_RLD("Get session info failed beacuse json threshold  json value is null and key is " << strSessionID << " and value is " << strValue);
        return false;
    }

    uiLoginType = jLoginType.asUInt();

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

bool SessionMgr::ExistID(const std::string &strID)
{
    if (!MemCacheExist(strID))
    {
        LOG_ERROR_RLD("ID not found in memcache and id is " << strID);
        return false;
    }

    LOG_INFO_RLD("ID already exist and id is " << strID);
    return true;
}

bool SessionMgr::Reset(const std::string &strSessionID)
{
    m_SessionCleanRunner.Post(boost::bind(&SessionMgr::ResetInner, this, strSessionID));
    return true;
}

bool SessionMgr::ResetInner(const std::string &strSessionID)
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

    {
        Json::Value jID = root["id"];

        if (!jID.isNull())
        {
            LOG_INFO_RLD("Get id info from session and sid is " << strSessionID << " and id is " << jID.asString());
            ResetID(jID.asString());
        }

    }
    
    LOG_INFO_RLD("Session was reseted and session id is " << strSessionID << " and threshold is " << jThreshold.asUInt());
    return true;
}

bool SessionMgr::ResetID(const std::string &strID)
{
    m_SessionCleanRunner.Post(boost::bind(&SessionMgr::ResetIDInner, this, strID));
    return true;
}


bool SessionMgr::ResetIDInner(const std::string &strID)
{
    boost::unique_lock<boost::mutex> lock(m_UpdateCacheMutex);

    std::string strValue;
    if (!MemCacheGet(strID, strValue))
    {
        LOG_ERROR_RLD("Reset id failed beacuse key not found from memcached and key is " << strID);
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(strValue, root, false))
    {
        LOG_ERROR_RLD("Reset id failed beacuse value parsed failed and key is " << strID << " and value is " << strValue);
        return false;
    }

    if (!root.isObject())
    {
        LOG_ERROR_RLD("Reset id failed beacuse json root parsed failed and key is " << strID << " and value is " << strValue);
        return false;
    }

    Json::Value jThreshold = root["threshold"];

    if (jThreshold.isNull())
    {
        LOG_ERROR_RLD("Reset id failed beacuse json threshold  json value is null and key is " << strID << " and value is " << strValue);
        return false;
    }

    Json::Value jsID = root["id"];
    Json::Value jsTerminaltype = root["terminaltype"];
    
    if (jsID.isNull() || jsTerminaltype.isNull())
    {
        LOG_ERROR_RLD("ID json or terminal type value is null and key is " << strID << " and value is " << strValue);
        return false;
    }

    Json::Value JSid = root["sid"];
    if (JSid.isNull())
    {
        LOG_ERROR_RLD("Sid json value is null and key is " << strID << " and value is " << strValue);
        return false;
    }

    if (!JSid.isArray())
    {
        LOG_ERROR_RLD("Sid json value is not array, raw data is : " << strValue);
        return false;
    }

    std::list<std::string> strSidValidList;
    unsigned int uiSize = JSid.size();
    for (unsigned int i = 0; i < uiSize; ++i)
    {
        auto jsSid = JSid[i];
        if (jsSid.isNull() || !jsSid.isString())
        {
            LOG_ERROR_RLD("Sid info failed, raw data is: " << strValue);
            return false;
        }

        if (Exist(jsSid.asString()))
        {
            strSidValidList.push_back(jsSid.asString());
            LOG_INFO_RLD("Sid of valid is  " << jsSid.asString() << " and ID is " << strID);
        }
    }

    if (strSidValidList.empty())
    {
        MemCacheRemove(strID);
        LOG_INFO_RLD("Valid session is empty and ID is " << strID);
        return true;
    }

    Json::Value jsBody;
    Json::Value jsSidValid;

    unsigned int i = 0;
    for (auto itBegin = strSidValidList.begin(), itEnd = strSidValidList.end(); itBegin != itEnd; ++itBegin, ++i)
    {
        jsSidValid[i] = *itBegin;
    }

    jsBody["sid"] = jsSidValid;
    jsBody["id"] = jsID.asString();
    jsBody["threshold"] = jThreshold.asUInt();
    jsBody["terminaltype"] = jsTerminaltype.asUInt();

    Json::FastWriter fastwriter;
    const std::string &strBody = fastwriter.write(jsBody);

    /**
     *Json::Value jsBody;
     jsBody["id"] = strID;
     jsBody["threshold"] = uiThreshold;
     jsBody["terminaltype"] = uiTerminalType;
     Json::Value jsSid;
     jsSid[(unsigned int)0] = strSessionID;
     jsBody["sid"] = jsSid;
     //jsBody["sid"] = strSessionID;

     Json::FastWriter fastwriter;
     const std::string &strBody = fastwriter.write(jsBody); //jsBody.toStyledString();

     if (!MemCacheCreate(strID, strBody, uiThreshold))
     {
     LOG_ERROR_RLD("Create id failed because memcache error.");
     return CACHE_ERROR;
     }
     *
     */


    if (!MemCacheReset(strID, strBody, jThreshold.asUInt()))
    {
        LOG_ERROR_RLD("Rest id failed becasue memecache failed, id is " << strID);
        return false;
    }

    LOG_INFO_RLD("ID was reseted and id is " << strID << " and threshold is " << jThreshold.asUInt());
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

bool SessionMgr::RemoveByID(const std::string &strID)
{
    std::string strValue;
    if (!MemCacheGet(strID, strValue))
    {
        LOG_ERROR_RLD("Remove session by id failed beacuse key not found from memcached and key is " << strID);
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(strValue, root, false))
    {
        LOG_ERROR_RLD("Remove session by id failed beacuse value parsed failed and key is " << strID << " and value is " << strValue);
        return false;
    }

    if (!root.isObject())
    {
        LOG_ERROR_RLD("Remove session by id failed beacuse json root parsed failed and key is " << strID << " and value is " << strValue);
        return false;
    }

    Json::Value JSid = root["sid"];
    if (JSid.isNull())
    {
        LOG_ERROR_RLD("Remove session by id failed beacuse json threshold  json value is null and key is " << strID << " and value is " << strValue);
        return false;
    }

    if (!JSid.isArray())
    {
        LOG_ERROR_RLD("Session id info failed, raw data is : " << strValue);
        return false;
    }

    for (unsigned int i = 0; i < JSid.size(); ++i)
    {
        auto jsSidValue = JSid[i];
        if (jsSidValue.isNull() || !jsSidValue.isString())
        {
            LOG_ERROR_RLD("Session id info failed, raw data is : " << strValue);
            return false;
        }

        LOG_INFO_RLD("Remove session  id is " << jsSidValue.asString());

        Remove(jsSidValue.asString());        
    }
    
    LOG_INFO_RLD("Remove session by id " << strID << " is succeed.");
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

    if (!m_strMemAddressGlobal.empty() && !m_strMemPortGlobal.empty())
    {
        if (MemcacheClient::CACHE_SUCCESS == (iRet = m_pMemClGlobal->exist(strKey.c_str())))
        {
            if (MemcacheClient::CACHE_SUCCESS != (iRet = m_pMemClGlobal->remove(strKey.c_str())))
            {
                LOG_ERROR_RLD("Memcache of global remove failed, key is " << strKey << " and result code is " << iRet);
                //return false;  //考虑到全局缓存可能是远程的，并且缓存目前都自带有超时配置，所以为了不影响本地cached业务，所以这里不返回失败
            }
        }       
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
        if (!m_strMemAddressGlobal.empty() && !m_strMemPortGlobal.empty())
        {
            if (MemcacheClient::CACHE_SUCCESS != (iRet = m_pMemClGlobal->exist(strKey.c_str())))
            {
                LOG_ERROR_RLD("Memcache of global and local not exist and key is " << strKey << " and result code is " << iRet);
                return false;
            }
        }

        LOG_ERROR_RLD("Memcache not exist and key is " << strKey << " and result code is " << iRet);
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

    if (!m_strMemAddressGlobal.empty() && !m_strMemPortGlobal.empty())
    {
        if (MemcacheClient::CACHE_SUCCESS != (iRet = m_pMemClGlobal->set(strKey.c_str(), strValue.c_str(), strValue.size(), CurrentTimeValue)))
        {
            LOG_ERROR_RLD("Memcache of global set failed, return code is " << iRet << " and key id is " << strKey);
            return false;
        }
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

bool SessionMgr::SetSessionStatus(const std::string &strID, const int iStatus)
{
    std::string strValue;
    if (!MemCacheGet(strID, strValue))
    {
        LOG_ERROR_RLD("Set session status failed beacuse key not found from memcached and key is " << strID);
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(strValue, root, false))
    {
        LOG_ERROR_RLD("Set session status failed beacuse value parsed failed and key is " << strID << " and value is " << strValue);
        return false;
    }

    if (!root.isObject())
    {
        LOG_ERROR_RLD("Set session status failed beacuse json root parsed failed and key is " << strID << " and value is " << strValue);
        return false;
    }

    Json::Value JSid = root["sid"];
    if (JSid.isNull())
    {
        LOG_ERROR_RLD("Set session status failed beacuse json threshold  json value is null and key is " << strID << " and value is " << strValue);
        return false;
    }

    if (!JSid.isArray())
    {
        LOG_ERROR_RLD("Session id info failed, raw data is : " << strValue);
        return false;
    }

    for (unsigned int i = 0; i < JSid.size(); ++i)
    {
        auto jsSidValue = JSid[i];
        if (jsSidValue.isNull() || !jsSidValue.isString())
        {
            LOG_ERROR_RLD("Session id info failed, raw data is : " << strValue);
            return false;
        }

        LOG_INFO_RLD("Set session id is " << jsSidValue.asString() << " and status value is " << iStatus);

        {
            //获取得到设备/用户ID对应的SessionID值
            const std::string &strSid = jsSidValue.asString();

            std::string strValueSid;
            if (!MemCacheGet(strSid, strValueSid))
            {
                LOG_ERROR_RLD("Set session status failed beacuse key not found from memcached and key is " << strSid);
                return false;
            }

            Json::Reader readersid;
            Json::Value rootsid;
            if (!readersid.parse(strValueSid, rootsid, false))
            {
                LOG_ERROR_RLD("Set session status failed beacuse value parsed failed and key is " << strSid << " and value is " << strValueSid);
                return false;
            }

            if (!rootsid.isObject())
            {
                LOG_ERROR_RLD("Set session status failed beacuse json root parsed failed and key is " << strSid << " and value is " << strValueSid);
                return false;
            }

            Json::Value jThreshold = rootsid["threshold"];

            if (jThreshold.isNull())
            {
                LOG_ERROR_RLD("Set session status failed beacuse json threshold  json value is null and key is " << strSid << " and value is " << strValueSid);
                return false;
            }

            rootsid["status"] = iStatus;

            Json::FastWriter fastwriter;
            const std::string &strBody = fastwriter.write(rootsid); //jsBody.toStyledString();

            if (!MemCacheReset(strSid, strBody, jThreshold.asUInt()))
            {
                LOG_ERROR_RLD("Set session status failed becasue memecache failed, id is " << strSid);
                return false;
            }

            LOG_INFO_RLD("Set session status and id is " << strSid << " and value is " << strBody);
        }
    }

    return true;
}

bool SessionMgr::GetTerminalType(const std::string &strID, unsigned int &uiTerminalType)
{
    std::string strValue;
    if (!MemCacheGet(strID, strValue))
    {
        LOG_ERROR_RLD("Get session info failed beacuse key not found from memcached and key is " << strID);
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(strValue, root, false))
    {
        LOG_ERROR_RLD("Get session info failed beacuse value parsed failed and key is " << strID << " and value is " << strValue);
        return false;
    }

    if (!root.isObject())
    {
        LOG_ERROR_RLD("Get session info failed beacuse json root parsed failed and key is " << strID << " and value is " << strValue);
        return false;
    }

    Json::Value jTerm = root["terminaltype"];

    if (jTerm.isNull())
    {
        LOG_ERROR_RLD("Get session info failed beacuse json threshold  json value is null and key is " << strID << " and value is " << strValue);
        return false;
    }

    uiTerminalType = jTerm.asUInt();

    return true;
}

bool SessionMgr::UpdateSidList(const std::string &strID, const std::string &strSessionID)
{
    std::string strValue;
    if (!MemCacheGet(strID, strValue))
    {
        LOG_ERROR_RLD("Update session list failed beacuse key not found from memcached and key is " << strID);
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(strValue, root, false))
    {
        LOG_ERROR_RLD("Update session list failed beacuse value parsed failed and key is " << strID << " and value is " << strValue);
        return false;
    }

    if (!root.isObject())
    {
        LOG_ERROR_RLD("Update session list failed beacuse json root parsed failed and key is " << strID << " and value is " << strValue);
        return false;
    }

    Json::Value JSid = root["sid"];
    if (JSid.isNull())
    {
        LOG_ERROR_RLD("Update session list failed beacuse sid  json value is null and key is " << strID << " and value is " << strValue);
        return false;
    }

    if (!JSid.isArray())
    {
        LOG_ERROR_RLD("Update session list failed, raw data is : " << strValue);
        return false;
    }

    unsigned int uiSize = JSid.size();
    JSid[uiSize] = strSessionID;

    LOG_INFO_RLD("Update session list and session id is " << strSessionID << " and id is " << strID << " and session list size is " << JSid.size());
    
    root["sid"] = JSid;

    Json::Value jThreshold = root["threshold"];

    if (jThreshold.isNull())
    {
        LOG_ERROR_RLD("Update session list failed beacuse json threshold  json value is null and session id is " << strSessionID << " and id is " << strID);
        return false;
    }

    Json::FastWriter fastwriter;
    const std::string &strBody = fastwriter.write(root); //jsBody.toStyledString();

    if (!MemCacheCreate(strID, strBody, jThreshold.asUInt()))
    {
        LOG_ERROR_RLD("Update session list failed because memcache error.");
        return CACHE_ERROR;
    }

    return true;
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
