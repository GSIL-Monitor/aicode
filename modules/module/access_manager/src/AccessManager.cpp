#include <map>
#include "AccessManager.h"
#include <boost/scope_exit.hpp>
#include "CommonUtility.h"
#include "ReturnCode.h"
#include "mysql_impl.h"
#include "boost/lexical_cast.hpp"
#include "json/json.h"
#include "UserLoginLTUserSite.h"
#include "P2PServerManager_SY.h"
#include "P2PServerManager_LT.h"
#include "HttpClient.h"

const std::string AccessManager::MAX_DATE = "2199-01-01 00:00:00";

const std::string AccessManager::ANDROID_APP = "Android_App";
const std::string AccessManager::IOS_APP = "iOS_App";
const std::string AccessManager::IPC = "IPC";

const std::string AccessManager::ONLINE = "online";
const std::string AccessManager::OFFLINE = "offline";

AccessManager::AccessManager(const ParamInfo &pinfo) : m_ParamInfo(pinfo), m_DBRuner(1), m_pProtoHandler(new InteractiveProtoHandler),
m_pMysql(new MysqlImpl), m_DBCache(m_pMysql), m_uiMsgSeq(0), m_pClusterAccessCollector(new ClusterAccessCollector(&m_SessionMgr, m_pMysql, &m_DBCache)),
m_DBTimer(NULL, 600)
{
    
}

AccessManager::~AccessManager()
{
    m_DBTimer.Stop();

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
    m_SessionMgr.SetSessionTimeoutCB(boost::bind(&ClusterAccessCollector::AddAccessTimeoutRecord, m_pClusterAccessCollector, _1, _2));

    if (!m_SessionMgr.Init())
    {
        LOG_ERROR_RLD("Session mgr init failed.");
        return false;
    }

    auto TmFunc = [&](const boost::system::error_code &ec) ->void
    {
        if (!m_pMysql->QueryExec(std::string("SET NAMES utf8")))
        {
            LOG_ERROR_RLD("Exec sql charset to utf8 failed, sql is SET NAMES utf8");
        }
        else
        {
            LOG_INFO_RLD("Set db to utf8 success.");
        }

        if (!RefreshAccessDomainName())
        {
            LOG_ERROR_RLD("Refresh access domain name failed.");
        }
        else
        {
            LOG_INFO_RLD("Refresh access domain name success");
        }
    };

    m_DBTimer.SetTimeOutCallBack(TmFunc);
    
    m_DBTimer.Run(true);

    m_DBCache.SetSqlCB(boost::bind(&AccessManager::UserInfoSqlCB, this, _1, _2, _3, _4));
    
    m_DBRuner.Run();

    m_SessionMgr.Run();

    LOG_INFO_RLD("UserManager init success");

    return true;
}

bool AccessManager::GetMsgType(const std::string &strMsg, int &iMsgType)
{
    InteractiveProtoHandler::MsgType mtype;
    if (!m_pProtoHandler->GetMsgType(strMsg, mtype))
    {
        LOG_ERROR_RLD("Get msg type failed.");
        return false;
    }

    iMsgType = mtype;
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
        InteractiveProtoHandler::MsgType::QueryAccessDomainNameReq_USR_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::GetDeviceAccessRecordReq_INNER_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::GetUserAccessRecordReq_INNER_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::QueryAppUpgradeReq_USR_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::QueryFirmwareUpgradeReq_DEV_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::AddConfigurationReq_MGR_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::DeleteConfigurationReq_MGR_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::ModifyConfigurationReq_MGR_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::QueryAllConfigurationReq_MGR_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::QueryUploadURLReq_MGR_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::P2pInfoReq_DEV_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::QueryIfP2pIDValidReq_USR_T == req.m_MsgType)
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
        RegUsrRsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
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
    bool blUserExist;
    if (ValidUser(RegUsrReq.m_userInfo.m_strUserID, RegUsrReq.m_userInfo.m_strUserName, blUserExist,
        "", RegUsrReq.m_userInfo.m_uiTypeInfo, true))
    {
        LOG_ERROR_RLD("Register user failed because user already exist and user name is " << RegUsrReq.m_userInfo.m_strUserName << 
            " and user id is " << RegUsrReq.m_userInfo.m_strUserID << " and user pwd is " << RegUsrReq.m_userInfo.m_strUserPassword);

        ReturnInfo::RetCode((ReturnInfo::USERNAME_EXISTED_USER));
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
        ModifyUsrRsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
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

    bool blUserExist;
    if (!ValidUser(ModifyUsrReq.m_userInfo.m_strUserID, ModifyUsrReq.m_userInfo.m_strUserName, blUserExist,
        "", ModifyUsrReq.m_userInfo.m_uiTypeInfo))
    {
        LOG_ERROR_RLD("Modify user failed because user is invalid and user name is " << ModifyUsrReq.m_userInfo.m_strUserName <<
            " and user id is " << ModifyUsrReq.m_userInfo.m_strUserID << " and user pwd is " << ModifyUsrReq.m_userInfo.m_strUserPassword);

        ReturnInfo::RetCode(ReturnInfo::USERID_NOT_EXISTED_USER);
        return false;
    }
    
    LOG_INFO_RLD("Modify user id is " << ModifyUsrReq.m_userInfo.m_strUserID << " session id is " << ModifyUsrReq.m_strSID 
        << " and user name is " << ModifyUsrReq.m_userInfo.m_strUserName << " and user pwd is " << ModifyUsrReq.m_userInfo.m_strUserPassword
        << " and user type is " << ModifyUsrReq.m_userInfo.m_uiTypeInfo << " and user extend is " << ModifyUsrReq.m_userInfo.m_strExtend);

    if (!ModifyUsrReq.m_strOldPwd.empty() && !IsUserPasswordValid(ModifyUsrReq.m_userInfo.m_strUserID, ModifyUsrReq.m_strOldPwd))
    {
        LOG_ERROR_RLD("Check user password valid failed, modify password must provide the correct old password, user id is " <<
            ModifyUsrReq.m_userInfo.m_strUserID << " and src id is " << ModifyUsrReq.m_strSID);

        ReturnInfo::RetCode(ReturnInfo::PASSWORD_INVALID_USER);
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
    std::list<std::string> strDevNameList;
    BOOST_SCOPE_EXIT(&blResult, this_, &RelationList, &strSessionID, &LoginReqUsr, &writer, &strSrcID, &strDevNameList)
    {
        InteractiveProtoHandler::LoginRsp_USR LoginRspUsr;

        LoginRspUsr.m_MsgType = InteractiveProtoHandler::MsgType::LoginRsp_USR_T;
        LoginRspUsr.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        LoginRspUsr.m_strSID = strSessionID;
        LoginRspUsr.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        LoginRspUsr.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        LoginRspUsr.m_strUserID = LoginReqUsr.m_userInfo.m_strUserID;
        LoginRspUsr.m_strValue = "value";

        if (blResult)
        {
            LoginRspUsr.m_reInfoList.swap(RelationList);
            LoginRspUsr.m_strDevNameList.swap(strDevNameList);

            std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
            std::string::size_type pos = strCurrentTime.find('T');
            strCurrentTime.replace(pos, 1, std::string(" "));

            this_->m_DBRuner.Post(boost::bind(&ClusterAccessCollector::AddUserAccessRecord, this_->m_pClusterAccessCollector,
                strSessionID, LoginReqUsr.m_userInfo.m_strUserID, LoginReqUsr.m_uiTerminalType, strCurrentTime, ""));
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
        
        if (blResult)
        {
            auto itBeginDevName = LoginRspUsr.m_strDevNameList.begin();
            auto itEndDevName = LoginRspUsr.m_strDevNameList.end();
            auto itBeginRelation = LoginRspUsr.m_reInfoList.begin();
            auto itEndRelation = LoginRspUsr.m_reInfoList.end();
                        
            while (itBeginDevName != itEndDevName && itBeginRelation != itEndRelation)
            {
                LOG_INFO_RLD("Query user device relation successful, user id is " << itBeginRelation->m_strUserID <<
                    " and device id is " << itBeginRelation->m_strDevID << " and device name is " << *itBeginDevName <<
                    " and relation is " << itBeginRelation->m_uiRelation <<
                    " and begin date is " << itBeginRelation->m_strBeginDate <<
                    " and end date is " << itBeginRelation->m_strEndDate);

                ++itBeginDevName;
                ++itBeginRelation;
            }
        }
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
    
    bool blUserExist = false;
    if (!ValidUser(LoginReqUsr.m_userInfo.m_strUserID, LoginReqUsr.m_userInfo.m_strUserName, blUserExist,
        LoginReqUsr.m_userInfo.m_strUserPassword, LoginReqUsr.m_userInfo.m_uiTypeInfo))
    {
        if (blUserExist)
        {
            LOG_ERROR_RLD("User login failed, user password is invalid, src id is " << strSrcID <<
                " and user name is " << LoginReqUsr.m_userInfo.m_strUserName);

            ReturnInfo::RetCode(ReturnInfo::PASSWORD_INVALID_USER);
            return false;
        }
        else
        {
            //用户会直接登录浪涛，不需要平台再作登录浪涛的请求
            //if (!LoginLTUserSiteReq(LoginReqUsr.m_userInfo.m_strUserName, LoginReqUsr.m_userInfo.m_strUserPassword,
            //    m_ParamInfo.m_strLTUserSite, m_ParamInfo.m_strLTUserSiteRC4Key, strSrcID, LoginReqUsr.m_userInfo.m_strUserID))
            //{
            //    LOG_ERROR_RLD("LoginLTUserSiteReq failed, login user name: " << LoginReqUsr.m_userInfo.m_strUserName);

            //    ReturnInfo::RetCode(ReturnInfo::USERNAME_OR_PASSWORD_INVALID_USER);
            //    return false;
            //}

            //LOG_INFO_RLD("LoginLTUserSiteReq seccessful, login user name: " << LoginReqUsr.m_userInfo.m_strUserName);
            LOG_ERROR_RLD("User login failed, user name or password is invalid, src id is " << strSrcID <<
                " and user name is " << LoginReqUsr.m_userInfo.m_strUserName);

            ReturnInfo::RetCode(ReturnInfo::USERNAME_OR_PASSWORD_INVALID_USER);
            return false;
        }
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
        boost::bind(&AccessManager::SessionTimeoutProcessCB, this, _1), ClusterAccessCollector::USER_SESSION, LoginReqUsr.m_userInfo.m_strUserID);
        
    if (!QueryRelationByUserID(LoginReqUsr.m_userInfo.m_strUserID, RelationList, strDevNameList))
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
    RegUsrReq.m_userInfo.m_strAliasName = "";
    RegUsrReq.m_userInfo.m_uiTypeInfo = 0;
    RegUsrReq.m_userInfo.m_strEmail = strUserName; //浪涛用户名和邮箱名一致
    RegUsrReq.m_userInfo.m_strCreatedate = "";
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

        if (blResult)
        {
            std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
            std::string::size_type pos = strCurrentTime.find('T');
            strCurrentTime.replace(pos, 1, std::string(" "));

            this_->m_DBRuner.Post(boost::bind(&ClusterAccessCollector::AddUserAccessRecord, this_->m_pClusterAccessCollector,
                LogoutReqUsr.m_strSID, "", 0xFFFFFFFF, "", strCurrentTime));
        }

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
    m_SessionMgr.ResetID(ShakehandReqUsr.m_strUserID);
    
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
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strDeviceID = blResult ? req.m_devInfo.m_strDevID : "";
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

    //P2PID、域名、设备ID只能输入一个字段，如果没有输入设备ID，则根据P2PID或者域名查询设备ID
    //std::string strDeviceID;
    //unsigned int counts = 0;
    //if (!req.m_devInfo.m_strDevID.empty())
    //{
    //    strDeviceID = req.m_devInfo.m_strDevID;
    //    ++counts;
    //}

    //if (!req.m_devInfo.m_strDomainName.empty())
    //{
    //    if (strDeviceID.empty() && !QueryDevPropertyByDevDomain(req.m_devInfo.m_strDomainName, strDeviceID))
    //    {
    //        LOG_ERROR_RLD("Add device query device id failed, src id is " << strSrcID <<
    //            " and device domain name is " << req.m_devInfo.m_strDomainName);
    //        return false;
    //    }
    //    ++counts;
    //}

    //if (!req.m_devInfo.m_strP2pID.empty())
    //{
    //    if (strDeviceID.empty() && !QueryDevIDByDevP2pID(req.m_devInfo.m_strP2pID, strDeviceID))
    //    {
    //        LOG_ERROR_RLD("Add device query device id failed, src id is " << strSrcID <<
    //            " and device p2pid is " << req.m_devInfo.m_strP2pID);
    //        return false;
    //    }
    //    ++counts;
    //}

    //if (1 != counts)
    //{
    //    LOG_ERROR_RLD("Add device input error, src id is " << strSrcID << ", only allow input one of deviceid|domainname|p2pid");

    //    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_MUCH);
    //    return false;
    //}

    //if (strDeviceID.empty())
    //{
    //    LOG_ERROR_RLD("Add device failed, the device is not recorded, src id is " << strSrcID <<
    //        " and p2p id is " << req.m_strP2pID <<
    //        " and domain name is " << req.m_strDomainName);

    //    ReturnInfo::RetCode(ReturnInfo::DEVICE_NOT_RECORDED_USER);
    //    return false;
    //}

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
        DevInfo.m_strP2pID = req.m_devInfo.m_strP2pID;
        DevInfo.m_strDomainName = req.m_devInfo.m_strDomainName;
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

        //m_DBRuner.Post(boost::bind(&AccessManager::AddNoOwnerFile, this, req.m_strUserID, strDeviceID));

        blResult = true;
    }
    else if (strUserID == req.m_strUserID)
    {
        LOG_INFO_RLD("Add devcice successful, the device has been added by current user, user id is" << strUserID);
        blResult = true;
    } 
    else
    {
        ReturnInfo::RetCode(ReturnInfo::DEVICE_IS_ADDED_USER);
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
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
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

    for (auto strDevID : req.m_strDevIDList)
    {
        std::string strUserID;
        if (!QueryOwnerUserIDByDeviceID(strDevID, strUserID))
        {
            LOG_ERROR_RLD("Delete device failed, query device owner error, src id is " << strSrcID);
            return false;
        }

        if (req.m_strUserID != strUserID)
        {
            LOG_ERROR_RLD("Delete device failed, the device is not belong to the user, src id is " << strSrcID <<
                " and user id is " << req.m_strUserID << " and device id is " << strDevID);

            ReturnInfo::RetCode(ReturnInfo::DEVICE_NOT_BELONG_TO_USER);
            return false;
        }
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
    std::string strVersion;
    std::string strUpdateDate;
    std::string strOnlineStatus;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &dev, &writer, &strSrcID, &strVersion, &strUpdateDate, &strOnlineStatus)
    {
        InteractiveProtoHandler::QueryDevInfoRsp_USR rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryDevInfoRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strVersion = blResult ? strVersion : "";
        rsp.m_strUpdateDate = blResult ? strUpdateDate : "";
        rsp.m_strOnlineStatus = blResult ? strOnlineStatus : "";
        rsp.m_strValue = "value";

        rsp.m_devInfo.m_strCreatedate = dev.m_strCreatedate;
        rsp.m_devInfo.m_strDevID = dev.m_strDevID;
        rsp.m_devInfo.m_strDevName = dev.m_strDevName;
        rsp.m_devInfo.m_strDevPassword = dev.m_strDevPassword;
        rsp.m_devInfo.m_strP2pID = dev.m_strP2pID;
        rsp.m_devInfo.m_strDomainName = dev.m_strDomainName;
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

    if (!QueryDeviceDateAndVersionToDB(req.m_strDevID, dev.m_uiTypeInfo, strUpdateDate, strVersion))
    {
        //已查到关键信息，此处只做日志记录，不返回错误
        LOG_ERROR_RLD("Query device updatedate and version from db failed, device id is " << req.m_strDevID);
        //return false;
    }

    if (m_SessionMgr.ExistID(req.m_strDevID))
    {
        strOnlineStatus = ONLINE;
    }
    else
    {
        strOnlineStatus = OFFLINE;
    }
    
    blResult = true;

    return blResult;
}

bool AccessManager::QueryDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;
    InteractiveProtoHandler::QueryDevReq_USR req;
    std::list<InteractiveProtoHandler::Relation> RelationList;
    std::list<std::string> strDevNameList;

    BOOST_SCOPE_EXIT(&blResult, this_, &RelationList, &req, &writer, &strSrcID, &strDevNameList)
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
            rsp.m_strDevNameList.swap(strDevNameList);
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

        if (blResult)
        {
            auto itBeginDevName = rsp.m_strDevNameList.begin();
            auto itEndDevName = rsp.m_strDevNameList.end();
            auto itBeginRelation = rsp.m_allRelationInfoList.begin();
            auto itEndRelation = rsp.m_allRelationInfoList.end();

            while (itBeginDevName != itEndDevName && itBeginRelation != itEndRelation)
            {
                LOG_INFO_RLD("Query user device relation successful, user id is " << itBeginRelation->m_strUserID <<
                    " and device id is " << itBeginRelation->m_strDevID << " and device name is " << *itBeginDevName <<
                    " and relation is " << itBeginRelation->m_uiRelation <<
                    " and begin date is " << itBeginRelation->m_strBeginDate <<
                    " and end date is " << itBeginRelation->m_strEndDate);

                ++itBeginDevName;
                ++itBeginRelation;
            }
        }
    }
    BOOST_SCOPE_EXIT_END


    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query device req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryRelationByUserID(req.m_strUserID, RelationList, strDevNameList, req.m_uiBeginIndex))
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
    std::list<std::string> strUserNameList;

    BOOST_SCOPE_EXIT(&blResult, this_, &RelationList, &req, &writer, &strSrcID, &strUserNameList)
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
            rsp.m_strUserNameList.swap(strUserNameList);
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

        if (blResult)
        {
            auto itBeginUserName = rsp.m_strUserNameList.begin();
            auto itEndUserName = rsp.m_strUserNameList.end();
            auto itBeginRelation = rsp.m_allRelationInfoList.begin();
            auto itEndRelation = rsp.m_allRelationInfoList.end();

            while (itBeginUserName != itEndUserName && itBeginRelation != itEndRelation)
            {
                LOG_INFO_RLD("Query device user relation successful, device id is " << itBeginRelation->m_strDevID <<
                    " and user id is " << itBeginRelation->m_strUserID << " and user name is " << *itBeginUserName <<
                    " and relation is " << itBeginRelation->m_uiRelation <<
                    " and begin date is " << itBeginRelation->m_strBeginDate <<
                    " and end date is " << itBeginRelation->m_strEndDate);

                ++itBeginUserName;
                ++itBeginRelation;
            }
        }
    }
    BOOST_SCOPE_EXIT_END


    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query user req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryRelationByDevID(req.m_strDevID, RelationList, strUserNameList, req.m_uiBeginIndex))
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
        LoginRspUsr.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        LoginRspUsr.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        LoginRspUsr.m_strValue = strValue;

        if (blResult)
        {
            std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
            std::string::size_type pos = strCurrentTime.find('T');
            strCurrentTime.replace(pos, 1, std::string(" "));

            this_->m_DBRuner.Post(boost::bind(&ClusterAccessCollector::AddDeviceAccessRecord, this_->m_pClusterAccessCollector,
                strSessionID, LoginReqDev.m_strDevID, LoginReqDev.m_uiDeviceType, strCurrentTime, ""));
        }

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

    if (!LoginReqDev.m_strDomainName.empty() && !IsValidDeviceDomain(LoginReqDev.m_strDevID, LoginReqDev.m_strDomainName))
    {
        LOG_ERROR_RLD("Device login vefiry device domain name failed, device id is " << LoginReqDev.m_strDevID <<
            " and domain name is " << LoginReqDev.m_strDomainName);

        ReturnInfo::RetCode(ReturnInfo::DEVICE_DOMAIN_USED_DEV);
        return false;
    }

    if (!LoginReqDev.m_strP2pID.empty() && !IsValidP2pIDProperty(LoginReqDev.m_strDevID, LoginReqDev.m_strP2pID))
    {
        LOG_ERROR_RLD("Device login vefiry device p2p id failed, device id is " << LoginReqDev.m_strDevID <<
            " and p2p id is " << LoginReqDev.m_strP2pID);

        ReturnInfo::RetCode(ReturnInfo::DEVICE_P2PID_USED_DEV);
        return false;
    }

    //烧录了P2P信息的设备登录时，同时上报P2P信息，平台记入数据库
    if (P2P_DEVICE_BUILDIN == LoginReqDev.m_uiP2pBuildin)
    {
        if (0XFFFFFFFF == LoginReqDev.m_uiP2pSupplier)
        {
            LOG_ERROR_RLD("Login req of device failed, field p2p type is empty");
            return false;
        }

        m_DBRuner.Post(boost::bind(&AccessManager::InsertP2pInfoToDB, this, LoginReqDev.m_uiP2pSupplier, LoginReqDev.m_strP2pID,
            LoginReqDev.m_strDevID, P2P_DEVICE_BUILDIN));
    }

    //记录设备登录时上报的信息到设备属性表
    if (DEVICE_TYPE_DOORBELL == LoginReqDev.m_uiDeviceType)
    {
        m_DBRuner.Post(boost::bind(&AccessManager::InsertDoorbellParameterToDB, this, LoginReqDev));
    }
    else
    {
        m_DBRuner.Post(boost::bind(&AccessManager::InsertDevPropertyToDB, this, LoginReqDev));
    }

    m_SessionMgr.Create(strSessionID, strBody, boost::lexical_cast<unsigned int>(m_ParamInfo.m_strDevSessionTimeoutCountThreshold),
        boost::bind(&AccessManager::SessionTimeoutProcessCB, this, _1), ClusterAccessCollector::DEVICE_SESSION, LoginReqDev.m_strDevID);

    blResult = true;

    return blResult;
}

