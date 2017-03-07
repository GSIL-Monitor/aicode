#include "AccessManager.h"
#include <boost/scope_exit.hpp>
#include "CommonUtility.h"
#include "ReturnCode.h"
#include "mysql_impl.h"
#include "boost/lexical_cast.hpp"
#include "json/json.h"
#include "UserLoginLTUserSite.h"
#include "P2PServerManager_SY.h"

const std::string AccessManager::MAX_DATE = "2199-01-01 00:00:00";

const std::string AccessManager::GET_IPINFO_SITE = "http://ip.taobao.com/service/getIpInfo.php";

AccessManager::AccessManager(const ParamInfo &pinfo) : m_ParamInfo(pinfo), m_DBRuner(1), m_pProtoHandler(new InteractiveProtoHandler),
m_pMysql(new MysqlImpl), m_DBCache(m_pMysql), m_uiMsgSeq(0)
{
    
}


AccessManager::~AccessManager()
{
    m_SessionMgr.Stop();

    m_DBRuner.Stop();

    delete m_pMysql;
    m_pMysql = NULL;

}

bool AccessManager::Init()
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

    if (!m_pMysql->QueryExec(std::string("SET NAMES utf8")))
    {
        LOG_ERROR_RLD("Init charset to utf8 failed, sql is SET NAMES utf8");
        return false;
    }

    if (!InitDefaultAccessDomainName())
    {
        LOG_ERROR_RLD("Init default access domain name failed.");
        return false;
    }

    m_DBCache.SetSqlCB(boost::bind(&AccessManager::UserInfoSqlCB, this, _1, _2, _3, _4));
    
    m_DBRuner.Run();

    m_SessionMgr.Run();

    LOG_INFO_RLD("UserManager init success");

    return true;
}

bool AccessManager::PreCommonHandler(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::Req req;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID)
    {
        if (blResult)
        {
            LOG_INFO_RLD("PreCommonHandler success, dst id is " << strSrcID <<
                " and session id is " << req.m_strSID);
            return;
        }
        InteractiveProtoHandler::MsgPreHandlerRsp_USR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::MsgPreHandlerRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";
        
        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("PreCommonHandler rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_ERROR_RLD("PreCommonHandler failed and rsp already send, dst id is " << strSrcID <<
            " and session id is " << req.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReqBase(strMsg, req))
    {
        LOG_ERROR_RLD("PreCommonHandler req unserialize failed, src id is " << strSrcID);
        return false;
    }
    
    if (InteractiveProtoHandler::MsgType::LoginReq_USR_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::RegisterUserReq_USR_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::RegisterUserRsp_USR_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::LoginReq_DEV_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::AddFileReq_DEV_T == req.m_MsgType || 
        InteractiveProtoHandler::MsgType::RetrievePwdReq_USR_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::QueryAccessDomainNameReq_DEV_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::QueryAccessDomainNameReq_USR_T == req.m_MsgType)
    {
        LOG_INFO_RLD("PreCommonHandler return true because no need to check and msg type is " << req.m_MsgType);
        blResult = true;
        return blResult;
    }

    //通用预处理操作
    if (!m_SessionMgr.Exist(req.m_strSID))
    {
        LOG_ERROR_RLD("PreCommonHandler return false because session is invalid and msg type is " << req.m_MsgType
            << " and session id is " << req.m_strSID);
        blResult = false;
        return blResult;
    }

    //普通命令（非握手命令）重置Session
    if (InteractiveProtoHandler::MsgType::ShakehandReq_USR_T != req.m_MsgType)
    {
        m_SessionMgr.Reset(req.m_strSID);
    }

    blResult = true;

    return blResult;
}

bool AccessManager::RegisterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    std::string strUserID;
    BOOST_SCOPE_EXIT(&blResult, this_, &strUserID, &writer, &strSrcID)
    {
        if (NULL == writer)
        {
            return;
        }

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
    
    //User info already exists
    if (ValidUser(RegUsrReq.m_userInfo.m_strUserID, RegUsrReq.m_userInfo.m_strUserName,
        "", RegUsrReq.m_userInfo.m_uiTypeInfo, true))
    {
        LOG_ERROR_RLD("Register user failed because user already exist and user name is " << RegUsrReq.m_userInfo.m_strUserName << 
            " and user id is " << RegUsrReq.m_userInfo.m_strUserID << " and user pwd is " << RegUsrReq.m_userInfo.m_strUserPassword);
        return false;
    }

    //注册用户是无需到数据库中校验的，用户名是可以重复的。
    //这里是异步执行sql，防止阻塞，后续可以使用其他方式比如MQ来消除数据库瓶颈
    
    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));

    InteractiveProtoHandler::User UsrInfo;
    UsrInfo.m_strUserID = strUserID = RegUsrReq.m_userInfo.m_strUserID.empty() ? CreateUUID() : RegUsrReq.m_userInfo.m_strUserID;
    UsrInfo.m_strUserName = RegUsrReq.m_userInfo.m_strUserName;
    UsrInfo.m_strUserPassword = RegUsrReq.m_userInfo.m_strUserPassword;
    UsrInfo.m_uiTypeInfo = RegUsrReq.m_userInfo.m_uiTypeInfo;
    UsrInfo.m_strCreatedate = strCurrentTime;
    UsrInfo.m_uiStatus = NORMAL_STATUS;
    UsrInfo.m_strExtend = RegUsrReq.m_userInfo.m_strExtend;
    UsrInfo.m_strAliasName = RegUsrReq.m_userInfo.m_strAliasName;
    UsrInfo.m_strEmail = RegUsrReq.m_userInfo.m_strEmail;
    
    m_DBRuner.Post(boost::bind(&AccessManager::InsertUserToDB, this, UsrInfo));


    blResult = true;
    
    
    return blResult;
}

bool AccessManager::UnRegisterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
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
    m_DBRuner.Post(boost::bind(&AccessManager::UnregisterUserToDB, this, UnRegUsrReq.m_userInfo.m_strUserID, DELETE_STATUS));

    blResult = true;

        
    return blResult;
}

bool AccessManager::QueryUsrInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::QueryUsrInfoReq_USR req;
    InteractiveProtoHandler::User usr;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &usr, &writer, &strSrcID)
    {
        InteractiveProtoHandler::QueryUsrInfoRsp_USR rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryUsrInfoRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        rsp.m_userInfo.m_strCreatedate = usr.m_strCreatedate;
        rsp.m_userInfo.m_strExtend = usr.m_strExtend;
        rsp.m_userInfo.m_strUserID = usr.m_strUserID;
        rsp.m_userInfo.m_strUserName = usr.m_strUserName;
        rsp.m_userInfo.m_strUserPassword = usr.m_strUserPassword;
        rsp.m_userInfo.m_uiStatus = usr.m_uiStatus;
        rsp.m_userInfo.m_uiTypeInfo = usr.m_uiTypeInfo;
        rsp.m_userInfo.m_strAliasName = usr.m_strAliasName;
        rsp.m_userInfo.m_strEmail = usr.m_strEmail;

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query user info rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query user info rsp already send, dst id is " << strSrcID << " and user id is " << req.m_strUserID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query user info req unserialize failed, src id is " << strSrcID);
        return false;
    }


    if (!QueryUserInfoToDB(req.m_strUserID, usr))
    {
        LOG_ERROR_RLD("Query user info from db failed, user id is " << req.m_strUserID);
        return false;
    }

        
    blResult = true;

    return blResult;

}

