#include "P2PServerManager.h"
#include "LogRLD.h"
#include "boost/any.hpp"
#include "boost/lexical_cast.hpp"
#include "mysql_impl.h"

boost::mutex P2PServerManager::m_smutex;

P2PServerManager::P2PServerManager(void)
{
    m_sFlag = "";
}

P2PServerManager::~P2PServerManager(void)
{
}

//
bool P2PServerManager::DeviceRequestP2PConnectParam(P2PConnectParam &p2pparams, string sDeviceID, string sIP, string sUserID)
{
    bool bRet = false;
    TimeZone timezone;
    if (sUserID == "")
    {
        //查找该IP所属区域
        bRet = m_timezone.GetCountryTime(sIP, timezone);

        //给该设备分配一个属于该集群的p2pid
        if (bRet)
        {
            bRet = AllocP2PID(timezone, sDeviceID);
        }
    }
    else
    {
        bRet = GetP2PID(sDeviceID, true, timezone);
    }
    

    //查找数据库表，找出与该区域对应的P2P服务器信息
    P2PServerInfo p2pserver;
    if (bRet)
    {
        bRet = GetP2PServerFromTimezone(timezone, p2pserver);
    }

    //解析p2p服务器连接参数
    if (bRet)
    {
        bRet = ParseConnectParams(p2pserver.connectparams);
    }
    
    if (bRet)
    {
        p2pparams = m_p2pConnectParams;
    }

    return bRet;
}

bool P2PServerManager::AllocP2PID( TimeZone timezone, string sDevID )
{
    //查询数据表m_table_name，若设备已关联p2pid，则直接获取，否则分配空闲的id
    bool bRet = false;
    bRet = GetP2pIDByDevID(sDevID, m_p2pConnectParams);
    if (bRet == false)
    {
        P2PServerManager::m_smutex.lock();
        bRet = GetFreeP2pID(timezone.sCode, m_table_name, m_p2pConnectParams);
        if (bRet)
        {
            bRet = UpdateP2PID(m_p2pConnectParams.sP2Pid, sDevID);
        }
        P2PServerManager::m_smutex.unlock();
    }
    return bRet;
}

bool P2PServerManager::ReleaseP2PID( string sDevID )
{
    bool bRet = true;
    //
    return bRet;
}

bool P2PServerManager::GetP2PID( string sDevID , bool bGetTimeZone, TimeZone &timezone)
{
    //查询数据表m_table_name，找出与sDevID相关联的p2pid
    bool bRet = true;
    m_p2pConnectParams.nTime = 180;
    bRet = GetP2pIDByDevID(sDevID, m_p2pConnectParams);
    if (bGetTimeZone)
    {
        bRet = m_timezone.GetCountryInfoByDevID(sDevID, timezone);
    }
    return bRet;
}

