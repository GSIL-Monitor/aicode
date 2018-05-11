#include <map>
#include "PassengerFlowManager.h"
#include <boost/scope_exit.hpp>
#include <boost/thread/thread.hpp>
#include <boost/algorithm/string.hpp>
#include "CommonUtility.h"
#include "ReturnCode.h"
#include "mysql_impl.h"
#include "boost/lexical_cast.hpp"
#include "json/json.h"
#include "HttpClient.h"

std::string PassengerFlowManager::ALLOW_ACCESS = "allow";
std::string PassengerFlowManager::DISALLOW_ACCESS = "disallow";

PassengerFlowManager::PassengerFlowManager(const ParamInfo &pinfo) :
    m_ParamInfo(pinfo),
    m_DBRuner(1),
    m_pProtoHandler(new PassengerFlowProtoHandler),
    m_pMysql(new MysqlImpl),
    m_DBCache(m_pMysql),
    m_uiMsgSeq(0),
    m_DBTimer(NULL, 600),
    m_pushGetuiIOS(pinfo.m_strPushServerUrl, pinfo.m_strIOSAppID, pinfo.m_strIOSAppKey, pinfo.m_strIOSMasterSecret, pinfo.m_iAuthExpire),
    m_pushGetuiAndroid(pinfo.m_strPushServerUrl, pinfo.m_strAndroidAppID, pinfo.m_strAndroidAppKey, pinfo.m_strAndroidMasterSecret, pinfo.m_iAuthExpire)
{
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
    if (!m_pMysql->Init(m_ParamInfo.m_strDBHost.c_str(), m_ParamInfo.m_strDBUser.c_str(), m_ParamInfo.m_strDBPassword.c_str(), m_ParamInfo.m_strDBName.c_str()))
    {
        LOG_ERROR_RLD("Init db failed, db host is " << m_ParamInfo.m_strDBHost
            << " db user is " << m_ParamInfo.m_strDBUser
            << " db pwd is " << m_ParamInfo.m_strDBPassword
            << " db name is " << m_ParamInfo.m_strDBName);
        return false;
    }

    m_SessionMgr.SetMemCacheAddRess(m_ParamInfo.m_strMemAddress, m_ParamInfo.m_strMemPort);
    m_SessionMgr.SetGlobalMemCacheAddRess(m_ParamInfo.m_strMemAddressGlobal, m_ParamInfo.m_strMemPortGlobal);
    //m_SessionMgr.SetSessionTimeoutCB(boost::bind(&ClusterAccessCollector::AddAccessTimeoutRecord, m_pClusterAccessCollector, _1, _2));

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
    };

    m_DBTimer.SetTimeOutCallBack(TmFunc);

    m_DBTimer.Run(true);

    m_DBRuner.Run();

    m_SessionMgr.Run();

    m_pushGetuiIOS.Init();

    m_pushGetuiAndroid.Init();

    LOG_INFO_RLD("PassengerFlowManager init success");

    return true;
}

bool PassengerFlowManager::GetMsgType(const std::string &strMsg, int &iMsgType)
{
    PassengerFlowProtoHandler::CustomerFlowMsgType mtype;
    if (!m_pProtoHandler->GetCustomerFlowMsgType(strMsg, mtype))
    {
        LOG_ERROR_RLD("Get msg type failed.");
        return false;
    }

    iMsgType = mtype;
    return true;
}

