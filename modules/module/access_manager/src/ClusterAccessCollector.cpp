#include <boost/scope_exit.hpp>

#include "ReturnCode.h"
#include "SessionMgr.h"
#include "mysql_impl.h"
#include "DBInfoCacheManager.h"
#include "LogRLD.h"
#include "ClusterAccessCollector.h"

ClusterAccessCollector::ClusterAccessCollector(SessionMgr *pSessionMgr, MysqlImpl *pMysqlImpl, DBInfoCacheManager *pDBInfo) :
m_pSessionMgr(pSessionMgr), m_pMysqlImpl(pMysqlImpl), m_pDBInfo(pDBInfo)
{

}

ClusterAccessCollector::~ClusterAccessCollector()
{

}

void ClusterAccessCollector::AddDeviceAccessRecord(const std::string &strAccessID, const std::string &strDeviceID,
    const unsigned int uiDeviceType, const std::string &strLoginTime, const std::string &strLogoutTime)
{
    std::string strDeviceName;
    if (!QueryDeviceInfo(strDeviceID, strDeviceName))
    {
        LOG_ERROR_RLD("AddDeviceAccessRecord failed, device info not found, device id is " << strDeviceID);
        return;
    }

    InteractiveProtoHandler::DeviceAccessRecord accessedDevice;
    accessedDevice.m_strAccessID = strAccessID;
    accessedDevice.m_strClusterID = "";
    accessedDevice.m_strDeviceID = strDeviceID;
    accessedDevice.m_strDeviceName = strDeviceName;
    accessedDevice.m_uiDeviceType = uiDeviceType;
    accessedDevice.m_strLoginTime = strLoginTime;
    accessedDevice.m_strLogoutTime = strLogoutTime;
    accessedDevice.m_uiOnlineDuration = 0;
    accessedDevice.m_strCreateDate = "";
    accessedDevice.m_uiStatus = 0;

    PushDeviceAccessRecord(accessedDevice);

    LOG_INFO_RLD("AddDeviceAccessRecord completed, access id is " << strAccessID <<
        " and device id is " << strDeviceID <<
        " and device name is " << strDeviceName <<
        " and device type is " << uiDeviceType <<
        " and login time is " << strLoginTime <<
        " and logout time is " << strLogoutTime);
}

void ClusterAccessCollector::AddUserAccessRecord(const std::string &strAccessID, const std::string &strUserID,
    const unsigned int uiTerminalType, const std::string &strLoginTime, const std::string &strLogoutTime)
{
    std::string strUserName;
    std::string strAliasname;
    if (!QueryUserInfo(strUserID, strUserName, strAliasname))
    {
        LOG_ERROR_RLD("AddUserAccessRecord failed, user info not found, user id is " << strUserID);
        return;
    }

    InteractiveProtoHandler::UserAccessRecord accessedUser;
    accessedUser.m_strAccessID = strAccessID;
    accessedUser.m_strClusterID = "";
    accessedUser.m_strUserID = strUserID;
    accessedUser.m_strUserName = strUserName;
    accessedUser.m_strUserAliasname = strAliasname;
    accessedUser.m_uiClientType = uiTerminalType;
    accessedUser.m_strLoginTime = strLoginTime;
    accessedUser.m_strLogoutTime = strLogoutTime;
    accessedUser.m_uiOnlineDuration = 0;
    accessedUser.m_strCreateDate = "";
    accessedUser.m_uiStatus = 0;

    PushUserAccessRecord(accessedUser);

    LOG_INFO_RLD("AddUserAccessRecord completed, access id is " << strAccessID <<
        " and user id is " << strUserID <<
        " and user name is " << strUserName <<
        " and user aliasname is " << strAliasname <<
        " and terminal type is " << uiTerminalType <<
        " and login time is " << strLoginTime <<
        " and logout time is " << strLogoutTime);
}