bool AccessManager::P2pInfoReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::P2pInfoReq_DEV req;

    std::string strDeviceID;
    std::string strP2pServer;
    std::string strP2pID;
    unsigned int uiLease = 0;
    std::string strLicenseKey;
    std::string strPushID;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID, &strP2pServer, &strP2pID, &uiLease, &strLicenseKey, &strPushID)
    {
        InteractiveProtoHandler::P2pInfoRsp_DEV rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::P2pInfoRsp_DEV_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strP2pID = blResult ? strP2pID : "";
        rsp.m_strP2pServer = blResult ? strP2pServer : "";
        rsp.m_uiLease = blResult ? uiLease : 0;
        rsp.m_strLicenseKey = blResult ? strLicenseKey : "";
        rsp.m_strPushID = blResult ? strPushID : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("P2p info of device rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("P2p info of device rsp already send, dst id is " << strSrcID << " and device id is " << req.m_strDevID <<
            " and domainname is " << req.m_strDomainName << " and device ip is " << req.m_strDevIpAddress <<
            " and p2p supplier is " << req.m_uiP2pSupplier << " and p2p id is " << strP2pID << " and p2p server is " << strP2pServer <<
            " and lease is " << uiLease << " and liecense key is " << strLicenseKey << " and push id is " << strPushID <<
            " and session id is " << req.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("P2p info of device req unserialize failed, src id is " << strSrcID);
        return false;
    }

    //根据二级域名获取设备ID
    if (req.m_strDevID.empty())
    {
        if (req.m_strDomainName.empty())
        {
            LOG_ERROR_RLD("P2p info of device failed, device id and domainname are both empty, src id is " << strSrcID);

            ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);
            return false;
        }
        else
        {
            if (!QueryDevPropertyByDevDomain(req.m_strDomainName, strDeviceID, strP2pID) || strDeviceID.empty())
            {
                LOG_ERROR_RLD("P2p info of device failed, get device id by domainname error, src id is " << strSrcID <<
                    " and domainname is " << req.m_strDomainName);
                ReturnInfo::RetCode(ReturnInfo::DEVICE_DOMAINNAME_INVALID);
                return false;
            }

            if (strP2pID.empty())
            {
                if (0xFFFF == req.m_uiP2pSupplier)
                {
                    LOG_ERROR_RLD("P2p info of device failed, the divice not assign P2PID, src id is " << strSrcID <<
                        " and domainname is " << req.m_strDomainName);
                    ReturnInfo::RetCode(ReturnInfo::DEVICE_DOMAINNAME_INVALID);
                    return false;
                }
            }
            else
            {
                LOG_INFO_RLD("P2p info of device successful, device p2p id is built-in, p2pid is " << strP2pID);

                blResult = true;
                return blResult;
            }

        }
    }
    else
    {
        strDeviceID = req.m_strDevID;
    }

    //获取p2pid和p2pserver
    P2PConnectParam p2pConnParam;
    boost::shared_ptr<P2PServerManager> p2pSvrManager;
    if (P2P_SUPPLIER_LT == req.m_uiP2pSupplier)
    {
        p2pSvrManager.reset(new P2PServerManager_LT("LT"));
    }
    else if (P2P_SUPPLIER_SY == req.m_uiP2pSupplier)
    {
        p2pSvrManager.reset(new P2PServerManager_SY());
    }
    else if (P2P_SUPPLIER_TUTK == req.m_uiP2pSupplier)
    {
        p2pSvrManager.reset(new P2PServerManager_LT("TUTK"));
    }
    else
    {
        LOG_ERROR_RLD("P2p info of device failed, unknown p2p supplier, src id is " << strSrcID << " and supplier is " << req.m_uiP2pSupplier);
        return false;
    }

    p2pSvrManager->SetUrl(m_ParamInfo.m_strGetIpInfoSite);
    p2pSvrManager->SetDBManager(&m_DBCache, m_pMysql);

    int iRetry = 0;
    do 
    {
        if (p2pSvrManager->DeviceRequestP2PConnectParam(p2pConnParam, strDeviceID, req.m_strDevIpAddress))
        {
            break;
        }

        boost::this_thread::sleep(boost::posix_time::seconds(1));
        ++iRetry;
        LOG_ERROR_RLD("Get device p2p info failed, device id is " << strDeviceID << " and ip is " << req.m_strDevIpAddress <<
            " and retry " << iRetry << " times");
    } while (iRetry < GET_TIMEZONE_RETRY_TIMES);

    if (GET_TIMEZONE_RETRY_TIMES == iRetry)
    {
        LOG_ERROR_RLD("Get device p2p info failed, device id is " << strDeviceID << " and ip is " << req.m_strDevIpAddress);
        return false;
    }

    strP2pID = p2pConnParam.sP2Pid;
    strP2pServer = p2pConnParam.sInitstring;
    uiLease = p2pConnParam.nTime;
    strLicenseKey = p2pConnParam.sparam1;
    strPushID = p2pConnParam.sparam2;

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
    m_SessionMgr.ResetID(ShakehandReqDev.m_strDevID);

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

        if (blResult)
        {
            std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
            std::string::size_type pos = strCurrentTime.find('T');
            strCurrentTime.replace(pos, 1, std::string(" "));

            this_->m_DBRuner.Post(boost::bind(&ClusterAccessCollector::AddDeviceAccessRecord, this_->m_pClusterAccessCollector,
                LogoutReqDev.m_strSID, LogoutReqDev.m_strDevID, 0xFFFFFFFF, "", strCurrentTime));
        }

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
    std::string strPushID;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID, &strP2pServer, &strP2pID, &uiLease, &strLicenseKey, &strPushID)
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
        rsp.m_strPushID = blResult ? strPushID : "";

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
            " and liecense key is " << strLicenseKey << " and push id is " << strPushID <<
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
    boost::shared_ptr<P2PServerManager> p2pSvrManager;
    if (P2P_SUPPLIER_LT == req.m_uiP2pSupplier)
    {
        p2pSvrManager.reset(new P2PServerManager_LT("LT"));
    }
    else if (P2P_SUPPLIER_SY == req.m_uiP2pSupplier)
    {
        p2pSvrManager.reset(new P2PServerManager_SY());
    }
    else if (P2P_SUPPLIER_TUTK == req.m_uiP2pSupplier)
    {
        p2pSvrManager.reset(new P2PServerManager_LT("TUTK"));
    }
    else
    {
        LOG_ERROR_RLD("P2p info of user failed, unknown p2p supplier, src id is " << strSrcID << " and supplier is " << req.m_uiP2pSupplier);
        return false;
    }

    p2pSvrManager->SetUrl(m_ParamInfo.m_strGetIpInfoSite);
    p2pSvrManager->SetDBManager(&m_DBCache, m_pMysql);
    if (!p2pSvrManager->DeviceRequestP2PConnectParam(p2pConnParam, req.m_strDevID, req.m_strUserIpAddress, req.m_strUserID))
    {
        LOG_ERROR_RLD("Get user p2p info failed,  user id is " << req.m_strUserID <<
            " and device id is " << req.m_strDevID << " and ip is " << req.m_strUserIpAddress);
        return false;
    }

    strP2pID = p2pConnParam.sP2Pid;
    strP2pServer = p2pConnParam.sInitstring;
    uiLease = p2pConnParam.nTime;
    strLicenseKey = p2pConnParam.sparam1;
    strPushID = p2pConnParam.sparam2;

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
        RetrievePwdRspUsr.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
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

        ReturnInfo::RetCode(ReturnInfo::EMAIL_NOT_MATCHED_USER);
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
    cTimeZone.setpostUrl(m_ParamInfo.m_strGetIpInfoSite);
    cTimeZone.SetDBManager(&m_DBCache, m_pMysql);

    AccessDomainInfo DomainInfo;
    int iRetry = 0;
    do 
    {
        if (!cTimeZone.GetCountryTime(req.m_strUserIpAddress, timezone))
        {
            LOG_ERROR_RLD("Get country timezone info failed, user ip is " << req.m_strUserIpAddress << " and retry " << ++iRetry << " times");

            boost::this_thread::sleep(boost::posix_time::seconds(1));
            continue;
        }

        if (QueryAccessDomainInfoByArea(timezone.sCode, "", DomainInfo))
        {
            break;
        }

        ++iRetry;
        LOG_ERROR_RLD("Query user access domain name failed, user ip is " << req.m_strUserIpAddress << " and retry " << iRetry << " times");
    } while (iRetry < GET_TIMEZONE_RETRY_TIMES);

    if (GET_TIMEZONE_RETRY_TIMES == iRetry)
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
    cTimeZone.setpostUrl(m_ParamInfo.m_strGetIpInfoSite);
    cTimeZone.SetDBManager(&m_DBCache, m_pMysql);

    AccessDomainInfo DomainInfo;
    int iRetry = 0;
    do 
    {
        if (!cTimeZone.GetCountryTime(req.m_strDevIpAddress, timezone))
        {
            LOG_ERROR_RLD("Get country timezone info failed, device ip is " << req.m_strDevIpAddress << " and retry " << ++iRetry << " times");

            boost::this_thread::sleep(boost::posix_time::seconds(1));
            continue;
        }

        if (QueryAccessDomainInfoByArea(timezone.sCode, "", DomainInfo))
        {
            break;
        }

        ++iRetry;
        LOG_ERROR_RLD("Query device access domain name failed, device ip is " << req.m_strDevIpAddress << " and retry " << iRetry << " times");
    } while (iRetry < GET_TIMEZONE_RETRY_TIMES);

    if (GET_TIMEZONE_RETRY_TIMES == iRetry)
    {
        LOG_ERROR_RLD("Query device access domain name failed, device ip is " << req.m_strDevIpAddress);
        return false;
    }

    strDomainName = DomainInfo.strDomainName;
    uiLease = DomainInfo.uiLease;

    blResult = true;

    return blResult;
}

