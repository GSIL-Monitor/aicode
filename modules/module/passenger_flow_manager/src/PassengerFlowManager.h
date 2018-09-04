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
#include "MessagePush_Getui.h"

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

    static const int UNREAD_STATE = 0;
    static const int READ_STATE = 1;

    static const int EVENT_REMOTE_PATROL = 0;
    static const int EVENT_REGULAR_PATROL = 1;
    static const int EVENT_STORE_EVALUATION = 2;
    static const int EVENT_ALARM_CREATED = 3;
    static const int EVENT_ALARM_RECOVERD = 4;

    static std::string ALLOW_ACCESS;
    static std::string DISALLOW_ACCESS;

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
        std::string m_strPushServerUrl;
        int m_iAuthExpire;
        std::string m_strAndroidAppID;
        std::string m_strAndroidAppKey;
        std::string m_strAndroidMasterSecret;
        std::string m_strIOSAppID;
        std::string m_strIOSAppKey;
        std::string m_strIOSMasterSecret;
        std::string m_strMessageTitle;
        std::string m_strMessageContentRegularPatrol;
        std::string m_strMessageContentRemotePatrol;
        std::string m_strMessageContentEvaluation;
        std::string m_strAlarmMessageTitle;
        std::string m_strAlarmMessageContentCreated;
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

    bool AddAreaReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeleteAreaReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifyAreaReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAreaInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAllAreaReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool BindPushClientIDReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool UnbindPushClientIDReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);


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


    bool AddRegularPatrolReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeleteRegularPatrolReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifyRegularPatrolReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryRegularPatrolInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAllRegularPatrolReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);


    bool UserJoinStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool UserQuitStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryStoreAllUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryCompanyAllUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);


    bool AddVIPCustomerReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeleteVIPCustomerReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifyVIPCustomerReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryVIPCustomerInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAllVIPCustomerReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);


    bool AddVIPConsumeHistoryReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeleteVIPConsumeHistoryReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifyVIPConsumeHistoryReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAllVIPConsumeHistoryReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);


    bool AddEvaluationTemplateReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeleteEvaluationTemplateReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifyEvaluationTemplateReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAllEvaluationTemplateReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);


    bool AddStoreEvaluationReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeleteStoreEvaluationReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifyStoreEvaluationReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryStoreEvaluationInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAllStoreEvaluationReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);


    bool AddRemotePatrolStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeleteRemotePatrolStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifyRemotePatrolStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryRemotePatrolStoreInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAllRemotePatrolStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);


    bool AddStoreSensorReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeleteStoreSensorReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifyStoreSensorReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryStoreSensorInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAllStoreSensorReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);


    bool ImportPOSDataReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryCustomerFlowStatisticReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryPatrolResultReportReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ReportCustomerFlowDataReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    
    bool ReportSensorInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);
    
    bool ReportSensorAlarmInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QuerySensorAlarmThresholdReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);


    bool RemoveSensorRecordsReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool RemoveSensorAlarmRecordsReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QuerySensorRecordsReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QuerySensorAlarmRecordsReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);


    bool AddRoleReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool RemoveRoleReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifyRoleReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryRoleReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAllRoleReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool UserBindRoleReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