bool PassengerFlowManager::PreCommonHandler(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::SESSION_TIMEOUT);

    bool blResult = false;

    PassengerFlowProtoHandler::Request req;

    BOOST_SCOPE_EXIT(&blResult, this_, &req, &writer, &strSrcID)
    {
        if (blResult)
        {
            LOG_INFO_RLD("PreCommonHandler success, dst id is " << strSrcID
                << " and session id is " << req.m_strSID);
            return;
        }

        PassengerFlowProtoHandler::CustomerFlowPreHandleRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::CustomerFlowPreHandleRsp_T;
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
        LOG_ERROR_RLD("PreCommonHandler failed and rsp already send, dst id is " << strSrcID
            << " and session id is " << req.m_strSID
            << " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReqBase(strMsg, req))
        {
            LOG_ERROR_RLD("PreCommonHandler req unserialize failed, src id is " << strSrcID);
            return false;
        }

    std::string strID;
    std::string strPermission;
    if (!req.m_strSID.empty() && !m_SessionMgr.GetIDBySessionID(req.m_strSID, strID))
    {
        LOG_ERROR_RLD("PreCommonHandler failed, get id error, src id is " << strSrcID);
        return false;
    }

    if (!strID.empty() && !UserAccessPermission(strID, req.m_MsgType, strPermission))
    {
        LOG_ERROR_RLD("PreCommonHandler failed, query user permission error, src id is " << strSrcID);
        return false;
    }

    if (strPermission == DISALLOW_ACCESS)
    {
        ReturnInfo::RetCode(ReturnInfo::NO_ACCESS_PERMISSION);
        LOG_ERROR_RLD("PreCommonHandler failed, the operation is disallowed, src id is " << strSrcID);
        return false;
    }

    //if (PassengerFlowProtoHandler::CustomerFlowMsgType::LoginReq_USR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::RegisterUserReq_USR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::RegisterUserRsp_USR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::LoginReq_DEV_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::AddFileReq_DEV_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::RetrievePwdReq_USR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAccessDomainNameReq_DEV_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAccessDomainNameReq_USR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAppUpgradeReq_USR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::QueryFirmwareUpgradeReq_DEV_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::AddConfigurationReq_MGR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteConfigurationReq_MGR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyConfigurationReq_MGR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllConfigurationReq_MGR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::QueryUploadURLReq_MGR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::P2pInfoReq_DEV_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::QueryIfP2pIDValidReq_USR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::QueryPlatformPushStateReq_DEV_T == req.m_MsgType)
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
            LOG_ERROR_RLD("PreCommonHandler failed, get session state error, msg type is " << req.m_MsgType
                << " and seesion id is " << req.m_strSID);
            blResult = false;
            return blResult;
        }

        if (SessionMgr::LOGIN_MUTEX_ERROR == iErrorCode)
        {
            LOG_ERROR_RLD("PreCommonHandler failed, the account is logged in at other terminal, msg type is " << req.m_MsgType
                << " and seesion id is " << req.m_strSID);
            ReturnInfo::RetCode(ReturnInfo::ACCOUNT_LOGIN_AT_OTHER_TERMINAL);
            blResult = false;
            return blResult;
        }
    }

    //普通命令（非握手命令）重置Session
    if (PassengerFlowProtoHandler::CustomerFlowMsgType::CustomerFlowPreHandleReq_T != req.m_MsgType)
    {
        m_SessionMgr.Reset(req.m_strSID);
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::AddAreaReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::string strAreaID;
    PassengerFlowProtoHandler::AddAreaReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &strAreaID)
    {
        PassengerFlowProtoHandler::AddAreaRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddAreaRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strAreaID = blResult ? strAreaID : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add area rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Add area rsp already send, dst id is " << strSrcID
            << " and area id is " << strAreaID
            << " and area name is " << req.m_areaInfo.m_strAreaID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Add area req unserialize failed, src id is " << strSrcID);
            return false;
        }

    std::string strCompanyID;
    if (!QueryUserCompany(req.m_strUserID, strCompanyID) || strCompanyID.empty())
    {
        LOG_ERROR_RLD("Add area failed, query user company error, user id is " << req.m_strUserID);
        return false;
    }

    if (!IsValidArea(req.m_strUserID, req.m_areaInfo.m_strAreaName))
    {
        LOG_ERROR_RLD("Add area failed, the area name is not available, name is " << req.m_areaInfo.m_strAreaName);
        return false;
    }

    PassengerFlowProtoHandler::Area areaInfo;
    areaInfo.m_strAreaID = strAreaID = CreateUUID();
    areaInfo.m_strAreaName = req.m_areaInfo.m_strAreaName;
    areaInfo.m_uiLevel = req.m_areaInfo.m_uiLevel;
    areaInfo.m_strParentAreaID = req.m_areaInfo.m_strParentAreaID;
    areaInfo.m_strCreateDate = CurrentTime();
    areaInfo.m_strExtend = req.m_areaInfo.m_strExtend;

    //这里是异步执行sql，防止阻塞，后续可以使用其他方式比如MQ来消除数据库瓶颈
    m_DBRuner.Post(boost::bind(&PassengerFlowManager::AddArea, this, req.m_strUserID, strCompanyID, areaInfo));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::DeleteAreaReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::DeleteAreaReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::DeleteAreaRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteAreaRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete area rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Delete area rsp already send, dst id is " << strSrcID
            << " and area id is " << req.m_strAreaID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Delete area req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::DeleteArea, this, req.m_strAreaID));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::ModifyAreaReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::ModifyAreaReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::ModifyAreaRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyAreaRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify area rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Modify area rsp already send, dst id is " << strSrcID
            << " and area id is " << req.m_areaInfo.m_strAreaID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Modify area req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::ModifyArea, this, req.m_areaInfo));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryAreaInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::Area area;
    PassengerFlowProtoHandler::QueryAreaInfoReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &area, &req)
    {
        PassengerFlowProtoHandler::QueryAreaInfoRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAreaInfoRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_areaInfo.m_strAreaID = area.m_strAreaID;
            rsp.m_areaInfo.m_strAreaName = area.m_strAreaName;
            rsp.m_areaInfo.m_uiLevel = area.m_uiLevel;
            rsp.m_areaInfo.m_strParentAreaID = area.m_strParentAreaID;
            rsp.m_areaInfo.m_strCreateDate = area.m_strCreateDate;
            rsp.m_areaInfo.m_strExtend = area.m_strExtend;
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query area info rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query area info rsp already send, dst id is " << strSrcID
            << " and area id is " << req.m_strAreaID
            << " and area name is " << area.m_strAreaName
            << " and level is " << area.m_uiLevel
            << " and parent id is " << area.m_strParentAreaID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query area info req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!PassengerFlowManager::QueryAreaInfo(req.m_strAreaID, area))
    {
        LOG_ERROR_RLD("Query area info failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryAllAreaReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::QueryAllAreaReq req;
    std::list<PassengerFlowProtoHandler::Area> areaList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &areaList)
    {
        PassengerFlowProtoHandler::QueryAllAreaRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllAreaRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_areaList.swap(areaList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all area rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query all area rsp already send, dst id is " << strSrcID
            << " and result is " << blResult);

        if (blResult)
        {
            int i = 0;
            for (auto &area : rsp.m_areaList)
            {
                LOG_INFO_RLD("Area info[" << i++ << "]:"
                    << " area id is " << area.m_strAreaID
                    << " and area name is " << area.m_strAreaName
                    << " and level is " << area.m_uiLevel
                    << " and parent area id is " << area.m_strParentAreaID);
            }
        }
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query all area req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryAllArea(req.m_strUserID, areaList))
    {
        LOG_ERROR_RLD("Query all area failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::BindPushClientIDReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::BindPushClientIDReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::BindPushClientIDRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::BindPushClientIDRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Bind push client ID rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Bind push client ID rsp already send, dst id is " << strSrcID
            << " and user id is " << req.m_strUserID
            << " and client id is " << req.m_strClientID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Bind push client ID req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::BindPushClientID, this, req.m_strUserID, req.m_strClientID));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::UnbindPushClientIDReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::UnbindPushClientIDReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::UnbindPushClientIDRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::UnbindPushClientIDRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Unbind push client ID rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Unbind push client ID rsp already send, dst id is " << strSrcID
            << " and user id is " << req.m_strUserID
            << " and client id is " << req.m_strClientID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Unbind push client ID req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::UnbindPushClientID, this, req.m_strUserID, req.m_strClientID));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::AddStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::string strStoreID;
    PassengerFlowProtoHandler::AddStoreReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &strStoreID)
    {
        PassengerFlowProtoHandler::AddStoreRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddStoreRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strStoreID = blResult ? strStoreID : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add store rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Add store rsp already send, dst id is " << strSrcID
            << " and store id is " << strStoreID
            << " and goods category is " << req.m_storeInfo.m_strGoodsCategory
            << " and address is " << req.m_storeInfo.m_strAddress
            << " and area id is " << req.m_storeInfo.m_area.m_strAreaID
            << " and open state is " << req.m_storeInfo.m_uiOpenState
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Add store req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!IsValidStore(req.m_strUserID, req.m_storeInfo.m_strStoreName))
    {
        LOG_ERROR_RLD("Add store failed, the store name is not available, name is " << req.m_storeInfo.m_strStoreName);
        return false;
    }

    PassengerFlowProtoHandler::Store storeInfo;
    storeInfo.m_strStoreID = strStoreID = CreateUUID();
    storeInfo.m_strStoreName = req.m_storeInfo.m_strStoreName;
    storeInfo.m_strGoodsCategory = req.m_storeInfo.m_strGoodsCategory;
    storeInfo.m_strAddress = req.m_storeInfo.m_strAddress;
    storeInfo.m_area.m_strAreaID = req.m_storeInfo.m_area.m_strAreaID;
    storeInfo.m_uiOpenState = req.m_storeInfo.m_uiOpenState;
    storeInfo.m_entranceList = req.m_storeInfo.m_entranceList;
    storeInfo.m_strTelephoneList = req.m_storeInfo.m_strTelephoneList;
    storeInfo.m_strCreateDate = CurrentTime();

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::AddStore, this, req.m_strUserID, storeInfo));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::DeleteStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::DeleteStoreReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::DeleteStoreRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteStoreRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete store rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Delete store rsp already send, dst id is " << strSrcID
            << " and store id is " << req.m_strStoreID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Delete store req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::DeleteStore, this, req.m_strStoreID));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::ModifyStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::ModifyStoreReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::ModifyStoreRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyStoreRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify store rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Modify store rsp already send, dst id is " << strSrcID
            << " and store id is " << req.m_storeInfo.m_strStoreID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Modify store req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::ModifyStore, this, req.m_storeInfo));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryStoreInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::QueryStoreInfoReq req;
    PassengerFlowProtoHandler::Store storeInfo;
    std::list<PassengerFlowProtoHandler::Area> areaList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &storeInfo, &areaList)
    {
        PassengerFlowProtoHandler::QueryStoreInfoRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryStoreInfoRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_storeInfo.m_strStoreID = storeInfo.m_strStoreID;
            rsp.m_storeInfo.m_strStoreName = storeInfo.m_strStoreName;
            rsp.m_storeInfo.m_strGoodsCategory = storeInfo.m_strGoodsCategory;
            rsp.m_storeInfo.m_strAddress = storeInfo.m_strAddress;
            rsp.m_storeInfo.m_area.m_strAreaID = storeInfo.m_area.m_strAreaID;
            rsp.m_storeInfo.m_area.m_strAreaName = storeInfo.m_area.m_strAreaName;
            rsp.m_storeInfo.m_uiOpenState = storeInfo.m_uiOpenState;
            rsp.m_storeInfo.m_strCreateDate = storeInfo.m_strCreateDate;
            rsp.m_storeInfo.m_entranceList.swap(storeInfo.m_entranceList);
            rsp.m_storeInfo.m_strTelephoneList.swap(storeInfo.m_strTelephoneList);

            rsp.m_areaList.swap(areaList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query store info rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query store info rsp already send, dst id is " << strSrcID
            << " and store id is " << storeInfo.m_strStoreID
            << " and store name is " << storeInfo.m_strStoreName
            << " and goods category is " << storeInfo.m_strGoodsCategory
            << " and address is " << storeInfo.m_strAddress
            << " and area id is " << storeInfo.m_area.m_strAreaID
            << " and open state is " << storeInfo.m_uiOpenState
            << " and create date is " << storeInfo.m_strCreateDate
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query store info req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryStoreInfo(req.m_strStoreID, storeInfo))
    {
        LOG_ERROR_RLD("Query store info failed, query store info error, src id is " << strSrcID);
        return false;
    }

    if (!QueryAreaParent(storeInfo.m_area.m_strAreaID, areaList))
    {
        LOG_ERROR_RLD("Query store info failed, query area parent error, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryAllStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::QueryAllStoreReq req;
    std::list<PassengerFlowProtoHandler::Store> storeList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &storeList)
    {
        PassengerFlowProtoHandler::QueryAllStoreRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllStoreRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_storeList.swap(storeList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all store rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query all store rsp already send, dst id is " << strSrcID
            << " and result is " << blResult);

        if (blResult)
        {
            int i = 0;
            for (auto &store : rsp.m_storeList)
            {
                LOG_INFO_RLD("Store info[" << i++ << "]:"
                    << " store id is " << store.m_strStoreID
                    << " and store name is " << store.m_strStoreName
                    << " and address is " << store.m_strAddress
                    << " and open state is " << store.m_uiOpenState);
            }
        }
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query all store req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryAllStore(req.m_strUserID, req.m_strAreaID, req.m_uiOpenState, storeList, req.m_uiBeginIndex))
    {
        LOG_ERROR_RLD("Query all store failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::AddEntranceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::string strEntranceID;
    PassengerFlowProtoHandler::AddEntranceReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &strEntranceID)
    {
        PassengerFlowProtoHandler::AddEntranceRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddEntranceRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strEntranceID = blResult ? strEntranceID : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add entrance rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Add entrance rsp already send, dst id is " << strSrcID
            << " and entrance id is " << strEntranceID
            << " and entrance name is " << req.m_entranceInfo.m_strEntranceName
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Add entrance req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!IsValidEntrance(req.m_strStoreID, req.m_entranceInfo.m_strEntranceName))
    {
        LOG_ERROR_RLD("Add entrance failed, the entrance name is not available, name is " << req.m_entranceInfo.m_strEntranceName);
        return false;
    }

    for (auto &device : req.m_entranceInfo.m_strDeviceIDList)
    {
        std::string strEntranceID;
        if (!QueryDeviceBoundEntrance(device, strEntranceID))
        {
            LOG_ERROR_RLD("Add entrance failed, query device bound entrance error, src id is " << strSrcID
                << " and device id is " << device);
            return false;
        }

        if (!strEntranceID.empty())
        {
            LOG_ERROR_RLD("Add entrance failed, the device is bound, src id is " << strSrcID
                << " and device id is " << device);
            return false;
        }
    }

    PassengerFlowProtoHandler::Entrance entranceInfo;
    entranceInfo.m_strEntranceID = strEntranceID = CreateUUID();
    entranceInfo.m_strEntranceName = req.m_entranceInfo.m_strEntranceName;
    entranceInfo.m_strPicture = req.m_entranceInfo.m_strPicture;
    entranceInfo.m_strDeviceIDList = req.m_entranceInfo.m_strDeviceIDList;

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::AddEntrance, this, req.m_strStoreID, entranceInfo));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::DeleteEntranceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::DeleteEntranceReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::DeleteEntranceRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteEntranceRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete entrance rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Delete entrance rsp already send, dst id is " << strSrcID
            << " and entrance id is " << req.m_strEntranceID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Delete entrance req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::DeleteEntrance, this, req.m_strEntranceID));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::ModifyEntranceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::ModifyEntranceReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::ModifyEntranceRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyEntranceRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify entrance rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Modify entrance rsp already send, dst id is " << strSrcID
            << " and entrance id is " << req.m_entranceInfo.m_strEntranceID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Modify entrance req unserialize failed, src id is " << strSrcID);
            return false;
        }

    for (auto &device : req.m_strAddedDeviceIDList)
    {
        std::string strEntranceID;
        if (!QueryDeviceBoundEntrance(device, strEntranceID))
        {
            LOG_ERROR_RLD("Modify entrance failed, query device bound entrance error, src id is " << strSrcID
                << " and device id is " << device);
            return false;
        }

        if (!strEntranceID.empty())
        {
            LOG_ERROR_RLD("Modify entrance failed, the device is bound, src id is " << strSrcID
                << " and device id is " << device);
            return false;
        }
    }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::ModifyEntrance, this, req.m_entranceInfo,
        req.m_strAddedDeviceIDList, req.m_strDeletedDeviceIDList));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::AddEntranceDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::AddEntranceDeviceReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::AddEntranceDeviceRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddEntranceDeviceRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add entrance device rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Add entrance device rsp already send, dst id is " << strSrcID
            << " and entrance id is " << req.m_strEntranceID
            << " and device id is " << req.m_strDeviceID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Add entrance device req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::AddEntranceDevice, this, req.m_strEntranceID, req.m_strDeviceID));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::DeleteEntranceDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::DeleteEntranceDeviceReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::DeleteEntranceDeviceRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteEntranceDeviceRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete entrance device rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Delete entrance device rsp already send, dst id is " << strSrcID
            << " and entrance id is " << req.m_strEntranceID
            << " and device id is " << req.m_strDeviceID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Delete entrance device req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::DeleteEntranceDevice, this, req.m_strEntranceID, req.m_strDeviceID));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::AddEventReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::string strEventID;
    PassengerFlowProtoHandler::AddEventReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &strEventID)
    {
        PassengerFlowProtoHandler::AddEventRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddEventRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strEventID = blResult ? strEventID : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add event rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Add event rsp already send, dst id is " << strSrcID
            << " and event id is " << strEventID
            << " and source is " << req.m_eventInfo.m_strSource
            << " and submit date is " << req.m_eventInfo.m_strSubmitDate
            << " and expire date is " << req.m_eventInfo.m_strExpireDate
            << " and user id is " << req.m_eventInfo.m_strUserID
            << " and device id is " << req.m_eventInfo.m_strDeviceID
            << " and process state is " << req.m_eventInfo.m_strProcessState
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Add event req unserialize failed, src id is " << strSrcID);
            return false;
        }

    PassengerFlowProtoHandler::Event eventInfo;
    eventInfo.m_strEventID = strEventID = CreateUUID();
    eventInfo.m_strSource = req.m_eventInfo.m_strSource;
    eventInfo.m_strSubmitDate = req.m_eventInfo.m_strSubmitDate;
    eventInfo.m_strExpireDate = req.m_eventInfo.m_strExpireDate;
    eventInfo.m_strUserID = req.m_eventInfo.m_strUserID;
    eventInfo.m_strDeviceID = req.m_eventInfo.m_strDeviceID;
    eventInfo.m_strProcessState = "0";
    eventInfo.m_strRemark = req.m_eventInfo.m_strRemark;
    eventInfo.m_uiViewState = UNREAD_STATE;
    eventInfo.m_uiTypeList = req.m_eventInfo.m_uiTypeList;
    eventInfo.m_strHandlerList = req.m_eventInfo.m_strHandlerList;
    eventInfo.m_strCreateDate = CurrentTime();

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::AddEvent, this, eventInfo));

    std::string strStoreID;
    std::string strStoreName;
    std::string strContent;
    const unsigned int eventType = req.m_eventInfo.m_uiTypeList.front();
    if (!QueryStoreByEvent(req.m_eventInfo.m_strSource, eventType, strStoreName, strStoreID))
    {
        LOG_ERROR_RLD("QueryStoreByEvent failed and event id is " << eventInfo.m_strEventID);

        blResult = true;
        return blResult;
    }

    if (eventType == EVENT_REMOTE_PATROL)
    {
        strContent = m_ParamInfo.m_strMessageContentRemotePatrol;
    }
    else if (eventType == EVENT_REGULAR_PATROL)
    {
        strContent = m_ParamInfo.m_strMessageContentRegularPatrol;
    }
    else if (eventType == EVENT_STORE_EVALUATION)
    {
        strContent = m_ParamInfo.m_strMessageContentEvaluation;
    }
    else
    {
        LOG_ERROR_RLD("Add event error, push message error, event type is " << eventType
            << " and src id is " << strSrcID);

        blResult = true;
        return blResult;
    }

    Json::Value jsPayloadInfo;
    jsPayloadInfo["event_id"] = strEventID;
    Json::FastWriter fastwriter;
    const std::string &strPayloadInfo = fastwriter.write(jsPayloadInfo);

    std::string var("$store");
    strContent.replace(strContent.find(var), var.size(), strStoreName);
    for (auto user : req.m_eventInfo.m_strHandlerList)
    {
        m_DBRuner.Post(boost::bind(&PassengerFlowManager::PushMessage, this, m_ParamInfo.m_strMessageTitle, strContent, strPayloadInfo, user));
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::DeleteEventReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::DeleteEventReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::DeleteEventRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteEventRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete event rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Delete event rsp already send, dst id is " << strSrcID
            << " and event id is " << req.m_strEventID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Delete event req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::DeleteEvent, this, req.m_strEventID, req.m_strUserID));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::ModifyEventReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::ModifyEventReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::ModifyEventRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyEventRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify event rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Modify event rsp already send, dst id is " << strSrcID
            << " and event id is " << req.m_eventInfo.m_strEventID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Modify event req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::ModifyEvent, this, req.m_eventInfo));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryEventInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::QueryEventInfoReq req;
    PassengerFlowProtoHandler::Event eventInfo;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &eventInfo)
    {
        PassengerFlowProtoHandler::QueryEventInfoRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryEventInfoRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_eventInfo.m_strEventID = eventInfo.m_strEventID;
            rsp.m_eventInfo.m_strSource = eventInfo.m_strSource;
            rsp.m_eventInfo.m_strSubmitDate = eventInfo.m_strSubmitDate;
            rsp.m_eventInfo.m_strExpireDate = eventInfo.m_strExpireDate;
            rsp.m_eventInfo.m_strUserID = eventInfo.m_strUserID;
            rsp.m_eventInfo.m_strDeviceID = eventInfo.m_strDeviceID;
            rsp.m_eventInfo.m_strProcessState = eventInfo.m_strProcessState;
            rsp.m_eventInfo.m_strRemark = eventInfo.m_strRemark;
            rsp.m_eventInfo.m_uiViewState = eventInfo.m_uiViewState;
            rsp.m_eventInfo.m_uiTypeList.swap(eventInfo.m_uiTypeList);
            rsp.m_eventInfo.m_strHandlerList.swap(eventInfo.m_strHandlerList);
            rsp.m_eventInfo.m_strCreateDate = eventInfo.m_strCreateDate;

            rsp.m_eventInfo.m_strStoreID = eventInfo.m_strStoreID;
            rsp.m_eventInfo.m_strStoreName = eventInfo.m_strStoreName;
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query event info rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query event info rsp already send, dst id is " << strSrcID
            << " and event id is " << eventInfo.m_strEventID
            << " and source is " << eventInfo.m_strSource
            << " and submit date is " << eventInfo.m_strSubmitDate
            << " and expire date is " << eventInfo.m_strExpireDate
            << " and user id is " << eventInfo.m_strUserID
            << " and device id is " << eventInfo.m_strDeviceID
            << " and process state is " << eventInfo.m_strProcessState
            << " and view state is " << eventInfo.m_uiViewState
            << " and result is " << blResult
            << " and store name is " << eventInfo.m_strStoreName
            << " and store id is " << eventInfo.m_strStoreID);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query event info req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryEventInfo(req.m_strEventID, eventInfo))
    {
        LOG_ERROR_RLD("Query event info failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryAllEventReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::QueryAllEventReq req;
    std::list<PassengerFlowProtoHandler::Event> eventList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &eventList)
    {
        PassengerFlowProtoHandler::QueryAllEventRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllEventRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_eventList.swap(eventList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all event rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query all event rsp already send, dst id is " << strSrcID
            << " and result is " << blResult);

        if (blResult)
        {
            int i = 0;
            for (auto &event : rsp.m_eventList)
            {
                LOG_INFO_RLD("Event info[" << i++ << "]:"
                    << " event id is " << event.m_strEventID
                    << " and source is " << event.m_strSource
                    << " and submit date is " << event.m_strSubmitDate
                    << " and expire date is " << event.m_strExpireDate
                    << " and user id is " << event.m_strUserID
                    << " and device id is " << event.m_strDeviceID
                    << " and process state is " << event.m_strProcessState
                    << " and view state is " << event.m_uiViewState
                    << " and store id is " << event.m_strStoreID
                    << " and store name is " << event.m_strStoreName);
            }
        }
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query all event req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (req.m_uiRelation == 0)
    {
        if (!QueryCreatedEvent(req.m_strUserID, boost::lexical_cast<std::string>(req.m_uiProcessState), eventList,
            req.m_strBeginDate, req.m_strEndDate, req.m_uiBeginIndex))
        {
            LOG_ERROR_RLD("Query all event failed, src id is " << strSrcID);
            return false;
        }
    }
    else if (req.m_uiRelation == 1) {
        if (!QueryHandledEvent(req.m_strUserID, boost::lexical_cast<std::string>(req.m_uiProcessState), eventList,
            req.m_strBeginDate, req.m_strEndDate, req.m_uiBeginIndex))
        {
            LOG_ERROR_RLD("Query all event failed, src id is " << strSrcID);
            return false;
        }
    }
    else
    {
        if (!QueryAllEvent(req.m_strUserID, boost::lexical_cast<std::string>(req.m_uiProcessState), eventList,
            req.m_strBeginDate, req.m_strEndDate, req.m_uiBeginIndex))
        {
            LOG_ERROR_RLD("Query all event failed, src id is " << strSrcID);
            return false;
        }
    }

    //req的eventtype巡店消息0、考评消息1
    //报文中的事件类型："远程巡店结果通知类型0、定时巡店结果通知类型1、考评结果通知类型2。
    if (0xFFFFFFFF != req.m_uiEventType)
    {
        auto itBegin = eventList.begin();
        auto itEnd = eventList.end();

        while (itBegin != itEnd)
        {
            if (1 == req.m_uiEventType)
            {
                if (itBegin->m_uiTypeList.front() != 2) //删除巡店，剩下考评
                {
                    eventList.erase(itBegin++);
                    continue;
                }
            }
            else
            {
                if (itBegin->m_uiTypeList.front() == 2) //删除考评，剩下巡店
                {
                    eventList.erase(itBegin++);
                    continue;
                }
            }
                        
            ++itBegin;
        }
    }
    

    for (auto &evt : eventList)
    {
        if (!QueryStoreByEvent(evt.m_strSource, evt.m_uiTypeList.front(), evt.m_strStoreName, evt.m_strStoreID))
        {
            LOG_ERROR_RLD("QueryStoreByEvent failed, query store id and name error, event id is" << evt.m_strEventID);
            //return false;
        }
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::AddSmartGuardStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::string strPlanID;
    PassengerFlowProtoHandler::AddSmartGuardStoreReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &strPlanID)
    {
        PassengerFlowProtoHandler::AddSmartGuardStoreRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddSmartGuardStoreRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strPlanID = blResult ? strPlanID : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add smart guard store rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Add smart guard store rsp already send, dst id is " << strSrcID
            << " and plan id is " << strPlanID
            << " and plan name is " << req.m_smartGuardStore.m_strPlanName
            << " and enable is " << req.m_smartGuardStore.m_strEnable
            << " and begin time is " << req.m_smartGuardStore.m_strBeginTime
            << " and end time is " << req.m_smartGuardStore.m_strEndTime
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Add smart guard store req unserialize failed, src id is " << strSrcID);
            return false;
        }

    PassengerFlowProtoHandler::SmartGuardStore smartGuardStore;
    smartGuardStore.m_strPlanID = strPlanID = CreateUUID();
    smartGuardStore.m_strPlanName = req.m_smartGuardStore.m_strPlanName;
    smartGuardStore.m_strEnable = req.m_smartGuardStore.m_strEnable;
    smartGuardStore.m_strBeginTime = req.m_smartGuardStore.m_strBeginTime;
    smartGuardStore.m_strEndTime = req.m_smartGuardStore.m_strEndTime;
    smartGuardStore.m_strBeginTime2 = req.m_smartGuardStore.m_strBeginTime2;
    smartGuardStore.m_strEndTime2 = req.m_smartGuardStore.m_strEndTime2;
    smartGuardStore.m_strEntranceIDList = req.m_smartGuardStore.m_strEntranceIDList;
    smartGuardStore.m_strCreateDate = CurrentTime();

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::AddSmartGuardStore, this, req.m_strUserID,
        req.m_smartGuardStore.m_strStoreID, smartGuardStore));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::DeleteSmartGuardStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::DeleteSmartGuardStoreReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::DeleteSmartGuardStoreRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteSmartGuardStoreRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete smart guard store rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Delete smart guard store rsp already send, dst id is " << strSrcID
            << " and plan id is " << req.m_strPlanID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Delete smart guard store req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::DeleteSmartGuardStore, this, req.m_strPlanID));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::ModifySmartGuardStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::ModifySmartGuardStoreReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::ModifySmartGuardStoreRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifySmartGuardStoreRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify smart guard store rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Modify smart guard store rsp already send, dst id is " << strSrcID
            << " and plan id is " << req.m_smartGuardStore.m_strPlanID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Modify smart guard store req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::ModifySmartGuardStore, this, req.m_smartGuardStore));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QuerySmartGuardStoreInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::QuerySmartGuardStoreInfoReq req;
    PassengerFlowProtoHandler::SmartGuardStore smartGuardStore;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &smartGuardStore)
    {
        PassengerFlowProtoHandler::QuerySmartGuardStoreInfoRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QuerySmartGuardStoreInfoRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_smartGuardStore.m_strStoreID = smartGuardStore.m_strStoreID;
            rsp.m_smartGuardStore.m_strStoreName = smartGuardStore.m_strStoreName;
            rsp.m_smartGuardStore.m_strPlanID = smartGuardStore.m_strPlanID;
            rsp.m_smartGuardStore.m_strPlanName = smartGuardStore.m_strPlanName;
            rsp.m_smartGuardStore.m_strEnable = smartGuardStore.m_strEnable;
            rsp.m_smartGuardStore.m_strBeginTime = smartGuardStore.m_strBeginTime;
            rsp.m_smartGuardStore.m_strEndTime = smartGuardStore.m_strEndTime;
            rsp.m_smartGuardStore.m_strBeginTime2 = smartGuardStore.m_strBeginTime2;
            rsp.m_smartGuardStore.m_strEndTime2 = smartGuardStore.m_strEndTime2;
            rsp.m_smartGuardStore.m_strEntranceIDList = smartGuardStore.m_strEntranceIDList;
            rsp.m_smartGuardStore.m_strCreateDate = smartGuardStore.m_strCreateDate;
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query smart guard store info rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query smart guard store info rsp already send, dst id is " << strSrcID
            << " and store id is " << smartGuardStore.m_strStoreID
            << " and store name is " << smartGuardStore.m_strStoreName
            << " and plan id is " << smartGuardStore.m_strPlanID
            << " and plan name is " << smartGuardStore.m_strPlanName
            << " and enable is " << smartGuardStore.m_strEnable
            << " and begin time is " << smartGuardStore.m_strBeginTime
            << " and end time is " << smartGuardStore.m_strEndTime
            << " and begin time 2 is " << smartGuardStore.m_strBeginTime2
            << " and end time 2 is " << smartGuardStore.m_strEndTime2
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query smart guard store info req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QuerySmartGuardStoreInfo(req.m_strPlanID, smartGuardStore))
    {
        LOG_ERROR_RLD("Query smart guard store info failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryAllSmartGuardStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::QueryAllSmartGuardStoreReq req;
    std::list<PassengerFlowProtoHandler::SmartGuardStore> planList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &planList)
    {
        PassengerFlowProtoHandler::QueryAllSmartGuardStoreRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllSmartGuardStoreRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_planList.swap(planList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all smart guard store rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query all smart guard store rsp already send, dst id is " << strSrcID
            << " and result is " << blResult);

        if (blResult)
        {
            int i = 0;
            for (auto &plan : rsp.m_planList)
            {
                LOG_INFO_RLD("SmartGuardStore info[" << i++ << "]:"
                    << " smart plan id is " << plan.m_strPlanID
                    << " and plan name is " << plan.m_strPlanName
                    << " and enable is " << plan.m_strEnable
                    << " and begin time is " << plan.m_strBeginTime
                    << " and end time is " << plan.m_strEndTime);
            }
        }
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query all smart guard store req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!req.m_strUserID.empty())
    {
        if (!QueryAllSmartGuardStoreByUser(req.m_strUserID, planList, req.m_uiBeginIndex))
        {
            LOG_ERROR_RLD("Query all smart guard store failed, src id is " << strSrcID);
            return false;
        }
    }
    else
    {
        if (!QueryAllSmartGuardStoreByDevice(req.m_strDeviceID, planList))
        {
            LOG_ERROR_RLD("Query all smart guard store failed, src id is " << strSrcID);
            return false;
        }
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::AddRegularPatrolReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::string strPlanID;
    PassengerFlowProtoHandler::AddRegularPatrolReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &strPlanID)
    {
        PassengerFlowProtoHandler::AddRegularPatrolRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddRegularPatrolRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strPlanID = blResult ? strPlanID : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add regular patrol rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Add regular patrol rsp already send, dst id is " << strSrcID
            << " and plan id is " << strPlanID
            << " and plan name is " << req.m_regularPatrol.m_strPlanName
            << " and enable is " << req.m_regularPatrol.m_strEnable
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Add regular patrol req unserialize failed, src id is " << strSrcID);
            return false;
        }

    PassengerFlowProtoHandler::RegularPatrol regularPatrol;
    regularPatrol.m_strPlanID = strPlanID = CreateUUID();
    regularPatrol.m_strPlanName = req.m_regularPatrol.m_strPlanName;
    regularPatrol.m_strEnable = req.m_regularPatrol.m_strEnable;
    regularPatrol.m_storeEntranceList = req.m_regularPatrol.m_storeEntranceList;
    regularPatrol.m_strPatrolTimeList = req.m_regularPatrol.m_strPatrolTimeList;
    regularPatrol.m_strHandlerList = req.m_regularPatrol.m_strHandlerList;
    regularPatrol.m_strCreateDate = CurrentTime();

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::AddRegularPatrol, this, req.m_strUserID, regularPatrol));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::DeleteRegularPatrolReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::DeleteRegularPatrolReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::DeleteRegularPatrolRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteRegularPatrolRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete regular patrol rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Delete regular patrol rsp already send, dst id is " << strSrcID
            << " and plan id is " << req.m_strPlanID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Delete regular patrol req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::DeleteRegularPatrol, this, req.m_strPlanID));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::ModifyRegularPatrolReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::ModifyRegularPatrolReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::ModifyRegularPatrolRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyRegularPatrolRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify regular patrol rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Modify regular patrol rsp already send, dst id is " << strSrcID
            << " and plan id is " << req.m_regularPatrol.m_strPlanID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Modify regular patrol req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::ModifyRegularPatrol, this, req.m_regularPatrol));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryRegularPatrolInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::QueryRegularPatrolInfoReq req;
    PassengerFlowProtoHandler::RegularPatrol regularPatrol;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &regularPatrol)
    {
        PassengerFlowProtoHandler::QueryRegularPatrolInfoRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryRegularPatrolInfoRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_regularPatrol.m_strPlanID = regularPatrol.m_strPlanID;
            rsp.m_regularPatrol.m_strPlanName = regularPatrol.m_strPlanName;
            rsp.m_regularPatrol.m_strEnable = regularPatrol.m_strEnable;
            rsp.m_regularPatrol.m_storeEntranceList = regularPatrol.m_storeEntranceList;
            rsp.m_regularPatrol.m_strPatrolTimeList = regularPatrol.m_strPatrolTimeList;
            rsp.m_regularPatrol.m_strHandlerList = regularPatrol.m_strHandlerList;
            rsp.m_regularPatrol.m_strCreateDate = regularPatrol.m_strCreateDate;
            rsp.m_regularPatrol.m_strUpdateDate = regularPatrol.m_strUpdateDate;
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query regular patrol info rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query regular patrol info rsp already send, dst id is " << strSrcID
            << " and user id is " << req.m_strUserID
            << " and plan id is " << req.m_strPlanID
            << " and plan name is " << regularPatrol.m_strPlanName
            << " and enable is " << regularPatrol.m_strEnable
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query regular patrol info req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryRegularPatrolInfo(req.m_strPlanID, regularPatrol))
    {
        LOG_ERROR_RLD("Query regular patrol info failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryAllRegularPatrolReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::QueryAllRegularPatrolReq req;
    std::list<PassengerFlowProtoHandler::RegularPatrol> planList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &planList)
    {
        PassengerFlowProtoHandler::QueryAllRegularPatrolRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllRegularPatrolRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_planList.swap(planList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all regular patrol rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query all regular patrol rsp already send, dst id is " << strSrcID
            << " and result is " << blResult);

        if (blResult)
        {
            int i = 0;
            for (auto &plan : rsp.m_planList)
            {
                LOG_INFO_RLD("RegularPatrol info[" << i++ << "]:"
                    << " regular plan id is " << plan.m_strPlanID
                    << " and plan name is " << plan.m_strPlanName
                    << " and enable is " << plan.m_strEnable);
            }
        }
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query all regular patrol req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!req.m_strUserID.empty())
    {
        if (!QueryAllRegularPatrolByUser(req.m_strUserID, planList, req.m_uiBeginIndex))
        {
            LOG_ERROR_RLD("Query all regular patrol failed, src id is " << strSrcID);
            return false;
        }
    }
    else
    {
        if (!QueryAllRegularPatrolByDevice(req.m_strDeviceID, planList))
        {
            LOG_ERROR_RLD("Query all regular patrol failed, src id is " << strSrcID);
            return false;
        }
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::UserJoinStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::UserJoinStoreReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::UserJoinStoreRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::UserJoinStoreRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("User join store rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("User join store rsp already send, dst id is " << strSrcID
            << " and user id is " << req.m_strUserID
            << " and store id is " << req.m_strStoreID
            << " and user role is " << req.m_strRole
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("User join store req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::UserJoinStore, this, req.m_strUserID, req.m_strStoreID, req.m_strRole));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::UserQuitStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::UserQuitStoreReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::UserQuitStoreRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::UserQuitStoreRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("User quit store rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("User quit store rsp already send, dst id is " << strSrcID
            << " and administrator id is " << req.m_strAdministratorID
            << " and user id is " << req.m_strUserID
            << " and store id is " << req.m_strStoreID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("User quit store req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::UserQuitStore, this, req.m_strUserID, req.m_strStoreID));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryStoreAllUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::list<PassengerFlowProtoHandler::UserBrief> userList;
    PassengerFlowProtoHandler::QueryStoreAllUserReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &userList, &req)
    {
        PassengerFlowProtoHandler::QueryStoreAllUserRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryStoreAllUserRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_userList.swap(userList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query store all user rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query store all user rsp already send, dst id is " << strSrcID
            << " and user id is " << req.m_strUserID
            << " and store id is " << req.m_strStoreID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query store all user req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryStoreAllUser(req.m_strStoreID, userList))
    {
        LOG_ERROR_RLD("Query store all user failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryCompanyAllUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::list<PassengerFlowProtoHandler::UserBrief> userList;
    PassengerFlowProtoHandler::QueryCompanyAllUserReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &userList, &req)
    {
        PassengerFlowProtoHandler::QueryCompanyAllUserRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryCompanyAllUserRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_userList.swap(userList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query company all user rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query company all user rsp already send, dst id is " << strSrcID
            << " and user id is " << req.m_strUserID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query company all user req unserialize failed, src id is " << strSrcID);
            return false;
        }

    std::string strCompanyID;
    if (!QueryUserCompany(req.m_strUserID, strCompanyID) || strCompanyID.empty())
    {
        LOG_ERROR_RLD("Query company all user failed, query user company error, src id is " << strSrcID);
        return false;
    }

    if (!QueryCompanyAllUser(strCompanyID, userList))
    {
        LOG_ERROR_RLD("Query company all user failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::AddVIPCustomerReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::string strVIPID;
    PassengerFlowProtoHandler::AddVIPCustomerReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &strVIPID)
    {
        PassengerFlowProtoHandler::AddVIPCustomerRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddVIPCustomerRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strVIPID = blResult ? strVIPID : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add vip customer rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Add vip customer rsp already send, dst id is " << strSrcID
            << " and vip id is " << strVIPID
            << " and profile picture is " << req.m_customerInfo.m_strProfilePicture
            << " and vip name is " << req.m_customerInfo.m_strVIPName
            << " and cellphone is " << req.m_customerInfo.m_strCellphone
            << " and register date is " << req.m_customerInfo.m_strRegisterDate
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Add vip customer req unserialize failed, src id is " << strSrcID);
            return false;
        }

    PassengerFlowProtoHandler::VIPCustomer customerInfo;
    customerInfo.m_strVIPID = strVIPID = CreateUUID();
    customerInfo.m_strProfilePicture = req.m_customerInfo.m_strProfilePicture;
    customerInfo.m_strVIPName = req.m_customerInfo.m_strVIPName;
    customerInfo.m_strCellphone = req.m_customerInfo.m_strCellphone;
    customerInfo.m_strVisitDate = req.m_customerInfo.m_strVisitDate;
    customerInfo.m_uiVisitTimes = req.m_customerInfo.m_uiVisitTimes;
    customerInfo.m_strRegisterDate = CurrentTime();

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::AddVIPCustomer, this, customerInfo));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::DeleteVIPCustomerReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::DeleteVIPCustomerReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::DeleteVIPCustomerRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteVIPCustomerRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete vip customer rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Delete vip customer rsp already send, dst id is " << strSrcID
            << " and vip id is " << req.m_strVIPID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Delete vip customer req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::DeleteVIPCustomer, this, req.m_strVIPID));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::ModifyVIPCustomerReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::ModifyVIPCustomerReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::ModifyVIPCustomerRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyVIPCustomerRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify vip customer rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Modify vip customer rsp already send, dst id is " << strSrcID
            << " and vip id is " << req.m_customerInfo.m_strVIPID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Modify vip customer req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::ModifyVIPCustomer, this, req.m_customerInfo));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryVIPCustomerInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::QueryVIPCustomerInfoReq req;
    PassengerFlowProtoHandler::VIPCustomer customerInfo;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &customerInfo)
    {
        PassengerFlowProtoHandler::QueryVIPCustomerInfoRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryVIPCustomerInfoRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_customerInfo.m_strVIPID = customerInfo.m_strVIPID;
            rsp.m_customerInfo.m_strProfilePicture = customerInfo.m_strProfilePicture;
            rsp.m_customerInfo.m_strVIPName = customerInfo.m_strVIPName;
            rsp.m_customerInfo.m_strCellphone = customerInfo.m_strCellphone;
            rsp.m_customerInfo.m_strVisitDate = customerInfo.m_strVisitDate;
            rsp.m_customerInfo.m_uiVisitTimes = customerInfo.m_uiVisitTimes;
            rsp.m_customerInfo.m_strRegisterDate = customerInfo.m_strRegisterDate;
            rsp.m_customerInfo.m_consumeHistoryList = customerInfo.m_consumeHistoryList;
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query vip customer info rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query vip customer info rsp already send, dst id is " << strSrcID
            << " and user id is " << req.m_strUserID
            << " and vip id is " << req.m_strVIPID
            << " and profile picture is " << customerInfo.m_strProfilePicture
            << " and vip name is " << customerInfo.m_strVIPName
            << " and cellphone is " << customerInfo.m_strCellphone
            << " and visit date is " << customerInfo.m_strVisitDate
            << " and visit times is " << customerInfo.m_uiVisitTimes
            << " and register date is " << customerInfo.m_strRegisterDate
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query vip customer info req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryVIPCustomerInfo(req.m_strVIPID, customerInfo))
    {
        LOG_ERROR_RLD("Query vip customer info failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryAllVIPCustomerReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::QueryAllVIPCustomerReq req;
    std::list<PassengerFlowProtoHandler::VIPCustomer> customerList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &customerList)
    {
        PassengerFlowProtoHandler::QueryAllVIPCustomerRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllVIPCustomerRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_customerList.swap(customerList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all vip customer rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query all vip customer rsp already send, dst id is " << strSrcID
            << " and result is " << blResult);

        if (blResult)
        {
            int i = 0;
            for (auto &customer : rsp.m_customerList)
            {
                LOG_INFO_RLD("VIPCustomer info[" << i++ << "]:"
                    << " smart vip id is " << customer.m_strVIPID
                    << " and profile picture is " << customer.m_strProfilePicture
                    << " and vip name is " << customer.m_strVIPName
                    << " and cellphone is " << customer.m_strCellphone
                    << " and visit date is " << customer.m_strVisitDate
                    << " and visit times is " << customer.m_uiVisitTimes
                    << " and register date is " << customer.m_strRegisterDate);
            }
        }
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query all vip customer req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryAllVIPCustomer(customerList, req.m_uiBeginIndex))
    {
        LOG_ERROR_RLD("Query all vip customer failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::AddVIPConsumeHistoryReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::string strConsumeID;
    PassengerFlowProtoHandler::AddVIPConsumeHistoryReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &strConsumeID)
    {
        PassengerFlowProtoHandler::AddVIPConsumeHistoryRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddVIPConsumeHistoryRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strConsumeID = blResult ? strConsumeID : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add vip consume history rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Add vip consume history rsp already send, dst id is " << strSrcID
            << " and consume id is " << strConsumeID
            << " and vip id is " << req.m_consumeHistory.m_strVIPID
            << " and goods name is " << req.m_consumeHistory.m_strGoodsName
            << " and goods number is " << req.m_consumeHistory.m_uiGoodsNumber
            << " and salesman is " << req.m_consumeHistory.m_strSalesman
            << " and consume amount is " << req.m_consumeHistory.m_dConsumeAmount
            << " and consume date is " << req.m_consumeHistory.m_strConsumeDate
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Add vip consume history req unserialize failed, src id is " << strSrcID);
            return false;
        }

    PassengerFlowProtoHandler::VIPConsumeHistory consumeHistory;
    consumeHistory.m_strConsumeID = strConsumeID = CreateUUID();
    consumeHistory.m_strVIPID = req.m_consumeHistory.m_strVIPID;
    consumeHistory.m_strGoodsName = req.m_consumeHistory.m_strGoodsName;
    consumeHistory.m_uiGoodsNumber = req.m_consumeHistory.m_uiGoodsNumber;
    consumeHistory.m_strSalesman = req.m_consumeHistory.m_strSalesman;
    consumeHistory.m_dConsumeAmount = req.m_consumeHistory.m_dConsumeAmount;
    consumeHistory.m_strConsumeDate = req.m_consumeHistory.m_strConsumeDate;

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::AddVIPConsumeHistory, this, consumeHistory));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::DeleteVIPConsumeHistoryReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::DeleteVIPConsumeHistoryReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::DeleteVIPConsumeHistoryRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteVIPConsumeHistoryRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete vip consume history rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Delete vip consume history rsp already send, dst id is " << strSrcID
            << " and consume id is " << req.m_strConsumeID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Delete vip consume history req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::DeleteVIPConsumeHistory, this, req.m_strConsumeID));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::ModifyVIPConsumeHistoryReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::ModifyVIPConsumeHistoryReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::ModifyVIPConsumeHistoryRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyVIPConsumeHistoryRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify vip consume history rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Modify vip consume history rsp already send, dst id is " << strSrcID
            << " and vip id is " << req.m_consumeHistory.m_strVIPID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Modify vip consume history req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::ModifyVIPConsumeHistory, this, req.m_consumeHistory));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryAllVIPConsumeHistoryReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::QueryAllVIPConsumeHistoryReq req;
    std::list<PassengerFlowProtoHandler::VIPConsumeHistory> consumeList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &consumeList)
    {
        PassengerFlowProtoHandler::QueryAllVIPConsumeHistoryRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllVIPConsumeHistoryRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_consumeHistoryList.swap(consumeList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all vip consume history rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query all vip consume history rsp already send, dst id is " << strSrcID
            << " and result is " << blResult);

        if (blResult)
        {
            int i = 0;
            for (auto &consume : rsp.m_consumeHistoryList)
            {
                LOG_INFO_RLD("VIPConsumeHistory info[" << i++ << "]:"
                    << " consume id is " << consume.m_strConsumeID
                    << " and goods name is " << consume.m_strGoodsName
                    << " and goods number is " << consume.m_uiGoodsNumber
                    << " and salesman is " << consume.m_strSalesman
                    << " and consume amount is " << consume.m_dConsumeAmount
                    << " and consume date is " << consume.m_strConsumeDate);
            }
        }
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query all vip consume history req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryAllVIPConsumeHistory(req.m_strVIPID, consumeList, req.m_uiBeginIndex))
    {
        LOG_ERROR_RLD("Query all vip consume history failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::AddEvaluationTemplateReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::string strEvaluationID;
    PassengerFlowProtoHandler::AddEvaluationTemplateReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &strEvaluationID)
    {
        PassengerFlowProtoHandler::AddEvaluationTemplateRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddEvaluationTemplateRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strEvaluationID = blResult ? strEvaluationID : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add evaluation template rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Add evaluation template rsp already send, dst id is " << strSrcID
            << " and evaluation id is " << strEvaluationID
            << " and evaluation item is " << req.m_evaluationItem.m_strItemName
            << " and description is " << req.m_evaluationItem.m_strDescription
            << " and total point is " << req.m_evaluationItem.m_dTotalPoint
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Add evaluation template req unserialize failed, src id is " << strSrcID);
            return false;
        }

    PassengerFlowProtoHandler::EvaluationItem evaluationItem;
    evaluationItem.m_strItemID = strEvaluationID = CreateUUID();
    evaluationItem.m_strItemName = req.m_evaluationItem.m_strItemName;
    evaluationItem.m_strDescription = req.m_evaluationItem.m_strDescription;
    evaluationItem.m_dTotalPoint = req.m_evaluationItem.m_dTotalPoint;

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::AddEvaluationTemplate, this, evaluationItem));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::DeleteEvaluationTemplateReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::DeleteEvaluationTemplateReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::DeleteEvaluationTemplateRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteEvaluationTemplateRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete evaluation template rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Delete evaluation template rsp already send, dst id is " << strSrcID
            << " and evaluation id is " << req.m_strEvaluationID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Delete evaluation template req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::DeleteEvaluationTemplate, this, req.m_strEvaluationID));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::ModifyEvaluationTemplateReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::ModifyEvaluationTemplateReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::ModifyEvaluationTemplateRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyEvaluationTemplateRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify evaluation template rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Modify evaluation template rsp already send, dst id is " << strSrcID
            << " and evaluation id is " << req.m_evaluationItem.m_strItemID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Modify evaluation template req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::ModifyEvaluationTemplate, this, req.m_evaluationItem));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryAllEvaluationTemplateReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::QueryAllEvaluationTemplateReq req;
    std::list<PassengerFlowProtoHandler::EvaluationItem> evaluationItemList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &evaluationItemList)
    {
        PassengerFlowProtoHandler::QueryAllEvaluationTemplateRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllEvaluationTemplateRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_evaluationItemList.swap(evaluationItemList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all evaluation template rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query all evaluation template rsp already send, dst id is " << strSrcID
            << " and result is " << blResult);

        if (blResult)
        {
            int i = 0;
            for (auto &item : rsp.m_evaluationItemList)
            {
                LOG_INFO_RLD("EvaluationTemplate info[" << i++ << "]:"
                    << " evaluation id is " << item.m_strItemID
                    << " and evaluation item is " << item.m_strItemName
                    << " and description is " << item.m_strDescription
                    << " and total point is " << item.m_dTotalPoint);
            }
        }
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query all evaluation template req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryAllEvaluationTemplate(evaluationItemList))
    {
        LOG_ERROR_RLD("Query all evaluation template failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::AddStoreEvaluationReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::string strEvaluationID;
    PassengerFlowProtoHandler::AddStoreEvaluationReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &strEvaluationID)
    {
        PassengerFlowProtoHandler::AddStoreEvaluationRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddStoreEvaluationRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strEvaluationID = blResult ? strEvaluationID : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add store evaluation rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Add store evaluation rsp already send, dst id is " << strSrcID
            << " and evaluation id is " << strEvaluationID
            << " and store id is " << req.m_storeEvaluation.m_strStoreID
            << " and check status is " << req.m_storeEvaluation.m_uiCheckStatus
            << " and check user id is " << req.m_storeEvaluation.m_strUserIDCheck
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Add store evaluation req unserialize failed, src id is " << strSrcID);
            return false;
        }

    PassengerFlowProtoHandler::StoreEvaluation storeEvaluation;
    storeEvaluation.m_strEvaluationID = strEvaluationID = CreateUUID();
    storeEvaluation.m_strStoreID = req.m_storeEvaluation.m_strStoreID;
    storeEvaluation.m_strUserIDCreate = req.m_storeEvaluation.m_strUserIDCreate;
    storeEvaluation.m_strUserIDCheck = req.m_storeEvaluation.m_strUserIDCheck;
    storeEvaluation.m_itemScoreList = req.m_storeEvaluation.m_itemScoreList;
    storeEvaluation.m_strPictureList = req.m_storeEvaluation.m_strPictureList;
    storeEvaluation.m_uiCheckStatus = 0;
    storeEvaluation.m_strCreateDate = CurrentTime();

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::AddStoreEvaluation, this, storeEvaluation));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::DeleteStoreEvaluationReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::DeleteStoreEvaluationReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::DeleteStoreEvaluationRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteStoreEvaluationRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete store evaluation rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Delete store evaluation rsp already send, dst id is " << strSrcID
            << " and evaluation id is " << req.m_strEvaluationID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Delete store evaluation req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::DeleteStoreEvaluation, this, req.m_strEvaluationID));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::ModifyStoreEvaluationReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::ModifyStoreEvaluationReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::ModifyStoreEvaluationRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyStoreEvaluationRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify store evaluation rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Modify store evaluation rsp already send, dst id is " << strSrcID
            << " and vip id is " << req.m_storeEvaluation.m_strEvaluationID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Modify store evaluation req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::ModifyStoreEvaluation, this, req.m_storeEvaluation));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryStoreEvaluationInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::QueryStoreEvaluationInfoReq req;
    PassengerFlowProtoHandler::StoreEvaluation storeEvaluation;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &storeEvaluation)
    {
        PassengerFlowProtoHandler::QueryStoreEvaluationInfoRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryStoreEvaluationInfoRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_storeEvaluation.m_strEvaluationID = storeEvaluation.m_strEvaluationID;
            rsp.m_storeEvaluation.m_strStoreID = storeEvaluation.m_strStoreID;
            rsp.m_storeEvaluation.m_strUserIDCreate = storeEvaluation.m_strUserIDCreate;
            rsp.m_storeEvaluation.m_strUserIDCheck = storeEvaluation.m_strUserIDCheck;
            rsp.m_storeEvaluation.m_dTotalScore = storeEvaluation.m_dTotalScore;
            rsp.m_storeEvaluation.m_uiCheckStatus = storeEvaluation.m_uiCheckStatus;
            rsp.m_storeEvaluation.m_strCreateDate = storeEvaluation.m_strCreateDate;
            rsp.m_storeEvaluation.m_itemScoreList.swap(storeEvaluation.m_itemScoreList);
            rsp.m_storeEvaluation.m_strPictureList.swap(storeEvaluation.m_strPictureList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query store evaluation info rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query store evaluation info rsp already send, dst id is " << strSrcID
            << " and evaluation id is " << storeEvaluation.m_strEvaluationID
            << " and store id is " << storeEvaluation.m_strStoreID
            << " and user id create is " << storeEvaluation.m_strUserIDCreate
            << " and user id check is " << storeEvaluation.m_strUserIDCheck
            << " and total score is " << storeEvaluation.m_dTotalScore
            << " and check status is " << storeEvaluation.m_uiCheckStatus
            << " and create date is " << storeEvaluation.m_strCreateDate
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query store evaluation info req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryStoreEvaluationInfo(req.m_strEvaluationID, storeEvaluation))
    {
        LOG_ERROR_RLD("Query store evaluation info failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryAllStoreEvaluationReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::QueryAllStoreEvaluationReq req;
    std::list<PassengerFlowProtoHandler::StoreEvaluation> storeEvaluationList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &storeEvaluationList)
    {
        PassengerFlowProtoHandler::QueryAllStoreEvaluationRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllStoreEvaluationRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_storeEvaluationList.swap(storeEvaluationList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all store evaluation rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query all store evaluation rsp already send, dst id is " << strSrcID
            << " and result is " << blResult);

        if (blResult)
        {
            int i = 0;
            for (auto &evaluation : rsp.m_storeEvaluationList)
            {
                LOG_INFO_RLD("StoreEvaluation info[" << i++ << "]:"
                    << " evaluation id is " << evaluation.m_strEvaluationID
                    << " and check status is " << evaluation.m_uiCheckStatus
                    << " and check user id is " << evaluation.m_strUserIDCheck);
            }
        }
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query all store evaluation req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryAllStoreEvaluation(req.m_strStoreID, storeEvaluationList, req.m_uiCheckStatus, req.m_strBeginDate,
        req.m_strEndDate, req.m_uiBeginIndex))
    {
        LOG_ERROR_RLD("Query all store evaluation failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::AddRemotePatrolStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::string strPatrolID;
    PassengerFlowProtoHandler::AddRemotePatrolStoreReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &strPatrolID)
    {
        PassengerFlowProtoHandler::AddRemotePatrolStoreRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddRemotePatrolStoreRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strPatrolID = blResult ? strPatrolID : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add remote patrol store rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Add remote patrol store rsp already send, dst id is " << strSrcID
            << " and patrol id is " << strPatrolID
            << " and user id is " << req.m_patrolStore.m_strUserID
            << " and device id is " << req.m_patrolStore.m_strDeviceID
            << " and store id is " << req.m_patrolStore.m_strStoreID
            << " and plan id is " << req.m_patrolStore.m_strPlanID
            << " and patrol date is " << req.m_patrolStore.m_strPatrolDate
            << " and patrol result is " << req.m_patrolStore.m_uiPatrolResult
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Add remote patrol store req unserialize failed, src id is " << strSrcID);
            return false;
        }

    PassengerFlowProtoHandler::RemotePatrolStore patrolStore;
    patrolStore.m_strPatrolID = strPatrolID = CreateUUID();
    patrolStore.m_strUserID = req.m_patrolStore.m_strUserID;
    patrolStore.m_strDeviceID = req.m_patrolStore.m_strDeviceID;
    patrolStore.m_strEntranceIDList = req.m_patrolStore.m_strEntranceIDList;
    patrolStore.m_strStoreID = req.m_patrolStore.m_strStoreID;
    patrolStore.m_strPlanID = req.m_patrolStore.m_strPlanID;
    patrolStore.m_strPatrolDate = req.m_patrolStore.m_strPatrolDate;
    patrolStore.m_patrolPictureList = req.m_patrolStore.m_patrolPictureList;
    patrolStore.m_strPatrolPictureList = req.m_patrolStore.m_strPatrolPictureList;
    patrolStore.m_uiPatrolResult = req.m_patrolStore.m_uiPatrolResult;
    patrolStore.m_strDescription = req.m_patrolStore.m_strDescription;
    patrolStore.m_strCreateDate = CurrentTime();

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::AddRemotePatrolStore, this, patrolStore));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::DeleteRemotePatrolStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::DeleteRemotePatrolStoreReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::DeleteRemotePatrolStoreRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteRemotePatrolStoreRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete remote patrol store rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Delete remote patrol store rsp already send, dst id is " << strSrcID
            << " and patrol id is " << req.m_strPatrolID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Delete remote patrol store req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::DeleteRemotePatrolStore, this, req.m_strPatrolID));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::ModifyRemotePatrolStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::ModifyRemotePatrolStoreReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::ModifyRemotePatrolStoreRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyRemotePatrolStoreRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify remote patrol store rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Modify remote patrol store rsp already send, dst id is " << strSrcID
            << " and vip id is " << req.m_patrolStore.m_strPatrolID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Modify remote patrol store req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::ModifyRemotePatrolStore, this, req.m_patrolStore));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryRemotePatrolStoreInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::QueryRemotePatrolStoreInfoReq req;
    PassengerFlowProtoHandler::RemotePatrolStore patrolStore;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &patrolStore)
    {
        PassengerFlowProtoHandler::QueryRemotePatrolStoreInfoRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryRemotePatrolStoreInfoRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_patrolStore.m_strPatrolID = patrolStore.m_strPatrolID;
            rsp.m_patrolStore.m_strUserID = patrolStore.m_strUserID;
            rsp.m_patrolStore.m_strDeviceID = patrolStore.m_strDeviceID;
            rsp.m_patrolStore.m_strEntranceIDList = patrolStore.m_strEntranceIDList;
            rsp.m_patrolStore.m_strStoreID = patrolStore.m_strStoreID;
            rsp.m_patrolStore.m_strPlanID = patrolStore.m_strPlanID;
            rsp.m_patrolStore.m_strPatrolDate = patrolStore.m_strPatrolDate;
            rsp.m_patrolStore.m_patrolPictureList = patrolStore.m_patrolPictureList;
            rsp.m_patrolStore.m_strPatrolPictureList = patrolStore.m_strPatrolPictureList;
            rsp.m_patrolStore.m_uiPatrolResult = patrolStore.m_uiPatrolResult;
            rsp.m_patrolStore.m_strDescription = patrolStore.m_strDescription;
            rsp.m_patrolStore.m_strCreateDate = patrolStore.m_strCreateDate;
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query remote patrol store info rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query remote patrol store info rsp already send, dst id is " << strSrcID
            << " and patrol id is " << patrolStore.m_strPatrolID
            << " and user id is " << patrolStore.m_strUserID
            << " and device id is " << patrolStore.m_strDeviceID
            << " and store id is " << patrolStore.m_strStoreID
            << " and plan id is " << patrolStore.m_strPlanID
            << " and patrol date is " << patrolStore.m_strPatrolDate
            << " and patrol result is " << patrolStore.m_uiPatrolResult
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query remote patrol store info req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryRemotePatrolStoreInfo(req.m_strPatrolID, patrolStore))
    {
        LOG_ERROR_RLD("Query remote patrol store info failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryAllRemotePatrolStoreReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::QueryAllRemotePatrolStoreReq req;
    std::list<PassengerFlowProtoHandler::RemotePatrolStore> patrolStoreList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &patrolStoreList)
    {
        PassengerFlowProtoHandler::QueryAllRemotePatrolStoreRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllRemotePatrolStoreRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_patrolStoreList.swap(patrolStoreList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all remote patrol store rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query all remote patrol store rsp already send, dst id is " << strSrcID
            << " and result is " << blResult);

        if (blResult)
        {
            int i = 0;
            for (auto &patrol : rsp.m_patrolStoreList)
            {
                LOG_INFO_RLD("RemotePatrolStore info[" << i++ << "]:"
                    << " patrol id is " << patrol.m_strPatrolID
                    << " and user id is " << patrol.m_strUserID
                    << " and device id is " << patrol.m_strDeviceID
                    << " and patrol date is " << patrol.m_strPatrolDate
                    << " and patrol result is " << patrol.m_uiPatrolResult);
            }
        }
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query all remote patrol store req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryAllRemotePatrolStore(req.m_strStoreID, req.m_strPlanID, req.m_uiPatrolResult, req.m_uiPlanFlag,
        patrolStoreList, req.m_strBeginDate, req.m_strEndDate, req.m_uiBeginIndex))
    {
        LOG_ERROR_RLD("Query all remote patrol store failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::AddStoreSensorReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::string strSensorID;
    PassengerFlowProtoHandler::AddStoreSensorReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &strSensorID)
    {
        PassengerFlowProtoHandler::AddStoreSensorRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddStoreSensorRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strSensorID = blResult ? strSensorID : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add store sensor rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Add store sensor rsp already send, dst id is " << strSrcID
            << " and sensor id is " << strSensorID
            << " and sensor name is " << req.m_sensorInfo.m_strSensorName
            << " and sensor type is " << req.m_sensorInfo.m_strSensorType
            << " and store id is " << req.m_sensorInfo.m_strStoreID
            << " and device id is " << req.m_sensorInfo.m_strDeviceID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Add store sensor req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!ValidDeviceSensor(req.m_sensorInfo.m_strDeviceID, boost::lexical_cast<unsigned int>(req.m_sensorInfo.m_strSensorType)))
    {
        LOG_ERROR_RLD("Add store sensor req vaild failed because same type already exist on the same device");
        ReturnInfo::RetCode(ReturnInfo::SENSOR_TYPE_DUPLICATE);
        return false;
    }

    PassengerFlowProtoHandler::Sensor sensorInfo;
    sensorInfo.m_strSensorID = strSensorID = CreateUUID();
    sensorInfo.m_strSensorName = req.m_sensorInfo.m_strSensorName;
    sensorInfo.m_strSensorType = req.m_sensorInfo.m_strSensorType;
    sensorInfo.m_strSensorAlarmThreshold = req.m_sensorInfo.m_strSensorAlarmThreshold;
    sensorInfo.m_strStoreID = req.m_sensorInfo.m_strStoreID;
    sensorInfo.m_strDeviceID = req.m_sensorInfo.m_strDeviceID;
    sensorInfo.m_strValue = req.m_sensorInfo.m_strValue;
    sensorInfo.m_strCreateDate = CurrentTime();

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::AddStoreSensor, this, sensorInfo));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::DeleteStoreSensorReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::DeleteStoreSensorReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::DeleteStoreSensorRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteStoreSensorRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete store sensor rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Delete store sensor rsp already send, dst id is " << strSrcID
            << " and sensor id is " << req.m_strSensorID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Delete store sensor req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::DeleteStoreSensor, this, req.m_strSensorID));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::ModifyStoreSensorReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::ModifyStoreSensorReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::ModifyStoreSensorRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyStoreSensorRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify store sensor rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Modify store sensor rsp already send, dst id is " << strSrcID
            << " and store sensor id is " << req.m_sensorInfo.m_strSensorID
            << " and sensor name is " << req.m_sensorInfo.m_strSensorName
            << " and sensor type is " << req.m_sensorInfo.m_strSensorType
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Modify store sensor req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!ValidDeviceSensor(req.m_sensorInfo.m_strDeviceID, boost::lexical_cast<unsigned int>(req.m_sensorInfo.m_strSensorType), true, req.m_sensorInfo.m_strSensorID))
    {
        LOG_ERROR_RLD("Modify store sensor req vaild failed because same type already exist on the same device");
        ReturnInfo::RetCode(ReturnInfo::SENSOR_TYPE_DUPLICATE);
        return false;
    }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::ModifyStoreSensor, this, req.m_sensorInfo));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryStoreSensorInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::Sensor sensor;
    PassengerFlowProtoHandler::QueryStoreSensorInfoReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &sensor, &req)
    {
        PassengerFlowProtoHandler::QueryStoreSensorInfoRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryStoreSensorInfoRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_sensorInfo.m_strSensorID = sensor.m_strSensorID;
            rsp.m_sensorInfo.m_strSensorName = sensor.m_strSensorName;
            rsp.m_sensorInfo.m_strSensorType = sensor.m_strSensorType;
            rsp.m_sensorInfo.m_strSensorAlarmThreshold = sensor.m_strSensorAlarmThreshold;
            rsp.m_sensorInfo.m_strStoreID = sensor.m_strStoreID;
            rsp.m_sensorInfo.m_strDeviceID = sensor.m_strDeviceID;
            rsp.m_sensorInfo.m_strValue = sensor.m_strValue;
            rsp.m_sensorInfo.m_strCreateDate = sensor.m_strCreateDate;
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query store sensor info rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query store sensor info rsp already send, dst id is " << strSrcID
            << " and sensor id is " << req.m_strSensorID
            << " and sensor name is " << sensor.m_strSensorName
            << " and sensor type is " << sensor.m_strSensorType
            << " and store id is " << sensor.m_strStoreID
            << " and device id is " << sensor.m_strDeviceID
            << " and value is " << sensor.m_strValue
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query store sensor info req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!PassengerFlowManager::QueryStoreSensorInfo(req.m_strSensorID, sensor))
    {
        LOG_ERROR_RLD("Query store sensor info failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryAllStoreSensorReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::QueryAllStoreSensorReq req;
    std::list<PassengerFlowProtoHandler::Sensor> sensorList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &sensorList)
    {
        PassengerFlowProtoHandler::QueryAllStoreSensorRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllStoreSensorRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_sensorList.swap(sensorList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all store sensor rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query all store sensor rsp already send, dst id is " << strSrcID
            << " and result is " << blResult);

        if (blResult)
        {
            int i = 0;
            for (auto &sensor : rsp.m_sensorList)
            {
                LOG_INFO_RLD("StoreSensor info[" << i++ << "]:"
                    << " sensor id is " << sensor.m_strSensorID
                    << " and sensor name is " << sensor.m_strSensorName
                    << " and sensor type is " << sensor.m_strSensorType
                    << " and store id is " << sensor.m_strStoreID
                    << " and device id is " << sensor.m_strDeviceID
                    << " and value is " << sensor.m_strValue);
            }
        }
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query all store sensor req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryAllStoreSensor(req.m_strStoreID, sensorList))
    {
        LOG_ERROR_RLD("Query all store sensor failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::ImportPOSDataReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::string strEntranceID;
    PassengerFlowProtoHandler::ImportPOSDataReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::ImportPOSDataRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ImportPOSDataRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Import POS data rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Import POS data rsp already send, dst id is " << strSrcID
            << " and store id is " << req.m_strStoreID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Import POS data req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::ImportPOSData, this, req.m_strStoreID, req.m_uiOrderAmount,
        req.m_uiGoodsAmount, req.m_dDealAmount, req.m_strDealDate));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryCustomerFlowStatisticReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::string strChartData;
    PassengerFlowProtoHandler::QueryCustomerFlowStatisticReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &strChartData)
    {
        PassengerFlowProtoHandler::QueryCustomerFlowStatisticRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryCustomerFlowStatisticRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strChartData = blResult ? strChartData : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query customer flow statistic rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query customer flow statistic rsp already send, dst id is " << strSrcID
            << " and chart data is " << strChartData
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query customer flow statistic req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryCustomerFlowStatistic(req.m_strStoreID, req.m_strBeginDate, req.m_strEndDate,
        req.m_uiTimePrecision, strChartData))
    {
        LOG_ERROR_RLD("Query customer flow statistic failed, statistic error, src id is " << strSrcID
            << " and store id is " << req.m_strStoreID);
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::QueryPatrolResultReportReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    std::string strChartData;
    PassengerFlowProtoHandler::QueryPatrolResultReportReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &strChartData)
    {
        PassengerFlowProtoHandler::QueryPatrolResultReportRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryPatrolResultReportRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strChartData = blResult ? strChartData : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query patrol result report rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query patrol result report rsp already send, dst id is " << strSrcID
            << " and chart data is " << strChartData
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query patrol result report req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryPatrolResultReport(req.m_strUserID, req.m_strStoreID, req.m_strPatrolUserID, req.m_uiPatrolResult,
        req.m_strBeginDate, req.m_strEndDate, strChartData))
    {
        LOG_ERROR_RLD("Query patrol result report failed, statistic error, src id is " << strSrcID
            << " and store id is " << req.m_strStoreID);
        return false;
    }

    LOG_ERROR_RLD("---debug, chart: " << strChartData);
    blResult = true;

    return blResult;
}

