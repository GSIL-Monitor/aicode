#ifndef FILE_HANDLER
#define FILE_HANDLER

#include "NetComm.h"
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include "FCGIManager.h"
#include "CommMsgHandler.h"
#include "json/json.h"

/************************************************************************/
/*负责实现消息处理具体细节，向FCGIManager注册处理函数，
 *其中包括实际的业务消息处理，例如：将用户注册的信息汇总，
 *调用接口实现用户注册的业务动作。*/
/************************************************************************/

typedef boost::function<int(CommMsgHandler::Packet &pt)> RspFuncCommon;

class PassengerFlowProtoHandler;

class PassengerFlowMsgHandler : public boost::noncopyable
{
public:
    static const std::string CREATE_DOMAIN;
    static const std::string REMOVE_DOMAIN;
    static const std::string MODIFY_DOMAIN;
    static const std::string QUERY_DOMAIN;
    static const std::string QUERY_ALL_DOMAIN;

    static const std::string REGISTER_USER_ACTION;
    static const std::string ADD_STORE_ACTION;
    static const std::string DEL_STORE_ACTION;
    static const std::string MOD_STORE_ACTION;
    static const std::string QUERY_STORE_ACTION;
    static const std::string QUERY_ALL_STORE_ACTION;

    static const std::string CREATE_ENTRANCE_ACTION;
    static const std::string DELETE_ENTRANCE_ACTION;
    static const std::string MODIFY_ENTRANCE_ACTION;

    static const std::string UPLOAD_PASSENGER_FLOW_ACTION;
    
    static const std::string BIND_ENTRANCE_DEVICE;
    static const std::string UNBIND_ENTRANCE_DEVICE;

    static const std::string IMPORT_POS_DATA;

    static const std::string QUERY_PASSENGER_FLOW_REPORT;

    static const std::string REPORT_EVENT;
    static const std::string DELETE_EVENT;
    static const std::string MODIFY_EVENT;
    static const std::string QUERY_EVENT;
    static const std::string QUERY_ALL_EVENT;

    static const std::string CREATE_GUARD_PLAN;
    static const std::string DELETE_GUARD_PLAN;
    static const std::string MODIFY_GUARD_PLAN;
    static const std::string QUERY_GUARD_PLAN;
    static const std::string QUERY_ALL_GUARD_PLAN;

    static const std::string CREATE_PATROL_PLAN;
    static const std::string DELETE_PATROL_PLAN;
    static const std::string MODIFY_PATROL_PLAN;
    static const std::string QUERY_PATROL_PLAN;
    static const std::string QUERY_ALL_PATROL_PLAN;

    static const std::string CREATE_VIP;
    static const std::string DELETE_VIP;
    static const std::string MODIFY_VIP;
    static const std::string QUERY_VIP;
    static const std::string QUERY_ALL_VIP;

    static const std::string CREATE_VIP_CONSUME;
    static const std::string DELETE_VIP_CONSUME;
    static const std::string MODIFY_VIP_CONSUME;
    static const std::string QUERY_VIP_CONSUME;

    static const std::string USER_JOIN_STORE;
    static const std::string USER_QUIT_STORE;
    static const std::string QUERY_USER_STORE;

    static const std::string CREATE_EVALUATION_TEMPLATE;
    static const std::string DELETE_EVALUATION_TEMPLATE;
    static const std::string MODIFY_EVALUATION_TEMPLATE;
    static const std::string QUERY_EVALUATION_TEMPLATE;

    static const std::string CREATE_EVALUATION_OF_STORE;
    static const std::string DELETE_EVALUATION_OF_STORE;
    static const std::string MODIFY_EVALUATION_OF_STORE;
    static const std::string QUERY_EVALUATION_OF_STORE;
    static const std::string QUERY_ALL_EVALUATION_OF_STORE;

    static const std::string CREATE_PATROL_RECORD;
    static const std::string DELETE_PATROL_RECORD;
    static const std::string MODIFY_PATROL_RECORD;
    static const std::string QUERY_PATROL_RECORD;
    static const std::string QUERY_ALL_PATROL_RECORD;

