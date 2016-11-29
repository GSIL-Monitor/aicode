#include "DBInfoCacheManager.h"
#include "intf.h"
#include "LogRLD.h"

DBInfoCacheManager::DBInfoCacheManager() : m_pCacheContainer(new CacheManager())
{
}


DBInfoCacheManager::~DBInfoCacheManager()
{
}

bool DBInfoCacheManager::QuerySql(const std::string &strQuerySql, std::list<boost::any> &ResultList, const bool IsForceFromDB)
{
    if (!IsForceFromDB)
    {
        boost::shared_ptr<CacheObj> pObj = m_pCacheContainer->Get(strQuerySql);
        if (NULL != pObj.get())
        {
            DBInfoObj *pDBObj = (DBInfoObj*)pObj.get();

            auto itBegin = pDBObj->m_Content->begin();
            auto itEnd = pDBObj->m_Content->end();
            while (itBegin != itEnd)
            {
                ResultList.push_back(*itBegin);
                ++itBegin;
            }

            return true;
        }
    }

    boost::shared_ptr<std::list<boost::any> > pResultList(new std::list<boost::any>);
    if (!DataAccessInstance::instance().QuerySql(strQuerySql, boost::bind(&DBInfoCacheManager::SqlHanler, this, _1, _2, _3, pResultList.get())))
    {
        LOG_ERROR_RLD("DBInfoCacheManager query file sql exec failed, sql is " << strQuerySql);
        return false;
    }

    //m_pCacheContainer->Remove(strQuerySql);

    DBInfoObj *pDBObj = new DBInfoObj(strQuerySql);
    boost::shared_ptr<CacheObj> pObj(pDBObj);
    pDBObj->m_Content = pResultList;
    m_pCacheContainer->Set(pObj);

    //ResultList.swap(*pResultList);

    auto itBegin = pResultList->begin();
    auto itEnd = pResultList->end();
    while (itBegin != itEnd)
    {
        ResultList.push_back(*itBegin);
        ++itBegin;
    }

    LOG_INFO_RLD("DBInfoCacheManager query result size is " << ResultList.size());

    return true;
}

void DBInfoCacheManager::SetSqlCB(DBSqlHandlerCB dcb)
{
    m_DBSqlHandlerCB = dcb;
}

void DBInfoCacheManager::SqlHanler(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, std::list<boost::any> *pReslutList)
{
    if (NULL == pReslutList)
    {
        LOG_ERROR_RLD("DBInfoCacheManager sql handler reslut list is null.");
        return;
    }

    if (NULL == m_DBSqlHandlerCB)
    {
        LOG_ERROR_RLD("DBInfoCacheManager sql handler call back func is null.");
        return;
    }

    boost::any result;
    m_DBSqlHandlerCB(uiRowNum, uiColumnNum, strColumn, result);
    if (result.empty())
    {
        LOG_ERROR_RLD("DBInfoCacheManager sql handler call back reslut is empty.");
        return;
    }

    pReslutList->push_back(result);

    LOG_INFO_RLD("DBInfoCacheManager sql hander result : " << "row num:" << uiRowNum << ", column num:" << uiColumnNum << ", column:" << strColumn);
}
