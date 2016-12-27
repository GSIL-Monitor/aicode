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

class MysqlImpl;

/************************************************************************/
/* 用户管理类，提供了管理用户所有相关的方法接口和实现。
 * 该类提供的方法会注册到到ControlCenter类中。
 * Author：尹宾
 * Date：2016-12-1*/
/************************************************************************/
class UserManager : public boost::noncopyable
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

    } ParamInfo;
    
    inline void SetParamInfo(const ParamInfo &pinfo)
    {
        m_ParamInfo = pinfo;
    };

    UserManager(const ParamInfo &pinfo);
    ~UserManager();
    bool Init();

    bool PreCommonHandler(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool RegisterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool UnRegisterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool QueryUsrInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool LoginReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

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

private:
    void InsertUserToDB(const InteractiveProtoHandler::User &UsrInfo);

    void UpdateUserToDB(const std::string &strUserID, const int iStatus);

    bool QueryRelationExist(const std::string &strUserID, const std::string &strDevID, const int iRelation, bool &blExist, const bool IsNeedCache = true);

    bool QueryRelationByUserID(const std::string &strUserID, std::list<InteractiveProtoHandler::Relation> &RelationList, 
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    bool QueryRelationByDevID(const std::string &strDevID, std::list<InteractiveProtoHandler::Relation> &RelationList,
        const unsigned int uiBeginIndex = 0, const unsigned int uiPageSize = 10);

    bool ValidUser(const std::string &strUserID, const std::string &strUserName = "", const std::string &strUserPwd = "", const int iTypeInfo = 0);

    void UserInfoSqlCB(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result);

    void DevInfoRelationSqlCB(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, 
        std::list<InteractiveProtoHandler::Relation> *pRelationList);

    void SessionTimeoutProcessCB(const std::string &strSessionID);

    void InsertDeviceToDB(const InteractiveProtoHandler::Device &DevInfo);

    void InsertRelationToDB(const RelationOfUsrAndDev &relation);

    void RemoveRelationToDB(const RelationOfUsrAndDev &relation);

    void DelDeviceToDB(const std::list<std::string> &strDevIDList, const int iStatus);

    void ModDeviceToDB(const InteractiveProtoHandler::Device &DevInfo);

    void SharingRelationToDB(const RelationOfUsrAndDev &relation);

    void CancelSharedRelationToDB(const RelationOfUsrAndDev &relation);

    bool QueryUserInfoToDB(const std::string &strUserID, InteractiveProtoHandler::User &usr, const bool IsNeedCache = true);

    bool QueryDevInfoToDB(const std::string &strDevID, InteractiveProtoHandler::Device &dev, const bool IsNeedCache = true);

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

    boost::mutex m_MemcachedMutex;
    
    boost::atomic_uint64_t m_uiMsgSeq;
    
    SessionMgr m_SessionMgr;
};



#endif