private:

    bool PushMessage(const std::string &strTitle, const std::string &strContent, const std::string &strPayload, const std::string &strUserID);

    std::string CurrentTime();

    int Year(const std::string &strDate);

    int Month(const std::string &strDate);

    int Quarter(const std::string &strDate);

    int MonthDuration(const std::string &strBeginDate, const std::string &strEndDate);

    int QuarterDuration(const std::string &strBeginDate, const std::string &strEndDate);

    int TimePrecisionScale(const std::string &strDate, const unsigned int uiTimePrecision);

    bool UserAccessPermission(const std::string &strUserID, const int iMsgType, std::string &strPermission);

    bool IsValidArea(const std::string &strUserID, const std::string &strAreaName);

    bool QueryUserCompany(const std::string &strUserID, std::string &strCompanyID);

    void AddArea(const std::string &strUserID, const std::string &strCompanyID, const PassengerFlowProtoHandler::Area &areaInfo);

    void DeleteArea(const std::string &strAreaID);

    void ModifyArea(const PassengerFlowProtoHandler::Area &areaInfo);

    bool QueryAreaInfo(const std::string &strAreaID, PassengerFlowProtoHandler::Area &areaInfo);

    bool QueryAreaParent(const std::string &strAreaID, std::list<PassengerFlowProtoHandler::Area> &areaList);

    bool QueryAllArea(const std::string &strUserID, std::list<PassengerFlowProtoHandler::Area> &areaList,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    void BindPushClientID(const std::string &strUserID, const std::string &strCllientID);

    void UnbindPushClientID(const std::string &strUserID, const std::string &strCllientID);

    bool QueryPushParameter(const std::string &strUserID, std::list<std::string> &strClientIDList);

    bool IsValidStore(const std::string &strUserID, const std::string &strStoreName);

    void AddStore(const std::string &strUserID, const PassengerFlowProtoHandler::Store &storeInfo);

    void AddUserStoreAssociation(const std::string &strUserID, const std::string &strStoreID);

    void DeleteStore(const std::string &strStoreID);

    void ModifyStore(const PassengerFlowProtoHandler::Store &storeInfo);

    bool QueryStoreInfo(const std::string &strStoreID, PassengerFlowProtoHandler::Store &storeInfo);

    bool QueryStoreEntrance(const std::string &strStoreID, std::list<PassengerFlowProtoHandler::Entrance> &entranceList);

    bool QueryEntranceDevice(const std::string &strEntranceID, std::list<std::string> &strDeviceIDList);

    bool QueryAllStore(const std::string &strUserID, const std::string &strAreaID, const unsigned int uiOpenState,
        std::list<PassengerFlowProtoHandler::Store> &storeList, const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    bool QuerySubArea(const std::string &strAreaID, std::list<std::string> &strSubAreaIDList);

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

    bool QueryStoreAndSensorBySensorInfo(const std::string &strDevID, const std::string &strSensorKey, std::string &strStoreName, 
        std::string &strSensorName, std::string &strSensorID, std::string &strSensorType);

    bool QueryStoreByEvent(const std::string &strSource, const unsigned int uiEventType, std::string &strStoreName, std::string &strStoreID);

    bool QueryStoreByRegularPatrol(const std::string &strPatrolID, std::string &strStoreName, std::string &strStoreID);

    bool QueryStoreByStoreEvaluation(const std::string &strEvaluationID, std::string &strStoreName, std::string &strStoreID);

    void DeleteEvent(const std::string &strEventID, const std::string &strUserID);

    void DeleteCreatedEvent(const std::string &strEventID, const std::string &strUserID);

    void DeleteHandledEvent(const std::string &strEventID, const std::string &strUserID);

    void ModifyEvent(const PassengerFlowProtoHandler::Event &eventInfo);

    void ModifyEventType(const std::string &strEventID, const std::list<unsigned int> &typeList);

    void ModifyEventHandler(const std::string &strEventID, const std::list<std::string> &handlerList);

    void DeleteEventType(const std::string &strEventID);

    void DeleteEventHandler(const std::string &strEventID);

    bool QueryEventInfo(const std::string &strEventID, PassengerFlowProtoHandler::Event &eventInfo);

    bool QueryEventType(const std::string &strEventID, std::list<unsigned int> &typeList);

    bool QueryEventUserAssociation(const std::string &strEventID, std::list<std::string> &handlerList);

    bool QueryEventRemark(const std::string &strEventID, std::string &strRemark);

    bool QueryAllEvent(const std::string &strUserID, const std::string &strProcessState, std::list<PassengerFlowProtoHandler::Event> &eventList,
        const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    bool QueryCreatedEvent(const std::string &strUserID, const std::string &strProcessState, std::list<PassengerFlowProtoHandler::Event> &eventList,
        const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    bool QueryHandledEvent(const std::string &strUserID, const std::string &strProcessState, std::list<PassengerFlowProtoHandler::Event> &eventList,
        const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    bool QueryEventSub(const std::string &strSql, std::list<PassengerFlowProtoHandler::Event> &eventList);

    void AddSmartGuardStore(const std::string &strUserID, const std::string &strStoreID,
        const PassengerFlowProtoHandler::SmartGuardStore &smartGuardStore);

    void AddGuardStoreEntranceAssociation(const std::string &strPlanID, const std::string &strEntranceID);

    void DeleteSmartGuardStore(const std::string &strPlanID);

    void ModifySmartGuardStore(const PassengerFlowProtoHandler::SmartGuardStore &smartGuardStore);

    void ModifyGuardStoreEntranceAssociation(const std::string &strPlanID, const std::list<std::string> &entranceList);

    void DeleteGuardStoreEntranceAssociation(const std::string &strPlanID);

    bool QuerySmartGuardStoreInfo(const std::string &strPlanID, PassengerFlowProtoHandler::SmartGuardStore &smartGuardStore);

    bool QueryGuardStoreEntranceAssociation(const std::string &strPlanID, std::list<std::string> &strEntranceIDList);

    bool QueryAllSmartGuardStoreByUser(const std::string &strUser, std::list<PassengerFlowProtoHandler::SmartGuardStore> &smartGuardStoreList,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    bool QueryAllSmartGuardStoreByDevice(const std::string &strDeviceID, std::list<PassengerFlowProtoHandler::SmartGuardStore> &smartGuardStoreList);

    void AddRegularPatrol(const std::string &strUserID, const PassengerFlowProtoHandler::RegularPatrol &regularPatrol);

    void AddPatrolPlanEntranceAssociation(const std::string &strPlanID, const std::string &strEntranceID);

    void AddPatrolPlanUserAssociation(const std::string &strPlanID, const std::string &strUserID, const std::string &strRole);

    void AddRegularPatrolTime(const std::string &strPlanID, const std::string &strTime);

    void DeleteRegularPatrol(const std::string &strPlanID);

    void ModifyRegularPatrol(const PassengerFlowProtoHandler::RegularPatrol &regularPatrol);

    void ModifyPatrolPlanEntranceAssociation(const std::string &strPlanID, const std::list<PassengerFlowProtoHandler::PatrolStoreEntrance> &storeList);

    void ModifyPatrolPlanUserAssociation(const std::string &strPlanID, const std::list<std::string> &strUserIDList);

    void DeletePatrolPlanEntranceAssociation(const std::string &strPlanID);

    void DeletePatrolPlanUserAssociation(const std::string &strPlanID);

    void ModifyRegularPatrolTime(const std::string &strPlanID, const std::list<std::string> &timeList);

    void DeleteRegularPatrolTime(const std::string &strPlanID);

    bool QueryRegularPatrolInfo(const std::string &strPlanID, PassengerFlowProtoHandler::RegularPatrol &regularPatrol);

    bool QueryPatrolPlanEntranceAssociation(const std::string &strPlanID, std::list<PassengerFlowProtoHandler::PatrolStoreEntrance> &storeEntranceList,
        const std::string &strEntranceID = std::string());

    bool QueryPatrolPlanUserAssociation(const std::string &strPlanID, std::list<std::string> &strUserIDList);

    bool QueryRegularPatrolTime(const std::string &strPlanID, std::list<std::string> &strTimeList);

    bool QueryAllRegularPatrolByUser(const std::string &strUser, std::list<PassengerFlowProtoHandler::RegularPatrol> &regularPatrolList,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    bool QueryAllRegularPatrolByDevice(const std::string &strDeviceID, std::list<PassengerFlowProtoHandler::RegularPatrol> &regularPatrolList);

    void UserJoinStore(const std::string &strUserID, const std::string &strStoreID, const std::string &strRole);

    void UserQuitStore(const std::string &strUserID, const std::string &strStoreID);

    bool QueryStoreAllUser(const std::string &strStoreID, std::list<PassengerFlowProtoHandler::UserBrief> &userList);

    bool QueryCompanyAllUser(const std::string &strCompanyID, std::list<PassengerFlowProtoHandler::UserBrief> &userList);

    void AddVIPCustomer(const PassengerFlowProtoHandler::VIPCustomer &customerInfo);

    void DeleteVIPCustomer(const std::string &strVIPID);

    void ModifyVIPCustomer(const PassengerFlowProtoHandler::VIPCustomer &customerInfo);

    bool QueryVIPCustomerInfo(const std::string &strPlanID, PassengerFlowProtoHandler::VIPCustomer &customerInfo);

    bool QueryAllVIPCustomer(std::list<PassengerFlowProtoHandler::VIPCustomer> &customerList,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    void AddVIPConsumeHistory(const PassengerFlowProtoHandler::VIPConsumeHistory &consumeHistory);

    void DeleteVIPConsumeHistory(const std::string &strConsumeID);

    void ModifyVIPConsumeHistory(const PassengerFlowProtoHandler::VIPConsumeHistory &consumeHistory);

    bool QueryAllVIPConsumeHistory(const std::string &strVIPID, std::list<PassengerFlowProtoHandler::VIPConsumeHistory> &consumeList,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    void AddEvaluationTemplate(const PassengerFlowProtoHandler::EvaluationItem &evaluationItem);

    void DeleteEvaluationTemplate(const std::string &strItemID);

    void ModifyEvaluationTemplate(const PassengerFlowProtoHandler::EvaluationItem &evaluationItem);

    bool QueryAllEvaluationTemplate(std::list<PassengerFlowProtoHandler::EvaluationItem> &evaluationItemList,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    void AddStoreEvaluation(const PassengerFlowProtoHandler::StoreEvaluation &storeEvaluation);

    void AddStoreEvaluationScore(const std::string &strEvaluationID, const PassengerFlowProtoHandler::EvaluationItemScore &itemScore);

    void AddStoreEvaluationItemPicture(const std::string &strEvaluationID, const std::string &strItemID, const std::string &strPicture);

    void AddStoreEvaluationPicture(const std::string &strEvaluationID, const std::string &strPicture);

    void DeleteStoreEvaluation(const std::string &strEvaluationID);

    void ModifyStoreEvaluation(const PassengerFlowProtoHandler::StoreEvaluation &storeEvaluation);

    void ModifyStoreEvaluationScore(const std::string &strEvaluationID, const std::list<PassengerFlowProtoHandler::EvaluationItemScore> &scoreList);

    void DeleteStoreEvaluationScore(const std::string &strEvaluationID);

    void ModifyStoreEvaluationItemPicture(const std::string &strEvaluationID, const std::string &strItemID, const std::list<std::string> &strPictureList);

    void DeleteStoreEvaluationItemPicture(const std::string &strEvaluationID, const std::string &strItemID);

    void ModifyStoreEvaluationPicture(const std::string &strEvaluationID, const std::list<std::string> &strPictureList);

    void DeleteStoreEvaluationPicture(const std::string &strEvaluationID);

    bool QueryStoreEvaluationInfo(const std::string &strEvaluationID, PassengerFlowProtoHandler::StoreEvaluation &storeEvaluation);

    bool QueryStoreEvaluationScore(const std::string &strEvaluationID, double &dTotalScore,
        std::list<PassengerFlowProtoHandler::EvaluationItemScore> &scoreList);

    bool QueryStoreEvaluationItemPicture(const std::string &strEvaluationID, const std::string &strItemID, std::list<std::string> &strPictureList);

    bool QueryStoreEvaluationPicture(const std::string &strEvaluationID, std::list<std::string> &strPictureList);

    bool QueryAllStoreEvaluation(const std::string &strStoreID, std::list<PassengerFlowProtoHandler::StoreEvaluation> &storeEvaluationList,
        const unsigned int uiCheckStatus, const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    void AddRemotePatrolStore(const PassengerFlowProtoHandler::RemotePatrolStore &patrolStore);

    void AddRemotePatrolStoreEntrance(const std::string &strPatrolID, const std::string &strEntranceID);

    void AddRemotePatrolStoreScreenshot(const std::string &strPatrolID, const std::string &strEntranceID, const std::string &strScreenshot);

    void DeleteRemotePatrolStore(const std::string &strPatrolID);

    void ModifyRemotePatrolStore(const PassengerFlowProtoHandler::RemotePatrolStore &patrolStore);

    void ModifyRemotePatrolStoreScreenshot(const std::string &strPatrolID, const std::string &strEntranceID, const std::list<std::string> &screenshotList);

    void DeleteRemotePatrolStoreScreenshot(const std::string &strPatrolID);

    bool QueryRemotePatrolStoreInfo(const std::string &strPatrolID, PassengerFlowProtoHandler::RemotePatrolStore &patrolStore);

    bool QueryRemotePatrolStoreEntrance(const std::string &strPatrolID, std::list<std::string> &entranceList);

    bool QueryRemotePatrolStoreScreenshot(const std::string &strPatrolID, std::list<PassengerFlowProtoHandler::EntrancePicture> &screenshotList);

    bool QueryAllRemotePatrolStore(const std::string &strStoreID, const std::string &strPlanID, const unsigned int uiPatrolResult, const unsigned int uiPlanFlag,
        std::list<PassengerFlowProtoHandler::RemotePatrolStore> &patrolStoreList, const std::string &strBeginDate, const std::string &strEndDate,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 12);

    void AddStoreSensor(const PassengerFlowProtoHandler::Sensor &sensorInfo);

    void DeleteStoreSensor(const std::string &strSensorID);

    void ModifyStoreSensor(const PassengerFlowProtoHandler::Sensor &sensorInfo);

    bool QueryStoreSensorInfo(const std::string &strSensorID, PassengerFlowProtoHandler::Sensor &sensorInfo);

    bool QuerySensorValue(const std::string &strSensorID, std::string &strValue);

    bool QueryAllStoreSensor(const std::string &strStoreID, std::list<PassengerFlowProtoHandler::Sensor> &sensorList,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    bool ValidDeviceSensor(const std::string &strDevID, const unsigned int uiType, const bool IsModify = false, const std::string &strSensorID = "");

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

    bool QueryPatrolResultReport(const std::string &strUserID, const std::string &strStoreID, const std::string &strPatrolUserID,
        const unsigned int uiPatrolResult, const std::string &strBeginDate, const std::string &strEndDate, std::string &strChartData);

    void ReportCustomerFlow(const std::string &strDeviceID, const std::list<PassengerFlowProtoHandler::RawCustomerFlow> &customerFlowList);

    void AddCustomerFlow(const std::string &strDeviceID, const PassengerFlowProtoHandler::RawCustomerFlow &customerFlowInfo);

    void ReportSensorInfo(const std::string &strDeviceID, const std::list<PassengerFlowProtoHandler::Sensor> &sensorList);

    bool QueryDeviceSensor(const std::string &strDeviceID, const std::string &strSensorType, const std::string &strSensorKey, std::string &strSensorID);

    void AddSensorInfo(const std::string &strDeviceID, const std::string &strSensorID, const PassengerFlowProtoHandler::Sensor &sensorInfo);


    void ReportSensorAlarmInfo(const PassengerFlowProtoHandler::Sensor &sr, const unsigned int uiRecover, const std::string &strFileID);

    void AddSensorAlarmInfo(const std::string &strSensorID, const PassengerFlowProtoHandler::Sensor &sr, const unsigned int uiRecover, const std::string &strFileID);

    bool QuerySensorAlarmThreshold(const std::string &strDeviceID, std::list<PassengerFlowProtoHandler::Sensor> &sensorList);


    void RemoveSensorRecords(const std::string &strUserID, std::list<std::string> &strRecordIDList);

    void RemoveSensorRecordsBySensorID(const std::string &strSensorID);

    void RemoveSensorAlarmRecords(const std::string &strUserID, std::list<std::string> &strRecordIDList);

    void RemoveSensorAlarmRecordsBySensorID(const std::string &strSensorID);

    bool QuerySensorRecords(const PassengerFlowProtoHandler::QuerySensorRecordsReq &req, std::list<PassengerFlowProtoHandler::Sensor> &srlist,
        std::list<std::string> &strRecordIDList, const unsigned int uiPageSize = 10);

    bool QuerySensorAlarmRecords(const PassengerFlowProtoHandler::QuerySensorAlarmRecordsReq &req, std::list<PassengerFlowProtoHandler::SensorAlarmRecord> &sarlist,
        const unsigned int uiPageSize = 10);


    bool ValidRoleID(const std::string &strRoleID, bool &blResult);
    bool IsRoleInUsed(const std::string &strRoleID, bool &blResult);
    bool IsUserBindRoleAlready(const std::string &strUserID, bool &blResult);

    void AddRole(const std::string &strUserID, const std::string &strRoleIDNew, const std::string &strRoleIDOld);
    void RemoveRole(const std::string &strUserID, const std::string &strRoleID);
    void ModifyRole(const std::string &strUserID, const PassengerFlowProtoHandler::Role &role);
    bool QueryRole(const std::string &strUserID, const std::string &strRoleID, PassengerFlowProtoHandler::Role &role);
    bool QueryAllRole(const std::string &strUserID, std::list<PassengerFlowProtoHandler::Role> &rolelist);
    void UserBindRole(const std::string &strUserID, const std::string &strRoleID);

private:
    ParamInfo m_ParamInfo;

    Runner m_DBRuner;
    boost::shared_ptr<PassengerFlowProtoHandler> m_pProtoHandler;

    MysqlImpl *m_pMysql;
    DBInfoCacheManager m_DBCache;

    boost::atomic_uint64_t m_uiMsgSeq;

    SessionMgr m_SessionMgr;

    TimeOutHandler m_DBTimer;

    MessagePush_Getui m_pushGetuiIOS;
    MessagePush_Getui m_pushGetuiAndroid;
};



#endif