bool PassengerFlowManager::ReportCustomerFlowDataReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::ReportCustomerFlowDataReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::ReportCustomerFlowDataRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ReportCustomerFlowDataRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Report customer flow data rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Report customer flow data rsp already send, dst id is " << strSrcID
            << " and device id is " << req.m_strDeviceID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Report customer flow data req unserialize failed, src id is " << strSrcID);
            return false;
        }

    std::string strEntranceID;
    if (!QueryDeviceBoundEntrance(req.m_strDeviceID, strEntranceID))
    {
        LOG_ERROR_RLD("Report customer flow data failed, query device bound entrance error, device id is " << req.m_strDeviceID);
        return false;
    }

    if (strEntranceID.empty())
    {
        LOG_ERROR_RLD("Report customer flow data error, the device is not bound, device id is " << req.m_strDeviceID);
    }
    else
    {
        m_DBRuner.Post(boost::bind(&PassengerFlowManager::ReportCustomerFlow, this, req.m_strDeviceID, req.m_customerFlowList));
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::ReportSensorInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

    bool blResult = false;

    PassengerFlowProtoHandler::ReportSensorInfoReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        PassengerFlowProtoHandler::ReportSensorInfoRsp rsp;
        rsp.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ReportSensorInfoRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::RetCode();
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Report sensor info rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Report sensor info rsp already send, dst id is " << strSrcID
            << " and device id is " << req.m_strDeviceID
            << " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Report sensor info req unserialize failed, src id is " << strSrcID);
            return false;
        }

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::ReportSensorInfo, this, req.m_strDeviceID, req.m_sensorList));

    blResult = true;

    return blResult;
}

bool PassengerFlowManager::PushMessage(const std::string &strTitle, const std::string &strContent, const std::string &strPayload, const std::string &strUserID)
{
    MessagePush_Getui::PushMessage message;
    message.strTitle = strTitle;
    message.strNotyContent = strContent;
    message.strPayloadContent = strPayload; //strContent;
    message.bIsOffline = true;
    message.iOfflineExpireTime = 3600;
    //message.strClientID = "9fff548fac1537a7963a49a9b191e195";
    message.strAlias = strUserID;

    //无法确定处理人登录时使用的终端，所以Android和iOS都推送
    m_pushGetuiIOS.PushSingle(message, MessagePush_Getui::PLATFORM_IOS);
    m_pushGetuiAndroid.PushSingle(message, MessagePush_Getui::PLATFORM_ANDROID);

    return true;
}

std::string PassengerFlowManager::CurrentTime()
{
    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    strCurrentTime.replace(strCurrentTime.find_first_of('T'), 1, std::string(" "));

    return strCurrentTime;
}

int PassengerFlowManager::Year(const std::string &strDate)
{
    return boost::lexical_cast<int>(strDate.substr(0, 4));
}

int PassengerFlowManager::Month(const std::string &strDate)
{
    return boost::lexical_cast<int>(strDate.substr(5, 2));
}

int PassengerFlowManager::Quarter(const std::string &strDate)
{
    return (Month(strDate) - 1) / 4 + 1;
}

int PassengerFlowManager::MonthDuration(const std::string &strBeginDate, const std::string &strEndDate)
{
    /*int beginYear = Year(strBeginDate);
    int endYear = Year(strEndDate);
    int beginMonth = Month(strBeginDate);
    int endMonth = Month(strEndDate);

    if (beginYear == endYear)
    {
        return endMonth - beginMonth + 1;
    }
    else
    {
        return (endYear - beginYear - 1) * 12 + endMonth + 12 - beginMonth + 1;
    }*/

    return (Year(strEndDate) - Year(strBeginDate)) * 12 + Month(strEndDate) - Month(strBeginDate) + 1;
}