bool AccessManager::ModifyUsrInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::ModifyUserInfoReq_USR ModifyUsrReq;

    BOOST_SCOPE_EXIT(&blResult, this_, &ModifyUsrReq, &writer, &strSrcID)
    {
        InteractiveProtoHandler::ModifyUserInfoRsp_USR ModifyUsrRsp;
        ModifyUsrRsp.m_MsgType = InteractiveProtoHandler::MsgType::ModifyUserInfoRsp_USR_T;
        ModifyUsrRsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        ModifyUsrRsp.m_strSID = ModifyUsrReq.m_strSID;
        ModifyUsrRsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        ModifyUsrRsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        ModifyUsrRsp.m_strValue = "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(ModifyUsrRsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify user rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);

        LOG_INFO_RLD("User modify rsp already send, dst id is " << strSrcID << " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END
    
    if (!m_pProtoHandler->UnSerializeReq(strMsg, ModifyUsrReq))
    {
        LOG_ERROR_RLD("Modify user req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (ModifyUsrReq.m_userInfo.m_strUserID.empty())
    {
        LOG_ERROR_RLD("Modify user id is empty, src id is " << strSrcID);
        return false;
    }

    if (!ValidUser(ModifyUsrReq.m_userInfo.m_strUserID, ModifyUsrReq.m_userInfo.m_strUserName,
        "", ModifyUsrReq.m_userInfo.m_uiTypeInfo))
    {
        LOG_ERROR_RLD("Modify user failed because user is invalid and user name is " << ModifyUsrReq.m_userInfo.m_strUserName <<
            " and user id is " << ModifyUsrReq.m_userInfo.m_strUserID << " and user pwd is " << ModifyUsrReq.m_userInfo.m_strUserPassword);
        return false;
    }
    
    LOG_INFO_RLD("Modify user id is " << ModifyUsrReq.m_userInfo.m_strUserID << " session id is " << ModifyUsrReq.m_strSID 
        << " and user name is " << ModifyUsrReq.m_userInfo.m_strUserName << " and user pwd is " << ModifyUsrReq.m_userInfo.m_strUserPassword
        << " and user type is " << ModifyUsrReq.m_userInfo.m_uiTypeInfo << " and user extend is " << ModifyUsrReq.m_userInfo.m_strExtend);

    if (!ModifyUsrReq.m_strOldPwd.empty() && !IsUserPasswordValid(ModifyUsrReq.m_userInfo.m_strUserID, ModifyUsrReq.m_strOldPwd))
    {
        LOG_ERROR_RLD("Check user password valid failed, modify password must provide the correct old password, user id is " <<
            ModifyUsrReq.m_userInfo.m_strUserID << " and src id is " << ModifyUsrReq.m_strSID);
        return false;
    }

    m_DBRuner.Post(boost::bind(&AccessManager::UpdateUserInfoToDB, this, ModifyUsrReq.m_userInfo));

    blResult = true;

    return blResult;
}

bool AccessManager::LoginReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::LoginReq_USR LoginReqUsr;

    const std::string &strSessionID = CreateUUID();
    std::list<InteractiveProtoHandler::Relation> RelationList;
    BOOST_SCOPE_EXIT(&blResult, this_, &RelationList, &strSessionID, &LoginReqUsr, &writer, &strSrcID)
    {
        InteractiveProtoHandler::LoginRsp_USR LoginRspUsr;

        LoginRspUsr.m_MsgType = InteractiveProtoHandler::MsgType::LoginRsp_USR_T;
        LoginRspUsr.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        LoginRspUsr.m_strSID = strSessionID;
        LoginRspUsr.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        LoginRspUsr.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        LoginRspUsr.m_strUserID = LoginReqUsr.m_userInfo.m_strUserID;
        LoginRspUsr.m_strValue = "value";

        if (blResult)
        {
            LoginRspUsr.m_reInfoList.swap(RelationList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(LoginRspUsr, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Login user rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("User login rsp already send, dst id is " << strSrcID << " and user id is " << LoginRspUsr.m_strUserID <<
            " and user name is " << LoginReqUsr.m_userInfo.m_strUserName <<
            " and user password is " << LoginReqUsr.m_userInfo.m_strUserPassword <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    
    if (!m_pProtoHandler->UnSerializeReq(strMsg, LoginReqUsr))
    {
        LOG_ERROR_RLD("Login user req unserialize failed, src id is " << strSrcID);
        return false;
    }
    
    if (LoginReqUsr.m_userInfo.m_strUserID.empty() && LoginReqUsr.m_userInfo.m_strUserName.empty())
    {
        LOG_ERROR_RLD("Login user req both user id and user name is empty.");
        return false;
    }

    ////
    //if (LoginReqUsr.m_userInfo.m_strUserPassword.empty())
    //{
    //    LOG_ERROR_RLD("Login user req user password is empty.");
    //    return false;
    //}
    
    if (!ValidUser(LoginReqUsr.m_userInfo.m_strUserID, LoginReqUsr.m_userInfo.m_strUserName, 
        LoginReqUsr.m_userInfo.m_strUserPassword, LoginReqUsr.m_userInfo.m_uiTypeInfo))
    {
        if (!LoginLTUserSiteReq(LoginReqUsr.m_userInfo.m_strUserName, LoginReqUsr.m_userInfo.m_strUserPassword,
            m_ParamInfo.m_strLTUserSite, m_ParamInfo.m_strLTUserSiteRC4Key, strSrcID, LoginReqUsr.m_userInfo.m_strUserID))
        {
            LOG_ERROR_RLD("LoginLTUserSiteReq failed, login user name: " << LoginReqUsr.m_userInfo.m_strUserName);
            return false;
        }
        LOG_INFO_RLD("LoginLTUserSiteReq seccessful, login user name: " << LoginReqUsr.m_userInfo.m_strUserName);
    }

    //用户登录之后，对应的Session信息就保存在memcached中去，key是SessionID，value是用户信息，json格式字符串，格式如下：
    //{"logindate":"2016-11-30 15:30:20","userid":"5167F842BB3AFF4C8CC1F2557E6EFB82"}
    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));

    
    Json::Value jsBody;
    jsBody["logindate"] = strCurrentTime;
    jsBody["userid"] = LoginReqUsr.m_userInfo.m_strUserID;
    jsBody["username"] = LoginReqUsr.m_userInfo.m_strUserName;
    Json::FastWriter fastwriter;
    const std::string &strBody = fastwriter.write(jsBody); //jsBody.toStyledString();
     
    m_SessionMgr.Create(strSessionID, strBody, boost::lexical_cast<unsigned int>(m_ParamInfo.m_strSessionTimeoutCountThreshold), 
        boost::bind(&AccessManager::SessionTimeoutProcessCB, this, _1));
        
    if (!QueryRelationByUserID(LoginReqUsr.m_userInfo.m_strUserID, RelationList))
    {
        LOG_ERROR_RLD("Query device info failed and user id is " << LoginReqUsr.m_userInfo.m_strUserID);
        return false;
    }

    //广播消息表示用户登录


    blResult = true;
    
    
    return blResult;
}

bool AccessManager::LoginLTUserSiteReq(const std::string &strUserName, const std::string &strPassword,
    const std::string &strLTUserSite, const std::string &strLTRC4Key, const std::string &strSrcID, std::string &strUserID)
{
    UserLoginLTUserSite userLogin(strLTUserSite, strLTRC4Key);
    if (userLogin.Login(strUserName, strPassword) != LOGIN_OK)
    {
        return false;
    }

    //登录成功后将用户数据插入到本地数据库
    InteractiveProtoHandler::RegisterUserReq_USR RegUsrReq;
    RegUsrReq.m_MsgType = InteractiveProtoHandler::MsgType::RegisterUserReq_USR_T;
    RegUsrReq.m_uiMsgSeq = 0;
    RegUsrReq.m_strSID = "";
    RegUsrReq.m_userInfo.m_strUserID = strUserID = CreateUUID();
    RegUsrReq.m_userInfo.m_strUserName = strUserName;
    RegUsrReq.m_userInfo.m_strUserPassword = strPassword;
    RegUsrReq.m_userInfo.m_strCreatedate = "";
    RegUsrReq.m_userInfo.m_uiTypeInfo = 0;
    RegUsrReq.m_userInfo.m_uiStatus = NORMAL_STATUS;
    RegUsrReq.m_userInfo.m_strExtend = "";
    RegUsrReq.m_strValue = "";

    std::string strSerializeLoginReq;
    if (!m_pProtoHandler->SerializeReq(RegUsrReq, strSerializeLoginReq))
    {
        LOG_ERROR_RLD("Insert register user info failed, user name: " << strUserName);
        return false;
    }

    LOG_INFO_RLD("Insert register user info, user name: " << strUserName);
    return RegisterUserReq(strSerializeLoginReq, strSrcID, NULL);
}

bool AccessManager::LogoutReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
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

bool AccessManager::ShakehandReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
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

bool AccessManager::AddDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;
    InteractiveProtoHandler::AddDevReq_USR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID)
    {
        InteractiveProtoHandler::AddDevRsp_USR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::AddDevRsp_USR_T;
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

    std::string strUserID;
    if (!QueryOwnerUserIDByDeviceID(req.m_devInfo.m_strDevID, strUserID))
    {
        LOG_ERROR_RLD("Add device query added device owner failed, src id is " << strSrcID);
        return false;
    }
    
    if (strUserID.empty())
    {
        std::string strUuid;
        if (!GetMySqlUUID(strUuid))
        {
            LOG_ERROR_RLD("Add device get mysql uuid failed, src id is " << strSrcID);
            return false;
        }

        InteractiveProtoHandler::Device DevInfo;
        DevInfo.m_strDevID = req.m_devInfo.m_strDevID;
        DevInfo.m_strDevName = req.m_devInfo.m_strDevName;
        DevInfo.m_strDevPassword = req.m_devInfo.m_strDevPassword;
        DevInfo.m_uiTypeInfo = req.m_devInfo.m_uiTypeInfo;
        DevInfo.m_strCreatedate = strCurrentTime;
        DevInfo.m_uiStatus = NORMAL_STATUS;
        DevInfo.m_strInnerinfo = req.m_devInfo.m_strInnerinfo;
        DevInfo.m_strExtend = req.m_devInfo.m_strExtend;

        m_DBRuner.Post(boost::bind(&AccessManager::InsertDeviceToDB, this, strUuid, DevInfo));

        RelationOfUsrAndDev relation;
        relation.m_iRelation = RELATION_OF_OWNER;
        relation.m_iStatus = NORMAL_STATUS;
        relation.m_strBeginDate = strCurrentTime;
        relation.m_strEndDate = MAX_DATE;
        relation.m_strCreateDate = strCurrentTime;
        relation.m_strDevID = req.m_devInfo.m_strDevID;
        relation.m_strExtend = req.m_devInfo.m_strExtend;
        relation.m_strUsrID = req.m_strUserID;

        m_DBRuner.Post(boost::bind(&AccessManager::InsertRelationToDB, this, strUuid, relation));

        AddNoOwnerFile(req.m_strUserID, req.m_devInfo.m_strDevID);

        blResult = true;
    }
    else if (strUserID == req.m_strUserID)
    {
        LOG_INFO_RLD("Add devcice successful, the device has been added by current user, user id is" << strUserID);
        blResult = true;
    } 
    else
    {
        LOG_ERROR_RLD("Add device failed, the device has been added, owner user id is " << strUserID << " and src id is " << strSrcID);
    }

    return blResult;
}

bool AccessManager::DelDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;
    InteractiveProtoHandler::DelDevReq_USR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID)
    {
        InteractiveProtoHandler::DelDevRsp_USR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::DelDevRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete device rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("User delete device rsp already send, dst id is " << strSrcID << " and user id is " << req.m_strUserID <<
            " and session id is " << req.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END


    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Delete device req unserialize failed, src id is " << strSrcID);
        return false;
    }

    
    m_DBRuner.Post(boost::bind(&AccessManager::DelDeviceToDB, this, req.m_strDevIDList, DELETE_STATUS));

    blResult = true;

    return blResult;
}

bool AccessManager::ModDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;
    InteractiveProtoHandler::ModifyDevReq_USR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID)
    {
        InteractiveProtoHandler::ModifyDevRsp_USR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::ModifyDevRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify device rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("User modify device rsp already send, dst id is " << strSrcID << " and user id is " << req.m_strUserID <<
            " and session id is " << req.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Modify device req unserialize failed, src id is " << strSrcID);
        return false;
    }

    m_DBRuner.Post(boost::bind(&AccessManager::ModDeviceToDB, this, req.m_devInfo));

    blResult = true;


    return blResult;    
}

bool AccessManager::QueryDevInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::QueryDevInfoReq_USR req;
    InteractiveProtoHandler::Device dev;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &dev, &writer, &strSrcID)
    {
        InteractiveProtoHandler::QueryDevInfoRsp_USR rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryDevInfoRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        rsp.m_devInfo.m_strCreatedate = dev.m_strCreatedate;
        rsp.m_devInfo.m_strDevID = dev.m_strDevID;
        rsp.m_devInfo.m_strDevName = dev.m_strDevName;
        rsp.m_devInfo.m_strDevPassword = dev.m_strDevPassword;
        rsp.m_devInfo.m_strExtend = dev.m_strExtend;
        rsp.m_devInfo.m_strInnerinfo = dev.m_strInnerinfo;
        rsp.m_devInfo.m_uiStatus = dev.m_uiStatus;
        rsp.m_devInfo.m_uiTypeInfo = dev.m_uiTypeInfo;

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query device info rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query device info rsp already send, dst id is " << strSrcID << " and device id is " << req.m_strDevID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query device info req unserialize failed, src id is " << strSrcID);
        return false;
    }
    
    if (!QueryDevInfoToDB(req.m_strDevID, dev))
    {
        LOG_ERROR_RLD("Query device info from db failed, device id is " << req.m_strDevID);
        return false;
    }
    
    blResult = true;

    return blResult;
}

bool AccessManager::QueryDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;
    InteractiveProtoHandler::QueryDevReq_USR req;
    std::list<InteractiveProtoHandler::Relation> RelationList;

    BOOST_SCOPE_EXIT(&blResult, this_, &RelationList, &req, &writer, &strSrcID)
    {
        InteractiveProtoHandler::QueryDevRsp_USR rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryDevRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_allRelationInfoList.swap(RelationList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query device rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query device rsp already send, dst id is " << strSrcID << " and user id is " << req.m_strUserID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END


    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query device req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryRelationByUserID(req.m_strUserID, RelationList, req.m_uiBeginIndex))
    {
        LOG_ERROR_RLD("Query device info failed and user id is " << req.m_strUserID);
        return false;
    }



    blResult = true;


    return blResult;
}

bool AccessManager::QueryUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;
    InteractiveProtoHandler::QueryUserReq_USR req;
    std::list<InteractiveProtoHandler::Relation> RelationList;


    BOOST_SCOPE_EXIT(&blResult, this_, &RelationList, &req, &writer, &strSrcID)
    {
        InteractiveProtoHandler::QueryUserRsp_USR rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryUserRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_allRelationInfoList.swap(RelationList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query user rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query user rsp already send, dst id is " << strSrcID << " and device id is " << req.m_strDevID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END


    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query user req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryRelationByDevID(req.m_strDevID, RelationList, req.m_uiBeginIndex))
    {
        LOG_ERROR_RLD("Query user info failed and user id is " << req.m_strDevID);
        return false;
    }


    blResult = true;

    return blResult;
}

bool AccessManager::SharingDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;
    InteractiveProtoHandler::SharingDevReq_USR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID)
    {
        InteractiveProtoHandler::SharingDevRsp_USR rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::SharingDevRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Sharing device rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("User sharing device rsp already send, dst id is " << strSrcID << " and user id is " << req.m_relationInfo.m_strUserID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Sharing device req unserialize failed, src id is " << strSrcID);
        return false;
    }

    //考虑到用户设备关系表后续查询的方便，用户与设备的关系在表中体现的是一条条记录
    //分享设备会在对应的用户设备关系表中只增加一条记录，原来的主动分享的记录不变化（还是表示设备最开始的归属）
    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));
    
    RelationOfUsrAndDev relation;
    relation.m_iRelation = RELATION_OF_BE_SHARED;
    relation.m_iStatus = NORMAL_STATUS;
    relation.m_strBeginDate = req.m_relationInfo.m_strBeginDate;
    relation.m_strEndDate = req.m_relationInfo.m_strEndDate;
    relation.m_strCreateDate = strCurrentTime;
    relation.m_strDevID = req.m_relationInfo.m_strDevID;
    relation.m_strExtend = req.m_relationInfo.m_strValue;
    relation.m_strUsrID = req.m_relationInfo.m_strUserID;

    m_DBRuner.Post(boost::bind(&AccessManager::SharingRelationToDB, this, relation));
    

    blResult = true;
    return blResult;
}

bool AccessManager::CancelSharedDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;
    InteractiveProtoHandler::CancelSharedDevReq_USR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID)
    {
        InteractiveProtoHandler::CancelSharedDevRsp_USR rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::CancelSharedDevRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Cancel shared device rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("User cancel shared device rsp already send, dst id is " << strSrcID << " and user id is " << req.m_relationInfo.m_strUserID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END


    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Cancel shared device req unserialize failed, src id is " << strSrcID);
        return false;
    }

    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));

    RelationOfUsrAndDev relation;
    relation.m_iRelation = RELATION_OF_BE_SHARED;
    relation.m_iStatus = NORMAL_STATUS;
    relation.m_strBeginDate = req.m_relationInfo.m_strBeginDate;
    relation.m_strEndDate = req.m_relationInfo.m_strEndDate;
    relation.m_strCreateDate = strCurrentTime;
    relation.m_strDevID = req.m_relationInfo.m_strDevID;
    relation.m_strExtend = req.m_relationInfo.m_strValue;
    relation.m_strUsrID = req.m_relationInfo.m_strUserID;

    m_DBRuner.Post(boost::bind(&AccessManager::CancelSharedRelationToDB, this, relation));

    blResult = true;

    return blResult;
}

