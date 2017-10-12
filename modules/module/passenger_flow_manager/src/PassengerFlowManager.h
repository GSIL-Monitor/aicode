#ifndef _USER_MANAGER_
#define _USER_MANAGER_

#include "boost/noncopyable.hpp"
#include "ControlCenter.h"
#include <unordered_map>
#include <string>
#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "NetComm.h"
#include "DBInfoCacheManager.h"
#include "boost/atomic.hpp"
#include "SessionMgr.h"
#include "PassengerFlowProtoHandler.h"

class MysqlImpl;

/************************************************************************/
/* 用户管理类，提供了管理用户所有相关的方法接口和实现。
 * 该类提供的方法会注册到到ControlCenter类中。
 * Author：尹宾
 * Date：2016-12-1*/
 /************************************************************************/
class PassengerFlowManager : public boost::noncopyable
{
public:

    static const unsigned int UNUSED_INPUT_UINT = 0xFFFFFFFF;

    static const std::string READ_STATE;
    static const std::string UNREAD_STATE;

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
        std::string m_strDevSessionTimeoutCountThreshold;
        std::string m_strFileServerURL;
        std::string m_strGetIpInfoSite;
        std::string m_strMemAddressGlobal;
        std::string m_strMemPortGlobal;
        std::string m_strUserLoginMutex;
        std::string m_strUserAllowDiffTerminal;
        std::string m_strUserKickoutType;
        std::string m_strMasterNode;
    } ParamInfo;

    typedef struct _AccessDomainInfo
    {
        std::string strDomainName;
        unsigned int uiLease;
    } AccessDomainInfo;

    inline void SetParamInfo(const ParamInfo &pinfo)
    {
        m_ParamInfo = pinfo;
    };

    PassengerFlowManager(const ParamInfo &pinfo);
    ~PassengerFlowManager();
    bool Init();

    bool GetMsgType(const std::string &strMsg, int &iMsgType);

    bool PreCommonHandler(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool AddStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeleteStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifyStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryStoreInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAllStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool AddEntranceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeleteEntranceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifyEntranceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool AddEntranceDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeleteEntranceDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool AddEventReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeleteEventReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifyEventReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryEventInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAllEventReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool AddSmartGuardStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeleteSmartGuardStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifySmartGuardStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QuerySmartGuardStoreInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAllSmartGuardStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ImportPOSDataReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryCustomerFlowStatisticReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ReportCustomerFlowDataReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

private:

    std::string CurrentTime();

    int Year(const std::string &strDate);

    int Month(const std::string &strDate);

    int Quarter(const std::string &strDate);

    int MonthDuration(const std::string &strBeginDate, const std::string &strEndDate);

    int QuarterDuration(const std::string &strBeginDate, const std::string &strEndDate);

    int TimePrecisionScale(const std::string &strDate, const unsigned int uiTimePrecision);

    bool IsValidStore(const std::string &strUserID, const std::string &strStoreName);

    void AddStore(const std::string &strUserID, const PassengerFlowProtoHandler::Store &storeInfo);

    void AddUserStoreAssociation(const std::string &strUserID, const std::string &strStoreID);

    void DeleteStore(const std::string &strStoreID);

    void ModifyStore(const PassengerFlowProtoHandler::Store &storeInfo);

    bool QueryStoreInfo(const std::string &strStoreID, PassengerFlowProtoHandler::Store &storeInfo);

    bool QueryStoreEntrance(const std::string &strStoreID, std::list<PassengerFlowProtoHandler::Entrance> &entranceList);

    bool QueryEntranceDevice(const std::string &strEntranceID, std::list<std::string> &strDeviceIDList);

    bool QueryAllStore(const std::string &strUserID, std::list<PassengerFlowProtoHandler::Store> &storeList,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    bool IsValidEntrance(const std::string &strStoreID, const std::string &strEntranceName);

    void AddEntrance(const std::string &strStoreID, const PassengerFlowProtoHandler::Entrance &entrance);

    bool QueryDeviceBoundEntrance(const std::string &strDeviceID, std::string &strEntranceID);

    void AddEntranceDevice(const std::string &strEntranceID, const std::string &strDeviceID);

    void DeleteEntrance(const std::string &strEntranceID);

    void DeleteEntranceDevice(const std::string &strEntranceID, const std::string &strDeviceID);

    void ModifyEntrance(const PassengerFlowProtoHandler::Entrance &entrance, const std::list<std::string> &strAddedDeviceIDList,
        const std::list<std::string> &strDeletedDeviceIDList);

    void AddEvent(const PassengerFlowProtoHandler::Event &eventInfo);

    void AddEventType(const std::string &strEventID, const unsigned int uiType);

    void AddEventUserAssociation(const std::string &strEventID, const std::string &strUserID, const std::string &strUserRole);

    void AddEventRemark(const std::string &strEventID, const std::string &strUserID, const std::string &strRemark);

    void DeleteEvent(const std::string &strEventID, const std::string &strUserID);

    void ModifyEvent(const PassengerFlowProtoHandler::Event &eventInfo);

    bool QueryEventInfo(const std::string &strEventID, PassengerFlowProtoHandler::Event &eventInfo);

    bool QueryEventType(const std::string &strEventID, std::list<unsigned int> &typeList);

    bool QueryEventUserAssociation(const std::string &strEventID, std::list<std::string> &handlerList);

    bool QueryEventRemark(const std::string &strEventID, std::list<std::list<std::string>> &strRemarkList);

    bool QueryAllEvent(const std::string &strUserID, std::list<PassengerFlowProtoHandler::Event> &eventList,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    void AddSmartGuardStore(const std::string &strUserID, const std::string &strStoreID,
        const PassengerFlowProtoHandler::SmartGuardStore &smartGuardStore);

    void AddGuardStoreEntranceAssociation(const std::string &strPlanID, const std::string &strEntranceID);

    void DeleteSmartGuardStore(const std::string &strPlanID);

    void ModifySmartGuardStore(const PassengerFlowProtoHandler::SmartGuardStore &smartGuardStore);

    bool QuerySmartGuardStoreInfo(const std::string &strPlanID, PassengerFlowProtoHandler::SmartGuardStore &smartGuardStore);

    bool QueryGuardStoreEntranceAssociation(const std::string &strPlanID, std::list<std::string> &strEntranceIDList);

    bool QueryAllSmartGuardStoreByUser(const std::string &strUser, std::list<PassengerFlowProtoHandler::SmartGuardStore> &smartGuardStoreList,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    bool QueryAllSmartGuardStoreByDevice(const std::string &strDeviceID, std::list<PassengerFlowProtoHandler::SmartGuardStore> &smartGuardStoreList);

    void ImportPOSData(const std::string &strStoreID, const unsigned int uiOrderAmount, const unsigned int uiGoodsAmount,
        const double dDealAmount, const std::string &strDealDate);

    bool QueryCustomerFlowStatistic(const std::string &strStoreID, const std::string &strBeginDate,
        const std::string &strEndDate, const unsigned int uiTimePrecision, std::string &strChartData);

    bool QueryRawCustomerFlow(const std::string &strStoreID, const std::string &strBeginDate,
        const std::string &strEndDate, const unsigned int uiTimePrecision, std::string &strChartData);

    bool QueryHourlyCustomerFlow(const std::string &strStoreID, const std::string &strBeginDate,
        const std::string &strEndDate, const unsigned int uiTimePrecision, std::string &strChartData);

    bool QueryDailyCustomerFlow(const std::string &strStoreID, const std::string &strBeginDate,
        const std::string &strEndDate, const unsigned int uiTimePrecision, std::string &strChartData);

    void GenerateChartData(const std::map<std::string, std::list<int>> &chartDataMap, const std::string &strBeginDate,
        const std::string &strEndDate, const unsigned int uiTimePrecision, std::string &strChartData);

    void GenerateChartDataWithPOS(const std::map<std::string, std::list<boost::any>> &chartDataMap, const std::string &strBeginDate,
        const std::string &strEndDate, const unsigned int uiTimePrecision, std::string &strChartData);

    void ReportCustomerFlow(const std::string &strDeviceID, const std::list<PassengerFlowProtoHandler::RawCustomerFlow> &customerFlowList);

    void AddCustomerFlow(const std::string &strDeviceID, const PassengerFlowProtoHandler::RawCustomerFlow &customerFlowInfo);

private:
    ParamInfo m_ParamInfo;

    Runner m_DBRuner;
    boost::shared_ptr<PassengerFlowProtoHandler> m_pProtoHandler;

    MysqlImpl *m_pMysql;
    DBInfoCacheManager m_DBCache;

    boost::atomic_uint64_t m_uiMsgSeq;

    SessionMgr m_SessionMgr;

    TimeOutHandler m_DBTimer;

};



#endif