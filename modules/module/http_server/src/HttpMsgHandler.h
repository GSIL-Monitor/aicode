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

class InteractiveProtoHandler;

class CacheMgr;

class HttpMsgHandler : public boost::noncopyable
{
public:
    static const std::string REGISTER_USER_ACTION;
    static const std::string UNREGISTER_USER_ACTION;
    static const std::string QUERY_USER_INFO_ACTION;
    static const std::string MODIFY_USER_INFO_ACTION;
    static const std::string USER_LOGIN_ACTION;
    static const std::string USER_LOGOUT_ACTION;
    static const std::string USER_SHAKEHAND_ACTION;
    static const std::string ADD_DEVICE_ACTION;
    static const std::string DELETE_DEVICE_ACTION;
    static const std::string MODIFY_DEVICE_ACTION;
    static const std::string QUERY_DEVICE_INFO_ACTION;
    static const std::string QUERY_DEVICE_INFO_MULTIPLE_ACTION;
    static const std::string QUERY_DEVICE_OF_USER_ACTION;
    static const std::string QUERY_USER_OF_DEVICE_ACTION;
    static const std::string SHARING_DEVICE_ACTION;
    static const std::string CANCELSHARED_DEVICE_ACTION;
    static const std::string ADD_FRIEND_ACTION;
    static const std::string DELETE_FRIEND_ACTION;
    static const std::string QUERY_FRIEND_ACTION;
    static const std::string P2P_INFO_ACTION;


    static const std::string DEVICE_LOGIN_ACTION;
    static const std::string DEVICE_P2P_INFO_ACTION;
    static const std::string DEVICE_SHAKEHAND_ACTION;
    static const std::string DEVICE_LOGOUT_ACTION;
    static const std::string DEVICE_SET_PROPERTY_ACTION;


    static const std::string QUERY_USER_FILE_ACTION;
    static const std::string DOWNLOAD_USER_FILE_ACTION;
    static const std::string DELETE_USER_FILE_ACTION;
    static const std::string ADD_FILE_ACTION;
    static const std::string RETRIEVE_PWD_ACTION;
    static const std::string DEVICE_QUERY_TIMEZONE_ACTION;

    static const std::string USER_QUERY_ACCESS_DOMAIN_ACTION;
    static const std::string DEVICE_QUERY_ACCESS_DOMAIN_ACTION;
    static const std::string DEVICE_QUERY_UPDATE_SERVICE_ACTION;

    static const std::string QUERY_UPLOAD_URL_ACTION;
    static const std::string ADD_CONFIG_ACTION;
    static const std::string DELETE_CONFIG_ACTION;
    static const std::string MOD_CONFIG_ACTION;
    static const std::string QUERY_CONFIG_ACTION;

    static const std::string QUERY_APP_UPGRADE_ACTION;
    static const std::string QUERY_DEV_UPGRADE_ACTION;

    static const std::string QUERY_DEVICE_PARAM_ACTION;

    static const std::string CHECK_DEVICE_P2PID_ACTION;
    static const std::string QUERY_PUSH_STATUS_ACTION;
    static const std::string DEVICE_EVENT_REPORT_ACTION;
    static const std::string QUERY_DEVICE_EVENT_ACTION;
    static const std::string DELETE_DEVICE_EVENT_ACTION;
    static const std::string MODIFY_DEVICE_EVENT_ACTION;

    static const std::string ADD_USER_SPACE_ACTION;
    static const std::string DELETE_USER_SPACE_ACTION;
    static const std::string MODIFY_USER_SPACE_ACTION;
    static const std::string QUERY_USER_SPACE_ACTION;
    static const std::string QUERY_STORAGE_SPACE_ACTION;

    static const std::string CMS_CALL_ACTION;
    static const std::string REGISTER_CMSCALL_ACTION;
    static const std::string UNREGISTER_CMSCALL_ACTION;
    
    static const std::string QUERY_SHARING_DEVICE_LIMIT_ACTION;

    static const std::string QUERY_DEVICE_CAPACITY_ACTION;
    static const std::string QUERY_ALL_DEVICE_CAPACITY_ACTION;

    static const std::string QUERY_DEVICE_P2PID;

    static const std::string UPLOAD_USR_CFG_ACTION;

    static const std::string QUERY_USER_CFG_ACTION;

    static const std::string QUERY_DEVICE_P2PID_ASP;

    static const std::string ADD_BLACK_ID;
    static const std::string REMOVE_BLACK_ID;
    static const std::string QUERY_ALL_BLACK_LIST;


