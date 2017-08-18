#include "InterProcessHandler.h"
#include <map>
#include "PassengerFlowManager.h"
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


PassengerFlowManager::PassengerFlowManager(const ParamInfo &pinfo) : m_ParamInfo(pinfo), m_DBRuner(1), m_pProtoHandler(new InteractiveProtoHandler),
m_pMysql(new MysqlImpl), m_DBCache(m_pMysql), m_uiMsgSeq(0), 
m_DBTimer(NULL, 600), 
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

PassengerFlowManager::~PassengerFlowManager()
{
    m_DBTimer.Stop();

    m_SessionMgr.Stop();

    m_DBRuner.Stop();
        
    delete m_pMysql;
    m_pMysql = NULL;

}

bool PassengerFlowManager::Init()
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


        if (m_ParamInfo.m_strMasterNode == "Yes")
        {
           
        }
    };

    m_DBTimer.SetTimeOutCallBack(TmFunc);
    
    m_DBTimer.Run(true);

    m_DBCache.SetSqlCB(boost::bind(&PassengerFlowManager::UserInfoSqlCB, this, _1, _2, _3, _4));
    
    m_DBRuner.Run();

    m_SessionMgr.Run();



    m_MsgReceiver->RunReceivedMsg();
    m_MsgSender->RunSendMsg();

    LOG_INFO_RLD("UserManager init success");

    return true;
}

bool PassengerFlowManager::GetMsgType(const std::string &strMsg, int &iMsgType)
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

bool PassengerFlowManager::PreCommonHandler(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

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
        InteractiveProtoHandler::MsgType::QueryPlatformPushStatusReq_DEV_T == req.m_MsgType)
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