bool AccessManager::QueryUpgradeSiteReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::QueryUpgradeSiteReq_DEV req;

    std::string strUpgradeUrl;
    unsigned int uiLease;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &strSrcID, &writer, &strUpgradeUrl, &uiLease)
    {
        InteractiveProtoHandler::QueryUpgradeSiteRsp_DEV rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryUpgradeSiteRsp_DEV_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strUpgradeSiteUrl = blResult ? strUpgradeUrl : "";
        rsp.m_uiLease = blResult ? uiLease : 0;

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query upgrade site url rsp serialize failed.");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query upgrade site url rsp already send, dst id is " << strSrcID <<
            " and device id is " << req.m_strDevID <<
            " and device ip is " << req.m_strDevIpAddress <<
            " and session id is " << req.m_strSID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query upgrade site url req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryUpgradeSiteToDB(strUpgradeUrl, uiLease))
    {
        LOG_ERROR_RLD("Query upgrade site url failed, device id is " << req.m_strDevID << " and device ip is " << req.m_strDevIpAddress);
        return false;
    }

    blResult = true;

    return blResult;
}

bool AccessManager::GetDeviceAccessRecordReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::GetDeviceAccessRecordReq_INNER req;
    unsigned int uiRecordTotal = 0;
    std::list<InteractiveProtoHandler::DeviceAccessRecord> deviceAccessRecordList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &deviceAccessRecordList, &uiRecordTotal)
    {
        InteractiveProtoHandler::GetDeviceAccessRecordRsp_INNER rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::GetDeviceAccessRecordRsp_INNER_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_uiRecordTotal = blResult ? uiRecordTotal : 0;

        if (blResult)
        {
            rsp.m_deviceAccessRecordList.swap(deviceAccessRecordList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Get device access record rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Get device access record rsp already send, dst id is " << strSrcID <<
            " and request begin index is " << req.m_uiBeginIndex <<
            " and result is " << blResult);

        if (blResult)
        {
            int i = 0;
            for (auto &deviceAccessRecord : rsp.m_deviceAccessRecordList)
            {
                LOG_INFO_RLD("Device access record[" << i << "]: "
                    " access id is " << deviceAccessRecord.m_strAccessID <<
                    " and cluster id is " << deviceAccessRecord.m_strClusterID <<
                    " and device id is " << deviceAccessRecord.m_strDeviceID <<
                    " and device name is " << deviceAccessRecord.m_strDeviceName <<
                    " and device type is " << deviceAccessRecord.m_uiDeviceType <<
                    " and login time is " << deviceAccessRecord.m_strLoginTime <<
                    " and logout time is " << deviceAccessRecord.m_strLogoutTime <<
                    " and online duration is " << deviceAccessRecord.m_uiOnlineDuration <<
                    " and create date is " << deviceAccessRecord.m_strCreateDate <<
                    " and status is " << deviceAccessRecord.m_uiStatus);

                ++i;
            }
        }
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Get device access record req unserialize failed, src id is " << strSrcID);
        return false;
    }

    uiRecordTotal = m_pClusterAccessCollector->DeviceAccessRecordSize(req.m_uiBeginIndex);

    if (!m_pClusterAccessCollector->GetDeviceAccessRecord(deviceAccessRecordList, req.m_uiBeginIndex))
    {
        LOG_ERROR_RLD("Get device access record failed, src id is " << strSrcID);
    }

    blResult = true;

    return blResult;
}

bool AccessManager::GetUserAccessRecordReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::GetUserAccessRecordReq_INNER req;
    unsigned int uiRecordTotal = 0;
    std::list<InteractiveProtoHandler::UserAccessRecord> userAccessRecordList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &userAccessRecordList, &uiRecordTotal)
    {
        InteractiveProtoHandler::GetUserAccessRecordRsp_INNER rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::GetUserAccessRecordRsp_INNER_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_uiRecordTotal = blResult ? uiRecordTotal : 0;

        if (blResult)
        {
            rsp.m_userAccessRecordList.swap(userAccessRecordList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Get user access record rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Get user access record rsp already send, dst id is " << strSrcID <<
            " and request begin index is " << req.m_uiBeginIndex <<
            " and result is " << blResult);

        if (blResult)
        {
            int i = 0;
            for (auto &userAccessRecord : rsp.m_userAccessRecordList)
            {
                LOG_INFO_RLD("User access record[" << i << "]: "
                    " access id is " << userAccessRecord.m_strAccessID <<
                    " and cluster id is " << userAccessRecord.m_strClusterID <<
                    " and user id is " << userAccessRecord.m_strUserID <<
                    " and user name is " << userAccessRecord.m_strUserName <<
                    " and user aliasname is " << userAccessRecord.m_strUserAliasname <<
                    " and client type is " << userAccessRecord.m_uiClientType <<
                    " and login time is " << userAccessRecord.m_strLoginTime <<
                    " and logout time is " << userAccessRecord.m_strLogoutTime <<
                    " and online duration is " << userAccessRecord.m_uiOnlineDuration <<
                    " and create date is " << userAccessRecord.m_strCreateDate <<
                    " and status is " << userAccessRecord.m_uiStatus);

                ++i;
            }
        }
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Get user access record req unserialize failed, src id is " << strSrcID);
        return false;
    }

    uiRecordTotal = m_pClusterAccessCollector->UserAccessRecordSize(req.m_uiBeginIndex);

    if (!m_pClusterAccessCollector->GetUserAccessRecord(userAccessRecordList, req.m_uiBeginIndex))
    {
        LOG_ERROR_RLD("Get user access record failed, src id is " << strSrcID);
    }

    blResult = true;

    return blResult;
}

bool AccessManager::QueryAppUpgradeReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::QueryAppUpgradeReq_USR req;
    InteractiveProtoHandler::AppUpgrade appUpgrade;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &appUpgrade, &writer, &strSrcID)
    {
        InteractiveProtoHandler::QueryAppUpgradeRsp_USR rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryAppUpgradeRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        rsp.m_appUpgrade.m_uiNewVersionValid = appUpgrade.m_uiNewVersionValid;
        rsp.m_appUpgrade.m_strAppName = appUpgrade.m_strAppName;
        rsp.m_appUpgrade.m_strAppPath = appUpgrade.m_strAppPath;
        rsp.m_appUpgrade.m_uiAppSize = appUpgrade.m_uiAppSize;
        rsp.m_appUpgrade.m_strVersion = appUpgrade.m_strVersion;
        rsp.m_appUpgrade.m_strDescription = appUpgrade.m_strDescription;
        rsp.m_appUpgrade.m_uiForceUpgrade = appUpgrade.m_uiForceUpgrade;
        rsp.m_appUpgrade.m_strUpdateDate = appUpgrade.m_strUpdateDate;

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query app upgrade rsp serialize failed.");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query app upgrade rsp already send, dst id is " << strSrcID << " and category is " << req.m_strCategory <<
            " and sub category is " << req.m_strSubCategory << " and current version is " << req.m_strCurrentVersion <<
            " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query app upgrade req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryAppUpgradeToDB(req.m_strCategory, req.m_strSubCategory, req.m_strCurrentVersion, appUpgrade))
    {
        LOG_ERROR_RLD("Query app upgrade from db failed, category is " << req.m_strCategory <<
            " and sub category is " << req.m_strSubCategory << " and current version is " << req.m_strCurrentVersion);
        return false;
    }

    blResult = true;

    return blResult;
}

bool AccessManager::QueryFirmwareUpgradeReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::QueryFirmwareUpgradeReq_DEV req;
    InteractiveProtoHandler::FirmwareUpgrade firmwareUpgrade;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &firmwareUpgrade, &writer, &strSrcID)
    {
        InteractiveProtoHandler::QueryFirmwareUpgradeRsp_DEV rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryFirmwareUpgradeRsp_DEV_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        rsp.m_firmwareUpgrade.m_uiNewVersionValid = firmwareUpgrade.m_uiNewVersionValid;
        rsp.m_firmwareUpgrade.m_strFirmwareName = firmwareUpgrade.m_strFirmwareName;
        rsp.m_firmwareUpgrade.m_strFirmwarePath = firmwareUpgrade.m_strFirmwarePath;
        rsp.m_firmwareUpgrade.m_uiFirmwareSize = firmwareUpgrade.m_uiFirmwareSize;
        rsp.m_firmwareUpgrade.m_strVersion = firmwareUpgrade.m_strVersion;
        rsp.m_firmwareUpgrade.m_strDescription = firmwareUpgrade.m_strDescription;
        rsp.m_firmwareUpgrade.m_uiForceUpgrade = firmwareUpgrade.m_uiForceUpgrade;
        rsp.m_firmwareUpgrade.m_strUpdateDate = firmwareUpgrade.m_strUpdateDate;

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query firmware upgrade rsp serialize failed.");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query firmware upgrade rsp already send, dst id is " << strSrcID << " and category is " << req.m_strCategory <<
            " and sub category is " << req.m_strSubCategory << " and current version is " << req.m_strCurrentVersion <<
            " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query firmware upgrade req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryFirwareUpgradeToDB(req.m_strCategory, req.m_strSubCategory, req.m_strCurrentVersion, firmwareUpgrade))
    {
        LOG_ERROR_RLD("Query firmware upgrade from db failed, category is " << req.m_strCategory <<
            " and sub category is " << req.m_strSubCategory << " and current version is " << req.m_strCurrentVersion);
        return false;
    }

    blResult = true;

    return blResult;
}

bool AccessManager::QueryUploadURLReqMgr(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::QueryUploadURLReq_MGR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &m_ParamInfo)
    {
        InteractiveProtoHandler::QueryUploadURLRsp_MGR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryUploadURLRsp_MGR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strUploadURL = blResult ? m_ParamInfo.m_strUploadURL : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query upload url rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query upload url rsp already send, dst id is " << strSrcID <<
            " and upload url is " << rsp.m_strUploadURL <<
            " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query upload url req unserialize failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool AccessManager::AddConfigurationReqMgr(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::AddConfigurationReq_MGR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        InteractiveProtoHandler::AddConfigurationRsp_MGR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::AddConfigurationRsp_MGR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add configuration item rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Add configuration item already send, dst id is " << strSrcID <<
            " and category is " << req.m_configuration.m_strCategory <<
            " and sub category is " << req.m_configuration.m_strSubCategory <<
            " and latest version is " << req.m_configuration.m_strLatestVersion <<
            " and description is " << req.m_configuration.m_strDescription <<
            " and force version is " << req.m_configuration.m_strForceVersion <<
            " and file server address is " << req.m_configuration.m_strServerAddress <<
            " and file name is " << req.m_configuration.m_strFileName <<
            " and file id is " << req.m_configuration.m_strFileID <<
            " and file size is " << req.m_configuration.m_uiFileSize <<
            " and file path is " << req.m_configuration.m_strFilePath <<
            " and lease duration is " << req.m_configuration.m_uiLeaseDuration <<
            " and update date is " << req.m_configuration.m_strUpdateDate <<
            " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Add configuration item req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!IsValidConfiguration(req.m_configuration.m_strCategory, req.m_configuration.m_strSubCategory))
    {
        LOG_ERROR_RLD("Add configuration verify configuration failed, src id is " << strSrcID <<
            " and category is " << req.m_configuration.m_strCategory << " and sub category is " << req.m_configuration.m_strSubCategory);
        return false;
    }

    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));

    InteractiveProtoHandler::Configuration configuration;
    configuration.m_strCategory = req.m_configuration.m_strCategory;
    configuration.m_strSubCategory = req.m_configuration.m_strSubCategory;
    configuration.m_strLatestVersion = req.m_configuration.m_strLatestVersion;
    configuration.m_strDescription = req.m_configuration.m_strDescription;
    configuration.m_strForceVersion = req.m_configuration.m_strForceVersion;
    configuration.m_strFileName = req.m_configuration.m_strFileName;
    configuration.m_strFileID = req.m_configuration.m_strFileID;
    configuration.m_uiFileSize = req.m_configuration.m_uiFileSize;
    configuration.m_strFilePath = "http://" + req.m_configuration.m_strServerAddress + "/filemgr.cgi?action=download_file&fileid=" + req.m_configuration.m_strFileID;
    configuration.m_uiLeaseDuration = req.m_configuration.m_uiLeaseDuration;
    //configuration.m_strUpdateDate = req.m_configuration.m_strUpdateDate;
    configuration.m_strUpdateDate = strCurrentTime;
    configuration.m_uiStatus = NORMAL_STATUS;
    configuration.m_strExtend = "";

    m_DBRuner.Post(boost::bind(&AccessManager::InsertConfigurationToDB, this, configuration));

    blResult = true;

    return blResult;
}

bool AccessManager::DeleteConfigurationReqMgr(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::DeleteConfigurationReq_MGR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        InteractiveProtoHandler::DeleteConfigurationRsp_MGR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::DeleteConfigurationRsp_MGR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete configuration item rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Delete configuration item rsp already send, dst id is " << strSrcID <<
            " and category is " << req.m_strCategory <<
            " and sub category is " << req.m_strSubCategory <<
            " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Delete configuration item req unserialize failed, src id is " << strSrcID);
        return false;
    }

    m_DBRuner.Post(boost::bind(&AccessManager::DeleteConfigurationToDB, this, req.m_strCategory, req.m_strSubCategory, DELETE_STATUS));

    blResult = true;

    return blResult;
}

bool AccessManager::ModifyConfigurationReqMgr(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::ModifyConfigurationReq_MGR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        InteractiveProtoHandler::ModifyConfigurationRsp_MGR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::ModifyConfigurationRsp_MGR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify configuration item rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Modify configuration item rsp already send, dst id is " << strSrcID <<
            " and category is " << req.m_configuration.m_strCategory <<
            " and sub category is " << req.m_configuration.m_strSubCategory <<
            " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Modify configuration item req unserialize failed, src id is " << strSrcID);
        return false;
    }

    m_DBRuner.Post(boost::bind(&AccessManager::ModifyConfigurationToDB, this, req.m_configuration));

    blResult = true;

    return blResult;
}

bool AccessManager::QueryAllConfigurationReqMgr(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::QueryAllConfigurationReq_MGR req;
    std::list<InteractiveProtoHandler::Configuration> configurationList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &configurationList)
    {
        InteractiveProtoHandler::QueryAllConfigurationRsp_MGR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryAllConfigurationRsp_MGR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_configurationList.swap(configurationList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all configuration item rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query all configuration item rsp already send, dst id is " << strSrcID <<
            " and begin index is " << req.m_uiBeginIndex <<
            " and result is " << blResult);

        if (blResult)
        {
            int i = 0;
            for (auto &configuration : rsp.m_configurationList)
            {
                LOG_INFO_RLD("Configuration item[" << i << "]: "
                    " and category is " << configuration.m_strCategory <<
                    " and sub category is " << configuration.m_strSubCategory <<
                    " and latest version is " << configuration.m_strLatestVersion <<
                    " and description is " << configuration.m_strDescription <<
                    " and force version is " << configuration.m_strForceVersion <<
                    " and file server address is " << configuration.m_strServerAddress <<
                    " and file name is " << configuration.m_strFileName <<
                    " and file id is " << configuration.m_strFileID << 
                    " and file size is " << configuration.m_uiFileSize << 
                    " and file path is " << configuration.m_strFilePath <<
                    " and lease duration is " << configuration.m_uiLeaseDuration << 
                    " and update date is " << configuration.m_strUpdateDate);

                ++i;
            }
        }
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query all configuration item req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryAllConfigurationToDB(configurationList, req.m_uiBeginIndex))
    {
        LOG_ERROR_RLD("Query all configuration item failed, src id is " << strSrcID);
    }

    blResult = true;

    return blResult;
}

bool AccessManager::ModifyDevicePropertyReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::ModifyDevicePropertyReq_DEV req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        InteractiveProtoHandler::ModifyDevicePropertyRsp_DEV rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::ModifyDevicePropertyRsp_DEV_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify device property rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Modify device property rsp already send, dst id is " << strSrcID <<
            " and device id is " << req.m_strDeviceID <<
            " and domain name is " << req.m_strDomainName <<
            " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Modify device property req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!req.m_strDomainName.empty() && !IsValidDeviceDomain(req.m_strDeviceID, req.m_strDomainName))
    {
        LOG_ERROR_RLD("Modify device property vefiry device domain name failed, device id is " << req.m_strDeviceID <<
            " and domain name is " << req.m_strDomainName);

        ReturnInfo::RetCode(ReturnInfo::DEVICE_DOMAIN_USED_DEV);
        return false;
    }

    if (!req.m_strP2pID.empty() && !IsValidP2pIDProperty(req.m_strDeviceID, req.m_strP2pID))
    {
        LOG_ERROR_RLD("Modify device property vefiry device p2p id failed, device id is " << req.m_strDeviceID <<
            " and p2p id is " << req.m_strP2pID);

        ReturnInfo::RetCode(ReturnInfo::DEVICE_P2PID_USED_DEV);
        return false;
    }

    if (DEVICE_TYPE_DOORBELL == req.m_uiDeviceType)
    {
        m_DBRuner.Post(boost::bind(&AccessManager::UpdateDoorbellParameterToDB, this, req.m_strDeviceID, req.m_doorbellParameter));
    }
    else if(DEVICE_TYPE_IPC == req.m_uiDeviceType)
    {
        m_DBRuner.Post(boost::bind(&AccessManager::UpdateDevicePropertyToDB, this, req));
    }
    else
    {
        LOG_ERROR_RLD("Modify device property failed, device type is invalid: " << req.m_uiDeviceType << " and src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool AccessManager::QueryDeviceParameterReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::QueryDeviceParameterReq_DEV req;
    InteractiveProtoHandler::DoorbellParameter doorbellParameter;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &doorbellParameter)
    {
        InteractiveProtoHandler::QueryDeviceParameterRsp_DEV rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryDeviceParameterRsp_DEV_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_doorbellParameter.m_strDoorbellName = doorbellParameter.m_strDoorbellName;
            rsp.m_doorbellParameter.m_strSerialNumber = doorbellParameter.m_strSerialNumber;
            rsp.m_doorbellParameter.m_strDoorbellP2Pid = doorbellParameter.m_strDoorbellP2Pid;
            rsp.m_doorbellParameter.m_strBatteryCapacity = doorbellParameter.m_strBatteryCapacity;
            rsp.m_doorbellParameter.m_strChargingState = doorbellParameter.m_strChargingState;
            rsp.m_doorbellParameter.m_strWifiSignal = doorbellParameter.m_strWifiSignal;
            rsp.m_doorbellParameter.m_strVolumeLevel = doorbellParameter.m_strVolumeLevel;
            rsp.m_doorbellParameter.m_strVersionNumber = doorbellParameter.m_strVersionNumber;
            rsp.m_doorbellParameter.m_strChannelNumber = doorbellParameter.m_strChannelNumber;
            rsp.m_doorbellParameter.m_strCodingType = doorbellParameter.m_strCodingType;
            rsp.m_doorbellParameter.m_strPIRAlarmSwtich = doorbellParameter.m_strPIRAlarmSwtich;
            rsp.m_doorbellParameter.m_strDoorbellSwitch = doorbellParameter.m_strDoorbellSwitch;
            rsp.m_doorbellParameter.m_strPIRAlarmLevel = doorbellParameter.m_strPIRAlarmLevel;
            rsp.m_doorbellParameter.m_strPIRIneffectiveTime = doorbellParameter.m_strPIRIneffectiveTime;
            rsp.m_doorbellParameter.m_strCurrentWifi = doorbellParameter.m_strCurrentWifi;
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query device parameter rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query device parameter rsp already send, dst id is " << strSrcID <<
            " and device name is " << doorbellParameter.m_strDoorbellName <<
            " and serial number is " << doorbellParameter.m_strSerialNumber <<
            " and device p2pid is " << doorbellParameter.m_strDoorbellP2Pid <<
            " and battery capacity is " << doorbellParameter.m_strBatteryCapacity <<
            " and charging state is " << doorbellParameter.m_strChargingState <<
            " and wifi signal is " << doorbellParameter.m_strWifiSignal <<
            " and volume level is " << doorbellParameter.m_strVolumeLevel <<
            " and version number is " << doorbellParameter.m_strVersionNumber <<
            " and channel number is " << doorbellParameter.m_strChannelNumber <<
            " and coding type is " << doorbellParameter.m_strCodingType <<
            " and PIR alarm switch is " << doorbellParameter.m_strPIRAlarmSwtich <<
            " and doorbell switch is " << doorbellParameter.m_strDoorbellSwitch <<
            " and PIR alarm level is " << doorbellParameter.m_strPIRAlarmLevel <<
            " and PIR ineffective time is " << doorbellParameter.m_strPIRIneffectiveTime <<
            " and current wifi is " << doorbellParameter.m_strCurrentWifi <<
            " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query device parameter req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryDeviceParameterToDB(req.m_strDeviceID, req.m_uiDeviceType, req.m_strQueryType, doorbellParameter))
    {
        LOG_ERROR_RLD("Query device parameter failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool AccessManager::QueryIfP2pIDValidReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::QueryIfP2pIDValidReq_USR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        InteractiveProtoHandler::QueryIfP2pIDValidRsp_USR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryIfP2pIDValidRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query if p2pID valid rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query if p2pID valid rsp already send, dst id is " << strSrcID <<
            " and p2pid is " << req.m_strP2pID <<
            " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query if p2pID valid req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryIfP2pIDValidToDB(req.m_strP2pID))
    {
        LOG_ERROR_RLD("Query if p2pID valid failed, p2p id is " << req.m_strP2pID);
        return false;
    }

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

    for (auto &fileInfo : FileInfoList)
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

        for (auto &fileUrl : *pFileUrlList)
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
        for (auto &fileUrl : FileUrlList)
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
    const unsigned int uiBusinessType, const std::string &strBeginDate, const std::string &strEndDate,
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

        for (auto &fileInfo : *pFileInfoList)
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

        for (auto &fileInfo : FileInfoList)
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

    boost::unique_lock<boost::mutex> lock(m_domainMutex);
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

    const std::list<AccessDomainInfo> &DomainInfoList = itPos->second;

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
        
        //租约控制在一个范围内，而不是所有设备使用相同的租约，避免租约同时到期时，查询接入域名的并发量过大
        srand((unsigned)time(NULL));
        DomainInfo.uiLease += rand() % 10;

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

                //租约控制在一个范围内，而不是所有设备使用相同的租约，避免租约同时到期时，查询接入域名的并发量过大
                srand((unsigned)time(NULL));
                DomainInfo.uiLease += rand() % 10;

                return true;
            }

            ++i;
            ++itBegin;
        }

        LOG_ERROR_RLD("QueryAccessDomainInfoByArea failed, random number is out of boundary, country id is " << strCountryID << " and area id is " << strAreaID);
        return false;
    }
}

