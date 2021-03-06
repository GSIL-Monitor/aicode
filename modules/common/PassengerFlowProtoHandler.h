#ifndef _PASSENGERFLOW_PROTO_HANDLER_
#define _PASSENGERFLOW_PROTO_HANDLER_

#include <map>
#include <list>
#include <boost/function.hpp>

namespace CustomerFlow
{
    namespace Interactive
    {
        namespace Message
        {
            class CustomerFlowMessage;
        }
    }
}
using CustomerFlow::Interactive::Message::CustomerFlowMessage;

class PassengerFlowProtoHandler
{
public:
    PassengerFlowProtoHandler();
    virtual ~PassengerFlowProtoHandler();

    enum CustomerFlowMsgType {
        Init_T = 0,

        CustomerFlowPreHandleReq_T = 9000,
        CustomerFlowPreHandleRsp_T = 9010,
        ShakehandReq_T = 9100,
        ShakehandRsp_T = 9110,

        AddGroupReq_T = 10000,                 //添加群组
        AddGroupRsp_T = 10010,
        DeleteGroupReq_T = 10020,              //删除群组
        DeleteGroupRsp_T = 10030,
        ModifyGroupReq_T = 10040,              //修改群组
        ModifyGroupRsp_T = 10050,
        QueryGroupInfoReq_T = 10060,           //查询群组信息
        QueryGroupInfoRsp_T = 10070,
        QueryAllGroupReq_T = 10080,            //查询所有群组
        QueryAllGroupRsp_T = 10090,

        AddStoreReq_T = 10100,                 //添加店铺
        AddStoreRsp_T = 10110,
        DeleteStoreReq_T = 10120,              //删除店铺
        DeleteStoreRsp_T = 10130,
        ModifyStoreReq_T = 10140,              //修改店铺
        ModifyStoreRsp_T = 10150,
        QueryStoreInfoReq_T = 10160,           //查询店铺信息
        QueryStoreInfoRsp_T = 10170,
        QueryAllStoreReq_T = 10180,            //查询所有店铺
        QueryAllStoreRsp_T = 10190,

        AddEntranceReq_T = 10200,              //添加出入口
        AddEntranceRsp_T = 10210,
        DeleteEntranceReq_T = 10220,           //删除出入口
        DeleteEntranceRsp_T = 10230,
        ModifyEntranceReq_T = 10240,           //修改出入口
        ModifyEntranceRsp_T = 10250,
        AddEntranceDeviceReq_T = 10260,        //出入口绑定设备
        AddEntranceDeviceRsp_T = 10270,
        DeleteEntranceDeviceReq_T = 10280,     //出入口解绑设备
        DeleteEntranceDeviceRsp_T = 10290,

        AddEventReq_T = 10340,                 //添加事件
        AddEventRsp_T = 10350,
        DeleteEventReq_T = 10360,              //删除事件
        DeleteEventRsp_T = 10370,
        ModifyEventReq_T = 10380,              //修改事件
        ModifyEventRsp_T = 10390,
        QueryEventInfoReq_T = 10400,           //查询事件信息
        QueryEventInfoRsp_T = 10410,
        QueryAllEventReq_T = 10420,            //查询所有事件
        QueryAllEventRsp_T = 10430,

        AddSmartGuardStoreReq_T = 10440,       //添加智能看店计划
        AddSmartGuardStoreRsp_T = 10450,
        DeleteSmartGuardStoreReq_T = 10460,    //删除智能看店计划
        DeleteSmartGuardStoreRsp_T = 10470,
        ModifySmartGuardStoreReq_T = 10480,    //修改智能看店计划
        ModifySmartGuardStoreRsp_T = 10490,
        QuerySmartGuardStoreInfoReq_T = 10500, //查询智能看店计划信息
        QuerySmartGuardStoreInfoRsp_T = 10510,
        QueryAllSmartGuardStoreReq_T = 10520,  //查询所有智能看店计划
        QueryAllSmartGuardStoreRsp_T = 10530,

        AddRegularPatrolReq_T = 10540,         //添加定时巡店计划
        AddRegularPatrolRsp_T = 10550,
        DeleteRegularPatrolReq_T = 10560,      //删除定时巡店计划
        DeleteRegularPatrolRsp_T = 10570,
        ModifyRegularPatrolReq_T = 10580,      //修改定时巡店计划
        ModifyRegularPatrolRsp_T = 10590,
        QueryRegularPatrolInfoReq_T = 10600,   //查询定时巡店计划信息
        QueryRegularPatrolInfoRsp_T = 10610,
        QueryAllRegularPatrolReq_T = 10620,    //查询所有定时巡店计划
        QueryAllRegularPatrolRsp_T = 10630,

        UserJoinStoreReq_T = 10700,            //用户加入店铺
        UserJoinStoreRsp_T = 10710,
        UserQuitStoreReq_T = 10720,            //用户退出店铺
        UserQuitStoreRsp_T = 10730,
        QueryStoreAllUserReq_T = 10740,        //查询店铺所有用户
        QueryStoreAllUserRsp_T = 10750,
        QueryCompanyAllUserReq_T = 10760,      //查询公司所有用户
        QueryCompanyAllUserRsp_T = 10770,

        AddVIPCustomerReq_T = 18000,           //添加VIP客户
        AddVIPCustomerRsp_T = 18010,
        DeleteVIPCustomerReq_T = 18020,        //删除VIP客户
        DeleteVIPCustomerRsp_T = 18030,
        ModifyVIPCustomerReq_T = 18040,        //修改VIP客户
        ModifyVIPCustomerRsp_T = 18050,
        QueryVIPCustomerInfoReq_T = 18060,     //查询VIP客户信息
        QueryVIPCustomerInfoRsp_T = 18070,
        QueryAllVIPCustomerReq_T = 18080,      //查询所有VIP客户
        QueryAllVIPCustomerRsp_T = 18090,

        AddVIPConsumeHistoryReq_T = 18100,        //添加VIP客户消费记录
        AddVIPConsumeHistoryRsp_T = 18110,
        DeleteVIPConsumeHistoryReq_T = 18120,     //删除VIP客户消费记录
        DeleteVIPConsumeHistoryRsp_T = 18130,
        ModifyVIPConsumeHistoryReq_T = 18140,     //修改VIP客户消费记录
        ModifyVIPConsumeHistoryRsp_T = 18150,
        QueryAllVIPConsumeHistoryReq_T = 18160,   //查询VIP客户消费记录
        QueryAllVIPConsumeHistoryRsp_T = 18170,

