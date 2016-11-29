#ifndef _INTERACTIVE_PROTOE_HANDLER_
#define _INTERACTIVE_PROTOE_HANDLER_

#include <string>
#include <list>
#include <map>
#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace Interactive
{
    namespace Message
    {
        class InteractiveMessage;
    }
}
using Interactive::Message::InteractiveMessage;

class InteractiveProtoHandler
{
public:
    InteractiveProtoHandler();
    ~InteractiveProtoHandler();

    enum MsgType
    {
        Init_T = 0,
        GetAccessAddressReq_DEV_T = 10000,       //设备获取接入服务器地址
        GetAccessAddressRsp_DEV_T = 10010,
        LoginReq_DEV_T = 10020,                         //设备登录接入管理服务器
        LoginRsp_DEV_T = 10030,
        LogoutReq_DEV_T = 10040,                      //设备登出接入管理服务器
        LogoutRsp_DEV_T = 10050,
        ShakehandReq_DEV_T = 10060,               //设备与接入管理服务器握手
        ShakehandRsp_DEV_T = 10070,
        ConfigInfoReq_DEV_T = 10080,               //设备请求配置信息
        ConfigInfoRsp_DEV_T = 10090,
        AuthReq_DEV_T = 10100,                        //平台请求设备鉴权
        AuthRsp_DEV_T = 10110,

        ////////////////////////////////////////////////////////

        GetAccessAddressReq_USR_T = 20000,    //用户获取用户设备管理服务器地址
        GetAccessAddressRsp_USR_T = 20010,
        RegisterUserReq_USR_T = 20020,            //用户注册
        RegisterUserRsp_USR_T = 20030,
        UnRegisterUserReq_USR_T = 20040,       //用户注销
        UnRegisterUserRsp_USR_T = 20050,
        LoginReq_USR_T = 20060,                      //用户登录用户设备管理服务器
        LoginRsp_USR_T = 20070,
        LogoutReq_USR_T = 20080,                   //用户登出用户设备管理服务器
        LogoutRsp_USR_T = 20090,
        ShakehandReq_USR_T = 20100,             //用户与用户设备管理服务器握手
        ShakehandRsp_USR_T = 20110,
        ConfigInfoReq_USR_T = 20120,             //用户请求配置信息
        ConfigInfoRsp_USR_T = 20130,

        AddDevReq_USR_T = 20140,                  //用户添加设备
        AddDevRsp_USR_T = 20150,
        DelDevReq_USR_T = 20160,                   //用户删除设备
        DelDevRsp_USR_T = 20170,
        ModifyDevReq_USR_T = 20180,             //用户修改设备
        ModifyDevRsp_USR_T = 20190,
        QueryDevReq_USR_T = 20200,               //用户查询设备
        QueryDevRsp_USR_T = 20210,
        SharingDevReq_USR_T = 20220,             //用户共享设备
        SharingDevRsp_USR_T = 20230,
        CancelSharedDevReq_USR_T = 20240,    //用户取消共享设备
        CancelSharedDevRsp_USR_T = 20250,

        AddFriendsReq_USR_T = 20260,             //用户添加好友
        AddFriendsRsp_USR_T = 20270,
        DelFriendsReq_USR_T = 20280,              //用户删除好友
        DelFriendsRsp_USR_T = 20290,
        ModifyFriendsReq_USR_T = 20300,        //用户修改好友，目前暂不实现
        ModifyFriendsRsp_USR_T = 20310,
        QueryFriendsReq_USR_T = 20320,          //用户查询好友
        QueryFriendsRsp_USR_T = 20330,

        ///////////////////////////////////////////////////////

        GetOnlineDevInfoReq_INNER_T = 30000,          //获取在线设备信息
        GetOnlineDevInfoRsp_INNER_T = 30010,
        BroadcastOnlineDevInfo__INNER_T = 30020,     //广播在线设备信息
        GetOnlineUserInfoReq_INNER_T = 30030,         //获取在线用户信息
        GetOnlineUserInfoRsp_INNER_T = 30040,
        BroadcastOnlineUserInfo__INNER_T = 30050    //广播在线用户信息
    };

