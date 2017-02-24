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

private:

    bool AddDeviceFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool AddUserFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

private:

    void WriteMsg(const std::map<std::string, std::string> &MsgMap, MsgWriter writer, const bool blResult = true, boost::function<void(void*)> PostFunc = NULL);

    bool PreCommonHandler(const std::string &strMsgReceived);

    bool RegisterUser(const std::string &strUserName, const std::string &strUserPwd, 
        const std::string &strType, const std::string &strExtend, const std::string &strAliasName, const std::string &strEmail, std::string &strUserID);

    bool UnRegisterUser(const std::string &strSid, const std::string &strUserID, const std::string &strUserName, const std::string &strUserPwd);
    
    template<typename T>
    bool UserLogin(const std::string &strUserName, const std::string &strUserPwd, std::list<T> &RelationList,
        std::string &strUserID, std::string &strSid);

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
    bool QueryDevicesOfUser(const std::string &strSid, const std::string &strUserID, const unsigned int uiBeginIndex, std::list<T> &RelationList);
    
    template<typename T>
    bool QueryUsersOfDevice(const std::string &strSid, const std::string &strDevID, const unsigned int uiBeginIndex, std::list<T> &RelationList);

    bool SharingDevice(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const std::string &strRelation, 
        const std::string &strBeginDate, const std::string &strEndDate);
   
    bool CancelSharedDevice(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const std::string &strRelation);

    bool AddFriends(const std::string &strSid, const std::string &strUserID, const std::string &strFriendID);

    bool DeleteFriends(const std::string &strSid, const std::string &strUserID, const std::string &strFriendID);

    bool QueryFriends(const std::string &strSid, const std::string &strUserID, const unsigned int uiBeginIndex, std::list<std::string> &FriendList);

    bool P2pInfo(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const std::string &strUserIpAddress,
        std::string &strP2pServer, std::string &strP2pID, unsigned int &uiLease);

    bool DeviceLogin(const std::string &strDevID, const std::string &strDevPwd, std::string &strSid);

    bool DeviceP2pInfo(const std::string &strSid, const std::string &strDevID, const std::string &strDevIpAddress,
        std::string &strP2pServer, std::string &strP2pID, unsigned int &uiLease);

    bool DeviceShakehand(const std::string &strSid, const std::string &strDevID);

    bool DeviceLogout(const std::string &strSid, const std::string &strDevID);

    template<typename T>
    bool QueryUserFile(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const unsigned int uiBeginIndex, 
        const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiBussinessType, std::list<T> &FileList);

    bool DownloadUserFile(const std::string &strSid, const std::string &strUserID, const std::string &strFileID, std::string &strFileUrl);

    bool DeleteUserFile(const std::string &strSid, const std::string &strUserID, const std::string &strFileID);

    bool AddDeviceFile(const std::string &strDevID, const std::string &strRemoteFileID, const std::string &strDownloadUrl, const std::string &strFileName,
        const std::string &strSuffixName, const unsigned long int uiFileSize, const std::string &strFileCreatedate, const std::string &strExtend, const unsigned int uiBussinessType);

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
