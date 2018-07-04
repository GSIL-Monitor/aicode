#ifndef NETDISK_SESSIONMGR
#define NETDISK_SESSIONMGR

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <string>
#include <boost/shared_ptr.hpp>

class MemcacheClient;

class CacheMgr : public boost::noncopyable
{
public:
    CacheMgr();
    ~CacheMgr();

    void SetMemCacheAddRess(const std::string &strMemAddress, const std::string &strMemPort);

    bool Init();

    bool MemCacheRemove(const std::string &strKey);

    bool MemCacheExist(const std::string &strKey);

    bool MemCacheCreate(const std::string &strKey, const std::string &strValue, const unsigned int uiThreshold);

    bool MemCacheReset(const std::string &strKey, const std::string &strValue, const unsigned int uiThreshold);

    bool MemCacheGet(const std::string &strKey, std::string &strValue);
    
private:

    std::string m_strMemAddress;
    std::string m_strMemPort;

    boost::mutex m_MemcachedMutex;
    MemcacheClient *m_pMemCl;


};


#endif


