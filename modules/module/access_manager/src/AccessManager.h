#ifndef _USER_MANAGER_
#define _USER_MANAGER_

#include "boost/noncopyable.hpp"
#include "ControlCenter.h"
#include "InteractiveProtoHandler.h"
#include <unordered_map>
#include <string>
#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "NetComm.h"
#include "DBInfoCacheManager.h"
#include "boost/atomic.hpp"
#include "SessionMgr.h"
#include "ClusterAccessCollector.h"

class MysqlImpl;
class InterProcessHandler;

/************************************************************************/
/* 用户管理类，提供了管理用户所有相关的方法接口和实现。
 * 该类提供的方法会注册到到ControlCenter类中。
 * Author：尹宾
 * Date：2016-12-1*/
/************************************************************************/
class AccessManager : public boost::noncopyable
{
public:
    static const int NORMAL_STATUS = 0;    
    static const int DELETE_STATUS = 1;
    static const int FROZEN_STATUS = 2;

    static const int RELATION_OF_OWNER = 0;
    static const int RELATION_OF_BE_SHARED = 1;
    static const int RELATION_OF_SHARING = 2;

    static const int RELATION_OF_FRIENDS = 0;

    static const std::string MAX_DATE;
    
    static const int NEW_VERSION_INVALID = 0;
    static const int NEW_VERSION_VALID = 1;

    static const int INTERACTIVE_UPGRADE = 0;
    static const int FORCE_UPGRADE = 1;

    static const std::string ANDROID_APP;
    static const std::string IOS_APP;
    static const std::string IPC;

    static const int P2P_SUPPLIER_LT = 1;
    static const int P2P_SUPPLIER_SY = 2;
    static const int P2P_SUPPLIER_TUTK = 3;
    static const int P2P_SUPPLIER_VR = 4;

    static const int P2P_DYNAMIC_ALLOCATE = 0;
    static const int P2P_DEVICE_BUILDIN = 1;

    static const int GET_TIMEZONE_RETRY_TIMES = 5;

    static const int DEVICE_TYPE_DOORBELL = 0;
    static const int DEVICE_TYPE_IPC = 1;

    static const std::string ONLINE;
    static const std::string OFFLINE;

    static const int EVENT_MESSAGE_ALL = 0;
    static const int EVENT_MESSAGE_UNREAD = 1;
    static const int EVENT_MESSAGE_READ = 2;

    static const int DEVICE_BELONGTO_USER = 0;
    static const int DEVICE_SHAREDWITH_USER = 1;

    static const unsigned int UNUSED_INPUT_UINT = 0xFFFFFFFF;

    typedef struct _Relation
    {
        std::string m_strUsrID;
        std::string m_strDevID;
        int m_iRelation;
        std::string m_strBeginDate;
        std::string m_strEndDate;
        std::string m_strCreateDate;
        int m_iStatus;
        std::string m_strExtend;
    } RelationOfUsrAndDev;

    typedef struct _RelationOfUsr
    {
        std::string m_strUsrID;
        std::string m_strRelationOfUsrID;
        int m_iRelation;
        std::string m_strCreateDate;
        int m_iStatus;
        std::string m_strExtend;
    } RelationOfUsr;

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
        std::string m_strLTUserSite;
        std::string m_strLTUserSiteRC4Key;
        std::string m_strFileServerURL;
        std::string m_strGetIpInfoSite;
        std::string m_strMemAddressGlobal;
        std::string m_strMemPortGlobal;
        std::string m_strUserLoginMutex;
        std::string m_strUserAllowDiffTerminal;
        std::string m_strUserKickoutType;
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

    AccessManager(const ParamInfo &pinfo);
    ~AccessManager();
    bool Init();

    bool GetMsgType(const std::string &strMsg, int &iMsgType);

