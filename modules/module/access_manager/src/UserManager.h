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

class MysqlImpl;

class MemcacheClient;

/************************************************************************/
/* 用户管理类，提供了管理用户所有相关的方法接口和实现。
 * 该类提供的方法会注册到到ControlCenter类中。
 * Author：尹宾
 * Date：2016-12-1*/
/************************************************************************/
class UserManager : public boost::noncopyable
{
public:
    static const int ONLINE_STATUS = 0;
    static const int OFFLINE_STATUS = 1;
    static const int DELETE_STATUS = 2;
    static const int FROZEN_STATUS = 3;

    typedef struct _ParamInfo
    {
        std::string strDBHost;
        std::string strDBPort;
        std::string strDBUser;
        std::string strDBPassword;
        std::string strDBName;
        std::string strMemAddress;
        std::string strMemPort;

    } ParamInfo;
    
    inline void SetParamInfo(const ParamInfo &pinfo)
    {
        m_ParamInfo = pinfo;
    };

    UserManager(const ParamInfo &pinfo);
    ~UserManager();
    bool Init();

    bool RegisterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool UnRegisterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool LoginReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool LogoutReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

    bool Shakehand(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer);

private:
    void InsertUserToDB(const std::string &strUserID, const std::string &strUserName, const std::string &strUserPwd, const int iTypeInfo,
        const std::string &strCreateDate, const int iStatus, const std::string &strExtend);

    void UpdateUserToDB(const std::string &strUserID, const int iStatus);

    bool QueryRelationByUserID(const std::string &strUserID, std::list<InteractiveProtoHandler::Device> &DevList);

    bool QueryRelationByDevID(const std::string &strDevID, const int iRelation, std::list<std::string> &UserIDList);

    bool ValidUser(const std::string &strUserID, const std::string &strUserName = "", const std::string &strUserPwd = "", const int iTypeInfo = 0);

    void UserInfoSqlCB(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result);

    void DevInfoRelationSqlCB(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, 
        std::list<InteractiveProtoHandler::Device> *pDevList);

    void UserInfoRelationSqlCB(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn,
        std::list<std::string> *pUserIDList);



private:
    ParamInfo m_ParamInfo;
    boost::mutex m_UserInfoMutex;
    std::unordered_map<std::string, boost::shared_ptr<InteractiveProtoHandler::User> > m_UserInfoMap; //Key is session id

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
    MemcacheClient *m_pMemCl;
    
    boost::atomic_uint64_t m_uiMsgSeq;


};



#endif