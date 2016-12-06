#include "UserManager.h"
#include <boost/scope_exit.hpp>
#include "intf.h"
#include "CommonUtility.h"
#include "ReturnCode.h"
#include "mysql_impl.h"
#include "libcacheclient.h"
#include "boost/lexical_cast.hpp"
#include "json/json.h"

UserManager::UserManager(const ParamInfo &pinfo) : m_ParamInfo(pinfo), m_DBRuner(1), m_pProtoHandler(new InteractiveProtoHandler),
m_pMysql(new MysqlImpl), m_DBCache(m_pMysql), m_pMemCl(NULL)
{
    
}


UserManager::~UserManager()
{
    m_DBRuner.Stop();

    delete m_pMysql;
    m_pMysql = NULL;

    MemcacheClient::destoy(m_pMemCl);
    m_pMemCl = NULL;
}

bool UserManager::Init()
{
    if (!m_pMysql->Init(m_ParamInfo.strDBHost.c_str(), m_ParamInfo.strDBUser.c_str(), m_ParamInfo.strDBPassword.c_str(), m_ParamInfo.strDBName.c_str()))
    {
        LOG_ERROR_RLD("Init db failed, db host is " << m_ParamInfo.strDBHost << " db user is " << m_ParamInfo.strDBUser << " db pwd is " <<
            m_ParamInfo.strDBPassword << " db name is " << m_ParamInfo.strDBName);
        return false;
    }

    m_pMemCl = MemcacheClient::create();
    if (MemcacheClient::CACHE_SUCCESS != m_pMemCl->addServer(m_ParamInfo.strMemAddress.c_str(), boost::lexical_cast<int>(m_ParamInfo.strMemPort)))
    {
        MemcacheClient::destoy(m_pMemCl);
        m_pMemCl = NULL;

        LOG_ERROR_RLD("memcached client init failed, remote ip: " << m_ParamInfo.strMemAddress << ", remote port:" << m_ParamInfo.strMemPort);
    }
    else
    {
        LOG_INFO_RLD("memcached client init succeed, remote ip: " << m_ParamInfo.strMemAddress << ", remote port:" << m_ParamInfo.strMemPort);
    }


    m_DBCache.SetSqlCB(boost::bind(&UserManager::UserInfoSqlCB, this, _1, _2, _3, _4));
    
    m_DBRuner.Run();

    LOG_INFO_RLD("UserManager init success");

    return true;
}

bool UserManager::RegisterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    InteractiveProtoHandler::RegisterUserReq_USR RegUsrReq;
    if (!m_pProtoHandler->UnSerializeReq(strMsg, RegUsrReq))
    {
        LOG_ERROR_RLD("Register user req unserialize failed, src id is " << strSrcID);
        return false;
    }

    //注册用户是无需到数据库中校验的，用户名是可以重复的。
    //这里是异步执行sql，防止阻塞，后续可以使用其他方式比如MQ来消除数据库瓶颈
    m_DBRuner.Post(boost::bind(&UserManager::InsertUserToDB, this, CreateUUID(), RegUsrReq.m_userInfo.m_strUserName,
        RegUsrReq.m_userInfo.m_strUserPassword, RegUsrReq.m_userInfo.m_uiTypeInfo, RegUsrReq.m_userInfo.m_strCreatedate,
        RegUsrReq.m_userInfo.m_uiStatus, RegUsrReq.m_userInfo.m_strExtend));

    InteractiveProtoHandler::RegisterUserRsp_USR RegUsrRsp;
    RegUsrRsp.m_MsgType = InteractiveProtoHandler::MsgType::RegisterUserRsp_USR_T;
    RegUsrRsp.m_uiMsgSeq = 1;
    RegUsrRsp.m_strSID = CreateUUID();
    RegUsrRsp.m_iRetcode = ReturnInfo::SUCCESS_CODE;
    RegUsrRsp.m_strRetMsg = ReturnInfo::SUCCESS_INFO;
    RegUsrRsp.m_strUserID = CreateUUID();
    RegUsrRsp.m_strValue = "value";

    std::string strSerializeOutPut;
    //InteractiveProtoHandler iphander;
    if (!m_pProtoHandler->SerializeReq(RegUsrRsp, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Register user rsp serialize failed.");
        return false;
    }

    writer(strSrcID, strSerializeOutPut);
    LOG_INFO_RLD("Register user rsp already send, dst id is " << strSrcID);
    return true;

}

bool UserManager::UnRegisterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    


    return true;
}