    bool PreCommonHandler(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool RegisterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool UnRegisterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryUsrInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifyUsrInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool LoginReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool LoginLTUserSiteReq(const std::string &strUserName, const std::string &strPassword,
        const std::string &strLTUserSite, const std::string &strLTRC4Key, const std::string &strSrcID, std::string &strUserID);

    bool LogoutReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ShakehandReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool AddDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DelDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryDevInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool SharingDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool CancelSharedDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool AddFriendsReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DelFriendsReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryFriendsReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool LoginReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool P2pInfoReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ShakehandReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool LogoutReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool AddFileReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeleteFileReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DownloadFileReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryFileReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool P2pInfoReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool RetrievePwdReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryTimeZoneReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAccessDomainNameReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAccessDomainNameReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryUpgradeSiteReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool GetDeviceAccessRecordReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool GetUserAccessRecordReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAppUpgradeReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryFirmwareUpgradeReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryUploadURLReqMgr(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool AddConfigurationReqMgr(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeleteConfigurationReqMgr(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifyConfigurationReqMgr(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAllConfigurationReqMgr(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifyDevicePropertyReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryDeviceParameterReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryIfP2pIDValidReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryPlatformPushStatusReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeviceEventReportReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryAllDeviceEventReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeleteDeviceEventReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifyDeviceEventReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);
    
    bool AddStorageDetailReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool DeleteStorageDetailReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool ModifyStorageDetailReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryStorageDetailReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryRegionStorageInfoReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

private:
    void InsertUserToDB(const InteractiveProtoHandler::User &UsrInfo);

    void UpdateUserInfoToDB(const InteractiveProtoHandler::User &UsrInfo);

    void UnregisterUserToDB(const std::string &strUserID, const int iStatus);

    bool QueryRelationExist(const std::string &strUserID, const std::string &strDevID, const int iRelation, bool &blExist, const bool IsNeedCache = true);

    bool QueryRelationByUserID(const std::string &strUserID, std::list<InteractiveProtoHandler::Relation> &RelationList, std::list<std::string> &strDevNameList,
        const unsigned int uiAppType, const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    bool QueryRelationByDevID(const std::string &strDevID, std::list<InteractiveProtoHandler::Relation> &RelationList, std::list<std::string> &strUserNameList,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    bool ValidUser(std::string &strUserID, std::string &strUserName, bool &blUserExist, const std::string &strUserPwd, const bool IsForceFromDB = false);

    bool GetMySqlUUID(std::string &strUuid);

    void UserInfoSqlCB(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result);

    void DevInfoRelationSqlCB(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, 
        std::list<InteractiveProtoHandler::Relation> *pRelationList, std::list<std::string> *pStrDevNameList);

    void SessionTimeoutProcessCB(const std::string &strSessionID);

    void InsertDeviceToDB(const std::string &strUuid, const InteractiveProtoHandler::Device &DevInfo);

    void InsertRelationToDB(const std::string &strUuid, const RelationOfUsrAndDev &relation, const std::string &strDeviceName);

    void RemoveRelationToDB(const RelationOfUsrAndDev &relation);

    void DelDeviceToDB(const std::list<std::string> &strDevIDList, const int iStatus);

    void ModDeviceToDB(const InteractiveProtoHandler::Device &DevInfo);

    void ModifySharedDeviceNameToDB(const std::string &strUserID, const std::string &strDeviceID, const std::string &strDeviceName);
    
    void SharingRelationToDB(const RelationOfUsrAndDev &relation);

    void CancelSharedRelationToDB(const RelationOfUsrAndDev &relation);

    bool QueryUserInfoToDB(const std::string &strUserID, InteractiveProtoHandler::User &usr, const bool IsNeedCache = true);

    bool QueryDevInfoToDB(const std::string &strUserID, const std::string &strDevID, const unsigned int uiDeviceShared,
        InteractiveProtoHandler::Device &dev, const bool IsNeedCache = true);

    void AddFriendsToDB(const RelationOfUsr &relation);

    void DelFriendsToDB(const std::string &strUserID, const std::list<std::string> &FriendIDList, const int iStatus);

    bool QueryUserRelationExist(const std::string &strUserID, const std::string &strFriendsID, const int iRelation, bool &blExist, const bool IsNeedCache = true);

    bool QueryUserRelationInfoToDB(const std::string &strUserID, const int iRelation, std::list<std::string> &strRelationIDList,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10, const bool IsNeedCache = true);

    void AddDeviceFileToDB(const std::string &strDevID, const std::list<InteractiveProtoHandler::File> &FileInfoList,
        std::list<std::string> &FileIDFailedList);

    void DeleteFileToDB(const std::string &strUserID, const std::list<std::string> &FileIDList, const int iStatus);

    bool DownloadFileToDB(const std::string &strUserID, const std::list<std::string> &FileIDList,
        std::list<InteractiveProtoHandler::FileUrl> &FileUrlList, const bool IsNeedCache = true);

    bool QueryFileToDB(const std::string &strUserID, const std::string &strDevID, std::list<InteractiveProtoHandler::File> &FileInfoList,
        const unsigned int uiBusinessType, const std::string &strBeginDate, const std::string &strEndDate,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10, const bool IsNeedCache = true);

    bool QueryOwnerUserIDByDeviceID(const std::string &strDevID, std::string &strUserID);

    bool IsUserPasswordValid(const std::string &strUserID, const std::string &strUserPassword);

    void AddNoOwnerFile(const std::string &strUserID, const std::string &strDevID);

    void UpdateFileUserIDToDB(const std::string &strUserID, std::list<std::string> &strIDList);

    bool InsertFileToDB(const InteractiveProtoHandler::File &FileInfo);

    void FileInfoSqlCB(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn,
        std::list<InteractiveProtoHandler::File> *pFileInfoList);

    bool CheckEmailByUserName(const std::string &strUserName, const std::string &strEmail);

    void ResetUserPasswordToDB(const std::string &strUserName, const std::string &strUserPassword);

    void SendUserResetPasswordEmail(const std::string &strUserName, const std::string &strUserPassword, const std::string &strEmail);

    bool GetTimeZone(const std::string &strIpAddress, std::string &strCountryCode, std::string &strCountryNameEn, std::string &strCountryNameZh,
        std::string &strTimeZone);

    bool QueryAccessDomainInfoByArea(const std::string &strCountryID, const std::string &strAreaID, AccessDomainInfo &DomainInfo);

    bool RefreshAccessDomainName();

    bool QueryUpgradeSiteToDB(std::string &strUpgradeUrl, unsigned int &uiLease);

    bool QueryAppUpgradeToDB(const std::string &strCategory, const std::string &strSubCategory, const std::string &strCurrentVersion,
        InteractiveProtoHandler::AppUpgrade &appUpgrade);
    
    bool QueryFirmwareUpgradeToDB(const std::string &strCategory, const std::string &strSubCategory, const std::string &strCurrentVersion,
        InteractiveProtoHandler::FirmwareUpgrade &firmwareUpgrade);

    void ConfigurationInfoSqlCB(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn,
        boost::any &Result, boost::shared_ptr<InteractiveProtoHandler::Configuration> pConfiguration);

    bool IsValidConfiguration(const std::string &strCategory, const std::string &strSubCategory);

    void InsertConfigurationToDB(const InteractiveProtoHandler::Configuration &configuration);

    void DeleteConfigurationToDB(const std::string &strCategory, const std::string &strSubCategory, const int iStatus);

    bool DeleteUpgradeFile(const std::string &strCategory, const std::string &strSubCategory);

    void ModifyConfigurationToDB(const InteractiveProtoHandler::Configuration &configuration);

    bool QueryAllConfigurationToDB(std::list<InteractiveProtoHandler::Configuration> &configurationList,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    bool IsValidP2pInfo(const unsigned int uiP2pSupplier, const std::string &strP2pID, const std::string &strDeviceID);

    void InsertP2pInfoToDB(const unsigned int uiP2pSupplier, const std::string &strP2pID, const std::string &strDeviceID, const unsigned int uiBuildin);

    bool IsValidDeviceDomain(const std::string &strDeviceID, const std::string &strDeviceDomain);

    bool IsValidP2pIDProperty(const std::string &strDeviceID, const std::string &strP2pID);

    void InsertDevPropertyToDB(const InteractiveProtoHandler::LoginReq_DEV &loginDevReq);

    void UpdateDevicePropertyToDB(const InteractiveProtoHandler::ModifyDevicePropertyReq_DEV &modifyDevicePropertyReq);

    bool QueryDevPropertyByDevDomain(const std::string &strDeviceDomain, std::string &strDeviceID, std::string &strP2pID);

    bool QueryDevIDByDevP2pID(const std::string &strDeviceP2pID, std::string &strDeviceID);

    void InsertDoorbellParameterToDB(const InteractiveProtoHandler::LoginReq_DEV &loginDevReq);
   
    void UpdateDoorbellParameterToDB(const std::string &strDeviceID, const InteractiveProtoHandler::DoorbellParameter &doorbellParameter);

    bool QueryDeviceParameterToDB(const std::string &strDeviceID, const unsigned int uiDeviceType, const std::string &strQueryType,
        InteractiveProtoHandler::DoorbellParameter &doorbellParameter);

    bool QueryDoorbellParameterToDB(const std::string &strDeviceID, const std::string &strQueryType,
        InteractiveProtoHandler::DoorbellParameter &doorbellParameter);

    bool QueryDeviceDateAndVersionToDB(const std::string &strDeviceID, const unsigned int uiDeviceType,
        std::string &strUpdateDate, std::string &strVersion);

    bool QueryIfP2pIDValidToDB(const std::string &strP2pID);

    bool QueryIfDeviceReportedToDB(const std::string &strP2PID, const unsigned int uiDeviceType, std::string &strDeviceID);

    bool QueryPlatformPushStatusToDB(std::string &strStatus);

    void InsertDeviceEventReportToDB(const std::string &strEventID, const std::string &strDeviceID, const unsigned int uiDeviceType, const unsigned int uiEventType,
        const unsigned int uiEventState, const unsigned int uiMessageStatus, const std::string &strFileID, const std::string &strEventTime);

    bool QueryAllDeviceEventToDB(const std::string &strDeviceID, const unsigned int uiEventType, const unsigned int uiReadState,
        std::list<InteractiveProtoHandler::DeviceEvent> &deviceEventList, const std::string &strBeginDate, const std::string &strEndDate,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    void UpdateEventReadStatusToDB(std::list<std::string> strEventIDList, const unsigned int uiReadStatus);

    void DeleteDeviceEventToDB(const std::string &strEventID);

    void ModifyDeviceEventToDB(const std::string &strEventID, const unsigned int uiEventState, const std::string &strUpdateTime,
        const std::string &strFileID);

    bool QuerySharedDeviceNameToDB(const std::string &strUserID, const std::string &strDeviceID, std::string &strDeviceName);

    void RemoveExpiredDeviceEventToDB(const std::string &strDeviceID, const bool blRemoveAll = false);

    void RemoveExpiredDeviceEventFile(const std::string &strDeviceID, const unsigned int uiExpiredTime);

    bool RemoveRemoteFile(const std::string &strFileID);

    bool QueryDeviceEventExpireTimeToDB(unsigned int &iExpireTime);

    void InsertStorageDetailToDB(const InteractiveProtoHandler::StorageDetail &storageDetail);

    void DeleteStorageDetailToDB(const std::string &strObjectID);

    void UpdateStorageDetailToDB(const InteractiveProtoHandler::StorageDetail &storageDetail);

    bool QueryStorageDetailToDB(const std::string &strObjectID, InteractiveProtoHandler::StorageDetail &storageDetail, int &iErrorCode);

    bool QueryRegionStorageInfoToDB(unsigned int &uiUsedSize, unsigned int &uiTotalSize);

    void UpdateDeviceEventStoredTime();

    void FileProcessHandler(const std::string &strEventID, const std::string &strFileID);

private:
    ParamInfo m_ParamInfo;

    Runner m_DBRuner;
    boost::shared_ptr<InteractiveProtoHandler> m_pProtoHandler;
    
    MysqlImpl *m_pMysql;
    DBInfoCacheManager m_DBCache;

    typedef struct
    {
        std::string strType;
        std::string strValue;
    } ValueInDB;
    
    boost::atomic_uint64_t m_uiMsgSeq;
    
    SessionMgr m_SessionMgr;

    std::map<std::string, std::list<AccessDomainInfo>> m_AreaDomainMap;
    boost::mutex m_domainMutex;

    boost::shared_ptr<ClusterAccessCollector> m_pClusterAccessCollector;

    TimeOutHandler m_DBTimer;

    unsigned long long m_ulTimerTimes;

    Runner m_EventFileProcessRunner;

    boost::shared_ptr<InterProcessHandler> m_MsgSender;
    boost::shared_ptr<InterProcessHandler> m_MsgReceiver;

};



#endif