bool AccessManager::RefreshAccessDomainName()
{
    std::string strSql = "select countryid, areaid, domainname, leaseduration from t_access_domain_info where status = 0";

    std::string strCountryID;
    std::string strAreaID;
    std::string strDomainName;
    unsigned int uiLease;

    boost::unique_lock<boost::mutex> lock(m_domainMutex);
    m_AreaDomainMap.clear();

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
        LOG_ERROR_RLD("RefreshAccessDomainName sql failed, sql is " << strSql);
        return false;
    }

    return true;
}

bool AccessManager::QueryUpgradeSiteToDB(std::string &strUpgradeUrl, unsigned int &uiLease)
{
    char sql[256] = { 0 };
    const char *sqlfmt = "select description, leaseduration from t_configuration_info where category = '%s' and subcategory = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, "Doorbell", "Firmware_upgrade_address");

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        switch (uiColumnNum)
        {
        case 0:
            strUpgradeUrl = strColumn;
            Result = strUpgradeUrl;
            break;
        case 1:
            uiLease = boost::lexical_cast<unsigned int>(strColumn);
            Result = uiLease;
            break;

        default:
            LOG_ERROR_RLD("QueryUpgradeSiteToDB sql callback error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("QueryUpgradeSiteToDB exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryUpgradeSiteToDB sql result is empty, sql is " << sql);
        return false;
    }

    strUpgradeUrl = boost::any_cast<std::string>(ResultList.front());
    uiLease = boost::any_cast<unsigned int>(ResultList.back());

    return true;
}

bool AccessManager::QueryAppUpgradeToDB(const std::string &strCategory, const std::string &strSubCategory, const std::string &strCurrentVersion,
    InteractiveProtoHandler::AppUpgrade &appUpgrade)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "select latestversion, description, forceversion, filename, filesize, filepath, updatedate from t_configuration_info"
        " where category = '%s' and subcategory = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strCategory.c_str(), strSubCategory.c_str());

    std::list<boost::any> ResultList;
    boost::shared_ptr<InteractiveProtoHandler::Configuration> pConfiguration(new InteractiveProtoHandler::Configuration);
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, boost::bind(&AccessManager::ConfigurationInfoSqlCB, this, _1, _2, _3, _4, pConfiguration)))
    {
        LOG_ERROR_RLD("QueryAppUpgradeToDB exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("QueryAppUpgradeToDB sql result is empty, there is no new version available");

        appUpgrade.m_uiNewVersionValid = NEW_VERSION_INVALID;
        appUpgrade.m_uiAppSize = 0;
        appUpgrade.m_uiForceUpgrade = INTERACTIVE_UPGRADE;
    }
    else
    {
        auto pResult = boost::any_cast<boost::shared_ptr<InteractiveProtoHandler::Configuration>>(ResultList.front());
        appUpgrade.m_uiNewVersionValid = pResult->m_strLatestVersion.compare(strCurrentVersion) > 0 ? NEW_VERSION_VALID : NEW_VERSION_INVALID;

        if (NEW_VERSION_VALID == appUpgrade.m_uiNewVersionValid)
        {
            appUpgrade.m_strAppName = pResult->m_strFileName;
            appUpgrade.m_strAppPath = pResult->m_strFilePath;
            appUpgrade.m_uiAppSize = pResult->m_uiFileSize;
            appUpgrade.m_strVersion = pResult->m_strLatestVersion;
            appUpgrade.m_strDescription = pResult->m_strDescription;
            appUpgrade.m_uiForceUpgrade = INTERACTIVE_UPGRADE;
            appUpgrade.m_strUpdateDate = pResult->m_strUpdateDate;

            LOG_INFO_RLD("QueryAppUpgradeToDB successful, found new version, version is " << appUpgrade.m_strVersion <<
                " and app name is " << appUpgrade.m_strAppName << " and app path is " << appUpgrade.m_strAppPath <<
                " and app size is " << appUpgrade.m_uiAppSize << " and latest version is " << appUpgrade.m_strVersion <<
                " and force upgrade is " << appUpgrade.m_uiForceUpgrade << " and update date is " << appUpgrade.m_strUpdateDate);
        }
        else
        {
            appUpgrade.m_uiAppSize = 0;
            appUpgrade.m_uiForceUpgrade = INTERACTIVE_UPGRADE;

            LOG_INFO_RLD("QueryAppUpgradeToDB successful, current version is latest, version is " << strCurrentVersion);
        }
    }
    
    return true;
}

bool AccessManager::QueryFirwareUpgradeToDB(const std::string &strCategory, const std::string &strSubCategory, const std::string &strCurrentVersion,
    InteractiveProtoHandler::FirmwareUpgrade &firmwareUpgrade)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "select latestversion, description, forceversion, filename, filesize, filepath, updatedate from t_configuration_info"
        " where category = '%s' and subcategory = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strCategory.c_str(), strSubCategory.c_str());

    std::list<boost::any> ResultList;
    boost::shared_ptr<InteractiveProtoHandler::Configuration> pConfiguration(new InteractiveProtoHandler::Configuration);
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, boost::bind(&AccessManager::ConfigurationInfoSqlCB, this, _1, _2, _3, _4, pConfiguration)))
    {
        LOG_ERROR_RLD("QueryFirwareUpgradeToDB exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("QueryFirwareUpgradeToDB sql result is empty, there is no new version available");

        firmwareUpgrade.m_uiNewVersionValid = NEW_VERSION_INVALID;
        firmwareUpgrade.m_uiFirmwareSize = 0;
        firmwareUpgrade.m_uiForceUpgrade = INTERACTIVE_UPGRADE;
    }
    else
    {
        auto pResult = boost::any_cast<boost::shared_ptr<InteractiveProtoHandler::Configuration>>(ResultList.front());
        //固件升级允许出现版本回退的情况
        firmwareUpgrade.m_uiNewVersionValid = pResult->m_strLatestVersion.compare(strCurrentVersion) != 0 ? NEW_VERSION_VALID : NEW_VERSION_INVALID;

        if (NEW_VERSION_VALID == firmwareUpgrade.m_uiNewVersionValid)
        {
            firmwareUpgrade.m_strFirmwareName = pResult->m_strFileName;
            firmwareUpgrade.m_strFirmwarePath = pResult->m_strFilePath;
            firmwareUpgrade.m_uiFirmwareSize = pResult->m_uiFileSize;
            firmwareUpgrade.m_strVersion = pResult->m_strLatestVersion;
            firmwareUpgrade.m_strDescription = pResult->m_strDescription;
            firmwareUpgrade.m_uiForceUpgrade = pResult->m_strForceVersion == strCurrentVersion ? FORCE_UPGRADE : INTERACTIVE_UPGRADE;
            firmwareUpgrade.m_strUpdateDate = pResult->m_strUpdateDate;

            LOG_INFO_RLD("QueryFirwareUpgradeToDB successful, found new version, version is " << firmwareUpgrade.m_strVersion <<
                " and firmware name is " << firmwareUpgrade.m_strFirmwareName << " and firmware path is " << firmwareUpgrade.m_strFirmwarePath <<
                " and firmware size is " << firmwareUpgrade.m_uiFirmwareSize << " and latest version is " << firmwareUpgrade.m_strVersion <<
                " and force upgrade is " << firmwareUpgrade.m_uiForceUpgrade << " and update date is " << firmwareUpgrade.m_strUpdateDate);
        }
        else
        {
            firmwareUpgrade.m_uiFirmwareSize = 0;
            firmwareUpgrade.m_uiForceUpgrade = INTERACTIVE_UPGRADE;

            LOG_INFO_RLD("QueryFirwareUpgradeToDB successful, current version is latest, version is " << strCurrentVersion);
        }
    }

    return true;
}

void AccessManager::ConfigurationInfoSqlCB(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn,
    boost::any &Result, boost::shared_ptr<InteractiveProtoHandler::Configuration> pConfiguration)
{
    switch (uiColumnNum)
    {
    case 0:
        pConfiguration->m_strLatestVersion = strColumn;
        break;
    case 1:
        pConfiguration->m_strDescription = strColumn;
        break;
    case 2:
        pConfiguration->m_strForceVersion = strColumn;
        break;
    case 3:
        pConfiguration->m_strFileName = strColumn;
        break;
    case 4:
        pConfiguration->m_uiFileSize = boost::lexical_cast<unsigned int>(strColumn);
        break;
    case 5:
        pConfiguration->m_strFilePath = strColumn;
        break;
    case 6:
        pConfiguration->m_strUpdateDate = strColumn;
        Result = pConfiguration;
        break;

    default:
        LOG_ERROR_RLD("ConfigurationInfoSqlCB sql callback error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
        break;
    }
}

bool AccessManager::IsValidConfiguration(const std::string &strCategory, const std::string &strSubCategory)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select count(id) from t_configuration_info where category = '%s' and subcategory = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strCategory.c_str(), strSubCategory.c_str());

    unsigned int uiResult;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        uiResult = boost::lexical_cast<unsigned int>(strColumn);
        Result = uiResult;

        LOG_INFO_RLD("IsValidConfiguration exec sql count(id) is " << uiResult);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("IsValidConfiguration exec sql error, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("IsValidConfiguration sql result is empty, category is " << strCategory << " and sub category is " << strSubCategory);
        return false;
    }

    uiResult = boost::any_cast<unsigned int>(ResultList.front());

    if (uiResult > 0)
    {
        LOG_ERROR_RLD("IsValidConfiguration failed, this configuration is existed, category is " << strCategory << " and sub category is " << strSubCategory);
        return false;
    }
    else
    {
        LOG_INFO_RLD("IsValidConfiguration successful, category is " << strCategory << " and sub category is " << strSubCategory);
        return true;
    }
}

bool AccessManager::GetTimeZone(const std::string &strIpAddress, std::string &strCountryCode, std::string &strCountryNameEn,
    std::string &strCountryNameZh, std::string &strTimeZone)
{
    TimeZone timeZone;
    CTimeZone cTimeZone;
    cTimeZone.setpostUrl(m_ParamInfo.m_strGetIpInfoSite);
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

void AccessManager::InsertConfigurationToDB(const InteractiveProtoHandler::Configuration &configuration)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_configuration_info"
        " values(uuid(), '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d, '%s', %d, '%s', %d, '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, configuration.m_strCategory.c_str(), configuration.m_strSubCategory.c_str(),
        configuration.m_strLatestVersion.c_str(), configuration.m_strDescription.c_str(), configuration.m_strForceVersion.c_str(),
        configuration.m_strFileName.c_str(), configuration.m_strFileID.c_str(), configuration.m_uiFileSize, configuration.m_strFilePath.c_str(),
        configuration.m_uiLeaseDuration, configuration.m_strUpdateDate.c_str(), configuration.m_uiStatus, configuration.m_strExtend.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("InsertConfigurationToDB exec sql failed, sql is " << sql);
    }
}

