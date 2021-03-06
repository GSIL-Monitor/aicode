#include "InterProcessHandler.h"
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
m_DBTimer(NULL, 600), m_ulTimerTimes(0), m_EventFileProcessRunner(1),
m_MsgSender(new InterProcessHandler(InterProcessHandler::SEND_MODE, "mp4_req")),
m_MsgReceiver(new InterProcessHandler(InterProcessHandler::RECEIVE_MODE, "mp4_rsp"))
{
    auto ReceivedHandler = [&](const std::string &strMsg) ->void
    {
        Json::Reader reader;
        Json::Value root;

        if (!reader.parse(strMsg, root))
        {
            LOG_ERROR_RLD("Receive file info failed, parse http post response data error, raw data is : " << strMsg);
            return;
        }

        if (!root.isObject())
        {
            LOG_ERROR_RLD("Receive file info failed, parse http post response data error, raw data is : " << strMsg);
            return;
        }

        auto jsRet = root["retcode"];
        if (jsRet.isNull() || !jsRet.isString() || jsRet.asString().empty())
        {
            LOG_ERROR_RLD("Receive file info failed, http post return error, raw data is: " << strMsg);
            return;
        }

        if (jsRet.asString() != "0")
        {
            LOG_ERROR_RLD("Receive file process result failed and return code is " << jsRet.asString());
            return;
        }

        auto jsFileid = root["fileid"];
        if (jsFileid.isNull() || !jsFileid.isString() ||  jsFileid.asString().empty())
        {
            LOG_ERROR_RLD("Receive file info failed, http post return error, raw data is: " << strMsg);
            return;
        }

        auto jsEventid = root["eventid"];
        if (jsEventid.isNull() || !jsEventid.isString() || jsEventid.asString().empty())
        {
            LOG_ERROR_RLD("Receive file info failed, http post return error, raw data is: " << strMsg);
            return;
        }

        std::string strThumbnailFileid;
        auto jsThumbnailFileid = root["thumbnail_fileid"];
        if (!jsThumbnailFileid.isNull() && jsThumbnailFileid.isString() && !jsThumbnailFileid.asString().empty())
        {
            strThumbnailFileid = jsThumbnailFileid.asString();
        }


        //更新事件记录中的文件ID字段
        const std::string &strFileIDOfMp4 = jsFileid.asString(); //jsFileid.asString() + ".mp4";
        char sql[1024] = { 0 };
        const char *sqlfmt = strThumbnailFileid.empty() ? "update t_device_event_info set  fileid = '%s' where eventid = '%s'" :
            "update t_device_event_info set  fileid = '%s', thumbnail = '%s' where eventid = '%s'";

        if (strThumbnailFileid.empty())
        {
            snprintf(sql, sizeof(sql), sqlfmt, strFileIDOfMp4.c_str(), jsEventid.asString().c_str());
        }
        else
        {
            snprintf(sql, sizeof(sql), sqlfmt, strFileIDOfMp4.c_str(), strThumbnailFileid.c_str(), jsEventid.asString().c_str());
        }
        
        if (!m_pMysql->QueryExec(std::string(sql)))
        {
            LOG_ERROR_RLD("Update device event error, sql is " << sql);
            return;
        }

        //删除无用的原始文件
        //bool blRet = RemoveRemoteFile(jsFileid.asString());

        //LOG_INFO_RLD("Receive msg of file processed and remove original file " << jsFileid.asString() << " and result is " << blRet);
    };

    m_MsgReceiver->SetMsgOfReceivedHandler(ReceivedHandler);
}

AccessManager::~AccessManager()
{
    m_DBTimer.Stop();

    m_SessionMgr.Stop();

    m_DBRuner.Stop();

    m_EventFileProcessRunner.Stop();
        
    delete m_pMysql;
    m_pMysql = NULL;

}

bool AccessManager::Init()
{
    bool blMsgSenderInit = m_MsgSender->Init();
    bool blMsgReceiverInit = m_MsgReceiver->Init();

    LOG_INFO_RLD("Msg sender init is " << blMsgSenderInit << " receiver init is " << blMsgReceiverInit);

    if (!blMsgReceiverInit || !blMsgSenderInit)
    {
        return false;
    }

    if (!m_pMysql->Init(m_ParamInfo.m_strDBHost.c_str(), m_ParamInfo.m_strDBUser.c_str(), m_ParamInfo.m_strDBPassword.c_str(), m_ParamInfo.m_strDBName.c_str()))
    {
        LOG_ERROR_RLD("Init db failed, db host is " << m_ParamInfo.m_strDBHost << " db user is " << m_ParamInfo.m_strDBUser << " db pwd is " <<
            m_ParamInfo.m_strDBPassword << " db name is " << m_ParamInfo.m_strDBName);
        return false;
    }

    m_SessionMgr.SetMemCacheAddRess(m_ParamInfo.m_strMemAddress, m_ParamInfo.m_strMemPort);
    m_SessionMgr.SetGlobalMemCacheAddRess(m_ParamInfo.m_strMemAddressGlobal, m_ParamInfo.m_strMemPortGlobal);
    m_SessionMgr.SetSessionTimeoutCB(boost::bind(&ClusterAccessCollector::AddAccessTimeoutRecord, m_pClusterAccessCollector, _1, _2));

    if (!m_SessionMgr.Init())
    {
        LOG_ERROR_RLD("Session mgr init failed.");
        return false;
    }

    m_SessionMgr.SetUserLoginMutex(boost::lexical_cast<bool>(m_ParamInfo.m_strUserLoginMutex));
    m_SessionMgr.SetUerLoginKickout(boost::lexical_cast<bool>(m_ParamInfo.m_strUserKickoutType));

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
            LOG_INFO_RLD("Refresh access domain name successful");
        }

        if (m_ParamInfo.m_strMasterNode == "Yes")
        {
            if (m_ulTimerTimes > 0 && m_ulTimerTimes % 6 == 0)  //每隔一小时执行一次
            {
                UpdateDeviceEventStoredTime();
            }
            ++m_ulTimerTimes;
        }
    };

    m_DBTimer.SetTimeOutCallBack(TmFunc);
    
    m_DBTimer.Run(true);

    m_DBCache.SetSqlCB(boost::bind(&AccessManager::UserInfoSqlCB, this, _1, _2, _3, _4));
    
    m_DBRuner.Run();

    m_SessionMgr.Run();

    m_EventFileProcessRunner.Run();

    m_MsgReceiver->RunReceivedMsg();
    m_MsgSender->RunSendMsg();

    if (!RefreshCmsCallInfo())
    {
        LOG_ERROR_RLD("Refresh cms call info failed.");
        return false;
    }

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
    ReturnInfo::RetCode(ReturnInfo::SESSION_TIMEOUT);

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
        rsp.m_iRetcode = ReturnInfo::RetCode();
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
    //
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
        InteractiveProtoHandler::MsgType::QueryIfP2pIDValidReq_USR_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::QueryPlatformPushStatusReq_DEV_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::RegisterCmsCallReq_USR_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::UnregisterCmsCallReq_USR_T == req.m_MsgType ||
        InteractiveProtoHandler::MsgType::QueryDeviceP2pIDReq_USR_T == req.m_MsgType)

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
    else
    {
        int iErrorCode = SessionMgr::CREATE_OK;
        if (!m_SessionMgr.GetSessionStatus(req.m_strSID, iErrorCode))
        {
            LOG_ERROR_RLD("PreCommonHandler failed, get session status error, msg type is " << req.m_MsgType <<
                " and seesion id is " << req.m_strSID);
            blResult = false;
            return blResult;
        }

        if (SessionMgr::LOGIN_MUTEX_ERROR == iErrorCode)
        {
            LOG_ERROR_RLD("PreCommonHandler failed, the account is logged in at other terminal, msg type is " << req.m_MsgType <<
                " and seesion id is " << req.m_strSID);
            ReturnInfo::RetCode(ReturnInfo::ACCOUNT_LOGIN_AT_OTHER_TERMINAL);
            blResult = false;
            return blResult;
        }
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
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

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
    if (ValidUser(RegUsrReq.m_userInfo.m_strUserID, RegUsrReq.m_userInfo.m_strUserName, blUserExist, "", true))
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

    m_DBRuner.Post(boost::bind(&AccessManager::SendUserEmailAction, this, RegUsrReq.m_userInfo.m_strUserName, RegUsrReq.m_userInfo.m_strUserPassword,
        RegUsrReq.m_userInfo.m_strEmail, RegUsrReq.m_userInfo.m_uiTypeInfo, "reg"));

    blResult = true;
    
    
    return blResult;
}

bool AccessManager::UnRegisterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

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
        UnRegRspUsr.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
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

    std::list<InteractiveProtoHandler::Relation> relationList;
    std::list<std::string> strDevNameList;
    if (!QueryRelationByUserID(UnRegUsrReq.m_userInfo.m_strUserID, relationList, strDevNameList, 9, 0))
    {
        LOG_ERROR_RLD("Unregister user failed, query usr device realtion error, user id is " << UnRegUsrReq.m_userInfo.m_strUserID);
        return false;
    }

    for (auto &relation : relationList)
    {
        if (RELATION_OF_OWNER == relation.m_uiRelation)
        {
            LOG_ERROR_RLD("Unregister user failed, user has undeleted device, user id is << " << UnRegUsrReq.m_userInfo.m_strUserID);

            ReturnInfo::RetCode(ReturnInfo::UNDELETED_DEVICE_EXISTED_USER);
            return false;
        }
    }

    std::string strCurrentLoginUserID;
    
    if (m_SessionMgr.GetIDBySessionID(UnRegUsrReq.m_strSID, strCurrentLoginUserID) &&
        UnRegUsrReq.m_userInfo.m_strUserID == strCurrentLoginUserID)
    {
        m_SessionMgr.Remove(UnRegUsrReq.m_strSID);
    }

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
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

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

    //bool blUserExist;
    //if (!ValidUser(ModifyUsrReq.m_userInfo.m_strUserID, ModifyUsrReq.m_userInfo.m_strUserName, ?, blUserExist,
    //    "", ModifyUsrReq.m_userInfo.m_uiTypeInfo))
    //{
    //    LOG_ERROR_RLD("Modify user failed because user is invalid and user name is " << ModifyUsrReq.m_userInfo.m_strUserName <<
    //        " and user id is " << ModifyUsrReq.m_userInfo.m_strUserID << " and user pwd is " << ModifyUsrReq.m_userInfo.m_strUserPassword);

    //    ReturnInfo::RetCode(ReturnInfo::USERID_NOT_EXISTED_USER);
    //    return false;
    //}
    
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

    m_DBRuner.Post(boost::bind(&AccessManager::UpdateUserInfoToDB, this, ModifyUsrReq.m_userInfo, ModifyUsrReq.m_strOldPwd));

    blResult = true;

    return blResult;
}

bool AccessManager::LoginReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

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
        LoginReqUsr.m_userInfo.m_strUserPassword, true))
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
        boost::bind(&AccessManager::SessionTimeoutProcessCB, this, _1), ClusterAccessCollector::USER_SESSION, LoginReqUsr.m_userInfo.m_strUserID,
        LoginReqUsr.m_uiTerminalType, LoginReqUsr.m_uiType);
        
    bool blTempLogin;
    if (!IsTempLogin(LoginReqUsr.m_userInfo.m_strUserName, blTempLogin))
    {
        LOG_ERROR_RLD("User login failed, check if user temp login error, src id is " << strSrcID <<
            " and user name is " << LoginReqUsr.m_userInfo.m_strUserName);
        return false;
    }

    if (blTempLogin)
    {
        LOG_ERROR_RLD("User login failed, user is login using temp password, src id is " << strSrcID <<
            " and user name is " << LoginReqUsr.m_userInfo.m_strUserName);
        ReturnInfo::RetCode(ReturnInfo::LOGIN_USING_TEMP_PASSWORD_USER);
        return false;
    }

    if (!QueryRelationByUserID(LoginReqUsr.m_userInfo.m_strUserID, RelationList, strDevNameList, LoginReqUsr.m_uiType,
        boost::lexical_cast<unsigned int>(LoginReqUsr.m_strValue), 0, 100))
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
                LogoutReqUsr.m_strSID, "", UNUSED_INPUT_UINT, "", strCurrentTime));
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
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;
    InteractiveProtoHandler::AddDevReq_USR req;
    std::string strDeviceID;
    std::string strUserNameOfCurrent;
    std::string strDevType;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID, &strDeviceID, &strUserNameOfCurrent, &strDevType)
    {
        InteractiveProtoHandler::AddDevRsp_USR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::AddDevRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strDeviceID = blResult ? strDeviceID : "";
        rsp.m_strValue = blResult ? "" : strUserNameOfCurrent + ":" +strDevType;

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

    LOG_INFO_RLD("Add device type is " << req.m_devInfo.m_uiTypeInfo << " and device report check is " << req.m_strDevReportCheck);

    //检查设备是否已经向平台上报过数据，网关设备类型不校验
    if ((DEVICE_TYPE_GATEWAY != req.m_devInfo.m_uiTypeInfo) && (req.m_strDevReportCheck == "1")) //网关设备不需要上报
    {
        if (!req.m_devInfo.m_strP2pID.empty() && !QueryIfDeviceReportedToDB(req.m_devInfo.m_strP2pID, req.m_devInfo.m_uiTypeInfo, strDeviceID))
        {
            LOG_ERROR_RLD("Add device failed, the device p2pid is not recorded, src id is " << strSrcID <<
                " and p2p id is " << req.m_devInfo.m_strP2pID);

            ReturnInfo::RetCode(ReturnInfo::DEVICE_P2PID_NOT_RECORDED_USER);
            return false;
        }
    }

    if (req.m_strAllowDuplicate == "1")
    {
        strDeviceID = CreateUUID();
    }

    //if (req.m_devInfo.m_strP2pID.empty() || DEVICE_TYPE_IPC != req.m_devInfo.m_uiTypeInfo)
    if (strDeviceID.empty())
    {
        strDeviceID = req.m_devInfo.m_strDevID;
    }

    //这里是异步执行sql，防止阻塞，后续可以使用其他方式比如MQ来消除数据库瓶颈
    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));

    std::string strUserID;
    if (!QueryOwnerUserIDByDeviceID(strDeviceID, strUserID, &strUserNameOfCurrent, &strDevType))
    {
        LOG_ERROR_RLD("Add device query added device owner failed, src id is " << strSrcID);
        return false;
    }

    unsigned int uiTypeInfo = 0xFFFFFFFF;
    if (!strDevType.empty())
    {
        try
        {
            uiTypeInfo = boost::lexical_cast<unsigned int>(strDevType);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Device type info is invalid and error msg is " << e.what() << " and input type is " << strDevType);
            return false;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Device type info is invalid" << " and input type is " << strDevType);
            return false;
        }
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
        DevInfo.m_strDevID = strDeviceID;
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
        relation.m_strDevID = strDeviceID;
        relation.m_strExtend = req.m_devInfo.m_strExtend;
        relation.m_strUsrID = req.m_strUserID;

        m_DBRuner.Post(boost::bind(&AccessManager::InsertRelationToDB, this, strUuid, relation, req.m_devInfo.m_strDevName));

        //m_DBRuner.Post(boost::bind(&AccessManager::AddNoOwnerFile, this, req.m_strUserID, strDeviceID));

        blResult = true;
    }
    else if (strUserID == req.m_strUserID)
    {
        //ReturnInfo::RetCode(ReturnInfo::DEVICE_ADDED_BY_CURRENT_USER);
        //LOG_ERROR_RLD("Add device failed, the device has been added by current user, user id is" << strUserID);
        
        //当已经添加的设备类型是与待添加的设备类型相同时，允许重复添加成功，反之，则返回错误码
        if (req.m_devInfo.m_uiTypeInfo == uiTypeInfo)
        {
            LOG_INFO_RLD("Add device duplicated and user id is " << strUserID << " and device type is " << uiTypeInfo); //允许重复添加成功
            blResult = true;
        }
        else
        {
            ReturnInfo::RetCode(ReturnInfo::DEVICE_ADDED_BY_CURRENT_USER_TYPE);
            LOG_ERROR_RLD("Add device failed, the device has been added by current user, user id is" << strUserID << " and device type is " << uiTypeInfo);
        }
    } 
    else
    {
        ReturnInfo::RetCode(ReturnInfo::DEVICE_IS_ADDED_USER);
        LOG_ERROR_RLD("Add device failed, the device has been added, owner user id is " << strUserID << " and user name  is " << strUserNameOfCurrent);
    }

    return blResult;
}