bool UserManager::LoginReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    InteractiveProtoHandler::LoginReq_USR LoginReqUsr;
    if (!m_pProtoHandler->UnSerializeReq(strMsg, LoginReqUsr))
    {
        LOG_ERROR_RLD("Login user req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (LoginReqUsr.m_userInfo.m_strUserID.empty())
    {
        LOG_ERROR_RLD("Login user req user id is empty.");
        return false;
    }

    if (LoginReqUsr.m_userInfo.m_strUserPassword.empty())
    {
        LOG_ERROR_RLD("Login user req user password is empty.");
        return false;
    }
    
    if (!ValidUser(LoginReqUsr.m_userInfo.m_strUserID, LoginReqUsr.m_userInfo.m_strUserName, 
        LoginReqUsr.m_userInfo.m_strUserPassword, LoginReqUsr.m_userInfo.m_uiTypeInfo))
    {
        return false;
    }

    //用户登录之后，对应的Session信息就保存在memcached中去，key是SessionID，value是用户信息，json格式字符串，格式如下：
    //{"logindate":"2016-11-30 15:30:20","userid":"5167F842BB3AFF4C8CC1F2557E6EFB82"}
    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));

    const std::string &strSessionID = CreateUUID();
    Json::Value jsBody;
    jsBody["logindate"] = strCurrentTime;
    jsBody["userid"] = LoginReqUsr.m_userInfo.m_strUserID;
    Json::FastWriter fastwriter;
    const std::string &strBody = fastwriter.write(jsBody); //jsBody.toStyledString();

    boost::unique_lock<boost::mutex> lock(m_MemcachedMutex);
    int iRet = 0;
    if (MemcacheClient::CACHE_SUCCESS != (iRet = m_pMemCl->set(strSessionID.c_str(), strBody.c_str(), strBody.size())))
    {
        LOG_ERROR_RLD("Login user set session id to cache failed, return code is " << iRet << " and user id is " << LoginReqUsr.m_userInfo.m_strUserID);
        return false;
    }

    //InteractiveProtoHandler::LoginRsp_USR LoginRspUsr;
    //LoginRspUsr.


    return true;
}

void UserManager::InsertUserToDB(const std::string &strUserID, const std::string &strUserName, const std::string &strUserPwd, 
    const int iTypeInfo, const std::string &strCreateDate, const int iStatus, const std::string &strExtend)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "insert into t_user_info("
        "id,userid,username, userpassword, typeinfo, createdate, status, extend) values(uuid(),"        
        "'%s','%s','%s','%d','%s', '%d','%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), strUserName.c_str(), strUserPwd.c_str(), iTypeInfo, strCreateDate.c_str(), iStatus, strExtend.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Insert t_user_info sql exec failed, sql is " << sql);        
    }
}

bool UserManager::QueryRelationByUserID(const std::string &strUserID, std::list<InteractiveProtoHandler::Device> &DevList)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "select dev.deviceid, dev.devicename, dev.devicepassword, dev.typeinfo, dev.createdate, dev.status, dev.innerinfo, dev.extend"
        "from t_device_info dev, t_user_device_relation rel"
        "where dev.deviceid = rel.deviceid and rel.userid = '%s' and rel.status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql), boost::bind(&UserManager::DevInfoRelationSqlCB, this, _1, _2, _3, &DevList)))
    {
        LOG_ERROR_RLD("Query relation failed and user id is " << strUserID);
        return false;
    }

    if (DevList.empty())
    {
        LOG_INFO_RLD("QueryRelationByUserID result is empty and user id is " << strUserID);
        return true;
    }

    memset(sql, 0, sizeof(sql));
    const char *sqlft = "select rel.userid from t_user_device_relation rel"
        "where rel.deviceid = '%s' and rel.relation = '%d'";
    
    
    auto itBegin = DevList.begin();
    auto itEnd = DevList.end();
    while (itBegin != itEnd)
    {
        //`relation` int(11) NOT NULL DEFAULT '0', #关系包括，拥有0、被分享1、分享中2、转移3，目前只用0、1、2
        {
            snprintf(sql, sizeof(sql), sqlft, itBegin->m_strDevID.c_str(), 0);
            std::list<std::string> UserIDList;
            if (!m_pMysql->QueryExec(std::string(sql), boost::bind(&UserManager::UserInfoRelationSqlCB, this, _1, _2, _3, &UserIDList)))
            {
                LOG_ERROR_RLD("Query device relation failed and user id is " << strUserID);
                return false;
            }

            if (!UserIDList.empty())
            {
                itBegin->m_strOwnerUserID = UserIDList.front();
            }
        }
        
        memset(sql, 0, sizeof(sql));

        {
            snprintf(sql, sizeof(sql), sqlft, itBegin->m_strDevID.c_str(), 2);
            std::list<std::string> UserIDList;
            if (!m_pMysql->QueryExec(std::string(sql), boost::bind(&UserManager::UserInfoRelationSqlCB, this, _1, _2, _3, &UserIDList)))
            {
                LOG_ERROR_RLD("Query device relation failed and user id is " << strUserID);
                return false;
            }

            if (!UserIDList.empty())
            {
                itBegin->m_sharingUserIDList.swap(UserIDList);
            }
        }

        memset(sql, 0, sizeof(sql));

        {
            snprintf(sql, sizeof(sql), sqlft, itBegin->m_strDevID.c_str(), 1);
            std::list<std::string> UserIDList;
            if (!m_pMysql->QueryExec(std::string(sql), boost::bind(&UserManager::UserInfoRelationSqlCB, this, _1, _2, _3, &UserIDList)))
            {
                LOG_ERROR_RLD("Query device relation failed and user id is " << strUserID);
                return false;
            }

            if (!UserIDList.empty())
            {
                itBegin->m_sharedUserIDList.swap(UserIDList);
            }
        }
        
        ++itBegin;
    }


    return true;
}