void AccessManager::DeleteConfigurationToDB(const std::string &strCategory, const std::string &strSubCategory, const int iStatus)
{
    if (!DeleteUpgradeFile(strCategory, strSubCategory))
    {
        LOG_ERROR_RLD("DeleteConfigurationToDB failed, delete upgrade file error, category is " << strCategory << " and sub category is " << strSubCategory);
        return;
    }

    char sql[1024] = { 0 };
    const char *sqlfmt = "update t_configuration_info set status = '%d' where category = '%s' and subcategory = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, iStatus, strCategory.c_str(), strSubCategory.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteConfigurationToDB exec sql failed, sql is " << sql);
    }
}

bool AccessManager::DeleteUpgradeFile(const std::string &strCategory, const std::string &strSubCategory)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select fileid, filepath from t_configuration_info where category = '%s' and subcategory = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strCategory.c_str(), strSubCategory.c_str());

    std::string strFileID;
    std::string strFilePath;

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            strFileID = strColumn;
            break;
        case 1:
            strFilePath = strColumn;
            break;

        default:
            LOG_ERROR_RLD("QueryAllConfigurationToDB sql callback error, row num is " <<
                uiRowNum << " and column num is " << uiColumnNum << " and value is " << strColumn);
            break;
        }

        result = strColumn;
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("DeleteUpgradeFile exec sql error, sql is " << sql);
        return false;
    }

    strFileID = boost::any_cast<std::string>(ResultList.front());
    strFilePath = boost::any_cast<std::string>(ResultList.back());

    std::map<std::string, std::string> reqFormMap;
    reqFormMap.insert(std::make_pair("fileid", strFileID));

    std::string strUrl = strFilePath.substr(0, strFilePath.find_first_of("?") + 1) + "action=delete_file";

    std::string strRsp;
    HttpClient httpClient;
    if (CURLE_OK != httpClient.PostForm(strUrl, reqFormMap, strRsp))
    {
        LOG_ERROR_RLD("DeleteUpgradeFile send http post failed, url is " << strUrl << " and file id is " << strFileID);
        return false;
    }

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(strRsp, root))
    {
        LOG_ERROR_RLD("DeleteUpgradeFile failed, parse http post response data error, raw data is: " << strRsp);
        return false;
    }

    auto retcode = root["retcode"];
    if (retcode.isNull() || !retcode.isString())
    {
        LOG_ERROR_RLD("DeleteUpgradeFile failed, http post response data is illegal, raw data is: " << strRsp);
        return false;
    }

    if ("0" != retcode.asString())
    {
        LOG_ERROR_RLD("DeleteUpgradeFile failed, http post return error, raw data is: " << strRsp);
        return false;
    }

    LOG_INFO_RLD("DeleteUpgradeFile successful, post url is " << strUrl << " and file id is " << strFileID);
    return true;
}

void AccessManager::ModifyConfigurationToDB(const InteractiveProtoHandler::Configuration &configuration)
{
    bool blModified = false;

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len;
    snprintf(sql, size, "update t_configuration_info set id = id");

    if (!configuration.m_strLatestVersion.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", latestversion = '%s'", configuration.m_strLatestVersion.c_str());

        blModified = true;
    }

    if (!configuration.m_strDescription.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", description = '%s'", configuration.m_strDescription.c_str());

        blModified = true;
    }

    if (!configuration.m_strForceVersion.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", forceversion = '%s'", configuration.m_strForceVersion.c_str());

        blModified = true;
    }

    if (!configuration.m_strFileName.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", filename = '%s'", configuration.m_strFileName.c_str());

        blModified = true;
    }

    //if (!configuration.m_strServerAddress.empty() && !configuration.m_strFileID.empty())
    //{
    //    len = strlen(sql);
    //    snprintf(sql + len, size - len, ", fileid = '%s', filesize = %d, filepath = '%s'", configuration.m_strFileID.c_str(), configuration.m_uiFileSize,
    //        ("http://" + configuration.m_strServerAddress + "/filemgr.cgi?action=download_file&fileid=" + configuration.m_strFileID).c_str());

    //    blModified = true;
    //}

    if (0xFFFFFFFF != configuration.m_uiLeaseDuration)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", leaseduration = %d", configuration.m_uiLeaseDuration);

        blModified = true;
    }

    if (!configuration.m_strUpdateDate.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", updatedate = '%s'", configuration.m_strUpdateDate.c_str());

        blModified = true;
    }

    if (!blModified)
    {
        LOG_INFO_RLD("ModifyConfigurationToDB completed, there is no change");
        return;
    }

    len = strlen(sql);
    snprintf(sql + len, size - len, " where category = '%s' and subcategory = '%s' and status = 0",
        configuration.m_strCategory.c_str(), configuration.m_strSubCategory.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("ModifyConfigurationToDB exec sql failed, sql is " << sql);
    }
}

bool AccessManager::QueryAllConfigurationToDB(std::list<InteractiveProtoHandler::Configuration> &configurationList,
    const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 10*/)
{
    char sql[256] = { 0 };
    const char *sqlfmt = "select category, subcategory, latestversion, description, forceversion, filename, filesize, filepath, leaseduration, updatedate"
        " from t_configuration_info where status = 0 limit %u, %u";
    snprintf(sql, sizeof(sql), sqlfmt, uiBeginIndex, uiPageSize);

    InteractiveProtoHandler::Configuration configuration;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            configuration.m_strCategory = strColumn;
            break;
        case 1:
            configuration.m_strSubCategory = strColumn;
            break;
        case 2:
            configuration.m_strLatestVersion = strColumn;
            break;
        case 3:
            configuration.m_strDescription = strColumn;
            break;
        case 4:
            configuration.m_strForceVersion = strColumn;
            break;
        case 5:
            configuration.m_strFileName = strColumn;
            break;
        case 6:
            configuration.m_uiFileSize = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 7:
            configuration.m_strFilePath = strColumn;
            break;
        case 8:
            configuration.m_uiLeaseDuration = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 9:
            configuration.m_strUpdateDate = strColumn;
            result = configuration;
            break;

        default:
            LOG_ERROR_RLD("QueryAllConfigurationToDB sql callback error, row num is " <<
                uiRowNum << " and column num is " << uiColumnNum << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("QueryAllConfigurationToDB exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("QueryAllConfigurationToDB sql result is empty, sql is " << sql);
        return true;
    }

    for (auto &result : ResultList)
    {
        configurationList.push_back(std::move(boost::any_cast<InteractiveProtoHandler::Configuration>(result)));
    }

    return true;
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
    const char* sqlfmt = "select count(rel.id) from t_user_device_relation rel, t_device_info dev where"
        " rel.userid = '%s' and rel.deviceid = '%s' and rel.relation = %d and rel.status = 0 and rel.devicekeyid = dev.id and dev.status = 0";
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
    std::list<std::string> &strDevNameList, const unsigned int uiBeginIndex, const unsigned int uiPageSize)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "select rel.userid, rel.deviceid, rel.relation, rel.begindate, rel.enddate, rel.createdate, rel.status, rel.extend, dev.devicename"
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

        boost::shared_ptr<std::list<std::string> > pStrDevNameList;
        pStrDevNameList = boost::any_cast<boost::shared_ptr<std::list<std::string> >>(ResultList.back());
        
        auto itBegin = pRelationList->begin();
        auto itEnd = pRelationList->end();
        while (itBegin != itEnd)
        {
            RelationList.push_back(*itBegin);
            ++itBegin;
        }

        auto itBeginDevName = pStrDevNameList->begin();
        auto itEndDevName = pStrDevNameList->end();
        while (itBeginDevName != itEndDevName)
        {
            strDevNameList.push_back(*itBeginDevName);
            ++itBeginDevName;
        }

        LOG_INFO_RLD("Query relation get result from cache and sql is " << strSql);
    }
    else
    {        
        if (!m_pMysql->QueryExec(strSql, boost::bind(&AccessManager::DevInfoRelationSqlCB, this, _1, _2, _3, &RelationList, &strDevNameList)))
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

        boost::shared_ptr<std::list<std::string> > pStrDevNameList(new std::list<std::string>);
        auto itBeginDevName = strDevNameList.begin();
        auto itEndDevName = strDevNameList.end();
        while (itBeginDevName != itEndDevName)
        {
            pStrDevNameList->push_back(*itBeginDevName);
            ++itBeginDevName;
        }

        boost::shared_ptr<std::list<boost::any> > pResultList(new std::list<boost::any>);
        pResultList->push_back(pRelationList);
        pResultList->push_back(pStrDevNameList);

        m_DBCache.SetResult(strSql, pResultList);

        LOG_INFO_RLD("Query relation get result from db and sql is " << strSql);
    }

    return true;
}

bool AccessManager::QueryRelationByDevID(const std::string &strDevID, std::list<InteractiveProtoHandler::Relation> &RelationList,
    std::list<std::string> &strUserNameList, const unsigned int uiBeginIndex, const unsigned int uiPageSize)
{
    char sql[1024] = { 0 };
    memset(sql, 0, sizeof(sql));
    const char *sqlft = "select rel.userid, rel.deviceid, rel.relation, rel.begindate, rel.enddate, rel.createdate, rel.status, rel.extend, usr.username"
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

        boost::shared_ptr<std::list<std::string> > pStrUserNameList;
        pStrUserNameList = boost::any_cast<boost::shared_ptr<std::list<std::string> >>(ResultList.back());

        auto itBegin = pRelationList->begin();
        auto itEnd = pRelationList->end();
        while (itBegin != itEnd)
        {
            RelationList.push_back(*itBegin);
            ++itBegin;
        }

        auto itBeginUserName = pStrUserNameList->begin();
        auto itEndUserName = pStrUserNameList->end();
        while (itBeginUserName != itEndUserName)
        {
            strUserNameList.push_back(*itBeginUserName);
            ++itBeginUserName;
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
            case 8:
                strUserNameList.push_back(strColumn);
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

        boost::shared_ptr<std::list<std::string> > pStrUserNameList(new std::list<std::string>);
        auto itBeginUserName = strUserNameList.begin();
        auto itEndUserName = strUserNameList.end();
        while (itBeginUserName != itEndUserName)
        {
            pStrUserNameList->push_back(*itBeginUserName);
            ++itBeginUserName;
        }

        boost::shared_ptr<std::list<boost::any> > pResultList(new std::list<boost::any>);
        pResultList->push_back(pRelationListTmp);
        pResultList->push_back(pStrUserNameList);

        m_DBCache.SetResult(strSql, pResultList);

        LOG_INFO_RLD("Query relation by device id get result from db and sql is " << strSql);
    }

    return true;
}

bool AccessManager::ValidUser(std::string &strUserID, std::string &strUserName, bool &blUserExist, const std::string &strUserPwd, const int iTypeInfo, const bool IsForceFromDB)
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

        blUserExist = false;
        return false;
    }

    blUserExist = true;

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
    std::list<InteractiveProtoHandler::Relation> *pRelationList, std::list<std::string> *pStrDevNameList)
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
    case 8:
        pStrDevNameList->push_back(strColumn);
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

    char sql[2048] = { 0 };
    const char* sqlfmt = "insert into t_device_info("
        "id,deviceid,devicename, devicepassword, typeinfo, createdate, status, innerinfo, extend, p2pid, domainname) values('%s',"
        "'%s','%s','%s','%d','%s', '%d', '%s', '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strUuid.c_str(), DevInfo.m_strDevID.c_str(), DevInfo.m_strDevName.c_str(), DevInfo.m_strDevPassword.c_str(), DevInfo.m_uiTypeInfo,
        DevInfo.m_strCreatedate.c_str(), DevInfo.m_uiStatus, strInner.c_str(), DevInfo.m_strExtend.c_str(), DevInfo.m_strP2pID.c_str(), DevInfo.m_strDomainName.c_str());

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

    if (!DevInfo.m_strP2pID.empty())
    {
        char cTmp[256] = { 0 };
        snprintf(cTmp, sizeof(cTmp), ", p2pid = '%s' ", DevInfo.m_strP2pID.c_str());
        strSql += cTmp;
    }

    if (!DevInfo.m_strDomainName.empty())
    {
        char cTmp[256] = { 0 };
        snprintf(cTmp, sizeof(cTmp), ", domainname = '%s' ", DevInfo.m_strDomainName.c_str());
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

        char cTmp[2048] = { 0 };
        snprintf(cTmp, sizeof(cTmp), ", innerinfo = '%s' ", strInner.c_str());
        strSql += cTmp;
    }

    if (!DevInfo.m_strExtend.empty())
    {
        char cTmp[2048] = { 0 };
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
    const char* sqlfmt = "select deviceid, devicename, devicepassword, typeinfo, createdate, status, innerinfo, extend, p2pid, domainname from t_device_info where deviceid = '%s' and status = 0";
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
        dev.m_strP2pID = Result.m_strP2pID;
        dev.m_strDomainName = Result.m_strDomainName;
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
            case 8:
                dev.m_strP2pID = strColumn;
                break;
            case 9:
                dev.m_strDomainName = strColumn;
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

bool AccessManager::IsValidP2pInfo(const unsigned int uiP2pSupplier, const std::string &strP2pID, const std::string &strDeviceID)
{
    char sql[1024] = { 0 };

    if (P2P_SUPPLIER_LT == uiP2pSupplier || P2P_SUPPLIER_TUTK == uiP2pSupplier)
    {
        const char *supplier;
        if (P2P_SUPPLIER_LT == uiP2pSupplier)
        {
            supplier = "LT";
        }
        else
        {
            supplier = "TUTK";
        }
 
        const char *sqlfmt = "select p2pid from t_p2pid_lt where p2pid = '%s' and supplier = '%s' and deviceid = '%s' and status = 0";
        snprintf(sql, sizeof(sql), sqlfmt, strP2pID.c_str(), supplier, strDeviceID.c_str());
    }
    else if (P2P_SUPPLIER_SY == uiP2pSupplier)
    {
        //目前尚云P2P不做上报的操作，不做校验操作，直接返回false
        //const char *sqlfmt = "select p2pid from t_p2pid_sy where p2pid = '%s' and deviceid = '%s' and status = 0";
        //snprintf(sql, sizeof(sql), sqlfmt, strP2pID.c_str(), strDeviceID.c_str());
        LOG_INFO_RLD("IsValidP2pInfo completed, skip shangyun p2p info");
        return false;
    }
    else
    {
        LOG_ERROR_RLD("IsValidP2pInfo failed, unknown p2p supplier, supplier is " << uiP2pSupplier);
        return false;
    }

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        Result = strColumn;

        LOG_INFO_RLD("IsValidP2pInfo sql p2pid is " << strColumn);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("IsValidP2pInfo exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("IsValidP2pInfo successful, the supplier is " << uiP2pSupplier <<
            " and p2pid is " << strP2pID << " and device id is " << strDeviceID);
        return true;
    }
    else
    {
        LOG_ERROR_RLD("IsValidP2pInfo failed, the p2pid is added, supplier is " << uiP2pSupplier <<
            " and p2pid is " << strP2pID << " and device id is " << strDeviceID);
        return false;
    }
}