bool AccessManager::DelDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

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
    
    m_DBRuner.Post(boost::bind(&AccessManager::RemoveExpiredDeviceEventToDB, this, req.m_strDevIDList.front(), true));

    m_DBRuner.Post(boost::bind(&AccessManager::DeleteDeviceParameter, this, req.m_strDevIDList.front()));

    m_DBRuner.Post(boost::bind(&AccessManager::DelDeviceToDB, this, req.m_strDevIDList, DELETE_STATUS, req.m_strUserID));

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

    if (DEVICE_SHAREDWITH_USER == req.m_uiDeviceShared)
    {
        m_DBRuner.Post(boost::bind(&AccessManager::ModifySharedDeviceNameToDB, this, req.m_strUserID,
            req.m_devInfo.m_strDevID, req.m_devInfo.m_strDevName));
    }
    else
    {
        m_DBRuner.Post(boost::bind(&AccessManager::ModDeviceToDB, this, req.m_devInfo));
    }

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
    
    if (!QueryDevInfoToDB(req.m_strUserID, req.m_strDevID, req.m_uiDeviceShared, dev, false))
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

	unsigned int uiAppType;
	m_SessionMgr.GetSessionLoginType(req.m_strSID, uiAppType);
    if (!QueryRelationByUserID(req.m_strUserID, RelationList, strDevNameList, uiAppType, boost::lexical_cast<unsigned int>(req.m_strValue), req.m_uiBeginIndex))
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
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;
    InteractiveProtoHandler::SharingDevReq_USR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID)
    {
        InteractiveProtoHandler::SharingDevRsp_USR rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::SharingDevRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
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

    std::string strUserID;
    if (!QueryOwnerUserIDByDeviceID(req.m_relationInfo.m_strDevID, strUserID))
    {
        LOG_ERROR_RLD("Sharing device failed, query device owner error, src id is " << strSrcID);
        return false;
    }

    if (strUserID.empty())
    {
        ReturnInfo::RetCode(ReturnInfo::SHARED_DEVICE_NOT_ADDED_BY_USER);

        LOG_ERROR_RLD("Sharing device failed, the device is not added, src id is " << strSrcID);
        return false;
    }

    std::string strSharedUserID;
    if (req.m_relationInfo.m_strUserID.empty())
    {
        if (req.m_strUserName.empty() || !QueryUserIDByUserName(req.m_strUserName, strSharedUserID))
        {
            ReturnInfo::RetCode(ReturnInfo::SHARED_USERNAME_IS_INVALID_USER);

            LOG_ERROR_RLD("Sharing device failed, query user id error, src id is " << strSrcID <<
                " and user name is " << req.m_strUserName);
            return false;
        }
    }
    else
    {
        strSharedUserID = req.m_relationInfo.m_strUserID;
    }

    if (!IsValidUserID(strSharedUserID))
    {
        ReturnInfo::RetCode(ReturnInfo::SHARED_USERID_IS_INVALID_USER);

        LOG_ERROR_RLD("Sharing device failed, the user id is not valid, src id is " << strSrcID <<
            " and shared user id is " << strSharedUserID);
        return false;
    }

    unsigned int uiUsedNum = 0;
    unsigned int uiCurrentLimitNum = SHARING_DEVICE_LIMIT;

    if (!QuerySharingDeviceLimit(strUserID, uiUsedNum))
    {
        LOG_ERROR_RLD("Query sharing device limit failed and  user id is " << strUserID);
        return false;
    }

    if (!QuerySharingDeviceCurrentLimit(uiCurrentLimitNum))
    {
        LOG_ERROR_RLD("Query sharing device current limit failed and  user id is " << strUserID);
        return false;
    }

    if (uiUsedNum >= uiCurrentLimitNum)
    {
        LOG_ERROR_RLD("Sharing device failed because limit is up on and used num is " << uiUsedNum << " and current limit num is " << uiCurrentLimitNum);
        ReturnInfo::RetCode(ReturnInfo::SHARING_DEVICE_UP_TO_LIMIT);
        return false;
    }

    LOG_INFO_RLD("Sharing device limit used num is " << uiUsedNum << " and current limit num is " << uiCurrentLimitNum);

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
    relation.m_strUsrID = strSharedUserID;

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
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

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

        LOG_INFO_RLD("Device login time zone info is :" << strValue << 
            " country code is " << strCountryCode << " and country name en is " << strCountryNameEn <<
            " and country name zh is " << strCountryNameZh);
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

    ////由于设备IPC的设备ID修改为p2pid，考虑到现有数据库中老数据兼容，故注释下面这段校验。
    //if (!LoginReqDev.m_strDomainName.empty() && !IsValidDeviceDomain(LoginReqDev.m_strDevID, LoginReqDev.m_strDomainName))
    //{
    //    LOG_ERROR_RLD("Device login vefiry device domain name failed, device id is " << LoginReqDev.m_strDevID <<
    //        " and domain name is " << LoginReqDev.m_strDomainName);

    //    ReturnInfo::RetCode(ReturnInfo::DEVICE_DOMAIN_USED_DEV);
    //    return false;
    //}
    ////
    //if (!LoginReqDev.m_strP2pID.empty() && !IsValidP2pIDProperty(LoginReqDev.m_strDevID, LoginReqDev.m_strP2pID))
    //{
    //    LOG_ERROR_RLD("Device login vefiry device p2p id failed, device id is " << LoginReqDev.m_strDevID <<
    //        " and p2p id is " << LoginReqDev.m_strP2pID);

    //    ReturnInfo::RetCode(ReturnInfo::DEVICE_P2PID_USED_DEV);
    //    return false;
    //}

    //烧录了P2P信息的设备登录时，同时上报P2P信息，平台记入数据库
    if (P2P_DEVICE_BUILDIN == LoginReqDev.m_uiP2pBuildin)
    {
        if (UNUSED_INPUT_UINT == LoginReqDev.m_uiP2pSupplier)
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
        boost::bind(&AccessManager::SessionTimeoutProcessCB, this, _1), ClusterAccessCollector::DEVICE_SESSION, LoginReqDev.m_strDevID, 
        SessionMgr::TERMINAL_DEV_TYPE, SessionMgr::TERMINAL_DEV_TYPE);

    blResult = true;

    return blResult;
}

bool AccessManager::P2pInfoReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

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
                if (UNUSED_INPUT_UINT == req.m_uiP2pSupplier)
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

        ++iRetry;
        LOG_ERROR_RLD("Get device p2p info failed, device id is " << strDeviceID << " and ip is " << req.m_strDevIpAddress <<
            " and retry " << iRetry << " times");
    } while (0); //(iRetry < GET_TIMEZONE_RETRY_TIMES);

    if (0 < iRetry) //(GET_TIMEZONE_RETRY_TIMES == iRetry)
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
                LogoutReqDev.m_strSID, LogoutReqDev.m_strDevID, UNUSED_INPUT_UINT, "", strCurrentTime));
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
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

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

    m_DBRuner.Post(boost::bind(&AccessManager::SendUserEmailAction, this, RetrievePwdReqUsr.m_strUserName, strRandPwd, RetrievePwdReqUsr.m_strEmail,
        RetrievePwdReqUsr.m_uiAppType, "rst"));

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
    unsigned int uiLease = 0;

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

    if (req.m_uiBusinessType != 0xFFFFFFFF)
    {
        if (!QueryAccessDomainOfBusiness(req.m_strUsername, req.m_uiBusinessType, strDomainName))
        {
            LOG_ERROR_RLD("Query user access domain by bussiness failed and user name is " << req.m_strUsername << " and business type is " << req.m_uiBusinessType);
            return false;
        }

        blResult = true;
        return blResult;
    }

    TimeZone timezone;
    CTimeZone cTimeZone;
    cTimeZone.setpostUrl(m_ParamInfo.m_strGetIpInfoSite);
    cTimeZone.SetDBManager(&m_DBCache, m_pMysql);

    AccessDomainInfo DomainInfo;

    ////由于阿里云查询ip地址的接口已经到期，而且目前没有业务使用这个，所以不在执行查询ip地址动作。
    //int iRetry = 0;
    //do 
    //{
    //    if (cTimeZone.GetCountryTime(req.m_strUserIpAddress, timezone))
    //    {
    //        break;
    //    }

    //    ++iRetry;
    //    LOG_ERROR_RLD("Query user access domain name failed, user ip is " << req.m_strUserIpAddress << " and retry " << iRetry << " times");
    //} while (0); //(iRetry < GET_TIMEZONE_RETRY_TIMES);

    //if (0 < iRetry) //(GET_TIMEZONE_RETRY_TIMES == iRetry)
    //{
    //    LOG_ERROR_RLD("Query user access domain name failed, user ip is " << req.m_strUserIpAddress);
    //    return false;
    //}

    timezone.sCode = "CN"; //缺省值
    if (!QueryAccessDomainInfoByArea(timezone.sCode, "", DomainInfo))
    {
        LOG_ERROR_RLD("QueryAccessDomainInfoByArea failed, user ip is " << req.m_strUserIpAddress);
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
        
    if (req.m_uiBusinessType != 0xFFFFFFFF)
    {
        if (!QueryAccessDomainOfBusinessDevice(req.m_strDevID, req.m_uiBusinessType, strDomainName))
        {
            LOG_ERROR_RLD("Query device access domain by bussiness failed and user name is " << req.m_strDevID << " and business type is " << req.m_uiBusinessType);
            return false;
        }

        blResult = true;
        return blResult;
    }

    TimeZone timezone;
    CTimeZone cTimeZone;
    cTimeZone.setpostUrl(m_ParamInfo.m_strGetIpInfoSite);
    cTimeZone.SetDBManager(&m_DBCache, m_pMysql);


    AccessDomainInfo DomainInfo;

    ////由于阿里云查询ip地址的接口已经到期，而且目前没有业务使用这个，所以不在执行查询ip地址动作。
    //int iRetry = 0;
    //do 
    //{
    //    if (cTimeZone.GetCountryTime(req.m_strDevIpAddress, timezone))
    //    {
    //        break;
    //    }

    //    ++iRetry;
    //    LOG_ERROR_RLD("Query device access domain name failed, device ip is " << req.m_strDevIpAddress << " and retry " << iRetry << " times");
    //} while (0); //(iRetry < GET_TIMEZONE_RETRY_TIMES);

    //if (0 < iRetry) //(GET_TIMEZONE_RETRY_TIMES == iRetry)
    //{
    //    LOG_ERROR_RLD("Query device access domain name failed, device ip is " << req.m_strDevIpAddress);
    //    return false;
    //}

    timezone.sCode = "CN"; //缺省值
    if (!QueryAccessDomainInfoByArea(timezone.sCode, "", DomainInfo))
    {
        LOG_ERROR_RLD("QueryAccessDomainInfoByArea failed, device ip is " << req.m_strDevIpAddress);
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

    if ("Doorbell" == req.m_strCategory)
    {
        InteractiveProtoHandler::DoorbellParameter doorbellParameter;
        if (!QueryDoorbellParameterToDB(req.m_strDeviceID, "All", doorbellParameter))
        {
            LOG_ERROR_RLD("Query firmware upgrade failed, query doorbell parameter error, src id is " << strSrcID <<
                " and device id is " << req.m_strDeviceID);
            return false;
        }

        if (!QueryFirmwareUpgradeToDB(req.m_strCategory, doorbellParameter.m_strSubCategory, doorbellParameter.m_strVersionNumber, firmwareUpgrade))
        //if (!QueryFirmwareUpgradeToDB(req.m_strCategory, "0", doorbellParameter.m_strVersionNumber, firmwareUpgrade))
        {
            LOG_ERROR_RLD("Query firmware upgrade from db failed, category is " << req.m_strCategory <<
                " and sub category is " << doorbellParameter.m_strSubCategory <<
                " and current version is " << doorbellParameter.m_strVersionNumber);
            return false;
        }
    }
    else
    {
        if (!QueryFirmwareUpgradeToDB(req.m_strCategory, req.m_strSubCategory, req.m_strCurrentVersion, firmwareUpgrade))
        {
            LOG_ERROR_RLD("Query firmware upgrade from db failed, category is " << req.m_strCategory <<
                " and sub category is " << req.m_strSubCategory << " and current version is " << req.m_strCurrentVersion);
            return false;
        }
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
        rsp.m_strUploadURL = blResult ? m_ParamInfo.m_strFileServerURL + "upload_file" : "";

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
    boost::replace_all(configuration.m_strFileID, "\\", "\\\\");  //保留string中的'\'字符
    configuration.m_uiFileSize = req.m_configuration.m_uiFileSize;
    configuration.m_strFilePath = "http://" + req.m_configuration.m_strServerAddress + "/filemgr.cgi?action=download_file&fileid="
        + configuration.m_strFileID;
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
        return false;
    }

    blResult = true;

    return blResult;
}

bool AccessManager::ModifyDevicePropertyReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

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

    ////由于设备IPC的设备ID修改为p2pid，考虑到现有数据库中老数据兼容，故注释下面这段校验。
    //if (!req.m_strDomainName.empty() && !IsValidDeviceDomain(req.m_strDeviceID, req.m_strDomainName))
    //{
    //    LOG_ERROR_RLD("Modify device property vefiry device domain name failed, device id is " << req.m_strDeviceID <<
    //        " and domain name is " << req.m_strDomainName);

    //    ReturnInfo::RetCode(ReturnInfo::DEVICE_DOMAIN_USED_DEV);
    //    return false;
    //}

    //if (!req.m_strP2pID.empty() && !IsValidP2pIDProperty(req.m_strDeviceID, req.m_strP2pID))
    //{
    //    LOG_ERROR_RLD("Modify device property vefiry device p2p id failed, device id is " << req.m_strDeviceID <<
    //        " and p2p id is " << req.m_strP2pID);

    //    ReturnInfo::RetCode(ReturnInfo::DEVICE_P2PID_USED_DEV);
    //    return false;
    //}

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
            rsp.m_doorbellParameter.m_strSubCategory = doorbellParameter.m_strSubCategory;
            rsp.m_doorbellParameter.m_strDisturbMode = doorbellParameter.m_strDisturbMode;
            rsp.m_doorbellParameter.m_strAntiTheftSwitch = doorbellParameter.m_strAntiTheftSwitch;
            rsp.m_doorbellParameter.m_strExtend = doorbellParameter.m_strExtend;
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
            " and sub category is " << doorbellParameter.m_strSubCategory <<
            " and disturb mode is " << doorbellParameter.m_strDisturbMode <<
            " and anti-theft switch is " << doorbellParameter.m_strAntiTheftSwitch <<
            " and extend is " << doorbellParameter.m_strExtend <<
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

bool AccessManager::QueryPlatformPushStatusReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::QueryPlatformPushStatusReq_DEV req;
    std::string strStatus;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &strStatus)
    {
        InteractiveProtoHandler::QueryPlatformPushStatusRsp_DEV rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryPlatformPushStatusRsp_DEV_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strStatus = blResult ? strStatus : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query platform push status rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query platform push status rsp already send, dst id is " << strSrcID <<
            " and push status is " << rsp.m_strStatus);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query platform push status req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryPlatformPushStatusToDB(strStatus))
    {
        LOG_ERROR_RLD("Query platform push status failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool AccessManager::DeviceEventReportReqDevice(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    InteractiveProtoHandler::DeviceEventReportReq_DEV req;
    std::string strEventID;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &strEventID)
    {
        InteractiveProtoHandler::DeviceEventReportRsp_DEV rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::DeviceEventReportRsp_DEV_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strEventID = blResult ? strEventID : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Device event report rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Device event report rsp already send, dst id is " << strSrcID <<
            " and event id is " << rsp.m_strEventID);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Device event report req unserialize failed, src id is " << strSrcID);
        return false;
    }

    std::string strUserID;
    if (!QueryOwnerUserIDByDeviceID(req.m_strDeviceID, strUserID) || strUserID.empty())
    {
        LOG_ERROR_RLD("Device event report failed, device is not added, src id is " << strSrcID <<
            " and device id is " << req.m_strDeviceID);

        ReturnInfo::RetCode(ReturnInfo::DEVICE_NOT_ADDED_BY_USER);
        return false;
    }

    strEventID = CreateUUID();

    std::string strFileID = req.m_strFileID;
    boost::replace_all(strFileID, "\\", "\\\\");  //保留string中的'\'字符
    m_DBRuner.Post(boost::bind(&AccessManager::InsertDeviceEventReportToDB, this, strEventID, req.m_strDeviceID,
        req.m_uiDeviceType, req.m_uiEventType, req.m_uiEventState, EVENT_MESSAGE_UNREAD, strFileID, req.m_strEventTime));

    m_DBRuner.Post(boost::bind(&AccessManager::RemoveExpiredDeviceEventToDB, this, req.m_strDeviceID, false));

    blResult = true;

    return blResult;
}