bool ClusterAccessCollector::GetDeviceAccessRecord(std::list<InteractiveProtoHandler::DeviceAccessRecord> &deviceAccessRecordList,
    const unsigned int uiBeginIndex, const unsigned int uiPageSize /*= 10*/)
{
    m_deviceAccessDequeMutex.lock();
    unsigned int size = m_deviceAccessRecordMappingDeque.size();
    if (uiBeginIndex >= size)
    {
        LOG_ERROR_RLD("GetDeviceAccessRecord failed, begin index is out of range, index is " << uiBeginIndex << " and deque size is " << size);

        m_deviceAccessDequeMutex.unlock();
        return false;
    }

    LOG_INFO_RLD("GetDeviceAccessRecord request begin index is " << uiBeginIndex << " and current deque size is" << size);

    unsigned int uiEndIndex = uiBeginIndex + uiPageSize > size ? size : uiBeginIndex + uiPageSize;

    for (unsigned int i = uiBeginIndex; i < uiEndIndex; ++i)
    {
        auto &deviceAccessRecordMapping = m_deviceAccessRecordMappingDeque[i];
        auto itPos = m_deviceAccessRecordMap.find(deviceAccessRecordMapping.strAccessID);
        if (itPos == m_deviceAccessRecordMap.end())
        {
            LOG_ERROR_RLD("GetDeviceAccessRecord error, device access record not found, access id is " << deviceAccessRecordMapping.strAccessID);
            continue;
        }

        auto &deviceAccessRecord = itPos->second;

        boost::posix_time::ptime lasttime;
        if (deviceAccessRecord.m_strLogoutTime.empty())
        {
            lasttime = boost::posix_time::second_clock::local_time();

            //如果会话ID已在MemoryCache中被清空，则认为该设备已下线
            if (!m_pSessionMgr->Exist(itPos->first))
            {
                LOG_INFO_RLD("GetDeviceAccessRecord check session id exist result is false");
                deviceAccessRecord.m_strLogoutTime = boost::posix_time::to_iso_extended_string(lasttime);
            }
        }
        else
        {
            lasttime = boost::posix_time::time_from_string(deviceAccessRecord.m_strLogoutTime);
        }

        deviceAccessRecord.m_uiOnlineDuration = (lasttime - boost::posix_time::time_from_string(deviceAccessRecord.m_strLoginTime)).total_seconds();

        deviceAccessRecordList.push_back(itPos->second);
    }

    m_deviceAccessDequeMutex.unlock();

    return true;
}

bool ClusterAccessCollector::GetUserAccessRecord(std::list<InteractiveProtoHandler::UserAccessRecord> &userAccessRecordList,
    const unsigned int uiBeginIndex, const unsigned int uiPageSize /*= 10*/)
{
    m_userAccessDequeMutex.lock();
    unsigned int size = m_userAccessRecordMappingDeque.size();
    if (uiBeginIndex >= size)
    {
        LOG_ERROR_RLD("GetUserAccessRecord failed, begin index is out of range, index is " << uiBeginIndex << " and deque size is " << size);

        m_userAccessDequeMutex.unlock();
        return false;
    }

    LOG_INFO_RLD("GetUserAccessRecord request begin index is " << uiBeginIndex << " and current deque size is" << size);

    unsigned int uiEndIndex = uiBeginIndex + uiPageSize > size ? size : uiBeginIndex + uiPageSize;

    for (unsigned int i = uiBeginIndex; i < uiEndIndex; ++i)
    {
        auto &userAccessRecordMapping = m_userAccessRecordMappingDeque[i];
        auto itPos = m_userAccessRecordMap.find(userAccessRecordMapping.strAccessID);
        if (itPos == m_userAccessRecordMap.end())
        {
            LOG_ERROR_RLD("GetUserAccessRecord error, user access record not found, access id is " << userAccessRecordMapping.strAccessID);
            continue;
        }

        auto &userAccessRecord = itPos->second;

        boost::posix_time::ptime lasttime;
        if (userAccessRecord.m_strLogoutTime.empty())
        {
            lasttime = boost::posix_time::second_clock::local_time();

            //如果会话ID已在MemoryCache中被清空，则认为该用户已下线
            if (!m_pSessionMgr->Exist(itPos->first))
            {
                LOG_INFO_RLD("GetUserAccessRecord check session id exist result is false");
                userAccessRecord.m_strLogoutTime = boost::posix_time::to_iso_extended_string(lasttime);
            }
        }
        else
        {
            lasttime = boost::posix_time::time_from_string(userAccessRecord.m_strLogoutTime);
        }

        userAccessRecord.m_uiOnlineDuration = (lasttime - boost::posix_time::time_from_string(userAccessRecord.m_strLoginTime)).total_seconds();

        userAccessRecordList.push_back(itPos->second);
    }

    m_userAccessDequeMutex.unlock();

    return true;
}

unsigned int ClusterAccessCollector::DeviceAccessRecordSize()
{
    m_deviceAccessDequeMutex.lock();
    unsigned int size = m_deviceAccessRecordMappingDeque.size();

    m_deviceAccessDequeMutex.unlock();

    return size;
}

unsigned int ClusterAccessCollector::UserAccessRecordSize()
{
    m_userAccessDequeMutex.lock();
    unsigned int size = m_userAccessRecordMappingDeque.size();

    m_userAccessDequeMutex.unlock();

    return size;
}

bool ClusterAccessCollector::QueryDeviceInfo(const std::string &strDeviceID, std::string &strDeviceName)
{
    if (strDeviceID.empty())
    {
        LOG_INFO_RLD("QueryDeviceInfo completed, device id is empty");
        return true;
    }

    char sql[256] = { 0 };
    const char *sqlfmt = "select devicename from t_device_info where deviceid = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strDeviceID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        strDeviceName = strColumn;
        result = strDeviceName;
        LOG_INFO_RLD("QueryDeviceInfo sql device name is " << strDeviceName);
    };

    std::list<boost::any> ResultList;
    if (!m_pDBInfo->QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("QueryDeviceInfo exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryDeviceInfo sql result is empty, sql is " << sql);
        return false;
    }

    strDeviceName = boost::any_cast<std::string>(ResultList.front());

    return true;
}

