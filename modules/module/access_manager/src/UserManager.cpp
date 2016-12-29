#include "UserManager.h"
#include <boost/scope_exit.hpp>
#include "CommonUtility.h"
#include "ReturnCode.h"
#include "mysql_impl.h"
#include "boost/lexical_cast.hpp"
#include "json/json.h"

const std::string UserManager::MAX_DATE = "2199-01-01 00:00:00";

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

bool UserManager::PreCommonHandler(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
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
        InteractiveProtoHandler::MsgType::RegisterUserRsp_USR_T == req.m_MsgType)
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
    UsrInfo.m_uiStatus = NORMAL_STATUS;
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
    m_DBRuner.Post(boost::bind(&UserManager::UpdateUserToDB, this, UnRegUsrReq.m_userInfo.m_strUserID, DELETE_STATUS));

    blResult = true;

        
    return blResult;
}

bool UserManager::QueryUsrInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
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

bool UserManager::LoginReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
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
        
    if (!QueryRelationByUserID(LoginReqUsr.m_userInfo.m_strUserID, RelationList))
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

    InteractiveProtoHandler::Device DevInfo;
    DevInfo.m_strDevID = req.m_devInfo.m_strDevID;
    DevInfo.m_strDevName = req.m_devInfo.m_strDevName;
    DevInfo.m_strDevPassword = req.m_devInfo.m_strDevPassword;
    DevInfo.m_uiTypeInfo = req.m_devInfo.m_uiTypeInfo;
    DevInfo.m_strCreatedate = strCurrentTime;
    DevInfo.m_uiStatus = NORMAL_STATUS;
    DevInfo.m_strInnerinfo = req.m_devInfo.m_strInnerinfo;
    DevInfo.m_strExtend = req.m_devInfo.m_strExtend;

    m_DBRuner.Post(boost::bind(&UserManager::InsertDeviceToDB, this, DevInfo));

    RelationOfUsrAndDev relation;
    relation.m_iRelation = RELATION_OF_OWNER;
    relation.m_iStatus = NORMAL_STATUS;
    relation.m_strBeginDate = strCurrentTime;
    relation.m_strEndDate = MAX_DATE;
    relation.m_strCreateDate = strCurrentTime;
    relation.m_strDevID = req.m_devInfo.m_strDevID;
    relation.m_strExtend = req.m_devInfo.m_strExtend;
    relation.m_strUsrID = req.m_strUserID;

    m_DBRuner.Post(boost::bind(&UserManager::InsertRelationToDB, this, relation));

    blResult = true;
    
    return blResult;
}

bool UserManager::DelDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
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

    
    m_DBRuner.Post(boost::bind(&UserManager::DelDeviceToDB, this, req.m_strDevIDList, DELETE_STATUS));

    blResult = true;

    return blResult;
}

bool UserManager::ModDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
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


    InteractiveProtoHandler::Device DevInfo;
    DevInfo.m_strDevID = req.m_devInfo.m_strDevID;
    DevInfo.m_strDevName = req.m_devInfo.m_strDevName;
    DevInfo.m_strDevPassword = req.m_devInfo.m_strDevPassword;
    DevInfo.m_uiTypeInfo = req.m_devInfo.m_uiTypeInfo;
    DevInfo.m_strCreatedate = req.m_devInfo.m_strCreatedate;
    DevInfo.m_uiStatus = req.m_devInfo.m_uiStatus;
    DevInfo.m_strInnerinfo = req.m_devInfo.m_strInnerinfo;
    DevInfo.m_strExtend = req.m_devInfo.m_strExtend;

    m_DBRuner.Post(boost::bind(&UserManager::ModDeviceToDB, this, DevInfo));

    blResult = true;


    return blResult;    
}

bool UserManager::QueryDevInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
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

bool UserManager::QueryDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
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

bool UserManager::QueryUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
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

bool UserManager::SharingDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
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

    m_DBRuner.Post(boost::bind(&UserManager::SharingRelationToDB, this, relation));
    

    blResult = true;
    return blResult;
}

bool UserManager::CancelSharedDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
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

    m_DBRuner.Post(boost::bind(&UserManager::CancelSharedRelationToDB, this, relation));

    blResult = true;

    return blResult;
}

bool UserManager::AddFriendsReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
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

    m_DBRuner.Post(boost::bind(&UserManager::AddFriendsToDB, this, relation));

    blResult = true;

    return blResult;
}

bool UserManager::DelFriendsReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
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
    
    m_DBRuner.Post(boost::bind(&UserManager::DelFriendsToDB, this, req.m_strUserID, req.m_strFriendUserIDList, DELETE_STATUS));
    
    blResult = true;

    return blResult;
}