int PassengerFlowManager::QuarterDuration(const std::string &strBeginDate, const std::string &strEndDate)
{
    return (Year(strEndDate) - Year(strBeginDate)) * 4 + Quarter(strEndDate) - Quarter(strBeginDate) + 1;
}

int PassengerFlowManager::TimePrecisionScale(const std::string &strDate, const unsigned int uiTimePrecision)
{
    boost::posix_time::ptime epoch = boost::posix_time::time_from_string("1970-01-01 00:00:00");
    boost::posix_time::ptime date = boost::posix_time::time_from_string(strDate);
    return ((date - epoch).total_seconds() / uiTimePrecision) * uiTimePrecision;
}

bool PassengerFlowManager::UserAccessPermission(const std::string &strUserID, const int iMsgType, std::string &strPermission)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select b.access_permission from"
        " t_user_role_association a join t_role_menu_association b on a.role_id = b.role_id"
        " where a.user_id = '%s' and b.menu_id = %d";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), iMsgType);

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = strColumn;
            break;

        default:
            LOG_ERROR_RLD("UserAccessPermission sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("UserAccessPermission exec sql failed, sql is " << sql);
        return false;
    }

    if (!ResultList.empty())
    {
        strPermission = boost::any_cast<std::string>(ResultList.front());
    }

    return true;
}

bool PassengerFlowManager::IsValidArea(const std::string &strUserID, const std::string &strAreaName)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select a.area_name from t_area_info a join t_user_area_association b"
        " on a.area_id = b.area_id where b.user_id = '%s' and a.area_name = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), strAreaName.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = strColumn;
            break;

        default:
            LOG_ERROR_RLD("IsValidArea sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("IsValidArea exec sql failed, sql is " << sql);
        return false;
    }

    if (!ResultList.empty())
    {
        LOG_ERROR_RLD("IsValidArea failed, the area name is used");
        return false;
    }

    return true;
}

bool PassengerFlowManager::QueryUserCompany(const std::string &strUserID, std::string &strCompanyID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select company_id from t_company_user_info where user_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = strColumn;
            break;

        default:
            LOG_ERROR_RLD("QueryUserCompany sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryUserCompany exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryUserCompany sql result is empty, sql is " << sql);
        return false;
    }

    strCompanyID = boost::any_cast<std::string>(ResultList.front());

    return true;
}

void PassengerFlowManager::AddArea(const std::string &strUserID, const std::string &strCompanyID, const PassengerFlowProtoHandler::Area &areaInfo)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_area_info (id, area_id, area_name, level, parent_area_id, company_id, create_date, extend)"
        " values (uuid(), '%s', '%s', %d, '%s', '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, areaInfo.m_strAreaID.c_str(), areaInfo.m_strAreaName.c_str(), areaInfo.m_uiLevel, areaInfo.m_strParentAreaID.c_str(),
        strCompanyID.c_str(), areaInfo.m_strCreateDate.c_str(), areaInfo.m_strExtend.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddArea exec sql failed, sql is " << sql);
        return;
    }
}

void PassengerFlowManager::DeleteArea(const std::string &strAreaID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_area_info where area_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strAreaID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteArea exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyArea(const PassengerFlowProtoHandler::Area &areaInfo)
{
    bool blModified = false;

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len = snprintf(sql, size, "update t_area_info set id = id");

    if (!areaInfo.m_strAreaName.empty())
    {
        len += snprintf(sql + len, size - len, ", area_name = '%s'", areaInfo.m_strAreaName.c_str());
        blModified = true;
    }

    if (!areaInfo.m_strExtend.empty())
    {
        len += snprintf(sql + len, size - len, ", extend = '%s'", areaInfo.m_strExtend.c_str());
        blModified = true;
    }

    if (!blModified)
    {
        LOG_INFO_RLD("ModifyArea completed, area info is not changed");
        return;
    }

    snprintf(sql + len, size - len, " where area_id = '%s'", areaInfo.m_strAreaID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("ModifyArea exec sql failed, sql is " << sql);
    }
}

bool PassengerFlowManager::QueryAreaInfo(const std::string &strAreaID, PassengerFlowProtoHandler::Area &areaInfo)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select area_id, area_name, level, parent_area_id, create_date, extend from"
        " t_area_info where area_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strAreaID.c_str());

    PassengerFlowProtoHandler::Area rstArea;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstArea.m_strAreaID = strColumn;
            break;
        case 1:
            rstArea.m_strAreaName = strColumn;
            break;
        case 2:
            rstArea.m_uiLevel = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 3:
            rstArea.m_strParentAreaID = strColumn;
            break;
        case 4:
            rstArea.m_strCreateDate = strColumn;
            break;
        case 5:
            rstArea.m_strExtend = strColumn;
            result = rstArea;
            break;

        default:
            LOG_ERROR_RLD("QueryAreaInfo sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryAreaInfo exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryAreaInfo sql result is empty, sql is " << sql);
        return false;
    }

    auto area = boost::any_cast<PassengerFlowProtoHandler::Area>(ResultList.front());
    areaInfo.m_strAreaID = area.m_strAreaID;
    areaInfo.m_strAreaName = area.m_strAreaName;
    areaInfo.m_uiLevel = area.m_uiLevel;
    areaInfo.m_strParentAreaID = area.m_strParentAreaID;
    areaInfo.m_strCreateDate = area.m_strCreateDate;
    areaInfo.m_strExtend = area.m_strExtend;

    return true;
}

bool PassengerFlowManager::QueryAreaParent(const std::string &strAreaID, std::list<PassengerFlowProtoHandler::Area> &areaList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select a.area_id, a.area_name, a.level, ifnull(b.area_id, ''), ifnull(b.area_name, ''), ifnull(b.level, 0),"
        " ifnull(c.area_id, ''), ifnull(c.area_name, ''), ifnull(c.level, 0) from"
        " t_area_info a left join t_area_info b on a.parent_area_id = b.area_id"
        " left join t_area_info c on b.parent_area_id = c.area_id"
        " where a.area_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strAreaID.c_str());

    PassengerFlowProtoHandler::Area rstArea;
    std::list<PassengerFlowProtoHandler::Area> rstAreaList;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstArea.m_strAreaID = strColumn;
            break;
        case 1:
            rstArea.m_strAreaName = strColumn;
            break;
        case 2:
            rstArea.m_uiLevel = boost::lexical_cast<unsigned int>(strColumn);
            rstAreaList.push_back(rstArea);
            break;
        case 3:
            rstArea.m_strAreaID = strColumn;
            break;
        case 4:
            rstArea.m_strAreaName = strColumn;
            break;
        case 5:
            rstArea.m_uiLevel = boost::lexical_cast<unsigned int>(strColumn);
            rstAreaList.push_back(rstArea);
            break;
        case 6:
            rstArea.m_strAreaID = strColumn;
            break;
        case 7:
            rstArea.m_strAreaName = strColumn;
            break;
        case 8:
            rstArea.m_uiLevel = boost::lexical_cast<unsigned int>(strColumn);
            rstAreaList.push_back(rstArea);
            result = rstAreaList;
            break;

        default:
            LOG_ERROR_RLD("QueryAreaParent sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryAreaParent exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryAreaParent sql result is empty, sql is " << sql);
        return false;
    }

    auto result = boost::any_cast<std::list<PassengerFlowProtoHandler::Area>>(ResultList.front());
    for (auto it = result.begin(), end = result.end(); it != end; ++it)
    {
        if (it->m_uiLevel == 0) break;
        areaList.push_front(*it);
    }

    return true;
}

bool PassengerFlowManager::QueryAllArea(const std::string &strUserID, std::list<PassengerFlowProtoHandler::Area> &areaList,
    const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 10*/)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select a.area_id, a.area_name, level, parent_area_id from t_area_info a"
        " where a.company_id = (select b.company_id from t_company_user_info b where b.user_id = '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str());

    PassengerFlowProtoHandler::Area rstArea;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstArea.m_strAreaID = strColumn;
            break;
        case 1:
            rstArea.m_strAreaName = strColumn;
            break;
        case 2:
            rstArea.m_uiLevel = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 3:
            rstArea.m_strParentAreaID = strColumn;
            result = rstArea;
            break;

        default:
            LOG_ERROR_RLD("QueryAllArea sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryAllArea exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        areaList.push_back(std::move(boost::any_cast<PassengerFlowProtoHandler::Area>(result)));
    }

    return true;
}

void PassengerFlowManager::BindPushClientID(const std::string &strUserID, const std::string &strCllientID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_user_client_id_association (id, user_id, client_id, create_date)"
        " values (uuid(), '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), strCllientID.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("BindPushClientID exec sql failed, sql is " << sql);
        return;
    }
}

void PassengerFlowManager::UnbindPushClientID(const std::string &strUserID, const std::string &strCllientID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_user_client_id_association (id, user_id, client_id, create_date)"
        " values (uuid(), '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), strCllientID.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("UnbindPushClientID exec sql failed, sql is " << sql);
        return;
    }
}

bool PassengerFlowManager::QueryPushParameter(const std::string &strUserID, std::list<std::string> &strClientIDList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select client_id from t_user_client_id_association where user_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = strColumn;
            break;

        default:
            LOG_ERROR_RLD("QueryPushParameter sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryPushParameter exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryPushParameter sql result is empty, sql is " << sql);
        return false;
    }

    for (auto result : ResultList)
    {
        strClientIDList.push_back(boost::any_cast<std::string>(ResultList.front()));
    }

    return true;
}

bool PassengerFlowManager::IsValidStore(const std::string &strUserID, const std::string &strStoreName)
{
    //可以添加相同名称的店铺
    return true;

    char sql[1024] = { 0 };
    const char *sqlfmt = "select a.store_name from t_store_info a join t_user_store_association b"
        " on a.store_id = b.store_id where b.user_id = '%s' and a.store_name = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), strStoreName.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = strColumn;
            break;

        default:
            LOG_ERROR_RLD("IsValidStore sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("IsValidStore exec sql failed, sql is " << sql);
        return false;
    }

    if (!ResultList.empty())
    {
        LOG_ERROR_RLD("IsValidStore failed, the store name is used");
        return false;
    }

    return true;
}

void PassengerFlowManager::AddStore(const std::string &strUserID, const PassengerFlowProtoHandler::Store &storeInfo)
{
    std::string tel;
    for (auto it = storeInfo.m_strTelephoneList.begin(), end = storeInfo.m_strTelephoneList.end(); it != end;)
    {
        tel.append(*it);
        if (++it != end) tel.append(",");
    }

    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_store_info (id, store_id, store_name, goods_category, address, area_id, open_state, telephone, create_date)"
        " values (uuid(), '%s', '%s', '%s', '%s', '%s', %d, '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, storeInfo.m_strStoreID.c_str(), storeInfo.m_strStoreName.c_str(), storeInfo.m_strGoodsCategory.c_str(),
        storeInfo.m_strAddress.c_str(), storeInfo.m_area.m_strAreaID.c_str(), storeInfo.m_uiOpenState, tel.c_str(), storeInfo.m_strCreateDate.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddStore exec sql failed, sql is " << sql);
        return;
    }

    AddUserStoreAssociation(strUserID, storeInfo.m_strStoreID);
}

void PassengerFlowManager::AddUserStoreAssociation(const std::string &strUserID, const std::string &strStoreID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_user_store_association (id, user_id, store_id, user_role, create_date)"
        " values (uuid(), '%s', '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), strStoreID.c_str(), "owner", CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddUserStoreAssociation exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::DeleteStore(const std::string &strStoreID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete a, b, c, d from"
        " t_store_info a left join t_user_store_association b on a.store_id = b.store_id"
        " left join t_entrance_info c on a.store_id = c.store_id"
        " left join t_entrance_device_association d on c.entrance_id = d.entrance_id"
        " where a.store_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strStoreID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteStore exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyStore(const PassengerFlowProtoHandler::Store &storeInfo)
{
    bool blModified = false;

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len = snprintf(sql, size, "update t_store_info set id = id");

    if (!storeInfo.m_strStoreName.empty())
    {
        len += snprintf(sql + len, size - len, ", store_name = '%s'", storeInfo.m_strStoreName.c_str());
        blModified = true;
    }

    if (!storeInfo.m_strGoodsCategory.empty())
    {
        len += snprintf(sql + len, size - len, ", goods_category = '%s'", storeInfo.m_strGoodsCategory.c_str());
        blModified = true;
    }

    if (!storeInfo.m_strAddress.empty())
    {
        len += snprintf(sql + len, size - len, ", address = '%s'", storeInfo.m_strAddress.c_str());
        blModified = true;
    }

    if (!storeInfo.m_area.m_strAreaID.empty())
    {
        len += snprintf(sql + len, size - len, ", area_id = '%s'", storeInfo.m_area.m_strAreaID.c_str());
        blModified = true;
    }

    if (storeInfo.m_uiOpenState != UNUSED_INPUT_UINT)
    {
        len += snprintf(sql + len, size - len, ", open_state = %d", storeInfo.m_uiOpenState);
        blModified = true;
    }

    if (!storeInfo.m_strTelephoneList.empty())
    {
        std::string tel;
        for (auto it = storeInfo.m_strTelephoneList.begin(), end = storeInfo.m_strTelephoneList.end(); it != end;)
        {
            tel.append(*it);
            if (++it != end) tel.append(",");
        }
        len += snprintf(sql + len, size - len, ", telephone = '%s'", tel.c_str());
        blModified = true;
    }

    if (!blModified)
    {
        LOG_INFO_RLD("ModifyStore completed, store info is not changed");
        return;
    }

    snprintf(sql + len, size - len, " where store_id = '%s'", storeInfo.m_strStoreID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("ModifyStore exec sql failed, sql is " << sql);
    }
}

bool PassengerFlowManager::QueryStoreInfo(const std::string &strStoreID, PassengerFlowProtoHandler::Store &storeInfo)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select a.store_id, a.store_name, a.goods_category, a.address, a.area_id, a.open_state, a.telephone, a.create_date, b.area_name from"
        " t_store_info a join t_area_info b on a.area_id = b.area_id where a.store_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strStoreID.c_str());

    PassengerFlowProtoHandler::Store rstStore;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstStore.m_strStoreID = strColumn;
            break;
        case 1:
            rstStore.m_strStoreName = strColumn;
            break;
        case 2:
            rstStore.m_strGoodsCategory = strColumn;
            break;
        case 3:
            rstStore.m_strAddress = strColumn;
            break;
        case 4:
            rstStore.m_area.m_strAreaID = strColumn;
            break;
        case 5:
            rstStore.m_uiOpenState = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 6:
        {
            if (!strColumn.empty())
            {
                std::list<std::string> telList;
                boost::split(telList, strColumn, boost::is_any_of(","));
                for (auto tel : telList) rstStore.m_strTelephoneList.push_back(tel);
            }
            break;
        }
        case 7:
            rstStore.m_strCreateDate = strColumn;
            break;
        case 8:
            rstStore.m_area.m_strAreaName = strColumn;
            result = rstStore;
            break;

        default:
            LOG_ERROR_RLD("QueryStoreInfo sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryStoreInfo exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryStoreInfo sql result is empty, sql is " << sql);
        return false;
    }

    if (!QueryStoreEntrance(strStoreID, storeInfo.m_entranceList))
    {
        LOG_ERROR_RLD("QueryStoreInfo failed, query store entrance error, store id is " << storeInfo.m_strStoreID);
        return false;
    }

    auto store = boost::any_cast<PassengerFlowProtoHandler::Store>(ResultList.front());
    storeInfo.m_strStoreID = store.m_strStoreID;
    storeInfo.m_strStoreName = store.m_strStoreName;
    storeInfo.m_strGoodsCategory = store.m_strGoodsCategory;
    storeInfo.m_strAddress = store.m_strAddress;
    storeInfo.m_area.m_strAreaID = store.m_area.m_strAreaID;
    storeInfo.m_uiOpenState = store.m_uiOpenState;
    storeInfo.m_strTelephoneList = store.m_strTelephoneList;
    storeInfo.m_strCreateDate = store.m_strCreateDate;

    return true;
}

bool PassengerFlowManager::QueryStoreEntrance(const std::string &strStoreID, std::list<PassengerFlowProtoHandler::Entrance> &entranceList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select entrance_id, entrance_name, screenshot_id from t_entrance_info where store_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strStoreID.c_str());

    struct ResultEntrance
    {
        std::string strEntranceID;
        std::string strEntranceName;
        std::string strPicture;
    } rstEntrance;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstEntrance.strEntranceID = strColumn;
            break;
        case 1:
            rstEntrance.strEntranceName = strColumn;
            break;
        case 2:
            rstEntrance.strPicture = strColumn.empty() ? strColumn : m_ParamInfo.m_strFileServerURL + "download_file&fileid=" + strColumn;
            result = rstEntrance;
            break;

        default:
            LOG_ERROR_RLD("QueryStoreEntrance sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryStoreEntrance exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        auto castRst = boost::any_cast<ResultEntrance>(result);
        PassengerFlowProtoHandler::Entrance entrance;
        entrance.m_strEntranceID = castRst.strEntranceID;
        entrance.m_strEntranceName = castRst.strEntranceName;
        entrance.m_strPicture = castRst.strPicture;

        QueryEntranceDevice(entrance.m_strEntranceID, entrance.m_strDeviceIDList);

        entranceList.push_back(std::move(entrance));
    }

    return true;
}

bool PassengerFlowManager::QueryEntranceDevice(const std::string &strEntranceID, std::list<std::string> &strDeviceIDList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select device_id from t_entrance_device_association where entrance_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEntranceID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = strColumn;
            break;

        default:
            LOG_ERROR_RLD("QueryEntranceDevice sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryEntranceDevice exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        strDeviceIDList.push_back(boost::any_cast<std::string>(result));
    }

    return true;
}

bool PassengerFlowManager::QueryAllStore(const std::string &strUserID, const std::string &strAreaID, const unsigned int uiOpenState,
    std::list<PassengerFlowProtoHandler::Store> &storeList, const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 10*/)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select a.store_id from"
        " t_store_info a join t_user_store_association b on a.store_id = b.store_id";
    //" where b.user_id = '%s'";
    int len = snprintf(sql, sizeof(sql), sqlfmt);
    //int len = snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str());

    if (!strAreaID.empty())
    {
        std::list<std::string> areaList;
        QuerySubArea(strAreaID, areaList);
        std::string areas = "'" + strAreaID + "'";
        for (auto area : areaList) areas.append(", '").append(area).append("'");

        len += snprintf(sql + len, size - len, " and a.area_id in (%s)", areas.c_str());
    }

    if (uiOpenState != UNUSED_INPUT_UINT)
    {
        len += snprintf(sql + len, size - len, " and a.open_state = %d", uiOpenState);
    }

    snprintf(sql + len, size - len, " limit %d, %d", uiBeginIndex, uiPageSize);

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = strColumn;
            break;

        default:
            LOG_ERROR_RLD("QueryAllStore sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryAllStore exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        PassengerFlowProtoHandler::Store store;
        QueryStoreInfo(boost::any_cast<std::string>(result), store);

        storeList.push_back(std::move(store));
    }

    return true;
}

bool PassengerFlowManager::QuerySubArea(const std::string &strAreaID, std::list<std::string> &strSubAreaIDList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select area_id, level from t_area_info where parent_area_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strAreaID.c_str());

    struct ResultArea
    {
        std::string strAreaID;
        int iLevel;
    } rstArea;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstArea.strAreaID = strColumn;
            break;
        case 1:
            rstArea.iLevel = boost::lexical_cast<int>(strColumn);
            result = rstArea;
            break;

        default:
            LOG_ERROR_RLD("QueryAllStore sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryAllStore exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        auto area = boost::any_cast<ResultArea>(result);
        if (area.iLevel < 3)
        {
            QuerySubArea(area.strAreaID, strSubAreaIDList);
        }

        strSubAreaIDList.push_back(area.strAreaID);
    }

    return true;
}

bool PassengerFlowManager::IsValidEntrance(const std::string &strStoreID, const std::string &strEntranceName)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select entrance_name from t_entrance_info"
        " where store_id = '%s' and entrance_name = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strStoreID.c_str(), strEntranceName.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = strColumn;
            break;

        default:
            LOG_ERROR_RLD("IsValidEntrance sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("IsValidEntrance exec sql failed, sql is " << sql);
        return false;
    }

    if (!ResultList.empty())
    {
        LOG_ERROR_RLD("IsValidEntrance failed, the entrance name is used");
        return false;
    }

    return true;
}

void PassengerFlowManager::AddEntrance(const std::string &strStoreID, const PassengerFlowProtoHandler::Entrance &entranceInfo)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_entrance_info (id, entrance_id, entrance_name, store_id, screenshot_id, create_date)"
        " values (uuid(), '%s', '%s', '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, entranceInfo.m_strEntranceID.c_str(), entranceInfo.m_strEntranceName.c_str(),
        strStoreID.c_str(), entranceInfo.m_strPicture.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddEntrance exec sql failed, sql is " << sql);
        return;
    }

    for (auto &device : entranceInfo.m_strDeviceIDList)
    {
        AddEntranceDevice(entranceInfo.m_strEntranceID, device);
    }
}

bool PassengerFlowManager::QueryDeviceBoundEntrance(const std::string &strDeviceID, std::string &strEntranceID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select entrance_id from t_entrance_device_association where device_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strDeviceID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = strColumn;
            break;

        default:
            LOG_ERROR_RLD("QueryDeviceBoundEntrance sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryDeviceBoundEntrance exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("QueryDeviceBoundEntrance sql result is empty, sql is " << sql);
    }
    else
    {
        strEntranceID = boost::any_cast<std::string>(ResultList.front());
    }

    return true;
}

void PassengerFlowManager::AddEntranceDevice(const std::string &strEntranceID, const std::string &strDeviceID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_entrance_device_association (id, entrance_id, device_id, create_date)"
        " values(uuid(), '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strEntranceID.c_str(), strDeviceID.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddEntranceDevice exec sql failed, sql is " << sql);
        return;
    }
}

void PassengerFlowManager::DeleteEntrance(const std::string &strEntranceID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete a, b from"
        " t_entrance_info a left join t_entrance_device_association b"
        " on a.entrance_id = b.entrance_id where a.entrance_id = '%s' ";
    snprintf(sql, sizeof(sql), sqlfmt, strEntranceID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteEntrance exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::DeleteEntranceDevice(const std::string &strEntranceID, const std::string &strDeviceID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_entrance_device_association"
        " where entrance_id = '%s' and device_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEntranceID.c_str(), strDeviceID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteEntranceDevice exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyEntrance(const PassengerFlowProtoHandler::Entrance &entrance,
    const std::list<std::string> &strAddedDeviceIDList, const std::list<std::string> &strDeletedDeviceIDList)
{
    bool blModified = false;

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len = snprintf(sql, size, "update t_entrance_info set id = id");

    if (!entrance.m_strEntranceName.empty())
    {
        len += snprintf(sql + len, size - len, ", entrance_name = '%s'", entrance.m_strEntranceName.c_str());
        blModified = true;
    }

    if (!entrance.m_strPicture.empty())
    {
        len += snprintf(sql + len, size - len, ", screenshot_id = '%s'", entrance.m_strPicture.c_str());
        blModified = true;
    }

    for (auto &device : strAddedDeviceIDList)
    {
        AddEntranceDevice(entrance.m_strEntranceID, device);
    }

    for (auto &device : strDeletedDeviceIDList)
    {
        DeleteEntranceDevice(entrance.m_strEntranceID, device);
    }

    if (!blModified)
    {
        LOG_INFO_RLD("ModifyEntrance completed, entrance info is not changed");
        return;
    }

    snprintf(sql + len, size - len, " where entrance_id = '%s'", entrance.m_strEntranceID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("ModifyEntrance exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::AddEvent(const PassengerFlowProtoHandler::Event &eventInfo)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_event_info (id, event_id, source, submit_date, expire_date, user_id, device_id, process_state, view_state, create_date)"
        " values (uuid(), '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d, '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, eventInfo.m_strEventID.c_str(), eventInfo.m_strSource.c_str(),
        eventInfo.m_strSubmitDate.c_str(), eventInfo.m_strExpireDate.empty() ? "2199-01-01 00:00:00" : eventInfo.m_strExpireDate.c_str(), eventInfo.m_strUserID.c_str(),
        eventInfo.m_strDeviceID.c_str(), eventInfo.m_strProcessState.c_str(), eventInfo.m_uiViewState, eventInfo.m_strCreateDate.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddEvent exec sql failed, sql is " << sql);
        return;
    }

    for (auto &type : eventInfo.m_uiTypeList)
    {
        AddEventType(eventInfo.m_strEventID, type);
    }

    for (auto &user : eventInfo.m_strHandlerList)
    {
        AddEventUserAssociation(eventInfo.m_strEventID, user, "handler");
    }

    if (!eventInfo.m_strRemark.empty())
    {
        AddEventRemark(eventInfo.m_strEventID, eventInfo.m_strUserID, eventInfo.m_strRemark);
    }
}

void PassengerFlowManager::AddEventType(const std::string &strEventID, const unsigned int uiType)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_event_type (id, event_id, event_type, create_date)"
        " values (uuid(), '%s', %d, '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strEventID.c_str(), uiType, CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddEventType exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::AddEventUserAssociation(const std::string &strEventID, const std::string &strUserID, const std::string &strUserRole)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_event_user_association (id, event_id, user_id, user_role, read_state, create_date)"
        " values (uuid(), '%s', '%s', '%s', %d, '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strEventID.c_str(), strUserID.c_str(), strUserRole.c_str(), UNREAD_STATE, CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddEventUserAssociation exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::AddEventRemark(const std::string &strEventID, const std::string &strUserID, const std::string &strRemark)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_event_remark (id, event_id, remark, user_id, create_date)"
        " values (uuid(), '%s', '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strEventID.c_str(), strRemark.c_str(), strUserID.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddEventRemark exec sql failed, sql is " << sql);
    }
}

