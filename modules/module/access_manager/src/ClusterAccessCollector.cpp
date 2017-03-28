#include <boost/scope_exit.hpp>

#include "ReturnCode.h"
#include "SessionMgr.h"
#include "mysql_impl.h"
#include "DBInfoCacheManager.h"
#include "LogRLD.h"
#include "ClusterAccessCollector.h"

ClusterAccessCollector::ClusterAccessCollector(SessionMgr *pSessionMgr, MysqlImpl *pMysqlImpl, DBInfoCacheManager *pDBInfo) :
m_uiDeviceAccessDequeSize(0), m_uiUserAccessDequeSize(0), m_uiDeviceAccessRecordCursor(0), m_uiUserAccessRecordCursor(0),
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
    if (0 == m_uiDeviceAccessDequeSize)
    {
        LOG_INFO_RLD("GetDeviceAccessRecord completed, current deque is empty");

        return true;
    }

    if (uiBeginIndex >= m_uiDeviceAccessDequeSize)
    {
        LOG_ERROR_RLD("GetDeviceAccessRecord failed, begin index is out of range, index is " << uiBeginIndex << " and record total size is " << m_uiDeviceAccessDequeSize);

        return false;
    }

    LOG_INFO_RLD("GetDeviceAccessRecord request begin index is " << uiBeginIndex << " and record total size is " << m_uiDeviceAccessDequeSize);

    m_deviceAccessRecordMutex.lock();

    unsigned int size = uiPageSize > m_uiDeviceAccessDequeSize - uiBeginIndex ? m_uiDeviceAccessDequeSize - uiBeginIndex : uiPageSize;

    for (unsigned int i = uiBeginIndex; i < uiBeginIndex + size; ++i)
    {
        auto &deviceAccessRecordMapping = m_deviceAccessRecordMappingDeque[i];
        auto &deviceAccessRecord = deviceAccessRecordMapping.itAccessIDPos->second;

        if (deviceAccessRecord.m_strLogoutTime.empty())
        {
            //如果会话ID已在MemoryCache中被清空，则认为该设备已下线
            if (!m_pSessionMgr->Exist(deviceAccessRecordMapping.itAccessIDPos->first))
            {
                LOG_INFO_RLD("GetDeviceAccessRecord check session id exist result is false, set device logout time as current time");

                std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
                strCurrentTime.replace(strCurrentTime.find('T'), 1, std::string(" "));

                deviceAccessRecord.m_strLogoutTime = strCurrentTime;
            }
        }

        deviceAccessRecordList.push_back(deviceAccessRecordMapping.itAccessIDPos->second);
    }

    m_uiDeviceAccessRecordCursor += size;

    //读取完最后一批数据时，清空本轮所有已读取的数据
    if (m_uiDeviceAccessRecordCursor == m_uiDeviceAccessDequeSize)
    {
        DiscardDeviceAccessRecord(m_uiDeviceAccessDequeSize);
    }

    m_deviceAccessRecordMutex.unlock();

    return true;
}