bool AccessManager::QueryAllDeviceEventReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::QueryAllDeviceEventReq_USR req;
    std::list<InteractiveProtoHandler::DeviceEvent> deviceEventList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &deviceEventList)
    {
        InteractiveProtoHandler::QueryAllDeviceEventRsp_USR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryAllDeviceEventRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_deviceEventList.swap(deviceEventList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all device event rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query all device event rsp already send, dst id is " << strSrcID <<
            " and user id is " << req.m_strUserID <<
            " and device id is " << req.m_strDeviceID <<
            " and result is " << blResult);

        if (blResult)
        {
            int i = 0;
            for (auto &deviceEvent : rsp.m_deviceEventList)
            {
                LOG_INFO_RLD("Device event info[" << i << "]: "
                    " device id is " << deviceEvent.m_strDeviceID <<
                    " and device type is " << deviceEvent.m_uiDeviceType <<
                    " and event id is " << deviceEvent.m_strEventID <<
                    " and event type is " << deviceEvent.m_uiEventType <<
                    " and event state is " << deviceEvent.m_uiEventState <<
                    " and file url is " << deviceEvent.m_strFileUrl <<
                    " and event time is " << deviceEvent.m_strEventTime <<
                    " and read state is " << deviceEvent.m_uiReadState <<
                    " and thumbnail url is " << deviceEvent.m_strThumbnailUrl);

                ++i;
            }
        }
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query all device event req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryAllDeviceEventToDB(req.m_strDeviceID, req.m_uiEventType, req.m_uiReadState, deviceEventList,
        req.m_strBeginDate, req.m_strEndDate, req.m_uiBeginIndex))
    {
        LOG_ERROR_RLD("Query all device event failed, src id is " << strSrcID);
        return false;
    }

    //if (EVENT_MESSAGE_READ != req.m_uiReadState && !deviceEventList.empty())
    //{
    //    std::list<std::string> strEventIDList;
    //    for (InteractiveProtoHandler::DeviceEvent &deviceEvent : deviceEventList)
    //    {
    //        strEventIDList.push_back(deviceEvent.m_strEventID);
    //    }

    //    m_DBRuner.Post(boost::bind(&AccessManager::UpdateEventReadStatusToDB, this, strEventIDList, EVENT_MESSAGE_READ));
    //}

    m_DBRuner.Post(boost::bind(&AccessManager::RemoveExpiredDeviceEventToDB, this, req.m_strDeviceID, false));

    blResult = true;

    return blResult;
}

bool AccessManager::DeleteDeviceEventReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::DeleteDeviceEventReq_USR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        InteractiveProtoHandler::DeleteDeviceEventRsp_USR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::DeleteDeviceEventRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete device event rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Delete device event rsp already send, dst id is " << strSrcID <<
            " and event id is " << req.m_strEventID);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Delete device event req unserialize failed, src id is " << strSrcID);
        return false;
    }

    m_DBRuner.Post(boost::bind(&AccessManager::DeleteDeviceEventToDB, this, req.m_strEventID));

    blResult = true;

    return blResult;
}

bool AccessManager::ModifyDeviceEventReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::ModifyDeviceEventReq_USR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        InteractiveProtoHandler::ModifyDeviceEventRsp_USR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::ModifyDeviceEventRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify device event rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Modify device event rsp already send, dst id is " << strSrcID <<
            " and event id is " << req.m_strEventID <<
            " and event state is " << req.m_uiEventState <<
            " and file id is " << req.m_strFileID);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Modify device event req unserialize failed, src id is " << strSrcID);
        return false;
    }

    m_DBRuner.Post(boost::bind(&AccessManager::ModifyDeviceEventToDB, this, req.m_strEventID,
        req.m_uiEventState, req.m_strUpdateTime, req.m_strFileID, req.m_uiReadState));

    blResult = true;

    return blResult;
}

bool AccessManager::AddStorageDetailReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::AddStorageDetailReq_USR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        InteractiveProtoHandler::AddStorageDetailRsp_USR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::AddStorageDetailRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add storage detail rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Add storage detail already send, dst id is " << strSrcID <<
            " and domain id is " << req.m_storageDetail.m_uiDomainID <<
            " and storage name is " << req.m_storageDetail.m_strStorageName <<
            " and overlap type is " << req.m_storageDetail.m_uiOverlapType <<
            " and storage time up limit is " << req.m_storageDetail.m_uiStorageTimeUpLimit <<
            " and storage time down limit is " << req.m_storageDetail.m_uiStorageTimeDownLimit <<
            " and begin date is " << req.m_storageDetail.m_strBeginDate <<
            " and end date is " << req.m_storageDetail.m_strEndDate);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Add storage detail req unserialize failed, src id is " << strSrcID);
        return false;
    }

    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));

    InteractiveProtoHandler::StorageDetail storageDetail;
    storageDetail.m_uiDomainID = req.m_storageDetail.m_uiDomainID;
    storageDetail.m_strObjectID = req.m_storageDetail.m_strObjectID;
    storageDetail.m_uiObjectType = req.m_storageDetail.m_uiObjectType;
    storageDetail.m_strStorageName = req.m_storageDetail.m_strStorageName;
    storageDetail.m_uiStorageType = req.m_storageDetail.m_uiStorageType;
    storageDetail.m_uiOverlapType = req.m_storageDetail.m_uiOverlapType;
    storageDetail.m_uiStorageTimeUpLimit = req.m_storageDetail.m_uiStorageTimeUpLimit;
    storageDetail.m_uiStorageTimeDownLimit = req.m_storageDetail.m_uiStorageTimeDownLimit;
    storageDetail.m_uiSizeOfSpaceUsed = req.m_storageDetail.m_uiSizeOfSpaceUsed;
    storageDetail.m_uiStorageUnitType = req.m_storageDetail.m_uiStorageUnitType;
    storageDetail.m_strBeginDate = req.m_storageDetail.m_strBeginDate;
    storageDetail.m_strEndDate = req.m_storageDetail.m_strEndDate;
    storageDetail.m_uiStatus = NORMAL_STATUS;
    storageDetail.m_strExtend = req.m_storageDetail.m_strExtend;

    m_DBRuner.Post(boost::bind(&AccessManager::InsertStorageDetailToDB, this, storageDetail));

    blResult = true;

    return blResult;
}

bool AccessManager::DeleteStorageDetailReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::DeleteStorageDetailReq_USR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        InteractiveProtoHandler::DeleteStorageDetailRsp_USR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::DeleteStorageDetailRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete storage detail rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Delete storage detail rsp already send, dst id is " << strSrcID <<
            " and object id is " << req.m_strObjectID);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Delete storage detail req unserialize failed, src id is " << strSrcID);
        return false;
    }

    m_DBRuner.Post(boost::bind(&AccessManager::DeleteStorageDetailToDB, this, req.m_strObjectID));

    blResult = true;

    return blResult;
}

bool AccessManager::ModifyStorageDetailReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::ModifyStorageDetailReq_USR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        InteractiveProtoHandler::ModifyStorageDetailRsp_USR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::ModifyStorageDetailRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify storage detail rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Modify storage detail rsp already send, dst id is " << strSrcID <<
            " and object id is " << req.m_storageDetail.m_strObjectID <<
            " and storage name is " << req.m_storageDetail.m_strStorageName <<
            " and overlap type is " << req.m_storageDetail.m_uiOverlapType <<
            " and storage time up limit is " << req.m_storageDetail.m_uiStorageTimeUpLimit <<
            " and storage time down limit is " << req.m_storageDetail.m_uiStorageTimeDownLimit <<
            " and begin date is " << req.m_storageDetail.m_strBeginDate <<
            " and end date is " << req.m_storageDetail.m_strEndDate);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Modify storage detail req unserialize failed, src id is " << strSrcID);
        return false;
    }

    m_DBRuner.Post(boost::bind(&AccessManager::UpdateStorageDetailToDB, this, req.m_storageDetail));

    blResult = true;

    return blResult;
}

bool AccessManager::QueryStorageDetailReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    InteractiveProtoHandler::QueryStorageDetailReq_USR req;
    InteractiveProtoHandler::StorageDetail storageDetail;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &storageDetail, &writer, &strSrcID)
    {
        InteractiveProtoHandler::QueryStorageDetailRsp_USR rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryStorageDetailRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        rsp.m_storageDetail.m_uiDomainID = storageDetail.m_uiDomainID;
        rsp.m_storageDetail.m_strObjectID = storageDetail.m_strObjectID;
        rsp.m_storageDetail.m_uiObjectType = storageDetail.m_uiObjectType;
        rsp.m_storageDetail.m_strStorageName = storageDetail.m_strStorageName;
        rsp.m_storageDetail.m_uiStorageType = storageDetail.m_uiStorageType;
        rsp.m_storageDetail.m_uiOverlapType = storageDetail.m_uiOverlapType;
        rsp.m_storageDetail.m_uiStorageTimeUpLimit = storageDetail.m_uiStorageTimeUpLimit;
        rsp.m_storageDetail.m_uiStorageTimeDownLimit = storageDetail.m_uiStorageTimeDownLimit;
        rsp.m_storageDetail.m_uiSizeOfSpaceUsed = storageDetail.m_uiSizeOfSpaceUsed;
        rsp.m_storageDetail.m_uiStorageUnitType = storageDetail.m_uiStorageUnitType;
        rsp.m_storageDetail.m_strBeginDate = storageDetail.m_strBeginDate;
        rsp.m_storageDetail.m_strEndDate = storageDetail.m_strEndDate;
        rsp.m_storageDetail.m_uiStatus = storageDetail.m_uiStatus;
        rsp.m_storageDetail.m_strExtend = storageDetail.m_strExtend;

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query storage detail rsp serialize failed.");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query storage detail rsp already send, dst id is " << strSrcID <<
            " and domain id is " << rsp.m_storageDetail.m_uiDomainID <<
            " and storage name is " << rsp.m_storageDetail.m_strStorageName <<
            " and overlap type is " << rsp.m_storageDetail.m_uiOverlapType <<
            " and storage time up limit is " << rsp.m_storageDetail.m_uiStorageTimeUpLimit <<
            " and storage time down limit is " << rsp.m_storageDetail.m_uiStorageTimeDownLimit <<
            " and begin date is " << rsp.m_storageDetail.m_strBeginDate <<
            " and end date is " << rsp.m_storageDetail.m_strEndDate <<
            " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query storage detail req unserialize failed, src id is " << strSrcID);
        return false;
    }

    int iErrorCode = 0;
    if (!QueryStorageDetailToDB(req.m_strObjectID, storageDetail, iErrorCode))
    {
        LOG_ERROR_RLD("Query storage detail from db failed, src id is " << strSrcID <<
            " and object id is " << req.m_strObjectID);

        ReturnInfo::RetCode(iErrorCode);
        return false;
    }

    blResult = true;

    return blResult;
}

bool AccessManager::QueryRegionStorageInfoReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::QueryRegionStorageInfoReq_USR req;
    unsigned int uiUsedSize = 0;
    unsigned int uiTotalSize = 0;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID, &uiUsedSize, &uiTotalSize)
    {
        InteractiveProtoHandler::QueryRegionStorageInfoRsp_USR rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryRegionStorageInfoRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_uiSizeOfSpaceUsed = uiUsedSize;
        rsp.m_uiSizeOfSpace = uiTotalSize;

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query region storage info rsp serialize failed.");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query region storage info rsp already send, dst id is " << strSrcID <<
            " and size of space used is " << rsp.m_uiSizeOfSpaceUsed <<
            " and size of space is " << rsp.m_uiSizeOfSpace <<
            " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query region storage info req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryRegionStorageInfoToDB(uiUsedSize, uiTotalSize))
    {
        LOG_ERROR_RLD("Query region storage info from db failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool AccessManager::QueryDeviceInfoMultiReqUser(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoHandler::QueryDeviceInfoMultiReq_USR req;
    std::list<InteractiveProtoHandler::DeviceStatus> deviceStatusList;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID, &deviceStatusList)
    {
        InteractiveProtoHandler::QueryDeviceInfoMultiRsp_USR rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryDeviceInfoMultiRsp_USR_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_deviceStatusList.swap(deviceStatusList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query device info multi rsp serialize failed.");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query device info multi rsp already send, dst id is " << strSrcID <<
            " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query device info multi req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryDeviceInfoMultiToDB(req.m_strDeviceIDList, deviceStatusList))
    {
        LOG_ERROR_RLD("Query device info multi from db failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool AccessManager::RegisterCmsCallReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::list<std::string> strP2pIDFailedList;
    std::string strAddress;
    std::string strPort;
    InteractiveProtoHandler::RegisterCmsCallReq_USR RegCmsReq;

    BOOST_SCOPE_EXIT(&blResult, this_, &RegCmsReq, &writer, &strSrcID, &strAddress, &strPort, &strP2pIDFailedList)
    {
        InteractiveProtoHandler::RegisterCmsCallRsp_USR RegCmsRsp;
        RegCmsRsp.m_MsgType = InteractiveProtoHandler::MsgType::RegisterCmsCallRsp_USR_T;
        RegCmsRsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        RegCmsRsp.m_strSID = RegCmsReq.m_strSID;
        RegCmsRsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        RegCmsRsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        
        RegCmsRsp.m_strAddress = strAddress;
        RegCmsRsp.m_strPort = strPort;
        RegCmsRsp.m_strP2pIDFailedList.swap(strP2pIDFailedList);

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(RegCmsRsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Register cms call rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);

        LOG_INFO_RLD("Register cms call rsp already send, dst id is " << strSrcID << " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, RegCmsReq))
    {
        LOG_ERROR_RLD("Register cms call req unserialize failed, src id is " << strSrcID);
        return false;
    }

    //若是CMS注册，则首先校验CMS对应的p2pidlist，确保没有和现有的重复
    if (!RegCmsReq.m_strCmsID.empty()) //CMS注册消息
    {
        if (!ValidCmsCallInfo(RegCmsReq.m_strCmsID, RegCmsReq.m_strCmsP2pIDList, strP2pIDFailedList))
        {
            ReturnInfo::RetCode(ReturnInfo::CMS_P2PID_DUPLICATE);
            LOG_ERROR_RLD("Valid failed and cms id is " << RegCmsReq.m_strCmsID);
            return false;
        }
    }
    
    //从内存中直接获取，若是CMS的注册消息，则若是成功，则更新内存cmsp2pidlist，并且保存到数据库中去，最后返回成功。
    //                                                             若是没有找到，则增加内存cmsp2pidlist，并且分配地址和端口，保存到数据库中，最后返回成功
    //反之，若是设备的注册消息，则纯粹从内存查找，则若是成功，直接返回成功， 若是没有找到，则给设备返回注册失败消息
    if (GetCmsCallInfo(RegCmsReq.m_strCmsID, RegCmsReq.m_strDeviceP2pID, strAddress, strPort))
    {
        if (!RegCmsReq.m_strCmsID.empty()) //CMS注册消息
        {
            std::list<CmsCall> CmsCallList;
            for (auto itBegin = RegCmsReq.m_strCmsP2pIDList.begin(), itEnd = RegCmsReq.m_strCmsP2pIDList.end(); itBegin != itEnd; ++itBegin)
            {
                CmsCall cca;
                cca.m_strAddress = strAddress;
                cca.m_strPort = strPort;
                cca.m_strCmsID = RegCmsReq.m_strCmsID;
                cca.m_strCmsP2pID = *itBegin;

                CmsCallList.push_back(std::move(cca));
            }

            if (!UpdateCmsCallInfo(RegCmsReq.m_strCmsID, CmsCallList))
            {
                LOG_ERROR_RLD("Update cms call failed and cms id is " << RegCmsReq.m_strCmsID);
                return false;
            }

            if (!SaveCmsCallInfo(RegCmsReq.m_strCmsID))
            {
                LOG_ERROR_RLD("Save cms call failed and cms id is " << RegCmsReq.m_strCmsID);
                return false;
            }
        }        

        LOG_INFO_RLD("Register cms call succeed and address is " << strAddress << " and port is " << strPort);

        blResult = true;
        return true;
    }

    //若是设备没有在内存中找到
    if (!RegCmsReq.m_strDeviceP2pID.empty())
    {
        LOG_ERROR_RLD("Device p2pid not found and value is " << RegCmsReq.m_strDeviceP2pID);
        return false;
    }

    //若是CMS没有在内存中找到
    if (!RegCmsReq.m_strCmsID.empty())
    {
        AllocCmsAddressAndPort(strAddress, strPort);

        std::list<CmsCall> CmsCallList;
        for (auto itBegin = RegCmsReq.m_strCmsP2pIDList.begin(), itEnd = RegCmsReq.m_strCmsP2pIDList.end(); itBegin != itEnd; ++itBegin)
        {
            CmsCall cca;
            cca.m_strAddress = strAddress;
            cca.m_strPort = strPort;
            cca.m_strCmsID = RegCmsReq.m_strCmsID;
            cca.m_strCmsP2pID = *itBegin;

            CmsCallList.push_back(std::move(cca));
        }

        if (!AddCmsCallInfo(RegCmsReq.m_strCmsID, CmsCallList))
        {
            LOG_ERROR_RLD("Add cms call failed and cms id is " << RegCmsReq.m_strCmsID);
            return false;
        }

        if (!SaveCmsCallInfo(RegCmsReq.m_strCmsID))
        {
            LOG_ERROR_RLD("Save cms call failed and cms id is " << RegCmsReq.m_strCmsID);
            return false;
        }

    }

    LOG_INFO_RLD("Register cms call succeed and address is " << strAddress << " and port is " << strPort);

    blResult = true;
    return true;
}

bool AccessManager::UnRegisterCmsCallReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    InteractiveProtoHandler::UnregisterCmsCallReq_USR UnRegCmsReq;

    BOOST_SCOPE_EXIT(&blResult, this_, &UnRegCmsReq, &writer, &strSrcID)
    {
        InteractiveProtoHandler::UnregisterCmsCallRsp_USR UnRegCmsRsp;
        UnRegCmsRsp.m_MsgType = InteractiveProtoHandler::MsgType::UnregisterCmsCallRsp_USR_T;
        UnRegCmsRsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        UnRegCmsRsp.m_strSID = UnRegCmsReq.m_strSID;
        UnRegCmsRsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        UnRegCmsRsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        UnRegCmsRsp.m_strValue = "";
        
        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(UnRegCmsRsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("UnRegister cms call rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);

        LOG_INFO_RLD("UnRegister cms call rsp already send, dst id is " << strSrcID << " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, UnRegCmsReq))
    {
        LOG_ERROR_RLD("UnRegister cms call req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!RemoveCmsCallInfo(UnRegCmsReq.m_strCmsID))
    {
        LOG_ERROR_RLD("Remove cms call info failed and cms id is " << UnRegCmsReq.m_strCmsID);
        return false;
    }

    if (!SaveRemoveCmsCallInfo(UnRegCmsReq.m_strCmsID))
    {
        LOG_ERROR_RLD("Save remove cms call info failed and cms id is " << UnRegCmsReq.m_strCmsID);
        return false;
    }

    blResult = true;
    return true;
}

bool AccessManager::QuerySharingDeviceLimitReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    unsigned int uiCurrentLimitNum = 0;
    unsigned int uiUsedNum = 0;
    InteractiveProtoHandler::QuerySharingDeviceLimitReq_USR QuerySDLReq;

    BOOST_SCOPE_EXIT(&blResult, this_, &QuerySDLReq, &writer, &strSrcID, &uiCurrentLimitNum, &uiUsedNum)
    {
        InteractiveProtoHandler::QuerySharingDeviceLimitRsp_USR QuerySDLRsp;
        QuerySDLRsp.m_MsgType = InteractiveProtoHandler::MsgType::QuerySharingDeviceLimitRsp_USR_T;
        QuerySDLRsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        QuerySDLRsp.m_strSID = QuerySDLReq.m_strSID;
        QuerySDLRsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        QuerySDLRsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        QuerySDLRsp.m_uiCurrentLimitNum = uiCurrentLimitNum;
        QuerySDLRsp.m_uiUsedNum = uiUsedNum;

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(QuerySDLRsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query sharing device limit rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);

        LOG_INFO_RLD("Query sharing device rsp already send, dst id is " << strSrcID << " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END
    
    if (!m_pProtoHandler->UnSerializeReq(strMsg, QuerySDLReq))
    {
        LOG_ERROR_RLD("Query sharing device limit req unserialize failed, src id is " << strSrcID);
        return false;
    }
    
    if (!QuerySharingDeviceLimit(QuerySDLReq.m_strUserID, uiUsedNum))
    {
        LOG_ERROR_RLD("Query sharing device limit failed and  user id is " << QuerySDLReq.m_strUserID);
        return false;
    }

    if (!QuerySharingDeviceCurrentLimit(uiCurrentLimitNum))
    {
        LOG_ERROR_RLD("Query sharing device current limit failed and  user id is " << QuerySDLReq.m_strUserID);
        return false;
    }
        
    blResult = true;
    return true;
}

bool AccessManager::QueryDeviceCapacityReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    DeviceCapacity devcap;
    InteractiveProtoHandler::QueryDeviceCapacityReq_USR QueryDevCapReq;

    BOOST_SCOPE_EXIT(&blResult, this_, &QueryDevCapReq, &writer, &strSrcID, &devcap)
    {
        InteractiveProtoHandler::QueryDeviceCapacityRsp_USR QueryDevCapRsp;
        QueryDevCapRsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryDeviceCapacityRsp_USR_T;
        QueryDevCapRsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        QueryDevCapRsp.m_strSID = QueryDevCapReq.m_strSID;
        QueryDevCapRsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        QueryDevCapRsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        QueryDevCapRsp.m_DevCap.m_uiDevType = QueryDevCapReq.m_uiDevType;
        QueryDevCapRsp.m_DevCap.m_strCapacityList.swap(devcap.m_strCapacityList);

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(QueryDevCapRsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query devcie capacity rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);

        LOG_INFO_RLD("Query device capacity rsp already send, dst id is " << strSrcID << " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, QueryDevCapReq))
    {
        LOG_ERROR_RLD("Query device capacity req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryDeviceCapacity(QueryDevCapReq.m_strUserID, QueryDevCapReq.m_uiDevType, devcap))
    {
        LOG_ERROR_RLD("Query device capacity failed and user id is " << QueryDevCapReq.m_strUserID << " and device type is " << QueryDevCapReq.m_uiDevType);
        return false;
    }

    blResult = true;
    return true;
}

bool AccessManager::QueryAllDeviceCapacityReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::list<DeviceCapacity> devcaplist;
    InteractiveProtoHandler::QueryAllDeviceCapacityReq_USR QueryAllDevCapReq;

    BOOST_SCOPE_EXIT(&blResult, this_, &QueryAllDevCapReq, &writer, &strSrcID, &devcaplist)
    {
        InteractiveProtoHandler::QueryAllDeviceCapacityRsp_USR QueryAllDevCapRsp;
        QueryAllDevCapRsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryALLDeviceCapacityRsp_USR_T;
        QueryAllDevCapRsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        QueryAllDevCapRsp.m_strSID = QueryAllDevCapReq.m_strSID;
        QueryAllDevCapRsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        QueryAllDevCapRsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        for (auto itBegin = devcaplist.begin(), itEnd = devcaplist.end(); itBegin != itEnd; ++itBegin)
        {
            InteractiveProtoHandler::DeviceCapacity devcap;
            devcap.m_uiDevType = itBegin->m_uiDevType;
            devcap.m_strCapacityList.swap(itBegin->m_strCapacityList);

            QueryAllDevCapRsp.m_DevCapList.push_back(std::move(devcap));
        }
        
        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(QueryAllDevCapRsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all devcie capacity rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);

        LOG_INFO_RLD("Query all device capacity rsp already send, dst id is " << strSrcID << " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, QueryAllDevCapReq))
    {
        LOG_ERROR_RLD("Query all device capacity req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryAllDeviceCapacity(QueryAllDevCapReq.m_strUserID, devcaplist))
    {
        LOG_ERROR_RLD("Query device capacity failed and user id is " << QueryAllDevCapReq.m_strUserID);
        return false;
    }

    blResult = true;
    return true;

}

bool AccessManager::QueryDeviceP2pIDReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    DevP2pIDInfo devp2pinfo;
    InteractiveProtoHandler::QueryDeviceP2pIDReq_USR QueryDevP2pIDReq;

    BOOST_SCOPE_EXIT(&blResult, this_, &QueryDevP2pIDReq, &writer, &strSrcID, &devp2pinfo)
    {
        InteractiveProtoHandler::QueryDeviceP2pIDRsp_USR QueryDevP2pIDRsp;
        QueryDevP2pIDRsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryDeviceP2pIDRsp_USR_T;
        QueryDevP2pIDRsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        QueryDevP2pIDRsp.m_strSID = QueryDevP2pIDReq.m_strSID;
        QueryDevP2pIDRsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        QueryDevP2pIDRsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        QueryDevP2pIDRsp.m_strChannelCount = devp2pinfo.m_strChannelCount;
        QueryDevP2pIDRsp.m_strDeviceSN = devp2pinfo.m_strDeviceSN;
        QueryDevP2pIDRsp.m_strExtend = devp2pinfo.m_strExtend;
        QueryDevP2pIDRsp.m_strMobilePort = devp2pinfo.m_strMobilePort;
        QueryDevP2pIDRsp.m_strP2pID = devp2pinfo.m_strP2pID;
        QueryDevP2pIDRsp.m_strUpdateTime = devp2pinfo.m_strUpdateTime;
        QueryDevP2pIDRsp.m_strWebPort = devp2pinfo.m_strWebPort;

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(QueryDevP2pIDRsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query device p2p id rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);

        LOG_INFO_RLD("Query device p2p id rsp already send, dst id is " << strSrcID << " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, QueryDevP2pIDReq))
    {
        LOG_ERROR_RLD("Query device p2p id req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryDeviceP2pID(QueryDevP2pIDReq.m_strDomainName, devp2pinfo))
    {
        LOG_ERROR_RLD("Query device p2p id failed and  user id is " << QueryDevP2pIDReq.m_strDomainName);
        return false;
    }
    
    blResult = true;
    return true;
}

bool AccessManager::UploadUserCfgReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    UserCfg ucfg;
    InteractiveProtoHandler::UploadUserCfgReq_USR UploadUsrCfgReq;

    BOOST_SCOPE_EXIT(&blResult, this_, &UploadUsrCfgReq, &writer, &strSrcID, &ucfg)
    {
        InteractiveProtoHandler::UploadUserCfgRsp_USR UploadUsrCfgRsp;
        UploadUsrCfgRsp.m_MsgType = InteractiveProtoHandler::MsgType::UploadUserCfgRsp_USR_T;
        UploadUsrCfgRsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        UploadUsrCfgRsp.m_strSID = UploadUsrCfgReq.m_strSID;
        UploadUsrCfgRsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        UploadUsrCfgRsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        UploadUsrCfgRsp.m_strVersion = ucfg.m_strVersion;
        
        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(UploadUsrCfgRsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Upload user cfg rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);

        LOG_INFO_RLD("Upload user cfg already send, dst id is " << strSrcID << " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, UploadUsrCfgReq))
    {
        LOG_ERROR_RLD("Upload user cfg req unserialize failed, src id is " << strSrcID);
        return false;
    }

    ucfg.m_strExtend = UploadUsrCfgReq.m_strExtend;
    ucfg.m_strFileID = UploadUsrCfgReq.m_strFileID;
    ucfg.m_strUserID = UploadUsrCfgReq.m_strUserID;
    ucfg.m_uiBusinessType = UploadUsrCfgReq.m_uiBusinessType;

    unsigned int uiMaxVersion = 0;
    
    boost::unique_lock<boost::mutex> lock(m_UserCfgMutex);
    if (!GetUsrCfgMaxVersion(ucfg.m_strUserID, ucfg.m_uiBusinessType, uiMaxVersion))
    {
        LOG_ERROR_RLD("Get user max version failed and user id is " << ucfg.m_strUserID);
        return false;
    }
    
    ++uiMaxVersion;
        
    ucfg.m_strVersion = boost::lexical_cast<std::string>(uiMaxVersion);
    if (!UploadUsrCfg(ucfg))
    {
        LOG_ERROR_RLD("Upload user cfg failed and  user id is " << ucfg.m_strUserID);
        return false;
    }
    
    blResult = true;
    return true;
}

bool AccessManager::QueryUserCfgReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    UserCfg ucfg;
    InteractiveProtoHandler::QueryUserCfgReq_USR QueryUsrCfgReq;

    BOOST_SCOPE_EXIT(&blResult, this_, &QueryUsrCfgReq, &writer, &strSrcID, &ucfg)
    {
        InteractiveProtoHandler::QueryUserCfgRsp_USR QueryUsrCfgRsp;
        QueryUsrCfgRsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryUserCfgRsp_USR_T;
        QueryUsrCfgRsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        QueryUsrCfgRsp.m_strSID = QueryUsrCfgReq.m_strSID;
        QueryUsrCfgRsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        QueryUsrCfgRsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        QueryUsrCfgRsp.m_strVersion = ucfg.m_strVersion;
        QueryUsrCfgRsp.m_strCfgURL = ucfg.m_strCfgURL;
        QueryUsrCfgRsp.m_strExtend = ucfg.m_strExtend;

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(QueryUsrCfgRsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query user cfg rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);

        LOG_INFO_RLD("Query user cfg already send, dst id is " << strSrcID << " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, QueryUsrCfgReq))
    {
        LOG_ERROR_RLD("Upload user cfg req unserialize failed, src id is " << strSrcID);
        return false;
    }

    ucfg.m_strUserID = QueryUsrCfgReq.m_strUserID;
    ucfg.m_uiBusinessType = QueryUsrCfgReq.m_uiBusinessType;

    boost::unique_lock<boost::mutex> lock(m_UserCfgMutex);    
    if (!QueryUsrCfg(ucfg.m_strUserID, ucfg.m_uiBusinessType, ucfg))
    {
        LOG_ERROR_RLD("Query user cfg failed and  user id is " << ucfg.m_strUserID);
        return false;
    }

    blResult = true;
    return true;
}

bool AccessManager::AddBlackIDReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;
    InteractiveProtoHandler::AddBlackIDReq_USR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID)
    {
        InteractiveProtoHandler::AddBlackIDRsp_USR rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::AddBlackIDRsp_USER_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add black id rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Add black id rsp already send, dst id is " << strSrcID << " and user id is " << req.m_strUserID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Add black id req unserialize failed, src id is " << strSrcID);
        return blResult;
    }
    
    //判断是否已经存在该BlackID    
    unsigned int uiIDNum = 0;
    if (!GetBlackIDNum(req.m_blkobj.m_strBlackID, uiIDNum))
    {
        LOG_ERROR_RLD("Get black id num failed");
        return blResult;
    }

    if (uiIDNum > 0)
    {
        LOG_INFO_RLD("Query black id exist and black id is " << req.m_blkobj.m_strBlackID);
        
        blResult = true;
        return blResult;
    }
    
    m_DBRuner.Post(boost::bind(&AccessManager::AddBlackID, this, req.m_blkobj.m_strBlackID, req.m_blkobj.m_strExtend, req.m_blkobj.m_uiIDType));

    blResult = true;
    return blResult;
}

bool AccessManager::RemoveBlackIDReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;
    InteractiveProtoHandler::RemoveBlackIDReq_USR req;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID)
    {
        InteractiveProtoHandler::RemoveBlackIDRsp_USR rsp;

        rsp.m_MsgType = InteractiveProtoHandler::MsgType::RemoveBlackIDRsp_USER_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Remove black id rsp serialize failed.");
            return; //false;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Remove black id rsp already send, dst id is " << strSrcID << " and user id is " << req.m_strUserID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Remove black id req unserialize failed, src id is " << strSrcID);
        return blResult;
    }

    //判断是否已经存在该BlackID    
    unsigned int uiIDNum = 0;
    if (!GetBlackIDNum(req.m_strBlackID, uiIDNum))
    {
        LOG_ERROR_RLD("Get black id num failed");
        return blResult;
    }

    if (uiIDNum == 0)
    {
        LOG_INFO_RLD("Query black id not exist and black id is " << req.m_strBlackID);
        return blResult;
    }

    m_DBRuner.Post(boost::bind(&AccessManager::RemoveBlackID, this, req.m_strBlackID));

    blResult = true;
    return blResult;
}

bool AccessManager::QueryAllBlackListReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    InteractiveProtoHandler::QueryAllBlackIDReq_USR req;
    std::list<InteractiveProtoHandler::BlackObj> blkobjList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &blkobjList)
    {
        InteractiveProtoHandler::QueryAllBlackIDRsp_USR rsp;
        rsp.m_MsgType = InteractiveProtoHandler::MsgType::QueryAllBlackIDRsp_USER_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_blkobjlist.swap(blkobjList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all black id rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query all black id rsp already send, dst id is " << strSrcID
            << " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query all black id req unserialize failed, src id is " << strSrcID);
        return blResult;
    }

    if (!QueryAllBlackInfo(req.m_uiIDType, req.m_uiBeginIndex, blkobjList))
    {
        LOG_ERROR_RLD("Query all black id failed and user id is " << req.m_strUserID);
        return blResult;
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

    if (UNUSED_INPUT_UINT != uiBusinessType)
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

bool AccessManager::QueryOwnerUserIDByDeviceID(const std::string &strDevID, std::string &strUserID, std::string *pstrUserName, std::string *pDevType)
{
    std::string strUserNameTmp;
    if (NULL == pstrUserName)
    {
        pstrUserName = &strUserNameTmp;
    }

    std::string strDevType;
    if (NULL == pDevType)
    {
        pDevType = &strDevType;
    }

    char sql[1024] = { 0 };
    const char* sqlfmt = "select rel.userid, usr.username, dev.typeinfo from"
        " t_user_info usr, t_device_info dev, t_user_device_relation rel"
        " where usr.userid = rel.userid and usr.status = 0 and"
        " dev.deviceid = '%s' and dev.id = rel.devicekeyid and dev.status = 0 and"
        " rel.relation = %d and rel.status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strDevID.c_str(), RELATION_OF_OWNER);
    std::string strSql(sql);

    strUserID.clear();
    pstrUserName->clear();
    pDevType->clear();

    auto FuncTmp = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)
    {
        switch (uiColumnNum)
        {
        case 0:
            strUserID = strColumn;
            break;
            
        case 1:
            *pstrUserName = strColumn;
            break;

        case 2:
            *pDevType = strColumn;
            break;

        default:
            LOG_ERROR_RLD("Query user id by device id is error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }

        LOG_INFO_RLD("Query user id by device id from db is " << strUserID << " and user name is " << *pstrUserName);
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

    LOG_INFO_RLD("QueryOwnerUserIDByDeviceID user id is " << strUserID << " and user name is " << *pstrUserName << " and device type is " << *pDevType);

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
    const char *sqlfmt = "update t_user_info set userpassword = '%s', exceptionstate = %d where username = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strUserPassword.c_str(), TEMP_LOGIN, strUserName.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Update t_user_info sql exec failed, sql is " << sql);
    }
}

void AccessManager::SendUserEmailAction(const std::string &strUserName, const std::string &strUserPassword, const std::string &strEmail, 
    const unsigned int uiAppType, const std::string &strAction)
{
    char cmd[1024] = { 0 };
    std::string strType = 0 == uiAppType ? "ring" : (1 == uiAppType ? "camviews": "cloudviews");
    const char *param = "./mail.sh '%s' '%s' '%s' '%s' '%s'";
    snprintf(cmd, sizeof(cmd), param, strEmail.c_str(), strUserName.c_str(), strUserPassword.c_str(), strType.c_str(), strAction.c_str());

    system(cmd);

    LOG_INFO_RLD("User email has been sended, email address is " << strEmail << " and user name is " << strUserName << " and action is " << strAction);
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

bool AccessManager::QueryFirmwareUpgradeToDB(const std::string &strCategory, const std::string &strSubCategory, const std::string &strCurrentVersion,
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

    std::string strUrl = strFilePath.substr(0, strFilePath.find_first_of('?') + 1) + "action=delete_file";

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

    if (UNUSED_INPUT_UINT != configuration.m_uiLeaseDuration)
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
        " from t_configuration_info where latestversion != '' and status = 0 limit %u, %u";
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

void AccessManager::UpdateUserInfoToDB(const InteractiveProtoHandler::User &UsrInfo, const std::string &strOldPasswd)
{
    std::list<std::string> strItemList;
    if (!UsrInfo.m_strUserPassword.empty())
    {
        char sql[1024] = { 0 };
        const char *sqlfmt = "userpassword = '%s'";
        snprintf(sql, sizeof(sql), sqlfmt, UsrInfo.m_strUserPassword.c_str());
        if (UsrInfo.m_strUserPassword != strOldPasswd)
        {
            strncat(sql, ", exceptionstate = 0", sizeof(sql) - strlen(sql));
        }
        strItemList.push_back(sql);
    }

    if (UNUSED_INPUT_UINT != UsrInfo.m_uiTypeInfo)
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
    const char *sqlfmt = "delete from t_user_info where userid = '%s'"; //"update t_user_info set status = '%d' where userid = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str());
    
    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Delete t_user_info sql exec failed, sql is " << sql);
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
    std::list<std::string> &strDevNameList, const unsigned int uiAppType, const unsigned int uiDevIsolation, const unsigned int uiBeginIndex, const unsigned int uiPageSize)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "select rel.userid, rel.deviceid, rel.relation, rel.begindate, rel.enddate, rel.createdate, rel.status, rel.extend, dev.devicename, rel.devicename"
        " from t_device_info dev, t_user_device_relation rel, t_user_info usr"
        " where dev.id = rel.devicekeyid and usr.userid = rel.userid and rel.userid = '%s' and rel.status = 0 and dev.status = 0 and usr.status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str());

	if (uiAppType != 9 && 1 == uiDevIsolation)
	{
		int len = strlen(sql);
		snprintf(sql + len, sizeof(sql) - len, " and dev.typeinfo = %d", uiAppType);
	}

	std::string strSql;
    char cTmp[128] = { 0 };
    snprintf(cTmp, sizeof(cTmp), " limit %u, %u", uiBeginIndex, uiPageSize);
    strSql = std::string(sql) + std::string(cTmp);
    
    std::list<boost::any> ResultList;

    //if (0) // (m_DBCache.GetResult(strSql, ResultList) && !ResultList.empty())
    //{
    //    boost::shared_ptr<std::list<InteractiveProtoHandler::Relation> > pRelationList;
    //    pRelationList = boost::any_cast<boost::shared_ptr<std::list<InteractiveProtoHandler::Relation> > >(ResultList.front());

    //    boost::shared_ptr<std::list<std::string> > pStrDevNameList;
    //    pStrDevNameList = boost::any_cast<boost::shared_ptr<std::list<std::string> >>(ResultList.back());
    //    
    //    auto itBegin = pRelationList->begin();
    //    auto itEnd = pRelationList->end();
    //    while (itBegin != itEnd)
    //    {
    //        RelationList.push_back(*itBegin);
    //        ++itBegin;
    //    }

    //    auto itBeginDevName = pStrDevNameList->begin();
    //    auto itEndDevName = pStrDevNameList->end();
    //    while (itBeginDevName != itEndDevName)
    //    {
    //        strDevNameList.push_back(*itBeginDevName);
    //        ++itBeginDevName;
    //    }

    //    LOG_INFO_RLD("Query relation get result from cache and sql is " << strSql);
    //}
    //else
    //{        
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

    LOG_INFO_RLD("QueryRelationByUserID success and user id is " << strUserID);
    return true;

    ////
    //boost::shared_ptr<std::list<boost::any> > pResultList(new std::list<boost::any>);
    //pResultList->push_back(pRelationList);
    //pResultList->push_back(pStrDevNameList);

    //m_DBCache.SetResult(strSql, pResultList);

    
    //LOG_INFO_RLD("Query relation get result from db and sql is " << strSql);
    
    //}

    //return true;
}

bool AccessManager::QueryRelationByDevID(const std::string &strDevID, std::list<InteractiveProtoHandler::Relation> &RelationList,
    std::list<std::string> &strUserNameList, const unsigned int uiBeginIndex, const unsigned int uiPageSize)
{
    char sql[1024] = { 0 };
    memset(sql, 0, sizeof(sql));
    const char *sqlft = "select rel.userid, rel.deviceid, rel.relation, rel.begindate, rel.enddate, rel.createdate, rel.status, rel.extend, usr.username"
        " from t_device_info dev, t_user_device_relation rel, t_user_info usr"
        " where dev.id = rel.devicekeyid and usr.userid = rel.userid and rel.deviceid = '%s' and rel.status = 0 and dev.status = 0 and usr.status = 0 order by rel.relation";
    snprintf(sql, sizeof(sql), sqlft, strDevID.c_str());

    std::string strSql;
    char cTmp[128] = { 0 };
    snprintf(cTmp, sizeof(cTmp), " limit %u, %u", uiBeginIndex, uiPageSize);
    strSql = std::string(sql) + std::string(cTmp);

    std::list<boost::any> ResultList;
    //if (m_DBCache.GetResult(strSql, ResultList) && !ResultList.empty()) //关闭缓存
    if (0)
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

bool AccessManager::ValidUser(std::string &strUserID, std::string &strUserName, bool &blUserExist, const std::string &strUserPwd, const bool IsForceFromDB)
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

bool AccessManager::IsValidUserID(const std::string &strUserID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select id from t_user_info where userid = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        Result = strColumn;

        LOG_INFO_RLD("IsValidUserID sql id is " << strColumn);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("IsValidUserID exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("IsValidUserID failed, the user id is not valid");
        return false;
    }

    return true;
}

bool AccessManager::IsTempLogin(const std::string &strUserName, bool &blTemp)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select exceptionstate from t_user_info where username = '%s' and exceptionstate = %d and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strUserName.c_str(), TEMP_LOGIN);

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        Result = strColumn;

        LOG_INFO_RLD("IsTempLogin sql user exception state is " << strColumn);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("IsTempLogin exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        blTemp = false;
    }
    else
    {
        blTemp = true;
        LOG_INFO_RLD("IsTempLogin successful, the user is using temp password login");
    }

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
        if (DEVICE_SHAREDWITH_USER != relationInfo.m_uiRelation)
        {
            pStrDevNameList->push_back(strColumn);
        }
        break;
    case 9:
        if (DEVICE_SHAREDWITH_USER == relationInfo.m_uiRelation)
        {
            pStrDevNameList->push_back(strColumn);
        }
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

void AccessManager::InsertRelationToDB(const std::string &strUuid, const RelationOfUsrAndDev &relation, const std::string &strDeviceName)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "insert into t_user_device_relation("
        "id, userid, deviceid, relation, devicekeyid, begindate, enddate, createdate, status, extend, devicename) values(uuid(),"
        "'%s','%s', '%d', '%s', '%s', '%s', '%s','%d', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, relation.m_strUsrID.c_str(), relation.m_strDevID.c_str(), relation.m_iRelation, strUuid.c_str(),
        relation.m_strBeginDate.c_str(), relation.m_strEndDate.c_str(), relation.m_strCreateDate.c_str(), relation.m_iStatus,
        relation.m_strExtend.c_str(), strDeviceName.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Insert t_user_device_relation sql exec failed, sql is " << sql);
    }
}

void AccessManager::RemoveRelationToDB(const RelationOfUsrAndDev &relation)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = //"update t_user_device_relation set status = %d where userid = '%s' and deviceid = '%s' and relation = %d and status = 0";
        "delete from t_user_device_relation where status = 0 and relation = 1 and userid = '%s' and deviceid = '%s' and devicekeyid = (select id from t_device_info where deviceid = '%s' and status = 0)";
    snprintf(sql, sizeof(sql), sqlfmt, relation.m_strUsrID.c_str(), relation.m_strDevID.c_str(), relation.m_strDevID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Update t_user_device_relation sql exec failed, sql is " << sql);
    }

}

void AccessManager::DelDeviceToDB(const std::list<std::string> &strDevIDList, const int iStatus, const std::string &strUserID)
{
    if (strDevIDList.empty())
    {
        LOG_ERROR_RLD("Delete device id list is empty.");
        return;
    }
    std::string strSql;

    //删除设备关系表
    for (auto strDevid : strDevIDList)
    {
        char sql[1024] = { 0 };
        const char* sqlfmt = "delete from t_user_device_relation where status = 0 and relation = 0 and userid = '%s' and deviceid = '%s' and devicekeyid = (select id from t_device_info where deviceid = '%s' and status = 0)";
        snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), strDevid.c_str(), strDevid.c_str());

        strSql = sql;

        if (!m_pMysql->QueryExec(strSql))
        {
            LOG_ERROR_RLD("Delete t_user_device_relation sql exec failed, sql is " << strSql);
        }

    }

    //"update t_device_info set status = '%d' where deviceid = '%s'";
    
    char sql[1024] = { 0 };
    const char* sqlfmt = "delete from t_device_info where "; //"update t_device_info set status = '%d' where ";
    snprintf(sql, sizeof(sql), sqlfmt); // , iStatus);
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

    if (DevInfo.m_uiTypeInfo != UNUSED_INPUT_UINT)
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

    if (DevInfo.m_uiStatus != UNUSED_INPUT_UINT)
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

void AccessManager::ModifySharedDeviceNameToDB(const std::string &strUserID, const std::string &strDeviceID,
    const std::string &strDeviceName)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "update t_user_device_relation set devicename = '%s' where userid = '%s' and"
        " devicekeyid = (select id from t_device_info where deviceid = '%s' and status = 0) and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strDeviceName.c_str(), strUserID.c_str(), strDeviceID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("ModifySharedDeviceNameToDB exec sql failed, sql is " << sql);
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
    const char *sqlfmt = "select id, devicename from t_device_info where deviceid = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, relation.m_strDevID.c_str());

    std::string strUuid;
    std::string strDeviceName;
    auto FuncTmp = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)
    {
        switch (uiColumnNum)
        {
        case 0:
            strUuid = strColumn;
            break;
        case 1:
            strDeviceName = strColumn;
            break;

        default:
            LOG_ERROR_RLD("SharingRelationToDB sql callback error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }
    };

    if (!m_pMysql->QueryExec(std::string(sql), FuncTmp))
    {
        LOG_ERROR_RLD("Query device key id failed and device id is " << relation.m_strDevID);
        return;
    }

    if (strUuid.empty())
    {
        LOG_ERROR_RLD("Query device key id failed, sql result is empty, device id is " << relation.m_strDevID);
        return;
    }

    InsertRelationToDB(strUuid, relation, strDeviceName);
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

bool AccessManager::QueryDevInfoToDB(const std::string &strUserID, const std::string &strDevID, const unsigned int uiDeviceShared,
    InteractiveProtoHandler::Device &dev, const bool IsNeedCache /*= true*/)
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

        if (DEVICE_SHAREDWITH_USER == uiDeviceShared && !QuerySharedDeviceNameToDB(strUserID, strDevID, dev.m_strDevName))
        {
            LOG_ERROR_RLD("Query device info failed, query shared device name error, user id is " << strUserID <<
                " and device id is " << strDevID);
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

    char sql[2048] = { 0 };
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

    if (UNUSED_INPUT_UINT != loginDevReq.m_uiDeviceType)
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

    if (UNUSED_INPUT_UINT != loginDevReq.m_uiP2pSupplier)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", p2psupplier = %d", loginDevReq.m_uiP2pSupplier);
    }

    if (UNUSED_INPUT_UINT != loginDevReq.m_uiP2pBuildin)
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
    char sql[2048] = { 0 };
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

    char sql[2048] = { 0 };
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

    if (UNUSED_INPUT_UINT != loginDevReq.m_uiDeviceType)
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

    if (UNUSED_INPUT_UINT != loginDevReq.m_uiP2pSupplier)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", p2psupplier = %d", loginDevReq.m_uiP2pSupplier);
    }

    if (UNUSED_INPUT_UINT != loginDevReq.m_uiP2pBuildin)
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
    char sql[4096] = { 0 };
    int size = sizeof(sql);
    int len;
    snprintf(sql, size, "update t_device_parameter_doorbell set updatedate = current_time");

    bool blModified = false;

    //输入字段为*时，表示把该字段清空
    if (!doorbellParameter.m_strDoorbellName.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", doorbell_name = '%s'", "*" == doorbellParameter.m_strDoorbellName ? "" : doorbellParameter.m_strDoorbellName.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strSerialNumber.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", serial_number = '%s'", "*" == doorbellParameter.m_strSerialNumber ? "" : doorbellParameter.m_strSerialNumber.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strDoorbellP2Pid.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", doorbell_p2pid = '%s'", "*" == doorbellParameter.m_strDoorbellP2Pid ? "" : doorbellParameter.m_strDoorbellP2Pid.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strBatteryCapacity.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", battery_capacity = '%s'", "*" == doorbellParameter.m_strBatteryCapacity ? "" : doorbellParameter.m_strBatteryCapacity.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strChargingState.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", charging_state = '%s'", "*" == doorbellParameter.m_strChargingState ? "" : doorbellParameter.m_strChargingState.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strWifiSignal.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", wifi_signal = '%s'", "*" == doorbellParameter.m_strWifiSignal ? "" : doorbellParameter.m_strWifiSignal.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strVolumeLevel.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", volume_level = '%s'", "*" == doorbellParameter.m_strVolumeLevel ? "" : doorbellParameter.m_strVolumeLevel.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strVersionNumber.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", version_number = '%s'", "*" == doorbellParameter.m_strVersionNumber ? "" : doorbellParameter.m_strVersionNumber.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strChannelNumber.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", channel_number = '%s'", "*" == doorbellParameter.m_strChannelNumber ? "" : doorbellParameter.m_strChannelNumber.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strCodingType.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", coding_type = '%s'", "*" == doorbellParameter.m_strCodingType ? "" : doorbellParameter.m_strCodingType.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strPIRAlarmSwtich.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", pir_alarm_swtich = '%s'", "*" == doorbellParameter.m_strPIRAlarmSwtich ? "" : doorbellParameter.m_strPIRAlarmSwtich.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strDoorbellSwitch.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", doorbell_switch = '%s'", "*" == doorbellParameter.m_strDoorbellSwitch ? "" : doorbellParameter.m_strDoorbellSwitch.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strPIRAlarmLevel.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", pir_alarm_level = '%s'", "*" == doorbellParameter.m_strPIRAlarmLevel ? "" : doorbellParameter.m_strPIRAlarmLevel.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strPIRIneffectiveTime.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", pir_ineffective_time = '%s'", "*" == doorbellParameter.m_strPIRIneffectiveTime ? "" : doorbellParameter.m_strPIRIneffectiveTime.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strCurrentWifi.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", current_wifi = '%s'", "*" == doorbellParameter.m_strCurrentWifi ? "" : doorbellParameter.m_strCurrentWifi.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strSubCategory.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", sub_category = '%s'", "*" == doorbellParameter.m_strSubCategory ? "" : doorbellParameter.m_strSubCategory.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strDisturbMode.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", disturb_mode = '%s'", "*" == doorbellParameter.m_strDisturbMode ? "" : doorbellParameter.m_strDisturbMode.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strAntiTheftSwitch.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", anti_theft = '%s'", "*" == doorbellParameter.m_strAntiTheftSwitch ? "" : doorbellParameter.m_strAntiTheftSwitch.c_str());

        blModified = true;
    }

    if (!doorbellParameter.m_strExtend.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", extend = '%s'", "*" == doorbellParameter.m_strExtend ? "" : doorbellParameter.m_strExtend.c_str());

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
    const char *sqlfmt = "select doorbell_name, serial_number, doorbell_p2pid, battery_capacity, charging_state, wifi_signal, volume_level,"
        " version_number, channel_number, coding_type, pir_alarm_swtich, doorbell_switch, pir_alarm_level, pir_ineffective_time, current_wifi, sub_category, disturb_mode, anti_theft, extend"
        " from t_device_parameter_doorbell where deviceid = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strDeviceID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        switch (uiColumnNum)
        {
        case 0:
            doorbellParameter.m_strDoorbellName = strColumn;
            break;
        case 1:
            doorbellParameter.m_strSerialNumber = strColumn;
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
            break;
        case 15:
            doorbellParameter.m_strSubCategory = strColumn;
            break;
        case 16:
            doorbellParameter.m_strDisturbMode = strColumn;
            break;
        case 17:
            doorbellParameter.m_strAntiTheftSwitch = strColumn;
            break;
        case 18:
            doorbellParameter.m_strExtend = strColumn;
            Result = doorbellParameter;
            break;

        default:
            LOG_ERROR_RLD("QueryDoorbellParameterToDB sql callback error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
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
    doorbellParameter.m_strSubCategory = result.m_strSubCategory;
    doorbellParameter.m_strDisturbMode = result.m_strDisturbMode;
    doorbellParameter.m_strAntiTheftSwitch = result.m_strAntiTheftSwitch;
    doorbellParameter.m_strExtend = result.m_strExtend;

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

bool AccessManager::QueryIfDeviceReportedToDB(const std::string &strP2PID, const unsigned int uiDeviceType, std::string &strDeviceID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt;
    if (DEVICE_TYPE_DOORBELL == uiDeviceType)
    {
        sqlfmt = "select deviceid from t_device_parameter_doorbell where doorbell_p2pid = '%s' and status = 0";
    }
    else if (DEVICE_TYPE_IPC == uiDeviceType)
    {
        sqlfmt = "select deviceid from t_device_parameter_ipc where p2pid = '%s' and status = 0";
    }
    else
    {
        LOG_ERROR_RLD("QueryIfDeviceReportedToDB failed, unknown device type: " << uiDeviceType);
        return false;
    }
    snprintf(sql, sizeof(sql), sqlfmt, strP2PID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        Result = strColumn;

        LOG_INFO_RLD("QueryIfDeviceReportedToDB sql result id is " << strColumn);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryIfDeviceReportedToDB exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryIfDeviceReportedToDB sql result is empty, sql is " << sql);
        return false;
    }

    strDeviceID = boost::any_cast<std::string>(ResultList.front());
    return true;
}

bool AccessManager::QueryPlatformPushStatusToDB(std::string &strStatus)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select description from t_configuration_info where category = '%s' and subcategory = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, "Platform_Feature", "Push_Support");

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        Result = strColumn;

        LOG_INFO_RLD("QueryPlatformPushStatusToDB sql result description is " << strColumn);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("QueryPlatformPushStatusToDB exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryPlatformPushStatusToDB sql result is empty, sql is " << sql);
        strStatus = "0";
    }
    else
    {
        strStatus = boost::any_cast<std::string>(ResultList.front());
    }

    return true;
}

void AccessManager::InsertDeviceEventReportToDB(const std::string &strEventID, const std::string &strDeviceID, const unsigned int uiDeviceType,
    const unsigned int uiEventType, const unsigned int uiEventState, const unsigned int uiMessageStatus, const std::string &strFileID, const std::string &strEventTime)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_device_event_info (id, eventid, deviceid, devicetype, eventtype, eventstate, readstate, fileid, createdate, status)"
        " values(uuid(), '%s', '%s', %d,  %d,  %d,  %d, '%s', '%s', %d)";
    snprintf(sql, sizeof(sql), sqlfmt, strEventID.c_str(), strDeviceID.c_str(), uiDeviceType, uiEventType, uiEventState, uiMessageStatus,
        strFileID.c_str(), strEventTime.c_str(), NORMAL_STATUS);

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("InsertDeviceEventReportToDB exec sql error, sql is " << sql);
        return;
    }

    //当文件ID不为空的时候，需要将对应的文件转换为mp4文件，目前暂不区分其它处理方式
    if (!strFileID.empty())
    {
        m_EventFileProcessRunner.Post(boost::bind(&AccessManager::FileProcessHandler, this, strEventID, strFileID));
    }
}

bool AccessManager::QueryAllDeviceEventToDB(const std::string &strDeviceID, const unsigned int uiEventType, const unsigned int uiReadState,
    std::list<InteractiveProtoHandler::DeviceEvent> &deviceEventList, const std::string &strBeginDate, const std::string &strEndDate,
    const unsigned int uiBeginIndex, const unsigned int uiPageSize)
{
    unsigned int uiExpireTime;
    if (!QueryDeviceEventExpireTimeToDB(uiExpireTime))
    {
        LOG_ERROR_RLD("QueryAllDeviceEventToDB failed, query event expire time error, device id is " << strDeviceID);
        return false;
    }

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len;
    const char *sqlfmt = "select deviceid, devicetype, eventid, eventtype, eventstate, fileid, thumbnail, createdate, readstate from t_device_event_info"
        " where deviceid = '%s' and storedtime < %d and status = 0";
    snprintf(sql, size, sqlfmt, strDeviceID.c_str(), uiExpireTime);

    if (0 != uiEventType)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, " and eventtype = %d", uiEventType);
    }

    if (!strBeginDate.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, " and createdate >= '%s'", strBeginDate.c_str());
    }

    if (!strEndDate.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, " and createdate <= '%s'", strEndDate.c_str());
    }

    len = strlen(sql);
    if (EVENT_MESSAGE_ALL == uiReadState)
    {
        snprintf(sql + len, size - len, " order by createdate desc limit %d, %d", uiBeginIndex, uiPageSize);
    }
    else
    {
        snprintf(sql + len, size - len, " and readstate = %d order by createdate desc limit %d, %d", uiReadState, uiBeginIndex, uiPageSize);
    }

    InteractiveProtoHandler::DeviceEvent deviceEvent;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        switch (uiColumnNum)
        {
        case 0:
            deviceEvent.m_strDeviceID = strColumn;
            break;
        case 1:
            deviceEvent.m_uiDeviceType = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 2:
            deviceEvent.m_strEventID = strColumn;
            break;
        case 3:
            deviceEvent.m_uiEventType = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 4:
            deviceEvent.m_uiEventState = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 5:
            deviceEvent.m_strFileUrl = strColumn.empty() ? strColumn :
                m_ParamInfo.m_strFileServerURL + "download_file&fileid=" + strColumn;
            break;
        case 6:
            deviceEvent.m_strThumbnailUrl = strColumn.empty() ? strColumn :
                m_ParamInfo.m_strFileServerURL + "download_file&fileid=" + strColumn;
            break;
        case 7:
            deviceEvent.m_strEventTime = strColumn;
            break;
        case 8:
            deviceEvent.m_uiReadState = boost::lexical_cast<unsigned int>(strColumn);
            Result = deviceEvent;
            break;

        default:
            LOG_ERROR_RLD("QueryAllDeviceEventToDB sql callback error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryAllDeviceEventToDB exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryAllDeviceEventToDB sql result is empty, sql is " << sql);
        return true;
    }

    for (auto &result : ResultList)
    {
        deviceEventList.push_back(std::move(boost::any_cast<InteractiveProtoHandler::DeviceEvent>(result)));
    }

    return true;
}

void AccessManager::UpdateEventReadStatusToDB(std::list<std::string> strEventIDList, const unsigned int uiReadStatus)
{
    if (strEventIDList.empty())
    {
        LOG_INFO_RLD("UpdateEventReadStatusToDB completed, event id list is empty");
        return;
    }

    char sql[1024] = { 0 };
    const char *sqlfmt = "update t_device_event_info set readstate = %d where readstate = %d and status = 0 and eventid in(";
    snprintf(sql , sizeof(sql), sqlfmt, uiReadStatus, EVENT_MESSAGE_UNREAD);
    std::string strSql(sql);

    for (std::string &strEventID : strEventIDList)
    {
        strSql += "'" + strEventID + "',";
    }

    strSql.replace(strSql.length() - 1, 1, std::string(")"));

    if (!m_pMysql->QueryExec(strSql))
    {
        LOG_ERROR_RLD("UpdateEventReadStatusToDB exec sql error, sql is " << strSql);
    }
}

void AccessManager::DeleteDeviceEventToDB(const std::string &strEventID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_device_event_info where eventid = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEventID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteDeviceEventToDB exec sql failed, sql is " << sql);
    }
}

void AccessManager::ModifyDeviceEventToDB(const std::string &strEventID, const unsigned int uiEventState,
    const std::string &strUpdateTime, const std::string &strFileID, const unsigned int uiReadState)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len;
    snprintf(sql, size, "update t_device_event_info set id = id");

    bool blModified = false;

    if (UNUSED_INPUT_UINT != uiEventState)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", eventstate = %d", uiEventState);

        blModified = true;
    }

    if (!strUpdateTime.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", createdate = '%s'", strUpdateTime.c_str());

        blModified = true;
    }

    if (!strFileID.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", fileid = '%s'", strFileID.c_str());

        blModified = true;
    }

    if (UNUSED_INPUT_UINT != uiReadState)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", readstate = %d", uiReadState);

        blModified = true;
    }

    if (!blModified)
    {
        LOG_INFO_RLD("ModifyDeviceEventToDB completed, there is no change");
        return;
    }

    len = strlen(sql);
    snprintf(sql + len, size - len, " where eventid = '%s' and status = 0", strEventID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("ModifyDeviceEventToDB exec sql failed, sql is " << sql);
        return;
    }

    //当文件ID不为空的时候，需要将对应的文件转换为mp4文件，目前暂不区分其它处理方式
    if (!strFileID.empty())
    {
        m_EventFileProcessRunner.Post(boost::bind(&AccessManager::FileProcessHandler, this, strEventID, strFileID));
    }
}