bool AccessManager::AddFriendsReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;
    InteractiveProtoHandler::AddFriendsReq_USR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID)
    {
        InteractiveProtoHandler::AddFriendsRsp_USR rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::AddFriendsRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add friend rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("User add friend rsp already send, dst id is " << strSrcID << " and user id is " << req.m_strUserID << 
            " and friend id is " << req.m_strFriendUserID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Add friend req unserialize failed, src id is " << strSrcID);
        return false;
    }

    //校验好友用户ID有效性
    InteractiveProtoHandler::User usr;
    if (!QueryUserInfoToDB(req.m_strFriendUserID, usr))
    {
        LOG_ERROR_RLD("Query friend user info from db failed, friend user id is " << req.m_strFriendUserID);
        return false;
    }

    if (usr.m_strUserID.empty())
    {
        LOG_ERROR_RLD("Query friend user info from db is empty, friend user id is " << req.m_strFriendUserID);
        return false;
    }
    
    //校验好友用户是否存在
    bool blExist = false;
    if (!QueryUserRelationExist(req.m_strUserID, req.m_strFriendUserID, RELATION_OF_FRIENDS, blExist, false))
    {
        LOG_ERROR_RLD("Query friend relation failed and user id is " << req.m_strUserID << " and friends id is " << req.m_strFriendUserID);
        return false;
    }

    //查询是否重复添加好友，若是，考虑到幂等性，也需要返回成功，只是不再需要插入数据库操作了
    if (blExist)
    {
        LOG_INFO_RLD("Query friend relation already exist and user id is " << req.m_strUserID << " and friends id is " << req.m_strFriendUserID);
        blResult = true;
        return blResult;
    }
        
    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));
    
    RelationOfUsr relation;
    relation.m_iRelation = RELATION_OF_FRIENDS;
    relation.m_iStatus = NORMAL_STATUS;
    relation.m_strCreateDate = strCurrentTime;
    //relation.m_strExtend
    relation.m_strRelationOfUsrID = req.m_strFriendUserID;
    relation.m_strUsrID = req.m_strUserID;

    m_DBRuner.Post(boost::bind(&AccessManager::AddFriendsToDB, this, relation));

    blResult = true;

    return blResult;
}

bool AccessManager::DelFriendsReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::DelFriendsReq_USR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID)
    {
        InteractiveProtoHandler::DelFriendsRsp_USR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::DelFriendsRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete friends rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("User delete friends rsp already send, dst id is " << strSrcID << " and user id is " << req.m_strUserID <<
            " and session id is " << req.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END
        
    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Delete friends req unserialize failed, src id is " << strSrcID);
        return false;
    }
    
    m_DBRuner.Post(boost::bind(&AccessManager::DelFriendsToDB, this, req.m_strUserID, req.m_strFriendUserIDList, DELETE_STATUS));
    
    blResult = true;

    return blResult;
}

bool AccessManager::QueryFriendsReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::QueryFriendsReq_USR req;
    std::list<std::string> strRelationIDList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strRelationIDList, &req, &writer, &strSrcID)
    {
        InteractiveProtoHandler::QueryFriendsRsp_USR rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryFriendsRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_allFriendUserIDList.swap(strRelationIDList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query user friends rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query user friends rsp already send, dst id is " << strSrcID << " and user id is " << req.m_strUserID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query friends req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryUserRelationInfoToDB(req.m_strUserID, RELATION_OF_FRIENDS, strRelationIDList))
    {
        LOG_ERROR_RLD("Query friends failed and user id is " << req.m_strUserID);
        return false;
    }


    blResult = true;
    
    return blResult;

}

bool AccessManager::AddFileReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::AddFileReq_DEV req;
    std::list<std::string> strFileIDFailedList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strFileIDFailedList, &req, &writer, &strSrcID)
    {
        InteractiveProtoHandler::AddFileRsp_DEV rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::AddFileRsp_DEV_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        if (blResult)
        {
            rsp.m_strFileIDFailedList.swap(strFileIDFailedList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add file device rsp serialize failed.");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Add file device rsp already send, dst id is " << strSrcID << " and device id is " << req.m_strDevID <<
            " and session id is " << req.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Add file device req unserialize failed, src id is " << strSrcID);
        return false;
    }

    m_DBRuner.Post(boost::bind(&AccessManager::AddDeviceFileToDB, this, req.m_strDevID, req.m_fileList, strFileIDFailedList));

    blResult = true;

    return blResult;
}

bool AccessManager::DeleteFileReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::DeleteFileReq_USR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID)
    {
        InteractiveProtoHandler::DeleteFileRsp_USR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::DeleteFileRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete file rsp serialize failed.");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("User delete file rsp already send, dst id is " << strSrcID << " and user id is " << req.m_strUserID <<
            " and session id is " << req.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Delete file req unserialize failed, src id is " << strSrcID);
        return false;
    }

    m_DBRuner.Post(boost::bind(&AccessManager::DeleteFileToDB, this, req.m_strUserID, req.m_strFileIDList, DELETE_STATUS));

    blResult = true;

    return blResult;
}

bool AccessManager::DownloadFileReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::DownloadFileReq_USR req;
    std::list<InteractiveProtoHandler::FileUrl> fileUrlList;

    BOOST_SCOPE_EXIT(&blResult, this_, &fileUrlList, &req, &writer, &strSrcID)
    {
        InteractiveProtoHandler::DownloadFileRsp_USR rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::DownloadFileRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        if (blResult)
        {
            rsp.m_fileUrlList.swap(fileUrlList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Download file rsp serialize failed.");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Download file rsp already send, dst id is " << strSrcID << " and user id is " << req.m_strUserID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Download file req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!DownloadFileToDB(req.m_strUserID, req.m_strFileIDList, fileUrlList))
    {
        LOG_ERROR_RLD("Download file failed and user id is " << req.m_strUserID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool AccessManager::QueryFileReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::QueryFileReq_USR req;
    std::list<InteractiveProtoHandler::File> fileInfoList;

    BOOST_SCOPE_EXIT(&blResult, this_, &fileInfoList, &req, &writer, &strSrcID)
    {
        InteractiveProtoHandler::QueryFileRsp_USR rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryFileRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        if (blResult)
        {
            rsp.m_fileList.swap(fileInfoList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query file rsp serialize failed.");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query file rsp already send, dst id is " << strSrcID << " and user id is " << req.m_strUserID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query file req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryFileToDB(req.m_strUserID, req.m_strDevID, fileInfoList, req.m_uiBusinessType, req.m_strBeginDate, req.m_strEndDate))
    {
        LOG_ERROR_RLD("Query file failed and user id is " << req.m_strUserID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool AccessManager::LoginReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;
    
    std::string strValue;
    InteractiveProtoHandler::LoginReq_DEV LoginReqDev;
    const std::string &strSessionID = CreateUUID();

    BOOST_SCOPE_EXIT(&blResult, this_, &strSessionID, &LoginReqDev, &writer, &strSrcID, &strValue)
    {
        InteractiveProtoHandler::LoginRsp_DEV LoginRspUsr;

        LoginRspUsr.m_MsgType = InteractiveProtoHandler::MsgType::LoginRsp_DEV_T;
        LoginRspUsr.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        LoginRspUsr.m_strSID = strSessionID;
        LoginRspUsr.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        LoginRspUsr.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        LoginRspUsr.m_strValue = strValue;

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(LoginRspUsr, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Login device rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Device login rsp already send, dst id is " << strSrcID << " and device id is " << LoginReqDev.m_strDevID <<
            " and device password is " << LoginReqDev.m_strPassword <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END
        
    if (!m_pProtoHandler->UnSerializeReq(strMsg, LoginReqDev))
    {
        LOG_ERROR_RLD("Login device req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (LoginReqDev.m_strDevID.empty())
    {
        LOG_ERROR_RLD("Login req of device id is empty.");
        return false;
    }

    std::string strCountryCode;
    std::string strCountryNameEn;
    std::string strCountryNameZh;
    std::string strTimeZone;
    if (!GetTimeZone(LoginReqDev.m_strValue, strCountryCode, strCountryNameEn, strCountryNameZh, strTimeZone)) //注意，这里使用了Value字段来传递ip
    {
        LOG_ERROR_RLD("Get time zone failed.");
        return false;
    }

    {
        Json::Value jsBody;
        jsBody["countrycode"] = strCountryCode;
        jsBody["countryname_en"] = strCountryNameEn;
        jsBody["countryname_zh"] = strCountryNameZh;
        jsBody["timezone"] = strTimeZone;
        Json::FastWriter fastwriter;
        const std::string &strBody = fastwriter.write(jsBody);
        strValue = strBody;
    }
    //目前暂不对设备进行校验
    //登录之后，对应的Session信息就保存在memcached中去，key是SessionID，value是信息，json格式字符串，格式如下：
    //{"logindate":"2016-11-30 15:30:20","userid":"5167F842BB3AFF4C8CC1F2557E6EFB82"}
    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));
    
    Json::Value jsBody;
    jsBody["logindate"] = strCurrentTime;
    jsBody["devid"] = LoginReqDev.m_strDevID;
    jsBody["devpwd"] = LoginReqDev.m_strPassword;
    Json::FastWriter fastwriter;
    const std::string &strBody = fastwriter.write(jsBody); //jsBody.toStyledString();

    m_SessionMgr.Create(strSessionID, strBody, boost::lexical_cast<unsigned int>(m_ParamInfo.m_strSessionTimeoutCountThreshold),
        boost::bind(&AccessManager::SessionTimeoutProcessCB, this, _1));

    blResult = true;

    return blResult;
}

bool AccessManager::P2pInfoReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::P2pInfoReq_DEV req;

    std::string strP2pServer;
    std::string strP2pID;
    unsigned int uiLease = 0;
    std::string strLicenseKey;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID, &strP2pServer, &strP2pID, &uiLease, &strLicenseKey)
    {
        InteractiveProtoHandler::P2pInfoRsp_DEV rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::P2pInfoRsp_DEV_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strP2pID = blResult ? strP2pID : "";
        rsp.m_strP2pServer = blResult ? strP2pServer : "";
        rsp.m_uiLease = blResult ? uiLease : 0;
        rsp.m_strLicenseKey = blResult ? strLicenseKey : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("P2p info of device rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("P2p info of device rsp already send, dst id is " << strSrcID << " and device id is " << req.m_strDevID <<
            " and device ip is " << req.m_strDevIpAddress << " and p2p id is " << strP2pID << " and p2p server is " << strP2pServer <<
            " and lease is " << uiLease << " and liecense key is " << strLicenseKey <<
            " and session id is " << req.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("P2p info of device req unserialize failed, src id is " << strSrcID);
        return false;
    }

    //获取p2pid和p2pserver
    P2PConnectParam p2pConnParam;
    P2PServerManager_SY p2pSvrManager;
    p2pSvrManager.SetUrl(GET_IPINFO_SITE);
    p2pSvrManager.SetDBManager(&m_DBCache, m_pMysql);
    if (!p2pSvrManager.DeviceRequestP2PConnectParam(p2pConnParam, req.m_strDevID, req.m_strDevIpAddress))
    {
        LOG_ERROR_RLD("Get device p2p info failed, device id is " << req.m_strDevID << " and ip is " << req.m_strDevIpAddress);
        return false;
    }

    strP2pID = p2pConnParam.sP2Pid;
    strP2pServer = p2pConnParam.sInitstring;
    uiLease = p2pConnParam.nTime;
    strLicenseKey = p2pConnParam.sparam1;

    blResult = true;

    return blResult;
}

bool AccessManager::ShakehandReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::ShakehandReq_DEV ShakehandReqDev;

    BOOST_SCOPE_EXIT(&blResult, this_, &ShakehandReqDev, &writer, &strSrcID)
    {
        InteractiveProtoHandler::ShakehandRsp_DEV ShakehandRspDev;
        ShakehandRspDev.m_MsgType = InteractiveProtoHandler::MsgType::ShakehandRsp_DEV_T;
        ShakehandRspDev.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        ShakehandRspDev.m_strSID = ShakehandReqDev.m_strSID;
        ShakehandRspDev.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        ShakehandRspDev.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        ShakehandRspDev.m_strValue = "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(ShakehandRspDev, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Shakehand device rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("User shakehand of device rsp already send, dst id is " << strSrcID << " and device id is " << ShakehandReqDev.m_strDevID <<
            " and session id is " << ShakehandReqDev.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, ShakehandReqDev))
    {
        LOG_ERROR_RLD("Shakehand device req unserialize failed, src id is " << strSrcID);
        return false;
    }

    m_SessionMgr.Reset(ShakehandReqDev.m_strSID);

    LOG_INFO_RLD("Shakehand device received and device id is " << ShakehandReqDev.m_strDevID << " and session id is " << ShakehandReqDev.m_strSID);

    blResult = true;

    return blResult;
}

bool AccessManager::LogoutReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::LogoutReq_DEV LogoutReqDev;

    BOOST_SCOPE_EXIT(&blResult, this_, &LogoutReqDev, &writer, &strSrcID)
    {
        InteractiveProtoHandler::LogoutRsp_DEV LogoutRspDev;
        LogoutRspDev.m_MsgType = InteractiveProtoHandler::MsgType::LogoutRsp_DEV_T;
        LogoutRspDev.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        LogoutRspDev.m_strSID = LogoutReqDev.m_strSID;
        LogoutRspDev.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        LogoutRspDev.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        LogoutRspDev.m_strValue = "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(LogoutRspDev, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Logout device rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Device logout rsp already send, dst id is " << strSrcID << " and device id is " << LogoutReqDev.m_strDevID <<
            " and session id is " << LogoutReqDev.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, LogoutReqDev))
    {
        LOG_ERROR_RLD("Logout device req unserialize failed, src id is " << strSrcID);
        return false;
    }


    if (LogoutReqDev.m_strSID.empty())
    {
        LOG_ERROR_RLD("Logout device req session id is empty and src id is " << strSrcID);
        return false;
    }

    //检查SessionID是否存在
    m_SessionMgr.Remove(LogoutReqDev.m_strSID); //移除会话
    

    blResult = true;

    return blResult;
}

bool AccessManager::P2pInfoReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::P2pInfoReq_USR req;

    std::string strP2pServer;
    std::string strP2pID;
    unsigned int uiLease = 0;
    std::string strLicenseKey;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID, &strP2pServer, &strP2pID, &uiLease, &strLicenseKey)
    {
        InteractiveProtoHandler::P2pInfoRsp_USR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::P2pInfoRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strP2pID = blResult ? strP2pID : "";
        rsp.m_strP2pServer = blResult ? strP2pServer : "";
        rsp.m_uiLease = blResult ? uiLease : 0;
        rsp.m_strLicenseKey = blResult ? strLicenseKey : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("P2p info of user rsp serialize failed.");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("P2p info of user rsp already send, dst id is " << strSrcID << " and user id is " << req.m_strUserID <<
            " and device id is " << req.m_strDevID << " and user ip is " << req.m_strUserIpAddress <<
            " and p2p id is " << strP2pID << " and p2p server is " << strP2pServer << " and lease is " << uiLease <<
            " and liecense key is " << strLicenseKey <<
            " and session id is " << req.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("P2p info of user req unserialize failed, src id is " << strSrcID);
        return false;
    }

    //获取p2pid和p2pserver
    P2PConnectParam p2pConnParam;
    P2PServerManager_SY p2pSvrManager;
    p2pSvrManager.SetUrl(GET_IPINFO_SITE);
    p2pSvrManager.SetDBManager(&m_DBCache, m_pMysql);
    if (!p2pSvrManager.DeviceRequestP2PConnectParam(p2pConnParam, req.m_strDevID, req.m_strUserIpAddress, req.m_strUserID))
    {
        LOG_ERROR_RLD("Get user p2p info failed,  user id is " << req.m_strUserID <<
            " and device id is " << req.m_strDevID << " and ip is " << req.m_strUserIpAddress);
        return false;
    }

    strP2pID = p2pConnParam.sP2Pid;
    strP2pServer = p2pConnParam.sInitstring;
    uiLease = p2pConnParam.nTime;
    strLicenseKey = p2pConnParam.sparam1;

    blResult = true;

    return blResult;
}

bool AccessManager::RetrievePwdReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::RetrievePwdReq_USR RetrievePwdReqUsr;

    BOOST_SCOPE_EXIT(&blResult, this_, &RetrievePwdReqUsr, &writer, &strSrcID)
    {
        InteractiveProtoHandler::RetrievePwdRsp_USR RetrievePwdRspUsr;
        RetrievePwdRspUsr.m_MsgType = InteractiveProtoHandler::MsgType::RetrievePwdRsp_USR_T;
        RetrievePwdRspUsr.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        RetrievePwdRspUsr.m_strSID = RetrievePwdReqUsr.m_strSID;
        RetrievePwdRspUsr.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        RetrievePwdRspUsr.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        RetrievePwdRspUsr.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(RetrievePwdRspUsr, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Retrieve user password rsp serialize failed.");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Retrieve user password rsp already send, dst id is " << strSrcID << " and user name is " << RetrievePwdReqUsr.m_strUserName <<
            " and session id is " << RetrievePwdReqUsr.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, RetrievePwdReqUsr))
    {
        LOG_ERROR_RLD("Retrieve user password req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!CheckEmailByUserName(RetrievePwdReqUsr.m_strUserName, RetrievePwdReqUsr.m_strEmail))
    {
        LOG_ERROR_RLD("Retrieve user password failed, the user name or email is not correct, user name is " << RetrievePwdReqUsr.m_strUserName <<
            " and email is " << RetrievePwdReqUsr.m_strEmail);
        return false;
    }

    std::string strRandPwd = CreateUUID().erase(8);

    m_DBRuner.Post(boost::bind(&AccessManager::ResetUserPasswordToDB, this, RetrievePwdReqUsr.m_strUserName, strRandPwd));

    m_DBRuner.Post(boost::bind(&AccessManager::SendUserResetPasswordEmail, this, RetrievePwdReqUsr.m_strUserName, strRandPwd, RetrievePwdReqUsr.m_strEmail));

    LOG_INFO_RLD("Retrieve user password received and user name is " << RetrievePwdReqUsr.m_strUserName << " and session id is " << RetrievePwdReqUsr.m_strSID);

    blResult = true;

    return blResult;
}

bool AccessManager::QueryTimeZoneReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::QueryTimeZoneReq_DEV req;

    std::string strCountryCode;
    std::string strCountryNameEn;
    std::string strCountryNameZh;
    std::string strTimeZone;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID, &strCountryCode, &strCountryNameEn, &strCountryNameZh, &strTimeZone)
    {
        InteractiveProtoHandler::QueryTimeZoneRsp_DEV rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryTimeZoneRsp_DEV_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strCountryCode = blResult ? strCountryCode : "";
        rsp.m_strCountryNameEn = blResult ? strCountryNameEn : "";
        rsp.m_strCountryNameZh = blResult ? strCountryNameZh : "";
        rsp.m_strTimeZone = blResult ? strTimeZone : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query timezone rsp serialize failed.");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query timezone info rsp already send, dst id is " << strSrcID << " and device id is " << req.m_strDevID <<
            " and device ip is " << req.m_strDevIpAddress << " and country code is " << strCountryCode << " and country name en is " << strCountryNameEn <<
            " and country name zh is " << strCountryNameZh << " and contry time zone is " << strTimeZone <<
            " and session id is " << req.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query timezone req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!GetTimeZone(req.m_strDevIpAddress, strCountryCode, strCountryNameEn, strCountryNameZh, strTimeZone))
    {
        LOG_ERROR_RLD("Get time zone failed.");
        return false;
    }

    blResult = true;

    return blResult;
}

bool AccessManager::QueryAccessDomainNameReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::QueryAccessDomainNameReq_USR req;

    std::string strDomainName;
    unsigned int uiLease;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &strSrcID, &writer, &strDomainName, &uiLease)
    {
        InteractiveProtoHandler::QueryAccessDomainNameRsp_USR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryAccessDomainNameRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strDomainName = blResult ? strDomainName : "";
        rsp.m_uiLease = blResult ? uiLease : 0;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query user access domain name rsp serialize failed.");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query user access domain name rsp already send, dst id is " << strSrcID <<
            " and user ip is " << req.m_strUserIpAddress <<
            " and session id is " << req.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query user access domain name req unserialize failed, src id is " << strSrcID);
        return false;
    }

    TimeZone timezone;
    CTimeZone cTimeZone;
    cTimeZone.setpostUrl(GET_IPINFO_SITE);
    cTimeZone.SetDBManager(&m_DBCache, m_pMysql);
    if (!cTimeZone.GetCountryTime(req.m_strUserIpAddress, timezone))
    {
        LOG_ERROR_RLD("Get country timezone info failed, user ip is " << req.m_strUserIpAddress);
        return false;
    }

    AccessDomainInfo DomainInfo;
    if (!QueryAccessDomainInfoByArea(timezone.sCode, "", DomainInfo))
    {
        LOG_ERROR_RLD("Query user access domain name failed, user ip is " << req.m_strUserIpAddress);
        return false;
    }

    strDomainName = DomainInfo.strDomainName;
    uiLease = DomainInfo.uiLease;

    blResult = true;

    return blResult;
}