bool PassengerFlowManager::QueryStoreByEvent(const std::string &strSource, const unsigned int uiEventType, std::string &strStoreName, std::string &strStoreID)
{
    if (uiEventType == EVENT_REMOTE_PATROL || uiEventType == EVENT_REGULAR_PATROL)
    {
        return QueryStoreByRegularPatrol(strSource, strStoreName, strStoreID);
    }
    else if (uiEventType == EVENT_STORE_EVALUATION)
    {
        return QueryStoreByStoreEvaluation(strSource, strStoreName, strStoreID);
    }
    else
    {
        LOG_ERROR_RLD("QueryStoreByEvent failed, event type error, type is " << uiEventType);
        return false;
    }
}

bool PassengerFlowManager::QueryStoreByRegularPatrol(const std::string &strPatrolID, std::string &strStoreName, std::string &strStoreID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select b.store_name, b.store_id from t_remote_patrol_store a"
        " join t_store_info b on a.store_id = b.store_id where a.patrol_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPatrolID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            //result = strColumn;
            strStoreName = strColumn;
            break;

        case 1:
            strStoreID = strColumn;

            result = strStoreID + "||" + strStoreName;
            break;

        default:
            LOG_ERROR_RLD("QueryStoreByRegularPatrol sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryStoreByRegularPatrol exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryStoreByRegularPatrol sql result is empty, sql is " << sql);
        return false;
    }

    //strStoreName = boost::any_cast<std::string>(ResultList.front());

    return true;
}

bool PassengerFlowManager::QueryStoreByStoreEvaluation(const std::string &strEvaluationID, std::string &strStoreName, std::string &strStoreID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select b.store_name, b.store_id from t_store_evaluation a"
        " join t_store_info b on a.store_id = b.store_id where a.evaluation_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEvaluationID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            //result = strColumn;
            strStoreName = strColumn;
            break;

        case 1:
            strStoreID = strColumn;

            result = strStoreID + "||" + strStoreName;
            break;

        default:
            LOG_ERROR_RLD("QueryStoreByStoreEvaluation sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryStoreByStoreEvaluation exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryStoreByStoreEvaluation sql result is empty, sql is " << sql);
        return false;
    }

    //strStoreName = boost::any_cast<std::string>(ResultList.front());

    return true;
}

void PassengerFlowManager::DeleteEvent(const std::string &strEventID, const std::string &strUserID)
{
    DeleteCreatedEvent(strEventID, strUserID);
    DeleteHandledEvent(strEventID, strUserID);
}

void PassengerFlowManager::DeleteCreatedEvent(const std::string &strEventID, const std::string &strUserID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "update t_event_info set state = 1 where event_id = '%s' and user_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEventID.c_str(), strUserID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteCreatedEvent exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::DeleteHandledEvent(const std::string &strEventID, const std::string &strUserID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_event_user_association where event_id = '%s' and user_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEventID.c_str(), strUserID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteHandledEvent exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyEvent(const PassengerFlowProtoHandler::Event &eventInfo)
{
    bool blModified = false;

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len = snprintf(sql, size, "update t_event_info set id = id");

    if (!eventInfo.m_strSource.empty())
    {
        len += snprintf(sql + len, size - len, ", source = '%s'", eventInfo.m_strSource.c_str());
        blModified = true;
    }

    if (!eventInfo.m_strExpireDate.empty())
    {
        len += snprintf(sql + len, size - len, ", expire_date = '%s'", eventInfo.m_strExpireDate.c_str());
        blModified = true;
    }

    if (!eventInfo.m_strProcessState.empty())
    {
        len += snprintf(sql + len, size - len, ", process_state = '%s'", eventInfo.m_strProcessState.c_str());
        blModified = true;
    }

    if (eventInfo.m_uiViewState != UNUSED_INPUT_UINT)
    {
        len += snprintf(sql + len, size - len, ", view_state = %d", eventInfo.m_uiViewState);
        blModified = true;
    }

    if (!eventInfo.m_uiTypeList.empty())
    {
        ModifyEventType(eventInfo.m_strEventID, eventInfo.m_uiTypeList);
    }

    if (!eventInfo.m_strHandlerList.empty())
    {
        ModifyEventHandler(eventInfo.m_strEventID, eventInfo.m_strHandlerList);
    }

    if (!eventInfo.m_strRemark.empty())
    {
        AddEventRemark(eventInfo.m_strEventID, eventInfo.m_strUserID, eventInfo.m_strRemark);
    }

    if (!blModified)
    {
        LOG_INFO_RLD("ModifyEvent completed, event info is not changed");
        return;
    }

    snprintf(sql + len, size - len, " where event_id = '%s'", eventInfo.m_strEventID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("ModifyEvent exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyEventType(const std::string &strEventID, const std::list<unsigned int> &typeList)
{
    DeleteEventType(strEventID);

    for (auto type : typeList)
    {
        AddEventType(strEventID, type);
    }
}

void PassengerFlowManager::ModifyEventHandler(const std::string &strEventID, const std::list<std::string> &handlerList)
{
    DeleteEventHandler(strEventID);

    for (auto &handler : handlerList)
    {
        AddEventUserAssociation(strEventID, handler, "handler");
    }
}

void PassengerFlowManager::DeleteEventType(const std::string &strEventID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_event_type where event_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEventID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteEventType exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::DeleteEventHandler(const std::string &strEventID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_event_user_association where event_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEventID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteEventHandler exec sql failed, sql is " << sql);
    }
}

bool PassengerFlowManager::QueryEventInfo(const std::string &strEventID, PassengerFlowProtoHandler::Event &eventInfo)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select event_id, source, submit_date, expire_date, user_id, device_id, process_state, view_state, create_date from"
        " t_event_info where event_id = '%s' and state = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strEventID.c_str());

    PassengerFlowProtoHandler::Event rstEvent;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstEvent.m_strEventID = strColumn;
            break;
        case 1:
            rstEvent.m_strSource = strColumn;
            break;
        case 2:
            rstEvent.m_strSubmitDate = strColumn;
            break;
        case 3:
            rstEvent.m_strExpireDate = strColumn;
            break;
        case 4:
            rstEvent.m_strUserID = strColumn;
            break;
        case 5:
            rstEvent.m_strDeviceID = strColumn;
            break;
        case 6:
            rstEvent.m_strProcessState = strColumn;
            break;
        case 7:
            rstEvent.m_uiViewState = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 8:
            rstEvent.m_strCreateDate = strColumn;
            result = rstEvent;
            break;

        default:
            LOG_ERROR_RLD("QueryEventInfo sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryEventInfo exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryEventInfo sql result is empty, sql is " << sql);
        return false;
    }

    auto event = boost::any_cast<PassengerFlowProtoHandler::Event>(ResultList.front());
    eventInfo.m_strEventID = event.m_strEventID;
    eventInfo.m_strSource = event.m_strSource;
    eventInfo.m_strSubmitDate = event.m_strSubmitDate;
    eventInfo.m_strExpireDate = event.m_strExpireDate;
    eventInfo.m_strUserID = event.m_strUserID;
    eventInfo.m_strDeviceID = event.m_strDeviceID;
    eventInfo.m_strProcessState = event.m_strProcessState;
    eventInfo.m_uiViewState = event.m_uiViewState;
    eventInfo.m_strCreateDate = event.m_strCreateDate;

    if (!QueryEventType(eventInfo.m_strEventID, eventInfo.m_uiTypeList))
    {
        LOG_ERROR_RLD("QueryEventInfo failed, query event type error, event id is " << eventInfo.m_strEventID);
        return false;
    }

    if (!QueryEventUserAssociation(eventInfo.m_strEventID, eventInfo.m_strHandlerList))
    {
        LOG_ERROR_RLD("QueryEventInfo failed, query event user association error, event id is " << eventInfo.m_strEventID);
        return false;
    }

    if (!QueryEventRemark(eventInfo.m_strEventID, eventInfo.m_strRemark))
    {
        LOG_ERROR_RLD("QueryEventInfo failed, query event remark error, event id is " << eventInfo.m_strEventID);
        return false;
    }

    if (!QueryStoreByEvent(eventInfo.m_strSource, eventInfo.m_uiTypeList.front(), eventInfo.m_strStoreName, eventInfo.m_strStoreID))
    {
        LOG_ERROR_RLD("QueryStoreByEvent failed, query store id and name error, event id is" << eventInfo.m_strEventID);
        //return false;
    }

    return true;
}

bool PassengerFlowManager::QueryEventType(const std::string &strEventID, std::list<unsigned int> &typeList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select event_type from t_event_type where event_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEventID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = boost::lexical_cast<unsigned int>(strColumn);
            break;

        default:
            LOG_ERROR_RLD("QueryEventType sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryEventType exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        typeList.push_back(boost::any_cast<unsigned int>(result));
    }

    return true;
}

bool PassengerFlowManager::QueryEventUserAssociation(const std::string &strEventID, std::list<std::string> &handlerList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select user_id, user_role from t_event_user_association where event_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEventID.c_str());

    struct ResultUser
    {
        std::string strUserID;
        std::string strUserRole;
    } rstUser;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstUser.strUserID = strColumn;
            break;
        case 1:
            rstUser.strUserRole = strColumn;
            result = rstUser;
            break;

        default:
            LOG_ERROR_RLD("QueryEventUserAssociation sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryEventUserAssociation exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        handlerList.push_back((boost::any_cast<ResultUser>(result)).strUserID);
    }

    return true;
}

bool PassengerFlowManager::QueryEventRemark(const std::string &strEventID, std::string &strRemark)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select create_date, user_id, remark from t_event_remark where event_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEventID.c_str());

    std::list<std::string> rstList;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstList.push_back(strColumn);
            break;
        case 1:
            rstList.push_back(strColumn);
            break;
        case 2:
            rstList.push_back(strColumn);
            result = rstList;
            rstList.clear();
            break;

        default:
            LOG_ERROR_RLD("QueryEventRemark sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryEventRemark exec sql failed, sql is " << sql);
        return false;
    }

    Json::Value root;
    Json::FastWriter writer;
    for (auto &result : ResultList)
    {
        auto remarkList = boost::any_cast<std::list<std::string>>(result);
        Json::Value value;
        for (auto &remark : remarkList) value.append(remark);
        root.append(value);
    }
    strRemark = writer.write(root);

    return true;
}

bool PassengerFlowManager::QueryAllEvent(const std::string &strUserID, const std::string &strProcessState, std::list<PassengerFlowProtoHandler::Event> &eventList,
    const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 10*/)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select * from"
        " (select a.event_id, a.source, a.submit_date, a.expire_date, a.user_id, a.device_id, a.process_state, a.view_state"
        " from t_event_info a where a.user_id = '%s' and a.state = 0 union all"
        " select b.event_id, b.source, b.submit_date, b.expire_date, b.user_id, b.device_id, b.process_state, b.view_state"
        " from t_event_info b join t_event_user_association c on b.event_id = c.event_id where b.state = 0 and c.user_id = '%s') d"
        " where 1 = 1";
    int len = snprintf(sql, size, sqlfmt, strUserID.c_str(), strUserID.c_str());

    if (strProcessState == "0" || strProcessState == "1")
    {
        len += snprintf(sql + len, size - len, " and d.process_state = '%s'", strProcessState.c_str());
    }

    if (!strBeginDate.empty())
    {
        len += snprintf(sql + len, size - len, " and d.submit_date >= '%s'", strBeginDate.c_str());
    }

    if (!strEndDate.empty())
    {
        len += snprintf(sql + len, size - len, " and d.submit_date <= '%s'", strEndDate.c_str());
    }

    snprintf(sql + len, size - len, " order by d.submit_date desc limit %d, %d", uiBeginIndex, uiPageSize);

    return QueryEventSub(std::string(sql), eventList);
}

bool PassengerFlowManager::QueryCreatedEvent(const std::string &strUserID, const std::string &strProcessState, std::list<PassengerFlowProtoHandler::Event> &eventList,
    const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 10*/)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select event_id, source, submit_date, expire_date, user_id, device_id, process_state, view_state"
        " from t_event_info where user_id = '%s' and state = 0";
    int len = snprintf(sql, size, sqlfmt, strUserID.c_str());

    if (strProcessState == "0" || strProcessState == "1")
    {
        len += snprintf(sql + len, size - len, " and process_state = '%s'", strProcessState.c_str());
    }

    if (!strBeginDate.empty())
    {
        len += snprintf(sql + len, size - len, " and submit_date >= '%s'", strBeginDate.c_str());
    }

    if (!strEndDate.empty())
    {
        len += snprintf(sql + len, size - len, " and submit_date <= '%s'", strEndDate.c_str());
    }

    snprintf(sql + len, size - len, " order by submit_date desc limit %d, %d", uiBeginIndex, uiPageSize);

    return QueryEventSub(std::string(sql), eventList);
}

bool PassengerFlowManager::QueryHandledEvent(const std::string &strUserID, const std::string &strProcessState, std::list<PassengerFlowProtoHandler::Event> &eventList,
    const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 10*/)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = " select a.event_id, a.source, a.submit_date, a.expire_date, a.user_id, a.device_id, a.process_state, a.view_state"
        " from t_event_info a join t_event_user_association b on a.event_id = b.event_id"
        " where b.user_id = '%s' and a.state = 0";
    int len = snprintf(sql, size, sqlfmt, strUserID.c_str(), strUserID.c_str());

    if (strProcessState == "0" || strProcessState == "1")
    {
        len += snprintf(sql + len, size - len, " and a.process_state = '%s'", strProcessState.c_str());
    }

    if (!strBeginDate.empty())
    {
        len += snprintf(sql + len, size - len, " and a.submit_date >= '%s'", strBeginDate.c_str());
    }

    if (!strEndDate.empty())
    {
        len += snprintf(sql + len, size - len, " and a.submit_date <= '%s'", strEndDate.c_str());
    }

    snprintf(sql + len, size - len, " order by a.submit_date desc limit %d, %d", uiBeginIndex, uiPageSize);

    return QueryEventSub(std::string(sql), eventList);
}

bool PassengerFlowManager::QueryEventSub(const std::string &strSql, std::list<PassengerFlowProtoHandler::Event> &eventList)
{
    PassengerFlowProtoHandler::Event rstEvent;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstEvent.m_strEventID = strColumn;
            break;
        case 1:
            rstEvent.m_strSource = strColumn;
            break;
        case 2:
            rstEvent.m_strSubmitDate = strColumn;
            break;
        case 3:
            rstEvent.m_strExpireDate = strColumn;
            break;
        case 4:
            rstEvent.m_strUserID = strColumn;
            break;
        case 5:
            rstEvent.m_strDeviceID = strColumn;
            break;
        case 6:
            rstEvent.m_strProcessState = strColumn;
            break;
        case 7:
            rstEvent.m_uiViewState = boost::lexical_cast<unsigned int>(strColumn);
            result = rstEvent;
            break;

        default:
            LOG_ERROR_RLD("QueryEventSub sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(strSql, ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryEventSub exec sql failed, sql is " << strSql);
        return false;
    }

    for (auto result : ResultList)
    {
        auto event = boost::any_cast<PassengerFlowProtoHandler::Event>(result);
        std::list<unsigned int> type;
        QueryEventType(event.m_strEventID, event.m_uiTypeList);

        std::list<std::string> handler;
        QueryEventUserAssociation(event.m_strEventID, event.m_strHandlerList);

        eventList.push_back(std::move(event));
    }

    return true;
}

void PassengerFlowManager::AddSmartGuardStore(const std::string &strUserID, const std::string &strStoreID,
    const PassengerFlowProtoHandler::SmartGuardStore &smartGuardStore)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_smart_guard_store_plan"
        " (id, plan_id, user_id, store_id, plan_name, enable, begin_time, end_time, begin_time2, end_time2, create_date)"
        " values(uuid(), '%s', '%s', '%s', '%s', '%s', '%s', '%s', %s, %s, '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, smartGuardStore.m_strPlanID.c_str(), strUserID.c_str(), strStoreID.c_str(), smartGuardStore.m_strPlanName.c_str(),
        smartGuardStore.m_strEnable.c_str(), smartGuardStore.m_strBeginTime.c_str(), smartGuardStore.m_strEndTime.c_str(),
        smartGuardStore.m_strBeginTime2.empty() ? "null" : ("'" + smartGuardStore.m_strBeginTime2 + "'").c_str(),
        smartGuardStore.m_strEndTime2.empty() ? "null" : ("'" + smartGuardStore.m_strEndTime2 + "'").c_str(), smartGuardStore.m_strCreateDate.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddSmartGuardStore exec sql failed, sql is " << sql);
    }

    for (auto &entrance : smartGuardStore.m_strEntranceIDList)
    {
        AddGuardStoreEntranceAssociation(smartGuardStore.m_strPlanID, entrance);
    }
}

void PassengerFlowManager::AddGuardStoreEntranceAssociation(const std::string &strPlanID, const std::string &strEntranceID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_guard_plan_entrance_association (id, plan_id, entrance_id, create_date)"
        " values(uuid(), '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strPlanID.c_str(), strEntranceID.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddGuardStoreEntranceAssociation exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::DeleteSmartGuardStore(const std::string &strPlanID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete a, b from"
        " t_smart_guard_store_plan a left join t_guard_plan_entrance_association b on a.plan_id = b.plan_id"
        " where a.plan_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPlanID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteSmartGuardStore exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifySmartGuardStore(const PassengerFlowProtoHandler::SmartGuardStore &smartGuardStore)
{
    bool blModified = false;

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len = snprintf(sql, size, "update t_smart_guard_store_plan set id = id");

    if (!smartGuardStore.m_strPlanName.empty())
    {
        len += snprintf(sql + len, size - len, ", plan_name = '%s'", smartGuardStore.m_strPlanName.c_str());
        blModified = true;
    }

    if (!smartGuardStore.m_strEnable.empty())
    {
        len += snprintf(sql + len, size - len, ", enable = '%s'", smartGuardStore.m_strEnable.c_str());
        blModified = true;
    }

    if (!smartGuardStore.m_strBeginTime.empty())
    {
        len += snprintf(sql + len, size - len, ", begin_time = '%s'", smartGuardStore.m_strBeginTime.c_str());
        blModified = true;
    }

    if (!smartGuardStore.m_strEndTime.empty())
    {
        len += snprintf(sql + len, size - len, ", end_time = '%s'", smartGuardStore.m_strEndTime.c_str());
        blModified = true;
    }

    if (!smartGuardStore.m_strBeginTime2.empty())
    {
        len += snprintf(sql + len, size - len, ", begin_time2 = '%s'", smartGuardStore.m_strBeginTime2.c_str());
        blModified = true;
    }

    if (!smartGuardStore.m_strEndTime2.empty())
    {
        len += snprintf(sql + len, size - len, ", end_time2 = '%s'", smartGuardStore.m_strEndTime2.c_str());
        blModified = true;
    }

    if (!smartGuardStore.m_strEntranceIDList.empty())
    {
        ModifyGuardStoreEntranceAssociation(smartGuardStore.m_strPlanID, smartGuardStore.m_strEntranceIDList);
    }

    if (!blModified)
    {
        LOG_INFO_RLD("ModifySmartGuardStore completed, smart guard sotre info is not changed");
        return;
    }

    snprintf(sql + len, size - len, " where plan_id = '%s'", smartGuardStore.m_strPlanID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("ModifySmartGuardStore exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyGuardStoreEntranceAssociation(const std::string &strPlanID, const std::list<std::string> &entranceList)
{
    DeleteGuardStoreEntranceAssociation(strPlanID);

    for (auto &entrance : entranceList)
    {
        AddGuardStoreEntranceAssociation(strPlanID, entrance);
    }
}

void PassengerFlowManager::DeleteGuardStoreEntranceAssociation(const std::string &strPlanID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_guard_plan_entrance_association where plan_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPlanID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteGuardStoreEntranceAssociation exec sql failed, sql is " << sql);
    }
}

bool PassengerFlowManager::QuerySmartGuardStoreInfo(const std::string &strPlanID, PassengerFlowProtoHandler::SmartGuardStore &smartGuardStore)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select b.store_id, b.store_name, a.plan_name, a.enable, a.begin_time, a.end_time, ifnull(a.begin_time2, ''), ifnull(a.end_time2, '')"
        " from t_smart_guard_store_plan a join t_store_info b on a.store_id = b.store_id"
        " where a.plan_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPlanID.c_str());

    PassengerFlowProtoHandler::SmartGuardStore rstGuardStore;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstGuardStore.m_strStoreID = strColumn;
            break;
        case 1:
            rstGuardStore.m_strStoreName = strColumn;
            break;
        case 2:
            rstGuardStore.m_strPlanName = strColumn;
            break;
        case 3:
            rstGuardStore.m_strEnable = strColumn;
            break;
        case 4:
            rstGuardStore.m_strBeginTime = strColumn;
            break;
        case 5:
            rstGuardStore.m_strEndTime = strColumn;
            break;
        case 6:
            rstGuardStore.m_strBeginTime2 = strColumn;
            break;
        case 7:
            rstGuardStore.m_strEndTime2 = strColumn;
            result = rstGuardStore;
            break;

        default:
            LOG_ERROR_RLD("QuerySmartGuardStoreInfo sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QuerySmartGuardStoreInfo exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QuerySmartGuardStoreInfo sql result is empty, sql is " << sql);
        return false;
    }

    auto guardPlan = boost::any_cast<PassengerFlowProtoHandler::SmartGuardStore>(ResultList.front());
    smartGuardStore.m_strStoreID = guardPlan.m_strStoreID;
    smartGuardStore.m_strStoreName = guardPlan.m_strStoreName;
    smartGuardStore.m_strPlanName = guardPlan.m_strPlanName;
    smartGuardStore.m_strEnable = guardPlan.m_strEnable;
    smartGuardStore.m_strBeginTime = guardPlan.m_strBeginTime;
    smartGuardStore.m_strEndTime = guardPlan.m_strEndTime;
    smartGuardStore.m_strBeginTime2 = guardPlan.m_strBeginTime2;
    smartGuardStore.m_strEndTime2 = guardPlan.m_strEndTime2;

    return QueryGuardStoreEntranceAssociation(strPlanID, smartGuardStore.m_strEntranceIDList);
}

bool PassengerFlowManager::QueryGuardStoreEntranceAssociation(const std::string &strPlanID, std::list<std::string> &strEntranceIDList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select entrance_id from t_guard_plan_entrance_association where plan_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPlanID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = strColumn;
            break;

        default:
            LOG_ERROR_RLD("QueryGuardStoreEntranceAssociation sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryGuardStoreEntranceAssociation exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        strEntranceIDList.push_back(boost::any_cast<std::string>(result));
    }

    return true;
}

bool PassengerFlowManager::QueryAllSmartGuardStoreByUser(const std::string &strUserID, std::list<PassengerFlowProtoHandler::SmartGuardStore> &smartGuardStoreList,
    const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 10*/)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select b.store_id, b.store_name, a.plan_id, a.plan_name, a.enable, a.begin_time, a.end_time, ifnull(a.begin_time2, ''), ifnull(a.end_time2, '')"
        " from t_smart_guard_store_plan a join t_store_info b on a.store_id = b.store_id"
        " where a.user_id = '%s' limit %d, %d";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), uiBeginIndex, uiPageSize);//TODO,不做对指定用户的过滤，对用户角色做过滤

    PassengerFlowProtoHandler::SmartGuardStore rstGuardStore;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstGuardStore.m_strStoreID = strColumn;
            break;
        case 1:
            rstGuardStore.m_strStoreName = strColumn;
            break;
        case 2:
            rstGuardStore.m_strPlanID = strColumn;
            break;
        case 3:
            rstGuardStore.m_strPlanName = strColumn;
            break;
        case 4:
            rstGuardStore.m_strEnable = strColumn;
            break;
        case 5:
            rstGuardStore.m_strBeginTime = strColumn;
            break;
        case 6:
            rstGuardStore.m_strEndTime = strColumn;
            break;
        case 7:
            rstGuardStore.m_strBeginTime2 = strColumn;
            break;
        case 8:
            rstGuardStore.m_strEndTime2 = strColumn;
            result = rstGuardStore;
            break;

        default:
            LOG_ERROR_RLD("QueryAllSmartGuardStoreByUser sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryAllSmartGuardStoreByUser exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        smartGuardStoreList.push_back(std::move(boost::any_cast<PassengerFlowProtoHandler::SmartGuardStore>(result)));
    }

    return true;
}

bool PassengerFlowManager::QueryAllSmartGuardStoreByDevice(const std::string &strDeviceID, std::list<PassengerFlowProtoHandler::SmartGuardStore> &smartGuardStoreList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select d.store_id, d.store_name, c.plan_id, c.plan_name, c.enable, c.begin_time, c.end_time, ifnull(c.begin_time2, ''), ifnull(c.end_time2, '')"
        " from t_entrance_device_association a join t_guard_plan_entrance_association b on a.entrance_id = b.entrance_id"
        " join t_smart_guard_store_plan c on b.plan_id = c.plan_id"
        " join t_store_info d on c.store_id = d.store_id"
        " where a.device_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strDeviceID.c_str());

    PassengerFlowProtoHandler::SmartGuardStore rstGuardStore;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstGuardStore.m_strStoreID = strColumn;
            break;
        case 1:
            rstGuardStore.m_strStoreName = strColumn;
            break;
        case 2:
            rstGuardStore.m_strPlanID = strColumn;
            break;
        case 3:
            rstGuardStore.m_strPlanName = strColumn;
            break;
        case 4:
            rstGuardStore.m_strEnable = strColumn;
            break;
        case 5:
            rstGuardStore.m_strBeginTime = strColumn;
            break;
        case 6:
            rstGuardStore.m_strEndTime = strColumn;
            break;
        case 7:
            rstGuardStore.m_strBeginTime2 = strColumn;
            break;
        case 8:
            rstGuardStore.m_strEndTime2 = strColumn;
            result = rstGuardStore;
            break;

        default:
            LOG_ERROR_RLD("QueryAllSmartGuardStoreByDevice sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryAllSmartGuardStoreByDevice exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        smartGuardStoreList.push_back(std::move(boost::any_cast<PassengerFlowProtoHandler::SmartGuardStore>(result)));
    }

    return true;
}

