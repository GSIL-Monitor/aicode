#include "LogRLD.h"
#include "boost/lexical_cast.hpp"
#include "mysql_impl.h"

#include "P2PServerManager_LT.h"

boost::mutex P2PServerManager_LT::m_p2pMutex;

P2PServerManager_LT::P2PServerManager_LT(const string &strSupplier)
{
    m_sFlag = strSupplier;
    m_table_name = "t_p2pid_lt";
}

P2PServerManager_LT::~P2PServerManager_LT(void)
{
}

bool P2PServerManager_LT::ParseConnectParams(string sconnectparams)
{
    m_p2pConnectParams.sInitstring = sconnectparams;
    return true;
}

bool P2PServerManager_LT::DeviceRequestP2PConnectParam(P2PConnectParam &p2pparams, string sDeviceID, string sIP, string sUserID /*= ""*/)
{
    p2pparams.sP2Pid.clear();

    if (!GetP2pID(sDeviceID, p2pparams))
    {
        LOG_ERROR_RLD("DeviceRequestP2PConnectParam failed, get p2p id error, device id is " << sDeviceID);
        return false;
    }

    if (!p2pparams.sP2Pid.empty())
    {
        LOG_INFO_RLD("DeviceRequestP2PConnectParam successful, userid is " << sUserID <<
            " and device id is " << sDeviceID << " and p2p id is " << p2pparams.sP2Pid);
        return true;
    }

    if (!sUserID.empty())
    {
        LOG_ERROR_RLD("DeviceRequestP2PConnectParam failed, get p2p id error, userid is " << sUserID <<
            " and device id is " << sDeviceID);
        return false;
    }

    boost::unique_lock<boost::mutex> lock(m_p2pMutex);
    if (!AllocateP2pID(sDeviceID, p2pparams))
    {
        LOG_ERROR_RLD("DeviceRequestP2PConnectParam failed, allocate p2p id error, userid is " << sUserID <<
            " and device id is " << sDeviceID);
        return false;
    }

    return true;
}

bool P2PServerManager_LT::GetP2pID(const string &strDeviceID, P2PConnectParam &p2pparams)
{
    if (NULL == m_pDBCache)
    {
        LOG_ERROR_RLD("DBCache is null.");
        return false;
    }

    char sql[1024] = { 0 };
    const char* sqlfmt = "select p2pid, validity_period from %s where deviceid='%s' and supplier='%s'";
    snprintf(sql, sizeof(sql), sqlfmt, m_table_name.c_str(), strDeviceID.c_str(), m_sFlag.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        switch (uiColumnNum)
        {
        case 0:
            p2pparams.sP2Pid = strColumn;
            break;
        case 1:
            p2pparams.nTime = boost::lexical_cast<int>(strColumn);
            Result = p2pparams;
            break;

        default:
            LOG_ERROR_RLD("GetP2pID sql cb error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_pDBCache->QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("GetP2pID sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("GetP2pID info not found, sql is " << sql);
        return true;
    }

    auto ResultInfo = boost::any_cast<P2PConnectParam>(ResultList.front());
    p2pparams.sP2Pid = ResultInfo.sP2Pid;
    p2pparams.nTime = ResultInfo.nTime;

    return true;
}

bool P2PServerManager_LT::AllocateP2pID(const string &strDeviceID, P2PConnectParam &p2pparams)
{
    if (!UpdateP2PID(strDeviceID))
    {
        LOG_ERROR_RLD("AllocateP2pID failed, update p2p id error, device id is " << strDeviceID.c_str());
        return false;
    }

    if (NULL == m_pDBCache)
    {
        LOG_ERROR_RLD("DBCache is null.");
        return false;
    }

    return GetP2pID(strDeviceID, p2pparams);
}

bool P2PServerManager_LT::UpdateP2PID(const string &strDeviceID)
{
    if (NULL == m_pMysql)
    {
        LOG_ERROR_RLD("Mysql impl is null.");
        return false;
    }

    char sql[1024] = { 0 };
    const char* sqlfmt = "update %s set deviceid = '%s' where deviceid is null and supplier = '%s' limit 1";
    snprintf(sql, sizeof(sql), sqlfmt, m_table_name.c_str(), strDeviceID.c_str(), m_sFlag.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("UpdateP2PID sql exec failed, sql is " << sql);
        return false;
    }

    return true;
}