    static const std::string CREATE_STORE_SENSOR;
    static const std::string DELETE_STORE_SENSOR;
    static const std::string MODIFY_STORE_SENSOR;
    static const std::string QUERY_STORE_SENSOR;
    static const std::string QUERY_ALL_STORE_SENSOR;

    static const std::string REPORT_STORE_SENSOR;

    typedef struct _ParamInfo
    {
        std::string m_strRemoteAddress;
        std::string m_strRemotePort;
        unsigned int m_uiShakehandOfChannelInterval;
        std::string m_strSelfID;
        unsigned int m_uiCallFuncTimeout;
        unsigned int m_uiThreadOfWorking;
    } ParamInfo;

    PassengerFlowMsgHandler(const ParamInfo &parminfo);
    ~PassengerFlowMsgHandler();

    bool ParseMsgOfCompact(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool CreateDomainHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool RemoveDomainHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyDomainHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryDomainHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryAllDomainHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool AddStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DelStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryAllStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool AddEntranceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DelEntranceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyEntranceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool BindEntranceDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool UnBindEntranceDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ImportPosDataHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryPassengerFlowReportHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool ReportEventHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeleteEventHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyEventHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryEventHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryAllEventHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool CreateGuardStorePlanHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeleteGuardStorePlanHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyGuardStorePlanHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryGuardStorePlanHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryAllGuardStorePlanHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool CreateRegularPatrolHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeleteRegularPatrolPlanHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyRegularPatrolPlanHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryRegularPatrolPlanHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryAllRegularPatrolPlanHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool CreateVIPHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeleteVIPHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyVIPHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryVIPHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryAllVIPHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool CreateVIPConsumeHistoryHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeleteVIPConsumeHistoryHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyVIPConsumeHistoryHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryVIPConsumeHistoryHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool UserJoinStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool UserQuitStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryUserOfStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool CreateEvaluationTemplateHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeleteEvaluationTemplateHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyEvaluationTemplateHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryEvaluationTemplateHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool CreateEvaluationOfStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeleteEvaluationOfStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyEvaluationOfStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryEvaluationOfStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryAllEvaluationOfStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool CreatePatrolRecordHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeletePatrolRecordHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyPatrolRecordHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryPatrolRecordHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryAllPatrolRecordHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool UploadPassengerFlowHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool CreateStoreSensorHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeleteStoreSensorHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyStoreSensorHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryStoreSensorHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryAllStoreSensorHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ReportSensorInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);
    
    void WriteMsg(const std::map<std::string, std::string> &MsgMap, MsgWriter writer, const bool blResult = true, boost::function<void(void*)> PostFunc = NULL);
    
private:
    
    struct DomainInfo
    {
        std::string m_strDomainID;
        std::string m_strDomainName;
        std::string m_strExtend;
        std::string m_strParentDomainID;
        unsigned int m_uiLevel;
    };

    struct StoreInfo
    {
        std::string m_strStoreID;
        std::string m_strStoreName;
        std::string m_strGoodsCategory;
        std::string m_strAddress;
        std::string m_strEntrance;
        std::string m_strAlterEntrance;
        std::string m_strDelEntrance;
        std::string m_strExtend;
        std::string m_strCreateDate;
        std::string m_strDomainID;
        std::string m_strDomainName;
        unsigned int m_uiOpenState;
    };

    struct EntranceInfo
    {
        std::string m_strID;
        std::string m_strName;
        std::list<std::string> m_DeviceIDList;
    };

    struct PassengerFlowInfo 
    {
        std::string m_strDateTime;
        unsigned int m_uiEnterNum;
        unsigned int m_uiLeaveNum;
        unsigned int m_uiStayNum;
    };