    typedef struct _ParamInfo
    {
        std::string m_strRemoteAddress;
        std::string m_strRemotePort;
        unsigned int m_uiShakehandOfChannelInterval;
        std::string m_strSelfID;
        unsigned int m_uiCallFuncTimeout;
        unsigned int m_uiThreadOfWorking;
    } ParamInfo;

    HttpMsgHandler(const ParamInfo &parminfo, CacheMgr &sgr);
    ~HttpMsgHandler();

    bool BlacklistHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ParseMsgOfCompact(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool RegisterUserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool UnRegisterUserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryUserInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyUserInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool RetrievePwdHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool UserLoginHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool UserLogoutHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ConfigInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ShakehandHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool AddDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeleteDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryDeviceMultipleHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryDevicesOfUserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryUsersOfDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool SharingDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool CancelSharedDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool AddFriendsHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeleteFriendsHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryFriendHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool P2pInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool CheckDeviceP2pidHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool DeviceLoginHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeviceP2pInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeviceShakehandHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeviceLogoutHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);
    
    bool DeviceSetPropertyHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool QueryUserFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DownloadUserFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeleteUserFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);
    
    bool AddFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeviceQueryTimeZoneHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool UserQueryAccessDomainNameHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeviceQueryAccessDomainNameHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeviceQueryUpdateServiceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryUploadURLHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool AddConfigurationHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeleteConfigurationHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyConfigurationHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryConfigurationHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryAppUpgradeHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryDevUpgradeHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryDevParamHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryPushStatusHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeviceEventReportHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryDeviceEventHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeleteDeviceEventHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyDeviceEventHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool AddUserSpaceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeleteUserSpaceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyUserSpaceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryUserSpaceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryStorageSpaceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool CmsCallHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool RegisterCmsCallHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool UnregisterCmsCallHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool QuerySharingDeviceLimitHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool QueryDeviceCapacityHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryAllDeviceCapacityHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool QueryDeviceP2pIDHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);
    
    bool UploadUserCfgHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);
    bool QueryUserCfgHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryDeviceP2pIDAspHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool AddBlackIDHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);
    bool RemoveBlackIDHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);
    bool QueryAllBlackListHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    void WriteMsg(const std::map<std::string, std::string> &MsgMap, MsgWriter writer, const bool blResult = true, boost::function<void(void*)> PostFunc = NULL);

    void WriteMsg(const std::string &strMsg, MsgWriter writer, const bool blResult = true);

private:

    bool AddDeviceFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool AddUserFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);
    
