#include "CacheManager.h"
#include <boost/scope_exit.hpp>


CacheManager::CacheManager(const boost::uint32_t uiType, const boost::uint32_t uiMaxSize,
    const boost::uint32_t uiLFUFreshInterval) :
    m_uiMaxSize(uiMaxSize), m_uiType(uiType), 
    m_TimeHr(boost::bind(&CacheManager::TimerOutHandler, this, _1), uiLFUFreshInterval)
{
    if (CacheManager::LFU == m_uiType)
    {
        m_TimeHr.Run();
    }
}


CacheManager::~CacheManager()
{
    if (CacheManager::LFU == m_uiType)
    {
        m_TimeHr.Stop();
    }
}

boost::shared_ptr<CacheObj> CacheManager::GetLFU(const std::string &strKey)
{
    m_Mutex.lock();

    BOOST_SCOPE_EXIT(&m_Mutex)
    {
        m_Mutex.unlock();
    }
    BOOST_SCOPE_EXIT_END

    CacheMap::iterator itFind = m_CacheMap.find(strKey);

    boost::shared_ptr<CacheObj> retObj;
    if (m_CacheMap.end() != itFind)
    {
        retObj = itFind->second;
        itFind->second->HitInc();
    }
    
    return retObj;
}

void CacheManager::SetLFU(boost::shared_ptr<CacheObj> pCacheObj)
{
    m_Mutex.lock();

    BOOST_SCOPE_EXIT(&m_Mutex)
    {
        m_Mutex.unlock();
    }
    BOOST_SCOPE_EXIT_END

    CacheMap::iterator itFind = m_CacheMap.find(pCacheObj->GetKey());
    if (m_CacheMap.end() == itFind)
    {
        if (m_uiMaxSize <= m_CacheMap.size())
        {
            m_CacheObjList.sort(boost::bind(&CacheManager::SortCache, this, _1, _2));
            CacheObj *pEraseObj = m_CacheObjList.back();
            m_CacheObjList.pop_back();
            m_CacheMap.erase(pEraseObj->GetKey());
        }

        pCacheObj->HitInc();
        m_CacheMap.insert(CacheMap::value_type(pCacheObj->GetKey(), pCacheObj));
        m_CacheObjList.push_back(pCacheObj.get());

    }
    else
    {
        //update
        itFind->second = pCacheObj;
        itFind->second->HitInc();

    }

}

bool CacheManager::SortCache(CacheObj *&t1, CacheObj *&t2)
{
    if (t1->GetHit() == t2->GetHit())
    {
        return false;
    }
    else
    {
        return !(t1->GetHit() < t2->GetHit());
    }
    
}

boost::shared_ptr<CacheObj> CacheManager::Get(const std::string &strKey)
{
    if (CacheManager::LFU == m_uiType)
    {
        return GetLFU(strKey);
    }
    else if (CacheManager::LRU == m_uiType)
    {
        return GetLRU(strKey);
    }
    else
    {
        return boost::shared_ptr<CacheObj>();
    }
}

void CacheManager::Set(boost::shared_ptr<CacheObj> pCacheObj)
{
    if (CacheManager::LFU == m_uiType)
    {
        SetLFU(pCacheObj);
    }
    else if (CacheManager::LRU == m_uiType)
    {
        SetLRU(pCacheObj);
    }
    else
    {
        
    }
}

void CacheManager::Remove(const std::string &strKey)
{
    if (CacheManager::LFU == m_uiType)
    {
        m_Mutex.lock();

        BOOST_SCOPE_EXIT(&m_Mutex)
        {
            m_Mutex.unlock();
        }
        BOOST_SCOPE_EXIT_END

        RemoveInner(strKey);
        return;
    }
    RemoveInner(strKey);    
}

void CacheManager::Clear()
{
    if (CacheManager::LFU == m_uiType)
    {
        m_Mutex.lock();

        BOOST_SCOPE_EXIT(&m_Mutex)
        {
            m_Mutex.unlock();
        }
        BOOST_SCOPE_EXIT_END

        m_CacheMap.clear();
        m_CacheObjList.clear();
        return;
    }
    
    m_CacheMap.clear();
    m_CacheObjList.clear();
}

void CacheManager::RemoveInner(const std::string &strKey)
{
    CacheMap::iterator itFind = m_CacheMap.find(strKey);

    if (m_CacheMap.end() != itFind)
    {
        m_CacheObjList.erase(itFind->second->GetPos());
        m_CacheMap.erase(itFind);

    }
}

void CacheManager::TimerOutHandler(const boost::system::error_code &ec)
{
    if (CacheManager::LFU != m_uiType)
    {
        return;
    }

    m_Mutex.lock();

    BOOST_SCOPE_EXIT(&m_Mutex)
    {
        m_Mutex.unlock();
    }
    BOOST_SCOPE_EXIT_END

    std::list<CacheObj*>::iterator itBegin = m_CacheObjList.begin();
    std::list<CacheObj*>::iterator itEnd = m_CacheObjList.end();
    while (itBegin != itEnd)
    {
        boost::uint64_t uiTmp =(*itBegin)->GetHit();
        uiTmp = (boost::uint64_t)(uiTmp * 0.5);
        if (0 == uiTmp)
        {
            uiTmp = 1;
        }
        (*itBegin)->Reset(uiTmp);

        ++itBegin;
    }
}

boost::shared_ptr<CacheObj> CacheManager::GetLRU(const std::string &strKey)
{
    CacheMap::iterator itFind = m_CacheMap.find(strKey);

    boost::shared_ptr<CacheObj> retObj;
    if (m_CacheMap.end() != itFind)
    {
        retObj = itFind->second;
        
        m_CacheObjList.splice(m_CacheObjList.begin(), m_CacheObjList, itFind->second->GetPos());
        itFind->second->SetPos(m_CacheObjList.begin());
    }

    return retObj;
}

void CacheManager::SetLRU(boost::shared_ptr<CacheObj> pCacheObj)
{
    CacheMap::iterator itFind = m_CacheMap.find(pCacheObj->GetKey());
    if (m_CacheMap.end() == itFind)
    {
        if (m_uiMaxSize <= m_CacheMap.size())
        {
            m_CacheMap.erase(m_CacheObjList.back()->GetKey());
            m_CacheObjList.pop_back();
        }

        m_CacheObjList.push_front(pCacheObj.get());
        pCacheObj->SetPos(m_CacheObjList.begin());
        m_CacheMap.insert(CacheMap::value_type(pCacheObj->GetKey(), pCacheObj));
                
    }
    else
    {
        //m_CacheObjList.splice(m_CacheObjList.begin(), m_CacheObjList, itFind->second->GetPos());
        m_CacheObjList.erase(itFind->second->GetPos());
        m_CacheObjList.push_front(pCacheObj.get());
        itFind->second = pCacheObj;
        itFind->second->SetPos(m_CacheObjList.begin());

    }
}




CacheObj::CacheObj() : m_uiHit(0)
{

}

void CacheObj::HitInc()
{
    ++m_uiHit;
}

boost::uint64_t CacheObj::GetHit()
{
    return m_uiHit;
}

void CacheObj::Reset(boost::uint64_t uiIni)
{
    m_uiHit = uiIni;
}

void CacheObj::SetPos(std::list<CacheObj*>::iterator itPos)
{
    m_Pos = itPos;
}

std::list<CacheObj*>::iterator CacheObj::GetPos()
{
    return m_Pos;
}