    struct EventInfo
    {
        std::string m_strEventID;
        std::string m_strUserID;
        std::string m_strDevID;
        std::string m_strSource;
        std::string m_strSubmitDate;
        std::string m_strExpireDate;
        std::string m_strProcessState;
        std::list<unsigned int> m_uiEventTypeList;
        std::list<std::string> m_strEventHandlerList;
        std::string m_strRemark;
        unsigned int m_uiState;
        std::string m_strExtend;
        std::string m_strCreateDate;
        std::string m_strViewState;
    };

    struct Plan 
    {
        std::string m_strPlanID;
        std::string m_strUserID;
        std::string m_strStoreID;
        std::string m_strStoreName;
        std::list<std::string> m_strEntranceIDList;
        std::string m_strPlanName;
        std::string m_strEnable;
        std::string m_strBeginTime;
        std::string m_strEndTime;
        std::string m_strBeginTime2;
        std::string m_strEndTime2;
    };

    typedef struct
    {
        StoreInfo stinfo;
        std::list<EntranceInfo> etinfolist;
    } StoreAndEntranceInfo;

    struct Patrol 
    {
        std::string m_strPatrolID;
        std::string m_strPatrolName;
        std::string m_strUserID;       
        std::string m_strEnable;
        std::list<std::string> m_strPatrolTimeList;
        std::list<std::string> m_strPatrolHandlerList;
        std::list<StoreAndEntranceInfo> m_SAEList;
        std::string m_strExtend;
    };

    struct VIP
    {
        std::string m_strVipID;
        std::string m_strVipName;
        std::string m_strVipUserID;
        std::string m_strVisitDate;
        unsigned int m_uiVisitTimes;
        std::string m_strCellphone;
        std::string m_strRegisterDate;
        std::string m_strProfilePicture;
        struct ConsumeHistory
        {
            std::string m_strConsumeID;
            std::string m_strGoodName;
            unsigned int m_uiGoodNum;
            std::string m_strSalesman;
            double m_dConsumeAmount;
            std::string m_strConsumeDate;
        };

        std::list<ConsumeHistory> m_ConsumeHistoryList;
    };

    struct UserOfStore 
    {
        std::string m_strUserID;
        std::string m_strUserName;
        std::string m_strStoreID;
        std::string m_strStoreName;
        std::string m_strRole;
    };

    struct EvaluationTemplate
    {
        std::string m_strUserIDOfCreataion;
        std::string m_strEvaluationTmpID;
        std::string m_strEvaluation;
        std::string m_strEvaluationDesc;
        double m_dEvaluationValue;

        std::string m_strEvaDescActive;
        double m_dEvaValueActive;

    };
        
    struct Evaluation
    {
        std::string m_strEvaluationID;
        std::string m_strUserID;
        std::string m_strUserIDOfCheck;
        std::string m_strStoreID;
        unsigned int m_uiCheckStatus;
        std::string m_strEvaluationDate;
        double m_dEvaluationTotalValue;

        std::list<EvaluationTemplate> m_evlist;
    };

    struct PatrolRecord
    {
        std::string m_strPatrolID;
        std::string m_strUserID;
        std::string m_strDevID;
        std::string m_strStoreID;
        std::string m_strPatrolDate;
        std::list<std::string> m_strPatrolPicIDList;
        std::list<std::string> m_strPatrolPicURLList;
        unsigned int m_uiPatrolResult;
        std::string m_strPatrolDesc;
        std::string m_strPlanID;
        std::string m_strEntranceID;
    };

    struct Sensor
    {
        std::string m_strID;
        std::string m_strName;
        unsigned int m_uiType;
        std::string m_strValue;
        std::string m_strStoreID;
        std::string m_strDevID;        
    };

    int RspFuncCommonAction(CommMsgHandler::Packet &pt, int *piRetCode, RspFuncCommon rspfunc);

    bool PreCommonHandler(const std::string &strMsgReceived, int &iRetCode);

    bool CreateDomain(const std::string &strSid, const std::string &strUserID, DomainInfo &dmi);

    bool RemoveDomain(const std::string &strSid, const std::string &strUserID, const std::string &strDomainID);

    bool ModifyDomain(const std::string &strSid, const std::string &strUserID, DomainInfo &dmi);

