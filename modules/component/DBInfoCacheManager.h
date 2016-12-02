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

    bool QuerySql(const std::string &strQuerySql, std::list<boost::any> &ResultList, const bool IsForceFromDB = false);

    void SetSqlCB(DBSqlHandlerCB dcb);

private:
    void SqlHanler(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, std::list<boost::any> *pReslutList);

private:
    struct DBInfoObj : public CacheObj
    {
        DBInfoObj(const std::string &strKey) : m_strKey(strKey){};
        virtual ~DBInfoObj(){};

        virtual const std::string &GetKey()
        {
            return m_strKey;
        };

        std::string m_strKey;
        boost::shared_ptr<std::list<boost::any> > m_Content;
    };

    boost::shared_ptr<CacheInterface> m_pCacheContainer;

    DBSqlHandlerCB m_DBSqlHandlerCB;

    MysqlImpl *m_pMysql;

};


#endif