    struct Device
    {
        std::string m_strDevID;
        std::string m_strDevName;
        std::string m_strDevPassword;
        unsigned int m_uiTypeInfo;
        std::string m_strCreatedate;
        std::string m_strInnerinfo;                          //设备上传到平台的信息  
        std::string m_strOwnerUserID;                    //设备的所有者
        std::list<std::string> m_sharingUserIDList;   //主动分享该设备的用户ID（这里之所以是列表类型，是考虑到二次分享，从而导致该主动分享的用户ID可能是多个）
        std::list<std::string> m_sharedUserIDList;    //设备被分享给其他用户的用户ID
        std::list<std::string> m_strItemsList;            //设备其他属性
    };

    struct User
    {
        std::string m_strUserID;
        std::string m_strUserName;
        std::string m_strUserPassword;
        unsigned int m_uiTypeInfo;
        std::string m_strCreatedate;
        std::list<Device> m_ownerDevInfoList;             //用户所拥有的设备
        std::list<Device> m_sharingDevInfoList;           //用户主动分享出去的设备
        std::list<Device> m_sharedDevInfoList;            //用户被分享到的设备
        std::list<std::string> m_strItemsList;                 //用户其他属性
    };

    
    struct Req
    {
        Req(){};
        virtual ~Req(){};
 