bool P2PServerManager::GetP2PServerFromTimezone( TimeZone countryTime, P2PServerInfo &p2pServerInfo)
{
    if (NULL == m_pDBCache)
    {
        LOG_ERROR_RLD("DBCache is null.");
        return false;
    }

    char sql[1024] = { 0 };
    const char* sqlfmt = "select cluster, connectparams from t_p2pserver_info where countrycode='%s' and flag='%s' limit 1";
    snprintf(sql, sizeof(sql), sqlfmt, countryTime.sCode.c_str(), m_sFlag.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        switch (uiColumnNum)
        {
        case 0:
            p2pServerInfo.cluster = strColumn;
            break;
        case 1:
            p2pServerInfo.connectparams = strColumn;
            Result = p2pServerInfo;
            break;        
        default:
            LOG_ERROR_RLD("Unknown sql cb error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }

    };

    std::list<boost::any> ResultList;
    if (!m_pDBCache->QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("GetClusterInfo sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("GetClusterInfo info not found, sql is " << sql);
        return false;
    }

    auto ResultInfo = boost::any_cast<P2PServerInfo>(ResultList.front());
    p2pServerInfo.cluster = ResultInfo.cluster;
    p2pServerInfo.connectparams = ResultInfo.connectparams;

    return true;
}


bool P2PServerManager::GetP2pIDByDevID(const std::string &strDevID, P2PConnectParam &p2pparams)
{
    if (NULL == m_pDBCache)
    {
        LOG_ERROR_RLD("DBCache is null.");
        return false;
    }

    char sql[1024] = { 0 };
    const char* sqlfmt = "select p2pid, validity_period, param1, param2 from %s where deviceid='%s'";
    snprintf(sql, sizeof(sql), sqlfmt, m_table_name.c_str(), strDevID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        switch (uiColumnNum)
        {
        case 0:
            p2pparams.sP2Pid = strColumn;
            break;
        case 1:
            p2pparams.nTime = boost::lexical_cast<int>(strColumn);
            break;
        case 2:
            p2pparams.sparam1 = strColumn;
			break;
		case 3:
			p2pparams.sparam2 = strColumn;
            Result = p2pparams;
            break;
        default:
            LOG_ERROR_RLD("Unknown sql cb error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }

    };

    std::list<boost::any> ResultList;
    if (!m_pDBCache->QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("GetP2pIDByDevID sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("GetP2pIDByDevID info not found, sql is " << sql);
        return false;
    }

    auto ResultInfo = boost::any_cast<P2PConnectParam>(ResultList.front());
    p2pparams.sP2Pid = ResultInfo.sP2Pid;
    p2pparams.nTime = ResultInfo.nTime;
    p2pparams.sparam1 = ResultInfo.sparam1;
    
    return true;
}

bool P2PServerManager::GetFreeP2pID(const std::string &strCnCode, const std::string &strP2pIDTableName, P2PConnectParam &p2pparams)
{
    if (NULL == m_pDBCache)
    {
        LOG_ERROR_RLD("DBCache is null.");
        return false;
    }

    char sql[1024] = { 0 };
    const char* sqlfmt = "select p2pid, validity_period, param1, param2 from %s where status=0 and cluster = (select cluster from t_p2pserver_info where status=0 and countrycode = '%s' and flag='%s') and deviceid is null limit 1";
    snprintf(sql, sizeof(sql), sqlfmt, strP2pIDTableName.c_str(), strCnCode.c_str(), m_sFlag.c_str());

    bool bRet = false;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)
    {
        switch (uiColumnNum)
        {
        case 0:
            p2pparams.sP2Pid = strColumn;
            break;
        case 1:
            p2pparams.nTime = boost::lexical_cast<int>(strColumn);
            break;
        case 2:
            p2pparams.sparam1 = strColumn;
            //Result = p2pparams;
			break;
		case 3:
			p2pparams.sparam2 = strColumn;
            bRet = true;
            break;
        default:
            LOG_ERROR_RLD("Unknown sql cb error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }

    };

    if (!m_pMysql->QueryExec(std::string(sql), SqlFunc))
    {
        LOG_ERROR_RLD("GetFreeP2pID sql failed, sql is " << sql);
        return false;
    }

    return bRet;
}

bool P2PServerManager::UpdateP2PID(const std::string &p2pid, const std::string &strDeviceID)
{
    if (NULL == m_pDBCache)
    {
        LOG_ERROR_RLD("DBCache is null.");
        return false;
    }

    char sql[1024] = { 0 };
    const char* sqlfmt = "update %s set deviceid = '%s' where p2pid = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, m_table_name.c_str(), strDeviceID.c_str(), p2pid.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("UpdateP2PID sql exec failed, sql is " << sql);
        return false;
    }

    return true;
}

void P2PServerManager::SetUrl( string sUrl )
{
    m_timezone.setpostUrl(sUrl);
}

void P2PServerManager::SetDBManager(DBInfoCacheManager *pDBCache, MysqlImpl *pMysql)
{
    m_pDBCache = pDBCache;
    m_pMysql = pMysql;
    m_timezone.SetDBManager(pDBCache, pMysql);
}