bool ClusterAccessCollector::QueryUserInfo(const std::string &strUserID, std::string &strUserName, std::string &strAliasName)
{
    if (strUserID.empty())
    {
        LOG_INFO_RLD("QueryUserInfo completed, user id is empty");
        return true;
    }

    char sql[256] = { 0 };
    const char *sqlfmt = "select username, aliasname from t_user_info where userid = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            strUserName = strColumn;
            break;
        case 1:
            strAliasName = strColumn;
            break;

        default:
            break;
            LOG_ERROR_RLD("QueryUserInfo sql callback error, row num is " <<
                uiRowNum << " and column num is " << uiColumnNum << " and value is " << strColumn);
        }

        result = strColumn;
    };

    std::list<boost::any> ResultList;
    if (!m_pDBInfo->QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("QueryUserInfo exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryUserInfo sql result is empty, sql is " << sql);
        return false;
    }

    strUserName = boost::any_cast<std::string>(ResultList.front());
    strAliasName = boost::any_cast<std::string>(ResultList.back());

    return true;
}

void ClusterAccessCollector::PushDeviceAccessRecord(const InteractiveProtoHandler::DeviceAccessRecord &accessedDevice)
{
    m_deviceAccessDequeMutex.lock();
    if (MAX_USER_ACCESS_RECORD_SIZE <= m_deviceAccessRecordMappingDeque.size())
    {
        //如果队列已存储的元素个数达到定义的最大值，则清空头部元素，在尾部插入新元素
        auto front = m_deviceAccessRecordMappingDeque.front();
        m_deviceAccessRecordMappingDeque.pop_front();

        auto itPos = m_deviceAccessRecordMap.find(front.strAccessID);
        if (itPos != m_deviceAccessRecordMap.end())
        {
            //同时清除Map中存储的详细接入信息
            m_deviceAccessRecordMap.erase(itPos);
        }
    }

    auto itPos = m_deviceAccessRecordMap.find(accessedDevice.m_strAccessID);
    if (itPos == m_deviceAccessRecordMap.end())
    {
        m_deviceAccessRecordMap.insert(std::make_pair(accessedDevice.m_strAccessID, accessedDevice));

        DeviceAccessRecordMapping deviceAccessRecordMapping;
        deviceAccessRecordMapping.strAccessID = accessedDevice.m_strAccessID;
        deviceAccessRecordMapping.itAccessIDPos = m_deviceAccessRecordMap.find(accessedDevice.m_strAccessID);

        m_deviceAccessRecordMappingDeque.push_back(deviceAccessRecordMapping);
    }
    else
    {
        //如果已经记录设备登录信息，则只刷新登出时间
        auto &deviceLoginRecord = itPos->second;
        deviceLoginRecord.m_strLogoutTime = accessedDevice.m_strLogoutTime;
    }

    m_deviceAccessDequeMutex.unlock();
}

void ClusterAccessCollector::PushUserAccessRecord(const InteractiveProtoHandler::UserAccessRecord &accessedUser)
{
    m_userAccessDequeMutex.lock();
    if (MAX_USER_ACCESS_RECORD_SIZE <= m_userAccessRecordMappingDeque.size())
    {
        //如果队列已存储的元素个数达到定义的最大值，则清空头部元素，在尾部插入新元素
        auto front = m_userAccessRecordMappingDeque.front();
        m_userAccessRecordMappingDeque.pop_front();

        auto itPos = m_userAccessRecordMap.find(front.strAccessID);
        if (itPos != m_userAccessRecordMap.end())
        {
            //同时清除Map中存储的详细接入信息
            m_userAccessRecordMap.erase(itPos);
        }
    }

    auto itPos = m_userAccessRecordMap.find(accessedUser.m_strAccessID);
    if (itPos == m_userAccessRecordMap.end())
    {
        m_userAccessRecordMap.insert(std::make_pair(accessedUser.m_strAccessID, accessedUser));

        UserAccessRecordMapping userAccessRecordMapping;
        userAccessRecordMapping.strAccessID = accessedUser.m_strAccessID;
        userAccessRecordMapping.itAccessIDPos = m_userAccessRecordMap.find(accessedUser.m_strAccessID);

        m_userAccessRecordMappingDeque.push_back(userAccessRecordMapping);
    }
    else
    {
        //如果已经记录用户登录信息，则只刷新登出时间
        auto &userLoginRecord = itPos->second;
        userLoginRecord.m_strLogoutTime = accessedUser.m_strLogoutTime;
    }

    m_userAccessDequeMutex.unlock();
}