bool AccessManager::QuerySharedDeviceNameToDB(const std::string &strUserID, const std::string &strDeviceID,
    std::string &strDeviceName)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select devicename from t_user_device_relation where userid = '%s' and"
        " devicekeyid = (select id from t_device_info where deviceid = '%s' and status = 0) and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), strDeviceID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        Result = strColumn;

        LOG_INFO_RLD("QuerySharedDeviceNameToDB sql result device name is " << strColumn);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("QuerySharedDeviceNameToDB exec sql error, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QuerySharedDeviceNameToDB sql result is empty, sql is " << sql);
        return true;
    }

    strDeviceName = boost::any_cast<std::string>(ResultList.front());
    return true;
}

void AccessManager::RemoveExpiredDeviceEventToDB(const std::string &strDeviceID, const bool blRemoveAll)
{
    unsigned int uiExpireTime = 0;
    if(!blRemoveAll && !QueryDeviceEventExpireTimeToDB(uiExpireTime))
    {
        LOG_ERROR_RLD("RemoveExpiredDeviceEventToDB failed, query device event expire time error, device id is " << strDeviceID);
        return;
    }

    RemoveExpiredDeviceEventFile(strDeviceID, uiExpireTime);

    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_device_event_info where deviceid = '%s' and storedtime >= %d and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strDeviceID.c_str(), uiExpireTime);

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("RemoveExpiredDeviceEventToDB exec sql error, sql is " << sql);
    }
}

