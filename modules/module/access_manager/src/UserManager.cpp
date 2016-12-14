#include "UserManager.h"
#include <boost/scope_exit.hpp>
#include "CommonUtility.h"
#include "ReturnCode.h"
#include "mysql_impl.h"
#include "boost/lexical_cast.hpp"
#include "json/json.h"

UserManager::UserManager(const ParamInfo &pinfo) : m_ParamInfo(pinfo), m_DBRuner(1), m_pProtoHandler(new InteractiveProtoHandler),
m_pMysql(new MysqlImpl), m_DBCache(m_pMysql), m_uiMsgSeq(0)
{
    
}


UserManager::~UserManager()
{
    m_SessionMgr.Stop();

    m_DBRuner.Stop();

    delete m_pMysql;
    m_pMysql = NULL;

}

bool UserManager::Init()
{
    if (!m_pMysql->Init(m_ParamInfo.m_strDBHost.c_str(), m_ParamInfo.m_strDBUser.c_str(), m_ParamInfo.m_strDBPassword.c_str(), m_ParamInfo.m_strDBName.c_str()))
    {
        LOG_ERROR_RLD("Init db failed, db host is " << m_ParamInfo.m_strDBHost << " db user is " << m_ParamInfo.m_strDBUser << " db pwd is " <<
            m_ParamInfo.m_strDBPassword << " db name is " << m_ParamInfo.m_strDBName);
        return false;
    }

    m_SessionMgr.SetMemCacheAddRess(m_ParamInfo.m_strMemAddress, m_ParamInfo.m_strMemPort);

    if (!m_SessionMgr.Init())
    {
        LOG_ERROR_RLD("Session mgr init failed.");
        return false;
    }


    m_DBCache.SetSqlCB(boost::bind(&UserManager::UserInfoSqlCB, this, _1, _2, _3, _4));
    
    m_DBRuner.Run();

    m_SessionMgr.Run();

    LOG_INFO_RLD("UserManager init success");

    return true;
}

bool UserManager::RegisterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    const std::string &strUserID = CreateUUID();
    BOOST_SCOPE_EXIT(&blResult, this_, &strUserID, &writer, &strSrcID)
    {
        InteractiveProtoHandler::RegisterUserRsp_USR RegUsrRsp;
        RegUsrRsp.m_MsgType = InteractiveProtoHandler::MsgType::RegisterUserRsp_USR_T;
        RegUsrRsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        RegUsrRsp.m_strSID = CreateUUID();
        RegUsrRsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        RegUsrRsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        RegUsrRsp.m_strUserID = strUserID;
        RegUsrRsp.m_strValue = "value";

        std::string strSerializeOutPut;
        //InteractiveProtoHandler iphander;
        if (!this_->m_pProtoHandler->SerializeReq(RegUsrRsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Register user rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Register user rsp already send, dst id is " << strSrcID << " and user id is " << strUserID << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    InteractiveProtoHandler::RegisterUserReq_USR RegUsrReq;
    if (!m_pProtoHandler->UnSerializeReq(strMsg, RegUsrReq))
    {
        LOG_ERROR_RLD("Register user req unserialize failed, src id is " << strSrcID);
        return false;
    }

    //注册用户是无需到数据库中校验的，用户名是可以重复的。
    //这里是异步执行sql，防止阻塞，后续可以使用其他方式比如MQ来消除数据库瓶颈
    
    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));

    InteractiveProtoHandler::User UsrInfo;
    UsrInfo.m_strUserID = strUserID;
    UsrInfo.m_strUserName = RegUsrReq.m_userInfo.m_strUserName;
    UsrInfo.m_strUserPassword = RegUsrReq.m_userInfo.m_strUserPassword;
    UsrInfo.m_uiTypeInfo = RegUsrReq.m_userInfo.m_uiTypeInfo;
    UsrInfo.m_strCreatedate = strCurrentTime;
    UsrInfo.m_uiStatus = 0;
    UsrInfo.m_strExtend = RegUsrReq.m_userInfo.m_strExtend;
    
    m_DBRuner.Post(boost::bind(&UserManager::InsertUserToDB, this, UsrInfo));


    blResult = true;
    
    
    return blResult;
}

