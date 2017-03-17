#ifndef _INTERACTIVE_PROTOE_MANAGEMENT_HANDLER_
#define _INTERACTIVE_PROTOE_MANAGEMENT_HANDLER_

#include <string>
#include <list>
#include <map>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "InteractiveProtoHandler.h"
#include "InteractiveProtocolManagement.pb.h"

/************************************************************************/
/* protobuffer协议序列化和反序列化相关接口与实现
* Author：蔡志勇
* Date：2017-3-7*/
/************************************************************************/

using Management::Interactive::Message::ManagementInteractiveMessage;

class InteractiveProtoManagementHandler : InteractiveProtoHandler
{
public:
    InteractiveProtoManagementHandler();
    ~InteractiveProtoManagementHandler();

    enum ManagementMsgType {
        Init_T = 0,

        AddClusterReq_T = 10000,
        AddClusterRsp_T = 10010,
        DeleteClusterReq_T = 10020,
        DeleteClusterRsp_T = 10030,
        ModifyClusterReq_T = 10040,
        ModifyClusterRsp_T = 10050,
        QueryClusterInfoReq_T = 10060,
        QueryClusterInfoRsp_T = 10070,

        ShakehandClusterReq_T = 10080,
        ShakehandClusterRsp_T = 10090,

        QueryAllClusterReq_T = 10100,
        QueryAllClusterRsp_T = 10110,
        QueryClusterDeviceReq_T = 10220,
        QueryClusterDeviceRsp_T = 10230,
        QueryClusterUserReq_T = 10300,
        QueryClusterUserRsp_T = 10310,

        PushClusterDeviceReq_T = 10500,
        PushClusterDeviceRsp_T = 10510,
        PushClusterUserReq_T = 10520,
        PushClusterUserRsp_T = 10530
    };

    struct Cluster                              //集群信息
    {
        std::string m_strClusterID;
        std::string m_strClusterAddress;
        std::string m_strManagementAddress;
        std::string m_strAliasname;
        std::string m_strCreatedate;
        unsigned int m_uiStatus;
    };

    struct ClusterStatus
    {
        Cluster m_clusterInfo;
        unsigned int m_uiStatus;
    };

    struct AccessedDevice                         //接入设备信息
    {
        std::string m_strDeviceID;
        std::string m_strDeviceName;
        unsigned int m_uiDeviceType;
        std::string m_strLoginTime;
        std::string m_strLogoutTime;
        unsigned int m_uiOnlineDuration;
    };

    struct AccessedUser                           //接入用户信息
    {
        std::string m_strUserID;
        std::string m_strUserName;
        std::string m_strUserAliasname;
        unsigned int m_uiClientType;
        std::string m_strLoginTime;
        std::string m_strLogoutTime;
        unsigned int m_uiOnlineDuration;
    };

    struct DeviceAccessRecord
    {
        unsigned int m_uiSequence;
        std::string m_strClusterID;
        AccessedDevice m_accessedDevice;
        std::string m_strCreateDate;
        unsigned int m_uiStatus;
    };

    struct UserAccessRecord
    {
        unsigned int m_uiSequence;
        std::string m_strClusterID;
        AccessedUser m_accessedUser;
        std::string m_strCreateDate;
        unsigned int m_uiStatus;
    };

    struct Request
    {
        Request() {};
        virtual ~Request() {};