void AccessManager::InsertP2pInfoToDB(const unsigned int uiP2pSupplier, const std::string &strP2pID, const std::string &strDeviceID, const unsigned int uiBuildin)
{
    if (!IsValidP2pInfo(uiP2pSupplier, strP2pID, strDeviceID))
    {
        LOG_INFO_RLD("InsertP2pInfoToDB completed, the p2pid info is invalid, p2pid is " << strP2pID << " and supplier is " << uiP2pSupplier);
        return;
    }

    char sql[1024] = { 0 };

    if (P2P_SUPPLIER_LT == uiP2pSupplier || P2P_SUPPLIER_TUTK == uiP2pSupplier)
    {
        const char *supplier;
        if (P2P_SUPPLIER_LT == uiP2pSupplier)
        {
            supplier = "LT";
        }
        else
        {
            supplier = "TUTK";
        }

        const char *sqlfmt = "insert into t_p2pid_lt (id, buildin, supplier, p2pid, deviceid) values(uuid(), %d, '%s', '%s', '%s')";
        snprintf(sql, sizeof(sql), sqlfmt, uiBuildin, supplier, strP2pID.c_str(), strDeviceID.c_str());
    }
    else if (P2P_SUPPLIER_SY == uiP2pSupplier)
    {
        //目前尚云P2P不做上报的操作，不做插表操作，直接返回
        LOG_INFO_RLD("InsertP2pInfoToDB completed, skip shangyun p2p info");
        return;
    }
    else
    {
        LOG_ERROR_RLD("InsertP2pInfoToDB failed, unknown p2p supplier, supplier is " << uiP2pSupplier);
        return;
    }

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("InsertP2pInfoToDB exec sql failed, sql is " << sql);
    }
}

bool AccessManager::IsValidDeviceDomain(const std::string &strDeviceID, const std::string &strDeviceDomain)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select deviceid from t_device_parameter_ipc where deviceid != '%s' and devicedomain = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strDeviceID.c_str(), strDeviceDomain.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        Result = strColumn;
        LOG_INFO_RLD("IsValidDeviceDomain sql deviceid is " << strColumn);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("IsValidDeviceDomain exec sql failed, sql is " << sql);
        return false;
    }

    if (!ResultList.empty())
    {
        LOG_ERROR_RLD("IsValidDeviceDomain failed, the device domain name is used: " << strDeviceDomain);
        return false;
    }

    return true;
}

bool AccessManager::IsValidP2pIDProperty(const std::string &strDeviceID, const std::string &strP2pID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select deviceid from t_device_parameter_ipc where deviceid != '%s' and p2pid = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strDeviceID.c_str(), strP2pID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        Result = strColumn;
        LOG_INFO_RLD("IsValidP2pIDProperty sql deviceid is " << strColumn);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("IsValidP2pIDProperty exec sql failed, sql is " << sql);
        return false;
    }

    if (!ResultList.empty())
    {
        LOG_ERROR_RLD("IsValidP2pIDProperty failed, the device p2pid is used: " << strP2pID);
        return false;
    }

    return true;
}

void AccessManager::InsertDevPropertyToDB(const InteractiveProtoHandler::LoginReq_DEV &loginDevReq)
{
    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len;

    const char *sqlfmt = "insert into t_device_parameter_ipc (id, deviceid, devicepassword, devicedomain, typeinfo, username, userpassword,"
        " p2pid, p2pserver, p2psupplier, p2pbuildin, licensekey, pushid, distributor, otherproperty, createdate, status, extend)"
        " values(uuid(), '%s', '%s', '%s', %d, '%s', '%s', '%s', '%s', %d, %d, '%s', '%s', '%s', '%s', '%s', %d, '%s')"
        " on duplicate key update id = id";
    snprintf(sql, size, sqlfmt, loginDevReq.m_strDevID.c_str(), loginDevReq.m_strPassword.c_str(), loginDevReq.m_strDomainName.c_str(), loginDevReq.m_uiDeviceType,
        loginDevReq.m_strUserName.c_str(), loginDevReq.m_strUserPassword.c_str(), loginDevReq.m_strP2pID.c_str(), loginDevReq.m_strP2pServr.c_str(),
        loginDevReq.m_uiP2pSupplier, loginDevReq.m_uiP2pBuildin, loginDevReq.m_strLicenseKey.c_str(), loginDevReq.m_strPushID.c_str(),
        loginDevReq.m_strDistributor.c_str(), loginDevReq.m_strOtherProperty.c_str(), strCurrentTime.c_str(), NORMAL_STATUS, "");

    if (!loginDevReq.m_strPassword.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", devicepassword = '%s'", loginDevReq.m_strPassword.c_str());
    }

    if (!loginDevReq.m_strDomainName.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", devicedomain = '%s'", loginDevReq.m_strDomainName.c_str());
    }

    if (0xFFFFFFFF != loginDevReq.m_uiDeviceType)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", typeinfo = %d", loginDevReq.m_uiDeviceType);
    }

    if (!loginDevReq.m_strUserName.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", username = '%s'", loginDevReq.m_strUserName.c_str());
    }

    if (!loginDevReq.m_strUserPassword.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", userpassword = '%s'", loginDevReq.m_strUserPassword.c_str());
    }

    if (!loginDevReq.m_strP2pID.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", p2pid = '%s'", loginDevReq.m_strP2pID.c_str());
    }

    if (!loginDevReq.m_strP2pServr.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", p2pserver = '%s'", loginDevReq.m_strP2pServr.c_str());
    }

    if (0xFFFFFFFF != loginDevReq.m_uiP2pSupplier)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", p2psupplier = %d", loginDevReq.m_uiP2pSupplier);
    }

    if (0xFFFFFFFF != loginDevReq.m_uiP2pBuildin)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", p2pbuildin = %d", loginDevReq.m_uiP2pBuildin);
    }

    if (!loginDevReq.m_strLicenseKey.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", licensekey = '%s'", loginDevReq.m_strLicenseKey.c_str());
    }

    if (!loginDevReq.m_strPushID.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", pushid = '%s'", loginDevReq.m_strPushID.c_str());
    }

    if (!loginDevReq.m_strDistributor.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", distributor = '%s'", loginDevReq.m_strDistributor.c_str());
    }

    if (!loginDevReq.m_strOtherProperty.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", otherproperty = '%s'", loginDevReq.m_strOtherProperty.c_str());
    }

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("InsertDevPropertyToDB exec sql error, sql is " << sql);
    }
}

void AccessManager::UpdateDevicePropertyToDB(const InteractiveProtoHandler::ModifyDevicePropertyReq_DEV &modifyDevicePropertyReq)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len;
    snprintf(sql, size, "update t_device_parameter_ipc set updatedate = current_time");

    bool blModified = false;

    if (!modifyDevicePropertyReq.m_strDomainName.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", devicedomain = '%s'", modifyDevicePropertyReq.m_strDomainName.c_str());

        blModified = true;
    }

    if (!modifyDevicePropertyReq.m_strP2pID.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", p2pid = '%s'", modifyDevicePropertyReq.m_strP2pID.c_str());

        blModified = true;
    }

    if (!modifyDevicePropertyReq.m_strCorpID.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", corpid = '%s'", modifyDevicePropertyReq.m_strCorpID.c_str());

        blModified = true;
    }

    if (!modifyDevicePropertyReq.m_strDeviceName.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", dvsname = '%s'", modifyDevicePropertyReq.m_strDeviceName.c_str());

        blModified = true;
    }

    if (!modifyDevicePropertyReq.m_strDeviceIP.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", dvsip = '%s'", modifyDevicePropertyReq.m_strDeviceIP.c_str());

        blModified = true;
    }

    if (!modifyDevicePropertyReq.m_strDeviceIP2.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", dvsip2 = '%s'", modifyDevicePropertyReq.m_strDeviceIP2.c_str());

        blModified = true;
    }

    if (!modifyDevicePropertyReq.m_strWebPort.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", webport = '%s'", modifyDevicePropertyReq.m_strWebPort.c_str());

        blModified = true;
    }

    if (!modifyDevicePropertyReq.m_strCtrlPort.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", ctrlport = '%s'", modifyDevicePropertyReq.m_strCtrlPort.c_str());

        blModified = true;
    }

    if (!modifyDevicePropertyReq.m_strProtocol.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", protocol = '%s'", modifyDevicePropertyReq.m_strProtocol.c_str());

        blModified = true;
    }

    if (!modifyDevicePropertyReq.m_strModel.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", model = '%s'", modifyDevicePropertyReq.m_strModel.c_str());

        blModified = true;
    }

    if (!modifyDevicePropertyReq.m_strPostFrequency.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", postfrequency = '%s'", modifyDevicePropertyReq.m_strPostFrequency.c_str());

        blModified = true;
    }

    if (!modifyDevicePropertyReq.m_strVersion.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", version = '%s'", modifyDevicePropertyReq.m_strVersion.c_str());

        blModified = true;
    }

    if (!modifyDevicePropertyReq.m_strDeviceStatus.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", devicestatus = '%s'", modifyDevicePropertyReq.m_strDeviceStatus.c_str());

        blModified = true;
    }

    if (!modifyDevicePropertyReq.m_strServerIP.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", serverip = '%s'", modifyDevicePropertyReq.m_strServerIP.c_str());

        blModified = true;
    }

    if (!modifyDevicePropertyReq.m_strServerPort.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", serverport = '%s'", modifyDevicePropertyReq.m_strServerPort.c_str());

        blModified = true;
    }

    if (!modifyDevicePropertyReq.m_strTransfer.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", transfer = '%s'", modifyDevicePropertyReq.m_strTransfer.c_str());

        blModified = true;
    }

    if (!modifyDevicePropertyReq.m_strMobilePort.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", mobileport = '%s'", modifyDevicePropertyReq.m_strMobilePort.c_str());

        blModified = true;
    }

    if (!modifyDevicePropertyReq.m_strChannelCount.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", channelcount = '%s'", modifyDevicePropertyReq.m_strChannelCount.c_str());

        blModified = true;
    }

    if (!blModified)
    {
        LOG_INFO_RLD("UpdateDevicePropertyToDB completed, there is no change");
        return;
    }

    len = strlen(sql);
    snprintf(sql + len, size - len, " where deviceid = '%s'", modifyDevicePropertyReq.m_strDeviceID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("UpdateDevicePropertyToDB exec sql failed, sql is " << sql);
    }
}

bool AccessManager::QueryDevPropertyByDevDomain(const std::string &strDeviceDomain, std::string &strDeviceID, std::string &strP2pID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select deviceid, p2pid from t_device_parameter_ipc where devicedomain = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strDeviceDomain.c_str());

    struct DevProperty
    {
        std::string strDeviceID;
        std::string strP2pID;
    };
    DevProperty devProperty;

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        switch (uiColumnNum)
        {
        case 0:
            devProperty.strDeviceID = strColumn;
            break;
        case 1:
            devProperty.strP2pID = strColumn;
            Result = devProperty;
            break;

        default:
            LOG_ERROR_RLD("QueryDevPropertyByDevDomain sql callback error, row num is " <<
                uiRowNum << " and column num is " << uiColumnNum << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryDevPropertyByDevDomain exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryDevPropertyByDevDomain sql result is empty, sql is " << sql);
    }
    else
    {
        auto result = boost::any_cast<DevProperty>(ResultList.front());
        strDeviceID = result.strDeviceID;
        strP2pID = result.strP2pID;
    }

    return true;
}