void PassengerFlowManager::AddRegularPatrol(const std::string &strUserID, const PassengerFlowProtoHandler::RegularPatrol &regularPatrol)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_regular_patrol_plan"
        " (id, plan_id, user_id, plan_name, enable, last_update_time, create_date)"
        " values(uuid(), '%s', '%s', '%s', '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, regularPatrol.m_strPlanID.c_str(), strUserID.c_str(), regularPatrol.m_strPlanName.c_str(),
        regularPatrol.m_strEnable.c_str(), regularPatrol.m_strCreateDate.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddRegularPatrol exec sql failed, sql is " << sql);
    }

    for (auto &store : regularPatrol.m_storeEntranceList)
    {
        for (auto &entrance : store.m_entranceList)
        {
            AddPatrolPlanEntranceAssociation(regularPatrol.m_strPlanID, entrance.m_strEntranceID);
        }
    }

    for (auto &time : regularPatrol.m_strPatrolTimeList)
    {
        AddRegularPatrolTime(regularPatrol.m_strPlanID, time);
    }

    for (auto &handler : regularPatrol.m_strHandlerList)
    {
        AddPatrolPlanUserAssociation(regularPatrol.m_strPlanID, handler, "handler");
    }
}

void PassengerFlowManager::AddPatrolPlanEntranceAssociation(const std::string &strPlanID, const std::string &strEntranceID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_patrol_plan_entrance_association (id, plan_id, entrance_id, create_date)"
        " values(uuid(), '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strPlanID.c_str(), strEntranceID.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddPatrolPlanEntranceAssociation exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::AddPatrolPlanUserAssociation(const std::string &strPlanID, const std::string &strUserID, const std::string &strRole)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_patrol_plan_user_association (id, plan_id, user_id, user_role, create_date)"
        " values(uuid(), '%s', '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strPlanID.c_str(), strUserID.c_str(), strRole.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddPatrolPlanUserAssociation exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::AddRegularPatrolTime(const std::string &strPlanID, const std::string &strTime)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_regular_patrol_time (id, plan_id, patrol_time, create_date)"
        " values(uuid(), '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strPlanID.c_str(), strTime.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddRegularPatrolTime exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::DeleteRegularPatrol(const std::string &strPlanID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete a, b, c, d from"
        " t_regular_patrol_plan a left join t_patrol_plan_entrance_association b on a.plan_id = b.plan_id"
        " left join t_regular_patrol_time c on a.plan_id = c.plan_id"
        " left join t_patrol_plan_user_association d on a.plan_id = d.plan_id"
        " where a.plan_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPlanID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteRegularPatrol exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyRegularPatrol(const PassengerFlowProtoHandler::RegularPatrol &regularPatrol)
{
    bool blModified = false;

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len = snprintf(sql, size, "update t_regular_patrol_plan set id = id");

    if (!regularPatrol.m_strPlanName.empty())
    {
        len += snprintf(sql + len, size - len, ", plan_name = '%s'", regularPatrol.m_strPlanName.c_str());
        blModified = true;
    }

    if (!regularPatrol.m_strEnable.empty())
    {
        len += snprintf(sql + len, size - len, ", enable = '%s'", regularPatrol.m_strEnable.c_str());
        blModified = true;
    }

    if (!regularPatrol.m_storeEntranceList.empty())
    {
        ModifyPatrolPlanEntranceAssociation(regularPatrol.m_strPlanID, regularPatrol.m_storeEntranceList);
    }

    if (!regularPatrol.m_strPatrolTimeList.empty())
    {
        ModifyRegularPatrolTime(regularPatrol.m_strPlanID, regularPatrol.m_strPatrolTimeList);
    }

    if (!regularPatrol.m_strHandlerList.empty())
    {
        ModifyPatrolPlanUserAssociation(regularPatrol.m_strPlanID, regularPatrol.m_strHandlerList);
    }

    if (!blModified)
    {
        LOG_INFO_RLD("ModifyRegularPatrol completed, regular patrol plan info is not changed");
        return;
    }

    snprintf(sql + len, size - len, " where plan_id = '%s'", regularPatrol.m_strPlanID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("ModifyRegularPatrol exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyPatrolPlanEntranceAssociation(const std::string &strPlanID, const std::list<PassengerFlowProtoHandler::PatrolStoreEntrance> &storeEntranceList)
{
    DeletePatrolPlanEntranceAssociation(strPlanID);

    for (auto &storeEntrance : storeEntranceList)
    {
        for (auto &entrance : storeEntrance.m_entranceList)
        {
            AddPatrolPlanEntranceAssociation(strPlanID, entrance.m_strEntranceID);
        }
    }
}

void PassengerFlowManager::ModifyPatrolPlanUserAssociation(const std::string &strPlanID, const std::list<std::string> &strUserIDList)
{
    DeletePatrolPlanUserAssociation(strPlanID);

    for (auto &user : strUserIDList)
    {
        AddPatrolPlanUserAssociation(strPlanID, user, "handler");
    }
}

void PassengerFlowManager::DeletePatrolPlanEntranceAssociation(const std::string &strPlanID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_patrol_plan_entrance_association where plan_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPlanID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeletePatrolPlanEntranceAssociation exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::DeletePatrolPlanUserAssociation(const std::string &strPlanID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_patrol_plan_user_association where plan_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPlanID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeletePatrolPlanUserAssociation exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyRegularPatrolTime(const std::string &strPlanID, const std::list<std::string> &timeList)
{
    DeleteRegularPatrolTime(strPlanID);

    for (auto &time : timeList)
    {
        AddRegularPatrolTime(strPlanID, time);
    }
}

void PassengerFlowManager::DeleteRegularPatrolTime(const std::string &strPlanID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_regular_patrol_time where plan_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPlanID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteRegularPatrolTime exec sql failed, sql is " << sql);
    }
}

bool PassengerFlowManager::QueryRegularPatrolInfo(const std::string &strPlanID, PassengerFlowProtoHandler::RegularPatrol &regularPatrol)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select plan_name, enable, create_date, update_date from t_regular_patrol_plan where plan_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPlanID.c_str());

    PassengerFlowProtoHandler::RegularPatrol rstGuardStore;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstGuardStore.m_strPlanName = strColumn;
            break;
        case 1:
            rstGuardStore.m_strEnable = strColumn;
            break;
        case 2:
            rstGuardStore.m_strCreateDate = strColumn;
            break;
        case 3:
            rstGuardStore.m_strUpdateDate = strColumn;
            result = rstGuardStore;
            break;

        default:
            LOG_ERROR_RLD("QueryRegularPatrolInfo sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryRegularPatrolInfo exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryRegularPatrolInfo sql result is empty, sql is " << sql);
        return false;
    }

    auto patrolPlan = boost::any_cast<PassengerFlowProtoHandler::RegularPatrol>(ResultList.front());
    regularPatrol.m_strPlanName = patrolPlan.m_strPlanName;
    regularPatrol.m_strEnable = patrolPlan.m_strEnable;
    regularPatrol.m_strCreateDate = patrolPlan.m_strCreateDate;
    regularPatrol.m_strUpdateDate = patrolPlan.m_strUpdateDate;

    return QueryPatrolPlanEntranceAssociation(strPlanID, regularPatrol.m_storeEntranceList)
        && QueryRegularPatrolTime(strPlanID, regularPatrol.m_strPatrolTimeList)
        && QueryPatrolPlanUserAssociation(strPlanID, regularPatrol.m_strHandlerList);
}

bool PassengerFlowManager::QueryPatrolPlanEntranceAssociation(const std::string &strPlanID, std::list<PassengerFlowProtoHandler::PatrolStoreEntrance> &storeEntranceList,
    const std::string &strEntranceID /*= std::string()*/)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select b.entrance_id, b.entrance_name, c.store_id, c.store_name from"
        " t_patrol_plan_entrance_association a join t_entrance_info b on a.entrance_id = b.entrance_id"
        " join t_store_info c on b.store_id = c.store_id"
        " where a.plan_id = '%s'";
    int len = snprintf(sql, size, sqlfmt, strPlanID.c_str());

    if (!strEntranceID.empty())
    {
        snprintf(sql + len, size - len, " and a.entrance_id = '%s'", strEntranceID.c_str());
    }

    PassengerFlowProtoHandler::Entrance rstEntrance;
    PassengerFlowProtoHandler::PatrolStoreEntrance rstStoreEntrance;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstEntrance.m_strEntranceID = strColumn;
            break;
        case 1:
            rstEntrance.m_strEntranceName = strColumn;
            break;
        case 2:
            rstStoreEntrance.m_strStoreID = strColumn;
            rstStoreEntrance.m_entranceList.push_back(rstEntrance);
            break;
        case 3:
            rstStoreEntrance.m_strStoreName = strColumn;
            result = rstStoreEntrance;
            rstStoreEntrance.m_entranceList.clear();
            break;

        default:
            LOG_ERROR_RLD("QueryPatrolPlanEntranceAssociation sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryPatrolPlanEntranceAssociation exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        auto storeEntrance = boost::any_cast<PassengerFlowProtoHandler::PatrolStoreEntrance>(result);
        auto it = storeEntranceList.begin();
        auto end = storeEntranceList.end();
        for (; it != end; ++it)
        {
            if (it->m_strStoreID == storeEntrance.m_strStoreID)
            {
                it->m_entranceList.push_back(storeEntrance.m_entranceList.front());
                break;
            }
        }

        if (it == end)
        {
            storeEntranceList.push_back(storeEntrance);
        }
    }

    return true;
}

bool PassengerFlowManager::QueryPatrolPlanUserAssociation(const std::string &strPlanID, std::list<std::string> &strUserIDList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select user_id, user_role from t_patrol_plan_user_association where plan_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPlanID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = strColumn;
            break;
        case 1:
            break;

        default:
            LOG_ERROR_RLD("QueryPatrolPlanUserAssociation sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryPatrolPlanUserAssociation exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        strUserIDList.push_back(boost::any_cast<std::string>(result));
    }

    return true;
}

bool PassengerFlowManager::QueryRegularPatrolTime(const std::string &strPlanID, std::list<std::string> &strTimeList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select patrol_time from t_regular_patrol_time where plan_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPlanID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = strColumn;
            if (strPlanID == "15EAFE46A3EF7D4F973EEEA37BA2ACB5") //TODO,配合设备调试
            {
                result = CurrentTime().erase(0, 11);
            }
            break;

        default:
            LOG_ERROR_RLD("QueryRegularPatrolTime sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryRegularPatrolTime exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        strTimeList.push_back(boost::any_cast<std::string>(result));
    }

    return true;
}

bool PassengerFlowManager::QueryAllRegularPatrolByUser(const std::string &strUserID, std::list<PassengerFlowProtoHandler::RegularPatrol> &regularPatrolList,
    const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 10*/)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select plan_id, plan_name, enable, create_date, update_date from t_regular_patrol_plan";//" where user_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt);
    //snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str());

    PassengerFlowProtoHandler::RegularPatrol rstGuardStore;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstGuardStore.m_strPlanID = strColumn;
            break;
        case 1:
            rstGuardStore.m_strPlanName = strColumn;
            break;
        case 2:
            rstGuardStore.m_strEnable = strColumn;
            break;
        case 3:
            rstGuardStore.m_strCreateDate = strColumn;
            break;
        case 4:
            rstGuardStore.m_strUpdateDate = strColumn;
            result = rstGuardStore;
            break;

        default:
            LOG_ERROR_RLD("QueryAllRegularPatrolByUser sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryAllRegularPatrolByUser exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        auto plan = boost::any_cast<PassengerFlowProtoHandler::RegularPatrol>(result);
        QueryPatrolPlanEntranceAssociation(plan.m_strPlanID, plan.m_storeEntranceList);
        QueryRegularPatrolTime(plan.m_strPlanID, plan.m_strPatrolTimeList);
        QueryPatrolPlanUserAssociation(plan.m_strPlanID, plan.m_strHandlerList);

        regularPatrolList.push_back(plan);
    }

    return true;
}

bool PassengerFlowManager::QueryAllRegularPatrolByDevice(const std::string &strDeviceID, std::list<PassengerFlowProtoHandler::RegularPatrol> &regularPatrolList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select a.entrance_id, c.plan_id, c.plan_name, c.enable from"
        " t_entrance_device_association a join t_patrol_plan_entrance_association b on a.entrance_id = b.entrance_id"
        " join t_regular_patrol_plan c on b.plan_id = c.plan_id"
        " where a.device_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strDeviceID.c_str());

    struct ResultGuardStore
    {
        std::string strEntranceID;
        PassengerFlowProtoHandler::RegularPatrol regularPatrol;
    } rstGuardStore;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstGuardStore.strEntranceID = strColumn;
            break;
        case 1:
            rstGuardStore.regularPatrol.m_strPlanID = strColumn;
            break;
        case 2:
            rstGuardStore.regularPatrol.m_strPlanName = strColumn;
            break;
        case 3:
            rstGuardStore.regularPatrol.m_strEnable = strColumn;
            result = rstGuardStore;
            break;

        default:
            LOG_ERROR_RLD("QueryAllRegularPatrolByDevice sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryAllRegularPatrolByDevice exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        auto plan = boost::any_cast<ResultGuardStore>(result);
        QueryPatrolPlanEntranceAssociation(plan.regularPatrol.m_strPlanID, plan.regularPatrol.m_storeEntranceList, plan.strEntranceID);
        QueryRegularPatrolTime(plan.regularPatrol.m_strPlanID, plan.regularPatrol.m_strPatrolTimeList);
        QueryPatrolPlanUserAssociation(plan.regularPatrol.m_strPlanID, plan.regularPatrol.m_strHandlerList);

        regularPatrolList.push_back(plan.regularPatrol);
    }

    return true;
}

void PassengerFlowManager::UserJoinStore(const std::string &strUserID, const std::string &strStoreID, const std::string &strRole)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_user_store_association (id, user_id, store_id, user_role, create_date)"
        " values (uuid(), '%s', '%s', '%s', '%s') on duplicate key update update_date = current_time";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), strStoreID.c_str(), strRole.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("UserJoinStore exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::UserQuitStore(const std::string &strUserID, const std::string &strStoreID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_user_store_association where user_id = '%s' and store_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), strStoreID.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("UserQuitStore exec sql failed, sql is " << sql);
    }
}

bool PassengerFlowManager::QueryStoreAllUser(const std::string &strStoreID, std::list<PassengerFlowProtoHandler::UserBrief> &userList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select a.user_id, b.aliasname, a.user_role from"
        " t_user_store_association a join PlatformDB.t_user_info b on a.user_id = b.userid"
        " where store_id = '%s' and b.status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strStoreID.c_str());

    PassengerFlowProtoHandler::UserBrief user;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            user.m_strUserID = strColumn;
            break;
        case 1:
            user.m_strUserName = strColumn;
            break;
        case 2:
            user.m_strRole = strColumn;
            result = user;
            break;

        default:
            LOG_ERROR_RLD("QueryStoreAllUser sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryStoreAllUser exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        userList.push_back(std::move(boost::any_cast<PassengerFlowProtoHandler::UserBrief>(result)));
    }

    return true;
}

bool PassengerFlowManager::QueryCompanyAllUser(const std::string &strCompanyID, std::list<PassengerFlowProtoHandler::UserBrief> &userList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select a.user_id, b.aliasname from"
        " t_company_user_info a join PlatformDB.t_user_info b on a.user_id = b.userid"
        " where company_id = '%s' and b.status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strCompanyID.c_str());

    PassengerFlowProtoHandler::UserBrief user;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            user.m_strUserID = strColumn;
            break;
        case 1:
            user.m_strUserName = strColumn;
            result = user;
            break;

        default:
            LOG_ERROR_RLD("QueryCompanyAllUser sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryCompanyAllUser exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        userList.push_back(std::move(boost::any_cast<PassengerFlowProtoHandler::UserBrief>(result)));
    }

    return true;
}

void PassengerFlowManager::AddVIPCustomer(const PassengerFlowProtoHandler::VIPCustomer &customerInfo)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_vip_customer_info"
        " (id, vip_id, profile_picture, vip_name, cellphone, visit_date, visit_times, register_date)"
        " values(uuid(), '%s', '%s', '%s', '%s', '%s', %d, '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, customerInfo.m_strVIPID.c_str(), customerInfo.m_strProfilePicture.c_str(),
        customerInfo.m_strVIPName.c_str(), customerInfo.m_strCellphone.c_str(), customerInfo.m_strVisitDate.c_str(),
        customerInfo.m_uiVisitTimes, CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddVIPCustomer exec sql failed, sql is " << sql);
    }

    for (auto &consume : customerInfo.m_consumeHistoryList)
    {
        AddVIPConsumeHistory(consume);
    }
}

void PassengerFlowManager::DeleteVIPCustomer(const std::string &strVIPID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete a, b from"
        " t_vip_customer_info a left join t_vip_consume_history b on a.vip_id = b.vip_id"
        " where a.vip_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strVIPID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteVIPCustomer exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyVIPCustomer(const PassengerFlowProtoHandler::VIPCustomer &customerInfo)
{
    bool blModified = false;

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len = snprintf(sql, size, "update t_vip_customer_info set id = id");

    if (!customerInfo.m_strProfilePicture.empty())
    {
        len += snprintf(sql + len, size - len, ", profile_picture = '%s'", customerInfo.m_strProfilePicture.c_str());
        blModified = true;
    }

    if (!customerInfo.m_strVIPName.empty())
    {
        len += snprintf(sql + len, size - len, ", vip_name = '%s'", customerInfo.m_strVIPName.c_str());
        blModified = true;
    }

    if (!customerInfo.m_strCellphone.empty())
    {
        len += snprintf(sql + len, size - len, ", cellphone = '%s'", customerInfo.m_strCellphone.c_str());
        blModified = true;
    }

    if (!customerInfo.m_strVisitDate.empty())
    {
        len += snprintf(sql + len, size - len, ", visit_date = '%s'", customerInfo.m_strVisitDate.c_str());
        blModified = true;
    }

    if (customerInfo.m_uiVisitTimes != UNUSED_INPUT_UINT)
    {
        len += snprintf(sql + len, size - len, ", visit_times = %d", customerInfo.m_uiVisitTimes);
        blModified = true;
    }

    if (!blModified)
    {
        LOG_INFO_RLD("ModifyVIPCustomer completed, vip customer info is not changed");
        return;
    }

    snprintf(sql + len, size - len, " where vip_id = '%s'", customerInfo.m_strVIPID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("ModifyVIPCustomer exec sql failed, sql is " << sql);
    }
}

bool PassengerFlowManager::QueryVIPCustomerInfo(const std::string &strVIPID, PassengerFlowProtoHandler::VIPCustomer &customerInfo)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select profile_picture, vip_name, cellphone, visit_date, visit_times, register_date from"
        " t_vip_customer_info where vip_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strVIPID.c_str());

    PassengerFlowProtoHandler::VIPCustomer rstCustomer;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstCustomer.m_strProfilePicture = strColumn;
            break;
        case 1:
            rstCustomer.m_strVIPName = strColumn;
            break;
        case 2:
            rstCustomer.m_strCellphone = strColumn;
            break;
        case 3:
            rstCustomer.m_strVisitDate = strColumn;
            break;
        case 4:
            rstCustomer.m_uiVisitTimes = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 5:
            rstCustomer.m_strRegisterDate = strColumn;
            result = rstCustomer;
            break;

        default:
            LOG_ERROR_RLD("QueryVIPCustomerInfo sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryVIPCustomerInfo exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryVIPCustomerInfo sql result is empty, sql is " << sql);
        return false;
    }

    auto customer = boost::any_cast<PassengerFlowProtoHandler::VIPCustomer>(ResultList.front());
    customerInfo.m_strProfilePicture = customer.m_strProfilePicture;
    customerInfo.m_strVIPName = customer.m_strVIPName;
    customerInfo.m_strCellphone = customer.m_strCellphone;
    customerInfo.m_strVisitDate = customer.m_strVisitDate;
    customerInfo.m_uiVisitTimes = customer.m_uiVisitTimes;
    customerInfo.m_strRegisterDate = customer.m_strRegisterDate;

    return QueryAllVIPConsumeHistory(strVIPID, customerInfo.m_consumeHistoryList);
}

bool PassengerFlowManager::QueryAllVIPCustomer(std::list<PassengerFlowProtoHandler::VIPCustomer> &customerInfoList,
    const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 10*/)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select vip_id, profile_picture, vip_name, cellphone, visit_date, visit_times, register_date"
        " from t_vip_customer_info limit %d, %d";
    snprintf(sql, sizeof(sql), sqlfmt, uiBeginIndex, uiPageSize);

    PassengerFlowProtoHandler::VIPCustomer rstCustomer;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstCustomer.m_strVIPID = strColumn;
            break;
        case 1:
            rstCustomer.m_strProfilePicture = strColumn;
            break;
        case 2:
            rstCustomer.m_strVIPName = strColumn;
            break;
        case 3:
            rstCustomer.m_strCellphone = strColumn;
            break;
        case 4:
            rstCustomer.m_strVisitDate = strColumn;
            break;
        case 5:
            rstCustomer.m_uiVisitTimes = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 6:
            rstCustomer.m_strRegisterDate = strColumn;
            result = rstCustomer;
            break;

        default:
            LOG_ERROR_RLD("QueryAllVIPCustomerByUser sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryAllVIPCustomerByUser exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        customerInfoList.push_back(std::move(boost::any_cast<PassengerFlowProtoHandler::VIPCustomer>(result)));
    }

    return true;
}

void PassengerFlowManager::AddVIPConsumeHistory(const PassengerFlowProtoHandler::VIPConsumeHistory &consumeHistory)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_vip_consume_history (id, consume_id, vip_id, goods_name, goods_number, salesman, consume_amount, consume_date)"
        " values(uuid(), '%s', '%s', '%s', %d, '%s', %f, '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, consumeHistory.m_strConsumeID.c_str(), consumeHistory.m_strVIPID.c_str(), consumeHistory.m_strGoodsName.c_str(),
        consumeHistory.m_uiGoodsNumber, consumeHistory.m_strSalesman.c_str(), consumeHistory.m_dConsumeAmount, consumeHistory.m_strConsumeDate.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddVIPConsumeHistory exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::DeleteVIPConsumeHistory(const std::string &strVIPID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_vip_consume_history where consume_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strVIPID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteVIPConsumeHistory exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyVIPConsumeHistory(const PassengerFlowProtoHandler::VIPConsumeHistory &consumeHistory)
{
    bool blModified = false;

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len = snprintf(sql, size, "update t_vip_consume_history set id = id");

    if (!consumeHistory.m_strGoodsName.empty())
    {
        len += snprintf(sql + len, size - len, ", goods_name = '%s'", consumeHistory.m_strGoodsName.c_str());
        blModified = true;
    }

    if (consumeHistory.m_uiGoodsNumber != UNUSED_INPUT_UINT)
    {
        len += snprintf(sql + len, size - len, ", goods_number = %d", consumeHistory.m_uiGoodsNumber);
        blModified = true;
    }

    if (!consumeHistory.m_strSalesman.empty())
    {
        len += snprintf(sql + len, size - len, ", salesman = '%s'", consumeHistory.m_strSalesman.c_str());
        blModified = true;
    }

    if (consumeHistory.m_dConsumeAmount >= 0.0)
    {
        len += snprintf(sql + len, size - len, ", consume_amount = %f", consumeHistory.m_dConsumeAmount);
        blModified = true;
    }

    if (!consumeHistory.m_strConsumeDate.empty())
    {
        len += snprintf(sql + len, size - len, ", consume_date = '%s'", consumeHistory.m_strConsumeDate.c_str());
        blModified = true;
    }

    if (!blModified)
    {
        LOG_INFO_RLD("ModifyVIPConsumeHistory completed, vip consume history info is not changed");
        return;
    }

    snprintf(sql + len, size - len, " where consume_id = '%s'", consumeHistory.m_strConsumeID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("ModifyVIPConsumeHistory exec sql failed, sql is " << sql);
    }
}

bool PassengerFlowManager::QueryAllVIPConsumeHistory(const std::string &strVIPID, std::list<PassengerFlowProtoHandler::VIPConsumeHistory> &consumeHistoryList,
    const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 10*/)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select consume_id, goods_name, goods_number, salesman, consume_amount, consume_date from"
        " t_vip_consume_history where vip_id = '%s' order by consume_date desc limit %d, %d";
    snprintf(sql, sizeof(sql), sqlfmt, strVIPID.c_str(), uiBeginIndex, uiPageSize);

    PassengerFlowProtoHandler::VIPConsumeHistory rstConsumeHistory;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstConsumeHistory.m_strConsumeID = strColumn;
            break;
        case 1:
            rstConsumeHistory.m_strGoodsName = strColumn;
            break;
        case 2:
            rstConsumeHistory.m_uiGoodsNumber = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 3:
            rstConsumeHistory.m_strSalesman = strColumn;
            break;
        case 4:
            rstConsumeHistory.m_dConsumeAmount = boost::lexical_cast<double>(strColumn);
            break;
        case 5:
            rstConsumeHistory.m_strConsumeDate = strColumn;
            result = rstConsumeHistory;
            break;

        default:
            LOG_ERROR_RLD("QueryAllVIPConsumeHistory sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryAllVIPConsumeHistory exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        consumeHistoryList.push_back(std::move(boost::any_cast<PassengerFlowProtoHandler::VIPConsumeHistory>(result)));
    }

    return true;
}