bool AccessManager::QueryAccessDomainNameReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::QueryAccessDomainNameReq_DEV req;

    std::string strDomainName;
    unsigned int uiLease;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &strSrcID, &writer, &strDomainName, &uiLease)
    {
        InteractiveProtoHandler::QueryAccessDomainNameRsp_DEV rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryAccessDomainNameRsp_DEV_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strDomainName = blResult ? strDomainName : "";
        rsp.m_uiLease = blResult ? uiLease : 0;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query device access domain name rsp serialize failed.");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query device access domain name rsp already send, dst id is " << strSrcID <<
            " and device ip is " << req.m_strDevIpAddress <<
            " and session id is " << req.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query device access domain name req unserialize failed, src id is " << strSrcID);
        return false;
    }

    TimeZone timezone;
    CTimeZone cTimeZone;
    cTimeZone.setpostUrl(GET_IPINFO_SITE);
    cTimeZone.SetDBManager(&m_DBCache, m_pMysql);
    if (!cTimeZone.GetCountryTime(req.m_strDevIpAddress, timezone))
    {
        LOG_ERROR_RLD("Get country timezone info failed, device ip is " << req.m_strDevIpAddress);
        return false;
    }

    AccessDomainInfo DomainInfo;
    if (!QueryAccessDomainInfoByArea(timezone.sCode, "", DomainInfo))
    {
        LOG_ERROR_RLD("Query user access domain name failed, device ip is " << req.m_strDevIpAddress);
        return false;
    }

    strDomainName = DomainInfo.strDomainName;
    uiLease = DomainInfo.uiLease;

    blResult = true;

    return blResult;
}

void AccessManager::AddDeviceFileToDB(const std::string &strDevID, const std::list<InteractiveProtoHandler::File> &FileInfoList,
    std::list<std::string> &FileIDFailedList)
{
    if (FileInfoList.empty())
    {
        LOG_ERROR_RLD("Add device file info list is empty and device id is " << strDevID);
        return;
    }

    std::string strUserID;
    if (!QueryOwnerUserIDByDeviceID(strDevID, strUserID))
    {
        LOG_ERROR_RLD("Add device file to db failed and device id " << strDevID);
        return;
    }

    FileIDFailedList.clear();

    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));

    for (auto fileInfo : FileInfoList)
    {
        InteractiveProtoHandler::File fileInfoTmp;
        fileInfoTmp.m_strFileID = CreateUUID();
        fileInfoTmp.m_strUserID = strUserID;
        fileInfoTmp.m_strDevID = fileInfo.m_strDevID;
        fileInfoTmp.m_strRemoteFileID = fileInfo.m_strRemoteFileID;
        fileInfoTmp.m_strDownloadUrl = fileInfo.m_strDownloadUrl;
        fileInfoTmp.m_strFileName = fileInfo.m_strFileName;
        fileInfoTmp.m_strSuffixName = fileInfo.m_strSuffixName;
        fileInfoTmp.m_ulFileSize = fileInfo.m_ulFileSize;
        fileInfoTmp.m_uiBusinessType = fileInfo.m_uiBusinessType;
        fileInfoTmp.m_strFileCreatedate = fileInfo.m_strFileCreatedate;
        fileInfoTmp.m_strCreatedate = strCurrentTime;
        fileInfoTmp.m_uiStatus = NORMAL_STATUS;
        fileInfoTmp.m_strExtend = fileInfo.m_strExtend;

        if (!InsertFileToDB(fileInfoTmp))
        {
            FileIDFailedList.push_back(fileInfoTmp.m_strFileID);
        }
    }
}

void AccessManager::DeleteFileToDB(const std::string &strUserID, const std::list<std::string> &FileIDList, const int iStatus)
{
    if (FileIDList.empty())
    {
        LOG_ERROR_RLD("Delete file id list is empty and user id is " << strUserID);
        return;
    }

    char sql[1024] = { 0 };
    const char* sqlfmt = "update t_file_info set status = %d where userid = '%s' and fileid in (";
    snprintf(sql, sizeof(sql), sqlfmt, iStatus, strUserID.c_str());
    std::string strSql(sql);

    for (std::string fileID : FileIDList)
    {
        char cTmp[256] = { 0 };
        snprintf(cTmp, sizeof(cTmp), "'%s',", fileID.c_str());
        strSql.append(cTmp);
    }

    strSql.replace(strSql.length() - 1, 1, std::string(")"));

    if (!m_pMysql->QueryExec(strSql))
    {
        LOG_ERROR_RLD("Delete t_file_info sql exec failed, sql is " << strSql);
    }
}

bool AccessManager::DownloadFileToDB(const std::string &strUserID, const std::list<std::string> &FileIDList,
    std::list<InteractiveProtoHandler::FileUrl> &FileUrlList, const bool IsNeedCache)
{
    if (FileIDList.empty())
    {
        LOG_ERROR_RLD("Download file id list is empty.");
        return true;
    }

    char sql[1024] = { 0 };
    const char* sqlfmt = "select fileid, downloadurl from t_file_info where userid = '%s' and status = 0 and fileid in(";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str());
    std::string strSql(sql);
    
    for (std::string fileID : FileIDList)
    {
        char cTmp[256] = { 0 };
        snprintf(cTmp, sizeof(cTmp), "'%s',", fileID.c_str());
        strSql.append(cTmp);
    }

    strSql.replace(strSql.length() - 1, 1, std::string(")"));

    std::list<boost::any> ResultList;
    if (IsNeedCache && m_DBCache.GetResult(strSql, ResultList) && !ResultList.empty())
    {
        boost::shared_ptr<std::list<InteractiveProtoHandler::FileUrl> > pFileUrlList;
        pFileUrlList = boost::any_cast<boost::shared_ptr<std::list<InteractiveProtoHandler::FileUrl> >>(ResultList.front());

        for (auto fileUrl : *pFileUrlList)
        {
            FileUrlList.push_back(fileUrl);
        }

        LOG_INFO_RLD("Download file get result from cache and sql is " << strSql);
    }
    else
    {
        auto FuncTmp = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)
        {
            if (FileUrlList.empty() || (FileUrlList.size() < (1 + uiRowNum)))
            {
                InteractiveProtoHandler::FileUrl urlTmp;
                FileUrlList.push_back(std::move(urlTmp));
            }

            InteractiveProtoHandler::FileUrl &fileUrl = FileUrlList.back();

            switch (uiColumnNum)
            {
            case 0:
                fileUrl.m_strFileID = strColumn;
                break;
            case 1:
                fileUrl.m_strDownloadUrl = strColumn;
                break;
            default:
                LOG_ERROR_RLD("FileUrlSqlCB error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
                break;
            }
        };

        if (!m_pMysql->QueryExec(strSql, FuncTmp))
        {
            LOG_ERROR_RLD("Download file failed and user id is " << strUserID);
            return false;
        }

        if (FileUrlList.empty())
        {
            LOG_INFO_RLD("DownloadFileToDB result is empty and user id is " << strUserID);
            return true;
        }

        boost::shared_ptr<std::list<InteractiveProtoHandler::FileUrl> > pFileUrlList(new std::list<InteractiveProtoHandler::FileUrl>);
        for (auto fileUrl : FileUrlList)
        {
            pFileUrlList->push_back(fileUrl);
        }

        boost::shared_ptr<std::list<boost::any> > pResultList(new std::list<boost::any>);
        pResultList->push_back(pFileUrlList);

        if (IsNeedCache)
        {
            m_DBCache.SetResult(strSql, pResultList);
        }

        LOG_INFO_RLD("Download file get result from db and sql is " << strSql);
    }

    return true;
}

