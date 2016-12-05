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

class MysqlImpl;

namespace roiland
{
    class MemcacheClient;
}

using roiland::MemcacheClient;

/************************************************************************/
/* �û������࣬�ṩ�˹����û�������صķ����ӿں�ʵ�֡�
 * �����ṩ�ķ�����ע�ᵽ��ControlCenter���С�
 * Author������
 * Date��2016-12-1*/
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

    

private:
    void InsertUserToDB(const std::string &strUserID, const std::string &strUserName, const std::string &strUserPwd, const int iTypeInfo,
        const std::string &strCreateDate, const int iStatus, const std::string &strExtend);

    bool ValidUser(const std::string &strUserID, const std::string &strUserName = "", const std::string &strUserPwd = "", const int iTypeInfo = 0);

    void SqlCB (const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result);


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
    
};



#endif