bool UserManager::ValidUser(const std::string &strUserID, const std::string &strUserName, const std::string &strUserPwd, const int iTypeInfo)
{
    //Valid user id
    char sql[1024] = { 0 };
    const char* sqlfmt = "select userid,username, userpassword, typeinfo, createdate, status, extend where userid = "
        "'%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str());

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList))
    {
        LOG_ERROR_RLD("Valid user query sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("Valid user info not found, sql is " << sql);
        return false;
    }

    if (strUserPwd.empty())
    {
        LOG_INFO_RLD("Valid user success and sql is " << sql);
        return true;
    }
    
    std::string strUserPwdInDB;
    auto itBegin = ResultList.begin();
    auto itEnd = ResultList.end();
    while (itBegin != itEnd)
    {
        auto Result = boost::any_cast<ValueInDB>(*itBegin);
        if (Result.strType == "userpassword")
        {
            strUserPwdInDB = Result.strValue;
            break;
        }

        ++itBegin;
    }
    
    if (strUserPwdInDB != strUserPwd)
    {

        LOG_ERROR_RLD("Valid user failed because pwd not correct and sql is " << sql);
        return false;
    }

    LOG_INFO_RLD("Valid user success and sql is " << sql);

    return true;
}

void UserManager::UserInfoSqlCB(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
{
    ValueInDB value;
    value.strValue = strColumn;

    switch (uiColumnNum)
    {
    case 0:
        value.strType = "userid";
        break;
    case 1:
        value.strType = "username";
        break;
    case 2:
        value.strType = "userpassword";
        break;
    case 3:
        value.strType = "typeinfo";
        break;
    case 4:
        value.strType = "createdate";
        break;
    case 5:
        value.strType = "status";
        break;
    case 6:
        value.strType = "extend";
        break;
    default:
        LOG_ERROR_RLD("UserInfoSqlCB error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
        break;
    }

    Result = value;

}

void UserManager::DevInfoRelationSqlCB(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, 
    std::list<InteractiveProtoHandler::Device> *pDevList)
{
    //select dev.deviceid, dev.devicename, dev.devicepassword, dev.typeinfo, dev.createdate, dev.status, dev.innerinfo, dev.extend        
    if (pDevList->empty() || (pDevList->size() < (1 + uiRowNum)))
    {
        InteractiveProtoHandler::Device devInfoTmp;
        pDevList->push_back(std::move(devInfoTmp));
    }
    
    InteractiveProtoHandler::Device &devInfo = pDevList->back();

    switch (uiColumnNum)
    {
    case 0:
        devInfo.m_strDevID = strColumn;
        break;
    case 1:
        devInfo.m_strDevName = strColumn;
        break;
    case 2:
        devInfo.m_strDevPassword = strColumn;
        break;
    case 3:
        devInfo.m_uiTypeInfo = boost::lexical_cast<unsigned int>(strColumn);
        break;
    case 4:
        devInfo.m_strCreatedate = strColumn;
        break;
    case 5:
        devInfo.m_uiStatus = boost::lexical_cast<unsigned int>(strColumn);
    case 6:
        devInfo.m_strInnerinfo = strColumn;
    case 7:
        devInfo.m_strExtend = strColumn;
    default:
        LOG_ERROR_RLD("DevInfoSqlCB error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
        break;
    }
}

void UserManager::UserInfoRelationSqlCB(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, 
    std::list<std::string> *pUserIDList)
{
    if (pUserIDList->empty() || (pUserIDList->size() < (1 + uiRowNum)))
    {
        std::string strUserIDTmp;
        pUserIDList->push_back(std::move(strUserIDTmp));
    }

    pUserIDList->back() = strColumn;

}