bool AccessManager::QueryFileToDB(const std::string &strUserID, const std::string &strDevID, std::list<InteractiveProtoHandler::File> &FileInfoList,
    unsigned int uiBusinessType, const std::string &strBeginDate, const std::string &strEndDate,
    const unsigned int uiBeginIndex, const unsigned int uiPageSize, const bool IsNeedCache)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len;

    const char* sqlfmt = "select fileid, userid, deviceid, downloadurl, filename, suffixname, filesize, businesstype, filecreatedate"
        " from t_file_info where userid = '%s' and status = 0";
    snprintf(sql, size, sqlfmt, strUserID.c_str());

    if (!strDevID.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, " and deviceid = '%s'", strDevID.c_str());
    }

    if (0xFFFFFFFF != uiBusinessType)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, " and businesstype = %d", uiBusinessType);
    }

    if (!strBeginDate.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, " and unix_timestamp(filecreatedate) >= unix_timestamp('%s')", strBeginDate.c_str());
    }

    if (!strEndDate.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, " and unix_timestamp(filecreatedate) <= unix_timestamp('%s')", strEndDate.c_str());
    }

    len = strlen(sql);
    snprintf(sql + len, size - len, " limit %u, %u", uiBeginIndex, uiPageSize);
    std::string strSql(sql);

    std::list<boost::any> ResultList;
    if (IsNeedCache && m_DBCache.GetResult(strSql, ResultList) && !ResultList.empty())
    {
        boost::shared_ptr<std::list<InteractiveProtoHandler::File> > pFileInfoList;
        pFileInfoList = boost::any_cast<boost::shared_ptr<std::list<InteractiveProtoHandler::File> >>(ResultList.front());

        for (auto fileInfo : *pFileInfoList)
        {
            FileInfoList.push_back(fileInfo);
        }

        LOG_INFO_RLD("Query file get result from cache and sql is " << strSql);
    }
    else
    {
        if (!m_pMysql->QueryExec(strSql, boost::bind(&AccessManager::FileInfoSqlCB, this, _1, _2, _3, &FileInfoList)))
        {
            LOG_ERROR_RLD("Query file failed and user id is " << strUserID);
            return false;
        }

        if (FileInfoList.empty())
        {
            LOG_INFO_RLD("Query file result is empty and user id is " << strUserID);
            return true;
        }

        boost::shared_ptr<std::list<InteractiveProtoHandler::File> > pFileInfoList(new std::list<InteractiveProtoHandler::File>);

        for (auto fileInfo : FileInfoList)
        {
            pFileInfoList->push_back(fileInfo);
        }

        boost::shared_ptr<std::list<boost::any> > pResultList(new std::list<boost::any>);
        pResultList->push_back(pFileInfoList);

        m_DBCache.SetResult(strSql, pResultList);

        LOG_INFO_RLD("Query file get result from db and sql is " << strSql);
    }

    return true;
}

bool AccessManager::QueryOwnerUserIDByDeviceID(const std::string &strDevID, std::string &strUserID)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "select rel.userid from"
        " t_user_info usr, t_device_info dev, t_user_device_relation rel"
        " where usr.userid = rel.userid and usr.status = 0 and"
        " dev.deviceid = '%s' and dev.id = rel.devicekeyid and dev.status = 0 and"
        " rel.relation = %d and rel.status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strDevID.c_str(), RELATION_OF_OWNER);
    std::string strSql(sql);

    strUserID.clear();

    auto FuncTmp = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)
    {
        strUserID = strColumn;
        LOG_INFO_RLD("Query user id by device id from db is " << strUserID);
    };

    if (!m_pMysql->QueryExec(strSql, FuncTmp))
    {
        LOG_ERROR_RLD("Query user id by device id failed and device id is " << strDevID);
        return false;
    }

    if (strUserID.empty())
    {
        LOG_INFO_RLD("Query user id by device id is empty and device id is " << strDevID);
    }

    return true;
}

bool AccessManager::InsertFileToDB(const InteractiveProtoHandler::File &FileInfo)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "insert into t_file_info ("
        "id, fileid, userid, deviceid, remotefileid, downloadurl, filename, suffixname, filesize, businesstype, filecreatedate, createdate, status, extend)"
        " values(uuid(), '%s', '%s', '%s', '%s', '%s', '%s', '%s', %ld, %d, '%s', '%s', %d, '%s')";

    snprintf(sql, sizeof(sql), sqlfmt, FileInfo.m_strFileID.c_str(), FileInfo.m_strUserID.c_str(), FileInfo.m_strDevID.c_str(), FileInfo.m_strRemoteFileID.c_str(),
        FileInfo.m_strDownloadUrl.c_str(), FileInfo.m_strFileName.c_str(), FileInfo.m_strSuffixName.c_str(), FileInfo.m_ulFileSize, FileInfo.m_uiBusinessType,
        FileInfo.m_strFileCreatedate.c_str(), FileInfo.m_strCreatedate.c_str(), FileInfo.m_uiStatus, FileInfo.m_strExtend.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Insert t_file_info sql exec failed, sql is " << sql);
        return false;
    }

    return true;
}

bool AccessManager::IsUserPasswordValid(const std::string &strUserID, const std::string &strUserPassword)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select count(id) from t_user_info where userid = '%s' and userpassword = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), strUserPassword.c_str());

    unsigned int uiResult;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        uiResult = boost::lexical_cast<unsigned int>(strColumn);
        Result = uiResult;

        LOG_INFO_RLD("Query whether password is valid sql count(id) is " << uiResult);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("IsUserPasswordValid sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("The user password is invalid, user id is " << strUserID << " and password is " << strUserPassword);
        return false;
    }

    auto result = boost::any_cast<unsigned int>(ResultList.front());
    uiResult = result;
    
    if (uiResult > 0)
    {
        LOG_INFO_RLD("The user password is valid, user id is " << strUserID);
        return true;
    }
    else
    {
        LOG_ERROR_RLD("The user password is invalid, user id is " << strUserID << " and password is " << strUserPassword);
        return false;
    }
}

void AccessManager::AddNoOwnerFile(const std::string &strUserID, const std::string &strDevID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select id from t_file_info where deviceid = '%s' and (userid is null or userid = '') and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strDevID.c_str());

    std::list <std::string> strIDList;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        strIDList.push_back(strColumn);
        Result = strIDList;
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("AddNoOwnerFile sql failed, sql is " << sql);
        return;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("AddNoOwnerFile sql result is empty, sql is " << sql);
        return;
    }

    auto result = boost::any_cast<std::list <std::string>>(ResultList.front());
    strIDList = result;

    if (strIDList.empty())
    {
        LOG_INFO_RLD("Query no owner file is empty, device id is " << strDevID);
        return;
    }

    m_DBRuner.Post(boost::bind(&AccessManager::UpdateFileUserIDToDB, this, strUserID, strIDList));
}

void AccessManager::UpdateFileUserIDToDB(const std::string &strUserID, std::list<std::string> &strIDList)
{
    if (strIDList.empty())
    {
        LOG_INFO_RLD("Update id list is empty, user id is " << strUserID);
        return;
    }

    char sql[1024] = { 0 };
    const char *sqlfmt = "update t_file_info set userid = '%s' where id in (";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str());
    std::string strSql(sql);

    for (std::string id : strIDList)
    {
        char cTmp[256] = { 0 };
        snprintf(cTmp, sizeof(cTmp), "'%s',", id.c_str());
        strSql.append(cTmp);
    }

    strSql.replace(strSql.length() - 1, 1, std::string(")"));

    if (!m_pMysql->QueryExec(strSql))
    {
        LOG_ERROR_RLD("Update t_file_info user id sql exec failed, sql is " << sql);
        return;
    }
}

bool AccessManager::CheckEmailByUserName(const std::string &strUserName, const std::string &strEmail)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select count(id) from t_user_info where username = '%s' and email = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strUserName.c_str(), strEmail.c_str());
    
    unsigned int uiResult;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        uiResult = boost::lexical_cast<unsigned int>(strColumn);
        Result = uiResult;

        LOG_INFO_RLD("Check email by user name sql count(id) is " << uiResult);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("CheckEmailByUserName sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("The user name or email is invalid, user name is " << strUserName << " and email is " << strEmail);
        return false;
    }

    auto result = boost::any_cast<unsigned int>(ResultList.front());
    uiResult = result;

    if (uiResult > 0)
    {
        LOG_INFO_RLD("The user email is valid, user name is " << strUserName << " and email is " << strEmail);
        return true;
    }
    else
    {
        LOG_ERROR_RLD("The user name or email is invalid, user name is " << strUserName << " and email is " << strEmail);
        return false;
    }
}

void AccessManager::ResetUserPasswordToDB(const std::string &strUserName, const std::string &strUserPassword)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "update t_user_info set userpassword = '%s' where username = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strUserPassword.c_str(), strUserName.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Update t_user_info sql exec failed, sql is " << sql);
    }
}

void AccessManager::SendUserResetPasswordEmail(const std::string &strUserName, const std::string &strUserPassword, const std::string &strEmail)
{
    char cmd[1024] = { 0 };
    const char *param = "./mail.sh '%s' '%s' '%s'";
    snprintf(cmd, sizeof(cmd), param, strEmail.c_str(), strUserName.c_str(), strUserPassword.c_str());

    system(cmd);

    LOG_INFO_RLD("User reset password email has been sended, email address is " << strEmail << " and user name is " << strUserName);
}

bool AccessManager::QueryAccessDomainInfoByArea(const std::string &strCountryID, const std::string &strAreaID, AccessDomainInfo &DomainInfo)
{
    std::string strKey = strCountryID;
    if (!strAreaID.empty())
    {
        strKey += "|" + strAreaID;
    }

    auto itPos = m_AreaDomainMap.find(strKey);
    if (itPos == m_AreaDomainMap.end())
    {
        std::string strDefaultCountryID = "CN";
        //std::string strDefaultAreaID = "800000";
        //strKey = strDefaultCountryID + "|" + strDefaultAreaID;
        strKey = strDefaultCountryID;

        LOG_INFO_RLD("QueryAccessDomainInfoByArea not found, use default domain name.");
        itPos = m_AreaDomainMap.find(strKey);
        if (itPos == m_AreaDomainMap.end())
        {
            LOG_ERROR_RLD("QueryAccessDomainInfoByArea default domain name not found.");
            return false;
        }
    }

    std::list<AccessDomainInfo> DomainInfoList = itPos->second;

    int size = DomainInfoList.size();
    if (size < 1)
    {
        LOG_ERROR_RLD("QueryAccessDomainInfoByArea not found, country id is " << strCountryID << " and area id is " << strAreaID);
        return false;
    }
    else if (size == 1)
    {
        AccessDomainInfo domainInfo = DomainInfoList.front();
        DomainInfo.strDomainName = domainInfo.strDomainName;
        DomainInfo.uiLease = domainInfo.uiLease;
        
        return true;
    }
    else
    {
        //若同一地区配置多个域名，则使用随机的一个域名登录
        srand((unsigned)time(NULL));
        int nNum = rand() % size;

        int i = 0;
        auto itBegin = DomainInfoList.begin();
        auto itEnd = DomainInfoList.end();
        while (itBegin != itEnd)
        {
            if (i == nNum)
            {
                DomainInfo.strDomainName = itBegin->strDomainName;
                DomainInfo.uiLease = itBegin->uiLease;
                return true;
            }

            i++;
            itBegin++;
        }

        LOG_ERROR_RLD("QueryAccessDomainInfoByArea failed, random number is out of boundary, country id is " << strCountryID << " and area id is " << strAreaID);
        return false;
    }
}

