#ifndef _INTERACTIVE_PROTOE_HANDLER_
#define _INTERACTIVE_PROTOE_HANDLER_

#include <string>
#include <list>
#include <map>
#include <boost/bind.hpp>
#include <boost/function.hpp>

/************************************************************************/
/* protobuffer协议序列化和反序列化相关接口与实现
 * Author：尹宾 
 * Date：2016-11-30*/
/************************************************************************/

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
        
        P2pInfoReq_DEV_T = 10220,          //P2P服务器信息
        P2pInfoRsp_DEV_T = 10230,

        AddFileReq_DEV_T = 10300,               //设备添加文件
        AddFileRsp_DEV_T = 10310,

        ////////////////////////////////////////////////////////

        MsgPreHandlerReq_USR_T = 19990,       //消息预处理
        MsgPreHandlerRsp_USR_T = 19991,
        GetAccessAddressReq_USR_T = 20000,    //用户获取用户设备管理服务器地址
        GetAccessAddressRsp_USR_T = 20010,
        RegisterUserReq_USR_T = 20020,            //用户注册
        RegisterUserRsp_USR_T = 20030,
        UnRegisterUserReq_USR_T = 20040,       //用户注销
        UnRegisterUserRsp_USR_T = 20050,
        QueryUsrInfoReq_USR_T = 20051,
        QueryUsrInfoRsp_USR_T = 20052,
        ModifyUserInfoReq_USR_T = 20055,      //用户修改
        ModifyUserInfoRsp_USR_T = 20056,
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
        QueryDevInfoReq_USR_T = 20191,
        QueryDevInfoRsp_USR_T = 20192,
        QueryDevReq_USR_T = 20200,               //用户查询设备
        QueryDevRsp_USR_T = 20210,
        QueryUserReq_USR_T = 20211,
        QueryUserRsp_USR_T = 20212,
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

        DeleteFileReq_USR_T = 20500,            //用户删除文件
        DeleteFileRsp_USR_T = 20510,        
        DownloadFileReq_USR_T = 20520,          //用户下载文件
        DownloadFileRsp_USR_T = 20530,      
        QueryFileReq_USR_T = 20540,             //用户查询文件
        QueryFileRsp_USR_T = 20550,

        P2pInfoReq_USR_T = 20800,               //用户P2P服务器信息
        P2pInfoRsp_USR_T = 20810,

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
        unsigned int m_uiStatus;
        std::string m_strExtend;
        std::string m_strInnerinfo;                          //设备上传到平台的信息  

    };

    struct User
    {
        std::string m_strUserID;
        std::string m_strUserName;
        std::string m_strUserPassword;
        unsigned int m_uiTypeInfo;
        std::string m_strCreatedate;
        unsigned int m_uiStatus;
        std::string m_strExtend;

    };

    struct Relation                                //用户与设备关系（设备与用户关系）
    {
        std::string m_strUserID;
        std::string m_strDevID;
        unsigned int m_uiRelation;                        //关系包括，拥有0、被分享1、分享中2、转移3，目前只有0，1，2这三种关系
        std::string m_strBeginDate;
        std::string m_strEndDate;
        std::string m_strValue;
    };

    struct File                                    //文件信息
    {
        std::string m_strFileID;
        std::string m_strUserID;
        std::string m_strDevID;
        std::string m_strRemoteFileID;           //服务器文件ID，与fileid一一对应
        std::string m_strDownloadUrl;            //文件URL地址
        std::string m_strFileName;
        std::string m_strSuffixName;             //文件后缀名称
        unsigned long int m_ulFileSize;          //文件大小，单位Byte
        std::string m_strFileCreatedate;
        std::string m_strCreatedate;
        unsigned int m_uiStatus;
        std::string m_strExtend;
    };

    struct FileUrl
    {
        std::string m_strFileID;
        std::string m_strDownloadUrl;
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

  
    struct MsgPreHandlerReq_USR : Req
    {

        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };


    struct MsgPreHandlerRsp_USR : Rsp
    {

        std::string m_strValue;

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

    struct UnRegisterUserRsp_USR : Rsp
    {

        std::string m_strUserID;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryUsrInfoReq_USR : Req
    {

        std::string m_strUserID;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryUsrInfoRsp_USR : Rsp
    {

        User m_userInfo;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct ModifyUserInfoReq_USR : Req
    {

        User m_userInfo;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct ModifyUserInfoRsp_USR : Rsp
    {

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
        std::string m_strUserID; //系统返回用户ID，这个ID后续用来代表用户的身份来进行后续的操作
        std::list<Relation> m_reInfoList; //用户登录之后返回用户所关联的设备信息，包括了拥有、分享中、被分享的所有设备，这些类型在Device中有字段表示
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

    struct QueryDevInfoReq_USR : Req
    {
        std::string m_strDevID;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryDevInfoRsp_USR : Rsp
    {
        Device m_devInfo;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryDevReq_USR : Req
    {

        std::string m_strUserID;
        unsigned int m_uiBeginIndex;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryDevRsp_USR : Rsp
    {
                        
        std::list<Relation> m_allRelationInfoList;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryUserReq_USR : Req
    {

        std::string m_strDevID;
        unsigned int m_uiBeginIndex;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryUserRsp_USR : Rsp
    {

        std::list<Relation> m_allRelationInfoList;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };
        
    struct SharingDevReq_USR : Req
    {

        Relation m_relationInfo;
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

        Relation m_relationInfo;
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
        unsigned int m_uiBeginIndex;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryFriendsRsp_USR : Rsp
    {

        std::list<std::string> m_allFriendUserIDList;
        
        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct DeleteFileReq_USR : Req
    {
        std::string m_strUserID;
        std::list<std::string> m_strFileIDList;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct DeleteFileRsp_USR : Rsp
    {
        std::string m_strValue;
        std::list<std::string> m_strFileIDFailedList;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct DownloadFileReq_USR : Req
    {
        std::string m_strUserID;
        std::list<std::string> m_strFileIDList;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct DownloadFileRsp_USR : Rsp
    {
        std::string m_strValue;
        std::list<FileUrl> m_fileUrlList;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryFileReq_USR : Req
    {
        std::string m_strUserID;
        std::string m_strDevID;
        unsigned int m_uiBeginIndex;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryFileRsp_USR : Rsp
    {
        std::string m_strValue;
        std::list<File> m_fileList;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };


    struct AddFileReq_DEV : Req
    {
        std::string m_strDevID;
        std::list<File> m_fileList;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct AddFileRsp_DEV : Rsp
    {
        std::string m_strValue;
        std::list<std::string> m_strFileIDFailedList;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct P2pInfoReq_USR : Req
    {
        std::string m_strUserID;
        std::string m_strUserIpAddress;
        std::string m_strDevID;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct P2pInfoRsp_USR : Rsp
    {
        std::string m_strP2pServer;
        std::string m_strP2pID;
        unsigned int m_uiLease;  //租约，单位为小时

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

    struct BroadcastOnlineDevInfo_INNER : Req
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

        std::list<User> m_userInfoList;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct BroadcastOnlineUserInfo_INNER : Req
    {

        std::list<User> m_userInfoList;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct LoginReq_DEV : Req
    {

        std::string m_strDevID;
        std::string m_strPassword;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct LoginRsp_DEV : Rsp
    {

        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct P2pInfoReq_DEV : Req
    {

        std::string m_strDevID;
        std::string m_strDevIpAddress;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct P2pInfoRsp_DEV : Rsp
    {

        std::string m_strP2pServer;
        std::string m_strP2pID;
        unsigned int m_uiLease;  //租约，单位为小时

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct ShakehandReq_DEV : Req
    {

        std::string m_strDevID;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct ShakehandRsp_DEV : Rsp
    {

        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct LogoutReq_DEV : Req
    {

        std::string m_strDevID;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct LogoutRsp_DEV : Rsp
    {

        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    
    bool GetMsgType(const std::string &strData, MsgType &msgtype);

    bool SerializeReq(const Req &req, std::string &strOutput);
    bool UnSerializeReq(const std::string &strData, Req &req);
    
    bool SerializeRsp(const Rsp &rsp, std::string &strOutput);
    bool UnSerializeRsp(const std::string &strData, Rsp &rsp);

    bool UnSerializeReqBase(const std::string &strData, Req &req);

private:

    bool MsgPreHandlerReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool MsgPreHandlerReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool MsgPreHandlerRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool MsgPreHandlerRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);
    
    bool RegisterUserReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool RegisterUserReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool RegisterUserRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool RegisterUserRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);
    
    bool UnRegisterUserReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool UnRegisterUserReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool UnRegisterUserRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool UnRegisterUserRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool QueryUsrInfoReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool QueryUsrInfoReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryUsrInfoRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryUsrInfoRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool ModifyUserInfoReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool ModifyUserInfoReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool ModifyUserInfoRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool ModifyUserInfoRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);


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
    
    bool QueryDevInfoReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool QueryDevInfoReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryDevInfoRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryDevInfoRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);
    
    bool QueryDevReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool QueryDevReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryDevRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryDevRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool QueryUserReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool QueryUserReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryUserRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryUserRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);


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

    bool DeleteFileReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool DeleteFileReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool DeleteFileRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool DeleteFileRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool DownloadFileReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool DownloadFileReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool DownloadFileRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool DownloadFileRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool QueryFileReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool QueryFileReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryFileRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryFileRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool AddFileReq_DEV_Serializer(const Req &req, std::string &strOutput);
    bool AddFileReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool AddFileRsp_DEV_Serializer(const Req &rsp, std::string &strOutput);
    bool AddFileRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool P2pInfoReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool P2pInfoReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool P2pInfoRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool P2pInfoRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);


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

    bool LoginReq_DEV_Serializer(const Req &req, std::string &strOutput);
    bool LoginReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool LoginRsp_DEV_Serializer(const Req &rsp, std::string &strOutput);
    bool LoginRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool P2pInfoReq_DEV_Serializer(const Req &req, std::string &strOutput);
    bool P2pInfoReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool P2pInfoRsp_DEV_Serializer(const Req &rsp, std::string &strOutput);
    bool P2pInfoRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool ShakehandReq_DEV_Serializer(const Req &req, std::string &strOutput);
    bool ShakehandReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool ShakehandRsp_DEV_Serializer(const Req &rsp, std::string &strOutput);
    bool ShakehandRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool LogoutReq_DEV_Serializer(const Req &req, std::string &strOutput);
    bool LogoutReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool LogoutRsp_DEV_Serializer(const Req &rsp, std::string &strOutput);
    bool LogoutRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);


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