bool ClusterAccessCollector::GetUserAccessRecord(std::list<InteractiveProtoHandler::UserAccessRecord> &userAccessRecordList,
    const unsigned int uiBeginIndex, const unsigned int uiPageSize /*= 10*/)
{
    if (0 == m_uiUserAccessDequeSize)
    {
        LOG_INFO_RLD("GetUserAccessRecord completed, current deque is empty");

        return true;
    }

    if (uiBeginIndex >= m_uiUserAccessDequeSize)
    {
        LOG_ERROR_RLD("GetUserAccessRecord failed, begin index is out of range, index is " << uiBeginIndex << " and record total size is " << m_uiUserAccessDequeSize);

        return false;
    }

    LOG_INFO_RLD("GetUserAccessRecord request begin index is " << uiBeginIndex << " and record total size is " << m_uiUserAccessDequeSize);

    m_userAccessRecordMutex.lock();

    unsigned int size = uiPageSize > m_uiUserAccessDequeSize - uiBeginIndex ? m_uiUserAccessDequeSize - uiBeginIndex : uiPageSize;

    for (unsigned int i = uiBeginIndex; i < uiBeginIndex + size; ++i)
    {
        auto &userAccessRecordMapping = m_userAccessRecordMappingDeque[i];
        auto &userAccessRecord = userAccessRecordMapping.itAccessIDPos->second;

        if (userAccessRecord.m_strLogoutTime.empty())
        {
            //如果会话ID已在MemoryCache中被清空，则认为该设备已下线
            if (!m_pSessionMgr->Exist(userAccessRecordMapping.itAccessIDPos->first))
            {
                LOG_INFO_RLD("GetUserAccessRecord check session id exist result is false, set user logout time as current time");

                std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
                strCurrentTime.replace(strCurrentTime.find('T'), 1, std::string(" "));

                userAccessRecord.m_strLogoutTime = strCurrentTime;
            }
        }

        userAccessRecordList.push_back(userAccessRecordMapping.itAccessIDPos->second);
    }

    m_uiUserAccessRecordCursor += size;

    //读取完最后一批数据时，清空本轮所有已读取的数据
    if (m_uiUserAccessRecordCursor == m_uiUserAccessDequeSize)
    {
        DiscardUserAccessRecord(m_uiUserAccessDequeSize);
    }

    m_userAccessRecordMutex.unlock();

    return true;
}

unsigned int ClusterAccessCollector::DeviceAccessRecordSize(const unsigned int uiBeginIndex, const unsigned int uiPageSize /*= 10*/)
{
    if (0 == uiBeginIndex)
    {
        m_deviceAccessRecordMutex.lock();

        UpdateDeviceAccessRecordParam(uiBeginIndex, uiPageSize);

        m_uiDeviceAccessDequeSize = m_deviceAccessRecordMappingDeque.size();

        m_deviceAccessRecordMutex.unlock();
    }

    return m_uiDeviceAccessDequeSize;
}