bool AccessManager::InitDefaultAccessDomainName()
{
    std::string strSql = "select countryid, areaid, domainname, leaseduration from t_access_domain_info where status = 0";

    std::string strCountryID;
    std::string strAreaID;
    std::string strDomainName;
    unsigned int uiLease;
    auto FuncTmp = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)
    {
        switch (uiColumnNum)
        {
        case 0:
            strCountryID = strColumn;
            break;
        case 1:
            strAreaID = strColumn;
            break;
        case 2:
            strDomainName = strColumn;
            break;
        case 3:
        {
            uiLease = boost::lexical_cast<unsigned int>(strColumn);

            std::string strKey = strCountryID;
            if (!strAreaID.empty())
            {
                strKey += "|" + strAreaID;
            }

            auto itPos = m_AreaDomainMap.find(strKey);
            if (itPos == m_AreaDomainMap.end())
            {
                AccessDomainInfo domainInfo;
                domainInfo.strDomainName = strDomainName;
                domainInfo.uiLease = uiLease;

                std::list<AccessDomainInfo> domainInfoList;
                domainInfoList.push_back(domainInfo);

                m_AreaDomainMap.insert(make_pair(strKey, domainInfoList));
            }
            else
            {
                AccessDomainInfo domainInfo;
                domainInfo.strDomainName = strDomainName;
                domainInfo.uiLease = uiLease;

                std::list<AccessDomainInfo> &domainInfoList = itPos->second;
                domainInfoList.push_back(domainInfo);
            }

            break;
        }
        default:
            LOG_ERROR_RLD("Access domain name info SqlCB error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }
    };

    if (!m_pMysql->QueryExec(strSql, FuncTmp))
    {
        LOG_ERROR_RLD("InitDefaultAccessDomainName sql failed, sql is " << strSql);
        return false;
    }

    return true;
}

bool AccessManager::GetTimeZone(const std::string &strIpAddress, std::string &strCountryCode, std::string &strCountryNameEn,
    std::string &strCountryNameZh, std::string &strTimeZone)
{
    TimeZone timeZone;
    CTimeZone cTimeZone;
    cTimeZone.setpostUrl(GET_IPINFO_SITE);
    cTimeZone.SetDBManager(&m_DBCache, m_pMysql);
    if (!cTimeZone.GetCountryTime(strIpAddress, timeZone))
    {
        LOG_ERROR_RLD("Query timezone info failed, and ip is " << strIpAddress);
        return false;
    }

    strCountryCode = timeZone.sCode;
    strCountryNameEn = timeZone.sCountryEn;
    strCountryNameZh = timeZone.sCountryCn;
    strTimeZone = timeZone.sCountrySQ;

    return true;
}

void AccessManager::InsertUserToDB(const InteractiveProtoHandler::User &UsrInfo)
{
    
    char sql[1024] = { 0 };
    const char* sqlfmt = "insert into t_user_info("
        "id,userid,username, userpassword, typeinfo, aliasname, email, createdate, status, extend) values(uuid(),"        
        " '%s', '%s', '%s', '%d', '%s', '%s', '%s', '%d','%s')";
    snprintf(sql, sizeof(sql), sqlfmt, UsrInfo.m_strUserID.c_str(), UsrInfo.m_strUserName.c_str(), UsrInfo.m_strUserPassword.c_str(), UsrInfo.m_uiTypeInfo, 
        UsrInfo.m_strAliasName.c_str(), UsrInfo.m_strEmail.c_str(), UsrInfo.m_strCreatedate.c_str(), UsrInfo.m_uiStatus, UsrInfo.m_strExtend.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Insert t_user_info sql exec failed, sql is " << sql);        
    }
}

void AccessManager::UpdateUserInfoToDB(const InteractiveProtoHandler::User &UsrInfo)
{
    std::list<std::string> strItemList;
    if (!UsrInfo.m_strUserPassword.empty())
    {
        char sql[1024] = { 0 };
        const char *sqlfmt = "userpassword = '%s'";
        snprintf(sql, sizeof(sql), sqlfmt, UsrInfo.m_strUserPassword.c_str());
        strItemList.push_back(sql);
    }

    if (0xFFFFFFFF != UsrInfo.m_uiTypeInfo)
    {
        char sql[1024] = { 0 };
        const char *sqlfmt = "typeinfo = %d";
        snprintf(sql, sizeof(sql), sqlfmt, UsrInfo.m_uiTypeInfo);
        strItemList.push_back(sql);
    }

    if (!UsrInfo.m_strAliasName.empty())
    {
        char sql[1024] = { 0 };
        const char *sqlfmt = "aliasname = '%s'";
        snprintf(sql, sizeof(sql), sqlfmt, UsrInfo.m_strAliasName.c_str());
        strItemList.push_back(sql);
    }

    if (!UsrInfo.m_strEmail.empty())
    {
        char sql[1024] = { 0 };
        const char *sqlfmt = "email = '%s'";
        snprintf(sql, sizeof(sql), sqlfmt, UsrInfo.m_strEmail.c_str());
        strItemList.push_back(sql);
    }

    if (!UsrInfo.m_strExtend.empty())
    {
        char sql[1024] = { 0 };
        const char *sqlfmt = "extend = '%s'";
        snprintf(sql, sizeof(sql), sqlfmt, UsrInfo.m_strExtend.c_str());
        strItemList.push_back(sql);
    }

    if (strItemList.empty())
    {
        LOG_INFO_RLD("No need to update user info to db.");
        return;
    }

    std::string strTmp;
    auto itBegin = strItemList.begin();
    auto itEnd = strItemList.end();
    while (itBegin != itEnd)
    {

        strTmp += *itBegin;

        auto itTmp = itBegin;
        ++itTmp;
        if (itTmp != itEnd)
        {
            strTmp += ", ";
        }
        
        ++itBegin;
    }
        
    char sql[1024] = { 0 };
    const char *sqlfmt = " where userid = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, UsrInfo.m_strUserID.c_str());
    std::string strWhereSql = sql;
    std::string strBeginSql = "update t_user_info set ";
    std::string strSql = strBeginSql + strTmp + strWhereSql;

    if (!m_pMysql->QueryExec(strSql))
    {
        LOG_ERROR_RLD("Update t_user_info sql exec failed, sql is " << strSql);
    }
}

void AccessManager::UnregisterUserToDB(const std::string &strUserID, const int iStatus)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "update t_user_info set status = '%d' where userid = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, iStatus, strUserID.c_str());
    
    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Update t_user_info sql exec failed, sql is " << sql);
    }

}

bool AccessManager::QueryRelationExist(const std::string &strUserID, const std::string &strDevID, const int iRelation, bool &blExist, const bool IsNeedCache)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "select count(id) from t_user_device_relation where userid = '%s' and deviceid = '%s' and relation = %d and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), strDevID.c_str(), iRelation);
    std::string strSql = sql;

    std::list<boost::any> ResultList;
    if (IsNeedCache && m_DBCache.GetResult(strSql, ResultList) && !ResultList.empty())
    {
        unsigned int uiResult = boost::any_cast<unsigned int>(ResultList.front());
        blExist = 0 < uiResult;

        LOG_INFO_RLD("Query relation exist get result from cache and sql is " << strSql << " and result is " << uiResult);
    }
    else
    {
        unsigned int uiResult = 0;
        auto FuncTmp = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)//, unsigned int &uiResult)
        {
            uiResult = boost::lexical_cast<unsigned int>(strColumn);
            LOG_INFO_RLD("The relation count number that from db  is " << uiResult);
        };

        if (!m_pMysql->QueryExec(strSql, FuncTmp)) //boost::bind(FuncTmp, _1, _2, _3, uiResult)))
        {
            LOG_ERROR_RLD("Query relation failed and user id is " << strUserID << " and device id is " << strDevID  << " and relation is " << iRelation);
            return false;
        }

        boost::shared_ptr<std::list<boost::any> > pResultList(new std::list<boost::any>);
        pResultList->push_back(uiResult);

        blExist = 0 < uiResult;

        if (IsNeedCache)
        {
            m_DBCache.SetResult(strSql, pResultList);
        }

        LOG_INFO_RLD("Query relation exist get result from db and sql is " << strSql << " and result is " << uiResult);
    }

    return true;
}

bool AccessManager::QueryRelationByUserID(const std::string &strUserID, std::list<InteractiveProtoHandler::Relation> &RelationList,
    const unsigned int uiBeginIndex, const unsigned int uiPageSize)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "select rel.userid, rel.deviceid, rel.relation, rel.begindate, rel.enddate, rel.createdate, rel.status, rel.extend"
        " from t_device_info dev, t_user_device_relation rel, t_user_info usr"
        " where dev.id = rel.devicekeyid and usr.userid = rel.userid and rel.userid = '%s' and rel.status = 0 and dev.status = 0 and usr.status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str());

    std::string strSql;
    char cTmp[128] = { 0 };
    snprintf(cTmp, sizeof(cTmp), " limit %u, %u", uiBeginIndex, uiPageSize);
    strSql = std::string(sql) + std::string(cTmp);
    
    std::list<boost::any> ResultList;
    if (m_DBCache.GetResult(strSql, ResultList) && !ResultList.empty())
    {
        boost::shared_ptr<std::list<InteractiveProtoHandler::Relation> > pRelationList;
        pRelationList = boost::any_cast<boost::shared_ptr<std::list<InteractiveProtoHandler::Relation> > >(ResultList.front());

        auto itBegin = pRelationList->begin();
        auto itEnd = pRelationList->end();
        while (itBegin != itEnd)
        {
            RelationList.push_back(*itBegin);
            ++itBegin;
        }

        LOG_INFO_RLD("Query relation get result from cache and sql is " << strSql);
    }
    else
    {        
        if (!m_pMysql->QueryExec(strSql, boost::bind(&AccessManager::DevInfoRelationSqlCB, this, _1, _2, _3, &RelationList)))
        {
            LOG_ERROR_RLD("Query relation failed and user id is " << strUserID);
            return false;
        }

        if (RelationList.empty())
        {
            LOG_INFO_RLD("QueryRelationByUserID result is empty and user id is " << strUserID);
            return true;
        }
        

        boost::shared_ptr<std::list<InteractiveProtoHandler::Relation> > pRelationList(new std::list<InteractiveProtoHandler::Relation>);
        auto itBeginRel = RelationList.begin();
        auto itEndRel = RelationList.end();
        while (itBeginRel != itEndRel)
        {
            pRelationList->push_back(*itBeginRel);
            ++itBeginRel;
        }

        boost::shared_ptr<std::list<boost::any> > pResultList(new std::list<boost::any>);
        pResultList->push_back(pRelationList);

        m_DBCache.SetResult(strSql, pResultList);

        LOG_INFO_RLD("Query relation get result from db and sql is " << strSql);
    }

    return true;
}

bool AccessManager::QueryRelationByDevID(const std::string &strDevID, std::list<InteractiveProtoHandler::Relation> &RelationList,
    const unsigned int uiBeginIndex, const unsigned int uiPageSize)
{
    char sql[1024] = { 0 };
    memset(sql, 0, sizeof(sql));
    const char *sqlft = "select rel.userid, rel.deviceid, rel.relation, rel.begindate, rel.enddate, rel.createdate, rel.status, rel.extend"
        " from t_device_info dev, t_user_device_relation rel, t_user_info usr"
        " where dev.id = rel.devicekeyid and usr.userid = rel.userid and rel.deviceid = '%s' and rel.status = 0 and dev.status = 0 and usr.status = 0";
    snprintf(sql, sizeof(sql), sqlft, strDevID.c_str());

    std::string strSql;
    char cTmp[128] = { 0 };
    snprintf(cTmp, sizeof(cTmp), " limit %u, %u", uiBeginIndex, uiPageSize);
    strSql = std::string(sql) + std::string(cTmp);

    std::list<boost::any> ResultList;
    if (m_DBCache.GetResult(strSql, ResultList) && !ResultList.empty())
    {
        boost::shared_ptr<std::list<InteractiveProtoHandler::Relation> > pRelationList;
        pRelationList = boost::any_cast<boost::shared_ptr<std::list<InteractiveProtoHandler::Relation> >>(ResultList.front());

        auto itBegin = pRelationList->begin();
        auto itEnd = pRelationList->end();
        while (itBegin != itEnd)
        {
            RelationList.push_back(*itBegin);
            ++itBegin;
        }

        LOG_INFO_RLD("Query relation by device id get result from cache and sql is " << strSql);
    }
    else
    {
        std::list<InteractiveProtoHandler::Relation> *pRelationList = &RelationList;
        auto FuncTmp = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)//, unsigned int &uiResult)
        {
            if (pRelationList->empty() || (pRelationList->size() < (1 + uiRowNum)))
            {
                InteractiveProtoHandler::Relation usrInfoTmp;
                pRelationList->push_back(std::move(usrInfoTmp));
            }

            InteractiveProtoHandler::Relation &relationInfo = pRelationList->back();

            switch (uiColumnNum)
            {
            case 0:
                relationInfo.m_strUserID = strColumn;
                break;
            case 1:
                relationInfo.m_strDevID = strColumn;
                break;
            case 2:
                relationInfo.m_uiRelation = boost::lexical_cast<unsigned int>(strColumn);
                break;
            case 3:
                relationInfo.m_strBeginDate = strColumn;
                break;
            case 4:
                relationInfo.m_strEndDate = strColumn;
                break;
            case 5:

                break;
            case 6:

                break;
            case 7:
                relationInfo.m_strValue = strColumn;
                break;
            default:
                LOG_ERROR_RLD("DevInfoSqlCB error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
                break;
            }
        };
        
        if (!m_pMysql->QueryExec(strSql, FuncTmp))
        {
            LOG_ERROR_RLD("Query relation by device id failed and device id is " << strDevID);
            return false;
        }

        if (RelationList.empty())
        {
            LOG_INFO_RLD("QueryRelationByDevID result is empty and device id is " << strDevID);
            return true;
        }
        
        boost::shared_ptr<std::list<InteractiveProtoHandler::Relation> > pRelationListTmp(new std::list<InteractiveProtoHandler::Relation>);
        auto itBeginRel = RelationList.begin();
        auto itEndRel = RelationList.end();
        while (itBeginRel != itEndRel)
        {
            pRelationListTmp->push_back(*itBeginRel);
            ++itBeginRel;
        }

        boost::shared_ptr<std::list<boost::any> > pResultList(new std::list<boost::any>);
        pResultList->push_back(pRelationListTmp);

        m_DBCache.SetResult(strSql, pResultList);

        LOG_INFO_RLD("Query relation by device id get result from db and sql is " << strSql);
    }

    return true;
}