bool UserManager::QueryFriendsReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
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

bool UserManager::QueryRelationExist(const std::string &strUserID, const std::string &strDevID, const int iRelation, bool &blExist, const bool IsNeedCache)
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

bool UserManager::QueryRelationByUserID(const std::string &strUserID, std::list<InteractiveProtoHandler::Relation> &RelationList,
    const unsigned int uiBeginIndex, const unsigned int uiPageSize)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "select rel.userid, rel.deviceid, rel.relation, rel.begindate, rel.enddate, rel.createdate, rel.status, rel.extend"
        " from t_device_info dev, t_user_device_relation rel, t_user_info usr"
        " where dev.deviceid = rel.deviceid and usr.userid = rel.userid and rel.userid = '%s' and rel.status = 0 and dev.status = 0 and usr.status = 0";
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
        if (!m_pMysql->QueryExec(strSql, boost::bind(&UserManager::DevInfoRelationSqlCB, this, _1, _2, _3, &RelationList)))
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

bool UserManager::QueryRelationByDevID(const std::string &strDevID, std::list<InteractiveProtoHandler::Relation> &RelationList,
    const unsigned int uiBeginIndex, const unsigned int uiPageSize)
{
    char sql[1024] = { 0 };
    memset(sql, 0, sizeof(sql));
    const char *sqlft = "select rel.userid, rel.deviceid, rel.relation, rel.begindate, rel.enddate, rel.createdate, rel.status, rel.extend"
        " from t_device_info dev, t_user_device_relation rel, t_user_info usr"
        " where dev.deviceid = rel.deviceid and usr.userid = rel.userid and rel.deviceid = '%s' and rel.status = 0 and dev.status = 0 and usr.status = 0";
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



void UserManager::SessionTimeoutProcessCB(const std::string &strSessionID)
{
        
    //广播消息表示用户会话超时


    LOG_INFO_RLD("Session timeout and session id is " << strSessionID);
}

void UserManager::InsertDeviceToDB(const InteractiveProtoHandler::Device &DevInfo)
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
        "id, userid, deviceid, relation, begindate, enddate, createdate, status, extend) values(uuid(),"
        "'%s','%s', '%d','%s', '%s', '%s','%d', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, relation.m_strUsrID.c_str(), relation.m_strDevID.c_str(), relation.m_iRelation,
        relation.m_strBeginDate.c_str(), relation.m_strEndDate.c_str(), relation.m_strCreateDate.c_str(), relation.m_iStatus, relation.m_strExtend.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Insert t_user_device_relation sql exec failed, sql is " << sql);
    }
}

void UserManager::RemoveRelationToDB(const RelationOfUsrAndDev &relation)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "update t_user_device_relation set status = %d where userid = '%s' and deviceid = '%s' and relation = %d and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, DELETE_STATUS, relation.m_strUsrID.c_str(), relation.m_strDevID.c_str(), relation.m_iRelation);

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Update t_user_device_relation sql exec failed, sql is " << sql);
    }

}

void UserManager::DelDeviceToDB(const std::list<std::string> &strDevIDList, const int iStatus)
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

void UserManager::ModDeviceToDB(const InteractiveProtoHandler::Device &DevInfo)
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

void UserManager::SharingRelationToDB(const RelationOfUsrAndDev &relation)
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
        
    InsertRelationToDB(relation);
}

void UserManager::CancelSharedRelationToDB(const RelationOfUsrAndDev &relation)
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

bool UserManager::QueryUserInfoToDB(const std::string &strUserID, InteractiveProtoHandler::User &usr, const bool IsNeedCache)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "select userid, username, userpassword, typeinfo, createdate, status, extend from t_user_info where userid = '%s' and status = 0";
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

bool UserManager::QueryDevInfoToDB(const std::string &strDevID, InteractiveProtoHandler::Device &dev, const bool IsNeedCache /*= true*/)
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

void UserManager::AddFriendsToDB(const RelationOfUsr &relation)
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

void UserManager::DelFriendsToDB(const std::string &strUserID, const std::list<std::string> &FriendIDList, const int iStatus)
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

bool UserManager::QueryUserRelationExist(const std::string &strUserID, const std::string &strFriendsID, const int iRelation, bool &blExist, const bool IsNeedCache /*= true*/)
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

bool UserManager::QueryUserRelationInfoToDB(const std::string &strUserID, const int iRelation, std::list<std::string> &strRelationIDList,
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

