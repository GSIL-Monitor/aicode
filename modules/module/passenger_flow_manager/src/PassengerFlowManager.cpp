#include <map>
#include "PassengerFlowManager.h"
#include <boost/scope_exit.hpp>
#include "CommonUtility.h"
#include "ReturnCode.h"
#include "mysql_impl.h"
#include "boost/lexical_cast.hpp"
#include "json/json.h"
#include "HttpClient.h"

const std::string PassengerFlowManager::READ_STATE = "read";
const std::string PassengerFlowManager::UNREAD_STATE = "unread";

PassengerFlowManager::PassengerFlowManager(const ParamInfo &pinfo) :
    m_ParamInfo(pinfo),
    m_DBRuner(1),
    m_pProtoHandler(new PassengerFlowProtoHandler),
    m_pMysql(new MysqlImpl),
    m_DBCache(m_pMysql),
    m_uiMsgSeq(0),
    m_DBTimer(NULL, 600)
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
    ReturnInfo::RetCode(ReturnInfo::FAILED_CODE);

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
    storeInfo.m_entranceList = req.m_storeInfo.m_entranceList;
    storeInfo.m_strCreateDate = CurrentTime();

    //这里是异步执行sql，防止阻塞，后续可以使用其他方式比如MQ来消除数据库瓶颈
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

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &storeInfo)
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
            rsp.m_storeInfo.m_strCreateDate = storeInfo.m_strCreateDate;
            rsp.m_storeInfo.m_entranceList.swap(storeInfo.m_entranceList);
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
        LOG_ERROR_RLD("Query store info failed, src id is " << strSrcID);
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
                    << " and store name is " << store.m_strStoreName);
            }
        }
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query all store req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryAllStore(req.m_strUserID, storeList))
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
    eventInfo.m_strProcessState = req.m_eventInfo.m_strProcessState;
    eventInfo.m_uiTypeList = req.m_eventInfo.m_uiTypeList;
    eventInfo.m_strHandlerList = req.m_eventInfo.m_strHandlerList;
    eventInfo.m_strCreateDate = CurrentTime();

    m_DBRuner.Post(boost::bind(&PassengerFlowManager::AddEvent, this, eventInfo));

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
            rsp.m_eventInfo.m_uiTypeList.swap(eventInfo.m_uiTypeList);
            rsp.m_eventInfo.m_strHandlerList.swap(eventInfo.m_strHandlerList);
            rsp.m_eventInfo.m_strCreateDate = eventInfo.m_strCreateDate;
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
            << " and result is " << blResult);
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
                    << " and process state is " << event.m_strProcessState);
            }
        }
    }
    BOOST_SCOPE_EXIT_END

        if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
        {
            LOG_ERROR_RLD("Query all event req unserialize failed, src id is " << strSrcID);
            return false;
        }

    if (!QueryAllEvent(req.m_strUserID, eventList))
    {
        LOG_ERROR_RLD("Query all event failed, src id is " << strSrcID);
        return false;
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
            rsp.m_smartGuardStore.m_strPlanID = smartGuardStore.m_strPlanID;
            rsp.m_smartGuardStore.m_strPlanName = smartGuardStore.m_strPlanName;
            rsp.m_smartGuardStore.m_strEnable = smartGuardStore.m_strEnable;
            rsp.m_smartGuardStore.m_strBeginTime = smartGuardStore.m_strBeginTime;
            rsp.m_smartGuardStore.m_strEndTime = smartGuardStore.m_strEndTime;
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
            << " and plan id is " << smartGuardStore.m_strPlanID
            << " and plan name is " << smartGuardStore.m_strPlanName
            << " and enable is " << smartGuardStore.m_strEnable
            << " and begin time is " << smartGuardStore.m_strBeginTime
            << " and end time is " << smartGuardStore.m_strEndTime
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
        if (!QueryAllSmartGuardStoreByUser(req.m_strUserID, planList))
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

bool PassengerFlowManager::IsValidStore(const std::string &strUserID, const std::string &strStoreName)
{
    char sql[512] = { 0 };
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
    char sql[512] = { 0 };
    const char *sqlfmt = "insert into t_store_info (id, store_id, store_name, goods_category, address, create_date)"
        " values (uuid(), '%s', '%s', '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, storeInfo.m_strStoreID.c_str(), storeInfo.m_strStoreName.c_str(),
        storeInfo.m_strGoodsCategory.c_str(), storeInfo.m_strAddress.c_str(), storeInfo.m_strCreateDate.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddStore exec sql failed, sql is " << sql);
        return;
    }

    AddUserStoreAssociation(strUserID, storeInfo.m_strStoreID);
}