bool UserManager::UnRegisterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    //注销用户之前，用户必须已经登录系统
    //注销用户之后，系统默认将该用户登出系统
    bool blResult = false;

    InteractiveProtoHandler::UnRegisterUserReq_USR UnRegUsrReq;

    BOOST_SCOPE_EXIT(&blResult, this_, &UnRegUsrReq, &writer, &strSrcID)
    {
        InteractiveProtoHandler::UnRegisterUserRsp_USR UnRegRspUsr;
        UnRegRspUsr.m_MsgType = InteractiveProtoHandler::MsgType::UnRegisterUserRsp_USR_T;
        UnRegRspUsr.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        UnRegRspUsr.m_strSID = UnRegUsrReq.m_strSID;
        UnRegRspUsr.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        UnRegRspUsr.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        UnRegRspUsr.m_strValue = "value";
        UnRegRspUsr.m_strUserID = UnRegUsrReq.m_userInfo.m_strUserID;

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(UnRegRspUsr, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Unregister user rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("User unregister rsp already send, dst id is " << strSrcID << " and user id is " << UnRegRspUsr.m_strUserID << " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, UnRegUsrReq))
    {
        LOG_ERROR_RLD("Unregister user req unserialize failed, src id is " << strSrcID);
        return false;
    }

    LOG_INFO_RLD("Unregister user id is " << UnRegUsrReq.m_userInfo.m_strUserID << " session id is " << UnRegUsrReq.m_strSID);

    if (UnRegUsrReq.m_userInfo.m_strUserID.empty())
    {
        LOG_ERROR_RLD("Unregister user id is empty, src id is " << strSrcID);
        return false;
    }

    m_SessionMgr.Remove(UnRegUsrReq.m_strSID);

    //广播消息表示用户注销


    //异步更新数据库内容
    m_DBRuner.Post(boost::bind(&UserManager::UpdateUserToDB, this, UnRegUsrReq.m_userInfo.m_strUserID, 1));

    blResult = true;

        
    return blResult;
}

bool UserManager::LoginReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::LoginReq_USR LoginReqUsr;

    const std::string &strSessionID = CreateUUID();
    std::list<InteractiveProtoHandler::Device> DeviceList;
    BOOST_SCOPE_EXIT(&blResult, this_, &DeviceList, &strSessionID, &LoginReqUsr, &writer, &strSrcID)
    {
        InteractiveProtoHandler::LoginRsp_USR LoginRspUsr;

        LoginRspUsr.m_MsgType = InteractiveProtoHandler::MsgType::LoginRsp_USR_T;
        LoginRspUsr.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        LoginRspUsr.m_strSID = strSessionID;
        LoginRspUsr.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        LoginRspUsr.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        LoginRspUsr.m_strValue = "value";

        if (blResult)
        {
            LoginRspUsr.m_devInfoList.swap(DeviceList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(LoginRspUsr, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Login user rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("User login rsp already send, dst id is " << strSrcID << " and user id is " << LoginReqUsr.m_userInfo.m_strUserID << 
            " and user password is " << LoginReqUsr.m_userInfo.m_strUserPassword <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    
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

    
    Json::Value jsBody;
    jsBody["logindate"] = strCurrentTime;
    jsBody["userid"] = LoginReqUsr.m_userInfo.m_strUserID;
    Json::FastWriter fastwriter;
    const std::string &strBody = fastwriter.write(jsBody); //jsBody.toStyledString();
     
    m_SessionMgr.Create(strSessionID, strBody, boost::lexical_cast<unsigned int>(m_ParamInfo.m_strSessionTimeoutCountThreshold), 
        boost::bind(&UserManager::SessionTimeoutProcessCB, this, _1));
        
    if (!QueryRelationByUserID(LoginReqUsr.m_userInfo.m_strUserID, DeviceList))
    {
        LOG_ERROR_RLD("Query device info failed and user id is " << LoginReqUsr.m_userInfo.m_strUserID);
        return false;
    }

    //广播消息表示用户登录


    blResult = true;
    
    
    return blResult;
}

bool UserManager::LogoutReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::LogoutReq_USR LogoutReqUsr;

    BOOST_SCOPE_EXIT(&blResult, this_, &LogoutReqUsr, &writer, &strSrcID)
    {
        InteractiveProtoHandler::LogoutRsp_USR LogoutRspUsr;
        LogoutRspUsr.m_MsgType = InteractiveProtoHandler::MsgType::LogoutRsp_USR_T;
        LogoutRspUsr.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        LogoutRspUsr.m_strSID = LogoutReqUsr.m_strSID;
        LogoutRspUsr.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        LogoutRspUsr.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        LogoutRspUsr.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(LogoutRspUsr, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Logout user rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("User logout rsp already send, dst id is " << strSrcID << " and user id is " << LogoutReqUsr.m_userInfo.m_strUserID << 
            " and session id is " << LogoutReqUsr.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, LogoutReqUsr))
    {
        LOG_ERROR_RLD("Logout user req unserialize failed, src id is " << strSrcID);
        return false;
    }

    //用戶登出，简化操作，不需要校验用户ID和用户密码
    if (LogoutReqUsr.m_strSID.empty())
    {
        LOG_ERROR_RLD("Logout req session id is empty and src id is " << strSrcID);
        return false;
    }

    //检查用户SessionID是否存在，表示是否用户已经登录
    m_SessionMgr.Remove(LogoutReqUsr.m_strSID); //移除会话


    //广播消息表示用户登出

    
    blResult = true;

    
    
    return blResult;
}

bool UserManager::ShakehandReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    //这里根据SessionID值来进行握手，SID值以memcached中的数据为参考源
    
    bool blResult = false;

    InteractiveProtoHandler::ShakehandReq_USR ShakehandReqUsr;

    BOOST_SCOPE_EXIT(&blResult, this_, &ShakehandReqUsr, &writer, &strSrcID)
    {
        InteractiveProtoHandler::ShakehandRsp_USR ShakehandRspUsr;
        ShakehandRspUsr.m_MsgType = InteractiveProtoHandler::MsgType::ShakehandRsp_USR_T;
        ShakehandRspUsr.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        ShakehandRspUsr.m_strSID = ShakehandReqUsr.m_strSID;
        ShakehandRspUsr.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        ShakehandRspUsr.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        ShakehandRspUsr.m_strValue = "value";
        
        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(ShakehandRspUsr, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Shakehand user rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("User shakehand rsp already send, dst id is " << strSrcID << " and user id is " << ShakehandReqUsr.m_strUserID << 
            " and session id is " << ShakehandReqUsr.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, ShakehandReqUsr))
    {
        LOG_ERROR_RLD("Shakehand req unserialize failed, src id is " << strSrcID);
        return false;
    }

    m_SessionMgr.Reset(ShakehandReqUsr.m_strSID);
    
    LOG_INFO_RLD("Shake hand user received and user id is " << ShakehandReqUsr.m_strUserID << " and session id is " << ShakehandReqUsr.m_strSID);

    blResult = true;

    

    return blResult;

}

bool UserManager::AddDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;
    InteractiveProtoHandler::AddDevReq_USR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID)
    {
        InteractiveProtoHandler::AddDevRsp_USR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::ShakehandRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add device rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("User add device rsp already send, dst id is " << strSrcID << " and user id is " << req.m_strUserID <<
            " and session id is " << req.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Add device req unserialize failed, src id is " << strSrcID);
        return false;
    }

    //这里是异步执行sql，防止阻塞，后续可以使用其他方式比如MQ来消除数据库瓶颈
    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));

    InteractiveProtoHandler::Device DevInfo;
    DevInfo.m_strDevID = req.m_devInfo.m_strDevID;
    DevInfo.m_strDevName = req.m_devInfo.m_strDevName;
    DevInfo.m_strDevPassword = req.m_devInfo.m_strDevPassword;
    DevInfo.m_uiTypeInfo = req.m_devInfo.m_uiTypeInfo;
    DevInfo.m_strCreatedate = strCurrentTime;
    DevInfo.m_uiStatus = 0;
    DevInfo.m_strInnerinfo = req.m_devInfo.m_strInnerinfo;
    DevInfo.m_strExtend = req.m_devInfo.m_strExtend;

    m_DBRuner.Post(boost::bind(&UserManager::InserDeviceToDB, this, DevInfo));

    RelationOfUsrAndDev relation;
    relation.m_iRelation = RELATION_OF_OWNER;
    relation.m_iStatus = 0;
    relation.m_strBeginDate = strCurrentTime;
    relation.m_strEndDate = ""; //结束时间为空，表示无限期
    relation.m_strCreateDate = strCurrentTime;
    relation.m_strDevID = req.m_devInfo.m_strDevID;
    relation.m_strExtend = req.m_devInfo.m_strExtend;
    relation.m_strOwnerID = req.m_strUserID;
    relation.m_strUsrID = req.m_strUserID;

    m_DBRuner.Post(boost::bind(&UserManager::InsertRelationToDB, this, relation));

    m_SessionMgr.Reset(req.m_strSID); //普通命令的处理也需要重置Session

    blResult = true;
    
    return true;
}

void UserManager::InsertUserToDB(const InteractiveProtoHandler::User &UsrInfo)
{
    
    char sql[1024] = { 0 };
    const char* sqlfmt = "insert into t_user_info("
        "id,userid,username, userpassword, typeinfo, createdate, status, extend) values(uuid(),"        
        "'%s','%s','%s','%d','%s', '%d','%s')";
    snprintf(sql, sizeof(sql), sqlfmt, UsrInfo.m_strUserID.c_str(), UsrInfo.m_strUserName.c_str(), UsrInfo.m_strUserPassword.c_str(), UsrInfo.m_uiTypeInfo, 
        UsrInfo.m_strCreatedate.c_str(), UsrInfo.m_uiStatus, UsrInfo.m_strExtend.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Insert t_user_info sql exec failed, sql is " << sql);        
    }
}

void UserManager::UpdateUserToDB(const std::string &strUserID, const int iStatus)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "update t_user_info set status = '%d' where userid = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, iStatus, strUserID.c_str());
    
    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Update t_user_info sql exec failed, sql is " << sql);
    }

}

bool UserManager::QueryRelationByUserID(const std::string &strUserID, std::list<InteractiveProtoHandler::Device> &DevList)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "select dev.deviceid, dev.devicename, dev.devicepassword, dev.typeinfo, dev.createdate, dev.status, dev.innerinfo, dev.extend"
        " from t_device_info dev, t_user_device_relation rel "
        " where dev.deviceid = rel.deviceid and rel.userid = '%s' and rel.status = 0";
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
    
    auto itBegin = DevList.begin();
    auto itEnd = DevList.end();
    while (itBegin != itEnd)
    {
        //`relation` int(11) NOT NULL DEFAULT '0', 关系包括，拥有0、被分享1、分享中2、转移3，目前只用0、1、2
        {
            std::list<std::string> UserIDList;
            if (!QueryRelationByDevID(itBegin->m_strDevID, 0, UserIDList))
            {
                LOG_ERROR_RLD("Query relation failed.");
                return false;
            }

            if (!UserIDList.empty())
            {
                itBegin->m_strOwnerUserID = UserIDList.front();
            }            

        }
        
        {
            std::list<std::string> UserIDList;
            if (!QueryRelationByDevID(itBegin->m_strDevID, 2, UserIDList))
            {
                LOG_ERROR_RLD("Query relation failed.");
                return false;
            }

            if (!UserIDList.empty())
            {
                itBegin->m_sharingUserIDList.swap(UserIDList);
            }
        }
        
        {
            std::list<std::string> UserIDList;
            if (!QueryRelationByDevID(itBegin->m_strDevID, 1, UserIDList))
            {
                LOG_ERROR_RLD("Query relation failed.");
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

bool UserManager::QueryRelationByDevID(const std::string &strDevID, const int iRelation, std::list<std::string> &UserIDList)
{
    char sql[1024] = { 0 };
    memset(sql, 0, sizeof(sql));
    const char *sqlft = "select rel.userid from t_user_device_relation rel"
        "where rel.deviceid = '%s' and rel.relation = '%d'";

    snprintf(sql, sizeof(sql), sqlft, strDevID.c_str(), iRelation);

    if (!m_pMysql->QueryExec(std::string(sql), boost::bind(&UserManager::UserInfoRelationSqlCB, this, _1, _2, _3, &UserIDList)))
    {
        LOG_ERROR_RLD("Query device relation failed and device id is " << strDevID << " and relation is " << iRelation);
        return false;
    }

    return true;
}

bool UserManager::ValidUser(const std::string &strUserID, const std::string &strUserName, const std::string &strUserPwd, const int iTypeInfo)
{
    //Valid user id
    char sql[1024] = { 0 };
    const char* sqlfmt = "select userid,username, userpassword, typeinfo, createdate, status, extend from t_user_info where userid = '%s'";
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

    LOG_INFO_RLD("Valid user success and user id is " << strUserName << " and user password is " << strUserPwd);

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
        break;
    case 6:
        devInfo.m_strInnerinfo = strColumn;
        break;
    case 7:
        devInfo.m_strExtend = strColumn;
        break;
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

void UserManager::SessionTimeoutProcessCB(const std::string &strSessionID)
{
        
    //广播消息表示用户会话超时


    LOG_INFO_RLD("Session timeout and session id is " << strSessionID);
}

void UserManager::InserDeviceToDB(const InteractiveProtoHandler::Device &DevInfo)
{
    //这里考虑到设备内部信息可能不一定是可打印字符，为了后续日志打印和维护方便，这里就将其内容文本化之后再存储到数据库中
    const std::string &strInner = DevInfo.m_strInnerinfo.empty() ? 
        DevInfo.m_strInnerinfo : Encode64((const unsigned char *)DevInfo.m_strInnerinfo.c_str(), DevInfo.m_strInnerinfo.size());

    char sql[1024] = { 0 };
    const char* sqlfmt = "insert into t_device_info("
        "id,deviceid,devicename, devicepassword, typeinfo, createdate, status, innerinfo, extend) values(uuid(),"
        "'%s','%s','%s','%d','%s', '%d','%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, DevInfo.m_strDevID.c_str(), DevInfo.m_strDevName.c_str(), DevInfo.m_strDevPassword.c_str(), DevInfo.m_uiTypeInfo,
        DevInfo.m_strCreatedate.c_str(), DevInfo.m_uiStatus, strInner.c_str(), DevInfo.m_strExtend.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Insert t_device_info sql exec failed, sql is " << sql);
    }
}

void UserManager::InsertRelationToDB(const RelationOfUsrAndDev &relation)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "insert into t_user_device_relation("
        "id, userid, deviceid, ownerid, relation, begindate, enddate, createdate, status, extend) values(uuid(),"
        "'%s','%s','%s','%d','%s', '%d','%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, relation.m_strUsrID.c_str(), relation.m_strDevID.c_str(), relation.m_strOwnerID.c_str(), relation.m_iRelation,
        relation.m_strBeginDate.c_str(), relation.m_strEndDate.c_str(), relation.m_strCreateDate.c_str(), relation.m_iStatus, relation.m_strExtend.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Insert t_user_device_relation sql exec failed, sql is " << sql);
    }

}