void AccessManager::RemoveExpiredDeviceEventFile(const std::string &strDeviceID, const unsigned int uiExpiredTime)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select fileid, thumbnail from t_device_event_info where deviceid = '%s' and storedtime >= %d";
    snprintf(sql, sizeof(sql), sqlfmt, strDeviceID.c_str(), uiExpiredTime);

    struct FilePath
    {
        std::string fileid;
        std::string thumbnail;
    } filepath;

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        switch (uiColumnNum)
        {
        case 0:
            filepath.fileid = strColumn;
            break;
        case 1:
            filepath.thumbnail = strColumn;
            Result = filepath;
            break;

        default:
            LOG_ERROR_RLD("RemoveExpiredDeviceEventFile sql callback error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("RemoveExpiredDeviceEventFile exec sql error, sql is " << sql);
        return;
    }

    for (auto &result : ResultList)
    {
        auto path = boost::any_cast<FilePath>(result);
        RemoveRemoteFile(path.fileid);
        RemoveRemoteFile(path.thumbnail);
    }
}

bool AccessManager::RemoveRemoteFile(const std::string &strFileID)
{
    if (strFileID.empty())
    {
        LOG_INFO_RLD("RemoveRemoteFile completed, file id is empty");
        return true;
    }

    std::map<std::string, std::string> reqFormMap;
    reqFormMap.insert(std::make_pair("fileid", strFileID));

    std::string strRsp;
    std::string strUrl = m_ParamInfo.m_strFileServerURL + "delete_file";
    HttpClient httpClient;
    if (CURLE_OK != httpClient.PostForm(strUrl, reqFormMap, strRsp))
    {
        LOG_ERROR_RLD("RemoveRemoteFile send http post failed, url is " << strUrl << " and file id is " << strFileID);
        return false;
    }

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(strRsp, root))
    {
        LOG_ERROR_RLD("RemoveRemoteFile failed, parse http post response data error, raw data is: " << strRsp);
        return false;
    }

    auto retcode = root["retcode"];
    if (retcode.isNull() || !retcode.isString() || "0" != retcode.asString())
    {
        LOG_ERROR_RLD("RemoveRemoteFile failed, http post return error, raw data is: " << strRsp);
        return false;
    }

    LOG_INFO_RLD("RemoveRemoteFile successful, post url is " << strUrl << " and file id is " << strFileID);
    return true;
}

bool AccessManager::QueryDeviceEventExpireTimeToDB(unsigned int &uiExpireTime)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select description from t_configuration_info where category = '%s' and subcategory = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, "Platform_Feature", "Event_Expire_Time");

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        Result = boost::lexical_cast<unsigned int>(strColumn);

        LOG_INFO_RLD("QueryDeviceEventExpireTimeToDB sql result event expire time is " << strColumn);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("QueryDeviceEventExpireTimeToDB exec sql error, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryDeviceEventExpireTimeToDB sql result is empty, sql is " << sql);

        uiExpireTime = 3 * 24;  //如果没有配置则事件过期时间则设置为3天
        return true;
    }

    uiExpireTime = boost::any_cast<unsigned int>(ResultList.front());
    return true;
}