void PassengerFlowManager::AddEvaluationTemplate(const PassengerFlowProtoHandler::EvaluationItem &evaluationItem)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_evaluation_item (id, item_id, item_name, description, total_point, create_date)"
        " values(uuid(), '%s', '%s', '%s', %f, '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, evaluationItem.m_strItemID.c_str(), evaluationItem.m_strItemName.c_str(),
        evaluationItem.m_strDescription.c_str(), evaluationItem.m_dTotalPoint, CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddEvaluationTemplate exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::DeleteEvaluationTemplate(const std::string &strItemID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_evaluation_item where item_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strItemID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteEvaluationTemplate exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyEvaluationTemplate(const PassengerFlowProtoHandler::EvaluationItem &evaluationItem)
{
    bool blModified = false;

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len = snprintf(sql, size, "update t_evaluation_item set id = id");

    if (!evaluationItem.m_strItemName.empty())
    {
        len += snprintf(sql + len, size - len, ", item_name = '%s'", evaluationItem.m_strItemName.c_str());
        blModified = true;
    }

    if (!evaluationItem.m_strDescription.empty())
    {
        len += snprintf(sql + len, size - len, ", description = '%s'", evaluationItem.m_strDescription.c_str());
        blModified = true;
    }

    if (evaluationItem.m_dTotalPoint >= 0.0)
    {
        len += snprintf(sql + len, size - len, ", total_point = %f", evaluationItem.m_dTotalPoint);
        blModified = true;
    }

    if (!blModified)
    {
        LOG_INFO_RLD("ModifyEvaluationTemplate completed, evaluation item info is not changed");
        return;
    }

    snprintf(sql + len, size - len, " where item_id = '%s'", evaluationItem.m_strItemID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("ModifyEvaluationTemplate exec sql failed, sql is " << sql);
    }
}

bool PassengerFlowManager::QueryAllEvaluationTemplate(std::list<PassengerFlowProtoHandler::EvaluationItem> &evaluationItemList,
    const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 10*/)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select item_id, item_name, description, total_point from t_evaluation_item";
    snprintf(sql, sizeof(sql), sqlfmt);

    PassengerFlowProtoHandler::EvaluationItem rstEvaluationItem;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstEvaluationItem.m_strItemID = strColumn;
            break;
        case 1:
            rstEvaluationItem.m_strItemName = strColumn;
            break;
        case 2:
            rstEvaluationItem.m_strDescription = strColumn;
            break;
        case 3:
            rstEvaluationItem.m_dTotalPoint = boost::lexical_cast<double>(strColumn);
            result = rstEvaluationItem;
            break;

        default:
            LOG_ERROR_RLD("QueryAllEvaluationTemplate sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryAllEvaluationTemplate exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        evaluationItemList.push_back(std::move(boost::any_cast<PassengerFlowProtoHandler::EvaluationItem>(result)));
    }

    return true;
}

void PassengerFlowManager::AddStoreEvaluation(const PassengerFlowProtoHandler::StoreEvaluation &storeEvaluation)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_store_evaluation (id, evaluation_id, store_id, user_id_create, user_id_check, check_status, create_date)"
        " values(uuid(), '%s', '%s', '%s', '%s', %d, '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, storeEvaluation.m_strEvaluationID.c_str(), storeEvaluation.m_strStoreID.c_str(),
        storeEvaluation.m_strUserIDCreate.c_str(), storeEvaluation.m_strUserIDCheck.c_str(), storeEvaluation.m_uiCheckStatus, CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddStoreEvaluation exec sql failed, sql is " << sql);
    }

    for (auto it = storeEvaluation.m_itemScoreList.begin(), end = storeEvaluation.m_itemScoreList.end(); it != end; ++it)
    {
        AddStoreEvaluationScore(storeEvaluation.m_strEvaluationID, *it);
    }

    for (auto it = storeEvaluation.m_strPictureList.begin(), end = storeEvaluation.m_strPictureList.end(); it != end; ++it)
    {
        AddStoreEvaluationPicture(storeEvaluation.m_strEvaluationID, *it);
    }
}

void PassengerFlowManager::AddStoreEvaluationScore(const std::string &strEvaluationID, const PassengerFlowProtoHandler::EvaluationItemScore &itemScore)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_store_evaluation_score (id, evaluation_id, item_id, score, description, create_date)"
        " values(uuid(), '%s', '%s', %f, '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strEvaluationID.c_str(), itemScore.m_evaluationItem.m_strItemID.c_str(), itemScore.m_dScore,
        itemScore.m_strDescription.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddStoreEvaluationScore exec sql failed, sql is " << sql);
    }

    for (auto it = itemScore.m_strPictureList.begin(), end = itemScore.m_strPictureList.end(); it != end; ++it)
    {
        AddStoreEvaluationItemPicture(strEvaluationID, itemScore.m_evaluationItem.m_strItemID, *it);
    }
}

void PassengerFlowManager::AddStoreEvaluationItemPicture(const std::string &strEvaluationID, const std::string &strItemID, const std::string &strPicture)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_evaluation_item_screenshot (id, evaluation_id, item_id, screenshot_id, create_date)"
        " values(uuid(), '%s', '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strEvaluationID.c_str(), strItemID.c_str(), strPicture.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddStoreEvaluationItemPicture exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::AddStoreEvaluationPicture(const std::string &strEvaluationID, const std::string &strPicture)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_store_evaluation_screenshot (id, evaluation_id, screenshot_id, create_date)"
        " values(uuid(), '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strEvaluationID.c_str(), strPicture.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddStoreEvaluationPicture exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::DeleteStoreEvaluation(const std::string &strEvaluationID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete a, b, c, d from"
        " t_store_evaluation a left join t_store_evaluation_score b on a.evaluation_id = b.evaluation_id"
        " left join t_evaluation_item_screenshot c on a.evaluation_id = c.evaluation_id"
        " left join t_store_evaluation_screenshot d on a.evaluation_id = d.evaluation_id"
        " where a.evaluation_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEvaluationID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteStoreEvaluation exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyStoreEvaluation(const PassengerFlowProtoHandler::StoreEvaluation &storeEvaluation)
{
    bool blModified = false;

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len = snprintf(sql, size, "update t_store_evaluation set id = id");

    if (!storeEvaluation.m_strUserIDCheck.empty())
    {
        len += snprintf(sql + len, size - len, ", user_id_check = '%s'", storeEvaluation.m_strUserIDCheck.c_str());
        blModified = true;
    }

    if (storeEvaluation.m_uiCheckStatus != UNUSED_INPUT_UINT)
    {
        len += snprintf(sql + len, size - len, ", check_status = %d", storeEvaluation.m_uiCheckStatus);
        blModified = true;
    }

    if (!storeEvaluation.m_itemScoreList.empty())
    {
        ModifyStoreEvaluationScore(storeEvaluation.m_strEvaluationID, storeEvaluation.m_itemScoreList);
    }

    if (!storeEvaluation.m_strPictureList.empty())
    {
        ModifyStoreEvaluationPicture(storeEvaluation.m_strEvaluationID, storeEvaluation.m_strPictureList);
    }

    if (!blModified)
    {
        LOG_INFO_RLD("ModifyStoreEvaluation completed, store evaluation info is not changed");
        return;
    }

    snprintf(sql + len, size - len, " where evaluation_id = '%s'", storeEvaluation.m_strEvaluationID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("ModifyStoreEvaluation exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyStoreEvaluationScore(const std::string &strEvaluationID, const std::list<PassengerFlowProtoHandler::EvaluationItemScore> &scoreList)
{
    DeleteStoreEvaluationScore(strEvaluationID);

    for (auto &score : scoreList)
    {
        AddStoreEvaluationScore(strEvaluationID, score);
        ModifyStoreEvaluationItemPicture(strEvaluationID, score.m_evaluationItem.m_strItemID, score.m_strPictureList);
    }
}

void PassengerFlowManager::DeleteStoreEvaluationScore(const std::string &strEvaluationID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_store_evaluation_score where evaluation_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEvaluationID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteStoreEvaluationScore exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyStoreEvaluationItemPicture(const std::string &strEvaluationID, const std::string &strItemID, const std::list<std::string> &strPictureList)
{
    DeleteStoreEvaluationItemPicture(strEvaluationID, strItemID);

    for (auto &picture : strPictureList)
    {
        AddStoreEvaluationItemPicture(strEvaluationID, strItemID, picture);
    }
}

void PassengerFlowManager::DeleteStoreEvaluationItemPicture(const std::string &strEvaluationID, const std::string &strItemID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_evaluation_item_screenshot where evaluation_id = '%s' and item_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEvaluationID.c_str(), strItemID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteStoreEvaluationItemPicture exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyStoreEvaluationPicture(const std::string &strEvaluationID, const std::list<std::string> &strPictureList)
{
    DeleteStoreEvaluationPicture(strEvaluationID);

    for (auto &picture : strPictureList)
    {
        AddStoreEvaluationPicture(strEvaluationID, picture);
    }
}

void PassengerFlowManager::DeleteStoreEvaluationPicture(const std::string &strEvaluationID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_store_evaluation_screenshot where evaluation_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEvaluationID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteStoreEvaluationPicture exec sql failed, sql is " << sql);
    }
}

bool PassengerFlowManager::QueryStoreEvaluationInfo(const std::string &strEvaluationID, PassengerFlowProtoHandler::StoreEvaluation &storeEvaluation)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select user_id_create, user_id_check, check_status, create_date"
        " from t_store_evaluation where evaluation_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEvaluationID.c_str());

    PassengerFlowProtoHandler::StoreEvaluation rstStoreEvaluation;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstStoreEvaluation.m_strUserIDCreate = strColumn;
            break;
        case 1:
            rstStoreEvaluation.m_strUserIDCheck = strColumn;
            break;
        case 2:
            rstStoreEvaluation.m_uiCheckStatus = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 3:
            rstStoreEvaluation.m_strCreateDate = strColumn;
            result = rstStoreEvaluation;
            break;

        default:
            LOG_ERROR_RLD("QueryStoreEvaluationInfo sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryStoreEvaluationInfo exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryStoreEvaluationInfo sql result is empty, sql is " << sql);
        return false;
    }

    auto evaluation = boost::any_cast<PassengerFlowProtoHandler::StoreEvaluation>(ResultList.front());
    storeEvaluation.m_strUserIDCreate = evaluation.m_strUserIDCreate;
    storeEvaluation.m_strUserIDCheck = evaluation.m_strUserIDCheck;
    storeEvaluation.m_uiCheckStatus = evaluation.m_uiCheckStatus;
    storeEvaluation.m_strCreateDate = evaluation.m_strCreateDate;

    return QueryStoreEvaluationScore(strEvaluationID, storeEvaluation.m_dTotalScore, storeEvaluation.m_itemScoreList)
        && QueryStoreEvaluationPicture(strEvaluationID, storeEvaluation.m_strPictureList);
}

bool PassengerFlowManager::QueryStoreEvaluationScore(const std::string &strEvaluationID, double &dTotalScore,
    std::list<PassengerFlowProtoHandler::EvaluationItemScore> &scoreList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select a.score, a.description, b.item_id, b.item_name, b.description, b.total_point from"
        " t_store_evaluation_score a join t_evaluation_item b on a.item_id = b.item_id"
        " where a.evaluation_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEvaluationID.c_str());

    PassengerFlowProtoHandler::EvaluationItemScore rstScore;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstScore.m_dScore = boost::lexical_cast<double>(strColumn);
            break;
        case 1:
            rstScore.m_strDescription = strColumn;
            break;
        case 2:
            rstScore.m_evaluationItem.m_strItemID = strColumn;
            break;
        case 3:
            rstScore.m_evaluationItem.m_strItemName = strColumn;
            break;
        case 4:
            rstScore.m_evaluationItem.m_strDescription = strColumn;
            break;
        case 5:
            rstScore.m_evaluationItem.m_dTotalPoint = boost::lexical_cast<double>(strColumn);
            result = rstScore;
            break;

        default:
            LOG_ERROR_RLD("QueryStoreEvaluationScore sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryStoreEvaluationScore exec sql failed, sql is " << sql);
        return false;
    }

    dTotalScore = 0.0;
    for (auto &result : ResultList)
    {
        auto score = boost::any_cast<PassengerFlowProtoHandler::EvaluationItemScore>(result);
        dTotalScore += score.m_dScore;

        QueryStoreEvaluationItemPicture(strEvaluationID, score.m_evaluationItem.m_strItemID, score.m_strPictureList);

        scoreList.push_back(std::move(score));
    }

    return true;
}

bool PassengerFlowManager::QueryStoreEvaluationItemPicture(const std::string &strEvaluationID, const std::string &strItemID, std::list<std::string> &strPictureList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select screenshot_id from t_evaluation_item_screenshot"
        " where evaluation_id = '%s' and item_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEvaluationID.c_str(), strItemID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = m_ParamInfo.m_strFileServerURL + "download_file&fileid=" + strColumn;
            break;

        default:
            LOG_ERROR_RLD("QueryStoreEvaluationItemPicture sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryStoreEvaluationItemPicture exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        strPictureList.push_back(boost::any_cast<std::string>(result));
    }

    return true;
}

bool PassengerFlowManager::QueryStoreEvaluationPicture(const std::string &strEvaluationID, std::list<std::string> &strPictureList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select screenshot_id from t_store_evaluation_screenshot"
        " where evaluation_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEvaluationID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = m_ParamInfo.m_strFileServerURL + "download_file&fileid=" + strColumn;
            break;

        default:
            LOG_ERROR_RLD("QueryStoreEvaluationPicture sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryStoreEvaluationPicture exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        strPictureList.push_back(boost::any_cast<std::string>(result));
    }

    return true;
}

bool PassengerFlowManager::QueryAllStoreEvaluation(const std::string &strStoreID, std::list<PassengerFlowProtoHandler::StoreEvaluation> &storeEvaluationList,
    const unsigned int uiCheckStatus, const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 10*/)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select a.evaluation_id, a.user_id_create, a.user_id_check, a.check_status, a.create_date, sum(b.score), sum(c.total_point) from"
        " t_store_evaluation a join t_store_evaluation_score b on a.evaluation_id = b.evaluation_id"
        " join t_evaluation_item c on b.item_id = c.item_id"
        " where a.store_id = '%s'";

    int len = snprintf(sql, size, sqlfmt, strStoreID.c_str());

    if (uiCheckStatus != UNUSED_INPUT_UINT)
    {
        len += snprintf(sql + len, size - len, " and a.check_status = %d", uiCheckStatus);
    }

    if (!strBeginDate.empty())
    {
        len += snprintf(sql + len, size - len, " and a.create_date >= '%s'", strBeginDate.c_str());
    }

    if (!strEndDate.empty())
    {
        len += snprintf(sql + len, size - len, " and a.create_date <= '%s'", strEndDate.c_str());
    }

    snprintf(sql + len, size - len, " group by a.evaluation_id, a.user_id_create, a.user_id_check, a.check_status, a.create_date limit %d, %d", uiBeginIndex, uiPageSize);

    PassengerFlowProtoHandler::StoreEvaluation rstStoreEvaluation;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstStoreEvaluation.m_strEvaluationID = strColumn;
            break;
        case 1:
            rstStoreEvaluation.m_strUserIDCreate = strColumn;
            break;
        case 2:
            rstStoreEvaluation.m_strUserIDCheck = strColumn;
            break;
        case 3:
            rstStoreEvaluation.m_uiCheckStatus = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 4:
            rstStoreEvaluation.m_strCreateDate = strColumn;
            break;
        case 5:
            rstStoreEvaluation.m_dTotalScore = boost::lexical_cast<double>(strColumn);
            break;
        case 6:
            rstStoreEvaluation.m_dTotalPoint = boost::lexical_cast<double>(strColumn);
            result = rstStoreEvaluation;
            break;

        default:
            LOG_ERROR_RLD("QueryAllStoreEvaluation sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryAllStoreEvaluation exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        storeEvaluationList.push_back(std::move(boost::any_cast<PassengerFlowProtoHandler::StoreEvaluation>(result)));
    }

    return true;
}

void PassengerFlowManager::AddRemotePatrolStore(const PassengerFlowProtoHandler::RemotePatrolStore &patrolStore)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_remote_patrol_store (id, patrol_id, user_id, device_id, store_id, plan_id, patrol_date, patrol_result, description, create_date)"
        " values(uuid(), '%s', '%s', '%s', '%s', '%s', '%s', %d, '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, patrolStore.m_strPatrolID.c_str(), patrolStore.m_strUserID.c_str(), patrolStore.m_strDeviceID.c_str(),
        patrolStore.m_strStoreID.c_str(), patrolStore.m_strPlanID.c_str(), patrolStore.m_strPatrolDate.c_str(),
        patrolStore.m_uiPatrolResult, patrolStore.m_strDescription.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddRemotePatrolStore exec sql failed, sql is " << sql);
    }

    if (patrolStore.m_strEntranceIDList.empty())  //设备创建巡店记录
    {
        std::string entranceID;
        QueryDeviceBoundEntrance(patrolStore.m_strDeviceID, entranceID);
        AddRemotePatrolStoreEntrance(patrolStore.m_strPatrolID, entranceID);

        for (auto it = patrolStore.m_strPatrolPictureList.begin(), end = patrolStore.m_strPatrolPictureList.end(); it != end; ++it)
        {
            AddRemotePatrolStoreScreenshot(patrolStore.m_strPatrolID, entranceID, *it);
        }
    }
    else
    {
        for (auto it = patrolStore.m_patrolPictureList.begin(), end = patrolStore.m_patrolPictureList.end(); it != end; ++it)
        {
            AddRemotePatrolStoreEntrance(patrolStore.m_strPatrolID, it->m_strEntranceID);

            for (auto it2 = it->m_strPatrolPictureList.begin(), end2 = it->m_strPatrolPictureList.end(); it2 != end2; ++it2)
            {
                AddRemotePatrolStoreScreenshot(patrolStore.m_strPatrolID, it->m_strEntranceID, *it2);
            }
        }
    }
}

void PassengerFlowManager::AddRemotePatrolStoreEntrance(const std::string &strPatrolID, const std::string &strEntranceID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_patrol_store_entrance_association (id, patrol_id, entrance_id, create_date)"
        " values(uuid(), '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strPatrolID.c_str(), strEntranceID.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddRemotePatrolStoreEntrance exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::AddRemotePatrolStoreScreenshot(const std::string &strPatrolID, const std::string &strEntranceID, const std::string &strScreenshot)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_patrol_store_screenshot_association (id, patrol_id, entrance_id, screenshot_id, create_date)"
        " values(uuid(), '%s', '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strPatrolID.c_str(), strEntranceID.c_str(), strScreenshot.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddRemotePatrolStoreScreenshot exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::DeleteRemotePatrolStore(const std::string &strPatrolID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete a, b, c from"
        " t_remote_patrol_store a left join t_patrol_store_entrance_association b on a.patrol_id = b.patrol_id"
        " left join t_patrol_store_screenshot_association c on a.patrol_id = c.patrol_id"
        " where a.patrol_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPatrolID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteRemotePatrolStore exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyRemotePatrolStore(const PassengerFlowProtoHandler::RemotePatrolStore &patrolStore)
{
    bool blModified = false;

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len = snprintf(sql, size, "update t_remote_patrol_store set id = id");

    if (!patrolStore.m_strPatrolDate.empty())
    {
        len += snprintf(sql + len, size - len, ", patrol_date = '%s'", patrolStore.m_strPatrolDate.c_str());
        blModified = true;
    }

    if (patrolStore.m_uiPatrolResult != UNUSED_INPUT_UINT)
    {
        len += snprintf(sql + len, size - len, ", patrol_result = %d", patrolStore.m_uiPatrolResult);
        blModified = true;
    }

    if (!patrolStore.m_strDescription.empty())
    {
        len += snprintf(sql + len, size - len, ", description = '%s'", patrolStore.m_strDescription.c_str());
        blModified = true;
    }

    if (!patrolStore.m_patrolPictureList.empty())
    {
        DeleteRemotePatrolStoreScreenshot(patrolStore.m_strPatrolID);
        for (auto it = patrolStore.m_patrolPictureList.begin(), end = patrolStore.m_patrolPictureList.end(); it != end; ++it)
        {
            ModifyRemotePatrolStoreScreenshot(patrolStore.m_strPatrolID, it->m_strEntranceID, it->m_strPatrolPictureList);
        }
    }

    if (!blModified)
    {
        LOG_INFO_RLD("ModifyRemotePatrolStore completed, remote patrol store info is not changed");
        return;
    }

    snprintf(sql + len, size - len, " where patrol_id = '%s'", patrolStore.m_strPatrolID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("ModifyRemotePatrolStore exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyRemotePatrolStoreScreenshot(const std::string &strPatrolID, const std::string &strEntranceID, const std::list<std::string> &screenshotList)
{
    for (auto &screenshot : screenshotList)
    {
        AddRemotePatrolStoreScreenshot(strPatrolID, strEntranceID, screenshot);
    }
}

void PassengerFlowManager::DeleteRemotePatrolStoreScreenshot(const std::string &strPatrolID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_patrol_store_screenshot_association where patrol_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPatrolID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteRemotePatrolStoreScreenshot exec sql failed, sql is " << sql);
    }
}

bool PassengerFlowManager::QueryRemotePatrolStoreInfo(const std::string &strPatrolID, PassengerFlowProtoHandler::RemotePatrolStore &patrolStore)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select user_id, device_id, store_id, plan_id, patrol_date, patrol_result, description"
        " from t_remote_patrol_store where patrol_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPatrolID.c_str());

    PassengerFlowProtoHandler::RemotePatrolStore rstPatrolStore;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstPatrolStore.m_strUserID = strColumn;
            break;
        case 1:
            rstPatrolStore.m_strDeviceID = strColumn;
            break;
        case 2:
            rstPatrolStore.m_strStoreID = strColumn;
            break;
        case 3:
            rstPatrolStore.m_strPlanID = strColumn;
            break;
        case 4:
            rstPatrolStore.m_strPatrolDate = strColumn;
            break;
        case 5:
            rstPatrolStore.m_uiPatrolResult = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 6:
            rstPatrolStore.m_strDescription = strColumn;
            result = rstPatrolStore;
            break;

        default:
            LOG_ERROR_RLD("QueryRemotePatrolStoreInfo sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryRemotePatrolStoreInfo exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryRemotePatrolStoreInfo sql result is empty, sql is " << sql);
        return false;
    }

    auto patrol = boost::any_cast<PassengerFlowProtoHandler::RemotePatrolStore>(ResultList.front());
    patrolStore.m_strUserID = patrol.m_strUserID;
    patrolStore.m_strDeviceID = patrol.m_strDeviceID;
    patrolStore.m_strStoreID = patrol.m_strStoreID;
    patrolStore.m_strPlanID = patrol.m_strPlanID;
    patrolStore.m_uiPatrolResult = patrol.m_uiPatrolResult;
    patrolStore.m_strPatrolDate = patrol.m_strPatrolDate;
    patrolStore.m_strDescription = patrol.m_strDescription;

    QueryRemotePatrolStoreScreenshot(strPatrolID, patrolStore.m_patrolPictureList);
    for (auto it = patrolStore.m_patrolPictureList.begin(), end = patrolStore.m_patrolPictureList.end(); it != end; ++it)
    {
        patrolStore.m_strEntranceIDList.push_back(it->m_strEntranceID);
    }

    return true;
}

bool PassengerFlowManager::QueryRemotePatrolStoreEntrance(const std::string &strPatrolID, std::list<std::string> &entranceList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select entrance_id from t_patrol_store_entrance_association where patrol_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPatrolID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = strColumn;
            break;

        default:
            LOG_ERROR_RLD("QueryRemotePatrolStoreEntrance sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryRemotePatrolStoreEntrance exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        entranceList.push_back(boost::any_cast<std::string>(result));
    }

    return true;
}

bool PassengerFlowManager::QueryRemotePatrolStoreScreenshot(const std::string &strPatrolID, std::list<PassengerFlowProtoHandler::EntrancePicture> &screenshotList)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select entrance_id, screenshot_id from t_patrol_store_screenshot_association where patrol_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPatrolID.c_str());

    PassengerFlowProtoHandler::EntrancePicture rstScreenshot;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstScreenshot.m_strEntranceID = strColumn;
            break;
        case 1:
            rstScreenshot.m_strPatrolPictureList.push_back(m_ParamInfo.m_strFileServerURL + "download_file&fileid=" + strColumn);
            result = rstScreenshot;
            rstScreenshot.m_strPatrolPictureList.clear();
            break;

        default:
            LOG_ERROR_RLD("QueryRemotePatrolStoreScreenshot sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryRemotePatrolStoreScreenshot exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        auto screenshot = boost::any_cast<PassengerFlowProtoHandler::EntrancePicture>(result);
        auto it = screenshotList.begin();
        auto end = screenshotList.end();
        for (; it != end; ++it)
        {
            if (it->m_strEntranceID == screenshot.m_strEntranceID)
            {
                it->m_strPatrolPictureList.push_back(screenshot.m_strPatrolPictureList.front());
                break;
            }
        }

        if (it == end)
        {
            screenshotList.push_back(screenshot);
        }
    }

    return true;
}

bool PassengerFlowManager::QueryAllRemotePatrolStore(const std::string &strStoreID, const std::string &strPlanID, const unsigned int uiPatrolResult, const unsigned int uiPlanFlag,
    std::list<PassengerFlowProtoHandler::RemotePatrolStore> &patrolStoreList, const std::string &strBeginDate, const std::string &strEndDate,
    const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 12*/)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select patrol_id, user_id, device_id, store_id, plan_id, patrol_date, patrol_result, description from"
        " t_remote_patrol_store where 1 = 1";
    int len = snprintf(sql, size, sqlfmt);

    if (!strStoreID.empty())
    {
        len += snprintf(sql + len, size - len, " and store_id = '%s'", strStoreID.c_str());
    }

    if (uiPatrolResult != UNUSED_INPUT_UINT)
    {
        len += snprintf(sql + len, size - len, " and patrol_result = %d", uiPatrolResult);
    }

    if (uiPlanFlag != UNUSED_INPUT_UINT)
    {
        len += snprintf(sql + len, size - len, " and plan_id %s", uiPlanFlag == 0 ? "= ''" : "!= ''");
    }

    if (uiPlanFlag == 1 && !strPlanID.empty())
    {
        len += snprintf(sql + len, size - len, " and plan_id = '%s'", strPlanID.c_str());
    }

    if (!strBeginDate.empty())
    {
        len += snprintf(sql + len, size - len, " and patrol_date >= '%s'", strBeginDate.c_str());
    }

    if (!strEndDate.empty())
    {
        len += snprintf(sql + len, size - len, " and patrol_date <= '%s'", strEndDate.c_str());
    }

    snprintf(sql + len, size - len, " order by patrol_date desc limit %d, %d", uiBeginIndex, uiPageSize);

    PassengerFlowProtoHandler::RemotePatrolStore rstPatrolStore;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstPatrolStore.m_strPatrolID = strColumn;
            break;
        case 1:
            rstPatrolStore.m_strUserID = strColumn;
            break;
        case 2:
            rstPatrolStore.m_strDeviceID = strColumn;
            break;
        case 3:
            rstPatrolStore.m_strStoreID = strColumn;
            break;
        case 4:
            rstPatrolStore.m_strPlanID = strColumn;
            break;
        case 5:
            rstPatrolStore.m_strPatrolDate = strColumn;
            break;
        case 6:
            rstPatrolStore.m_uiPatrolResult = boost::lexical_cast<unsigned int>(strColumn);
            break;
        case 7:
            rstPatrolStore.m_strDescription = strColumn;
            result = rstPatrolStore;
            break;

        default:
            LOG_ERROR_RLD("QueryAllRemotePatrolStore sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryAllRemotePatrolStore exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        auto patrolStore = boost::any_cast<PassengerFlowProtoHandler::RemotePatrolStore>(result);
        QueryRemotePatrolStoreScreenshot(patrolStore.m_strPatrolID, patrolStore.m_patrolPictureList);
        for (auto it = patrolStore.m_patrolPictureList.begin(), end = patrolStore.m_patrolPictureList.end(); it != end; ++it)
        {
            patrolStore.m_strEntranceIDList.push_back(it->m_strEntranceID);
        }

        patrolStoreList.push_back(std::move(patrolStore));
    }

    return true;
}

