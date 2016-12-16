#include "DBInfoCacheManager.h"
#include "intf.h"
#include "LogRLD.h"
#include "mysql_impl.h"

DBInfoCacheManager::DBInfoCacheManager(MysqlImpl *pMysql) : m_pCacheContainer(new CacheManager()), m_pMysql(pMysql)
{
}


DBInfoCacheManager::~DBInfoCacheManager()
{
}

bool DBInfoCacheManager::QuerySql(const std::string &strQuerySql, std::list<boost::any> &ResultList, DBSqlHandlerCB scb, const bool IsForceFromDB)
{
    if (!IsForceFromDB)
    {
        if (GetResult(strQuerySql, ResultList))
        {
            LOG_INFO_RLD("DBInfoCacheManager get result from cache and sql is " << strQuerySql);
            return true;
        }

    }

    boost::shared_ptr<std::list<boost::any> > pResultList(new std::list<boost::any>);
    
    if (NULL == m_pMysql)
    {
        //优先使用参数中callback值
        if (!DataAccessInstance::instance().QuerySql(strQuerySql, boost::bind(&DBInfoCacheManager::SqlHanler, this, _1, _2, _3,
            (NULL == scb ? m_DBSqlHandlerCB : scb),
            pResultList.get())))
        {
            LOG_ERROR_RLD("DBInfoCacheManager query file sql exec failed, sql is " << strQuerySql);
            return false;
        }
    }
    else
    {
        if (!m_pMysql->QueryExec(strQuerySql, boost::bind(&DBInfoCacheManager::SqlHanler, this, _1, _2, _3,
            (NULL == scb ? m_DBSqlHandlerCB : scb),
            pResultList.get())))
        {
            LOG_ERROR_RLD("DBInfoCacheManager query file sql exec failed, sql is " << strQuerySql);
            return false;
        }
    }    

    auto itBegin = pResultList->begin();
    auto itEnd = pResultList->end();
    while (itBegin != itEnd)
    {
        ResultList.push_back(*itBegin);
        ++itBegin;
    }

    SetResult(strQuerySql, pResultList);
        
    LOG_INFO_RLD("DBInfoCacheManager query result size is " << ResultList.size());

    return true;
}

void DBInfoCacheManager::SetSqlCB(DBSqlHandlerCB dcb)
{
    m_DBSqlHandlerCB = dcb;
}

bool DBInfoCacheManager::GetResult(const std::string &strQuerySql, std::list<boost::any> &ResultList)
{
    boost::shared_ptr<CacheObj> pObj = Get(strQuerySql);
    if (NULL != pObj.get())
    {
        DBInfoObj *pDBObj = (DBInfoObj*)pObj.get();

        auto itBegin = pDBObj->m_pContent->begin();
        auto itEnd = pDBObj->m_pContent->end();
        while (itBegin != itEnd)
        {
            ResultList.push_back(*itBegin);
            ++itBegin;
        }

        return true;
    }

    return false;
}

void DBInfoCacheManager::SetResult(const std::string &strQuerySql, boost::shared_ptr<std::list<boost::any> > pResultList)
{
    boost::shared_ptr<CacheObj> pObj(new DBInfoObj(strQuerySql, pResultList));
    Set(pObj);
}

void DBInfoCacheManager::SqlHanler(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, 
    DBSqlHandlerCB scb, std::list<boost::any> *pReslutList)
{
    if (NULL == pReslutList)
    {
        LOG_ERROR_RLD("DBInfoCacheManager sql handler reslut list is null.");
        return;
    }

    if (NULL == scb)
    {
        LOG_ERROR_RLD("DBInfoCacheManager sql handler call back func is null.");
        return;
    }

    boost::any result;
    scb(uiRowNum, uiColumnNum, strColumn, result);
    if (result.empty())
    {
        LOG_INFO_RLD("DBInfoCacheManager sql handler call back reslut is empty.");
    }
    else
    {
        pReslutList->push_back(result);
    }

    LOG_INFO_RLD("DBInfoCacheManager sql hander result : " << "row num:" << uiRowNum << ", column num:" << uiColumnNum << ", column:" << strColumn);
}