        ManagementMsgType m_MngMsgType;
        unsigned long long m_uiMsgSeq;
        std::string m_strSID;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct Response : Request
    {
        Response() {};
        virtual ~Response() {};

        int m_iRetcode;
        std::string m_strRetMsg;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct AddClusterReq : Request
    {
        Cluster m_clusterInfo;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };


    struct AddClusterRsp : Response
    {
        std::string m_strClusterID;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };


    struct DeleteClusterReq : Request
    {
        std::string m_strClusterID;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct DeleteClusterRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct ModifyClusterReq : Request
    {
        Cluster m_clusterInfo;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct ModifyClusterRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct QueryClusterInfoReq : Request
    {
        std::string m_strClusterID;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct QueryClusterInfoRsp : Response
    {
        Cluster m_clusterInfo;
        unsigned m_uiStatus;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct ShakehandClusterReq : Request
    {
        std::string m_strClusterID;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct ShakehandClusterRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct QueryAllClusterReq : Request
    {
        std::string m_strManagementAddress;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct QueryAllClusterRsp : Response
    {
        std::list<ClusterStatus> m_clusterStatusList;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct QueryClusterDeviceReq : Request
    {
        std::string m_strClusterID;
        std::string m_strBegindate;
        std::string m_strEnddate;
        unsigned int m_uiRecordType;
        unsigned int m_uiBeginIndex;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct QueryClusterDeviceRsp : Response
    {
        std::list<AccessedDevice> m_accessedDeviceInfoList;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct QueryClusterUserReq : Request
    {
        std::string m_strClusterID;
        std::string m_strBegindate;
        std::string m_strEnddate;
        unsigned int m_uiRecordType;
        unsigned int m_uiBeginIndex;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct QueryClusterUserRsp : Response
    {
        std::list<AccessedUser> m_accessedUserInfoList;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct PushClusterDeviceReq : Request
    {
        std::string m_strClusterID;
        std::list<DeviceAccessRecord> m_deviceAccessRecordList;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct PushClusterDeviceRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct PushClusterUserReq : Request
    {
        std::string m_strClusterID;
        std::list<UserAccessRecord> m_userAccessRecordList;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };

    struct PushClusterUserRsp : Response
    {
        std::string m_strValue;

        virtual void Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const;
        virtual void UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg);
    };


    bool GetManagementMsgType(const std::string &strData, ManagementMsgType &msgtype);

    bool SerializeReq(const Request &req, std::string &strOutput);
    bool UnSerializeReq(const std::string &strData, Request &req);

    bool SerializeRsp(const Response &rsp, std::string &strOutput);
    bool UnSerializeRsp(const std::string &strData, Response &rsp);

    bool UnSerializeReqBase(const std::string &strData, Request &req);


private:
    bool AddClusterReq_Serializer(const Request &req, std::string &strOutput);
    bool AddClusterReq_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req);
    bool AddClusterRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool AddClusterRsp_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &rsp);

    bool DeleteClusterReq_Serializer(const Request &req, std::string &strOutput);
    bool DeleteClusterReq_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req);
    bool DeleteClusterRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool DeleteClusterRsp_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &rsp);

    bool ModifyClusterReq_Serializer(const Request &req, std::string &strOutput);
    bool ModifyClusterReq_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req);
    bool ModifyClusterRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ModifyClusterRsp_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &rsp);

    bool QueryClusterInfoReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryClusterInfoReq_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req);
    bool QueryClusterInfoRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryClusterInfoRsp_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &rsp);

    bool ShakehandClusterReq_Serializer(const Request &req, std::string &strOutput);
    bool ShakehandClusterReq_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req);
    bool ShakehandClusterRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool ShakehandClusterRsp_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &rsp);

    bool QueryAllClusterReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryAllClusterReq_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req);
    bool QueryAllClusterRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryAllClusterRsp_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &rsp);

    bool QueryClusterDeviceReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryClusterDeviceReq_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req);
    bool QueryClusterDeviceRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryClusterDeviceRsp_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &rsp);

    bool QueryClusterUserReq_Serializer(const Request &req, std::string &strOutput);
    bool QueryClusterUserReq_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req);
    bool QueryClusterUserRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool QueryClusterUserRsp_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &rsp);

    bool PushClusterDeviceReq_Serializer(const Request &req, std::string &strOutput);
    bool PushClusterDeviceReq_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req);
    bool PushClusterDeviceRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool PushClusterDeviceRsp_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &rsp);

    bool PushClusterUserReq_Serializer(const Request &req, std::string &strOutput);
    bool PushClusterUserReq_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req);
    bool PushClusterUserRsp_Serializer(const Request &rsp, std::string &strOutput);
    bool PushClusterUserRsp_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &rsp);

    typedef boost::function<bool(const Request &req, std::string &strOutput)> Serializer;
    typedef boost::function<bool(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req)> UnSerializer;

    struct SerializeHandler
    {
        Serializer Szr;
        UnSerializer UnSzr;
    };

    std::map<int, SerializeHandler> m_ReqAndRspHandlerMap;

};



#endif