void PassengerFlowManager::AddStoreSensor(const PassengerFlowProtoHandler::Sensor &sensorInfo)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_store_sensor (id, store_id, sensor_id, device_id, sensor_name, sensor_type, sensor_alarm_threshold, create_date)"
        " values (uuid(), '%s', '%s', '%s', '%s', '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, sensorInfo.m_strStoreID.c_str(), sensorInfo.m_strSensorID.c_str(), sensorInfo.m_strDeviceID.c_str(),
        sensorInfo.m_strSensorName.c_str(), sensorInfo.m_strSensorType.c_str(), sensorInfo.m_strSensorAlarmThreshold.c_str(), sensorInfo.m_strCreateDate.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddStoreSensor exec sql failed, sql is " << sql);
        return;
    }
}

void PassengerFlowManager::DeleteStoreSensor(const std::string &strSensorID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "delete a, b from"
        " t_store_sensor a left join t_sensor_value b on a.sensor_id = b.sensor_id"
        " where a.sensor_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strSensorID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteStoreSensor exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyStoreSensor(const PassengerFlowProtoHandler::Sensor &sensorInfo)
{
    bool blModified = false;

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    int len = snprintf(sql, size, "update t_store_sensor set id = id");

    if (!sensorInfo.m_strSensorName.empty())
    {
        len += snprintf(sql + len, size - len, ", sensor_name = '%s'", sensorInfo.m_strSensorName.c_str());
        blModified = true;
    }

    if (!sensorInfo.m_strSensorType.empty())
    {
        len += snprintf(sql + len, size - len, ", sensor_type = '%s'", sensorInfo.m_strSensorType.c_str());
        blModified = true;
    }

    if (!sensorInfo.m_strSensorAlarmThreshold.empty())
    {
        len += snprintf(sql + len, size - len, ", sensor_alarm_threshold = '%s'", sensorInfo.m_strSensorAlarmThreshold.c_str());
        blModified = true;
    }
    
    if (!sensorInfo.m_strStoreID.empty())
    {
        len += snprintf(sql + len, size - len, ", store_id = '%s'", sensorInfo.m_strStoreID.c_str());
        blModified = true;
    }

    if (!sensorInfo.m_strDeviceID.empty())
    {
        len += snprintf(sql + len, size - len, ", device_id = '%s'", sensorInfo.m_strDeviceID.c_str());
        blModified = true;
    }

    if (!blModified)
    {
        LOG_INFO_RLD("ModifyStoreSensor completed, sensor info is not changed");
        return;
    }

    snprintf(sql + len, size - len, " where sensor_id = '%s'", sensorInfo.m_strSensorID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("ModifyStoreSensor exec sql failed, sql is " << sql);
    }
}

bool PassengerFlowManager::QueryStoreSensorInfo(const std::string &strSensorID, PassengerFlowProtoHandler::Sensor &sensorInfo)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select a.store_id, a.sensor_id, a.device_id, a.sensor_name, a.sensor_type, ifnull(b.value, ''), a.create_date, a.sensor_alarm_threshold from"
        " t_store_sensor a left join t_sensor_value b on a.sensor_id = b.sensor_id"
        " where a.sensor_id = '%s' order by b.create_date desc limit 0, 1";
    snprintf(sql, sizeof(sql), sqlfmt, strSensorID.c_str());

    PassengerFlowProtoHandler::Sensor rstStoreSensor;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstStoreSensor.m_strStoreID = strColumn;
            break;
        case 1:
            rstStoreSensor.m_strSensorID = strColumn;
            break;
        case 2:
            rstStoreSensor.m_strDeviceID = strColumn;
            break;
        case 3:
            rstStoreSensor.m_strSensorName = strColumn;
            break;
        case 4:
            rstStoreSensor.m_strSensorType = strColumn;
            break;
        case 5:
            rstStoreSensor.m_strValue = strColumn;
            break;
        case 6:
            rstStoreSensor.m_strCreateDate = strColumn;            
            break;
        case 7:
            rstStoreSensor.m_strSensorAlarmThreshold = strColumn;
            result = rstStoreSensor;
            break;

        default:
            LOG_ERROR_RLD("QueryStoreSensorInfo sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryStoreSensorInfo exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryStoreSensorInfo sql result is empty, sql is " << sql);
        return false;
    }

    auto sensor = boost::any_cast<PassengerFlowProtoHandler::Sensor>(ResultList.front());
    sensorInfo.m_strStoreID = sensor.m_strStoreID;
    sensorInfo.m_strSensorID = sensor.m_strSensorID;
    sensorInfo.m_strDeviceID = sensor.m_strDeviceID;
    sensorInfo.m_strSensorName = sensor.m_strSensorName;
    sensorInfo.m_strSensorType = sensor.m_strSensorType;
    sensorInfo.m_strSensorAlarmThreshold = sensor.m_strSensorAlarmThreshold;
    sensorInfo.m_strValue = sensor.m_strValue;
    sensorInfo.m_strCreateDate = sensor.m_strCreateDate;

    return true;
}

bool PassengerFlowManager::QuerySensorValue(const std::string &strSensorID, std::string &strValue)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select value, create_date from t_sensor_value"
        " where sensor_id = '%s' order by create_date desc limit 0, 1";
    snprintf(sql, sizeof(sql), sqlfmt, strSensorID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = strColumn;
            break;
        case 1:
            break;

        default:
            LOG_ERROR_RLD("QuerySensorValue sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QuerySensorValue exec sql failed, sql is " << sql);
        return false;
    }

    if (!ResultList.empty())
    {
        strValue = boost::any_cast<std::string>(ResultList.front());
    }

    return true;
}

bool PassengerFlowManager::QueryAllStoreSensor(const std::string &strStoreID, std::list<PassengerFlowProtoHandler::Sensor> &sensorList,
    const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 10*/)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select store_id, sensor_id, device_id, sensor_name, sensor_type, create_date, sensor_alarm_threshold from t_store_sensor"
        " where store_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strStoreID.c_str());

    PassengerFlowProtoHandler::Sensor rstStoreSensor;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstStoreSensor.m_strStoreID = strColumn;
            break;
        case 1:
            rstStoreSensor.m_strSensorID = strColumn;
            break;
        case 2:
            rstStoreSensor.m_strDeviceID = strColumn;
            break;
        case 3:
            rstStoreSensor.m_strSensorName = strColumn;
            break;
        case 4:
            rstStoreSensor.m_strSensorType = strColumn;
            break;
        case 5:
            rstStoreSensor.m_strCreateDate = strColumn;
            break;
        case 6:
            rstStoreSensor.m_strSensorAlarmThreshold = strColumn;
            result = rstStoreSensor;
            break;

        default:
            LOG_ERROR_RLD("QueryAllStoreSensor sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryAllStoreSensor exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        auto sensor = boost::any_cast<PassengerFlowProtoHandler::Sensor>(result);
        QuerySensorValue(sensor.m_strSensorID, sensor.m_strValue);

        sensorList.push_back(std::move(sensor));
    }

    return true;
}

bool PassengerFlowManager::ValidDeviceSensor(const std::string &strDevID, const unsigned int uiType, const bool IsModify, const std::string &strSensorID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = !IsModify ? "select count(id) from t_store_sensor where device_id = '%s' and sensor_type = %u and state = 0" :
        "select count(id) from t_store_sensor where device_id = '%s' and sensor_type = %u and state = 0 and sensor_id != '%s'";
    if (!IsModify)
    {
        snprintf(sql, sizeof(sql), sqlfmt, strDevID.c_str(), uiType);
    }
    else
    {
        snprintf(sql, sizeof(sql), sqlfmt, strDevID.c_str(), uiType, strSensorID.c_str());
    }
    
    unsigned int uiResult;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        uiResult = boost::lexical_cast<unsigned int>(strColumn);
        Result = uiResult;

        LOG_INFO_RLD("Query sensor valid sql count(id) is " << uiResult);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("IsUserPasswordValid sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("Not found sensor existed, device id is " << strDevID << " and type is " << uiType);
        return true;
    }

    uiResult = boost::any_cast<unsigned int>(ResultList.front());

    if (uiResult > 0)
    {
        LOG_ERROR_RLD("The sensor is invalid, device id is " << strDevID << " and type is " << uiType);
        return false;
    }

    LOG_INFO_RLD("The sensor is valid, device id is " << strDevID << " and type is " << uiType);
    return true;
}

void PassengerFlowManager::ImportPOSData(const std::string &strStoreID, const unsigned int uiOrderAmount,
    const unsigned int uiGoodsAmount, const double dDealAmount, const std::string &strDealDate)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_store_deal_pos (id, store_id, deal_date, order_amount, goods_amount, deal_amount, create_date)"
        " values(uuid(), '%s', '%s', %d, %d, %f, '%s')"
        " on duplicate key update order_amount = %d, goods_amount = %d, deal_amount = %f";
    snprintf(sql, sizeof(sql), sqlfmt, strStoreID.c_str(), strDealDate.c_str(), uiOrderAmount,
        uiGoodsAmount, dDealAmount, CurrentTime().c_str(), uiOrderAmount, uiGoodsAmount, dDealAmount);

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("ImportPOSData exec sql failed, sql is " << sql);
    }
}

bool PassengerFlowManager::QueryCustomerFlowStatistic(const std::string &strStoreID, const std::string &strBeginDate,
    const std::string &strEndDate, const unsigned int uiTimePrecision, std::string &strChartData)
{
    if (uiTimePrecision < 1800)
    {
        return QueryRawCustomerFlow(strStoreID, strBeginDate, strEndDate, uiTimePrecision, strChartData);
    }
    else if (uiTimePrecision < 86400)
    {
        return QueryHourlyCustomerFlow(strStoreID, strBeginDate, strEndDate, uiTimePrecision, strChartData);
    }
    else
    {
        return QueryDailyCustomerFlow(strStoreID, strBeginDate, strEndDate, uiTimePrecision, strChartData);
    }
}

bool PassengerFlowManager::QueryRawCustomerFlow(const std::string &strStoreID, const std::string &strBeginDate,
    const std::string &strEndDate, const unsigned int uiTimePrecision, std::string &strChartData)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select from_unixtime(floor(unix_timestamp(c.data_time) / %d) * %d) tm, sum(c.enter_number) from"
        " t_entrance_info a join t_entrance_device_association b on a.entrance_id = b.entrance_id"
        " join t_customer_flow_original_%d c on b.device_id = c.device_id"
        " where a.store_id = '%s' and c.data_time >= '%s' and c.data_time <= '%s'"
        " group by tm";

    int len = 0;
    int beginMonth = Month(strBeginDate);
    int totalMonth = MonthDuration(strBeginDate, strEndDate) < 12 ? MonthDuration(strBeginDate, strEndDate) : 12;
    for (int i = 0; i < totalMonth; ++i)
    {
        if (i > 0)
        {
            len += snprintf(sql + len, size - len, " union all ");
        }

        len += snprintf(sql + len, size - len, sqlfmt, uiTimePrecision, uiTimePrecision, beginMonth + i == 12 ? 12 : (beginMonth + i) % 12,
            strStoreID.c_str(), strBeginDate.c_str(), strEndDate.c_str());
    }
    strncat(sql, " order by tm", size - len);

    std::string key;
    std::list<int> value;
    std::map<std::string, std::list<int>> chartMap;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            key = strColumn;
            break;
        case 1:
            value.push_back(boost::lexical_cast<int>(strColumn));
            chartMap.insert(std::make_pair(key, value));
            value.clear();
            break;

        default:
            LOG_ERROR_RLD("QueryRawCustomerFlow sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryRawCustomerFlow exec sql failed, sql is " << sql);
        return false;
    }

    GenerateChartData(chartMap, strBeginDate, strEndDate, uiTimePrecision, strChartData);

    return true;
}

bool PassengerFlowManager::QueryHourlyCustomerFlow(const std::string &strStoreID, const std::string &strBeginDate,
    const std::string &strEndDate, const unsigned int uiTimePrecision, std::string &strChartData)
{
    std::string strTablePre;
    if (uiTimePrecision < 3600)
    {
        strTablePre = "t_customer_flow_statistic_half_hourly_";
    }
    else
    {
        strTablePre = "t_customer_flow_statistic_hourly_";
    }

    char sql[2048] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select c.data_time tm, sum(c.enter_number) from"
        " t_entrance_info a join t_entrance_device_association b on a.entrance_id = b.entrance_id"
        " join %s%d c on b.device_id = c.device_id"
        " where a.store_id = '%s' and c.data_time >= '%s' and c.data_time <= '%s'"
        " group by c.data_time";

    int len = 0;
    int beginQuarter = Quarter(strBeginDate);
    int totalQuarter = QuarterDuration(strBeginDate, strEndDate) < 4 ? QuarterDuration(strBeginDate, strEndDate) : 4;
    for (int i = 0; i < totalQuarter; ++i)
    {
        if (i > 0)
        {
            len += snprintf(sql + len, size - len, " union all ");
        }

        len += snprintf(sql + len, size - len, sqlfmt, strTablePre.c_str(), (beginQuarter + i) == 4 ? 4 : (beginQuarter + i) % 4,
            strStoreID.c_str(), strBeginDate.c_str(), strEndDate.c_str());
    }
    strncat(sql, " order by tm", size - len);

    std::string key;
    std::list<int> value;
    std::map<std::string, std::list<int>> chartMap;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            key = strColumn;
            break;
        case 1:
            value.push_back(boost::lexical_cast<int>(strColumn));
            chartMap.insert(std::make_pair(key, value));
            value.clear();
            break;

        default:
            LOG_ERROR_RLD("QueryHourlyCustomerFlow sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryHourlyCustomerFlow exec sql failed, sql is " << sql);
        return false;
    }

    GenerateChartData(chartMap, strBeginDate, strEndDate, uiTimePrecision, strChartData);

    return true;
}

bool PassengerFlowManager::QueryDailyCustomerFlow(const std::string &strStoreID, const std::string &strBeginDate,
    const std::string &strEndDate, const unsigned int uiTimePrecision, std::string &strChartData)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select c.data_time, sum(c.enter_number), ifnull(sum(d.order_amount), 0), ifnull(sum(d.deal_amount), 0) from"
        " t_entrance_info a join t_entrance_device_association b on a.entrance_id = b.entrance_id"
        " join t_customer_flow_statistic_daily c on b.device_id = c.device_id"
        " left join t_store_deal_pos d on a.store_id = d.store_id"
        " where a.store_id = '%s' and c.data_time >= '%s' and c.data_time <= '%s'"
        " group by c.data_time order by c.data_time";
    snprintf(sql, sizeof(sql), sqlfmt, strStoreID.c_str(), strBeginDate.c_str(), strEndDate.c_str());

    int enterNum = 0;
    std::string key;
    std::list<boost::any> value;
    std::map<std::string, std::list<boost::any>> chartMap;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            key = strColumn;
            break;
        case 1:
            enterNum = boost::lexical_cast<int>(strColumn);
            value.push_back(enterNum);
            break;
        case 2:
        {
            double dealRate = enterNum > 0 ? boost::lexical_cast<double>(strColumn) * 100 / enterNum : 0;
            value.push_back(boost::lexical_cast<int>(strColumn));
            value.push_back((double)((int)((dealRate + 0.005) * 100)) / 100);
            break;
        }
        case 3:
            value.push_back(boost::lexical_cast<double>(strColumn));
            chartMap.insert(std::make_pair(key, value));
            value.clear();
            break;

        default:
            LOG_ERROR_RLD("QueryDailyCustomerFlow sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryDailyCustomerFlow exec sql failed, sql is " << sql);
        return false;
    }

    GenerateChartDataWithPOS(chartMap, strBeginDate, strEndDate, uiTimePrecision, strChartData);

    return true;
}

void PassengerFlowManager::GenerateChartData(const std::map<std::string, std::list<int>> &chartDataMap,
    const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiTimePrecision, std::string &strChartData)
{
    //const int series = 1;
    const char *legend[] = { "enter_number" };
    Json::Value label;
    Json::Value flowNum;

    for (int begin = TimePrecisionScale(strBeginDate, uiTimePrecision), end = TimePrecisionScale(strEndDate, uiTimePrecision); begin <= end; begin += uiTimePrecision)
    {
        std::string time = boost::posix_time::to_iso_extended_string(boost::posix_time::from_time_t(begin));
        time.replace(time.find_first_of('T'), 1, std::string(" "));

        label.append(time);

        auto pos = chartDataMap.find(time);
        if (pos == chartDataMap.end())
        {
            flowNum.append(0);
        }
        else
        {
            flowNum.append(pos->second.front());
        }
    }

    Json::Value root;
    Json::FastWriter writer;
    root["chart_label"] = label;
    root["chart_legend"] = std::string(legend[0]);
    root[legend[0]] = flowNum;
    strChartData = writer.write(root);
}

void PassengerFlowManager::GenerateChartDataWithPOS(const std::map<std::string, std::list<boost::any>> &chartDataMap, const std::string &strBeginDate,
    const std::string &strEndDate, const unsigned int uiTimePrecision, std::string &strChartData)
{
    const int series = 4;
    const char *legend[] = { "enter_number", "order_amount", "deal_rate", "deal_amount" };
    Json::Value label;
    Json::Value statData[series];

    for (int begin = TimePrecisionScale(strBeginDate, uiTimePrecision), end = TimePrecisionScale(strEndDate, uiTimePrecision); begin <= end; begin += uiTimePrecision)
    {
        std::string time = boost::posix_time::to_iso_extended_string(boost::posix_time::from_time_t(begin));
        time.replace(time.find_first_of('T'), 1, std::string(" "));

        label.append(time);

        auto pos = chartDataMap.find(time);
        if (pos == chartDataMap.end())
        {
            for (int i = 0; i < series; ++i) statData[i].append(0);
        }
        else
        {
            int i = 0;
            for (auto &num : pos->second)
            {
                if (i < 2)
                {
                    statData[i++].append(boost::any_cast<int>(num));
                }
                else
                {
                    statData[i++].append(boost::any_cast<double>(num));
                }
            }
        }
    }

    Json::Value root;
    Json::FastWriter writer;
    root["chart_lebel"] = label;
    for (int i = 0; i < series; ++i)
    {
        root["chart_legend"].append(std::string(legend[i]));
        root[legend[i]] = statData[i];
    }
    strChartData = writer.write(root);
}

bool PassengerFlowManager::QueryPatrolResultReport(const std::string &strUserID, const std::string &strStoreID, const std::string &strPatrolUserID,
    const unsigned int uiPatrolResult, const std::string &strBeginDate, const std::string &strEndDate, std::string &strChartData)
{
    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "select date_format(a.patrol_date, '%%Y-%%m-%%d') d, group_concat(distinct b.store_name order by b.store_name separator '\\n'),"
        " count(a.patrol_result=0 or null), count(a.patrol_result=1 or null), count(a.patrol_result=2 or null)"
        " from t_remote_patrol_store a join t_store_info b on a.store_id = b.store_id where 1 = 1";
    int len = snprintf(sql, size, sqlfmt);

    if (!strStoreID.empty())
    {
        len += snprintf(sql + len, size - len, " and a.store_id = '%s'", strStoreID.c_str());
    }

    if (!strPatrolUserID.empty())
    {
        len += snprintf(sql + len, size - len, " and a.user_id = '%s'", strPatrolUserID.c_str());
    }

    if (uiPatrolResult != UNUSED_INPUT_UINT)
    {
        len += snprintf(sql + len, size - len, " and a.patrol_result = %d", uiPatrolResult);
    }

    if (!strBeginDate.empty())
    {
        len += snprintf(sql + len, size - len, " and a.patrol_date >= '%s'", strBeginDate.c_str());
    }

    if (!strEndDate.empty())
    {
        len += snprintf(sql + len, size - len, " and a.patrol_date <= '%s'", strEndDate.c_str());
    }

    snprintf(sql + len, size - len, " group by d order by d");

    struct ResultPatrolRecord
    {
        std::string strPatrolDate;
        std::string strStoreName;
        int iPassCount;
        int iNotPassCount;
        int iTodoCount;
    } rstPatrolRecord;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            rstPatrolRecord.strPatrolDate = strColumn;
            break;
        case 1:
            rstPatrolRecord.strStoreName = strColumn;
            break;
        case 2:
            rstPatrolRecord.iPassCount = boost::lexical_cast<int>(strColumn);
            break;
        case 3:
            rstPatrolRecord.iNotPassCount = boost::lexical_cast<int>(strColumn);
            break;
        case 4:
            rstPatrolRecord.iTodoCount = boost::lexical_cast<int>(strColumn);
            result = rstPatrolRecord;
            break;

        default:
            LOG_ERROR_RLD("QueryPatrolResultReport sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryPatrolResultReport exec sql failed, sql is " << sql);
        return false;
    }

    const int seriesNum = 3;
    //const char *legend[] = { "合格", "不合格", "待处理" };
    const char *legend[] = { "pass", "not_pass", "todo" };
    Json::Value label;
    Json::Value series[seriesNum];
    Json::Value statData[seriesNum];

    unsigned int uiTimePrecision = 3600 * 24;
    for (int begin = TimePrecisionScale(strBeginDate, uiTimePrecision), end = TimePrecisionScale(strEndDate, uiTimePrecision); begin <= end; begin += uiTimePrecision)
    {
        std::string time = boost::posix_time::to_iso_extended_string(boost::posix_time::from_time_t(begin)).erase(10);

        label.append(time);

        auto rstIt = ResultList.begin();
        auto rstEnd = ResultList.end();
        for (; rstIt != rstEnd; ++rstIt)
        {
            auto patrol = boost::any_cast<ResultPatrolRecord>(*rstIt);

            LOG_ERROR_RLD("---debug, result record: " << patrol.strPatrolDate
                << ", " << patrol.strStoreName
                << ", " << patrol.iPassCount
                << ", " << patrol.iNotPassCount
                << ", " << patrol.iTodoCount);

            if (patrol.strPatrolDate == time)
            {
                series[0]["data"].append(patrol.iPassCount);
                series[0]["tip"].append(patrol.strStoreName);
                series[1]["data"].append(patrol.iNotPassCount);
                series[1]["tip"].append(patrol.strStoreName);
                series[2]["data"].append(patrol.iTodoCount);
                series[2]["tip"].append(patrol.strStoreName);

                break;
            }
        }

        if (rstIt == rstEnd)
        {
            series[0]["data"].append(0);
            series[0]["tip"].append(std::string());
            series[1]["data"].append(0);
            series[1]["tip"].append(std::string());
            series[2]["data"].append(0);
            series[2]["tip"].append(std::string());
        }
    }

    Json::Value root;
    Json::FastWriter writer;
    root["chart_label"] = label;
    for (int i = 0; i < seriesNum; ++i)
    {
        series[i]["name"] = std::string(legend[i]);
        root["chart_legend"].append(std::string(legend[i]));
        root["series"].append(series[i]);
    }
    strChartData = writer.write(root);

    return true;
}

void PassengerFlowManager::ReportCustomerFlow(const std::string &strDeviceID, const std::list<PassengerFlowProtoHandler::RawCustomerFlow> &customerFlowList)
{
    for (auto &customerFlow : customerFlowList)
    {
        AddCustomerFlow(strDeviceID, customerFlow);
    }
}

void PassengerFlowManager::AddCustomerFlow(const std::string &strDeviceID, const PassengerFlowProtoHandler::RawCustomerFlow &customerFlowInfo)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_customer_flow_original_%d (id, device_id, data_time, enter_number, leave_number, stay_number)"
        " values(uuid(), '%s', '%s', %d, %d, %d)";
    snprintf(sql, sizeof(sql), sqlfmt, Month(customerFlowInfo.m_strDataTime), customerFlowInfo.m_strDataTime.c_str(),
        customerFlowInfo.m_uiEnterNumber, customerFlowInfo.m_uiLeaveNumber, customerFlowInfo.m_uiStayNumber);

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddCustomerFlow exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ReportSensorInfo(const std::string &strDeviceID, const std::list<PassengerFlowProtoHandler::Sensor> &sensorList)
{
    for (auto &sensor : sensorList)
    {
        std::string strSensorID;
        if (!QueryDeviceSensor(strDeviceID, sensor.m_strSensorType, strSensorID) || strSensorID.empty())
        {
            LOG_ERROR_RLD("ReportSensorInfo error, query device sensor error, device id is " << strDeviceID
                << " and sensor type is " << sensor.m_strSensorType);
            continue;
        }

        AddSensorInfo(strDeviceID, strSensorID, sensor);
    }
}

bool PassengerFlowManager::QueryDeviceSensor(const std::string &strDeviceID, const std::string &strSensorType, std::string &strSensorID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select sensor_id from t_store_sensor where device_id = '%s' and sensor_type = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strDeviceID.c_str(), strSensorType.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = strColumn;
            break;

        default:
            LOG_ERROR_RLD("QueryDeviceSensor sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryDeviceSensor exec sql failed, sql is " << sql);
        return false;
    }

    if (!ResultList.empty())
    {
        strSensorID = boost::any_cast<std::string>(ResultList.front());
    }

    return true;
}

void PassengerFlowManager::AddSensorInfo(const std::string &strDeviceID, const std::string &strSensorID, const PassengerFlowProtoHandler::Sensor &sensorInfo)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_sensor_value (id, sensor_id, value, create_date)"
        " values(uuid(), '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strSensorID.c_str(), sensorInfo.m_strValue.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddSensorInfo exec sql failed, sql is " << sql);
    }
}