bool AccessManager::QueryDevIDByDevP2pID(const std::string &strDeviceP2pID, std::string &strDeviceID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select deviceid from t_device_parameter_ipc where p2pid = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strDeviceP2pID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        strDeviceID = strColumn;
        Result = strDeviceID;

        LOG_INFO_RLD("QueryDevIDByDevP2pID sql deviceid is " << strDeviceID);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryDevIDByDevP2pID exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryDevIDByDevP2pID sql result is empty, sql is " << sql);
    }
    else
    {
        strDeviceID = boost::any_cast<std::string>(ResultList.front());
    }

    return true;
}

void AccessManager::InsertDoorbellParameterToDB(const InteractiveProtoHandler::LoginReq_DEV &loginDevReq)
{
    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len;

    const char *sqlfmt = "insert into t_device_parameter_doorbell (id, deviceid, devicepassword, devicedomain, typeinfo, username, userpassword,"
        " doorbell_p2pid, p2pserver, p2psupplier, p2pbuildin, licensekey, pushid, distributor, otherproperty, createdate, status, extend)"
        " values(uuid(), '%s', '%s', '%s', %d, '%s', '%s', '%s', '%s', %d, %d, '%s', '%s', '%s', '%s', '%s', %d, '%s')"
        " on duplicate key update id = id";
    snprintf(sql, size, sqlfmt, loginDevReq.m_strDevID.c_str(), loginDevReq.m_strPassword.c_str(), loginDevReq.m_strDomainName.c_str(), loginDevReq.m_uiDeviceType,
        loginDevReq.m_strUserName.c_str(), loginDevReq.m_strUserPassword.c_str(), loginDevReq.m_strP2pID.c_str(), loginDevReq.m_strP2pServr.c_str(),
        loginDevReq.m_uiP2pSupplier, loginDevReq.m_uiP2pBuildin, loginDevReq.m_strLicenseKey.c_str(), loginDevReq.m_strPushID.c_str(),
        loginDevReq.m_strDistributor.c_str(), loginDevReq.m_strOtherProperty.c_str(), strCurrentTime.c_str(), NORMAL_STATUS, "");

    if (!loginDevReq.m_strPassword.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", devicepassword = '%s'", loginDevReq.m_strPassword.c_str());
    }

    if (!loginDevReq.m_strDomainName.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", devicedomain = '%s'", loginDevReq.m_strDomainName.c_str());
    }

    if (0xFFFFFFFF != loginDevReq.m_uiDeviceType)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", typeinfo = %d", loginDevReq.m_uiDeviceType);
    }

    if (!loginDevReq.m_strUserName.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", username = '%s'", loginDevReq.m_strUserName.c_str());
    }

    if (!loginDevReq.m_strUserPassword.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", userpassword = '%s'", loginDevReq.m_strUserPassword.c_str());
    }

    if (!loginDevReq.m_strP2pID.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", doorbell_p2pid = '%s'", loginDevReq.m_strP2pID.c_str());
    }

    if (!loginDevReq.m_strP2pServr.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", p2pserver = '%s'", loginDevReq.m_strP2pServr.c_str());
    }

    if (0xFFFFFFFF != loginDevReq.m_uiP2pSupplier)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", p2psupplier = %d", loginDevReq.m_uiP2pSupplier);
    }

    if (0xFFFFFFFF != loginDevReq.m_uiP2pBuildin)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", p2pbuildin = %d", loginDevReq.m_uiP2pBuildin);
    }

    if (!loginDevReq.m_strLicenseKey.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", licensekey = '%s'", loginDevReq.m_strLicenseKey.c_str());
    }

    if (!loginDevReq.m_strPushID.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", pushid = '%s'", loginDevReq.m_strPushID.c_str());
    }

    if (!loginDevReq.m_strDistributor.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", distributor = '%s'", loginDevReq.m_strDistributor.c_str());
    }

    if (!loginDevReq.m_strOtherProperty.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", otherproperty = '%s'", loginDevReq.m_strOtherProperty.c_str());
    }

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("InsertDoorbellParameterToDB exec sql error, sql is " << sql);
    }
}

void AccessManager::UpdateDoorbellParameterToDB(const std::string &strDeviceID, const InteractiveProtoHandler::DoorbellParameter &doorbellParameter)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len;
    snprintf(sql, size, "update t_device_parameter_doorbell set updatedate = current_time");

    bool blModified = false;

    if (!doorbellParameter.m_strDoorbellName.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", doorbell_name = '%s'", doorbellParameter.m_strDoorbellName.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strSerialNumber.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", serial_number = '%s'", doorbellParameter.m_strSerialNumber.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strDoorbellP2Pid.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", doorbell_p2pid = '%s'", doorbellParameter.m_strDoorbellP2Pid.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strBatteryCapacity.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", battery_capacity = '%s'", doorbellParameter.m_strBatteryCapacity.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strChargingState.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", charging_state = '%s'", doorbellParameter.m_strChargingState.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strWifiSignal.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", wifi_signal = '%s'", doorbellParameter.m_strWifiSignal.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strVolumeLevel.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", volume_level = '%s'", doorbellParameter.m_strVolumeLevel.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strVersionNumber.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", version_number = '%s'", doorbellParameter.m_strVersionNumber.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strChannelNumber.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", channel_number = '%s'", doorbellParameter.m_strChannelNumber.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strCodingType.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", coding_type = '%s'", doorbellParameter.m_strCodingType.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strPIRAlarmSwtich.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", pir_alarm_swtich = '%s'", doorbellParameter.m_strPIRAlarmSwtich.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strDoorbellSwitch.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", doorbell_switch = '%s'", doorbellParameter.m_strDoorbellSwitch.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strPIRAlarmLevel.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", pir_alarm_level = '%s'", doorbellParameter.m_strPIRAlarmLevel.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strPIRIneffectiveTime.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", pir_ineffective_time = '%s'", doorbellParameter.m_strPIRIneffectiveTime.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strCurrentWifi.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", current_wifi = '%s'", doorbellParameter.m_strCurrentWifi.c_str());

        blModified = true;
    }

    if (!blModified)
    {
        LOG_INFO_RLD("UpdateDoorbellParameterToDB completed, there is no change");
        return;
    }

    len = strlen(sql);
    snprintf(sql + len, size - len, " where deviceid = '%s'", strDeviceID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("UpdateDoorbellParameterToDB exec sql failed, sql is " << sql);
    }
}

bool AccessManager::QueryDeviceParameterToDB(const std::string &strDeviceID, const unsigned int uiDeviceType, const std::string &strQueryType,
    InteractiveProtoHandler::DoorbellParameter &doorbellParameter)
{
    if (DEVICE_TYPE_DOORBELL == uiDeviceType)
    {
        return QueryDoorbellParameterToDB(strDeviceID, strQueryType, doorbellParameter);
    }
    else if (DEVICE_TYPE_IPC == uiDeviceType)
    {
        return true;
    }
    else
    {
        LOG_ERROR_RLD("QueryDeviceParameterToDB failed, unknown device type: " << uiDeviceType);
        return false;
    }
}

bool AccessManager::QueryDoorbellParameterToDB(const std::string &strDeviceID, const std::string &strQueryType, InteractiveProtoHandler::DoorbellParameter &doorbellParameter)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select doorbell_name, serial_number, doorbell_p2pid, battery_capacity, charging_state, wifi_signal, volume_level, version_number, channel_number, coding_type,"
        " pir_alarm_swtich, doorbell_switch, pir_alarm_level, pir_ineffective_time current_wifi from t_device_parameter_doorbell"
        " where deviceid = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strDeviceID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        switch (uiColumnNum)
        {
        case 0:
            doorbellParameter.m_strDoorbellName = strColumn;
            break;
        case 1:
            doorbellParameter.m_strVersionNumber = strColumn;
            break;
        case 2:
            doorbellParameter.m_strDoorbellP2Pid = strColumn;
            break;
        case 3:
            doorbellParameter.m_strBatteryCapacity = strColumn;
            break;
        case 4:
            doorbellParameter.m_strChargingState = strColumn;
            break;
        case 5:
            doorbellParameter.m_strWifiSignal = strColumn;
            break;
        case 6:
            doorbellParameter.m_strVolumeLevel = strColumn;
            break;
        case 7:
            doorbellParameter.m_strVersionNumber = strColumn;
            break;
        case 8:
            doorbellParameter.m_strChannelNumber = strColumn;
            break;
        case 9:
            doorbellParameter.m_strCodingType = strColumn;
            break;
        case 10:
            doorbellParameter.m_strPIRAlarmSwtich = strColumn;
            break;
        case 11:
            doorbellParameter.m_strDoorbellSwitch = strColumn;
            break;
        case 12:
            doorbellParameter.m_strPIRAlarmLevel = strColumn;
            break;
        case 13:
            doorbellParameter.m_strPIRIneffectiveTime = strColumn;
            break;
        case 14:
            doorbellParameter.m_strCurrentWifi = strColumn;
            Result = doorbellParameter;
            break;

        default:
            LOG_ERROR_RLD("QueryDoorbellParameterToDB sql callback error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("QueryDoorbellParameterToDB exec sql error, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryDoorbellParameterToDB sql result is empty, sql is " << sql);
        return false;
    }

    auto result = boost::any_cast<InteractiveProtoHandler::DoorbellParameter>(ResultList.front());
    doorbellParameter.m_strDoorbellName = result.m_strDoorbellName;
    doorbellParameter.m_strSerialNumber = result.m_strSerialNumber;
    doorbellParameter.m_strDoorbellP2Pid = result.m_strDoorbellP2Pid;
    doorbellParameter.m_strBatteryCapacity = result.m_strBatteryCapacity;
    doorbellParameter.m_strChargingState = result.m_strChargingState;
    doorbellParameter.m_strWifiSignal = result.m_strWifiSignal;
    doorbellParameter.m_strVolumeLevel = result.m_strVolumeLevel;
    doorbellParameter.m_strVersionNumber = result.m_strVersionNumber;
    doorbellParameter.m_strChannelNumber = result.m_strChannelNumber;
    doorbellParameter.m_strCodingType = result.m_strCodingType;
    doorbellParameter.m_strPIRAlarmSwtich = result.m_strPIRAlarmSwtich;
    doorbellParameter.m_strDoorbellSwitch = result.m_strDoorbellSwitch;
    doorbellParameter.m_strPIRAlarmLevel = result.m_strPIRAlarmLevel;
    doorbellParameter.m_strPIRIneffectiveTime = result.m_strPIRIneffectiveTime;
    doorbellParameter.m_strCurrentWifi = result.m_strCurrentWifi;

    return true;
}

bool AccessManager::QueryDeviceDateAndVersionToDB(const std::string &strDeviceID, const unsigned int uiDeviceType,
    std::string &strUpdateDate, std::string &strVersion)
{
    char sql[1024] = { 0 };
    const char *sqlfmt;

    if (DEVICE_TYPE_DOORBELL == uiDeviceType)
    {
        sqlfmt = "select version_number, updatedate from t_device_parameter_doorbell where deviceid = '%s' and status = 0";
    }
    else if (DEVICE_TYPE_IPC == uiDeviceType)
    {
        sqlfmt = "select version, updatedate from t_device_parameter_ipc  where deviceid = '%s' and status = 0";
    }
    else
    {
        LOG_ERROR_RLD("QueryDeviceDateAndVersionToDB, unknown device type: " << uiDeviceType);
        return false;
    }
    
    snprintf(sql, sizeof(sql), sqlfmt, strDeviceID.c_str());

    struct DeviceParam 
    {
        std::string strVersion;
        std::string strUpdateDate;
    };
    DeviceParam deviceParam;

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        switch (uiColumnNum)
        {
        case 0:
            deviceParam.strVersion = strColumn;
            break;
        case 1:
            deviceParam.strUpdateDate = strColumn;
            Result = deviceParam;
            break;

        default:
            LOG_ERROR_RLD("QueryDeviceDateAndVersionToDB sql callback error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("QueryDeviceDateAndVersionToDB exec sql error, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryDeviceDateAndVersionToDB sql result is empty, sql is << " << sql);
        return true;
    }

    auto result = boost::any_cast<DeviceParam>(ResultList.front());
    strVersion = result.strVersion;
    strUpdateDate = result.strUpdateDate;

    return true;
}

bool AccessManager::QueryIfP2pIDValidToDB(const std::string &strP2pID)
{
    char sql[1024] = { 0 };
    unsigned int size = sizeof(sql);
    const char *sqlfmt = "select deviceid from t_device_parameter_ipc where p2pid = '%s' and status = 0";
    snprintf(sql, size, sqlfmt, strP2pID.c_str());

    std::string strDeviceID;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        strDeviceID = strColumn;
        Result = strDeviceID;

        LOG_INFO_RLD("QueryIfP2pIDValidToDB sql device id is " << strDeviceID);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("QueryIfP2pIDValidToDB exec sql error, sql is " << sql);
        return false;
    }

    if (!ResultList.empty())
    {
        return true;
    }

    memset(sql, 0, size);
    sqlfmt = "select deviceid from t_device_parameter_doorbell where doorbell_p2pid = '%s' and status = 0";
    snprintf(sql, size, sqlfmt, strP2pID.c_str());

    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("QueryIfP2pIDValidToDB exec sql error, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryIfP2pIDValidToDB sql result is empty, sql is " << sql);
        return false;
    }

    return true;
}