    bool QueryDomain(const std::string &strSid, const std::string &strUserID, const std::string &strDomainID, DomainInfo &dmi);

    bool QueryAllDomain(const std::string &strSid, const std::string &strUserID, std::list<DomainInfo> &dmilist);


    bool AddStore(const std::string &strSid, const std::string &strUserID, StoreInfo &store);

    bool DelStore(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID);

    bool ModifyStore(const std::string &strSid, const std::string &strUserID, const StoreInfo &store);

    bool QueryStore(const std::string &strSid, const std::string &strUserID, StoreInfo &store, std::list<EntranceInfo> &entranceInfolist, std::list<DomainInfo> &dmilist);

    bool QueryAllStore(const std::string &strSid, const std::string &strUserID, const unsigned int uiBeginIndex, std::list<StoreAndEntranceInfo> &storelist);

    
    bool AddEntrance(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, EntranceInfo &einfo);

    bool DelEntrance(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, const std::string &strEntranceID);

    bool ModifyEntrance(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, 
        const std::string &strEntranceID, const std::string &strEntranceName, 
        EntranceInfo &einfoadd, EntranceInfo &einfodel);

    bool UploadPassengerFlow(const std::string &strSid, const std::string &strDevID, const std::list<PassengerFlowInfo> &pfinfolist);

    bool BindEntranceDevice(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, 
        const std::string &strEntranceID, const std::string &strDevID);

    bool UnbindEntranceDevice(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID,
        const std::string &strEntranceID, const std::string &strDevID);

    bool ImportPosData(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, 
        const double dDealAmount, const unsigned int uiOrderAmount, const unsigned int uiGoodsAmount, const std::string &strDealDate);
    
    bool QueryPassengerFlowReport(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID,
        const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiTimePrecision, std::string &strChartData);

    bool ReportEvent(const std::string &strSid, EventInfo &eventinfo);
    
    bool DeleteEvent(const std::string &strSid, const std::string &strUserID, const std::string &strEventID);

    bool ModifyEvent(const std::string &strSid, EventInfo &eventinfo);

    bool QueryEvent(const std::string &strSid, EventInfo &eventinfo);

    bool QueryAllEvent(const std::string &strSid, const std::string &strUserID, const unsigned int uiBeginIndex, 
        const unsigned int uiProcessState, const std::string &strBeginDate, const std::string &strEndDate, std::list<EventInfo> &eventinfoList);


    bool CreateGuardStorePlan(const std::string &strSid, Plan &plan);

    bool DeleteGuardStorePlan(const std::string &strSid, const std::string &strUserID, const std::string &strPlanID);

    bool ModifyGuardStorePlan(const std::string &strSid, Plan &plan);

    bool QueryGuardStorePlan(const std::string &strSid, Plan &plan);

    bool QueryAllGuardStorePlan(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, std::list<Plan> &planlist);


    bool CreateRegularPatrol(const std::string &strSid, Patrol &pat);
    
    bool DeleteRegularPatrol(const std::string &strSid, const std::string &strUserID, const std::string &strPlanID);

    bool ModifyRegularPatrol(const std::string &strSid, Patrol &pat);

    bool QueryRegularPatrolPlan(const std::string &strSid, Patrol &pat);

    bool QueryAllRegularPatrolPlan(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, std::list<Patrol> &patlist);


    bool CreateVIP(const std::string &strSid, VIP &vip);

    bool DeleteVIP(const std::string &strSid, const std::string &strUserID, const std::string &strVipID);

    bool ModifyVIP(const std::string &strSid, VIP &vip);

    bool QueryVIP(const std::string &strSid, VIP &vip);

    bool QueryAllVIP(const std::string &strSid, const std::string &strUserID, const unsigned int uiBeginIndex, std::list<VIP> &viplist);


    bool CreateVIPConsumeHistory(const std::string &strSid, const std::string &strUserID, const std::string &strVipID, VIP::ConsumeHistory &vph);

    bool DeleteVIPConsumeHistory(const std::string &strSid, const std::string &strUserID, const std::string &strConsumeID);

