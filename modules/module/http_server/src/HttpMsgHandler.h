#ifndef FILE_HANDLER
#define FILE_HANDLER

#include "NetComm.h"
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include "FCGIManager.h"

/************************************************************************/
/*负责实现消息处理具体细节，向FCGIManager注册处理函数，
 *其中包括实际的业务消息处理，例如：将用户注册的信息汇总，
 *调用接口实现用户注册的业务动作。*/
/************************************************************************/

class InteractiveProtoHandler;

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

    
    typedef struct _ParamInfo
    {
        std::string m_strRemoteAddress;
        std::string m_strRemotePort;
        unsigned int m_uiShakehandOfChannelInterval;
        std::string m_strSelfID;
        unsigned int m_uiCallFuncTimeout;
        unsigned int m_uiThreadOfWorking;
    } ParamInfo;

    HttpMsgHandler(const ParamInfo &parminfo);
    ~HttpMsgHandler();

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

    bool QueryDevicesOfUserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryUsersOfDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool SharingDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool CancelSharedDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool AddFriendsHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeleteFriendsHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryFriendHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool P2pInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool DeviceLoginHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeviceP2pInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeviceShakehandHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeviceLogoutHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


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


    void WriteMsg(const std::map<std::string, std::string> &MsgMap, MsgWriter writer, const bool blResult = true, boost::function<void(void*)> PostFunc = NULL);

private:

    bool AddDeviceFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool AddUserFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);
    
private:

    bool PreCommonHandler(const std::string &strMsgReceived);

    bool RegisterUser(const std::string &strUserName, const std::string &strUserPwd, 
        const std::string &strType, const std::string &strExtend, const std::string &strAliasName, const std::string &strEmail, std::string &strUserID);

    bool UnRegisterUser(const std::string &strSid, const std::string &strUserID, const std::string &strUserName, const std::string &strUserPwd);
    
    template<typename T>
    bool UserLogin(const std::string &strUserName, const std::string &strUserPwd, const unsigned int uiTerminalType, std::list<T> &RelationList,
        std::string &strUserID, std::string &strSid, std::list<std::string> &strDevNameList);

    template<typename T>
    bool QueryUserInfo(const std::string &strSid, const std::string &strUserID, T &UserInfo);

    bool ModifyUserInfo(const std::string &strSid, const std::string &strUserID, const std::string &strUserName, const std::string &strNewUserPwd, 
        const std::string &strOldUserPwd, const unsigned int uiType, const std::string &strExtend, const std::string &strAliasName, const std::string &strEmail);

    bool UserLogout(const std::string &strSid, const std::string &strUserID);

    bool Shakehand(const std::string &strSid, const std::string &strUserID);

    bool AddDevice(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const std::string &strDevName,
        const std::string &strDevPwd, const std::string &strDevType, const std::string &strDevExtend, const std::string &strDevInnerInfo);

    bool DeleteDevice(const std::string &strSid, const std::string &strUserID, const std::string &strDevID);

    bool ModifyDevice(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const std::string &strDevName,
        const std::string &strDevPwd, const std::string &strDevType, const std::string &strDevExtend, const std::string &strDevInnerInfo);

    template<typename T>
    bool QueryDeviceInfo(const std::string &strSid, const std::string &strDevID, T &DevInfo);

    template<typename T>
    bool QueryDevicesOfUser(const std::string &strSid, const std::string &strUserID, const unsigned int uiBeginIndex, std::list<T> &RelationList, 
        std::list<std::string> &strDevNameList);
    
    template<typename T>
    bool QueryUsersOfDevice(const std::string &strSid, const std::string &strDevID, const unsigned int uiBeginIndex, std::list<T> &RelationList,
        std::list<std::string> &strUsrNameList);

    bool SharingDevice(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const std::string &strRelation, 
        const std::string &strBeginDate, const std::string &strEndDate);
   
    bool CancelSharedDevice(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const std::string &strRelation);

    bool AddFriends(const std::string &strSid, const std::string &strUserID, const std::string &strFriendID);

    bool DeleteFriends(const std::string &strSid, const std::string &strUserID, const std::string &strFriendID);

    bool QueryFriends(const std::string &strSid, const std::string &strUserID, const unsigned int uiBeginIndex, std::list<std::string> &FriendList);

    bool P2pInfo(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const std::string &strUserIpAddress, const unsigned int uiP2pType,
        std::string &strP2pServer, std::string &strP2pID, unsigned int &uiLease, std::string &strLicenseKey, std::string &strPushID);

    bool DeviceLogin(const std::string &strDevID, const std::string &strDevPwd, const std::string &strDevIpAddress, 
        const unsigned int &uiDevType, const unsigned int uiP2pType, const std::string &strP2pserver, const std::string &strP2pID, 
        const unsigned int uiP2pidBuildin, std::string &strSid, std::string &strValue);

    bool DeviceP2pInfo(const std::string &strSid, const std::string &strDevID, const std::string &strDevIpAddress, const unsigned int uiP2pType,
        std::string &strP2pServer, std::string &strP2pID, unsigned int &uiLease, std::string &strLicenseKey, std::string &strPushID);

    bool DeviceShakehand(const std::string &strSid, const std::string &strDevID);

    bool DeviceLogout(const std::string &strSid, const std::string &strDevID);

    template<typename T>
    bool QueryUserFile(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const unsigned int uiBeginIndex, 
        const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiBussinessType, std::list<T> &FileList);

    bool DownloadUserFile(const std::string &strSid, const std::string &strUserID, const std::string &strFileID, std::string &strFileUrl);

    bool DeleteUserFile(const std::string &strSid, const std::string &strUserID, const std::string &strFileID);

    bool AddDeviceFile(const std::string &strDevID, const std::string &strRemoteFileID, const std::string &strDownloadUrl, const std::string &strFileName,
        const std::string &strSuffixName, const unsigned long int uiFileSize, const std::string &strFileCreatedate, const std::string &strExtend, const unsigned int uiBussinessType);

    bool RetrievePwd(const std::string &strUserName, const std::string &strEmail);

    bool DeviceQueryTimeZone(const std::string &strSid, const std::string &strDevID, const std::string &strDevIpAddress, std::string &strCountrycode,
        std::string &strCountryNameEn, std::string &strCountryNameZh, std::string &strTimeZone);

    bool UserQueryAccessDomainName(const std::string &strIpAddress, const std::string &strUserName, std::string &strAccessDomainName, std::string &strLease);

    bool DeviceQueryAccessDomainName(const std::string &strIpAddress, const std::string &strDevID, std::string &strAccessDomainName, std::string &strLease);

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

    bool QueryDevUpgrade(const std::string &strCategory, const std::string &strSubcategory, const std::string &strCurrentVersion, std::string &strNewVersionValid,
        std::string &strFirmwareName, std::string &strFirmwarePath, unsigned int &uiFirmwareSize, std::string &strNewVersion, std::string &strDesc,
        std::string &strForceUpgrade, std::string &strUpdateDate);

private:
    ParamInfo m_ParamInfo;
    boost::shared_ptr<InteractiveProtoHandler> m_pInteractiveProtoHandler;

private:
    static const std::string SUCCESS_CODE;
    static const std::string SUCCESS_MSG;
    static const std::string FAILED_CODE;
    static const std::string FAILED_MSG;

};

#endif