void PassengerFlowManager::AddUserStoreAssociation(const std::string &strUserID, const std::string &strStoreID)
{
    char sql[512] = { 0 };
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
    char sql[512] = { 0 };
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

    char sql[512] = { 0 };
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
    char sql[512] = { 0 };
    const char *sqlfmt = "select store_id, store_name, goods_category, address, create_date from"
        " t_store_info where store_id = '%s'";
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
            rstStore.m_strCreateDate = strColumn;
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
    storeInfo.m_strCreateDate = store.m_strCreateDate;

    return true;
}

bool PassengerFlowManager::QueryStoreEntrance(const std::string &strStoreID, std::list<PassengerFlowProtoHandler::Entrance> &entranceList)
{
    char sql[512] = { 0 };
    const char *sqlfmt = "select entrance_id, entrance_name from t_entrance_info where store_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strStoreID.c_str());

    struct ResultEntrance
    {
        std::string strEntranceID;
        std::string strEntranceName;
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

        QueryEntranceDevice(entrance.m_strEntranceID, entrance.m_strDeviceIDList);

        entranceList.push_back(std::move(entrance));
    }

    return true;
}

bool PassengerFlowManager::QueryEntranceDevice(const std::string &strEntranceID, std::list<std::string> &strDeviceIDList)
{
    char sql[512] = { 0 };
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

bool PassengerFlowManager::QueryAllStore(const std::string &strUserID, std::list<PassengerFlowProtoHandler::Store> &storeList,
    const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 10*/)
{
    char sql[512] = { 0 };
    const char *sqlfmt = "select a.store_id, a.store_name from"
        " t_store_info a join t_user_store_association b on a.store_id = b.store_id"
        " where b.user_id = '%s' limit %d, %d";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), uiBeginIndex, uiPageSize);

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
            result = rstStore;
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
        storeList.push_back(std::move(boost::any_cast<PassengerFlowProtoHandler::Store>(result)));
    }

    return true;
}

bool PassengerFlowManager::IsValidEntrance(const std::string &strStoreID, const std::string &strEntranceName)
{
    char sql[512] = { 0 };
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
    char sql[512] = { 0 };
    const char *sqlfmt = "insert into t_entrance_info (id, entrance_id, entrance_name, store_id, create_date)"
        " values (uuid(), '%s', '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, entranceInfo.m_strEntranceID.c_str(), entranceInfo.m_strEntranceName.c_str(),
        strStoreID.c_str(), CurrentTime().c_str());

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
    char sql[512] = { 0 };
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
    char sql[512] = { 0 };
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
    char sql[512] = { 0 };
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
    char sql[512] = { 0 };
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

    char sql[512] = { 0 };
    int size = sizeof(sql);
    int len = snprintf(sql, size, "update t_entrance_info set id = id");

    if (!entrance.m_strEntranceName.empty())
    {
        len += snprintf(sql + len, size - len, ", entrance_name = '%s'", entrance.m_strEntranceName.c_str());
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
    char sql[512] = { 0 };
    const char *sqlfmt = "insert into t_event_info (id, event_id, source, submit_date, expire_date, user_id, device_id, process_state, create_date)"
        " values (uuid(), '%s', '%s', '%s', '%s', '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, eventInfo.m_strEventID.c_str(), eventInfo.m_strSource.c_str(),
        eventInfo.m_strSubmitDate.c_str(), eventInfo.m_strExpireDate.c_str(), eventInfo.m_strUserID.c_str(),
        eventInfo.m_strDeviceID.c_str(), eventInfo.m_strProcessState.c_str(), eventInfo.m_strCreateDate.c_str());

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

    AddEventRemark(eventInfo.m_strEventID, eventInfo.m_strUserID, eventInfo.m_strRemark);
}

void PassengerFlowManager::AddEventType(const std::string &strEventID, const unsigned int uiType)
{
    char sql[512] = { 0 };
    const char *sqlfmt = "insert into t_event_type (id, event_id, event_type, create_date)"
        " values (uuid(), '%s', %d, '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strEventID.c_str(), uiType, CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddEventType exec sql failed, sql is " << sql);
    }
}