        AddEvaluationTemplateReq_T = 18200,       //添加店铺考评模板
        AddEvaluationTemplateRsp_T = 18210,
        DeleteEvaluationTemplateReq_T = 18220,    //删除店铺考评模板
        DeleteEvaluationTemplateRsp_T = 18230,
        ModifyEvaluationTemplateReq_T = 18240,    //修改店铺考评模板
        ModifyEvaluationTemplateRsp_T = 18250,
        QueryAllEvaluationTemplateReq_T = 18260,  //查询店铺考评模板
        QueryAllEvaluationTemplateRsp_T = 18270,

        AddStoreEvaluationReq_T = 18300,          //添加店铺考评
        AddStoreEvaluationRsp_T = 18310,
        DeleteStoreEvaluationReq_T = 18320,       //删除店铺考评
        DeleteStoreEvaluationRsp_T = 18330,
        ModifyStoreEvaluationReq_T = 18340,       //修改店铺考评
        ModifyStoreEvaluationRsp_T = 18350,
        QueryStoreEvaluationInfoReq_T = 18360,    //查询店铺考评信息
        QueryStoreEvaluationInfoRsp_T = 18370,
        QueryAllStoreEvaluationReq_T = 18380,     //查询所有店铺考评
        QueryAllStoreEvaluationRsp_T = 18390,

        AddRemotePatrolStoreReq_T = 18400,        //添加远程巡店记录
        AddRemotePatrolStoreRsp_T = 18410,
        DeleteRemotePatrolStoreReq_T = 18420,     //删除远程巡店记录
        DeleteRemotePatrolStoreRsp_T = 18430,
        ModifyRemotePatrolStoreReq_T = 18440,     //修改远程巡店记录
        ModifyRemotePatrolStoreRsp_T = 18450,
        QueryRemotePatrolStoreInfoReq_T = 18460,  //查询远程巡店记录信息
        QueryRemotePatrolStoreInfoRsp_T = 18470,
        QueryAllRemotePatrolStoreReq_T = 18480,   //查询所有远程巡店记录
        QueryAllRemotePatrolStoreRsp_T = 18490,

        AddAreaReq_T = 19000,                     //添加区域
        AddAreaRsp_T = 19010,
        DeleteAreaReq_T = 19020,                  //删除区域
        DeleteAreaRsp_T = 19030,
        ModifyAreaReq_T = 19040,                  //修改区域
        ModifyAreaRsp_T = 19050,
        QueryAreaInfoReq_T = 19060,               //查询区域
        QueryAreaInfoRsp_T = 19070,
        QueryAllAreaReq_T = 19080,                //查询所有区域
        QueryAllAreaRsp_T = 19090,

        BindPushClientIDReq_T = 19100,            //用户上报推送参数
        BindPushClientIDRsp_T = 19110,
        UnbindPushClientIDReq_T = 19120,          //解绑推送Client ID
        UnbindPushClientIDRsp_T = 19130,

        AddStoreSensorReq_T = 19200,              //添加店铺传感器
        AddStoreSensorRsp_T = 19210,
        DeleteStoreSensorReq_T = 19220,           //删除店铺传感器
        DeleteStoreSensorRsp_T = 19230,
        ModifyStoreSensorReq_T = 19240,           //修改店铺传感器
        ModifyStoreSensorRsp_T = 19250,
        QueryStoreSensorInfoReq_T = 19260,        //查询店铺传感器信息
        QueryStoreSensorInfoRsp_T = 19270,
        QueryAllStoreSensorReq_T = 19280,         //查询所有店铺传感器
        QueryAllStoreSensorRsp_T = 19290,

        ImportPOSDataReq_T = 20000,               //录入POS数据
        ImportPOSDataRsp_T = 20010,

        QueryCustomerFlowStatisticReq_T = 20900,  //查询客流统计
        QueryCustomerFlowStatisticRsp_T = 20910,

        QueryPatrolResultReportReq_T = 21000,     //查询巡店结果统计
        QueryPatrolResultReportRsp_T = 21010,

        //////////////////////////////////////////////////////////

        ReportCustomerFlowDataReq_T = 30000,      //设备上报客流数据
        ReportCustomerFlowDataRsp_T = 30010,

        ReportSensorInfoReq_T = 30100,            //上报传感器信息
        ReportSensorInfoRsp_T = 30110,

        ReportSensorAlarmInfoReq_T = 30120,       //告警上报
        ReportSensorAlarmInfoRsp_T = 30130,

        QuerySensorAlarmThresholdReq_T = 30140,    //查询告警阈值
        QuerySensorAlarmThresholdRsp_T = 30150,

        RemoveSensorRecordsReq_T = 30160,         //删除传感器监测记录
        RemoveSensorRecordsRsp_T = 30170,

        RemoveSensorAlarmRecordsReq_T = 30180,    //删除传感器告警记录
        RemoveSensorAlarmRecordsRsp_T = 30190,

        QuerySensorRecordsReq_T = 30200,          //查询传感器监测记录
        QuerySensorRecordsRsp_T = 30210,

        QuerySensorAlarmRecordsReq_T = 30220,          //查询传感器告警记录
        QuerySensorAlarmRecordsRsp_T = 30230,

        AddRoleReq_T = 30240,                     //增加角色
        AddRoleRsp_T = 30250,

        RemoveRoleReq_T = 30260,                  //删除角色
        RemoveRoleRsp_T = 30270,

        ModifyRoleReq_T = 30280,                  //修改角色
        ModifyRoleRsp_T = 30290,

        QueryRoleReq_T = 30300,                   //查询角色
        QueryRoleRsp_T = 30310,

        QueryAllRoleReq_T = 30320,                //查询所有角色
        QueryAllRoleRsp_T = 30330,

        UserBindRoleReq_T = 30340,                //用户绑定角色
        UserBindRoleRsp_T = 30350

    };

    struct Area                             //区域
    {
        std::string m_strAreaID;
        std::string m_strAreaName;
        unsigned int m_uiLevel;
        std::string m_strParentAreaID;
        unsigned int m_uiState;
        std::string m_strCreateDate;
        std::string m_strExtend;
    };

    struct Group                            //群组信息
    {
        std::string m_strGroupID;
        std::string m_strGroupName;
        std::string m_strCreateDate;
        unsigned int m_uiState;
    };

    struct Entrance                         //出入口信息
    {
        std::string m_strEntranceID;
        std::string m_strEntranceName;
        std::list<std::string> m_strDeviceIDList;
        std::string m_strPicture;
    };

