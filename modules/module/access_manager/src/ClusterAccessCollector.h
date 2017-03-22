#ifndef _CLUSTER_ACCESS_COLLECTOR_
#define _CLUSTER_ACCESS_COLLECTOR_

#include <map>
#include <list>
#include <deque>
#include <string>
#include "boost/thread/mutex.hpp"

#include "InteractiveProtoHandler.h"

class SessionMgr;
class MysqlImpl;
class DBInfoCacheManager;


class ClusterAccessCollector
{
public:
    static const unsigned int MAX_DEVICE_ACCESS_RECORD_SIZE = 50000;  //��Լ10M���ڴ�ʹ��
    static const unsigned int MAX_USER_ACCESS_RECORD_SIZE = 50000;


    ClusterAccessCollector(SessionMgr *pSessionMgr, MysqlImpl *pMysqlImpl, DBInfoCacheManager *pDBInfo);
    ~ClusterAccessCollector();

    void AddDeviceAccessRecord(const std::string &strAccessID, const std::string &strDeviceID, const unsigned int uiDeviceType,
        const std::string &strLoginTime, const std::string &strLogoutTime);
    void AddUserAccessRecord(const std::string &strAccessID, const std::string &strUserID, const unsigned int uiTerminalType,
        const std::string &strLoginTime, const std::string &strLogoutTime);

    bool GetDeviceAccessRecord(std::list<InteractiveProtoHandler::DeviceAccessRecord> &deviceAccessRecordList,
        const unsigned int uiBeginIndex, const unsigned int uiPageSize = 10);
    bool GetUserAccessRecord(std::list<InteractiveProtoHandler::UserAccessRecord> &userAccessRecordListconst,
        const unsigned int uiBeginIndex, const unsigned int uiPageSize = 10);

    unsigned int DeviceAccessRecordSize();

    unsigned int UserAccessRecordSize();


private:
    bool QueryDeviceInfo(const std::string &strDeviceID, std::string &strDeviceName);
    bool QueryUserInfo(const std::string &strUserID, std::string &strUserName, std::string &strAliasName);

    void PushDeviceAccessRecord(const InteractiveProtoHandler::DeviceAccessRecord &accessedDevice);
    void PushUserAccessRecord(const InteractiveProtoHandler::UserAccessRecord &accessedUser);

    //ʹ��Map�洢�û����豸����ϸ������Ϣ������ֱ�Ӹ��µǳ�ʱ��
    std::map<std::string, InteractiveProtoHandler::DeviceAccessRecord> m_deviceAccessRecordMap;
    std::map<std::string, InteractiveProtoHandler::UserAccessRecord> m_userAccessRecordMap;

    struct DeviceAccessRecordMapping
    {
        std::string strAccessID;
        std::map<std::string, InteractiveProtoHandler::DeviceAccessRecord>::iterator itAccessIDPos;
    };

    struct UserAccessRecordMapping
    {
        std::string strAccessID;
        std::map<std::string, InteractiveProtoHandler::UserAccessRecord>::iterator itAccessIDPos;
    };


    //ʹ�ö��д洢SessionID���û����豸������Ϣ���Ӧ��ӳ���ϵ��֧���ڶ�����β����ɾ�������Լ��Զ��е�������ʲ���
    std::deque<DeviceAccessRecordMapping> m_deviceAccessRecordMappingDeque;
    std::deque<UserAccessRecordMapping> m_userAccessRecordMappingDeque;
    boost::mutex m_deviceAccessDequeMutex;
    boost::mutex m_userAccessDequeMutex;

    SessionMgr *m_pSessionMgr;

    MysqlImpl *m_pMysqlImpl;

    DBInfoCacheManager *m_pDBInfo;
};

#endif
