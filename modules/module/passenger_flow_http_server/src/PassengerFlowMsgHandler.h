#ifndef FILE_HANDLER
#define FILE_HANDLER

#include "NetComm.h"
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include "FCGIManager.h"
#include "CommMsgHandler.h"

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


    bool UploadPassengerFlowHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    void WriteMsg(const std::map<std::string, std::string> &MsgMap, MsgWriter writer, const bool blResult = true, boost::function<void(void*)> PostFunc = NULL);
    
private:
    
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

    int RspFuncCommonAction(CommMsgHandler::Packet &pt, int *piRetCode, RspFuncCommon rspfunc);

    bool PreCommonHandler(const std::string &strMsgReceived, int &iRetCode);

    bool AddStore(const std::string &strSid, const std::string &strUserID, StoreInfo &store);

    bool DelStore(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID);

    bool ModifyStore(const std::string &strSid, const std::string &strUserID, const StoreInfo &store);

    bool QueryStore(const std::string &strSid, const std::string &strUserID, StoreInfo &store, std::list<EntranceInfo> &entranceInfolist);

    bool QueryAllStore(const std::string &strSid, const std::string &strUserID, const unsigned int uiBeginIndex, std::list<StoreInfo> &storelist);

    
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

    bool QueryAllEvent(const std::string &strSid, const std::string &strUserID, const unsigned int uiBeginIndex, std::list<EventInfo> &eventinfoList);

    bool CreateGuardStorePlan(const std::string &strSid, Plan &plan);

    bool DeleteGuardStorePlan(const std::string &strSid, const std::string &strUserID, const std::string &strPlanID);

    bool ModifyGuardStorePlan(const std::string &strSid, Plan &plan);

    bool QueryGuardStorePlan(const std::string &strSid, Plan &plan);

    bool QueryAllGuardStorePlan(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, std::list<Plan> &planlist);

private:
    bool ValidDatetime(const std::string &strDatetime);

    bool GetEntrance(const std::string &strEntrance, std::list<EntranceInfo> &einfolist);

    bool GetEntrance(const std::string &strDeviceIDInfo, EntranceInfo &einfo);

    bool GetPassengerFlowInfo(const std::string &strPassengerFlowInfo, std::list<PassengerFlowInfo> &pfinfolist);

    template<typename T>
    bool ValidType(const std::string &strValue, T &ValueT);

    bool GetValueList(const std::string &strValue, std::list<std::string> &strValueList);

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