    bool ModifyVIPConsumeHistory(const std::string &strSid, const std::string &strUserID, VIP::ConsumeHistory &vph);

    bool QueryVIPConsumeHistory(const std::string &strSid, const std::string &strUserID, const std::string &strVipID, 
        const unsigned int uiBeginIndex, std::list<VIP::ConsumeHistory> &vphlist);


    bool UserJoinStore(const std::string &strSid, UserOfStore &us);

    bool UserQuitStore(const std::string &strSid, const std::string &strAdminID, UserOfStore &us);

    bool QueryUserOfStore(const std::string &strSid, const UserOfStore &us, std::list<UserOfStore> &uslist);


    bool CreateEvaluationTemplate(const std::string &strSid, EvaluationTemplate &evt);
    bool DeleteEvaluationTemplate(const std::string &strSid, const std::string &strUserID, const std::string &strEvaluationID);
    bool ModifyEvaluationTemplate(const std::string &strSid, EvaluationTemplate &evt);
    bool QueryEvalutaionTemplate(const std::string &strSid, const std::string &strUserID, std::list<EvaluationTemplate> &evtlist);

    bool CreateEvaluation(const std::string &strSid, Evaluation &ev);
    bool DeleteEvaluation(const std::string &strSid, const std::string &strUserID, const std::string &strEvaluationIDOfStore);
    bool ModifyEvaluation(const std::string &strSid, Evaluation &ev);
    bool QueryEvaluation(const std::string &strSid, const std::string &strUserID, Evaluation &ev);
    bool QueryAllEvaluationOfStore(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, 
        const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiBeginIndex, std::list<Evaluation> &evlist);

    bool CreatePatrolRecord(const std::string &strSid, PatrolRecord &pr);
    bool DeletePatrolRecord(const std::string &strSid, const std::string &strUserID, const std::string &strPatrolID);
    bool ModifyPatrolRecord(const std::string &strSid, PatrolRecord &pr);
    bool QueryPatrolRecord(const std::string &strSid, PatrolRecord &pr);
    bool QueryAllPatrolRecord(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, const std::string &strPlanID,
        const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiBeginIndex, std::list<PatrolRecord> &prlist);

    bool CreateStoreSensor(const std::string &strSid, const std::string &strUserID, Sensor &sr);
    bool DeleteStoreSensor(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, const std::string &strSensorID);
    bool ModifyStoreSensor(const std::string &strSid, const std::string &strUserID, Sensor &sr);
    bool QueryStoreSensor(const std::string &strSid, const std::string &strUserID, Sensor &sr);
    bool QueryAllStoreSensor(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, std::list<Sensor> &srlist);

    bool ReportSensorInfo(const std::string &strSid, const std::string &strDevID, std::list<Sensor> &srlist);

private:
    bool ValidDatetime(const std::string &strDatetime, const bool IsTime = false);

    bool ValidNumber(const std::string &strNumber, const unsigned int uiCount = 1);

    bool GetEntrance(const std::string &strEntrance, std::list<EntranceInfo> &einfolist);

    bool GetEntrance(const std::string &strDeviceIDInfo, EntranceInfo &einfo);

    bool GetPassengerFlowInfo(const std::string &strPassengerFlowInfo, std::list<PassengerFlowInfo> &pfinfolist);

    template<typename T>
    bool ValidType(const std::string &strValue, T &ValueT);

    bool GetEntrance(const std::string &strValue, std::list<StoreAndEntranceInfo> &saelist);

    bool GetValueList(const std::string &strValue, std::list<std::string> &strValueList);

    bool GetValueFromList(const std::string &strValue, boost::function<bool(Json::Value &)> ParseFunc);

private:
    ParamInfo m_ParamInfo;
    boost::shared_ptr<PassengerFlowProtoHandler> m_pInteractiveProtoHandler;

private:
    static const std::string SUCCESS_CODE;
    static const std::string SUCCESS_MSG;
    static const std::string FAILED_CODE;
    static const std::string FAILED_MSG;

};


#endif