bool AccessManager::ValidUser(std::string &strUserID, std::string &strUserName, const std::string &strUserPwd, const int iTypeInfo, const bool IsForceFromDB)
{
    //Valid user id
    char sql[1024] = { 0 };
    const char* sqlfmt = !strUserID.empty() ? "select userid,username, userpassword, typeinfo, createdate, status, extend from t_user_info where userid = '%s' and status = 0"
        : "select userid,username, userpassword, typeinfo, createdate, status, extend from t_user_info where username = '%s' and status = 0";

    snprintf(sql, sizeof(sql), sqlfmt, !strUserID.empty() ? strUserID.c_str() : strUserName.c_str());

    std::list<boost::any> ResultList;
    if (IsForceFromDB)
    {
        auto FuncTmp = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)
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

            ResultList.push_back(value);
        };

        if (!m_pMysql->QueryExec(std::string(sql), FuncTmp))
        {
            LOG_ERROR_RLD("Valid user query sql failed, sql is " << sql);
            return false;
        }
    }
    else
    {
        if (!m_DBCache.QuerySql(std::string(sql), ResultList, NULL, IsForceFromDB))
        {
            LOG_ERROR_RLD("Valid user query sql failed, sql is " << sql);
            return false;
        }
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("Valid user info not found, sql is " << sql);
        return false;
    }

    if (strUserPwd.empty())
    {
        LOG_INFO_RLD("Valid user success and sql is " << sql);
        return true;
    }

    {
        std::string strType = strUserID.empty() ? "userid" : "username";
        std::string &strValue = strUserID.empty() ? strUserID : strUserName;
        auto itBegin = ResultList.begin();
        auto itEnd = ResultList.end();
        while (itBegin != itEnd)
        {
            auto Result = boost::any_cast<ValueInDB>(*itBegin);
            if (Result.strType == strType)
            {
                strValue = Result.strValue;
                break;
            }           

            ++itBegin;
        }
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

    LOG_INFO_RLD("Valid user success and user id is " << strUserID << " and user name is " << strUserName << " and user password is " << strUserPwd);

    return true;
}

bool AccessManager::GetMySqlUUID(std::string &strUuid)
{
    std::string strSql = "select uuid()";
    auto FuncTmp = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)
    {
        strUuid = strColumn;
        LOG_INFO_RLD("Mysql uuid from db is " << strUuid);
    };

    if (!m_pMysql->QueryExec(strSql, FuncTmp))
    {
        LOG_ERROR_RLD("Get mysql uuid sql exec failed, sql is " << strSql);
        return false;
    }

    return true;
}

void AccessManager::UserInfoSqlCB(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
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

void AccessManager::DevInfoRelationSqlCB(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, 
    std::list<InteractiveProtoHandler::Relation> *pRelationList)
{
    //select dev.deviceid, dev.devicename, dev.devicepassword, dev.typeinfo, dev.createdate, dev.status, dev.innerinfo, dev.extend        
    if (pRelationList->empty() || (pRelationList->size() < (1 + uiRowNum)))
    {
        InteractiveProtoHandler::Relation devInfoTmp;
        pRelationList->push_back(std::move(devInfoTmp));
    }
    
    InteractiveProtoHandler::Relation &relationInfo = pRelationList->back();

    switch (uiColumnNum)
    {
    case 0:
        relationInfo.m_strUserID = strColumn;
        break;
    case 1:
        relationInfo.m_strDevID = strColumn;
        break;
    case 2:
        relationInfo.m_uiRelation = boost::lexical_cast<unsigned int>(strColumn);
        break;
    case 3:
        relationInfo.m_strBeginDate = strColumn;
        break;
    case 4:
        relationInfo.m_strEndDate = strColumn;
        break;
    case 5:

        break;
    case 6:

        break;
    case 7:
        relationInfo.m_strValue = strColumn;
        break;
    default:
        LOG_ERROR_RLD("DevInfoSqlCB error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
        break;
    }
}

void AccessManager::FileInfoSqlCB(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn,
    std::list<InteractiveProtoHandler::File> *pFileInfoList)
{
    //select fileid, userid, deviceid, filename, suffixname, filesize, businesstype, filecreatedate from t_file_info
    if (pFileInfoList->empty() || (pFileInfoList->size() < (1 + uiRowNum)))
    {
        InteractiveProtoHandler::File fileInfoTmp;
        pFileInfoList->push_back(std::move(fileInfoTmp));
    }

    InteractiveProtoHandler::File &fileInfo = pFileInfoList->back();

    switch (uiColumnNum)
    {
    case 0:
        fileInfo.m_strFileID = strColumn;
        break;
    case 1:
        fileInfo.m_strUserID = strColumn;
        break;
    case 2:
        fileInfo.m_strDevID = strColumn;
        break;
    case 3:
        fileInfo.m_strDownloadUrl = strColumn;
        break;
    case 4:
        fileInfo.m_strFileName = strColumn;
        break;
    case 5:
        fileInfo.m_strSuffixName = strColumn;
        break;
    case 6:
        fileInfo.m_ulFileSize = boost::lexical_cast<unsigned long int>(strColumn);
        break;
    case 7:
        fileInfo.m_uiBusinessType = boost::lexical_cast<unsigned int>(strColumn);
    case 8:
        fileInfo.m_strFileCreatedate = strColumn;
        break;
    default:
        LOG_ERROR_RLD("DevInfoSqlCB error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
        break;
    }
}


void AccessManager::SessionTimeoutProcessCB(const std::string &strSessionID)
{
        
    //广播消息表示用户会话超时


    LOG_INFO_RLD("Session timeout and session id is " << strSessionID);
}

void AccessManager::InsertDeviceToDB(const std::string &strUuid, const InteractiveProtoHandler::Device &DevInfo)
{
    //这里考虑到设备内部信息可能不一定是可打印字符，为了后续日志打印和维护方便，这里就将其内容文本化之后再存储到数据库中
    const std::string &strInner = DevInfo.m_strInnerinfo.empty() ? 
        DevInfo.m_strInnerinfo : Encode64((const unsigned char *)DevInfo.m_strInnerinfo.c_str(), DevInfo.m_strInnerinfo.size());

    char sql[1024] = { 0 };
    const char* sqlfmt = "insert into t_device_info("
        "id,deviceid,devicename, devicepassword, typeinfo, createdate, status, innerinfo, extend) values('%s',"
        "'%s','%s','%s','%d','%s', '%d','%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strUuid.c_str(), DevInfo.m_strDevID.c_str(), DevInfo.m_strDevName.c_str(), DevInfo.m_strDevPassword.c_str(), DevInfo.m_uiTypeInfo,
        DevInfo.m_strCreatedate.c_str(), DevInfo.m_uiStatus, strInner.c_str(), DevInfo.m_strExtend.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Insert t_device_info sql exec failed, sql is " << sql);
    }
}

void AccessManager::InsertRelationToDB(const std::string &strUuid, const RelationOfUsrAndDev &relation)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "insert into t_user_device_relation("
        "id, userid, deviceid, relation, devicekeyid, begindate, enddate, createdate, status, extend) values(uuid(),"
        "'%s','%s', '%d', '%s', '%s', '%s', '%s','%d', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, relation.m_strUsrID.c_str(), relation.m_strDevID.c_str(), relation.m_iRelation, strUuid.c_str(),
        relation.m_strBeginDate.c_str(), relation.m_strEndDate.c_str(), relation.m_strCreateDate.c_str(), relation.m_iStatus, relation.m_strExtend.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Insert t_user_device_relation sql exec failed, sql is " << sql);
    }
}

void AccessManager::RemoveRelationToDB(const RelationOfUsrAndDev &relation)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "update t_user_device_relation set status = %d where userid = '%s' and deviceid = '%s' and relation = %d and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, DELETE_STATUS, relation.m_strUsrID.c_str(), relation.m_strDevID.c_str(), relation.m_iRelation);

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Update t_user_device_relation sql exec failed, sql is " << sql);
    }

}

void AccessManager::DelDeviceToDB(const std::list<std::string> &strDevIDList, const int iStatus)
{
    if (strDevIDList.empty())
    {
        LOG_ERROR_RLD("Delete device id list is empty.");
        return;
    }

    //"update t_device_info set status = '%d' where deviceid = '%s'";

    std::string strSql;
    char sql[1024] = { 0 };
    const char* sqlfmt = "update t_device_info set status = '%d' where ";
    snprintf(sql, sizeof(sql), sqlfmt, iStatus);
    strSql = sql;

    auto itBegin = strDevIDList.begin();
    auto itEnd = strDevIDList.end();
    while (itBegin != itEnd)
    {
        std::string strTmp;
        char cTmp[256] = { 0 };
        snprintf(cTmp, sizeof(cTmp), "deviceid = '%s'", itBegin->c_str());
        strTmp = cTmp;

        auto itTmp = itBegin;
        ++itTmp;
        if (itTmp != itEnd)
        {
            strTmp += " or ";
        }

        strSql += strTmp;

        ++itBegin;
    }
    
    if (!m_pMysql->QueryExec(strSql))
    {
        LOG_ERROR_RLD("Delete t_device_info sql exec failed, sql is " << strSql);
    }
}

void AccessManager::ModDeviceToDB(const InteractiveProtoHandler::Device &DevInfo)
{
    //注意，只有给Device对象的字段赋过值的才需要更新到数据库中。
    //"update t_device_info set status = '%d' where deviceid = '%s'";

    if (DevInfo.m_strDevID.empty())
    {
        LOG_INFO_RLD("No need to update device info to db because device id is empty.");
        return;
    }

    std::string strTmp;
    std::string strSql;

    if (!DevInfo.m_strDevName.empty())
    {        
        char cTmp[256] = { 0 };
        snprintf(cTmp, sizeof(cTmp), " devicename = '%s' ", DevInfo.m_strDevName.c_str());
        strSql = cTmp;

    }

    if (!DevInfo.m_strDevPassword.empty())
    {
        char cTmp[256] = { 0 };
        snprintf(cTmp, sizeof(cTmp), ", devicepassword = '%s' ", DevInfo.m_strDevPassword.c_str());
        strSql += cTmp;
    }

    if (DevInfo.m_uiTypeInfo != 0xFFFFFFFF)
    {
        char cTmp[256] = { 0 };
        snprintf(cTmp, sizeof(cTmp), ", typeinfo = '%u' ", DevInfo.m_uiTypeInfo);
        strSql += cTmp;
    }

    if (!DevInfo.m_strCreatedate.empty())
    {
        char cTmp[256] = { 0 };
        snprintf(cTmp, sizeof(cTmp), ", createdate = '%s' ", DevInfo.m_strCreatedate.c_str());
        strSql += cTmp;
    }

    if (DevInfo.m_uiStatus != 0xFFFFFFFF)
    {
        char cTmp[256] = { 0 };
        snprintf(cTmp, sizeof(cTmp), ", status = '%u' ", DevInfo.m_uiStatus);
        strSql += cTmp;
    }

    if (!DevInfo.m_strInnerinfo.empty())
    {
        const std::string &strInner = Encode64((const unsigned char *)DevInfo.m_strInnerinfo.c_str(), DevInfo.m_strInnerinfo.size());

        char cTmp[256] = { 0 };
        snprintf(cTmp, sizeof(cTmp), ", innerinfo = '%s' ", strInner.c_str());
        strSql += cTmp;
    }

    if (!DevInfo.m_strExtend.empty())
    {
        char cTmp[256] = { 0 };
        snprintf(cTmp, sizeof(cTmp), ", extend = '%s' ", DevInfo.m_strExtend.c_str());
        strSql += cTmp;
    }
        
    if (strSql.empty())
    {
        LOG_INFO_RLD("No need to update device info to db and device id is " << DevInfo.m_strDevID);
        return;
    }

    char cTmp[256] = { 0 };
    snprintf(cTmp, sizeof(cTmp), "where deviceid = '%s'", DevInfo.m_strDevID.c_str());
    strSql += cTmp;

    strSql = "update t_device_info set " + strSql;

    if (!m_pMysql->QueryExec(strSql))
    {
        LOG_ERROR_RLD("Insert t_device_info sql exec failed, sql is " << strSql);
    }
    
}