unsigned int ClusterAccessCollector::UserAccessRecordSize(const unsigned int uiBeginIndex, const unsigned int uiPageSize /*= 10*/)
{
    if (0 == uiBeginIndex)
    {
        m_userAccessRecordMutex.lock();

        UpdateUserAccessRecordParam(uiBeginIndex, uiPageSize);

        m_uiUserAccessDequeSize = m_userAccessRecordMappingDeque.size();

        m_userAccessRecordMutex.unlock();
    }

    return m_uiUserAccessDequeSize;
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

void ClusterAccessCollector::UpdateDeviceAccessRecordParam(const unsigned int uiBeginIndex, const unsigned int uiPageSize /*= 10*/)
{
    if (0 == uiBeginIndex)
    {
        //在上一轮数据接收异常时，保留最后一批数据，清除之前的所有数据
        if (m_uiDeviceAccessRecordCursor > 0 && uiPageSize < m_uiDeviceAccessDequeSize - m_uiDeviceAccessRecordCursor)
        {
            int eraseSize = m_uiDeviceAccessRecordCursor - uiPageSize;

            if (eraseSize > 0)
            {
                DiscardDeviceAccessRecord(eraseSize);
            }
        }
    }

    m_uiDeviceAccessRecordCursor = uiBeginIndex;
}

void ClusterAccessCollector::UpdateUserAccessRecordParam(const unsigned int uiBeginIndex, const unsigned int uiPageSize /*= 10*/)
{
    if (0 == uiBeginIndex)
    {
        //在上一轮数据接收异常时，保留最后一批数据，清除之前的所有数据
        if (m_uiUserAccessRecordCursor > 0 && uiPageSize < m_uiUserAccessDequeSize - m_uiUserAccessRecordCursor)
        {
            int eraseSize = m_uiUserAccessRecordCursor - uiPageSize;

            if (eraseSize > 0)
            {
                DiscardUserAccessRecord(eraseSize);
            }
        }
    }

    m_uiUserAccessRecordCursor = uiBeginIndex;
}

void ClusterAccessCollector::DiscardDeviceAccessRecord(const unsigned int uiSize)
{
    for (unsigned int i = 0; i < uiSize; ++i)
    {
        if (!m_deviceAccessRecordMappingDeque.empty())
        {
            auto mapping = m_deviceAccessRecordMappingDeque.front();
            m_deviceAccessRecordMappingDeque.pop_front();

            if (!m_deviceAccessRecordMap.empty())
            {
                m_deviceAccessRecordMap.erase(mapping.itAccessIDPos);
            }
        }
    }
}

void ClusterAccessCollector::DiscardUserAccessRecord(const unsigned int uiSize)
{
    for (unsigned int i = 0; i < uiSize; ++i)
    {
        if (!m_userAccessRecordMappingDeque.empty())
        {
            auto mapping = m_userAccessRecordMappingDeque.front();
            m_userAccessRecordMappingDeque.pop_front();

            if (!m_userAccessRecordMap.empty())
            {
                m_userAccessRecordMap.erase(mapping.itAccessIDPos);
            }
        }
    }
}

void ClusterAccessCollector::PushDeviceAccessRecord(const InteractiveProtoHandler::DeviceAccessRecord &accessedDevice)
{
    m_deviceAccessRecordMutex.lock();

    //队列长度达到设定的最大值时，清除头部元素，在尾部插入新元素
    if (MAX_DEVICE_ACCESS_RECORD_SIZE <= m_deviceAccessRecordMappingDeque.size())
    {
        DiscardDeviceAccessRecord(1);
    }

    auto itPos = m_deviceAccessRecordMap.find(accessedDevice.m_strAccessID);
    if (itPos == m_deviceAccessRecordMap.end())
    {
        auto itPos = m_deviceAccessRecordMap.insert(std::make_pair(accessedDevice.m_strAccessID, accessedDevice)).first;

        DeviceAccessRecordMapping deviceAccessRecordMapping;
        deviceAccessRecordMapping.strAccessID = accessedDevice.m_strAccessID;
        deviceAccessRecordMapping.itAccessIDPos = itPos;

        m_deviceAccessRecordMappingDeque.push_back(deviceAccessRecordMapping);
    }
    else
    {
        //如果已经记录设备登录信息，则只刷新登出时间
        auto &deviceLoginRecord = itPos->second;
        deviceLoginRecord.m_strLogoutTime = accessedDevice.m_strLogoutTime;
    }

    m_deviceAccessRecordMutex.unlock();
}

void ClusterAccessCollector::PushUserAccessRecord(const InteractiveProtoHandler::UserAccessRecord &accessedUser)
{
    m_userAccessRecordMutex.lock();

    //队列长度达到设定的最大值时，清除头部元素，在尾部插入新元素
    if (MAX_USER_ACCESS_RECORD_SIZE <= m_userAccessRecordMappingDeque.size())
    {
        DiscardUserAccessRecord(1);
    }

    auto itPos = m_userAccessRecordMap.find(accessedUser.m_strAccessID);
    if (itPos == m_userAccessRecordMap.end())
    {
        auto itPos = m_userAccessRecordMap.insert(std::make_pair(accessedUser.m_strAccessID, accessedUser)).first;

        UserAccessRecordMapping userAccessRecordMapping;
        userAccessRecordMapping.strAccessID = accessedUser.m_strAccessID;
        userAccessRecordMapping.itAccessIDPos = itPos;

        m_userAccessRecordMappingDeque.push_back(userAccessRecordMapping);
    }
    else
    {
        //如果已经记录用户登录信息，则只刷新登出时间
        auto &userLoginRecord = itPos->second;
        userLoginRecord.m_strLogoutTime = accessedUser.m_strLogoutTime;
    }

    m_userAccessRecordMutex.unlock();
}