        MsgType m_MsgType;
        unsigned long long m_uiMsgSeq;
        std::string m_strSID;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);
        
        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct Rsp : Req
    {
        Rsp(){};
        virtual ~Rsp(){};

        int m_iRetcode;
        std::string m_strRetMsg;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct RegisterUserReq_USR : Req
    {

        User m_userInfo;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };
    
    struct RegisterUserRsp_USR : Rsp
    {

        std::string m_strUserID;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct UnRegisterUserReq_USR : Req
    {

        User m_userInfo;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct UnRegisterUserRsp_USR :Rsp
    {

        std::string m_strUserID;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct LoginReq_USR : Req
    {

        User m_userInfo;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct LoginRsp_USR : Rsp
    {

        std::list<Device> m_devInfoList; //用户登录之后返回用户所关联的设备信息，包括了拥有、分享中、被分享的所有设备，这些类型在Device中有字段表示
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct LogoutReq_USR : Req
    {

        User m_userInfo;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct LogoutRsp_USR : Rsp
    {
        
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct ShakehandReq_USR : Req
    {

        std::string m_strUserID;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct ShakehandRsp_USR : Rsp
    {

        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };


    struct ConfigInfoReq_USR : Req
    {

        std::string m_strUserID;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct ConfigInfoRsp_USR : Rsp
    {

        std::string m_strValue;
        std::list<std::string> m_strItemsList;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct AddDevReq_USR : Req
    {

        std::string m_strUserID;
        Device m_devInfo;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct AddDevRsp_USR : Rsp
    {

        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct DelDevReq_USR : Req
    {

        std::string m_strUserID;
        std::list<std::string> m_strDevIDList;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct DelDevRsp_USR : Rsp
    {

        std::string m_strValue;
        std::list<std::string> m_strDevIDFailedList;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct ModifyDevReq_USR : Req
    {

        std::string m_strUserID;
        Device m_devInfo;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct ModifyDevRsp_USR : Rsp
    {

        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryDevReq_USR : Req
    {

        std::string m_strUserID;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryDevRsp_USR : Rsp
    {

        std::list<Device> m_allDevInfoList;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };
    
    struct SharingDevReq_USR : Req
    {

        std::string m_strUserID;                //主动分享用户
        std::string m_strToUserID;             //被分享用户
        Device m_devInfo;
        unsigned int m_uiRelation;           //关系包括，拥有0、被分享1、分享中2、转移3，目前只有0，1，2这三种关系
        std::string m_strBeginDate;
        std::string m_strEndDate;
        std::string m_strCreateDate;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct SharingDevRsp_USR : Rsp
    {

        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct CancelSharedDevReq_USR : Req
    {

        std::string m_strUserID;
        std::string m_strDevID;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct CancelSharedDevRsp_USR : Rsp
    {

        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct AddFriendsReq_USR : Req
    {

        std::string m_strUserID;
        std::string m_strFriendUserID;
        
        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct AddFriendsRsp_USR : Rsp
    {

        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct DelFriendsReq_USR : Req
    {

        std::string m_strUserID;
        std::list<std::string> m_strFriendUserIDList;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct DelFriendsRsp_USR : Rsp
    {

        std::string m_strValue;
        std::list<std::string> m_strFriendUserIDFailedList;          //考虑到批量删除好友，部分失败的情况。

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryFriendsReq_USR : Req
    {

        std::string m_strUserID;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryFriendsRsp_USR : Rsp
    {

        std::list<User> m_allFriendUserInfoList;
        std::string m_strValue;
        
        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct GetOnlineDevInfoReq_INNER : Req
    {

        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct GetOnlineDevInfoRsp_INNER : Rsp
    {

        std::list<Device> m_devInfoList;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct BroadcastOnlineDevInfo__INNER : Req
    {

        std::list<Device> m_devInfoList;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct GetOnlineUserInfoReq_INNER : Req
    {

        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct GetOnlineUserInfoRsp_INNER : Rsp
    {

        std::list<User> m_devInfoList;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct BroadcastOnlineUserInfo_INNER : Req
    {

        std::list<User> m_devInfoList;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    
    bool GetMsgType(const std::string &strData, MsgType &msgtype);

    bool SerializeReq(const Req &req, std::string &strOutput);
    bool UnSerializeReq(const std::string &strData, Req &req);
    
    bool SerializeRsp(const Rsp &rsp, std::string &strOutput);
    bool UnSerializeRsp(const std::string &strData, Rsp &rsp);

private:

    bool RegisterUserReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool RegisterUserReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool RegisterUserRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool RegisterUserRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);
    
    bool UnRegisterUserReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool UnRegisterUserReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool UnRegisterUserRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool UnRegisterUserRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool LoginReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool LoginReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool LoginRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool LoginRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool LogoutReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool LogoutReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool LogoutRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool LogoutRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool ShakehandReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool ShakehandReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool ShakehandRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool ShakehandRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool ConfigInfoReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool ConfigInfoReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool ConfigInfoRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool ConfigInfoRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool AddDevReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool AddDevReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool AddDevRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool AddDevRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool DelDevReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool DelDevReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool DelDevRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool DelDevRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool ModifyDevReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool ModifyDevReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool ModifyDevRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool ModifyDevRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);
    
    bool QueryDevReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool QueryDevReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryDevRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryDevRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool SharingDevReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool SharingDevReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool SharingDevRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool SharingDevRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool CancelSharedDevReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool CancelSharedDevReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool CancelSharedDevRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool CancelSharedDevRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool AddFriendsReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool AddFriendsReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool AddFriendsRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool AddFriendsRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool DelFriendsReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool DelFriendsReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool DelFriendsRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool DelFriendsRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool QueryFriendsReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool QueryFriendsReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryFriendsRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryFriendsRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool GetOnlineDevInfoReq_INNER_Serializer(const Req &req, std::string &strOutput);
    bool GetOnlineDevInfoReq_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool GetOnlineDevInfoRsp_INNER_Serializer(const Req &rsp, std::string &strOutput);
    bool GetOnlineDevInfoRsp_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool BroadcastOnlineDevInfo_INNER_Serializer(const Req &req, std::string &strOutput);
    bool BroadcastOnlineDevInfo_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);

    bool GetOnlineUserInfoReq_INNER_Serializer(const Req &req, std::string &strOutput);
    bool GetOnlineUserInfoReq_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool GetOnlineUserInfoRsp_INNER_Serializer(const Req &rsp, std::string &strOutput);
    bool GetOnlineUserInfoRsp_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool BroadcastOnlineUserInfo_INNER_Serializer(const Req &req, std::string &strOutput);
    bool BroadcastOnlineUserInfo_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);



private:    
    typedef boost::function<bool(const Req &req, std::string &strOutput)> Serializer;
    typedef boost::function<bool(const InteractiveMessage &InteractiveMsg, Req &req)> UnSerializer;

    struct SerializeHandler
    {
        Serializer Szr;
        UnSerializer UnSzr;
    };

    std::map<int, SerializeHandler> m_ReqAndRspHandlerMap;

};



#endif