    struct Store                            //店铺信息
    {
        std::string m_strStoreID;
        std::string m_strStoreName;
        std::string m_strGoodsCategory;
        std::string m_strAddress;
        std::list<Entrance> m_entranceList;
        Area m_area;
        unsigned int m_uiOpenState;
        std::string m_strCreateDate;
        std::string m_strExtend;
        unsigned int m_uiState;
        std::list<std::string> m_strTelephoneList;
    };

    struct UserGroupAssociation             //用户群组关联信息
    {
        std::string m_strUserID;
        std::string m_strGroupID;
        std::string m_strUserRole;
    };

    struct UserStoreAssociation             //用户店铺关联信息
    {
        std::string m_strUserID;
        std::string m_strStoreID;
        std::string m_strUserRole;
    };

    struct Event
    {
        std::string m_strEventID;
        std::string m_strSource;
        std::string m_strSubmitDate;
        std::string m_strExpireDate;
        std::string m_strUserID;
        std::string m_strDeviceID;
        std::string m_strProcessState;
        std::string m_strRemark;
        std::list<unsigned int> m_uiTypeList;
        std::list<std::string> m_strHandlerList;
        unsigned int m_uiViewState;
        std::string m_strCreateDate;
        std::string m_strExtend;
        unsigned int m_uiState;

        std::string m_strStoreID;
        std::string m_strStoreName;
    };

    struct SmartGuardStore
    {
        std::string m_strPlanID;
        std::string m_strStoreID;
        std::string m_strStoreName;
        std::string m_strPlanName;
        std::list<std::string> m_strEntranceIDList;
        std::string m_strEnable;
        std::string m_strBeginTime;
        std::string m_strEndTime;
        std::string m_strBeginTime2;
        std::string m_strEndTime2;
        unsigned int m_uiState;
        std::string m_strCreateDate;
    };

    struct PatrolStoreEntrance              //巡店店铺出入口
    {
        std::string m_strStoreID;
        std::string m_strStoreName;
        std::list<Entrance> m_entranceList;
    };

    struct RegularPatrol                    //定时巡店
    {
        std::string m_strPlanID;
        std::string m_strPlanName;
        std::string m_strEnable;
        std::list<PatrolStoreEntrance> m_storeEntranceList;
        std::list<std::string> m_strPatrolTimeList;
        std::list<std::string> m_strHandlerList;
        unsigned int m_uiState;
        std::string m_strCreateDate;
        std::string m_strExtend;
        std::string m_strUpdateDate;
    };

    struct UserBrief                        //用户简要信息 
    {
        std::string m_strUserID;
        std::string m_strUserName;
        std::string m_strAliasName;
        std::string m_strRole;
    };

    struct VIPConsumeHistory                //VIP客户消费记录
    {
        std::string m_strConsumeID;
        std::string m_strVIPID;
        std::string m_strGoodsName;
        unsigned int m_uiGoodsNumber;
        std::string m_strSalesman;
        double m_dConsumeAmount;
        std::string m_strConsumeDate;
    };

    struct VIPCustomer                      //VIP客户资料
    {
        std::string m_strVIPID;
        std::string m_strProfilePicture;
        std::string m_strVIPName;
        std::string m_strCellphone;
        std::string m_strVisitDate;
        unsigned int m_uiVisitTimes;
        std::string m_strRegisterDate;
        std::list<VIPConsumeHistory> m_consumeHistoryList;
        unsigned int m_uiState;
    };

    struct EvaluationItem                   //店铺评分条目
    {
        std::string m_strItemID;
        std::string m_strItemName;
        std::string m_strDescription;
        double m_dTotalPoint;
    };

    struct EvaluationItemScore              //店铺考评条目得分
    {
        EvaluationItem m_evaluationItem;
        double m_dScore;
        std::string m_strDescription;
        std::list<std::string> m_strPictureList;
    };

    struct StoreEvaluation                  //店铺考评
    {
        std::string m_strEvaluationID;
        std::string m_strStoreID;
        std::string m_strUserIDCreate;
        std::string m_strUserIDCheck;
        double m_dTotalScore;
        unsigned int m_uiCheckStatus;
        std::list<EvaluationItemScore> m_itemScoreList;
        std::list<std::string> m_strPictureList;
        double m_dTotalPoint;
        std::string m_strCreateDate;
    };

    struct EntrancePicture                  //出入口截图
    {
        std::string m_strEntranceID;
        std::list<std::string> m_strPatrolPictureList;
    };

    struct RemotePatrolStore                //远程巡店记录
    {
        std::string m_strPatrolID;
        std::string m_strUserID;
        std::string m_strDeviceID;
        std::list<std::string> m_strEntranceIDList;
        std::string m_strStoreID;
        std::string m_strPlanID;
        std::string m_strPatrolDate;
        std::list<EntrancePicture> m_patrolPictureList;
        std::list<std::string> m_strPatrolPictureList;
        unsigned int m_uiPatrolResult;
        std::string m_strDescription;
        std::string m_strCreateDate;
    };

    struct Sensor                           //传感器
    {
        std::string m_strSensorID;
        std::string m_strSensorName;
        std::string m_strSensorType;
        std::string m_strSensorAlarmThreshold;
        std::string m_strStoreID;
        std::string m_strDeviceID;
        std::string m_strValue;
        unsigned int m_uiState;
        std::string m_strCreateDate;
        std::string m_strSensorKey;
        std::string m_strLocation;
    };

    struct RawCustomerFlow                  //设备客流信息
    {
        unsigned int m_uiEnterNumber;
        unsigned int m_uiLeaveNumber;
        unsigned int m_uiStayNumber;
        std::string m_strDataTime;
    };

    struct Request
    {
        Request() {};
        virtual ~Request() {};

