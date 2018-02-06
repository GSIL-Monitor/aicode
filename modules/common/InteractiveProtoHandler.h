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
    virtual ~InteractiveProtoHandler();

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

        QueryTimeZoneReq_DEV_T = 10320,    //查询时区
        QueryTimeZoneRsp_DEV_T = 10330,

        QueryAccessDomainNameReq_DEV_T = 10340,  //设备查询接入服务器域名
        QueryAccessDomainNameRsp_DEV_T = 10350,

        QueryUpgradeSiteReq_DEV_T = 10360,    //设备查询升级站点
        QueryUpgradeSiteRsp_DEV_T = 10370,

        QueryFirmwareUpgradeReq_DEV_T = 10380,   //设备查询固件升级数据
        QueryFirmwareUpgradeRsp_DEV_T = 10390,

        ModifyDevicePropertyReq_DEV_T = 10400,   //修改设备属性
        ModifyDevicePropertyRsp_DEV_T = 10410,
        QueryDeviceParameterReq_DEV_T = 10420,
        QueryDeviceParameterRsp_DEV_T = 10430,

        QueryPlatformPushStatusReq_DEV_T = 10500,  //查询平台是否支持消息推送
        QueryPlatformPushStatusRsp_DEV_T = 10510,

        DeviceEventReportReq_DEV_T = 10600,        //设备事件上报
        DeviceEventReportRsp_DEV_T = 10610,

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
        RetrievePwdReq_USR_T = 20057,           //用户取回密码
        RetrievePwdRsp_USR_T = 20058,
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

        P2pInfoReq_USR_T = 20360,               //用户P2P服务器信息
        P2pInfoRsp_USR_T = 20370,

        QueryAccessDomainNameReq_USR_T = 20600,    //用户查询接入服务器域名
        QueryAccessDomainNameRsp_USR_T = 20610,

        QueryAppUpgradeReq_USR_T = 20620,          //用户查询APP升级版本数据
        QueryAppUpgradeRsp_USR_T = 20630,

        QueryIfP2pIDValidReq_USR_T = 20640,        //用户查询P2PID是否有效
        QueryIfP2pIDValidRsp_USR_T = 20650,

        QueryAllDeviceEventReq_USR_T = 20700,      //查看设备事件请求
        QueryAllDeviceEventRsp_USR_T = 20710,
        DeleteDeviceEventReq_USR_T = 20720,        //删除设备事件请求
        DeleteDeviceEventRsp_USR_T = 20730,
        ModifyDeviceEventReq_USR_T = 20740,        //修改设备事件请求
        ModifyDeviceEventRsp_USR_T = 20750,

        AddStorageDetailReq_USR_T = 20800,         //用户新增存储详情
        AddStorageDetailRsp_USR_T = 20810,
        DeleteStorageDetailReq_USR_T = 20820,      //用户删除存储详情
        DeleteStorageDetailRsp_USR_T = 20830,
        ModifyStorageDetailReq_USR_T = 20840,      //用户修改存储详情
        ModifyStorageDetailRsp_USR_T = 20850,
        QueryStorageDetailReq_USR_T = 20860,       //用户查询存储详情
        QueryStorageDetailRsp_USR_T = 20870,
        QueryRegionStorageInfoReq_USR_T = 20880,
        QueryRegionStorageInfoRsp_USR_T = 20890,

        QueryDeviceInfoMultiReq_USR_T = 20950,        //批量查询设备信息
        QueryDeviceInfoMultiRsp_USR_T = 20960,

        ///////////////////////////////////////////////////////

        GetOnlineDevInfoReq_INNER_T = 30000,          //获取在线设备信息
        GetOnlineDevInfoRsp_INNER_T = 30010,
        BroadcastOnlineDevInfo__INNER_T = 30020,     //广播在线设备信息
        GetOnlineUserInfoReq_INNER_T = 30030,         //获取在线用户信息
        GetOnlineUserInfoRsp_INNER_T = 30040,
        BroadcastOnlineUserInfo__INNER_T = 30050,    //广播在线用户信息
 
        GetDeviceAccessRecordReq_INNER_T = 30200,    //获取设备接入记录
        GetDeviceAccessRecordRsp_INNER_T = 30210,
        GetUserAccessRecordReq_INNER_T = 30220,      //获取用户接入记录
        GetUserAccessRecordRsp_INNER_T = 30230,

        ///////////////////////////////////////////////////////

        QueryUploadURLReq_MGR_T = 40000,
        QueryUploadURLRsp_MGR_T = 40010,

        AddConfigurationReq_MGR_T = 40020,           //添加配置项
        AddConfigurationRsp_MGR_T = 40030,
        DeleteConfigurationReq_MGR_T = 40040,        //删除配置项
        DeleteConfigurationRsp_MGR_T = 40050,
        ModifyConfigurationReq_MGR_T = 40060,        //修改配置项
        ModifyConfigurationRsp_MGR_T = 40070,
        QueryAllConfigurationReq_MGR_T = 40080,      //查询所有配置项
        QueryAllConfigurationRsp_MGR_T = 40090
   };

    struct Device
    {
        std::string m_strDevID;
        std::string m_strDevName;
        std::string m_strDevPassword;
        unsigned int m_uiTypeInfo;
        std::string m_strP2pID;
        std::string m_strDomainName;
        std::string m_strCreatedate;
        unsigned int m_uiStatus;
        std::string m_strExtend;
        std::string m_strInnerinfo;                          //设备上传到平台的信息  
    };

    struct DeviceStatus
    {
        Device m_deviceInfo;
        std::string m_strOnlineStatus;
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
        std::string m_strAliasName;                     //别名
        std::string m_strEmail;

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
        unsigned int m_uiBusinessType;           //文件业务类型
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

    struct DeviceAccessRecord
    {
        std::string m_strAccessID;
        std::string m_strClusterID;
        std::string m_strDeviceID;
        std::string m_strDeviceName;
        unsigned int m_uiDeviceType;
        std::string m_strLoginTime;
        std::string m_strLogoutTime;
        unsigned int m_uiOnlineDuration;
        std::string m_strCreateDate;
        unsigned int m_uiStatus;
    };
    
    struct UserAccessRecord
    {
        std::string m_strAccessID;
        std::string m_strClusterID;
        std::string m_strUserID;
        std::string m_strUserName;
        std::string m_strUserAliasname;
        unsigned int m_uiClientType;
        std::string m_strLoginTime;
        std::string m_strLogoutTime;
        unsigned int m_uiOnlineDuration;
        std::string m_strCreateDate;
        unsigned int m_uiStatus;
    };

    struct Configuration
    {
        std::string m_strCategory;
        std::string m_strSubCategory;
        std::string m_strLatestVersion;
        std::string m_strDescription;
        std::string m_strForceVersion;
        std::string m_strServerAddress;
        std::string m_strFileName;
        std::string m_strFileID;
        unsigned int m_uiFileSize;
        std::string m_strFilePath;
        unsigned int m_uiLeaseDuration;
        std::string m_strUpdateDate;
        unsigned int m_uiStatus;
        std::string m_strExtend;
    };

    struct AppUpgrade
    {
        unsigned int m_uiNewVersionValid;
        std::string m_strAppName;
        std::string m_strAppPath;
        unsigned int m_uiAppSize;
        std::string m_strVersion;
        std::string m_strDescription;
        unsigned int m_uiForceUpgrade;
        std::string m_strUpdateDate;
    };

    struct FirmwareUpgrade
    {
        unsigned int m_uiNewVersionValid;
        std::string m_strFirmwareName;
        std::string m_strFirmwarePath;
        unsigned int m_uiFirmwareSize;
        std::string m_strVersion;
        std::string m_strDescription;
        unsigned int m_uiForceUpgrade;
        std::string m_strUpdateDate;
    };

    struct DoorbellParameter
    {
        std::string m_strDoorbellName;
        std::string m_strSerialNumber;
        std::string m_strDoorbellP2Pid;
        std::string m_strBatteryCapacity;
        std::string m_strChargingState;
        std::string m_strWifiSignal;
        std::string m_strVolumeLevel;
        std::string m_strVersionNumber;
        std::string m_strChannelNumber;
        std::string m_strCodingType;
        std::string m_strPIRAlarmSwtich;
        std::string m_strDoorbellSwitch;
        std::string m_strPIRAlarmLevel;
        std::string m_strPIRIneffectiveTime;
        std::string m_strCurrentWifi;
        std::string m_strSubCategory;
        std::string m_strDisturbMode;
        std::string m_strAntiTheftSwitch;
        std::string m_strExtend;
    };

    struct DeviceEvent
    {
        std::string m_strDeviceID;
        unsigned int m_uiDeviceType;
        std::string m_strEventID;
        unsigned int m_uiEventType;
        unsigned int m_uiEventState;
        std::string m_strFileUrl;
        std::string m_strEventTime;
        unsigned int m_uiReadState;
        std::string m_strThumbnailUrl;
    };

    struct StorageDetail
    {
        unsigned int m_uiDomainID;
        std::string m_strObjectID;
        unsigned int m_uiObjectType;
        std::string m_strStorageName;
        unsigned int m_uiStorageType;
        unsigned int m_uiOverlapType;
        unsigned int m_uiStorageTimeUpLimit;
        unsigned int m_uiStorageTimeDownLimit;
        unsigned int m_uiSizeOfSpaceUsed;
        unsigned int m_uiStorageUnitType;
        std::string m_strBeginDate;
        std::string m_strEndDate;
        unsigned int m_uiStatus;
        std::string m_strExtend;
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
        std::string m_strOldPwd;
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

    struct RetrievePwdReq_USR : Req
    {
        std::string m_strUserName;
        std::string m_strEmail;
        unsigned int m_uiAppType;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct RetrievePwdRsp_USR : Rsp
    {

        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };



    struct LoginReq_USR : Req
    {
        User m_userInfo;
        std::string m_strValue;
        unsigned int m_uiTerminalType;
        unsigned int m_uiType;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct LoginRsp_USR : Rsp
    {
        std::string m_strUserID; //系统返回用户ID，这个ID后续用来代表用户的身份来进行后续的操作
        std::list<Relation> m_reInfoList; //用户登录之后返回用户所关联的设备信息，包括了拥有、分享中、被分享的所有设备，这些类型在Device中有字段表示
        std::list<std::string> m_strDevNameList;
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
        std::string m_strDeviceID;
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
        unsigned int m_uiDeviceShared;

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
        std::string m_strUserID;
        std::string m_strDevID;
        unsigned int m_uiDeviceShared;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryDevInfoRsp_USR : Rsp
    {
        Device m_devInfo;
        std::string m_strVersion;
        std::string m_strOnlineStatus;
        std::string m_strUpdateDate;
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
        std::list<std::string> m_strDevNameList;

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
        std::list<std::string> m_strUserNameList;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };
        
    struct SharingDevReq_USR : Req
    {

        Relation m_relationInfo;
        std::string m_strUserName;
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
        std::string m_strBeginDate;
        std::string m_strEndDate;
        unsigned int m_uiBusinessType;
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

    struct QueryAppUpgradeReq_USR : Req
    {
        std::string m_strCategory;
        std::string m_strSubCategory;
        std::string m_strCurrentVersion;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryAppUpgradeRsp_USR : Rsp
    {
        AppUpgrade m_appUpgrade;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryIfP2pIDValidReq_USR : Req
    {
        std::string m_strP2pID;
        unsigned int m_uiSuppliser;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryIfP2pIDValidRsp_USR : Rsp
    {
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryAllDeviceEventReq_USR : Req
    {
        std::string m_strUserID;
        std::string m_strDeviceID;
        unsigned int m_uiDeviceShared;
        unsigned int m_uiEventType;
        unsigned int m_uiReadState;
        unsigned int m_uiBeginIndex;
        std::string m_strBeginDate;
        std::string m_strEndDate;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryAllDeviceEventRsp_USR : Rsp
    {
        std::list<DeviceEvent> m_deviceEventList;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct DeleteDeviceEventReq_USR : Req
    {
        std::string m_strUserID;
        std::string m_strDeviceID;
        std::string m_strEventID;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct DeleteDeviceEventRsp_USR : Rsp
    {
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct ModifyDeviceEventReq_USR : Req
    {
        std::string m_strUserID;
        std::string m_strDeviceID;
        std::string m_strEventID;
        unsigned int m_uiEventType;
        unsigned int m_uiEventState;
        std::string m_strUpdateTime;
        std::string m_strFileID;
        unsigned int m_uiReadState;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct ModifyDeviceEventRsp_USR : Rsp
    {
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct AddStorageDetailReq_USR : Req
    {
        StorageDetail m_storageDetail;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct AddStorageDetailRsp_USR : Rsp
    {
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct DeleteStorageDetailReq_USR : Req
    {
        std::string m_strObjectID;
        unsigned int m_uiDomainID;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct DeleteStorageDetailRsp_USR : Rsp
    {
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct ModifyStorageDetailReq_USR : Req
    {
        StorageDetail m_storageDetail;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct ModifyStorageDetailRsp_USR : Rsp
    {
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryStorageDetailReq_USR : Req
    {
        std::string m_strObjectID;
        unsigned int m_uiDomainID;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryStorageDetailRsp_USR : Rsp
    {
        StorageDetail m_storageDetail;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryRegionStorageInfoReq_USR : Req
    {
        std::string m_strUserID;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryRegionStorageInfoRsp_USR : Rsp
    {
        unsigned int m_uiDomainID;
        unsigned int m_uiSizeOfSpace;
        unsigned int m_uiSizeOfSpaceUsed;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryDeviceInfoMultiReq_USR : Req
    {
        std::list<std::string> m_strDeviceIDList;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryDeviceInfoMultiRsp_USR : Rsp
    {
        std::list<DeviceStatus> m_deviceStatusList;

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
        unsigned int m_uiP2pSupplier;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct P2pInfoRsp_USR : Rsp
    {
        std::string m_strP2pServer;
        std::string m_strP2pID;
        unsigned int m_uiLease;  //租约，单位为小时
        std::string m_strLicenseKey;
        std::string m_strPushID;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryAccessDomainNameReq_DEV : Req
    {
        std::string m_strDevIpAddress;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryAccessDomainNameRsp_DEV : Rsp
    {
        std::string m_strDomainName;
        unsigned int m_uiLease;  //租约，单位为小时
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
        unsigned int m_uiDeviceType;
        unsigned int m_uiP2pSupplier;
        std::string m_strP2pID;
        std::string m_strP2pServr;
        unsigned int m_uiP2pBuildin;
        std::string m_strLicenseKey;
        std::string m_strPushID;
        std::string m_strUserName;
        std::string m_strUserPassword;
        std::string m_strDistributor;
        std::string m_strDomainName;
        std::string m_strOtherProperty;
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
        unsigned int m_uiP2pSupplier;
        std::string m_strDomainName;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct P2pInfoRsp_DEV : Rsp
    {

        std::string m_strP2pServer;
        std::string m_strP2pID;
        unsigned int m_uiLease;  //租约，单位为小时
        std::string m_strLicenseKey;
        std::string m_strPushID;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryAccessDomainNameReq_USR : Req
    {
        std::string m_strUserIpAddress;
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryAccessDomainNameRsp_USR : Rsp
    {
        std::string m_strDomainName;
        unsigned int m_uiLease;  //租约，单位为小时
        std::string m_strValue;

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

    struct QueryTimeZoneReq_DEV : Req
    {

        std::string m_strDevID;
        std::string m_strDevIpAddress;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryTimeZoneRsp_DEV : Rsp
    {

        std::string m_strCountryCode;
        std::string m_strCountryNameEn;
        std::string m_strCountryNameZh;
        std::string m_strTimeZone;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryUpgradeSiteReq_DEV : Req
    {

        std::string m_strDevID;
        std::string m_strDevIpAddress;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryUpgradeSiteRsp_DEV : Rsp
    {
        std::string m_strUpgradeSiteUrl;
        unsigned int m_uiLease;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryFirmwareUpgradeReq_DEV : Req
    {
        std::string m_strCategory;
        std::string m_strSubCategory;
        std::string m_strCurrentVersion;
        std::string m_strDeviceID;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryFirmwareUpgradeRsp_DEV : Rsp
    {
        FirmwareUpgrade m_firmwareUpgrade;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct ModifyDevicePropertyReq_DEV : Req
    {
        std::string m_strDeviceID;
        std::string m_strDomainName;
        std::string m_strP2pID;
        std::string m_strCorpID;
        std::string m_strDeviceName;
        std::string m_strDeviceIP;
        std::string m_strDeviceIP2;
        std::string m_strWebPort;
        std::string m_strCtrlPort;
        std::string m_strProtocol;
        std::string m_strModel;
        std::string m_strPostFrequency;
        std::string m_strVersion;
        std::string m_strDeviceStatus;
        std::string m_strServerIP;
        std::string m_strServerPort;
        std::string m_strTransfer;
        std::string m_strMobilePort;
        std::string m_strChannelCount;
        unsigned int m_uiDeviceType;
        std::string m_strRequestSource;
        DoorbellParameter m_doorbellParameter;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct ModifyDevicePropertyRsp_DEV : Rsp
    {
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryDeviceParameterReq_DEV : Req
    {
        std::string m_strDeviceID;
        unsigned int m_uiDeviceType;
        std::string m_strQueryType;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryDeviceParameterRsp_DEV : Rsp
    {
        DoorbellParameter m_doorbellParameter;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryPlatformPushStatusReq_DEV : Req
    {
        std::string m_strDeviceID;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryPlatformPushStatusRsp_DEV : Rsp
    {
        std::string m_strStatus;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct DeviceEventReportReq_DEV : Req
    {
        std::string m_strDeviceID;
        unsigned int m_uiDeviceType;
        unsigned int m_uiEventType;
        unsigned int m_uiEventState;
        std::string m_strFileID;
        std::string m_strEventTime;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct DeviceEventReportRsp_DEV : Rsp
    {
        std::string m_strEventID;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };


    struct GetDeviceAccessRecordReq_INNER : Req
    {
        unsigned int m_uiBeginIndex;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct GetDeviceAccessRecordRsp_INNER : Rsp
    {
        std::list<DeviceAccessRecord> m_deviceAccessRecordList;
        unsigned int m_uiRecordTotal;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct GetUserAccessRecordReq_INNER : Req
    {
        unsigned int m_uiBeginIndex;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct GetUserAccessRecordRsp_INNER : Rsp
    {
        std::list<UserAccessRecord> m_userAccessRecordList;
        unsigned int m_uiRecordTotal;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryUploadURLReq_MGR : Req
    {
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryUploadURLRsp_MGR : Rsp
    {
        std::string m_strUploadURL;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct AddConfigurationReq_MGR : Req
    {
        Configuration m_configuration;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct AddConfigurationRsp_MGR : Rsp
    {
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct DeleteConfigurationReq_MGR : Req
    {
        std::string m_strCategory;
        std::string m_strSubCategory;
 
        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct DeleteConfigurationRsp_MGR : Rsp
    {
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct ModifyConfigurationReq_MGR : Req
    {
        Configuration m_configuration;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct ModifyConfigurationRsp_MGR : Rsp
    {
        std::string m_strValue;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryAllConfigurationReq_MGR : Req
    {
        unsigned int m_uiBeginIndex;

        virtual void UnSerializer(const InteractiveMessage &InteractiveMsg);

        virtual void Serializer(InteractiveMessage &InteractiveMsg) const;
    };

    struct QueryAllConfigurationRsp_MGR : Rsp
    {
        std::list<Configuration> m_configurationList;

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

    bool QueryAccessDomainNameReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool QueryAccessDomainNameReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryAccessDomainNameRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryAccessDomainNameRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);


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

    bool QueryAccessDomainNameReq_DEV_Serializer(const Req &req, std::string &strOutput);
    bool QueryAccessDomainNameReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryAccessDomainNameRsp_DEV_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryAccessDomainNameRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);


    bool ShakehandReq_DEV_Serializer(const Req &req, std::string &strOutput);
    bool ShakehandReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool ShakehandRsp_DEV_Serializer(const Req &rsp, std::string &strOutput);
    bool ShakehandRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool LogoutReq_DEV_Serializer(const Req &req, std::string &strOutput);
    bool LogoutReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool LogoutRsp_DEV_Serializer(const Req &rsp, std::string &strOutput);
    bool LogoutRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool RetrievePwdReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool RetrievePwdReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool RetrievePwdRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool RetrievePwdRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool QueryTimeZoneReq_DEV_Serializer(const Req &req, std::string &strOutput);
    bool QueryTimeZoneReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryTimeZoneRsp_DEV_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryTimeZoneRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool QueryUpgradeSiteReq_DEV_Serializer(const Req &req, std::string &strOutput);
    bool QueryUpgradeSiteReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryUpgradeSiteRsp_DEV_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryUpgradeSiteRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool GetDeviceAccessRecordReq_INNER_Serializer(const Req &req, std::string &strOutput);
    bool GetDeviceAccessRecordReq_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool GetDeviceAccessRecordRsp_INNER_Serializer(const Req &rsp, std::string &strOutput);
    bool GetDeviceAccessRecordRsp_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool GetUserAccessRecordReq_INNER_Serializer(const Req &req, std::string &strOutput);
    bool GetUserAccessRecordReq_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool GetUserAccessRecordRsp_INNER_Serializer(const Req &rsp, std::string &strOutput);
    bool GetUserAccessRecordRsp_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool QueryAppUpgradeReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool QueryAppUpgradeReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryAppUpgradeRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryAppUpgradeRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool QueryFirmwareUpgradeReq_DEV_Serializer(const Req &req, std::string &strOutput);
    bool QueryFirmwareUpgradeReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryFirmwareUpgradeRsp_DEV_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryFirmwareUpgradeRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool QueryUploadURLReq_MGR_Serializer(const Req &req, std::string &strOutput);
    bool QueryUploadURLReq_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryUploadURLRsp_MGR_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryUploadURLRsp_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool AddConfigurationReq_MGR_Serializer(const Req &req, std::string &strOutput);
    bool AddConfigurationReq_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool AddConfigurationRsp_MGR_Serializer(const Req &rsp, std::string &strOutput);
    bool AddConfigurationRsp_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool DeleteConfigurationReq_MGR_Serializer(const Req &req, std::string &strOutput);
    bool DeleteConfigurationReq_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool DeleteConfigurationRsp_MGR_Serializer(const Req &rsp, std::string &strOutput);
    bool DeleteConfigurationRsp_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool ModifyConfigurationReq_MGR_Serializer(const Req &req, std::string &strOutput);
    bool ModifyConfigurationReq_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool ModifyConfigurationRsp_MGR_Serializer(const Req &rsp, std::string &strOutput);
    bool ModifyConfigurationRsp_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool QueryAllConfigurationReq_MGR_Serializer(const Req &req, std::string &strOutput);
    bool QueryAllConfigurationReq_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryAllConfigurationRsp_MGR_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryAllConfigurationRsp_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool ModifyDevicePropertyReq_DEV_Serializer(const Req &req, std::string &strOutput);
    bool ModifyDevicePropertyReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool ModifyDevicePropertyRsp_DEV_Serializer(const Req &rsp, std::string &strOutput);
    bool ModifyDevicePropertyRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool QueryDeviceParameterReq_DEV_Serializer(const Req &req, std::string &strOutput);
    bool QueryDeviceParameterReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryDeviceParameterRsp_DEV_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryDeviceParameterRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool QueryIfP2pIDValidReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool QueryIfP2pIDValidReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryIfP2pIDValidRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryIfP2pIDValidRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool QueryPlatformPushStatusReq_DEV_Serializer(const Req &req, std::string &strOutput);
    bool QueryPlatformPushStatusReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryPlatformPushStatusRsp_DEV_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryPlatformPushStatusRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool DeviceEventReportReq_DEV_Serializer(const Req &req, std::string &strOutput);
    bool DeviceEventReportReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool DeviceEventReportRsp_DEV_Serializer(const Req &rsp, std::string &strOutput);
    bool DeviceEventReportRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool QueryAllDeviceEventReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool QueryAllDeviceEventReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryAllDeviceEventRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryAllDeviceEventRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool DeleteDeviceEventReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool DeleteDeviceEventReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool DeleteDeviceEventRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool DeleteDeviceEventRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool ModifyDeviceEventReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool ModifyDeviceEventReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool ModifyDeviceEventRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool ModifyDeviceEventRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool AddStorageDetailReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool AddStorageDetailReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool AddStorageDetailRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool AddStorageDetailRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool DeleteStorageDetailReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool DeleteStorageDetailReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool DeleteStorageDetailRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool DeleteStorageDetailRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool ModifyStorageDetailReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool ModifyStorageDetailReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool ModifyStorageDetailRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool ModifyStorageDetailRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool QueryStorageDetailReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool QueryStorageDetailReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryStorageDetailRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryStorageDetailRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool QueryRegionStorageInfoReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool QueryRegionStorageInfoReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryRegionStorageInfoRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryRegionStorageInfoRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);

    bool QueryDeviceInfoMultiReq_USR_Serializer(const Req &req, std::string &strOutput);
    bool QueryDeviceInfoMultiReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req);
    bool QueryDeviceInfoMultiRsp_USR_Serializer(const Req &rsp, std::string &strOutput);
    bool QueryDeviceInfoMultiRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp);


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