void AccessManager::InsertStorageDetailToDB(const InteractiveProtoHandler::StorageDetail &storageDetail)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_storage_detail_info (id, domainid, objid, objtype, storagename, storagetype, overlaptype,"
        " storagetimeuplimit, storagetimedownlimit, sizeofspaceused, storageunittype, begindate, enddate, status, extend)"
        " values(uuid(), %d, '%s', %d, '%s', %d, %d, %d, %d, %d, %d, '%s', '%s', %d, '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, storageDetail.m_uiDomainID, storageDetail.m_strObjectID.c_str(), storageDetail.m_uiObjectType,
        storageDetail.m_strStorageName.c_str(), storageDetail.m_uiStorageType, storageDetail.m_uiOverlapType, storageDetail.m_uiStorageTimeUpLimit,
        storageDetail.m_uiStorageTimeDownLimit, storageDetail.m_uiSizeOfSpaceUsed, storageDetail.m_uiStorageUnitType, storageDetail.m_strBeginDate.c_str(),
        storageDetail.m_strEndDate.c_str(), storageDetail.m_uiStatus, storageDetail.m_strExtend.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("InsertStorageDetailToDB exec sql error, sql is " << sql);
    }
}

void AccessManager::DeleteStorageDetailToDB(const std::string &strObjectID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "update t_storage_detail_info set status = %d where objid = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, DELETE_STATUS, strObjectID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteStorageDetailToDB exec sql error, sql is " << sql);
    }
}

void AccessManager::UpdateStorageDetailToDB(const InteractiveProtoHandler::StorageDetail &storageDetail)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len;
    snprintf(sql, size, "update t_storage_detail_info set objid = objid");

    bool blModified = false;

    if (!storageDetail.m_strStorageName.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", storagename = '%s'", storageDetail.m_strStorageName.c_str());

        blModified = true;
    }

    if (UNUSED_INPUT_UINT != storageDetail.m_uiOverlapType)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", overlaptype = %d", storageDetail.m_uiOverlapType);

        blModified = true;
    }

    if (UNUSED_INPUT_UINT != storageDetail.m_uiStorageTimeUpLimit)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", storagetimeuplimit = %d", storageDetail.m_uiStorageTimeUpLimit);

        blModified = true;
    }

    if (UNUSED_INPUT_UINT != storageDetail.m_uiStorageTimeDownLimit)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", storagetimedownlimit = %d", storageDetail.m_uiStorageTimeDownLimit);

        blModified = true;
    }

    if (!storageDetail.m_strBeginDate.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", begindate = '%s'", storageDetail.m_strBeginDate.c_str());

        blModified = true;
    }

    if (!storageDetail.m_strEndDate.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", enddate = '%s'", storageDetail.m_strEndDate.c_str());

        blModified = true;
    }

    if (!storageDetail.m_strExtend.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, ", extend = '%s'", storageDetail.m_strExtend.c_str());

        blModified = true;
    }

    if (!blModified)
    {
        LOG_INFO_RLD("UpdateStorageDetailToDB completed, there is no change");
        return;
    }

    len = strlen(sql);
    snprintf(sql + len, size - len, " where objid = '%s' and status = 0", storageDetail.m_strObjectID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("UpdateStorageDetailToDB exec sql failed, sql is " << sql);
    }
}

bool AccessManager::QueryStorageDetailToDB(const std::string &strObjectID, InteractiveProtoHandler::StorageDetail &storageDetail, int &iErrorCode)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select domainid, storagename, overlaptype, storagetimeuplimit, storagetimedownlimit, begindate, enddate, extend"
        " from t_storage_detail_info where objid = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strObjectID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        switch (uiColumnNum)
        {
        case 0:
            storageDetail.m_uiDomainID = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 1:
            storageDetail.m_strStorageName = strColumn;
            break;
        case 2:
            storageDetail.m_uiOverlapType = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 3:
            storageDetail.m_uiStorageTimeUpLimit = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 4:
            storageDetail.m_uiStorageTimeDownLimit = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 5:
            storageDetail.m_strBeginDate = strColumn;
            break;
        case 6:
            storageDetail.m_strEndDate = strColumn;
            break;
        case 7:
            storageDetail.m_strExtend = strColumn;
            Result = storageDetail;
            break;

        default:
            LOG_ERROR_RLD("QueryStorageDetailToDB sql callback error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("QueryStorageDetailToDB exec sql failed, sql is " << sql);

        iErrorCode = ReturnInfo::FAILED_CODE;
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryStorageDetailToDB sql result is empty, sql is " << sql);

        iErrorCode = ReturnInfo::CLOUD_STORAGE_NOT_PAID_USER;
        return false;
    }

    iErrorCode = ReturnInfo::SUCCESS_CODE;
    storageDetail = boost::any_cast<InteractiveProtoHandler::StorageDetail>(ResultList.front());
    return true;
}

bool AccessManager::QueryRegionStorageInfoToDB(unsigned int &uiUsedSize, unsigned int &uiTotalSize)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select sizeofspace, sizeofspaceused from t_storage_info";
    snprintf(sql, sizeof(sql), sqlfmt);

    struct StorageInfo
    {
        unsigned int usedSize;
        unsigned int totalSize;
    } storageInfo;

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        switch (uiColumnNum)
        {
        case 0:
            storageInfo.totalSize = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 1:
            storageInfo.usedSize = boost::lexical_cast<unsigned int>(strColumn);
            Result = storageInfo;
            break;

        default:
            LOG_ERROR_RLD("QueryRegionStorageInfoToDB sql callback error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("QueryRegionStorageInfoToDB exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryRegionStorageInfoToDB sql result is empty, sql is " << sql);
        return false;
    }

    auto result = boost::any_cast<StorageInfo>(ResultList.front());
    uiUsedSize = result.usedSize;
    uiTotalSize = result.totalSize;

    return true;
}

bool AccessManager::QueryDeviceInfoMultiToDB(const std::list<std::string> &strDeviceIDList, std::list<InteractiveProtoHandler::DeviceStatus> &deviceStatusList)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select deviceid, devicename, devicepassword, p2pid, domainname, innerinfo, typeinfo, extend from t_device_info"
        " where status = 0 and deviceid in (";
    int len = snprintf(sql, size, sqlfmt);

    for (auto device : strDeviceIDList)
    {
        len += snprintf(sql + len, size - len, "'%s', ", device.c_str());
    }

    std::string strSql(sql);
    strSql.replace(strSql.size() - 2, 2, std::string(")"));

    InteractiveProtoHandler::Device rstDevice;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstDevice.m_strDevID = strColumn;
            break;
        case 1:
            rstDevice.m_strDevName = strColumn;
            break;
        case 2:
            rstDevice.m_strDevPassword = strColumn;
            break;
        case 3:
            rstDevice.m_strP2pID = strColumn;
            break;
        case 4:
            rstDevice.m_strDomainName = strColumn;
            break;
        case 5:
            rstDevice.m_strInnerinfo = strColumn;            
            break;
        case 6:
            rstDevice.m_uiTypeInfo = boost::lexical_cast<unsigned int>(strColumn);            
            break;
        case 7:
            rstDevice.m_strExtend = strColumn;
            Result = rstDevice;
            break;
        default:
            LOG_ERROR_RLD("QueryDeviceInfoMultiToDB sql callback error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(strSql, ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("QueryDeviceInfoMultiToDB exec sql failed, sql is " << strSql);
        return false;
    }

    for (auto result : ResultList)
    {
        InteractiveProtoHandler::DeviceStatus deviceStatus;
        deviceStatus.m_deviceInfo = boost::any_cast<InteractiveProtoHandler::Device>(result);
        deviceStatus.m_strOnlineStatus = m_SessionMgr.ExistID(deviceStatus.m_deviceInfo.m_strDevID) ? ONLINE : OFFLINE;

        deviceStatusList.push_back(std::move(deviceStatus));
    }

    return true;
}

void AccessManager::UpdateDeviceEventStoredTime()
{
    const char *sql = "update t_device_event_info set storedtime = storedtime + 1 where status = 0";

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("UpdateDeviceEventStoredTime exec sql error, sql is " << sql);
    }
}

void AccessManager::FileProcessHandler(const std::string &strEventID, const std::string &strFileID)
{
    //查询文件id对应的本地路径
    std::map<std::string, std::string> reqFormMap;
    reqFormMap.insert(std::make_pair("fileid", strFileID));

    std::string strRsp;
    std::string strUrl = m_ParamInfo.m_strFileServerURL + "query_file";
    HttpClient httpClient;
    if (CURLE_OK != httpClient.PostForm(strUrl, reqFormMap, strRsp))
    {
        LOG_ERROR_RLD("Query file info send http post failed, url is " << strUrl << " and file id is " << strFileID);
        return;
    }

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(strRsp, root))
    {
        LOG_ERROR_RLD("Query file info failed, parse http post response data error, raw data is : " << strRsp);
        return;
    }

    if (!root.isObject())
    {
        LOG_ERROR_RLD("Query file info failed, parse http post response data error, raw data is : " << strRsp);
        return;
    }

    auto retcode = root["retcode"];
    if (retcode.isNull() || !retcode.isString() || "0" != retcode.asString())
    {
        LOG_ERROR_RLD("Query file info failed, http post return error, raw data is: " << strRsp);
        return;
    }

    auto jsLocalPath = root["localpath"];
    if (jsLocalPath.isNull() || !jsLocalPath.isString() || jsLocalPath.asString().empty())
    {
        LOG_ERROR_RLD("Query file info failed, http post return error, raw data is: " << strRsp);
        return;
    }

    std::string strResolution("1280x720");


    Json::Value jsBody;
    jsBody["localpath"] = jsLocalPath.asString();
    jsBody["fileid"] = strFileID;
    jsBody["eventid"] = strEventID;
    jsBody["img_resolution"] = strResolution;
    
    Json::FastWriter fastwriter;
    const std::string &strBody = fastwriter.write(jsBody); //jsBody.toStyledString();

    LOG_INFO_RLD("File id of event is " << strFileID << " and event id is " << strEventID << " and file local path is " << jsLocalPath.asString()
        << " and send file handler process msg is " << strBody);

    m_MsgSender->SendMsg(strBody);

    ////更新事件记录中的文件ID字段
    //std::string strFileIDOfMp4 = strFileID + ".mp4";
    //char sql[1024] = { 0 };
    //const char *sqlfmt = "update t_device_event_info set  fileid = '%s' where eventid = '%s'";
    //snprintf(sql, sizeof(sql), sqlfmt, strFileIDOfMp4.c_str(), strEventID.c_str());

    //if (!m_pMysql->QueryExec(std::string(sql)))
    //{
    //    LOG_ERROR_RLD("Update device event error, sql is " << sql);
    //    return;
    //}
    //
    ////删除无用的原始文件
    //RemoveRemoteFile(strFileID);
}

void AccessManager::DeleteDeviceParameter(const std::string &strDeviceID)
{
    DeleteIPCParameter(strDeviceID);
    DeleteDoorbellParameter(strDeviceID);
}

void AccessManager::DeleteIPCParameter(const std::string &strDeviceID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_device_parameter_ipc where deviceid = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strDeviceID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteIPCParameter exec sql error, sql is " << sql);
    }
}

void AccessManager::DeleteDoorbellParameter(const std::string &strDeviceID)
{
    //设备参数不需要标记为删除状态，直接删除
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_device_parameter_doorbell where deviceid = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strDeviceID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteDoorbellParameter exec sql error, sql is " << sql);
    }
}

bool AccessManager::QueryUserIDByUserName(const std::string &strUserName, std::string &strUserID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select userid from t_user_info where username = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strUserName.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        Result = strColumn;
        LOG_INFO_RLD("QueryUserIDByUserName sql result user id is " << strColumn);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("QueryUserIDByUserName exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryUserIDByUserName sql result is empty, sql is " << sql);
        return false;
    }

    strUserID = boost::any_cast<std::string>(ResultList.front());
    return true;
}

bool AccessManager::RefreshCmsCallInfo()
{
    boost::unique_lock<boost::mutex> lock(m_CmsMutex);

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select cmsid, cmsp2pid, address, port from t_cms_call_info"
        " where status = 0";
    snprintf(sql, size, sqlfmt);

    std::string strSql(sql);

    CmsCall cmscall;
    
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        switch (uiColumnNum)
        {
        case 0:
            cmscall.m_strCmsID = strColumn;
            break;

        case 1:
            cmscall.m_strCmsP2pID = strColumn;            
            break;

        case 2:
            cmscall.m_strAddress = strColumn;
            break;

        case 3:
            cmscall.m_strPort = strColumn;
            Result = cmscall;
            break;

        default:
            LOG_ERROR_RLD("Refresh cms call info sql callback error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;

        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(strSql, ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("RefreshCmsCallInfo exec sql failed, sql is " << strSql);
        return false;
    }

    m_CmsCallInfoMap.clear();

    for (const auto &result : ResultList)
    {
        cmscall = boost::any_cast<CmsCall>(result);

        auto itFind = m_CmsCallInfoMap.find(cmscall.m_strCmsID);
        if (m_CmsCallInfoMap.end() == itFind)
        {
            std::list<CmsCall> strCmsCallInfoList;
            strCmsCallInfoList.push_back(cmscall);
            m_CmsCallInfoMap.insert(std::map<std::string, std::list<CmsCall> >::value_type(cmscall.m_strCmsID, strCmsCallInfoList));
        }
        else
        {
            itFind->second.push_back(cmscall);
        }
    }

    return true;
}

bool AccessManager::GetCmsCallInfo(const std::string &strCmsID, const std::string &strDeviceP2pID, std::string &strAddress, std::string &strPort)
{
    boost::unique_lock<boost::mutex> lock(m_CmsMutex);

    if (!strCmsID.empty())
    {
        auto itFind = m_CmsCallInfoMap.find(strCmsID);
        if (m_CmsCallInfoMap.end() == itFind)
        {
            LOG_ERROR_RLD("Cms id not found, value is " << strCmsID);
            return false;
        }

        strAddress = itFind->second.front().m_strAddress;
        strPort = itFind->second.front().m_strPort;

        LOG_INFO_RLD("Cms id found and value is " << strCmsID << " and address is " << strAddress << " and port is " << strPort);
        return true;
    }

    if (!strDeviceP2pID.empty())
    {
        for (auto itBegin = m_CmsCallInfoMap.begin(), itEnd = m_CmsCallInfoMap.end(); itBegin != itEnd; ++itBegin)
        {
            for (auto itB1 = itBegin->second.begin(), itE1 = itBegin->second.end(); itB1 != itE1; ++itB1)
            {
                LOG_INFO_RLD("P2pid in memory is " << itB1->m_strCmsP2pID);

                if (strDeviceP2pID == itB1->m_strCmsP2pID)
                {
                    strAddress = itB1->m_strAddress;
                    strPort = itB1->m_strPort;

                    LOG_INFO_RLD("Device p2pid found and p2pid is " << strDeviceP2pID << " and address is " << strAddress << " and port is " << strPort);
                    return true;
                }
            }
        }
    }

    LOG_ERROR_RLD("Not found address and port, cms id is " << strCmsID << " device p2pid is " << strDeviceP2pID);
    return false;
}

bool AccessManager::SaveCmsCallInfo(const std::string &strCmsID)
{
    boost::unique_lock<boost::mutex> lock(m_CmsMutex);

    char sql[1024] = { 0 };
    const char* sqlfmt = "delete from t_cms_call_info where cmsid = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strCmsID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("delete t_cms_call_info sql exec failed, sql is " << std::string(sql));
        return false;
    }

    auto itFind = m_CmsCallInfoMap.find(strCmsID);
    if (m_CmsCallInfoMap.end() == itFind)
    {
        LOG_ERROR_RLD("Cmd id not found, value is " << strCmsID);
        return false;
    }

    for (auto itBegin = itFind->second.begin(), itEnd = itFind->second.end(); itBegin != itEnd; ++itBegin)
    {
        char sql[1024] = { 0 };
        const char* sqlfmt = "insert into t_cms_call_info ("
            "id, cmsid, cmsp2pid, address, port)"
            " values(uuid(), '%s', '%s', '%s', '%s')";

        snprintf(sql, sizeof(sql), sqlfmt, itBegin->m_strCmsID.c_str(), itBegin->m_strCmsP2pID.c_str(), 
            itBegin->m_strAddress.c_str(), itBegin->m_strPort.c_str());

        if (!m_pMysql->QueryExec(std::string(sql)))
        {
            LOG_ERROR_RLD("Insert t_cms_call_info sql exec failed, sql is " << std::string(sql));
            return false;
        }
    }

    return true;
}

bool AccessManager::ValidCmsCallInfo(const std::string &strCmsID, const std::list<std::string> &strCmsP2pIDList, std::list<std::string> &strP2pIDFailedList)
{
    boost::unique_lock<boost::mutex> lock(m_CmsMutex);
    if (strCmsID.empty())
    {
        LOG_ERROR_RLD("Cms id is empty.");
        return false;
    }

    bool blFlag = true;
    for (auto itB1 = strCmsP2pIDList.begin(), itE1 = strCmsP2pIDList.end(); itB1 != itE1; ++itB1)
    {
        for (auto itBegin = m_CmsCallInfoMap.begin(), itEnd = m_CmsCallInfoMap.end(); itBegin != itEnd; ++itBegin)
        {
            if (itBegin->first == strCmsID) //找到原来旧的cmsid对应的信息，则忽略掉
            {
                LOG_INFO_RLD("Old cms info found and continue.");
                continue;
            }

            for (auto itB2 = itBegin->second.begin(), itE2 = itBegin->second.end(); itB2 != itE2; ++itB2)
            {
                if (*itB1 == itB2->m_strCmsP2pID)
                {
                    strP2pIDFailedList.push_back(*itB1);

                    LOG_ERROR_RLD("Valid failed because cmsid of pending have same value in current map and p2p id is " << *itB1
                        << " and cms id is " << strCmsID);

                    blFlag = false;
                    break;
                }
            }
        }
    }

    if (!blFlag)
    {
        return false;
    }

    LOG_INFO_RLD("Valid success and cms id is " << strCmsID);
    return true;
}

bool AccessManager::UpdateCmsCallInfo(const std::string &strCmsID, const std::list<CmsCall> &CmsCallList)
{
    boost::unique_lock<boost::mutex> lock(m_CmsMutex);
    if (strCmsID.empty())
    {
        LOG_ERROR_RLD("Cms id is empty.");
        return false;
    }

    auto itFind = m_CmsCallInfoMap.find(strCmsID);
    if (m_CmsCallInfoMap.end() == itFind)
    {
        LOG_ERROR_RLD("Cmd id not found, value is " << strCmsID);
        return false;
    }

    m_CmsCallInfoMap.erase(itFind);

    m_CmsCallInfoMap.insert(std::map<std::string, std::list<CmsCall> >::value_type(strCmsID, CmsCallList));

    return true;
}

bool AccessManager::AddCmsCallInfo(const std::string &strCmsID, const std::list<CmsCall> &CmsCallList)
{
    boost::unique_lock<boost::mutex> lock(m_CmsMutex);
    if (strCmsID.empty())
    {
        LOG_ERROR_RLD("Cms id is empty.");
        return false;
    }

    auto itFind = m_CmsCallInfoMap.find(strCmsID);
    if (m_CmsCallInfoMap.end() != itFind)
    {
        LOG_ERROR_RLD("Cmd id found, value is " << strCmsID);
        return false;
    }

    m_CmsCallInfoMap.insert(std::map<std::string, std::list<CmsCall> >::value_type(strCmsID, CmsCallList));

    return true;
}

void AccessManager::AllocCmsAddressAndPort(std::string &strAddress, std::string &strPort)
{
    boost::unique_lock<boost::mutex> lock(m_CmsMutex);

    strAddress = m_ParamInfo.m_strCmsCallAddress;

    unsigned int uiMaxPort = 16990;
    for (auto itBegin = m_CmsCallInfoMap.begin(), itEnd = m_CmsCallInfoMap.end(); itBegin != itEnd; ++itBegin)
    {
        unsigned int uiPort = boost::lexical_cast<unsigned int>(itBegin->second.front().m_strPort);
        if (uiPort > uiMaxPort)
        {
            uiMaxPort = uiPort;
        }
    }

    ++uiMaxPort;

    strPort = boost::lexical_cast<std::string>(uiMaxPort);
}

bool AccessManager::RemoveCmsCallInfo(const std::string &strCmsID)
{
    boost::unique_lock<boost::mutex> lock(m_CmsMutex);
    if (strCmsID.empty())
    {
        LOG_ERROR_RLD("Cms id is empty.");
        return false;
    }

    auto itFind = m_CmsCallInfoMap.find(strCmsID);
    if (m_CmsCallInfoMap.end() == itFind)
    {
        LOG_ERROR_RLD("Cmd id not found, value is " << strCmsID);
        return false;
    }

    m_CmsCallInfoMap.erase(itFind);

    return true;
}

bool AccessManager::SaveRemoveCmsCallInfo(const std::string &strCmsID)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "delete from t_cms_call_info where cmsid = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strCmsID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("update t_cms_call_info sql exec failed, sql is " << std::string(sql));
        return false;
    }
    
    return true;
}