void PassengerFlowManager::AddEventRemark(const std::string &strEventID, const std::string &strUserID, const std::string &strRemark)
{
    char sql[512] = { 0 };
    const char *sqlfmt = "insert into t_event_remark (id, event_id, remark, user_id, create_date)"
        " values (uuid(), '%s', %d, '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strEventID.c_str(), strRemark.c_str(), strUserID.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddEventRemark exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::AddEventUserAssociation(const std::string &strEventID, const std::string &strUserID, const std::string &strUserRole)
{
    char sql[512] = { 0 };
    const char *sqlfmt = "insert into t_event_user_association (id, event_id, user_id, user_role, read_state, create_date)"
        " values (uuid(), '%s', '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strEventID.c_str(), strUserID.c_str(), strUserRole.c_str(), UNREAD_STATE.c_str(), CurrentTime().c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddEventUserAssociation exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::DeleteEvent(const std::string &strEventID, const std::string &strUserID)
{
    char sql[512] = { 0 };
    const char *sqlfmt = "delete from t_event_user_association where event_id = '%s' and user_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEventID.c_str(), strUserID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteEvent exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifyEvent(const PassengerFlowProtoHandler::Event &eventInfo)
{
    bool blModified = false;

    char sql[512] = { 0 };
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

bool PassengerFlowManager::QueryEventInfo(const std::string &strEventID, PassengerFlowProtoHandler::Event &eventInfo)
{
    char sql[512] = { 0 };
    const char *sqlfmt = "select event_id, source, submit_date, expire_date, user_id, device_id, process_state, create_date from"
        " t_event_info where event_id = '%s'";
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

    std::list<std::list<std::string>> remarkList;
    if (!QueryEventRemark(eventInfo.m_strEventID, remarkList))
    {
        LOG_ERROR_RLD("QueryEventInfo failed, query event remark error, event id is " << eventInfo.m_strEventID);
        return false;
    }

    Json::Value root;
    Json::FastWriter writer;
    root.resize(0);
    for (auto &remark : remarkList)
    {
        Json::Value value;
        for (auto &elem : remark) value.append(elem);

        root.append(value);
    }
    eventInfo.m_strRemark = writer.write(root);

    return true;
}

bool PassengerFlowManager::QueryEventType(const std::string &strEventID, std::list<unsigned int> &typeList)
{
    char sql[512] = { 0 };
    const char *sqlfmt = "select event_type from t_event_type where event_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strEventID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            result = strColumn;
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
    char sql[512] = { 0 };
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

bool PassengerFlowManager::QueryEventRemark(const std::string &strEventID, std::list<std::list<std::string>> &strRemarkList)
{
    char sql[512] = { 0 };
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

    for (auto &result : ResultList)
    {
        strRemarkList.push_back(boost::any_cast<std::list<std::string>>(result));
    }

    return true;
}

bool PassengerFlowManager::QueryAllEvent(const std::string &strUserID, std::list<PassengerFlowProtoHandler::Event> &eventList,
    const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 10*/)
{
    char sql[512] = { 0 };
    const char *sqlfmt = "select a.event_id, a.source, a.submit_date, a.user_id, a.device_id, a.process_state from"
        " t_event_info a join t_event_user_association b on a.event_id = b.event_id"
        " where b.user_id = '%s' limit %d, %d";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), uiBeginIndex, uiPageSize);

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
            rstEvent.m_strUserID = strColumn;
            break;
        case 4:
            rstEvent.m_strDeviceID = strColumn;
            break;
        case 5:
            rstEvent.m_strProcessState = strColumn;
            result = rstEvent;
            break;

        default:
            LOG_ERROR_RLD("QueryAllEvent sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryAllEvent exec sql failed, sql is " << sql);
        return false;
    }

    for (auto &result : ResultList)
    {
        eventList.push_back(std::move(boost::any_cast<PassengerFlowProtoHandler::Event>(result)));
    }

    return true;
}

void PassengerFlowManager::AddSmartGuardStore(const std::string &strUserID, const std::string &strStoreID,
    const PassengerFlowProtoHandler::SmartGuardStore &smartGuardStore)
{
    char sql[512] = { 0 };
    const char *sqlfmt = "insert into t_smart_guard_store_plan"
        " (id, plan_id, user_id, store_id, plan_name, enable, begin_time, end_time, begin_time2, end_time2)"
        " values(uuid(), '%s', '%s', '%s', '%s', '%s', '%s', '%s', %s, %s)";
    snprintf(sql, sizeof(sql), sqlfmt, smartGuardStore.m_strPlanID.c_str(), strUserID.c_str(), strStoreID.c_str(), smartGuardStore.m_strPlanName.c_str(),
        smartGuardStore.m_strEnable.c_str(), smartGuardStore.m_strBeginTime.c_str(), smartGuardStore.m_strEndTime.c_str(),
        smartGuardStore.m_strBeginTime2.empty() ? "null" : ("'" + smartGuardStore.m_strBeginTime2 + "'").c_str(),
        smartGuardStore.m_strEndTime2.empty() ? "null" : ("'" + smartGuardStore.m_strEndTime2 + "'").c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddSmartGuardStore exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::AddGuardStoreEntranceAssociation(const std::string &strPlanID, const std::string &strEntranceID)
{
    char sql[512] = { 0 };
    const char *sqlfmt = "insert into t_guard_plan_entrance_association (id, plan_id, entrance_id)"
        " values(uuid(), '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strPlanID.c_str(), strEntranceID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddGuardStoreEntranceAssociation exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::DeleteSmartGuardStore(const std::string &strPlanID)
{
    char sql[512] = { 0 };
    const char *sqlfmt = "delete a, b from"
        " t_smart_guard_store_plan a left join t_guard_plan_entrance_association b on a.plan_id = b.plan_id"
        " where plan_id = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPlanID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteSmartGuardStore exec sql failed, sql is " << sql);
    }
}

void PassengerFlowManager::ModifySmartGuardStore(const PassengerFlowProtoHandler::SmartGuardStore &smartGuardStore)
{
    bool blModified = false;

    char sql[512] = { 0 };
    int size = sizeof(sql);
    int len = snprintf(sql, size, "update t_entrance_info set id = id");

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

    if (!blModified)
    {
        LOG_INFO_RLD("ModifySmartGuardStore completed, entrance info is not changed");
        return;
    }

    snprintf(sql + len, size - len, " where plan_id = '%s'", smartGuardStore.m_strPlanID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("ModifySmartGuardStore exec sql failed, sql is " << sql);
    }
}

bool PassengerFlowManager::QuerySmartGuardStoreInfo(const std::string &strPlanID, PassengerFlowProtoHandler::SmartGuardStore &smartGuardStore)
{
    char sql[512] = { 0 };
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

    auto event = boost::any_cast<PassengerFlowProtoHandler::SmartGuardStore>(ResultList.front());
    smartGuardStore.m_strStoreID = rstGuardStore.m_strStoreID;
    smartGuardStore.m_strStoreName = rstGuardStore.m_strStoreName;
    smartGuardStore.m_strPlanName = rstGuardStore.m_strPlanName;
    smartGuardStore.m_strEnable = rstGuardStore.m_strEnable;
    smartGuardStore.m_strBeginTime = rstGuardStore.m_strBeginTime;
    smartGuardStore.m_strEndTime = rstGuardStore.m_strEndTime;
    smartGuardStore.m_strBeginTime2 = rstGuardStore.m_strBeginTime2;
    smartGuardStore.m_strEndTime2 = rstGuardStore.m_strEndTime2;

    return QueryGuardStoreEntranceAssociation(strPlanID, smartGuardStore.m_strEntranceIDList);
}

bool PassengerFlowManager::QueryGuardStoreEntranceAssociation(const std::string &strPlanID, std::list<std::string> &strEntranceIDList)
{
    char sql[512] = { 0 };
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
    char sql[512] = { 0 };
    const char *sqlfmt = "select b.store_id, b.store_name, a.plan_id, a.plan_name, a.enable, a.begin_time, a.end_time, ifnull(a.begin_time2, ''), ifnull(a.end_time2, '')"
        " from t_smart_guard_store_plan a join t_store_info b on a.store_id = b.store_id"
        " where a.user_id = '%s' limit %d, %d";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), uiBeginIndex, uiPageSize);

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
    char sql[512] = { 0 };
    const char *sqlfmt = "select d.store_id, d.store_name, d.plan_name, d.enable, d.begin_time, d.end_time, ifnull(d.begin_time2, ''), ifnull(d.end_time2, '')"
        " t_entrance_device_association a join t_guard_plan_entrance_association b on a.entrance_id = b.entrance_id"
        " join t_guard_plan_entrance_association c on b.entrance_id = c.entrance_id"
        " join t_smart_guard_store_plan d on c.plan_id = d.plan_id"
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

void PassengerFlowManager::ImportPOSData(const std::string &strStoreID, const unsigned int uiOrderAmount,
    const unsigned int uiGoodsAmount, const double dDealAmount, const std::string &strDealDate)
{
    char sql[512] = { 0 };
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

void PassengerFlowManager::ReportCustomerFlow(const std::string &strDeviceID, const std::list<PassengerFlowProtoHandler::RawCustomerFlow> &customerFlowList)
{
    for (auto &customerFlow : customerFlowList)
    {
        AddCustomerFlow(strDeviceID, customerFlow);
    }
}

void PassengerFlowManager::AddCustomerFlow(const std::string &strDeviceID, const PassengerFlowProtoHandler::RawCustomerFlow &customerFlowInfo)
{
    char sql[512] = { 0 };
    const char *sqlfmt = "insert into t_customer_flow_original_%d (id, device_id, data_time, enter_number, leave_number, stay_number)"
        " values(uuid(), '%s', '%s', %d, %d, %d)";
    snprintf(sql, sizeof(sql), sqlfmt, Month(customerFlowInfo.m_strDataTime), customerFlowInfo.m_strDataTime.c_str(),
        customerFlowInfo.m_uiEnterNumber, customerFlowInfo.m_uiLeaveNumber, customerFlowInfo.m_uiStayNumber);

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddCustomerFlow exec sql failed, sql is " << sql);
    }
}

