#ifndef CACHE_MANAGER
#define CACHE_MANAGER

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 创建日期：2015-1-8
// 作    者：尹宾
// 修改日期：
// 修 改 者：
// 修改说明：
// 类 摘 要：缓存管理器
// 详细说明：
// 附加说明：
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <string>
#include <stdio.h>
//#include <map>
#include <unordered_map>
#include <boost/shared_ptr.hpp>
#include <list>
#ifdef _WIN32
#include "../Storage/NetComm.h"
#else
#include "NetComm.h"
#endif

#include "boost/atomic.hpp"

class CacheObj
{
public:
    CacheObj();
    virtual ~CacheObj(){};

    void HitInc();

    boost::uint64_t GetHit();

    void Reset(boost::uint64_t uiIni = 0);

    virtual const std::string &GetKey() = 0;

    void SetPos(std::list<CacheObj*>::iterator itPos);

    std::list<CacheObj*>::iterator GetPos();

    void SetTickSnapshot(const boost::uint64_t uiTickSnapshot);

    boost::uint64_t GetTickSnapshot();

    void SetTimeoutInterval(const boost::uint32_t uiTimeoutInterval);

    boost::uint32_t GetTimeoutInterval();

    void SetResetTickSnapshot(const bool blResetTickSnapshot);

    bool GetResetTickSnapshot();

private:
    boost::uint64_t m_uiHit;

    std::list<CacheObj*>::iterator m_Pos;

    boost::uint64_t m_uiTickSnapshot;
    boost::uint32_t m_uiTimeoutInterval;
    bool m_blResetTickSnapshot;
};

class CacheInterface
{
public:
    CacheInterface()
    {

    };
    virtual ~CacheInterface()
    {

    };

    virtual boost::shared_ptr<CacheObj>  Get(const std::string &strKey) = 0;
    virtual void Set(boost::shared_ptr<CacheObj> pCacheObj) = 0;
    virtual void Remove(const std::string &strKey) = 0;
    virtual void Clear() = 0;
};

class CacheManager : public boost::noncopyable, public CacheInterface
{
public:
    CacheManager(const boost::uint32_t uiType = LRU, const boost::uint32_t uiMaxSize = MAX_CACHEOBJ_NUM,
        const boost::uint32_t uiLFUFreshInterval = 60, const boost::uint32_t uiCacheObjectTimeoutInterval = 30);
    ~CacheManager();

    boost::shared_ptr<CacheObj>  Get(const std::string &strKey);
    void Set(boost::shared_ptr<CacheObj> pCacheObj);
    void Remove(const std::string &strKey);
    void Clear();

    void SetCacheObjectTimeoutInterval(const boost::uint32_t uiCacheObjectTimeoutInterval);

    static const boost::uint32_t LRU = 0;
    static const boost::uint32_t LFU = 1;

    static const boost::uint32_t MAX_CACHEOBJ_NUM = 20480;

private:
    boost::shared_ptr<CacheObj>  GetLFU(const std::string &strKey);
    void SetLFU(boost::shared_ptr<CacheObj> pCacheObj);
    bool SortCache(CacheObj *&t1, CacheObj *&t2);
    void TimerOutHandler(const boost::system::error_code &ec);

    boost::shared_ptr<CacheObj>  GetLRU(const std::string &strKey);
    void SetLRU(boost::shared_ptr<CacheObj> pCacheObj);
    void RemoveInner(const std::string &strKey);

    void TickTimeHandler(const boost::system::error_code &ec);

private:
    typedef std::unordered_map<std::string, boost::shared_ptr<CacheObj> > CacheMap;
    CacheMap m_CacheMap;
    const boost::uint32_t m_uiMaxSize;
    const boost::uint32_t m_uiType;

    std::list<CacheObj*> m_CacheObjList;
    TimeOutHandler m_TimeHr;
    
    boost::shared_mutex m_Mutex;

    boost::atomic_uint64_t m_uiTickNum;
    TimeOutHandler m_TickTimer;

    boost::uint32_t m_uiCacheObjectTimeoutInterval;
};

#endif
