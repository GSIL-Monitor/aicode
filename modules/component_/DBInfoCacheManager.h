#ifndef DBINFO_CACHE_MANAGER
#define DBINFO_CACHE_MANAGER

#include "CacheManager.h"
#include "NetComm.h"
#include <list>
#include "boost/any.hpp"

class MysqlImpl;

class DBInfoCacheManager : public boost::noncopyable
{
public:
    DBInfoCacheManager(MysqlImpl *pMysql = NULL);
    ~DBInfoCacheManager();

    typedef boost::function<void(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)> DBSqlHandlerCB;

    bool QuerySql(const std::string &strQuerySql, std::list<boost::any> &ResultList, DBSqlHandlerCB scb = NULL, const bool IsForceFromDB = false);

    void SetSqlCB(DBSqlHandlerCB dcb);
    
    /////////////////////////////////////////////////////////

    bool GetResult(const std::string &strQuerySql, std::list<boost::any> &ResultList);
    void SetResult(const std::string &strQuerySql, boost::shared_ptr<std::list<boost::any> > pResultList);

private:
    void SqlHanler(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, 
        DBSqlHandlerCB scb, std::list<boost::any> *pReslutList);
        
    inline boost::shared_ptr<CacheObj> Get(const std::string &strKey)
    {
        boost::unique_lock<boost::shared_mutex> lock(m_CacheMutex);
        return m_pCacheContainer->Get(strKey);
    }

    inline void Set(boost::shared_ptr<CacheObj> pCacheObj)
    {
        boost::unique_lock<boost::shared_mutex> lock(m_CacheMutex);
        m_pCacheContainer->Set(pCacheObj);
    }

private:
    struct DBInfoObj : public CacheObj
    {
        DBInfoObj(const std::string &strKey, boost::shared_ptr<std::list<boost::any> > pContent) : m_strKey(strKey), m_pContent(pContent){};
        virtual ~DBInfoObj(){};

        virtual const std::string &GetKey()
        {
            return m_strKey;
        };

        std::string m_strKey;
        boost::shared_ptr<std::list<boost::any> > m_pContent;
    };

    boost::shared_mutex m_CacheMutex; //¶ÁÐ´Ëø
    boost::shared_ptr<CacheInterface> m_pCacheContainer;

    DBSqlHandlerCB m_DBSqlHandlerCB;

    MysqlImpl *m_pMysql;

};


#endif