        CustomerFlowMsgType m_MsgType;
        unsigned long long m_uiMsgSeq;
        std::string m_strSID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct Response : Request
    {
        Response() {};
        virtual ~Response() {};

        int m_iRetcode;
        std::string m_strRetMsg;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct CustomerFlowPreHandleReq : Request
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct CustomerFlowPreHandleRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ShakehandReq : Request
    {
        std::string m_strID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ShakehandRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddAreaReq : Request
    {
        std::string m_strUserID;
        Area m_areaInfo;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddAreaRsp : Response
    {
        std::string m_strAreaID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteAreaReq : Request
    {
        std::string m_strUserID;
        std::string m_strAreaID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteAreaRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyAreaReq : Request
    {
        std::string m_strUserID;
        Area m_areaInfo;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyAreaRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAreaInfoReq : Request
    {
        std::string m_strUserID;
        std::string m_strAreaID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAreaInfoRsp : Response
    {
        Area m_areaInfo;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllAreaReq : Request
    {
        std::string m_strUserID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllAreaRsp : Response
    {
        std::list<Area> m_areaList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct BindPushClientIDReq : Request
    {
        std::string m_strUserID;
        std::string m_strClientID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct BindPushClientIDRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct UnbindPushClientIDReq : Request
    {
        std::string m_strUserID;
        std::string m_strClientID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct UnbindPushClientIDRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddGroupReq : Request
    {
        std::string m_strUserID;
        Group m_groupInfo;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddGroupRsp : Response
    {
        std::string m_strGroupID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteGroupReq : Request
    {
        std::string m_strUserID;
        std::string m_strGroupID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteGroupRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyGroupReq : Request
    {
        std::string m_strUserID;
        Group m_groupInfo;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyGroupRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryGroupInfoReq : Request
    {
        std::string m_strUserID;
        std::string m_strGroupID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryGroupInfoRsp : Response
    {
        Group m_groupInfo;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllGroupReq : Request
    {
        std::string m_strUserID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllGroupRsp : Response
    {
        std::list<Group> m_groupList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddStoreReq : Request
    {
        std::string m_strUserID;
        Store m_storeInfo;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddStoreRsp : Response
    {
        std::string m_strStoreID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteStoreReq : Request
    {
        std::string m_strUserID;
        std::string m_strStoreID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteStoreRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyStoreReq : Request
    {
        std::string m_strUserID;
        Store m_storeInfo;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyStoreRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryStoreInfoReq : Request
    {
        std::string m_strUserID;
        std::string m_strStoreID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryStoreInfoRsp : Response
    {
        Store m_storeInfo;
        std::list<Area> m_areaList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllStoreReq : Request
    {
        std::string m_strUserID;
        std::string m_strAreaID;
        unsigned int m_uiOpenState;
        unsigned int m_uiBeginIndex;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllStoreRsp : Response
    {
        std::list<Store> m_storeList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddEntranceReq : Request
    {
        std::string m_strUserID;
        std::string m_strStoreID;
        Entrance m_entranceInfo;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddEntranceRsp : Response
    {
        std::string m_strEntranceID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteEntranceReq : Request
    {
        std::string m_strUserID;
        std::string m_strStoreID;
        std::string m_strEntranceID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteEntranceRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyEntranceReq : Request
    {
        std::string m_strUserID;
        std::string m_strStoreID;
        Entrance m_entranceInfo;
        std::list<std::string> m_strAddedDeviceIDList;
        std::list<std::string> m_strDeletedDeviceIDList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyEntranceRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddEntranceDeviceReq : Request
    {
        std::string m_strUserID;
        std::string m_strStoreID;
        std::string m_strEntranceID;
        std::string m_strDeviceID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddEntranceDeviceRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteEntranceDeviceReq : Request
    {
        std::string m_strUserID;
        std::string m_strStoreID;
        std::string m_strEntranceID;
        std::string m_strDeviceID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteEntranceDeviceRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddEventReq : Request
    {
        Event m_eventInfo;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddEventRsp : Response
    {
        std::string m_strEventID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteEventReq : Request
    {
        std::string m_strUserID;
        std::string m_strEventID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteEventRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyEventReq : Request
    {
        std::string m_strUserID;
        Event m_eventInfo;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyEventRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryEventInfoReq : Request
    {
        std::string m_strUserID;
        std::string m_strEventID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryEventInfoRsp : Response
    {
        Event m_eventInfo;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllEventReq : Request
    {
        std::string m_strUserID;
        unsigned int m_uiProcessState;
        unsigned int m_uiRelation;
        std::string m_strBeginDate;
        std::string m_strEndDate;
        unsigned int m_uiBeginIndex;
        unsigned int m_uiEventType;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllEventRsp : Response
    {
        std::list<Event> m_eventList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddSmartGuardStoreReq : Request
    {
        std::string m_strUserID;
        SmartGuardStore m_smartGuardStore;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddSmartGuardStoreRsp : Response
    {
        std::string m_strPlanID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteSmartGuardStoreReq : Request
    {
        std::string m_strUserID;
        std::string m_strPlanID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteSmartGuardStoreRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifySmartGuardStoreReq : Request
    {
        std::string m_strUserID;
        SmartGuardStore m_smartGuardStore;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifySmartGuardStoreRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QuerySmartGuardStoreInfoReq : Request
    {
        std::string m_strUserID;
        std::string m_strPlanID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QuerySmartGuardStoreInfoRsp : Response
    {
        SmartGuardStore m_smartGuardStore;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllSmartGuardStoreReq : Request
    {
        std::string m_strUserID;
        std::string m_strDeviceID;
        unsigned int m_uiBeginIndex;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllSmartGuardStoreRsp : Response
    {
        std::list<SmartGuardStore> m_planList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddRegularPatrolReq : Request
    {
        std::string m_strUserID;
        RegularPatrol m_regularPatrol;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddRegularPatrolRsp : Response
    {
        std::string m_strPlanID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteRegularPatrolReq : Request
    {
        std::string m_strUserID;
        std::string m_strPlanID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteRegularPatrolRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyRegularPatrolReq : Request
    {
        std::string m_strUserID;
        RegularPatrol m_regularPatrol;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyRegularPatrolRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryRegularPatrolInfoReq : Request
    {
        std::string m_strUserID;
        std::string m_strPlanID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryRegularPatrolInfoRsp : Response
    {
        RegularPatrol m_regularPatrol;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllRegularPatrolReq : Request
    {
        std::string m_strUserID;
        std::string m_strDeviceID;
        unsigned int m_uiBeginIndex;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllRegularPatrolRsp : Response
    {
        std::list<RegularPatrol> m_planList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct UserJoinStoreReq : Request
    {
        std::string m_strAdministratorID;
        std::string m_strUserID;
        std::string m_strStoreID;
        std::string m_strRole;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct UserJoinStoreRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct UserQuitStoreReq : Request
    {
        std::string m_strAdministratorID;
        std::string m_strUserID;
        std::string m_strStoreID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct UserQuitStoreRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryStoreAllUserReq : Request
    {
        std::string m_strUserID;
        std::string m_strStoreID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryStoreAllUserRsp : Response
    {
        std::list<UserBrief> m_userList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryCompanyAllUserReq : Request
    {
        std::string m_strUserID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryCompanyAllUserRsp : Response
    {
        std::list<UserBrief> m_userList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddVIPCustomerReq : Request
    {
        std::string m_strUserID;
        VIPCustomer m_customerInfo;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddVIPCustomerRsp : Response
    {
        std::string m_strVIPID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteVIPCustomerReq : Request
    {
        std::string m_strUserID;
        std::string m_strVIPID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteVIPCustomerRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyVIPCustomerReq : Request
    {
        std::string m_strUserID;
        VIPCustomer m_customerInfo;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyVIPCustomerRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryVIPCustomerInfoReq : Request
    {
        std::string m_strUserID;
        std::string m_strVIPID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryVIPCustomerInfoRsp : Response
    {
        VIPCustomer m_customerInfo;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllVIPCustomerReq : Request
    {
        std::string m_strUserID;
        unsigned int m_uiBeginIndex;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllVIPCustomerRsp : Response
    {
        std::list<VIPCustomer> m_customerList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddVIPConsumeHistoryReq : Request
    {
        std::string m_strUserID;
        VIPConsumeHistory m_consumeHistory;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddVIPConsumeHistoryRsp : Response
    {
        std::string m_strConsumeID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteVIPConsumeHistoryReq : Request
    {
        std::string m_strUserID;
        std::string m_strConsumeID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteVIPConsumeHistoryRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyVIPConsumeHistoryReq : Request
    {
        std::string m_strUserID;
        VIPConsumeHistory m_consumeHistory;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyVIPConsumeHistoryRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllVIPConsumeHistoryReq : Request
    {
        std::string m_strUserID;
        std::string m_strVIPID;
        unsigned int m_uiBeginIndex;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllVIPConsumeHistoryRsp : Response
    {
        std::list<VIPConsumeHistory> m_consumeHistoryList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddEvaluationTemplateReq : Request
    {
        std::string m_strUserID;
        EvaluationItem m_evaluationItem;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddEvaluationTemplateRsp : Response
    {
        std::string m_strEvaluationID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteEvaluationTemplateReq : Request
    {
        std::string m_strUserID;
        std::string m_strEvaluationID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteEvaluationTemplateRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyEvaluationTemplateReq : Request
    {
        std::string m_strUserID;
        EvaluationItem m_evaluationItem;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyEvaluationTemplateRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllEvaluationTemplateReq : Request
    {
        std::string m_strUserID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllEvaluationTemplateRsp : Response
    {
        std::list<EvaluationItem> m_evaluationItemList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddStoreEvaluationReq : Request
    {
        StoreEvaluation m_storeEvaluation;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddStoreEvaluationRsp : Response
    {
        std::string m_strEvaluationID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteStoreEvaluationReq : Request
    {
        std::string m_strUserID;
        std::string m_strEvaluationID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteStoreEvaluationRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyStoreEvaluationReq : Request
    {
        std::string m_strUserID;
        StoreEvaluation m_storeEvaluation;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyStoreEvaluationRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryStoreEvaluationInfoReq : Request
    {
        std::string m_strUserID;
        std::string m_strStoreID;
        std::string m_strEvaluationID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryStoreEvaluationInfoRsp : Response
    {
        StoreEvaluation m_storeEvaluation;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllStoreEvaluationReq : Request
    {
        std::string m_strUserID;
        std::string m_strStoreID;
        std::string m_strBeginDate;
        std::string m_strEndDate;
        unsigned int m_uiBeginIndex;
        unsigned int m_uiCheckStatus;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllStoreEvaluationRsp : Response
    {
        std::list<StoreEvaluation> m_storeEvaluationList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddRemotePatrolStoreReq : Request
    {
        RemotePatrolStore m_patrolStore;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddRemotePatrolStoreRsp : Response
    {
        std::string m_strPatrolID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteRemotePatrolStoreReq : Request
    {
        std::string m_strUserID;
        std::string m_strPatrolID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteRemotePatrolStoreRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyRemotePatrolStoreReq : Request
    {
        std::string m_strUserID;
        RemotePatrolStore m_patrolStore;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyRemotePatrolStoreRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryRemotePatrolStoreInfoReq : Request
    {
        std::string m_strUserID;
        std::string m_strPatrolID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryRemotePatrolStoreInfoRsp : Response
    {
        RemotePatrolStore m_patrolStore;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllRemotePatrolStoreReq : Request
    {
        std::string m_strUserID;
        std::string m_strStoreID;
        unsigned int m_uiPatrolResult;
        unsigned int m_uiPlanFlag;
        std::string m_strPlanID;
        std::string m_strBeginDate;
        std::string m_strEndDate;
        unsigned int m_uiBeginIndex;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllRemotePatrolStoreRsp : Response
    {
        std::list<RemotePatrolStore> m_patrolStoreList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddStoreSensorReq :Request
    {
        std::string m_strUserID;
        Sensor m_sensorInfo;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddStoreSensorRsp :Response
    {
        std::string m_strSensorID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteStoreSensorReq :Request
    {
        std::string m_strUserID;
        std::string m_strStoreID;
        std::string m_strSensorID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct DeleteStoreSensorRsp :Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyStoreSensorReq :Request
    {
        std::string m_strUserID;
        Sensor m_sensorInfo;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyStoreSensorRsp :Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryStoreSensorInfoReq :Request
    {
        std::string m_strUserID;
        std::string m_strSensorID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryStoreSensorInfoRsp :Response
    {
        Sensor m_sensorInfo;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllStoreSensorReq :Request
    {
        std::string m_strUserID;
        std::string m_strStoreID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllStoreSensorRsp :Response
    {
        std::list<Sensor> m_sensorList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ImportPOSDataReq : Request
    {
        std::string m_strUserID;
        std::string m_strStoreID;
        unsigned int m_uiOrderAmount;
        unsigned int m_uiGoodsAmount;
        double m_dDealAmount;
        std::string m_strDealDate;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ImportPOSDataRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryCustomerFlowStatisticReq : Request
    {
        std::string m_strUserID;
        std::string m_strStoreID;
        std::string m_strBeginDate;
        std::string m_strEndDate;
        unsigned int m_uiTimePrecision;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryCustomerFlowStatisticRsp : Response
    {
        std::string m_strChartData;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryPatrolResultReportReq : Request
    {
        std::string m_strUserID;
        std::string m_strStoreID;
        std::string m_strBeginDate;
        std::string m_strEndDate;
        unsigned int m_uiPatrolResult;
        std::string m_strPatrolUserID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryPatrolResultReportRsp : Response
    {
        std::string m_strChartData;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ReportCustomerFlowDataReq : Request
    {
        std::string m_strDeviceID;
        std::list<RawCustomerFlow> m_customerFlowList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ReportCustomerFlowDataRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ReportSensorInfoReq : Request
    {
        std::string m_strDeviceID;
        std::list<Sensor> m_sensorList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ReportSensorInfoRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ReportSensorAlarmInfoReq : Request
    {
        Sensor m_sensorInfo;
        unsigned int m_uiRecover;
        std::string m_strFileID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ReportSensorAlarmInfoRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QuerySensorAlarmThresholdReq : Request
    {
        std::string m_strDeviceID;
        
        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QuerySensorAlarmThresholdRsp : Response
    {
        std::list<Sensor> m_sensorList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct RemoveSensorRecordsReq : Request
    {
        std::string m_strUserID;
        std::list<std::string> m_strRecordIDList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct RemoveSensorRecordsRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct RemoveSensorAlarmRecordsReq : Request
    {
        std::string m_strUserID;
        std::list<std::string> m_strRecordIDList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct RemoveSensorAlarmRecordsRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QuerySensorRecordsReq : Request
    {
        std::string m_strUserID;
        std::string m_strStoreID;
        std::string m_strSensorID;
        std::string m_strSensorType;
        std::string m_strBeginDate;
        std::string m_strEndDate;
        unsigned int m_uiBeginIndex;
		unsigned int m_uiTimeRangeType;
		unsigned int m_uiTimeRangeBase;
		std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QuerySensorRecordsRsp : Response
    {
        std::list<std::string> m_strRecordIDList;
        std::list<Sensor> m_sensorList;
		unsigned int m_uiRealRecordNum;
		std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QuerySensorAlarmRecordsReq : Request
    {
        std::string m_strUserID;
        std::string m_strStoreID;
        std::string m_strSensorID;
        std::string m_strSensorType;
        unsigned int m_uiRecover;
        std::string m_strBeginDate;
        std::string m_strEndDate;
        unsigned int m_uiBeginIndex;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct SensorAlarmRecord
    {
        Sensor m_sensorInfo;
        unsigned int m_uiRecover;
        std::string m_strFileID;
        std::string m_strRecordID;
    };

    struct QuerySensorAlarmRecordsRsp : Response
    {        
        std::list<SensorAlarmRecord> m_sardList;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct Permission
    {
        unsigned int m_uiFuncID;
        std::string m_strAccess;
        std::string m_strExtend;
    };

    struct Role
    {
        std::string m_strRoleID;
        unsigned int m_uiState;
        std::string m_strCreateDate;
        std::string m_strUpdateDate;
        std::list<Permission> m_pmlist;
    };

    struct AddRoleReq : Request
    {
        std::string m_strUserID;
        std::string m_strRoleIDNew;
        std::string m_strRoleIDOld;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct AddRoleRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct RemoveRoleReq : Request
    {
        std::string m_strUserID;
        std::string m_strRoleID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct RemoveRoleRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyRoleReq : Request
    {
        std::string m_strUserID;
        Role m_role;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct ModifyRoleRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryRoleReq : Request
    {
        std::string m_strUserID;
        std::string m_strRoleID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryRoleRsp : Response
    {
        Role m_role;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllRoleReq : Request
    {
        std::string m_strUserID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct QueryAllRoleRsp : Response
    {
        std::list<Role> m_rolelist;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct UserBindRoleReq : Request
    {
        std::string m_strUserID;
        std::string m_strRoleID;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    struct UserBindRoleRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(CustomerFlowMessage &message) const;
        virtual void UnSerializer(const CustomerFlowMessage &message);
    };

    
    bool GetCustomerFlowMsgType(const std::string &strData, CustomerFlowMsgType &msgtype);

    bool SerializeReq(const Request &req, std::string &strOutput);
    bool UnSerializeReq(const std::string &strData, Request &req);

    bool SerializeRsp(const Response &rsp, std::string &strOutput);
    bool UnSerializeRsp(const std::string &strData, Response &rsp);

    bool UnSerializeReqBase(const std::string &strData, Request &req);


private:
    bool CustomerFlowPreHandleReq_Serializer(const Request &req, std::string &strOutput);
    bool CustomerFlowPreHandleReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool CustomerFlowPreHandleRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool CustomerFlowPreHandleRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ShakehandReq_Serializer(const Request &req, std::string &strOutput);
    bool ShakehandReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ShakehandRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ShakehandRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool AddAreaReq_Serializer(const Request &req, std::string &strOutput);
    bool AddAreaReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool AddAreaRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool AddAreaRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool DeleteAreaReq_Serializer(const Request &req, std::string &strOutput);
    bool DeleteAreaReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool DeleteAreaRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool DeleteAreaRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ModifyAreaReq_Serializer(const Request &req, std::string &strOutput);
    bool ModifyAreaReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ModifyAreaRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ModifyAreaRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryAreaInfoReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryAreaInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryAreaInfoRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryAreaInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryAllAreaReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryAllAreaReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryAllAreaRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryAllAreaRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool BindPushClientIDReq_Serializer(const Request &req, std::string &strOutput);
    bool BindPushClientIDReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool BindPushClientIDRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool BindPushClientIDRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool UnbindPushClientIDReq_Serializer(const Request &req, std::string &strOutput);
    bool UnbindPushClientIDReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool UnbindPushClientIDRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool UnbindPushClientIDRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool AddGroupReq_Serializer(const Request &req, std::string &strOutput);
    bool AddGroupReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool AddGroupRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool AddGroupRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool DeleteGroupReq_Serializer(const Request &req, std::string &strOutput);
    bool DeleteGroupReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool DeleteGroupRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool DeleteGroupRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ModifyGroupReq_Serializer(const Request &req, std::string &strOutput);
    bool ModifyGroupReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ModifyGroupRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ModifyGroupRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryGroupInfoReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryGroupInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryGroupInfoRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryGroupInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryAllGroupReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryAllGroupReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryAllGroupRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryAllGroupRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool AddStoreReq_Serializer(const Request &req, std::string &strOutput);
    bool AddStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool AddStoreRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool AddStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool DeleteStoreReq_Serializer(const Request &req, std::string &strOutput);
    bool DeleteStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool DeleteStoreRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool DeleteStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ModifyStoreReq_Serializer(const Request &req, std::string &strOutput);
    bool ModifyStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ModifyStoreRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ModifyStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryStoreInfoReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryStoreInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryStoreInfoRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryStoreInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryAllStoreReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryAllStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryAllStoreRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryAllStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool AddEntranceReq_Serializer(const Request &req, std::string &strOutput);
    bool AddEntranceReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool AddEntranceRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool AddEntranceRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool DeleteEntranceReq_Serializer(const Request &req, std::string &strOutput);
    bool DeleteEntranceReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool DeleteEntranceRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool DeleteEntranceRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ModifyEntranceReq_Serializer(const Request &req, std::string &strOutput);
    bool ModifyEntranceReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ModifyEntranceRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ModifyEntranceRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool AddEntranceDeviceReq_Serializer(const Request &req, std::string &strOutput);
    bool AddEntranceDeviceReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool AddEntranceDeviceRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool AddEntranceDeviceRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool DeleteEntranceDeviceReq_Serializer(const Request &req, std::string &strOutput);
    bool DeleteEntranceDeviceReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool DeleteEntranceDeviceRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool DeleteEntranceDeviceRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool AddEventReq_Serializer(const Request &req, std::string &strOutput);
    bool AddEventReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool AddEventRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool AddEventRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool DeleteEventReq_Serializer(const Request &req, std::string &strOutput);
    bool DeleteEventReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool DeleteEventRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool DeleteEventRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ModifyEventReq_Serializer(const Request &req, std::string &strOutput);
    bool ModifyEventReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ModifyEventRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ModifyEventRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryEventInfoReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryEventInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryEventInfoRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryEventInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryAllEventReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryAllEventReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryAllEventRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryAllEventRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool AddSmartGuardStoreReq_Serializer(const Request &req, std::string &strOutput);
    bool AddSmartGuardStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool AddSmartGuardStoreRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool AddSmartGuardStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool DeleteSmartGuardStoreReq_Serializer(const Request &req, std::string &strOutput);
    bool DeleteSmartGuardStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool DeleteSmartGuardStoreRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool DeleteSmartGuardStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ModifySmartGuardStoreReq_Serializer(const Request &req, std::string &strOutput);
    bool ModifySmartGuardStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ModifySmartGuardStoreRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ModifySmartGuardStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QuerySmartGuardStoreInfoReq_Serializer(const Request &req, std::string &strOutput);
    bool QuerySmartGuardStoreInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QuerySmartGuardStoreInfoRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QuerySmartGuardStoreInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryAllSmartGuardStoreReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryAllSmartGuardStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryAllSmartGuardStoreRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryAllSmartGuardStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool AddRegularPatrolReq_Serializer(const Request &req, std::string &strOutput);
    bool AddRegularPatrolReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool AddRegularPatrolRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool AddRegularPatrolRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool DeleteRegularPatrolReq_Serializer(const Request &req, std::string &strOutput);
    bool DeleteRegularPatrolReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool DeleteRegularPatrolRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool DeleteRegularPatrolRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ModifyRegularPatrolReq_Serializer(const Request &req, std::string &strOutput);
    bool ModifyRegularPatrolReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ModifyRegularPatrolRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ModifyRegularPatrolRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryRegularPatrolInfoReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryRegularPatrolInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryRegularPatrolInfoRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryRegularPatrolInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryAllRegularPatrolReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryAllRegularPatrolReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryAllRegularPatrolRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryAllRegularPatrolRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool UserJoinStoreReq_Serializer(const Request &req, std::string &strOutput);
    bool UserJoinStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool UserJoinStoreRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool UserJoinStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool UserQuitStoreReq_Serializer(const Request &req, std::string &strOutput);
    bool UserQuitStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool UserQuitStoreRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool UserQuitStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryStoreAllUserReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryStoreAllUserReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryStoreAllUserRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryStoreAllUserRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryCompanyAllUserReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryCompanyAllUserReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryCompanyAllUserRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryCompanyAllUserRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool AddVIPCustomerReq_Serializer(const Request &req, std::string &strOutput);
    bool AddVIPCustomerReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool AddVIPCustomerRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool AddVIPCustomerRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool DeleteVIPCustomerReq_Serializer(const Request &req, std::string &strOutput);
    bool DeleteVIPCustomerReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool DeleteVIPCustomerRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool DeleteVIPCustomerRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ModifyVIPCustomerReq_Serializer(const Request &req, std::string &strOutput);
    bool ModifyVIPCustomerReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ModifyVIPCustomerRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ModifyVIPCustomerRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryVIPCustomerInfoReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryVIPCustomerInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryVIPCustomerInfoRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryVIPCustomerInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryAllVIPCustomerReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryAllVIPCustomerReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryAllVIPCustomerRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryAllVIPCustomerRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool AddVIPConsumeHistoryReq_Serializer(const Request &req, std::string &strOutput);
    bool AddVIPConsumeHistoryReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool AddVIPConsumeHistoryRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool AddVIPConsumeHistoryRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool DeleteVIPConsumeHistoryReq_Serializer(const Request &req, std::string &strOutput);
    bool DeleteVIPConsumeHistoryReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool DeleteVIPConsumeHistoryRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool DeleteVIPConsumeHistoryRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ModifyVIPConsumeHistoryReq_Serializer(const Request &req, std::string &strOutput);
    bool ModifyVIPConsumeHistoryReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ModifyVIPConsumeHistoryRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ModifyVIPConsumeHistoryRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryAllVIPConsumeHistoryReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryAllVIPConsumeHistoryReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryAllVIPConsumeHistoryRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryAllVIPConsumeHistoryRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool AddEvaluationTemplateReq_Serializer(const Request &req, std::string &strOutput);
    bool AddEvaluationTemplateReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool AddEvaluationTemplateRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool AddEvaluationTemplateRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool DeleteEvaluationTemplateReq_Serializer(const Request &req, std::string &strOutput);
    bool DeleteEvaluationTemplateReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool DeleteEvaluationTemplateRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool DeleteEvaluationTemplateRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ModifyEvaluationTemplateReq_Serializer(const Request &req, std::string &strOutput);
    bool ModifyEvaluationTemplateReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ModifyEvaluationTemplateRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ModifyEvaluationTemplateRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryAllEvaluationTemplateReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryAllEvaluationTemplateReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryAllEvaluationTemplateRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryAllEvaluationTemplateRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool AddStoreEvaluationReq_Serializer(const Request &req, std::string &strOutput);
    bool AddStoreEvaluationReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool AddStoreEvaluationRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool AddStoreEvaluationRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool DeleteStoreEvaluationReq_Serializer(const Request &req, std::string &strOutput);
    bool DeleteStoreEvaluationReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool DeleteStoreEvaluationRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool DeleteStoreEvaluationRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ModifyStoreEvaluationReq_Serializer(const Request &req, std::string &strOutput);
    bool ModifyStoreEvaluationReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ModifyStoreEvaluationRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ModifyStoreEvaluationRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryStoreEvaluationInfoReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryStoreEvaluationInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryStoreEvaluationInfoRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryStoreEvaluationInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryAllStoreEvaluationReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryAllStoreEvaluationReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryAllStoreEvaluationRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryAllStoreEvaluationRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool AddRemotePatrolStoreReq_Serializer(const Request &req, std::string &strOutput);
    bool AddRemotePatrolStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool AddRemotePatrolStoreRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool AddRemotePatrolStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool DeleteRemotePatrolStoreReq_Serializer(const Request &req, std::string &strOutput);
    bool DeleteRemotePatrolStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool DeleteRemotePatrolStoreRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool DeleteRemotePatrolStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ModifyRemotePatrolStoreReq_Serializer(const Request &req, std::string &strOutput);
    bool ModifyRemotePatrolStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ModifyRemotePatrolStoreRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ModifyRemotePatrolStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryRemotePatrolStoreInfoReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryRemotePatrolStoreInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryRemotePatrolStoreInfoRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryRemotePatrolStoreInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryAllRemotePatrolStoreReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryAllRemotePatrolStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryAllRemotePatrolStoreRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryAllRemotePatrolStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool AddStoreSensorReq_Serializer(const Request &req, std::string &strOutput);
    bool AddStoreSensorReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool AddStoreSensorRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool AddStoreSensorRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool DeleteStoreSensorReq_Serializer(const Request &req, std::string &strOutput);
    bool DeleteStoreSensorReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool DeleteStoreSensorRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool DeleteStoreSensorRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ModifyStoreSensorReq_Serializer(const Request &req, std::string &strOutput);
    bool ModifyStoreSensorReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ModifyStoreSensorRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ModifyStoreSensorRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryStoreSensorInfoReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryStoreSensorInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryStoreSensorInfoRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryStoreSensorInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryAllStoreSensorReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryAllStoreSensorReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryAllStoreSensorRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryAllStoreSensorRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ImportPOSDataReq_Serializer(const Request &req, std::string &strOutput);
    bool ImportPOSDataReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ImportPOSDataRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ImportPOSDataRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryCustomerFlowStatisticReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryCustomerFlowStatisticReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryCustomerFlowStatisticRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryCustomerFlowStatisticRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryPatrolResultReportReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryPatrolResultReportReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryPatrolResultReportRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryPatrolResultReportRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ReportCustomerFlowDataReq_Serializer(const Request &req, std::string &strOutput);
    bool ReportCustomerFlowDataReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ReportCustomerFlowDataRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ReportCustomerFlowDataRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ReportSensorInfoReq_Serializer(const Request &req, std::string &strOutput);
    bool ReportSensorInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ReportSensorInfoRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ReportSensorInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ReportSensorAlarmInfoReq_Serializer(const Request &req, std::string &strOutput);
    bool ReportSensorAlarmInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ReportSensorAlarmInfoRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ReportSensorAlarmInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QuerySensorAlarmThresholdReq_Serializer(const Request &req, std::string &strOutput);
    bool QuerySensorAlarmThresholdReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QuerySensorAlarmThresholdRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QuerySensorAlarmThresholdRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool RemoveSensorRecordsReq_Serializer(const Request &req, std::string &strOutput);
    bool RemoveSensorRecordsReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool RemoveSensorRecordsRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool RemoveSensorRecordsRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool RemoveSensorAlarmRecordsReq_Serializer(const Request &req, std::string &strOutput);
    bool RemoveSensorAlarmRecordsReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool RemoveSensorAlarmRecordsRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool RemoveSensorAlarmRecordsRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QuerySensorRecordsReq_Serializer(const Request &req, std::string &strOutput);
    bool QuerySensorRecordsReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QuerySensorRecordsRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QuerySensorRecordsRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);
    
    bool QuerySensorAlarmRecordsReq_Serializer(const Request &req, std::string &strOutput);
    bool QuerySensorAlarmRecordsReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QuerySensorAlarmRecordsRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QuerySensorAlarmRecordsRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);


    bool AddRoleReq_Serializer(const Request &req, std::string &strOutput);
    bool AddRoleReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool AddRoleRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool AddRoleRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool RemoveRoleReq_Serializer(const Request &req, std::string &strOutput);
    bool RemoveRoleReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool RemoveRoleRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool RemoveRoleRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool ModifyRoleReq_Serializer(const Request &req, std::string &strOutput);
    bool ModifyRoleReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool ModifyRoleRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ModifyRoleRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryRoleReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryRoleReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryRoleRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryRoleRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool QueryAllRoleReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryAllRoleReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool QueryAllRoleRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryAllRoleRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);

    bool UserBindRoleReq_Serializer(const Request &req, std::string &strOutput);
    bool UserBindRoleReq_UnSerializer(const CustomerFlowMessage &message, Request &req);
    bool UserBindRoleRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool UserBindRoleRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp);


    typedef boost::function<bool(const Request &req, std::string &strOutput)> Serializer;
    typedef boost::function<bool(const CustomerFlowMessage &message, Request &req)> UnSerializer;

    struct SerializeHandler
    {
        Serializer Szr;
        UnSerializer UnSzr;
    };

    std::map<int, SerializeHandler> m_ReqAndRspHandlerMap;

};

#endif