bool AccessManager::QuerySharingDeviceLimit(const std::string &strUserID, unsigned int &uiUsedNum)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select count(rel.id) from t_device_info dev, t_user_device_relation rel, t_user_info usr "
        "where dev.id = rel.devicekeyid and usr.userid = rel.userid and rel.status = 0 and dev.status = 0 and usr.status = 0 "
        "and rel.relation = 1 and rel.deviceid "
        "in (select deviceid from t_user_device_relation where userid = '%s' and relation = 0 and status = 0)";
    snprintf(sql, size, sqlfmt, strUserID.c_str());

    unsigned int uiResult = 0;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        uiResult = boost::lexical_cast<unsigned int>(strColumn);
        Result = uiResult;

        LOG_INFO_RLD("Query sharing device sql count(id) is " << uiResult);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("Query sharing device failed, user id is " << strUserID);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("Query sharing device failed, user id is " << strUserID);
        return false;
    }

    auto result = boost::any_cast<unsigned int>(ResultList.front());
    uiResult = result;

    uiUsedNum = uiResult;

    return true;
}

bool AccessManager::QuerySharingDeviceCurrentLimit(unsigned int &uiCurrentLimitNum)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select description from t_configuration_info where category = 'Platform_Feature' and subcategory = 'SharingLimitNum'";
    snprintf(sql, size, sqlfmt);

    unsigned int uiResult = SHARING_DEVICE_LIMIT;

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        uiResult = boost::lexical_cast<unsigned int>(strColumn);
        Result = uiResult;

        LOG_INFO_RLD("Query sharing device current limit is " << uiResult);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("Query sharing device current limit failed");
        return false;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("Query sharing device current limit empty and default limit is " << uiResult);
    }
    else
    {
        auto result = boost::any_cast<unsigned int>(ResultList.front());
        uiResult = result;
    }

    uiCurrentLimitNum = uiResult;

    return true;
}

bool AccessManager::QueryDeviceCapacity(const std::string &strUserID, const unsigned int uiDevType, DeviceCapacity &devcap)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select capacity_id from t_device_type_capacity_info where device_type = %u and status = 0";
    snprintf(sql, size, sqlfmt, uiDevType);
    
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        switch (uiColumnNum)
        {
        case 0:
            Result = strColumn;            
            break;

        default:
            LOG_ERROR_RLD("Query device capacity sql callback error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("Query device capacity exec sql failed, sql is " << sql);
        return false;
    }

    for (const auto &result : ResultList)
    {
        devcap.m_strCapacityList.push_back(boost::any_cast<std::string>(result));               
    }

    return true;
}

bool AccessManager::QueryAllDeviceCapacity(const std::string &strUserID, std::list<DeviceCapacity> &devcaplist)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select device_type, capacity_id from t_device_type_capacity_info where status = 0 order by device_type";
    snprintf(sql, size, sqlfmt);

    std::map<unsigned int, std::list<std::string> > devcapmap;
    unsigned int uiKeyTmp = 0xFFFFFFFF;
    unsigned int uiDevType = 0xFFFFFFFF;

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {  
        switch (uiColumnNum)
        {
        case 0:
            uiDevType = uiKeyTmp = boost::lexical_cast<unsigned int>(strColumn);
            if (devcapmap.end() == devcapmap.find(uiDevType))
            {                
                devcapmap.insert(std::map<unsigned int, std::list<std::string> >::value_type(uiDevType, std::list<std::string>()));
            }
            break;

        case 1:
            if (devcapmap.end() == devcapmap.find(uiKeyTmp))
            {
                LOG_ERROR_RLD("Can not found key tmp " << uiKeyTmp);
                break;
            }

            devcapmap.find(uiKeyTmp)->second.push_back(strColumn);            

            break;

        default:
            LOG_ERROR_RLD("Query all device capacity sql callback error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("Query all device capacity exec sql failed, sql is " << sql);
        return false;
    }

    for (const auto &result : devcapmap)
    {
        DeviceCapacity devcap;
        devcap.m_uiDevType = result.first;
        devcap.m_strCapacityList = result.second;

        devcaplist.push_back(std::move(devcap));
    }

    return true;
}

bool AccessManager::QueryDeviceP2pID(const std::string &strDomainName, DevP2pIDInfo &devp2pinfo)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select updatedate, webport, mobileport, channelcount, deviceid, p2pid, extend"
        " from t_device_parameter_ipc where devicedomain = '%s' and status = 0";
    snprintf(sql, size, sqlfmt, strDomainName.c_str());
    
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        switch (uiColumnNum)
        {
        case 0:
            devp2pinfo.m_strUpdateTime = strColumn;
            break;

        case 1:
            devp2pinfo.m_strWebPort = strColumn;
            break;

        case 2:
            devp2pinfo.m_strMobilePort = strColumn;
            break;

        case 3:
            devp2pinfo.m_strChannelCount = strColumn;
            break;

        case 4:
            devp2pinfo.m_strDeviceSN = strColumn;
            break;

        case 5:
            devp2pinfo.m_strP2pID = strColumn;
            break;

        case 6:
            devp2pinfo.m_strExtend = strColumn;

            Result = devp2pinfo;
            break;
            
        default:
            LOG_ERROR_RLD("Query device p2p id sql callback error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("Query device p2p id exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("Query device p2p id result is empty.");
        return false;
    }

    devp2pinfo = boost::any_cast<DevP2pIDInfo>(ResultList.front());

    return true;
}

bool AccessManager::QueryAccessDomainOfBusiness(const std::string &strUserName, const unsigned int uiBussinessType, std::string &strAccessDomainInfo)
{
    if (1 == uiBussinessType)
    {
        char sql[1024] = { 0 };
        int size = sizeof(sql);
        const char *sqlfmt = "select description from t_configuration_info where category = 'Business' and subcategory = 'ProductAddress'";
        snprintf(sql, size, sqlfmt);

        auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
        {
            strAccessDomainInfo = strColumn;
        };

        std::list<boost::any> ResultList;
        if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
        {
            LOG_ERROR_RLD("Query access domain of business exec sql failed, sql is " << sql);
            return false;
        }

        return true;
    }

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select a.access_domain from t_access_user_business_info a , t_user_info b where a.userid = b.userid"
        " and b.username = '%s' and a.business_type = %u and a.status = 0 and b.status = 0";
    snprintf(sql, size, sqlfmt, strUserName.c_str(), uiBussinessType);

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        strAccessDomainInfo = strColumn;
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("Query access domain of business exec sql failed, sql is " << sql);
        return false;
    }

    ////这里查询不需要以下动作
    //if (ResultList.empty())
    //{
    //    LOG_ERROR_RLD("Query access domain of business result is empty.");
    //    return true;
    //}

    //strAccessDomainInfo = boost::any_cast<std::string>(ResultList.front());

    return true;
}

bool AccessManager::QueryAccessDomainOfBusinessDevice(const std::string &strDevID, const unsigned int uiBussinessType, std::string &strAccessDomainInfo)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select a.access_domain from t_access_device_business_info a where a.deviceid = '%s' and a.business_type = %u and a.status = 0";        
    snprintf(sql, size, sqlfmt, strDevID.c_str(), uiBussinessType);

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        strAccessDomainInfo = strColumn;
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("Query access domain of business exec sql failed, sql is " << sql);
        return false;
    }

    ////这里查询不需要以下动作
    //if (ResultList.empty())
    //{
    //    LOG_ERROR_RLD("Query access domain of business result is empty.");
    //    return true;
    //}

    //strAccessDomainInfo = boost::any_cast<std::string>(ResultList.front());

    return true;
}

bool AccessManager::UploadUsrCfg(const UserCfg &ucfg)
{
    char sql[2048] = { 0 };
    const char* sqlfmt = "insert into t_user_config_info(id,userid,fileid, business_type, version) values(uuid(), '%s', '%s', %u, %u)";
    snprintf(sql, sizeof(sql), sqlfmt, ucfg.m_strUserID.c_str(), ucfg.m_strFileID.c_str(), ucfg.m_uiBusinessType, boost::lexical_cast<unsigned int>(ucfg.m_strVersion));

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("UploadUsrCfg failed, sql is " << sql);
        return false;
    }

    return true;
}

bool AccessManager::GetUsrCfgMaxVersion(const std::string &strUserID, const unsigned int uiBusinessType, unsigned int &uiMaxVersion)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select version from t_user_config_info where userid = '%s' and business_type = %u and "
        "status = 0 order by version desc limit 0, 1";
    snprintf(sql, size, sqlfmt, strUserID.c_str(), uiBusinessType);

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        uiMaxVersion = boost::lexical_cast<unsigned int>(strColumn);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("Get user cfg sql failed, sql is " << sql);
        return false;
    }
    
    return true;
}

bool AccessManager::QueryUsrCfg(const std::string &strUserID, const unsigned int uiBusinessType, UserCfg &ucfg)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select version, fileid, extend from t_user_config_info where userid = '%s' and business_type = %u and "
        "status = 0 order by version desc limit 0, 1";
    snprintf(sql, size, sqlfmt, strUserID.c_str(), uiBusinessType);

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {        
        switch (uiColumnNum)
        {
        case 0:
            ucfg.m_strVersion = strColumn;
            break;
        case 1:
            ucfg.m_strFileID = strColumn;
            break;        
        case 2:
            ucfg.m_strExtend = strColumn;
            break;
        default:
            LOG_ERROR_RLD("Query user cfg callback error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("Get user cfg sql failed, sql is " << sql);
        return false;
    }

    if (!ucfg.m_strFileID.empty())
    {
        ucfg.m_strCfgURL = m_ParamInfo.m_strFileServerURL + "download_file&fileid=" + ucfg.m_strFileID;
    }

    return true;
}

bool AccessManager::GetBlackIDNum(const std::string &strBlackID, unsigned int &uiNum)
{
    bool blResult = false;

    char sql[1024] = { 0 };
    const char *sqlfmt = "SELECT COUNT(id) FROM t_user_device_blacklist WHERE black_id = '%s' AND STATUS = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strBlackID.c_str());

    unsigned int uiIDNum;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        uiIDNum = boost::lexical_cast<unsigned int>(strColumn);
        Result = uiIDNum;

        LOG_INFO_RLD("Query black id exist sql count(id) is " << uiIDNum);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("Query black id exist sql failed, sql is " << sql);
        return blResult;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("Query black id return empty, black id is " << strBlackID);
        return blResult;
    }

    uiIDNum = boost::any_cast<unsigned int>(ResultList.front());

    uiNum = uiIDNum;

    blResult = true;
    return blResult;
}

void AccessManager::AddBlackID(const std::string &strBlackID, const std::string &strExtend, unsigned int uiIDType)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_user_device_blacklist (id, black_id, id_type, extend)"
        " values(uuid(), '%s', '%d', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strBlackID.c_str(), uiIDType, strExtend.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Add black id exec sql failed, sql is " << sql);
    }
}

void AccessManager::RemoveBlackID(const std::string &strBlackID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_user_device_blacklist where black_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strBlackID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Remove black id exec sql failed, sql is " << sql);
    }
}

bool AccessManager::QueryAllBlackInfo(unsigned int uiIDType, unsigned int uiBeginIndex, std::list<InteractiveProtoHandler::BlackObj> &blkobjlist)
{
    bool blResult = false;
    
    char sql[2048] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "SELECT black_id, id_type, extend FROM t_user_device_blacklist WHERE STATUS = 0 ";
    int len = snprintf(sql, sizeof(sql), sqlfmt);

    if (0xFFFFFFFF != uiIDType)
    {
        len += snprintf(sql + len, size - len, " AND id_type = %d", uiIDType);
    }

    snprintf(sql + len, size - len, " order by createdate desc limit %d, %d", uiBeginIndex, 100);

    InteractiveProtoHandler::BlackObj bo;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            bo.m_strBlackID = strColumn;
            break;
        case 1:
            bo.m_uiIDType = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 2:
            bo.m_strExtend = strColumn;
            blkobjlist.push_back(bo);
            break;

        default:
            LOG_ERROR_RLD("Query all black id sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("Query all black id exec sql failed, sql is " << sql);
        return blResult;
    }

    blResult = true;
    return blResult;
}
