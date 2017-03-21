#ifndef _MANAGEMENT_CENTER_
#define _MANAGEMENT_CENTER_

#include "boost/noncopyable.hpp"
#include "ControlCenter.h"
#include "InteractiveProtoManagementHandler.h"
#include <unordered_map>
#include <string>
#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "NetComm.h"
#include "mysql_impl.h"
#include "DBInfoCacheManager.h"
#include "boost/atomic.hpp"
#include "SessionMgr.h"


/************************************************************************/
/* 管理中心类，提供了管理相关操作的实现。
 * 该类提供的方法会注册到到ControlCenter类中。
 * Author：尹宾
 * Date：2017-3-3*/
/************************************************************************/
class ManagementCenter : public boost::noncopyable
{
public:

    static const int NORMAL_STATUS = 0;
    static const int DELETE_STATUS = 1;
    static const int FROZEN_STATUS = 2;

    static const int CLUSTER_ONLINE = 0;
    static const int CLUSTER_OFFLINE = 1;

    static const int MAX_CLUSTER_SHAKEHAND_COUNTS = 3;

    static const std::string MAX_DATE;

    typedef struct _ParamInfo
    {
        std::string m_strDBHost;
        std::string m_strDBPort;
        std::string m_strDBUser;
        std::string m_strDBPassword;
        std::string m_strDBName;
        std::string m_strMemAddress;
        std::string m_strMemPort;
        std::string m_strSessionTimeoutCountThreshold;
    } ParamInfo;

    inline void SetParamInfo(const ParamInfo &pinfo)
    {
        m_ParamInfo = pinfo;
    };

    ManagementCenter(const ParamInfo &pinfo);
    ~ManagementCenter();

    bool Init();

    bool GetMsgType(const std::string &strMsg, int &iMsgType);

    bool PreCommonHandler(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool AddClusterReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeleteClusterReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifyClusterReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryClusterInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ShakehandClusterReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAllClusterReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryClusterDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryClusterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool PushClusterDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool PushClusterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);



private:

    void AddCluster(const InteractiveProtoManagementHandler::Cluster &clusterInfo);

    void DeleteCluster(const std::string &strClusterID);

    void ModifyCluster(const InteractiveProtoManagementHandler::Cluster &clusterInfo);

    bool QueryClusterInfo(const std::string &strClusterID, InteractiveProtoManagementHandler::Cluster &clusterInfo);

    bool QueryAllCluster(const std::string &strManagementAddress, std::list<InteractiveProtoManagementHandler::ClusterStatus> &clusterStatusList);

    bool QueryClusterDevice(const std::string &strClusterID, const std::string &strBegindate, const std::string &strEnddate,
        const unsigned int uiRecordType, std::list<InteractiveProtoManagementHandler::AccessedDevice> &accessedDeviceList,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    bool QueryClusterUser(const std::string &strClusterID, const std::string &strBegindate, const std::string &strEnddate,
        const unsigned int uiRecordType, std::list<InteractiveProtoManagementHandler::AccessedUser> &accessedUserList,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    bool IsValidCluster(const std::string &strClusterAddress);

    void AddClusterPost(const std::string &strUrl, const std::string &strClusterID, const std::string &strManagementAddress);

    bool InitClusterSession();

    void ShakehandCluster();

    void RefreshClusterSession(const std::string &strClusterID, const std::string &strClusterAddress, const unsigned int uiStatus, const bool blAdd);

    void PushClusterDevice(const std::list<InteractiveProtoManagementHandler::DeviceAccessRecord> &deviceAccessRecordList);

    bool InsertAccessedDevice(const InteractiveProtoManagementHandler::DeviceAccessRecord &deviceAccessRecord);

    void PushClusterUser(const std::list<InteractiveProtoManagementHandler::UserAccessRecord> &userAccessRecordList);

    bool InsertAccessedUser(const InteractiveProtoManagementHandler::UserAccessRecord &userAccessRecord);



private:

    ParamInfo m_ParamInfo;

    Runner m_DBRuner;
    boost::shared_ptr<InteractiveProtoManagementHandler> m_pProtoHandler;

    MysqlImpl *m_pMysql;
    DBInfoCacheManager m_DBCache;

    boost::atomic_uint64_t m_uiMsgSeq;

    TimeOutHandler m_timer;

    boost::mutex m_mutex;

    struct ClusterSession
    {
        unsigned int uiStatus;
        unsigned int uiFailedCounts;
        std::string strClusterAddress;
    };
    std::map<std::string, ClusterSession> m_clusterSessionMap;

};



#endif