private:
    typedef struct
    {
        std::string m_strDevid;
        unsigned int m_uiDevType;
        std::string m_strDomainName;
        std::string m_strCorpid;
        std::string m_strDvsname;
        std::string m_strDvsip;
        std::string m_strWebport;
        std::string m_strCtrlport;
        std::string m_strProtocol;
        std::string m_strUserid;
        std::string m_strPassword;
        std::string m_strModel;
        std::string m_strPostfrequency;
        std::string m_strVersion;
        std::string m_strStatus;
        std::string m_strServerIp;
        std::string m_strServerPort;
        std::string m_strTransfer;
        std::string m_strMobilePort;
        std::string m_strChannelCount;
        std::string m_strP2pid;
        std::string m_strDvsip2;
        std::string m_strDoorbellName;
        std::string m_strSerialNum;
        std::string m_strDoorbellP2pid;
        std::string m_strBatteryCap;
        std::string m_strChargingState;
        std::string m_strWifiSig;
        std::string m_strVolumeLevel;
        std::string m_strVersionNum;
        std::string m_strChannelNum;
        std::string m_strCodingType;
        std::string m_strPirAlarmSwitch;
        std::string m_strDoorbellSwitch;
        std::string m_strPirAlarmLevel;
        std::string m_strPirIneffectiveTime;
        std::string m_strCurrentWifi;
        std::string m_strSubCategory;
        std::string m_strDisturbMode;
        std::string m_strPreventDisassembly;
        std::string m_strExtend;
    } DeviceProperty;

    typedef struct
    {
        std::string m_strDevID;
        std::string m_strDevPwd;
        std::string m_strDevIpAddress;
        unsigned int m_uiDevType;
        unsigned int m_uiP2pType;
        std::string m_strP2pserver;
        std::string m_strP2pID;
        unsigned int m_uiP2pidBuildin;
        std::string m_strUserName;
        std::string m_strUserPwd;
        std::string m_strDistributor;
        std::string m_strOtherProperty;
        std::string m_strDomainName;
    } DeviceLoginInfo;

    typedef struct
    {
        std::string m_strDevID;
        std::string m_strDevName;
        std::string m_strDevPwd;
        std::string m_strDevType;
        std::string m_strDevExtend;
        std::string m_strDevInnerInfo;
        std::string m_strP2pid;
        std::string m_strDomainname;
        std::string m_strIpaddress;
        std::string m_strOnline;
    } DeviceIf;

    typedef struct
    {
        std::string m_strDevID;
        unsigned int m_uiDevType;
        unsigned int m_uiEventType;
        unsigned int m_uiEventStatus;
        std::string m_strFileID;
        std::string m_strEventID;
        std::string m_strFileURL;
        std::string m_strEventTime;
        unsigned int m_uiViewAlready;
        std::string m_strThumbnailURL;
    } Event;

    typedef struct
    {
        unsigned int m_uiDomainID;
        std::string m_strStorageName;
        unsigned int m_uiOverlapType;
        unsigned int m_uiStorageTimeUpLimit;
        unsigned int m_uiStorageTimeDownLimit;
        std::string m_strBeginDate;
        std::string m_strEndDate;
        std::string m_strExtend;

    } SpaceInfo;

    typedef struct
    {
        unsigned int m_uiDomainID;
        unsigned int m_uiSpaceSize;
        unsigned int m_uiSpaceSizeUsed;

    } StorageInfo;

    typedef struct
    {
        std::string m_strAddressBest;
        std::list<std::string> m_strAddressOthersList;

    } NotifyServer;

    typedef struct
    {
        unsigned int m_uiDevType;
        std::list<std::string> m_strCapacityList;
    } DeviceCapacity;

    typedef struct
    {
        std::string m_strUpdateTime;
        std::string m_strWebPort;
        std::string m_strMobilePort;
        std::string m_strChannelCount;
        std::string m_strDeviceSN;
        std::string m_strP2pID;
        std::string m_strExtend;
    } DevP2pIDInfo;


    typedef struct
    {
        unsigned int m_uiBusinessType;
        std::string m_strUserID;
        std::string m_strFileID;
        std::string m_strExtend;
        std::string m_strVersion;
        std::string m_strCfgURL;
    } UserCfg;

    typedef struct
    {
        std::string m_strBlackID;
        unsigned int m_uiIDType;
        std::string m_strExtend;
    } BlkObj;

    int RspFuncCommonAction(CommMsgHandler::Packet &pt, int *piRetCode, RspFuncCommon rspfunc);

    bool PreCommonHandler(const std::string &strMsgReceived, int &iRetCode);

    bool RegisterUser(const std::string &strUserName, const std::string &strUserPwd, 
        const std::string &strType, const std::string &strExtend, const std::string &strAliasName, const std::string &strEmail, std::string &strUserID);

    bool UnRegisterUser(const std::string &strSid, const std::string &strUserID, const std::string &strUserName, const std::string &strUserPwd);
    
    template<typename T>
    bool UserLogin(const std::string &strUserName, const std::string &strUserPwd, const unsigned int uiType, const unsigned int uiDevIsolation,
        const unsigned int uiTerminalType, std::list<T> &RelationList,
        std::string &strUserID, std::string &strSid, std::list<std::string> &strDevNameList);

    template<typename T>
    bool QueryUserInfo(const std::string &strSid, const std::string &strUserID, T &UserInfo);

    bool ModifyUserInfo(const std::string &strSid, const std::string &strUserID, const std::string &strUserName, const std::string &strNewUserPwd, 
        const std::string &strOldUserPwd, const unsigned int uiType, const std::string &strExtend, const std::string &strAliasName, const std::string &strEmail);

    bool UserLogout(const std::string &strSid, const std::string &strUserID);

    bool Shakehand(const std::string &strSid, const std::string &strUserID);

    bool AddDevice(const std::string &strSid, const std::string &strDevReportCheck, const std::string &strAllowDuplicate, const std::string &strUserID, const DeviceIf &devif,
        std::string &strDevID, std::string &strUserName, std::string &strDevType);

    bool DeleteDevice(const std::string &strSid, const std::string &strUserID, const std::string &strDevID);

    bool ModifyDevice(const std::string &strSid, const std::string &strUserID, const DeviceIf &devif, const unsigned int uiDevShared);

    template<typename T>
    bool QueryDeviceInfo(const std::string &strSid, const std::string &strDevID, T &DevInfo, std::string &strUpdateDate, std::string &strVersion, std::string &strOnline,
        const unsigned int uiDevShared, const std::string &strUserID);

    bool QueryDeviceInfoMultiple(const std::string &strSid, std::list<std::string> &strDevIDList, std::list<DeviceIf> &DevInfoList);

    template<typename T>
    bool QueryDevicesOfUser(const std::string &strSid, const std::string &strUserID, const unsigned int uiBeginIndex, const unsigned int uiDevIsolation, std::list<T> &RelationList,
        std::list<std::string> &strDevNameList);
    
    template<typename T>
    bool QueryUsersOfDevice(const std::string &strSid, const std::string &strDevID, const unsigned int uiBeginIndex, std::list<T> &RelationList,
        std::list<std::string> &strUsrNameList);

    bool SharingDevice(const std::string &strSid, const std::string &strUserID, const std::string &strUserNameShared, const std::string &strDevID, const std::string &strRelation,
        const std::string &strBeginDate, const std::string &strEndDate);
   
    bool CancelSharedDevice(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const std::string &strRelation);

    bool AddFriends(const std::string &strSid, const std::string &strUserID, const std::string &strFriendID);

    bool DeleteFriends(const std::string &strSid, const std::string &strUserID, const std::string &strFriendID);

    bool QueryFriends(const std::string &strSid, const std::string &strUserID, const unsigned int uiBeginIndex, std::list<std::string> &FriendList);

    bool P2pInfo(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const std::string &strUserIpAddress, const unsigned int uiP2pType,
        std::string &strP2pServer, std::string &strP2pID, unsigned int &uiLease, std::string &strLicenseKey, std::string &strPushID);
        
    bool DeviceLogin(const DeviceLoginInfo &DevLogInfo, std::string &strSid, std::string &strValue);

    bool DeviceP2pInfo(const std::string &strSid, const std::string &strDevID, const std::string &strDevIpAddress, const unsigned int uiP2pType,
        const std::string &strDomainName, std::string &strP2pServer, std::string &strP2pID, unsigned int &uiLease, std::string &strLicenseKey, std::string &strPushID);

    bool DeviceShakehand(const std::string &strSid, const std::string &strDevID);

    bool DeviceLogout(const std::string &strSid, const std::string &strDevID);
    
    bool DeviceSetProperty(const std::string &strSid, const DeviceProperty &devpt);

    template<typename T>
    bool QueryUserFile(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const unsigned int uiBeginIndex, 
        const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiBussinessType, std::list<T> &FileList);

    bool DownloadUserFile(const std::string &strSid, const std::string &strUserID, const std::string &strFileID, std::string &strFileUrl);

    bool DeleteUserFile(const std::string &strSid, const std::string &strUserID, const std::string &strFileID);

    bool AddDeviceFile(const std::string &strDevID, const std::string &strRemoteFileID, const std::string &strDownloadUrl, const std::string &strFileName,
        const std::string &strSuffixName, const unsigned long int uiFileSize, const std::string &strFileCreatedate, const std::string &strExtend, const unsigned int uiBussinessType);

    bool RetrievePwd(const std::string &strUserName, const std::string &strEmail, const unsigned int uiType);

    bool DeviceQueryTimeZone(const std::string &strSid, const std::string &strDevID, const std::string &strDevIpAddress, std::string &strCountrycode,
        std::string &strCountryNameEn, std::string &strCountryNameZh, std::string &strTimeZone);

    bool UserQueryAccessDomainName(const std::string &strIpAddress, const std::string &strUserName, const unsigned int  uiBusinessType,
        std::string &strAccessDomainName, std::string &strLease);

    bool DeviceQueryAccessDomainName(const std::string &strIpAddress, const std::string &strDevID, const unsigned int uiBusinessType,
        std::string &strAccessDomainName, NotifyServer &nts, std::string &strLease);

    bool DeviceQueryUpdateService(const std::string &strSid, const std::string &strIpAddress, const std::string &strDevID, std::string &strUpdateAddress, std::string &strLease);

    bool AddConfiguration(const std::string &strCategory, const std::string &strSubcategory, const std::string &strLatestVersion, const std::string &strDesc,
        const std::string &strForceVersion, const std::string &strServerAddress, const std::string &strFilename, const std::string &strFileID, 
        const unsigned int uiFileSize, const unsigned int uiLease);

    bool DeleteConfiguration(const std::string &strCategory, const std::string &strSubcategory);

    bool ModifyConfiguration(const std::string &strCategory, const std::string &strSubcategory, const std::string &strLatestVersion, const std::string &strDesc,
        const std::string &strForceVersion, const std::string &strServerAddress, const std::string &strFilename, const std::string &strFileID,
        const unsigned int uiFileSize, const unsigned int uiLease);

    template<typename T>
    bool QueryConfiguration(const unsigned int uiBeginIndex, std::list<T> &CfgList);

    bool QueryUploadURL(std::string &strURL);

    bool QueryAppUpgrade(const std::string &strCategory, const std::string &strSubcategory,const std::string &strCurrentVersion, std::string &strNewVersionValid, 
        std::string &strAppName, std::string &strAppPath, unsigned int &uiAppSize, std::string &strNewVersion, std::string &strDesc, 
        std::string &strForceUpgrade, std::string &strUpdateDate);

    bool QueryDevUpgrade(const std::string &strCategory, const std::string &strSubcategory, const std::string &strCurrentVersion, 
        const std::string &strDevID, std::string &strNewVersionValid,
        std::string &strFirmwareName, std::string &strFirmwarePath, unsigned int &uiFirmwareSize, std::string &strNewVersion, std::string &strDesc,
        std::string &strForceUpgrade, std::string &strUpdateDate);

    bool QueryDevParam(const std::string &strSid, const std::string &strDevID, const unsigned int uiDevType, const std::string &strQueryType, DeviceProperty &devpt);

    bool CheckDeviceP2pid(const std::string &strP2pid, const unsigned int uiP2pType);

    bool QueryPushStatus(const std::string &strDevID, std::string &strPushStatus);

    bool DeviceEventReport(const std::string &strSid, Event &ev);

    bool QueryDeviceEvent(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const unsigned int uiDevShared, const unsigned int uiEventType,
        const unsigned int uiView, const unsigned int uiBeginIndex, const std::string &strBeginDate, const std::string &strEndDate, std::list<Event> &evlist);

    bool DeleteDeviceEvent(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const std::string &strEventID);

    bool ModifyDeviceEvent(const std::string &strSid, const Event &ev);

    bool AddUserSpace(const std::string &strSid, const std::string &strUserID, const SpaceInfo &stif);

    bool DeleteUserSpace(const std::string &strSid, const std::string &strUserID, const unsigned int &uiDomainID);

    bool ModifyUserSpace(const std::string &strSid, const std::string &strUserID, const SpaceInfo &stif);

    bool QueryUserSpace(const std::string &strSid, const std::string &strUserID, const unsigned int &uiDomainID, SpaceInfo &stif);

    bool QueryStorageSpace(const std::string &strSid, const std::string &strUserID, StorageInfo &stif);

    bool CmsCall(const std::string &strCmsCallMsg, std::string &strRetMsg);

    bool RegisterCmsCall(const std::string &strCmsID, std::list<std::string> &strCmsP2pIDList, const std::string &strDeviceMac, const std::string &strDeviceP2pID,
        std::string &strAddress, std::string &strPort, std::list<std::string> &strP2pIDFailedList);

    bool UnregisterCmsCall(const std::string &strCmsID);

    bool QuerySharingDeviceLimit(const std::string &strSid, const std::string &strUserID, unsigned int &uiCurrentLimitNum, unsigned int &uiUsedNum);

    bool QueryDeviceCapacity(const std::string &strSid, const std::string &strUserID, const unsigned int uiDevType, DeviceCapacity &devcap);

    bool QueryAllDeviceCapacity(const std::string &strSid, const std::string &strUserID, std::list<DeviceCapacity> &devcaplist);

    bool QueryDeviceP2pID(const std::string &strDomainName, DevP2pIDInfo &p2pinfo);

    bool UploadUserCfg(const std::string &strSid, UserCfg &ucfg);

    bool QueryUserCfg(const std::string &strSid, UserCfg &ucfg);

    void GetDeviceP2pIDAspXMLReport(const DevP2pIDInfo &p2pinfo, std::string &strXMLReport);

    bool BlacklistCompare(const std::string &strKey, const std::string &strValue);


    bool AddBlackID(const std::string &strSid, const std::string &strUserID, const BlkObj &blkobj);

    bool RemoveBlackID(const std::string &strSid, const std::string &strUserID, const std::string &strBlkID);

    bool QueryAllBlack(const std::string &strSid, const std::string &strUserID, unsigned int uiIDType, unsigned int uiBeginIndex, std::list<BlkObj> &blkobjlist);

private:
    bool ValidDatetime(const std::string &strDatetime);

    bool GetValueList(const std::string &strValue, std::list<std::string> &strValueList);

private:
    ParamInfo m_ParamInfo;
    boost::shared_ptr<InteractiveProtoHandler> m_pInteractiveProtoHandler;

    CacheMgr &m_CacheMgr;

private:
    static const std::string SUCCESS_CODE;
    static const std::string SUCCESS_MSG;
    static const std::string FAILED_CODE;
    static const std::string FAILED_MSG;

    static const int USER_PWD_RESET = 1016;

};

#endif