void AccessManager::SharingRelationToDB(const RelationOfUsrAndDev &relation)
{
    bool blExist = false;
    if (!QueryRelationExist(relation.m_strUsrID, relation.m_strDevID, relation.m_iRelation, blExist, false))
    {
        LOG_ERROR_RLD("Sharing relation failed and user id is " << relation.m_strUsrID
            << " and device id is " << relation.m_strDevID << " and relation is " << relation.m_iRelation);
        return;
    }

    if (blExist)
    {
        LOG_ERROR_RLD("Sharing relation already exist and user id is " << relation.m_strUsrID
            << " and device id is " << relation.m_strDevID << " and relation is " << relation.m_iRelation);
        return;
    }
        
    char sql[256] = { 0 };
    const char *sqlfmt = "select id from t_device_info where deviceid = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, relation.m_strDevID.c_str());

    std::string strUuid;
    auto FuncTmp = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)
    {
        strUuid = strColumn;
        LOG_INFO_RLD("The device key id from db is " << strUuid);
    };

    if (!m_pMysql->QueryExec(std::string(sql), FuncTmp))
    {
        LOG_ERROR_RLD("Query device key id failed and device id is " << relation.m_strDevID);
        return;
    }

    InsertRelationToDB(strUuid, relation);
}

void AccessManager::CancelSharedRelationToDB(const RelationOfUsrAndDev &relation)
{
    bool blExist = false;
    if (!QueryRelationExist(relation.m_strUsrID, relation.m_strDevID, relation.m_iRelation, blExist, false))
    {
        LOG_ERROR_RLD("Cancel shared relation failed and user id is " << relation.m_strUsrID
            << " and device id is " << relation.m_strDevID << " and relation is " << relation.m_iRelation);
        return;
    }

    if (!blExist)
    {
        LOG_ERROR_RLD("Cancel shared relation not exist and user id is " << relation.m_strUsrID
            << " and device id is " << relation.m_strDevID << " and relation is " << relation.m_iRelation);
        return;
    }

    RemoveRelationToDB(relation);

}

bool AccessManager::QueryUserInfoToDB(const std::string &strUserID, InteractiveProtoHandler::User &usr, const bool IsNeedCache)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "select userid, username, userpassword, typeinfo, createdate, status, extend, aliasname, email from t_user_info where userid = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str());
    std::string strSql = sql;

    std::list<boost::any> ResultList;
    if (IsNeedCache && m_DBCache.GetResult(strSql, ResultList) && !ResultList.empty())
    {
        auto Result = boost::any_cast<InteractiveProtoHandler::User>(ResultList.front());
        
        usr.m_strCreatedate = Result.m_strCreatedate;
        usr.m_strExtend = Result.m_strExtend;
        usr.m_strUserID = Result.m_strUserID;
        usr.m_strUserName = Result.m_strUserName;
        usr.m_strUserPassword = Result.m_strUserPassword;
        usr.m_uiStatus = Result.m_uiStatus;
        usr.m_uiTypeInfo = Result.m_uiTypeInfo;
        usr.m_strAliasName = Result.m_strAliasName;
        usr.m_strEmail = Result.m_strEmail;

        LOG_INFO_RLD("Query user info exist get result from cache and sql is " << strSql << " and user id is " << usr.m_strUserID);
    }
    else
    {
        
        auto FuncTmp = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)//, unsigned int &uiResult)
        {
            switch (uiColumnNum)
            {
            case 0:
                usr.m_strUserID = strColumn;
                break;
            case 1:
                usr.m_strUserName = strColumn;
                break;
            case 2:
                usr.m_strUserPassword = strColumn;
                break;
            case 3:
                usr.m_uiTypeInfo = boost::lexical_cast<unsigned int>(strColumn);
                break;
            case 4:
                usr.m_strCreatedate = strColumn;
                break;
            case 5:
                usr.m_uiStatus = boost::lexical_cast<unsigned int>(strColumn);
                break;
            case 6:
                usr.m_strExtend = strColumn;
                break;
            case 7:
                usr.m_strAliasName = strColumn;
                break;
            case 8:
                usr.m_strEmail = strColumn;
                break;
            default:
                LOG_ERROR_RLD("UserInfoSqlCB error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
                break;
            }
        };

        if (!m_pMysql->QueryExec(strSql, FuncTmp)) //boost::bind(FuncTmp, _1, _2, _3, uiResult)))
        {
            LOG_ERROR_RLD("Query user info failed and user id is " << strUserID);
            return false;
        }

        boost::shared_ptr<std::list<boost::any> > pResultList(new std::list<boost::any>);
        pResultList->push_back(usr);

        if (IsNeedCache)
        {
            m_DBCache.SetResult(strSql, pResultList);
        }

        LOG_INFO_RLD("Query user info exist get result from db and sql is " << strSql << " and user id is " << usr.m_strUserID);
    }

    return true;

}

bool AccessManager::QueryDevInfoToDB(const std::string &strDevID, InteractiveProtoHandler::Device &dev, const bool IsNeedCache /*= true*/)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "select deviceid, devicename, devicepassword, typeinfo, createdate, status, innerinfo, extend from t_device_info where deviceid = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strDevID.c_str());
    std::string strSql = sql;

    std::list<boost::any> ResultList;
    if (IsNeedCache && m_DBCache.GetResult(strSql, ResultList) && !ResultList.empty())
    {
        auto Result = boost::any_cast<InteractiveProtoHandler::Device>(ResultList.front());

        dev.m_strCreatedate = Result.m_strCreatedate;
        dev.m_strExtend = Result.m_strExtend;
        dev.m_strDevID = Result.m_strDevID;
        dev.m_strDevName = Result.m_strDevName;
        dev.m_strDevPassword = Result.m_strDevPassword;
        dev.m_uiStatus = Result.m_uiStatus;
        dev.m_uiTypeInfo = Result.m_uiTypeInfo;
        dev.m_strInnerinfo = Result.m_strInnerinfo;

        LOG_INFO_RLD("Query user info exist get result from cache and sql is " << strSql << " and device id is " << dev.m_strDevID);
    }
    else
    {

        auto FuncTmp = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)//, unsigned int &uiResult)
        {
            switch (uiColumnNum)
            {
            case 0:
                dev.m_strDevID = strColumn;
                break;
            case 1:
                dev.m_strDevName = strColumn;
                break;
            case 2:
                dev.m_strDevPassword = strColumn;
                break;
            case 3:
                dev.m_uiTypeInfo = boost::lexical_cast<unsigned int>(strColumn);
                break;
            case 4:
                dev.m_strCreatedate = strColumn;
                break;
            case 5:
                dev.m_uiStatus = boost::lexical_cast<unsigned int>(strColumn);
                break;
            case 6:
                dev.m_strInnerinfo = strColumn;
                break;
            case 7:
                dev.m_strExtend = strColumn;
                break;
            default:
                LOG_ERROR_RLD("UserInfoSqlCB error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
                break;
            }
        };

        if (!m_pMysql->QueryExec(strSql, FuncTmp)) //boost::bind(FuncTmp, _1, _2, _3, uiResult)))
        {
            LOG_ERROR_RLD("Query device info failed and device id is " << strDevID);
            return false;
        }

        boost::shared_ptr<std::list<boost::any> > pResultList(new std::list<boost::any>);
        pResultList->push_back(dev);

        if (IsNeedCache)
        {
            m_DBCache.SetResult(strSql, pResultList);
        }

        LOG_INFO_RLD("Query device info exist get result from db and sql is " << strSql << " and device id is " << dev.m_strDevID);
    }

    return true;
}

void AccessManager::AddFriendsToDB(const RelationOfUsr &relation)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "insert into t_user_relation("
        "id, userid, relation_userid, relation, createdate, status, extend) values(uuid(),"
        "'%s', '%s', '%d', '%s', '%d', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, relation.m_strUsrID.c_str(), relation.m_strRelationOfUsrID.c_str(), relation.m_iRelation,
        relation.m_strCreateDate.c_str(), relation.m_iStatus, relation.m_strExtend.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Insert t_user_relation sql exec failed, sql is " << sql);
    }
}

void AccessManager::DelFriendsToDB(const std::string &strUserID, const std::list<std::string> &FriendIDList, const int iStatus)
{
    if (FriendIDList.empty())
    {
        LOG_ERROR_RLD("Delete friends id list is empty and user id is " << strUserID);
        return;
    }

    //update t_user_relation set status = 2 where userid = 'sss' and relation_userid in ('xxx', 'mmm', 'aaw');

    std::string strSql;
    char sql[1024] = { 0 };
    const char* sqlfmt = "update t_user_relation set status = '%d' where userid = '%s' and relation_userid in (";
    snprintf(sql, sizeof(sql), sqlfmt, iStatus, strUserID.c_str());
    strSql = sql;

    auto itBegin = FriendIDList.begin();
    auto itEnd = FriendIDList.end();
    while (itBegin != itEnd)
    {
        std::string strTmp;
        char cTmp[256] = { 0 };
        snprintf(cTmp, sizeof(cTmp), "'%s'", itBegin->c_str());
        strTmp = cTmp;

        auto itTmp = itBegin;
        ++itTmp;
        if (itTmp != itEnd)
        {
            strTmp += ", ";
        }

        strSql += strTmp;

        ++itBegin;
    }

    strSql += ")";

    if (!m_pMysql->QueryExec(strSql))
    {
        LOG_ERROR_RLD("Delete t_user_relation sql exec failed, sql is " << strSql);
    }

}

bool AccessManager::QueryUserRelationExist(const std::string &strUserID, const std::string &strFriendsID, const int iRelation, bool &blExist, const bool IsNeedCache /*= true*/)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "select count(id) from t_user_relation where userid = '%s' and relation_userid = '%s' and relation = %d and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), strFriendsID.c_str(), iRelation);
    std::string strSql = sql;

    std::list<boost::any> ResultList;
    if (IsNeedCache && m_DBCache.GetResult(strSql, ResultList) && !ResultList.empty())
    {
        unsigned int uiResult = boost::any_cast<unsigned int>(ResultList.front());
        blExist = 0 < uiResult;

        LOG_INFO_RLD("Query user relation exist get result from cache and sql is " << strSql << " and result is " << uiResult);
    }
    else
    {
        unsigned int uiResult = 0;
        auto FuncTmp = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)//, unsigned int &uiResult)
        {
            uiResult = boost::lexical_cast<unsigned int>(strColumn);
            LOG_INFO_RLD("The user relation count number that from db  is " << uiResult);
        };

        if (!m_pMysql->QueryExec(strSql, FuncTmp)) //boost::bind(FuncTmp, _1, _2, _3, uiResult)))
        {
            LOG_ERROR_RLD("Query user relation failed and user id is " << strUserID << " and friends id is " << strFriendsID << " and relation is " << iRelation);
            return false;
        }

        boost::shared_ptr<std::list<boost::any> > pResultList(new std::list<boost::any>);
        pResultList->push_back(uiResult);

        blExist = 0 < uiResult;

        if (IsNeedCache)
        {
            m_DBCache.SetResult(strSql, pResultList);
        }

        LOG_INFO_RLD("Query user relation exist get result from db and sql is " << strSql << " and result is " << uiResult);
    }

    return true;
}

bool AccessManager::QueryUserRelationInfoToDB(const std::string &strUserID, const int iRelation, std::list<std::string> &strRelationIDList,
    const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 10*/, const bool IsNeedCache /*= true*/)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "select relation_userid from t_user_relation where userid = '%s' and relation = %d and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), iRelation);

    std::string strSql;
    char cTmp[128] = { 0 };
    snprintf(cTmp, sizeof(cTmp), " limit %u, %u", uiBeginIndex, uiPageSize);
    strSql = std::string(sql) + std::string(cTmp);

    std::list<boost::any> ResultList;
    if (IsNeedCache && m_DBCache.GetResult(strSql, ResultList) && !ResultList.empty())
    {
        boost::shared_ptr<std::list<std::string> > pRelationIDList;
        pRelationIDList = boost::any_cast<boost::shared_ptr<std::list<std::string> >>(ResultList.front());

        auto itBegin = pRelationIDList->begin();
        auto itEnd = pRelationIDList->end();
        while (itBegin != itEnd)
        {
            strRelationIDList.push_back(*itBegin);
            ++itBegin;
        }

        LOG_INFO_RLD("Query user relation get result from cache and sql is " << strSql);
    }
    else
    {
        auto FuncTmp = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)
        {
            strRelationIDList.push_back(strColumn);
        };

        if (!m_pMysql->QueryExec(strSql, FuncTmp))
        {
            LOG_ERROR_RLD("Query user relation failed and user id is " << strUserID);
            return false;
        }

        if (strRelationIDList.empty())
        {
            LOG_INFO_RLD("QueryUserRelationInfoToDB result is empty and user id is " << strUserID);
            return true;
        }


        boost::shared_ptr<std::list<std::string> > pRelationList(new std::list<std::string>);
        auto itBeginRel = strRelationIDList.begin();
        auto itEndRel = strRelationIDList.end();
        while (itBeginRel != itEndRel)
        {
            pRelationList->push_back(*itBeginRel);
            ++itBeginRel;
        }

        boost::shared_ptr<std::list<boost::any> > pResultList(new std::list<boost::any>);
        pResultList->push_back(pRelationList);

        if (IsNeedCache)
        {
            m_DBCache.SetResult(strSql, pResultList);
        }

        LOG_INFO_RLD("Query user relation get result from db and sql is " << strSql);
    }

    return true;

}

