#include "PassengerFlowMsgHandler.h"
#include <boost/algorithm/string.hpp>  
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scope_exit.hpp>

#include "util.h"
#include "mime_types.h"
#include "LogRLD.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include "boost/regex.hpp"
#include "ReturnCode.h"
#include "boost/date_time/gregorian/gregorian.hpp"
#include "PassengerFlowProtoHandler.h"

const std::string PassengerFlowMsgHandler::SUCCESS_CODE = "0";
const std::string PassengerFlowMsgHandler::SUCCESS_MSG = "Ok";
const std::string PassengerFlowMsgHandler::FAILED_CODE = "-1";
const std::string PassengerFlowMsgHandler::FAILED_MSG = "Inner failed";

const std::string PassengerFlowMsgHandler::REGISTER_USER_ACTION("register_user");


const std::string PassengerFlowMsgHandler::ADD_STORE_ACTION("create_store");

const std::string PassengerFlowMsgHandler::DEL_STORE_ACTION("delete_store");

const std::string PassengerFlowMsgHandler::MOD_STORE_ACTION("modify_store");

const std::string PassengerFlowMsgHandler::QUERY_STORE_ACTION("query_store");

const std::string PassengerFlowMsgHandler::QUERY_ALL_STORE_ACTION("query_all_store");

const std::string PassengerFlowMsgHandler::CREATE_ENTRANCE_ACTION("create_entrance");

const std::string PassengerFlowMsgHandler::DELETE_ENTRANCE_ACTION("delete_entrance");

const std::string PassengerFlowMsgHandler::MODIFY_ENTRANCE_ACTION("modify_entrance");

const std::string PassengerFlowMsgHandler::UPLOAD_PASSENGER_FLOW_ACTION("upload_customer_flow");

const std::string PassengerFlowMsgHandler::BIND_ENTRANCE_DEVICE("bind_entrance_device");

const std::string PassengerFlowMsgHandler::UNBIND_ENTRANCE_DEVICE("unbind_entrance_device");

const std::string PassengerFlowMsgHandler::IMPORT_POS_DATA("import_pos_data");

const std::string PassengerFlowMsgHandler::QUERY_PASSENGER_FLOW_REPORT("query_customer_flow");

const std::string PassengerFlowMsgHandler::REPORT_EVENT("report_event");

const std::string PassengerFlowMsgHandler::DELETE_EVENT("delete_event");

const std::string PassengerFlowMsgHandler::MODIFY_EVENT("modify_event");

const std::string PassengerFlowMsgHandler::QUERY_EVENT("query_event");

const std::string PassengerFlowMsgHandler::QUERY_ALL_EVENT("query_all_event");

const std::string PassengerFlowMsgHandler::CREATE_GUARD_PLAN("create_smart_guard_store");

const std::string PassengerFlowMsgHandler::DELETE_GUARD_PLAN("delete_smart_guard_store");

const std::string PassengerFlowMsgHandler::MODIFY_GUARD_PLAN("modify_smart_guard_store");

const std::string PassengerFlowMsgHandler::QUERY_GUARD_PLAN("query_smart_guard_store");

const std::string PassengerFlowMsgHandler::QUERY_ALL_GUARD_PLAN("query_all_smart_guard_store");

const std::string PassengerFlowMsgHandler::CREATE_PATROL_PLAN("create_regular_patrol");

const std::string PassengerFlowMsgHandler::DELETE_PATROL_PLAN("delete_regular_patrol");

const std::string PassengerFlowMsgHandler::MODIFY_PATROL_PLAN("modify_regular_patrol");

const std::string PassengerFlowMsgHandler::QUERY_PATROL_PLAN("query_regular_patrol");

const std::string PassengerFlowMsgHandler::QUERY_ALL_PATROL_PLAN("query_all_regular_patrol");

const std::string PassengerFlowMsgHandler::CREATE_VIP("create_vip_customer");

const std::string PassengerFlowMsgHandler::DELETE_VIP("delete_vip_customer");

const std::string PassengerFlowMsgHandler::MODIFY_VIP("modify_vip_customer");

const std::string PassengerFlowMsgHandler::QUERY_VIP("query_vip_customer");

const std::string PassengerFlowMsgHandler::QUERY_ALL_VIP("query_all_vip_customer");

const std::string PassengerFlowMsgHandler::CREATE_VIP_CONSUME("add_consume_history");

const std::string PassengerFlowMsgHandler::DELETE_VIP_CONSUME("delete_consume_history");

const std::string PassengerFlowMsgHandler::MODIFY_VIP_CONSUME("modify_consume_history");

const std::string PassengerFlowMsgHandler::QUERY_VIP_CONSUME("query_all_consume_history");

const std::string PassengerFlowMsgHandler::USER_JOIN_STORE("user_join_store");

const std::string PassengerFlowMsgHandler::USER_QUIT_STORE("user_quit_store");

const std::string PassengerFlowMsgHandler::QUERY_USER_STORE("query_store_all_user");

const std::string PassengerFlowMsgHandler::CREATE_EVALUATION_TEMPLATE("create_evaluation_template");

const std::string PassengerFlowMsgHandler::DELETE_EVALUATION_TEMPLATE("remove_evaluation_template");

const std::string PassengerFlowMsgHandler::MODIFY_EVALUATION_TEMPLATE("modify_evaluation_template");

const std::string PassengerFlowMsgHandler::QUERY_EVALUATION_TEMPLATE("query_evaluation_template");


const std::string PassengerFlowMsgHandler::CREATE_EVALUATION_OF_STORE("create_store_evaluation");

const std::string PassengerFlowMsgHandler::DELETE_EVALUATION_OF_STORE("remove_store_evaluation");

const std::string PassengerFlowMsgHandler::MODIFY_EVALUATION_OF_STORE("modify_store_evaluation");

const std::string PassengerFlowMsgHandler::QUERY_EVALUATION_OF_STORE("query_store_evaluation");

const std::string PassengerFlowMsgHandler::QUERY_ALL_EVALUATION_OF_STORE("query_all_store_evaluation");

PassengerFlowMsgHandler::PassengerFlowMsgHandler(const ParamInfo &parminfo):
m_ParamInfo(parminfo),
m_pInteractiveProtoHandler(new PassengerFlowProtoHandler)
{

}

PassengerFlowMsgHandler::~PassengerFlowMsgHandler()
{

}

int PassengerFlowMsgHandler::RspFuncCommonAction(CommMsgHandler::Packet &pt, int *piRetCode, RspFuncCommon rspfunc)
{
    const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);
    int iPreCode = 0;
    if (!PreCommonHandler(strMsgReceived, iPreCode))
    {
        ReturnInfo::RetCode(iPreCode);
        return CommMsgHandler::FAILED;
    }

    if (NULL == rspfunc)
    {
        LOG_ERROR_RLD("Rsp function is null");
        return CommMsgHandler::FAILED;
    }

    int iRspfuncRet = rspfunc(pt);

    ReturnInfo::RetCode(*piRetCode);

    return iRspfuncRet;
}

bool PassengerFlowMsgHandler::PreCommonHandler(const std::string &strMsgReceived, int &iRetCode)
{
    PassengerFlowProtoHandler::CustomerFlowMsgType mtype;
    if (!m_pInteractiveProtoHandler->GetCustomerFlowMsgType(strMsgReceived, mtype))
    {
        LOG_ERROR_RLD("Get msg type failed.");
        return false;
    }

    if (PassengerFlowProtoHandler::CustomerFlowMsgType::CustomerFlowPreHandleReq_T == mtype)
    {
        PassengerFlowProtoHandler::CustomerFlowPreHandleRsp rsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, rsp))
        {
            LOG_ERROR_RLD("Msg prehandler rsp unserialize failed.");
            return false;
        }

        LOG_INFO_RLD("Msg prehandler rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
            " and user session id is " << rsp.m_strSID);

        if (CommMsgHandler::SUCCEED != (iRetCode = rsp.m_iRetcode))
        {
            LOG_ERROR_RLD("Msg prehandler rsp return failed.");
            return false;
        }
    }

    return true;
}

bool PassengerFlowMsgHandler::AddStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;
    
    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("store_name");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store name not found.");
        return blResult;
    }
    const std::string strStoreName = itFind->second;

    std::string strGoodsCategory;
    itFind = pMsgInfoMap->find("goods_category");
    if (pMsgInfoMap->end() != itFind)
    {
        strGoodsCategory = itFind->second;
    }

    std::string strAddress;
    itFind = pMsgInfoMap->find("address");
    if (pMsgInfoMap->end() != itFind)
    {
        strAddress = itFind->second;
    }

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Add store info received and session id is " << strSid << " and user id is " << strUserID << " and store name is " << strStoreName 
        << " and goods category is " << strGoodsCategory
        << " and extend is " << strExtend << " and address is " << strAddress);

    StoreInfo store;
    store.m_strAddress = strAddress;
    store.m_strExtend = strExtend;
    store.m_strGoodsCategory = strGoodsCategory;
    store.m_strStoreName = strStoreName;

    if (!AddStore(strSid, strUserID, store))
    {
        LOG_ERROR_RLD("Add store handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("storeid", store.m_strStoreID));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::DelStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;
        
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete store info received and session id is " << strSid << " and user id is " << strUserID << " and store id is " << strStoreID);

    if (!DelStore(strSid, strUserID, strStoreID))
    {
        LOG_ERROR_RLD("Delete store handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::ModifyStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;

    std::string strStoreName;
    itFind = pMsgInfoMap->find("store_name");
    if (pMsgInfoMap->end() != itFind)
    {
        strStoreName = itFind->second;
    }

    std::string strGoodsCategory;
    itFind = pMsgInfoMap->find("goods_category");
    if (pMsgInfoMap->end() != itFind)
    {
        strGoodsCategory = itFind->second;
    }

    std::string strAddress;
    itFind = pMsgInfoMap->find("address");
    if (pMsgInfoMap->end() != itFind)
    {
        strAddress = itFind->second;
    }

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify store info received and session id is " << strSid << " and user id is " << strUserID << " and store id is " << strStoreID
        << " and store name is " << strStoreName << " and goods category is " << strGoodsCategory
        << " and extend is " << strExtend << " and address is " << strAddress);

    StoreInfo store;
    store.m_strAddress = strAddress;
    store.m_strExtend = strExtend;
    store.m_strGoodsCategory = strGoodsCategory;
    store.m_strStoreName = strStoreName;
    store.m_strStoreID = strStoreID;

    if (!ModifyStore(strSid, strUserID, store))
    {
        LOG_ERROR_RLD("Modify store handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsStoreInfoList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsStoreInfoList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["entrance"] = jsStoreInfoList;

            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }

    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query store info received and session id is " << strSid << " and user id is " << strUserID << " and store id is " << strStoreID);

    std::list<EntranceInfo> einfo;
    StoreInfo sinfo;
    sinfo.m_strStoreID = strStoreID;

    if (!QueryStore(strSid, strUserID, sinfo, einfo))
    {
        LOG_ERROR_RLD("Query store handle failed");
        return blResult;
    }

    auto itBegin = einfo.begin();
    auto itEnd = einfo.end();
    while (itBegin != itEnd)
    {
        Json::Value jsEntrance;
        jsEntrance["entrance_name"] = itBegin->m_strName;
        jsEntrance["entrance_id"] = itBegin->m_strID;
        
        Json::Value jsDevid;
        unsigned int i = 0;
        auto itB1 = itBegin->m_DeviceIDList.begin();
        auto itE1 = itBegin->m_DeviceIDList.end();
        while (itB1 != itE1)
        {
            jsDevid[i] = *itB1;

            ++itB1;
            ++i;
        }

        jsEntrance["device_id"] = jsDevid;

        jsStoreInfoList.append(jsEntrance);

        ++itBegin;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("store_id", sinfo.m_strStoreID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("store_name", sinfo.m_strStoreName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("goods_category", sinfo.m_strGoodsCategory));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("address", sinfo.m_strAddress));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("create_date", sinfo.m_strCreateDate));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("extend", sinfo.m_strExtend));

    blResult = true;

    return blResult;

}

bool PassengerFlowMsgHandler::QueryAllStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsStoreInfoList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsStoreInfoList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["store"] = jsStoreInfoList;

            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }

    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    unsigned int uiBeginIndex = 0;
    itFind = pMsgInfoMap->find("beginindex");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiBeginIndex = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Begin index is invalid and error msg is " << e.what() << " and input index is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Begin index is invalid and input index is " << itFind->second);
            return blResult;
        }
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query all store info received and session id is " << strSid << " and user id is " << strUserID << " and begin index is " << uiBeginIndex);

    std::list<StoreInfo> storelist;

    if (!QueryAllStore(strSid, strUserID, uiBeginIndex, storelist))
    {
        LOG_ERROR_RLD("Query all store handle failed");
        return blResult;
    }

    auto itBegin = storelist.begin();
    auto itEnd = storelist.end();
    while (itBegin != itEnd)
    {
        Json::Value jsStore;
        jsStore["store_id"] = itBegin->m_strStoreID;
        jsStore["store_name"] = itBegin->m_strStoreName;
        //jsStore["create_date"] = itBegin->m_strCreateDate;

        jsStoreInfoList.append(jsStore);

        ++itBegin;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::AddEntranceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;

    itFind = pMsgInfoMap->find("entrance_name");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Entrance name not found.");
        return blResult;
    }
    const std::string strEntranceName = itFind->second;

    std::string strDevIDInfo;
    itFind = pMsgInfoMap->find("deviceid");
    if (pMsgInfoMap->end() != itFind)
    {
        strDevIDInfo = itFind->second;
    }

    EntranceInfo einfo;
    einfo.m_strName = strEntranceName;

    if (!strDevIDInfo.empty())
    {
        if (!GetEntrance(strDevIDInfo, einfo))
        {
            LOG_ERROR_RLD("Get entrance failed");
            return blResult;
        }
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Add entrance info received and session id is " << strSid << " and user id is " << strUserID << " and entrance name is " << strEntranceName
        << " and device id is " << strDevIDInfo);
        
    if (!AddEntrance(strSid, strUserID, strStoreID, einfo))
    {
        LOG_ERROR_RLD("Add entrance info handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("entrance_id", einfo.m_strID));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::DelEntranceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END
    
    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;

    itFind = pMsgInfoMap->find("entranceid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Entrance id not found.");
        return blResult;
    }
    const std::string strEntranceID = itFind->second;
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete entrance info received and session id is " << strSid << " and user id is " << strUserID << " and store id is " << strStoreID
        << " and entrance id is " << strEntranceID);

    if (!DelEntrance(strSid, strUserID, strStoreID, strEntranceID))
    {
        LOG_ERROR_RLD("Delete entrance info handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::ModifyEntranceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;

    itFind = pMsgInfoMap->find("entranceid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Entrance id not found.");
        return blResult;
    }
    const std::string strEntranceID = itFind->second;

    std::string strEntranceName;
    itFind = pMsgInfoMap->find("entrance_name");
    if (pMsgInfoMap->end() != itFind)
    {
        strEntranceName = itFind->second;
    }

    std::string strAddDevIDInfo;
    itFind = pMsgInfoMap->find("added_deviceid");
    if (pMsgInfoMap->end() != itFind)
    {
        strAddDevIDInfo = itFind->second;
    }

    EntranceInfo einfoadd;
    if (!strAddDevIDInfo.empty())
    {
        if (!GetEntrance(strAddDevIDInfo, einfoadd))
        {
            LOG_ERROR_RLD("Get entrance failed");
            return blResult;
        }
    }

    std::string strDelDevIDInfo;
    itFind = pMsgInfoMap->find("deleted_deviceid");
    if (pMsgInfoMap->end() != itFind)
    {
        strDelDevIDInfo = itFind->second;
    }

    EntranceInfo einfodel;    
    if (!strDelDevIDInfo.empty())
    {
        if (!GetEntrance(strDelDevIDInfo, einfodel))
        {
            LOG_ERROR_RLD("Get entrance failed");
            return blResult;
        }
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify entrance info received and session id is " << strSid << " and user id is " << strUserID 
        << " and entrance id is " << strEntranceID << " and entrance name is " << strEntranceName
        << " and added device id is " << strAddDevIDInfo << " and deleted device id is " << strDelDevIDInfo);

    if (!ModifyEntrance(strSid, strUserID, strStoreID, strEntranceID, strEntranceName, einfoadd, einfodel))
    {
        LOG_ERROR_RLD("Modify entrance info handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::BindEntranceDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;

    itFind = pMsgInfoMap->find("entranceid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Entrance id not found.");
        return blResult;
    }
    const std::string strEntranceID = itFind->second;

    itFind = pMsgInfoMap->find("deviceid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Bind entrance device info received and session id is " << strSid << " and user id is " << strUserID << " and store id is " << strStoreID
        << " and entrance id is " << strEntranceID << " and device id is " << strDevID);

    if (!BindEntranceDevice(strSid, strUserID, strStoreID, strEntranceID, strDevID))
    {
        LOG_ERROR_RLD("Bind entrance device handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::UnBindEntranceDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;

    itFind = pMsgInfoMap->find("entranceid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Entrance id not found.");
        return blResult;
    }
    const std::string strEntranceID = itFind->second;

    itFind = pMsgInfoMap->find("deviceid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Unbind entrance device info received and session id is " << strSid << " and user id is " << strUserID << " and store id is " << strStoreID
        << " and entrance id is " << strEntranceID << " and device id is " << strDevID);

    if (!UnbindEntranceDevice(strSid, strUserID, strStoreID, strEntranceID, strDevID))
    {
        LOG_ERROR_RLD("Unbind entrance device handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::ImportPosDataHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;

    itFind = pMsgInfoMap->find("deal_amount");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Deal amount not found.");
        return blResult;
    }
    const std::string strDealAmount = itFind->second;

    double dDealAmount = 0;
    if (!ValidType<double>(strDealAmount, dDealAmount))
    {
        LOG_ERROR_RLD("Deal amount is invalid and value is " << strDealAmount);
        return blResult;
    }

    itFind = pMsgInfoMap->find("order_amount");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Order amount not found.");
        return blResult;
    }
    const std::string strOrderAmount = itFind->second;

    unsigned int uiOrderAmount = 0;
    if (!ValidType<unsigned int>(strOrderAmount, uiOrderAmount))
    {
        LOG_ERROR_RLD("Order amount is invalid and value is " << strOrderAmount);
        return blResult;
    }

    itFind = pMsgInfoMap->find("goods_amount");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Goods amount not found.");
        return blResult;
    }
    const std::string strGoodsAmount = itFind->second;

    unsigned int uiGoodsAmount = 0;
    if (!ValidType<unsigned int>(strGoodsAmount, uiGoodsAmount))
    {
        LOG_ERROR_RLD("Goods amount is invalid and value is " << strGoodsAmount);
        return blResult;
    }

    itFind = pMsgInfoMap->find("deal_date");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Deal date not found.");
        return blResult;
    }
    const std::string strDealDate = itFind->second;

    if (!ValidDatetime(strDealDate))
    {
        LOG_ERROR_RLD("Deal date is invalid and value is " << strDealDate);
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Import pos data info received and session id is " << strSid << " and user id is " << strUserID << " and store id is " << strStoreID
        << " and deal amount is " << strDealAmount << " and order amount is " << strOrderAmount << " and goods amount is " << strGoodsAmount
        << " and deal date is " << strDealDate);

    if (!ImportPosData(strSid, strUserID, strStoreID, dDealAmount, uiOrderAmount, uiGoodsAmount, strDealDate))
    {
        LOG_ERROR_RLD("Import pos data handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryPassengerFlowReportHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsChartData;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsChartData)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["chart_data"] = jsChartData;
            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;

    itFind = pMsgInfoMap->find("begin_date");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Begin date not found.");
        return blResult;
    }
    const std::string strBeginDate = itFind->second;

    if (!ValidDatetime(strBeginDate))
    {
        LOG_ERROR_RLD("Begin data is invalid and value is " << strBeginDate);
        return blResult;
    }

    itFind = pMsgInfoMap->find("end_date");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("End date not found.");
        return blResult;
    }
    const std::string strEndDate = itFind->second;

    if (!ValidDatetime(strEndDate))
    {
        LOG_ERROR_RLD("End data is invalid and value is " << strEndDate);
        return blResult;
    }

    itFind = pMsgInfoMap->find("time_precision");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Time precision not found.");
        return blResult;
    }
    const std::string strTimePrecision = itFind->second;

    unsigned int uiTimePrecision = 0;
    if (!ValidType(strTimePrecision, uiTimePrecision))
    {
        LOG_ERROR_RLD("Time precision is invalid and value is " << strTimePrecision);
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query passenger flow report info received and session id is " << strSid << " and user id is " << strUserID << " and store id is " << strStoreID
        << " and begin date is " << strBeginDate << " and end date is " << strEndDate << " and time precision is " << strTimePrecision);

    std::string strChartData;
    if (!QueryPassengerFlowReport(strSid, strUserID, strStoreID, strBeginDate, strEndDate, uiTimePrecision, strChartData))
    {
        LOG_ERROR_RLD("Query passenger flow report handle failed");
        return blResult;
    }

    Json::Reader reader;
    if (!reader.parse(strChartData, jsChartData, false))
    {
        LOG_ERROR_RLD("Parsed failed and value is " << strChartData);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    //ResultInfoMap.insert(std::map<std::string, std::string>::value_type("chart_data", strChartData));
    
    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::ReportEventHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    std::string strUserID;
    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() != itFind)
    {
        strUserID = itFind->second;
    }
    
    std::string strDevID;
    itFind = pMsgInfoMap->find("deviceid");
    if (pMsgInfoMap->end() == itFind)
    {
        strDevID = itFind->second;
    }
    
    if (strUserID.empty() && strDevID.empty())
    {
        LOG_ERROR_RLD("User id and device id not found.");
        return blResult;
    }

    itFind = pMsgInfoMap->find("source");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Source not found.");
        return blResult;
    }
    const std::string strSource = itFind->second;

    itFind = pMsgInfoMap->find("submit_date");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Submit date not found.");
        return blResult;
    }
    const std::string strSubmitDate = itFind->second;

    if (!ValidDatetime(strSubmitDate))
    {
        LOG_ERROR_RLD("Submit date is invalid and value is " << strSubmitDate);
        return blResult;
    }

    itFind = pMsgInfoMap->find("expire_date");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Expire date not found.");
        return blResult;
    }
    const std::string strExpireDate = itFind->second;

    if (!ValidDatetime(strExpireDate))
    {
        LOG_ERROR_RLD("Expire date is invalid and value is " << strExpireDate);
        return blResult;
    }

    itFind = pMsgInfoMap->find("event_type");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Event type not found.");
        return blResult;
    }
    const std::string strEventType = itFind->second;

    std::list<std::string> strEventTypeList;
    if (!GetValueList(strEventType, strEventTypeList))
    {
        LOG_ERROR_RLD("Event type list parse failed.");
        return blResult;
    }

    itFind = pMsgInfoMap->find("handler");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Event handler not found.");
        return blResult;
    }
    const std::string strEventHandler = itFind->second;

    std::list<std::string> strEventHandlerList;
    if (!GetValueList(strEventHandler, strEventHandlerList))
    {
        LOG_ERROR_RLD("Event handler list parse failed.");
        return blResult;
    }

    itFind = pMsgInfoMap->find("remark");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Remark not found.");
        return blResult;
    }
    const std::string strRemark = itFind->second;

    EventInfo einfo;
    einfo.m_strDevID = strDevID;
    einfo.m_strEventHandlerList.swap(strEventHandlerList);
    einfo.m_strExpireDate = strExpireDate;
    einfo.m_strRemark = strRemark;
    einfo.m_strSource = strSource;
    einfo.m_strSubmitDate = strSubmitDate;
    einfo.m_strUserID = strUserID;

    auto itBegin = strEventTypeList.begin();
    auto itEnd = strEventTypeList.end();
    while (itBegin != itEnd)
    {
        unsigned int uiType = 0;
        if (!ValidType<unsigned int>(*itBegin, uiType))
        {
            LOG_ERROR_RLD("Event type is invalid and value is " << *itBegin);
            return blResult;
        }
        
        einfo.m_uiEventTypeList.push_back(uiType);

        ++itBegin;
    }
    
    if (!ReportEvent(strSid, einfo))
    {
        LOG_ERROR_RLD("Report event failed.");
        return blResult;
    }
            
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Report event info received and session id is " << strSid << " and device id is " << strDevID << " and user id is " << strUserID
        << " and event id  is " << einfo.m_strEventID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("event_id", einfo.m_strEventID));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::DeleteEventHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("eventid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Event id not found.");
        return blResult;
    }
    const std::string strEventID = itFind->second;
    
    if (!DeleteEvent(strSid, strUserID, strEventID))
    {
        LOG_ERROR_RLD("Delete event failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete event info received and session id is " << strSid << " and user id is " << strUserID
        << " and event id  is " << strEventID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::ModifyEventHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;
        
    itFind = pMsgInfoMap->find("eventid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Event id not found.");
        return blResult;
    }
    const std::string strEventID = itFind->second;
    
    std::string strSource;
    itFind = pMsgInfoMap->find("source");
    if (pMsgInfoMap->end() != itFind)
    {
        strSource = itFind->second;
    }
    
    std::string strSubmitDate;
    itFind = pMsgInfoMap->find("submit_date");
    if (pMsgInfoMap->end() != itFind)
    {
        strSubmitDate = itFind->second;
        if (!ValidDatetime(strSubmitDate))
        {
            LOG_ERROR_RLD("Submit date is invalid and value is " << strSubmitDate);
            return blResult;
        }
    }
    
    std::string strExpireDate;
    itFind = pMsgInfoMap->find("expire_date");
    if (pMsgInfoMap->end() != itFind)
    {
        strExpireDate = itFind->second;
        if (!ValidDatetime(strExpireDate))
        {
            LOG_ERROR_RLD("Expire date is invalid and value is " << strExpireDate);
            return blResult;
        }
    }

    std::string strProcessState;
    itFind = pMsgInfoMap->find("process_state");
    if (pMsgInfoMap->end() != itFind)
    {
        strProcessState = itFind->second;
    }

    std::list<std::string> strEventTypeList;
    std::string strEventType;
    itFind = pMsgInfoMap->find("event_type");
    if (pMsgInfoMap->end() != itFind)
    {
        strEventType = itFind->second;
        if (!GetValueList(strEventType, strEventTypeList))
        {
            LOG_ERROR_RLD("Event type list parse failed.");
            return blResult;
        }
    }

    std::list<std::string> strEventHandlerList;
    std::string strEventHandler;
    itFind = pMsgInfoMap->find("handler");
    if (pMsgInfoMap->end() != itFind)
    {
        strEventHandler = itFind->second;
        if (!GetValueList(strEventHandler, strEventHandlerList))
        {
            LOG_ERROR_RLD("Event handler list parse failed.");
            return blResult;
        }
    }

    itFind = pMsgInfoMap->find("remark");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Remark not found.");
        return blResult;
    }
    const std::string strRemark = itFind->second;

    EventInfo einfo;
    //einfo.m_strDevID = strDevID;
    einfo.m_strEventHandlerList.swap(strEventHandlerList);
    einfo.m_strEventID = strEventID;
    einfo.m_strExpireDate = strExpireDate;
    einfo.m_strRemark = strRemark;
    einfo.m_strSource = strSource;
    einfo.m_strSubmitDate = strSubmitDate;
    einfo.m_strUserID = strUserID;
    einfo.m_strProcessState = strProcessState;

    auto itBegin = strEventTypeList.begin();
    auto itEnd = strEventTypeList.end();
    while (itBegin != itEnd)
    {
        unsigned int uiType = 0;
        if (!ValidType<unsigned int>(*itBegin, uiType))
        {
            LOG_ERROR_RLD("Event type is invalid and value is " << *itBegin);
            return blResult;
        }

        einfo.m_uiEventTypeList.push_back(uiType);

        ++itBegin;
    }

    if (!ModifyEvent(strSid, einfo))
    {
        LOG_ERROR_RLD("Modify event failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify event info received and session id is " << strSid << " and user id is " << strUserID
        << " and event id  is " << einfo.m_strEventID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;

}

bool PassengerFlowMsgHandler::QueryEventHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsEventTypeList;
    Json::Value jsEventHandlerList;
    Json::Value jsRemark;
    
    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsEventTypeList, &jsEventHandlerList, &jsRemark)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["event_type"] = jsEventTypeList;
                (*pJsBody)["handler"] = jsEventHandlerList;
                (*pJsBody)["remark"] = jsRemark;
            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("eventid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Event id not found.");
        return blResult;
    }
    const std::string strEventID = itFind->second;

    EventInfo einfo;
    einfo.m_strUserID = strUserID;
    einfo.m_strEventID = strEventID;

    if (!QueryEvent(strSid, einfo))
    {
        LOG_ERROR_RLD("Query event failed.");
        return blResult;
    }

    unsigned int i = 0;
    for (auto itBegin = einfo.m_uiEventTypeList.begin(), itEnd = einfo.m_uiEventTypeList.end(); itBegin != itEnd; ++itBegin, ++i)
    {
        jsEventTypeList[i] = boost::lexical_cast<std::string>(*itBegin);
    }

    i = 0;
    for (auto itBegin = einfo.m_strEventHandlerList.begin(), itEnd = einfo.m_strEventHandlerList.end(); itBegin != itEnd; ++itBegin, ++i)
    {
        jsEventHandlerList[i] = *itBegin;
    }

    Json::Reader reader;
    if (!reader.parse(einfo.m_strRemark, jsRemark, false))
    {
        LOG_ERROR_RLD("Parsed failed and value is " << einfo.m_strRemark);
        return blResult;
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query event info received and session id is " << strSid << " and user id is " << strUserID
        << " and event id  is " << strEventID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("source", einfo.m_strSource));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("submit_date", einfo.m_strSubmitDate));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("expire_date", einfo.m_strExpireDate));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("process_state", einfo.m_strProcessState));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("create_date", einfo.m_strCreateDate));
    //ResultInfoMap.insert(std::map<std::string, std::string>::value_type("remark", einfo.m_strRemark));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("userid", einfo.m_strUserID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("deviceid", einfo.m_strDevID));
        
    blResult = true;

    return blResult;

}

bool PassengerFlowMsgHandler::QueryAllEventHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsEventInfoList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsEventInfoList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["event"] = jsEventInfoList;
            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }

    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    unsigned int uiBeginIndex = 0;
    itFind = pMsgInfoMap->find("beginindex");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiBeginIndex = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast &e)
        {
            LOG_ERROR_RLD("Begin index is invalid and error msg is " << e.what() << " and input index is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Begin index is invalid and input index is " << itFind->second);
            return blResult;
        }
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query all event info received and session id is " << strSid << " and user id is " << strUserID << " and begin index is " << uiBeginIndex);

    std::list<EventInfo> einfoList;    
    if (!QueryAllEvent(strSid, strUserID, uiBeginIndex, einfoList))
    {
        LOG_ERROR_RLD("Query all event handle failed");
        return blResult;
    }

    auto itBegin = einfoList.begin();
    auto itEnd = einfoList.end();
    while (itBegin != itEnd)
    {
        Json::Value jsRemark;
        Json::Reader reader;
        if (!reader.parse(itBegin->m_strRemark, jsRemark, false))
        {
            LOG_ERROR_RLD("Parsed failed and value is " << itBegin->m_strRemark);
            return blResult;
        }

        Json::Value jsEventInfo;
        jsEventInfo["event_id"] = itBegin->m_strEventID;
        jsEventInfo["source"] = itBegin->m_strSource;
        jsEventInfo["submit_date"] = itBegin->m_strSubmitDate;
        jsEventInfo["expire_date"] = itBegin->m_strExpireDate;
        jsEventInfo["process_state"] = itBegin->m_strProcessState;
        jsEventInfo["create_date"] = itBegin->m_strCreateDate;
        jsEventInfo["remark"] = jsRemark;
        jsEventInfo["userid"] = itBegin->m_strUserID;
        jsEventInfo["deviceid"] = itBegin->m_strDevID;

        Json::Value jsEventTypeList;
        Json::Value jsEventHandlerList;
        
        unsigned int i = 0;
        for (auto itB = itBegin->m_uiEventTypeList.begin(), itE = itBegin->m_uiEventTypeList.end(); itB != itE; ++itB, ++i)
        {
            jsEventTypeList[i] = boost::lexical_cast<std::string>(*itB);
        }

        i = 0;
        for (auto itB = itBegin->m_strEventHandlerList.begin(), itE = itBegin->m_strEventHandlerList.end(); itB != itE; ++itB, ++i)
        {
            jsEventHandlerList[i] = *itB;
        }

        jsEventInfo["event_type"] = jsEventTypeList;
        jsEventInfo["handler"] = jsEventHandlerList;

        jsEventInfoList.append(jsEventInfo);

        ++itBegin;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::CreateGuardStorePlanHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;

    itFind = pMsgInfoMap->find("entranceid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Entrance id not found.");
        return blResult;
    }
    const std::string strEntranceID = itFind->second;

    std::list<std::string> strEntranceIDList;
    if (!GetValueList(strEntranceID, strEntranceIDList))
    {
        LOG_ERROR_RLD("Entrance id list parse failed");
        return blResult;
    }

    itFind = pMsgInfoMap->find("plan_name");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Plan name not found.");
        return blResult;
    }
    const std::string strPlanName = itFind->second;

    itFind = pMsgInfoMap->find("enable");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Enable not found.");
        return blResult;
    }
    std::string strEnable = itFind->second;
    boost::to_lower(strEnable);
    if (strEnable != "yes" && strEnable != "no")
    {
        LOG_ERROR_RLD("Enable string format is invalid and value is " << strEnable);
        return blResult;
    }
    
    itFind = pMsgInfoMap->find("begin_time");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Begin time not found.");
        return blResult;
    }
    const std::string strBeginTime = itFind->second;

    if (!ValidDatetime(strBeginTime, true))
    {
        LOG_ERROR_RLD("Begin time is invalid and value is " << strBeginTime);
        return blResult;
    }

    itFind = pMsgInfoMap->find("end_time");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("End time not found.");
        return blResult;
    }
    const std::string strEndTime = itFind->second;

    if (!ValidDatetime(strEndTime, true))
    {
        LOG_ERROR_RLD("End time is invalid and value is " << strEndTime);
        return blResult;
    }

    std::string strBeginTime2;
    itFind = pMsgInfoMap->find("begin_time2");
    if (pMsgInfoMap->end() != itFind)
    {
        strBeginTime2 = itFind->second;
        if (!ValidDatetime(strBeginTime2, true))
        {
            LOG_ERROR_RLD("Begin time2 is invalid and value is " << strBeginTime2);
            return blResult;
        }
    }
    
    std::string strEndTime2;
    itFind = pMsgInfoMap->find("end_time2");
    if (pMsgInfoMap->end() != itFind)
    {
        strEndTime2 = itFind->second;
        if (!ValidDatetime(strEndTime2, true))
        {
            LOG_ERROR_RLD("End time2 is invalid and value is " << strEndTime2);
            return blResult;
        }
    }
    
    Plan plan;
    plan.m_strBeginTime = strBeginTime;
    plan.m_strBeginTime2 = strBeginTime2;
    plan.m_strEnable = strEnable;
    plan.m_strEndTime = strEndTime;
    plan.m_strEndTime2 = strEndTime2;
    plan.m_strEntranceIDList.swap(strEntranceIDList);
    plan.m_strPlanName = strPlanName;
    plan.m_strStoreID = strStoreID;
    plan.m_strUserID = strUserID;
    
    if (!CreateGuardStorePlan(strSid, plan))
    {
        LOG_ERROR_RLD("Create guard store plan failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Create guard store plan info received and session id is " << strSid << " and store id is " << strStoreID << " and user id is " << strUserID
        << " and plan name is " << strPlanName << " and plan id is " << plan.m_strPlanID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("plan_id", plan.m_strPlanID));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::DeleteGuardStorePlanHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("planid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Plan id not found.");
        return blResult;
    }
    const std::string strPlanID = itFind->second;

    if (!DeleteGuardStorePlan(strSid, strUserID, strPlanID))
    {
        LOG_ERROR_RLD("Delete plan failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete guard plan info received and session id is " << strSid << " and user id is " << strUserID
        << " and plan id  is " << strPlanID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;

}

bool PassengerFlowMsgHandler::ModifyGuardStorePlanHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("planid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Plan id not found.");
        return blResult;
    }
    const std::string strPlanID = itFind->second;

    std::list<std::string> strEntranceIDList;
    std::string strEntranceID;
    itFind = pMsgInfoMap->find("entranceid");
    if (pMsgInfoMap->end() != itFind)
    {
        strEntranceID = itFind->second;
        if (!GetValueList(strEntranceID, strEntranceIDList))
        {
            LOG_ERROR_RLD("Entrance id list parse failed");
            return blResult;
        }
    }
    
    std::string strPlanName;
    itFind = pMsgInfoMap->find("plan_name");
    if (pMsgInfoMap->end() != itFind)
    {
        strPlanName = itFind->second;
    }
    
    std::string strEnable;
    itFind = pMsgInfoMap->find("enable");
    if (pMsgInfoMap->end() != itFind)
    {
        strEnable = itFind->second;
        boost::to_lower(strEnable);
        if (strEnable != "yes" && strEnable != "no")
        {
            LOG_ERROR_RLD("Enable string format is invalid and value is " << strEnable);
            return blResult;
        }
    }
    
    std::string strBeginTime;
    itFind = pMsgInfoMap->find("begin_time");
    if (pMsgInfoMap->end() != itFind)
    {
        strBeginTime = itFind->second;
        if (!ValidDatetime(strBeginTime, true))
        {
            LOG_ERROR_RLD("Begin time is invalid and value is " << strBeginTime);
            return blResult;
        }
    }
    
    std::string strEndTime;
    itFind = pMsgInfoMap->find("end_time");
    if (pMsgInfoMap->end() != itFind)
    {
        strEndTime = itFind->second;
        if (!ValidDatetime(strEndTime, true))
        {
            LOG_ERROR_RLD("End time is invalid and value is " << strEndTime);
            return blResult;
        }
    }
    
    std::string strBeginTime2;
    itFind = pMsgInfoMap->find("begin_time2");
    if (pMsgInfoMap->end() != itFind)
    {
        strBeginTime2 = itFind->second;
        if (!ValidDatetime(strBeginTime2, true))
        {
            LOG_ERROR_RLD("Begin time2 is invalid and value is " << strBeginTime2);
            return blResult;
        }
    }

    std::string strEndTime2;
    itFind = pMsgInfoMap->find("end_time2");
    if (pMsgInfoMap->end() != itFind)
    {
        strEndTime2 = itFind->second;
        if (!ValidDatetime(strEndTime2, true))
        {
            LOG_ERROR_RLD("End time2 is invalid and value is " << strEndTime2);
            return blResult;
        }
    }

    Plan plan;
    plan.m_strBeginTime = strBeginTime;
    plan.m_strBeginTime2 = strBeginTime2;
    plan.m_strEnable = strEnable;
    plan.m_strEndTime = strEndTime;
    plan.m_strEndTime2 = strEndTime2;
    plan.m_strEntranceIDList.swap(strEntranceIDList);
    plan.m_strPlanName = strPlanName;
    plan.m_strPlanID = strPlanID;
    plan.m_strUserID = strUserID;

    if (!ModifyGuardStorePlan(strSid, plan))
    {
        LOG_ERROR_RLD("Modify guard store plan failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify guard store plan info received and session id is " << strSid << " and plan id is " << strPlanID << " and user id is " << strUserID
        << " and plan name is " << strPlanName);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryGuardStorePlanHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsEntranceList;
    
    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsEntranceList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["entrance"] = jsEntranceList;                
            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("planid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Plan id not found.");
        return blResult;
    }
    const std::string strPlanID = itFind->second;

    Plan plan;
    plan.m_strUserID = strUserID;
    plan.m_strPlanID = strPlanID;

    if (!QueryGuardStorePlan(strSid, plan))
    {
        LOG_ERROR_RLD("Query plan failed.");
        return blResult;
    }

    unsigned int i = 0;
    for (auto itBegin = plan.m_strEntranceIDList.begin(), itEnd = plan.m_strEntranceIDList.end(); itBegin != itEnd; ++itBegin, ++i)
    {
        jsEntranceList[i] = boost::lexical_cast<std::string>(*itBegin);
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query plan info received and session id is " << strSid << " and user id is " << strUserID
        << " and plan id  is " << strPlanID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("store_id", plan.m_strStoreID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("store_name", plan.m_strStoreName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("plan_name", plan.m_strPlanName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("plan_id", plan.m_strPlanID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("enable", plan.m_strEnable));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("begin_time", plan.m_strBeginTime));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("end_time", plan.m_strEndTime));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("begin_time2", plan.m_strBeginTime2));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("end_time2", plan.m_strEndTime2));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryAllGuardStorePlanHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsPlanInfoList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsPlanInfoList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["guard_store"] = jsPlanInfoList;

            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }

    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    std::string strUserID;
    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() != itFind)
    {
        strUserID = itFind->second;
    }

    std::string strDevID;
    itFind = pMsgInfoMap->find("deviceid");
    if (pMsgInfoMap->end() != itFind)
    {
        strDevID = itFind->second;
    }

    if (strUserID.empty() && strDevID.empty())
    {
        LOG_ERROR_RLD("User id and device id not found.");
        return blResult;
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query all plan info received and session id is " << strSid << " and user id is " << strUserID << " and device id is " << strDevID);

    std::list<Plan> planinfoList;
    if (!QueryAllGuardStorePlan(strSid, strUserID, strDevID, planinfoList))
    {
        LOG_ERROR_RLD("Query all plan handle failed");
        return blResult;
    }

    auto itBegin = planinfoList.begin();
    auto itEnd = planinfoList.end();
    while (itBegin != itEnd)
    {
        Json::Value jsPlanInfo;
        jsPlanInfo["store_id"] = itBegin->m_strStoreID;
        jsPlanInfo["store_name"] = itBegin->m_strStoreName;
        jsPlanInfo["plan_id"] = itBegin->m_strPlanID;
        jsPlanInfo["plan_name"] = itBegin->m_strPlanName;
        jsPlanInfo["enable"] = itBegin->m_strEnable;
        jsPlanInfo["begin_time"] = itBegin->m_strBeginTime;
        jsPlanInfo["end_time"] = itBegin->m_strEndTime;
        jsPlanInfo["begin_time2"] = itBegin->m_strBeginTime2;
        jsPlanInfo["end_time2"] = itBegin->m_strEndTime2;

        Json::Value jsEntranceIDList;

        unsigned int i = 0;
        for (auto itB = itBegin->m_strEntranceIDList.begin(), itE = itBegin->m_strEntranceIDList.end(); itB != itE; ++itB, ++i)
        {
            jsEntranceIDList[i] = *itB;
        }

        jsPlanInfo["entrance"] = jsEntranceIDList;

        jsPlanInfoList.append(jsPlanInfo);

        ++itBegin;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::CreateRegularPatrolHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("plan_name");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Plan name not found.");
        return blResult;
    }
    const std::string strPatrolName = itFind->second;

    itFind = pMsgInfoMap->find("enable");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Enable not found.");
        return blResult;
    }
    std::string strEnable = itFind->second;
    boost::to_lower(strEnable);
    if (strEnable != "yes" && strEnable != "no")
    {
        LOG_ERROR_RLD("Enable string format is invalid and value is " << strEnable);
        return blResult;
    }

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;

    std::list<std::string> strStoreIDList;
    if (!GetValueList(strStoreID, strStoreIDList))
    {
        LOG_ERROR_RLD("Store id list parse failed");
        return blResult;
    }

    itFind = pMsgInfoMap->find("patrol_time");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Patrol time not found.");
        return blResult;
    }
    const std::string strPatrolTime = itFind->second;

    std::list<std::string> strPatrolTimeList;
    if (!GetValueList(strPatrolTime, strPatrolTimeList))
    {
        LOG_ERROR_RLD("Patrol time list parse failed");
        return blResult;
    }

    for (auto itBegin = strPatrolTimeList.begin(), itEnd = strPatrolTimeList.end(); itBegin != itEnd; ++itBegin)
    {
        if (!ValidDatetime(*itBegin, true))
        {
            LOG_ERROR_RLD("Patrol time is invalid and value is " << *itBegin);
            return blResult;
        }
    }

    Patrol pat;
    pat.m_strEnable = strEnable;
    pat.m_strPatrolName = strPatrolName;
    pat.m_strPatrolTimeList.swap(strPatrolTimeList);
    pat.m_strStoreIDList.swap(strStoreIDList);
    pat.m_strUserID = strUserID;

    if (!CreateRegularPatrol(strSid, pat))
    {
        LOG_ERROR_RLD("Create regular patrol failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Create regular patrol info received and session id is " << strSid << " and patrol id is " << pat.m_strPatrolID << " and user id is " << strUserID
        << " and patrol name is " << strPatrolName);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("plan_id", pat.m_strPatrolID));

    blResult = true;

    return blResult;

}

bool PassengerFlowMsgHandler::DeleteRegularPatrolPlanHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("planid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Plan id not found.");
        return blResult;
    }
    const std::string strPlanID = itFind->second;

    if (!DeleteRegularPatrol(strSid, strUserID, strPlanID))
    {
        LOG_ERROR_RLD("Delete regular patrol failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete regular patrol info received and session id is " << strSid << " and user id is " << strUserID
        << " and plan id  is " << strPlanID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::ModifyRegularPatrolPlanHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("planid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Plan id not found.");
        return blResult;
    }
    const std::string strPatrolID = itFind->second;

    std::string strPatrolName;
    itFind = pMsgInfoMap->find("plan_name");
    if (pMsgInfoMap->end() != itFind)
    {
        strPatrolName = itFind->second;
    }
    
    std::string strEnable;
    itFind = pMsgInfoMap->find("enable");
    if (pMsgInfoMap->end() != itFind)
    {
        strEnable = itFind->second;
        boost::to_lower(strEnable);
        if (strEnable != "yes" && strEnable != "no")
        {
            LOG_ERROR_RLD("Enable string format is invalid and value is " << strEnable);
            return blResult;
        }
    }
    
    std::list<std::string> strStoreIDList;
    std::string strStoreID;
    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() != itFind)
    {
        strStoreID = itFind->second;
        if (!GetValueList(strStoreID, strStoreIDList))
        {
            LOG_ERROR_RLD("Store id list parse failed");
            return blResult;
        }
    }
    
    std::list<std::string> strPatrolTimeList;
    std::string strPatrolTime;
    itFind = pMsgInfoMap->find("patrol_time");
    if (pMsgInfoMap->end() != itFind)
    {
        strPatrolTime = itFind->second;
        if (!GetValueList(strPatrolTime, strPatrolTimeList))
        {
            LOG_ERROR_RLD("Patrol time list parse failed");
            return blResult;
        }

        for (auto itBegin = strPatrolTimeList.begin(), itEnd = strPatrolTimeList.end(); itBegin != itEnd; ++itBegin)
        {
            if (!ValidDatetime(*itBegin, true))
            {
                LOG_ERROR_RLD("Patrol time is invalid and value is " << *itBegin);
                return blResult;
            }
        }
    }
    
    Patrol pat;
    pat.m_strEnable = strEnable;
    pat.m_strPatrolName = strPatrolName;
    pat.m_strPatrolTimeList.swap(strPatrolTimeList);
    pat.m_strStoreIDList.swap(strStoreIDList);
    pat.m_strUserID = strUserID;
    pat.m_strPatrolID = strPatrolID;

    if (!ModifyRegularPatrol(strSid, pat))
    {
        LOG_ERROR_RLD("Modify regular patrol failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify regular patrol info received and session id is " << strSid << " and patrol id is " << pat.m_strPatrolID << " and user id is " << strUserID
        << " and patrol name is " << strPatrolName);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    
    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryRegularPatrolPlanHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsStoreList;
    Json::Value jsPatrolTimeList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsStoreList, &jsPatrolTimeList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["store"] = jsStoreList;
                (*pJsBody)["patrol_time"] = jsPatrolTimeList;
            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("planid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Plan id not found.");
        return blResult;
    }
    const std::string strPlanID = itFind->second;

    Patrol pat;
    pat.m_strUserID = strUserID;
    pat.m_strPatrolID = strPlanID;

    if (!QueryRegularPatrolPlan(strSid, pat))
    {
        LOG_ERROR_RLD("Query regular patrol failed.");
        return blResult;
    }

    unsigned int i = 0;
    for (auto itBegin = pat.m_strPatrolTimeList.begin(), itEnd = pat.m_strPatrolTimeList.end(); itBegin != itEnd; ++itBegin, ++i)
    {
        jsPatrolTimeList[i] = *itBegin;
    }

    Json::Reader reader;
    if (!reader.parse(pat.m_strPatrolInfo, jsStoreList, false))
    {
        LOG_ERROR_RLD("Parsed failed and value is " << pat.m_strPatrolInfo);
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query regulan patrol info received and session id is " << strSid << " and user id is " << strUserID
        << " and plan id  is " << strPlanID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("plan_id", pat.m_strPatrolID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("plan_name", pat.m_strPatrolName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("enable", pat.m_strEnable));

    blResult = true;

    return blResult;

}

bool PassengerFlowMsgHandler::QueryAllRegularPatrolPlanHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsPatrolInfoList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsPatrolInfoList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["regular_patrol"] = jsPatrolInfoList;

            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }

    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    std::string strUserID;
    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() != itFind)
    {
        strUserID = itFind->second;
    }

    std::string strDevID;
    itFind = pMsgInfoMap->find("deviceid");
    if (pMsgInfoMap->end() != itFind)
    {
        strDevID = itFind->second;
    }

    if (strUserID.empty() && strDevID.empty())
    {
        LOG_ERROR_RLD("User id and device id not found.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query all regular patrol info received and session id is " << strSid << " and user id is " << strUserID << " and device id is " << strDevID);

    std::list<Patrol> patrolinfoList;
    if (!QueryAllRegularPatrolPlan(strSid, strUserID, strDevID, patrolinfoList))
    {
        LOG_ERROR_RLD("Query all regular patrol handle failed");
        return blResult;
    }

    auto itBegin = patrolinfoList.begin();
    auto itEnd = patrolinfoList.end();
    while (itBegin != itEnd)
    {
        Json::Value jsPlanInfo;
        jsPlanInfo["plan_id"] = itBegin->m_strPatrolID;
        jsPlanInfo["plan_name"] = itBegin->m_strPatrolName;
        jsPlanInfo["enable"] = itBegin->m_strEnable;
        
        Json::Value jsPatrolTimeList;
        unsigned int i = 0;
        for (auto itB = itBegin->m_strPatrolTimeList.begin(), itE = itBegin->m_strPatrolTimeList.end(); itB != itE; ++itB, ++i)
        {
            jsPatrolTimeList[i] = *itB;
        }

        jsPlanInfo["patrol_time"] = jsPatrolTimeList;
                
        Json::Value jsStoreList;
        Json::Reader reader;
        if (!reader.parse(itBegin->m_strPatrolInfo, jsStoreList, false))
        {
            LOG_ERROR_RLD("Parsed failed and value is " << itBegin->m_strPatrolInfo);
            return blResult;
        }

        jsPlanInfo["store"] = jsStoreList;

        jsPatrolInfoList.append(jsPlanInfo);

        ++itBegin;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::CreateVIPHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("profile_picture");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Picture not found.");
        return blResult;
    }
    const std::string strPicture = itFind->second;

    itFind = pMsgInfoMap->find("cellphone");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Cell phone not found.");
        return blResult;
    }
    std::string strCellphone = itFind->second;

    itFind = pMsgInfoMap->find("vip_name");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Vip name not found.");
        return blResult;
    }
    std::string strVipName = itFind->second;

    std::string strVisitDate;
    itFind = pMsgInfoMap->find("visit_date");
    if (pMsgInfoMap->end() != itFind)
    {
        strVisitDate = itFind->second;
        if (!ValidDatetime(strVisitDate))
        {
            LOG_ERROR_RLD("Visit date is invalid and value is " << strVisitDate);
            return blResult;
        }
    }

    std::string strVisitTimes;
    unsigned int uiVisitTimes;
    itFind = pMsgInfoMap->find("visit_times");
    if (pMsgInfoMap->end() != itFind)
    {
        strVisitTimes = itFind->second;
        if (!ValidType<unsigned int>(strVisitTimes, uiVisitTimes))
        {
            LOG_ERROR_RLD("Visit times is invalid and value is " << strVisitTimes);
            return blResult;
        }
    }

    VIP vip;
    vip.m_strCellphone = strCellphone;
    vip.m_strVipName = strVipName;
    vip.m_strVipUserID = strUserID;
    vip.m_strVisitDate = strVisitDate;
    vip.m_uiVisitTimes = uiVisitTimes;
    vip.m_strProfilePicture = strPicture;

    if (!CreateVIP(strSid, vip))
    {
        LOG_ERROR_RLD("Create vip failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Create vip info received and session id is " << strSid << " and vip id is " << vip.m_strVipID << " and user id is " << strUserID
        << " and vip name is " << strVipName);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("vipid", vip.m_strVipID));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::DeleteVIPHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("vipid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Vip id not found.");
        return blResult;
    }
    const std::string strVipID = itFind->second;

    if (!DeleteVIP(strSid, strUserID, strVipID))
    {
        LOG_ERROR_RLD("Delete vip failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete vip info received and session id is " << strSid << " and user id is " << strUserID
        << " and vip id  is " << strVipID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::ModifyVIPHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("vipid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Vip id not found.");
        return blResult;
    }
    const std::string strVipID = itFind->second;

    std::string strPicture;
    itFind = pMsgInfoMap->find("profile_picture");
    if (pMsgInfoMap->end() != itFind)
    {
        strPicture = itFind->second;
    }
    
    std::string strCellphone;
    itFind = pMsgInfoMap->find("cellphone");
    if (pMsgInfoMap->end() != itFind)
    {
        strCellphone = itFind->second;
    }
    
    std::string strVipName;
    itFind = pMsgInfoMap->find("vip_name");
    if (pMsgInfoMap->end() != itFind)
    {
        strVipName = itFind->second;
    }
    
    std::string strVisitDate;
    itFind = pMsgInfoMap->find("visit_date");
    if (pMsgInfoMap->end() != itFind)
    {
        strVisitDate = itFind->second;
        if (!ValidDatetime(strVisitDate))
        {
            LOG_ERROR_RLD("Visit date is invalid and value is " << strVisitDate);
            return blResult;
        }
    }

    std::string strVisitTimes;
    unsigned int uiVisitTimes;
    itFind = pMsgInfoMap->find("visit_times");
    if (pMsgInfoMap->end() != itFind)
    {
        strVisitTimes = itFind->second;
        if (!ValidType<unsigned int>(strVisitTimes, uiVisitTimes))
        {
            LOG_ERROR_RLD("Visit times is invalid and value is " << strVisitTimes);
            return blResult;
        }
    }

    VIP vip;
    vip.m_strCellphone = strCellphone;
    vip.m_strVipName = strVipName;
    vip.m_strVipID = strVipID;
    vip.m_strVipUserID = strUserID;
    vip.m_strVisitDate = strVisitDate;
    vip.m_uiVisitTimes = uiVisitTimes;
    vip.m_strProfilePicture = strPicture;

    if (!ModifyVIP(strSid, vip))
    {
        LOG_ERROR_RLD("Modify vip failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify vip info received and session id is " << strSid << " and vip id is " << strVipID << " and user id is " << strUserID
        << " and vip name is " << strVipName);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryVIPHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsComHisList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsComHisList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["consume_history"] = jsComHisList;
            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("vipid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Vip id not found.");
        return blResult;
    }
    const std::string strVipID = itFind->second;

    VIP vip;
    vip.m_strVipUserID = strUserID;
    vip.m_strVipID = strVipID;

    if (!QueryVIP(strSid, vip))
    {
        LOG_ERROR_RLD("Query vip failed.");
        return blResult;
    }

    unsigned int i = 0;
    for (auto itBegin = vip.m_ConsumeHistoryList.begin(), itEnd = vip.m_ConsumeHistoryList.end(); itBegin != itEnd; ++itBegin, ++i)
    {
        Json::Value jsComHis;
        jsComHis["goods_name"] = itBegin->m_strGoodName;
        jsComHis["goods_number"] = itBegin->m_uiGoodNum;
        jsComHis["salesman"] = itBegin->m_strSalesman;
        jsComHis["consume_amount"] = boost::lexical_cast<std::string>(itBegin->m_dConsumeAmount);
        jsComHis["consume_date"] = itBegin->m_strConsumeDate;

        jsComHisList[i] = jsComHis;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query vip info received and session id is " << strSid << " and user id is " << strUserID
        << " and vip id  is " << strVipID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("profile_picture", vip.m_strProfilePicture));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("vip_name", vip.m_strVipName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("vip_id", vip.m_strVipID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("cellphone", vip.m_strCellphone));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("visit_date", vip.m_strVisitDate));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("visit_times", boost::lexical_cast<std::string>(vip.m_uiVisitTimes)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("register_date", vip.m_strRegisterDate));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryAllVIPHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsVipInfoList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsVipInfoList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["vip_customer"] = jsVipInfoList;

            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }

    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    std::string strUserID;
    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() != itFind)
    {
        strUserID = itFind->second;
    }

    unsigned int uiBeginIndex = 0;
    itFind = pMsgInfoMap->find("beginindex");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiBeginIndex = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Begin index is invalid and error msg is " << e.what() << " and input index is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Begin index is invalid and input index is " << itFind->second);
            return blResult;
        }
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query all vip info received and session id is " << strSid << " and user id is " << strUserID << " and begin index is " << uiBeginIndex);

    std::list<VIP> VipList;
    if (!QueryAllVIP(strSid, strUserID, uiBeginIndex, VipList))
    {
        LOG_ERROR_RLD("Query all vip handle failed");
        return blResult;
    }

    auto itBegin = VipList.begin();
    auto itEnd = VipList.end();
    while (itBegin != itEnd)
    {
        Json::Value jsVipInfo;
        jsVipInfo["profile_picture"] = itBegin->m_strProfilePicture;
        jsVipInfo["vip_name"] = itBegin->m_strVipName;
        jsVipInfo["vip_id"] = itBegin->m_strVipID;
        jsVipInfo["cellphone"] = itBegin->m_strCellphone;
        jsVipInfo["visit_date"] = itBegin->m_strVisitDate;
        jsVipInfo["visit_times"] = boost::lexical_cast<std::string>(itBegin->m_uiVisitTimes);
        jsVipInfo["register_date"] = itBegin->m_strRegisterDate;
        
        jsVipInfoList.append(jsVipInfo);

        ++itBegin;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::CreateVIPConsumeHistoryHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("vipid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Vip id not found.");
        return blResult;
    }
    const std::string strVipID = itFind->second;
    
    itFind = pMsgInfoMap->find("goods_name");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Goods name not found.");
        return blResult;
    }
    const std::string strGoodsName = itFind->second;

    itFind = pMsgInfoMap->find("goods_number");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Goods number not found.");
        return blResult;
    }
    
    unsigned int uiGoodsNumber;
    if (!ValidType<unsigned int>(itFind->second, uiGoodsNumber))
    {
        LOG_ERROR_RLD("Goods number is invalid and value is " << itFind->second);
        return blResult;
    }

    itFind = pMsgInfoMap->find("salesman");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sales man not found.");
        return blResult;
    }
    std::string strSalesMan = itFind->second;


    itFind = pMsgInfoMap->find("consume_amount");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Consume amount not found.");
        return blResult;
    }

    double dConsumAmount;
    if (!ValidType<double>(itFind->second, dConsumAmount))
    {
        LOG_ERROR_RLD("Consume amount is invalid and value is " << itFind->second);
        return blResult;
    }

    itFind = pMsgInfoMap->find("consume_date");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Consume date not found.");
        return blResult;
    }

    std::string strConsumeDate = itFind->second;
    if (!ValidDatetime(strConsumeDate))
    {
        LOG_ERROR_RLD("Consume date is invalid and value is " << strConsumeDate);
        return blResult;
    }
    
    VIP::ConsumeHistory vch;
    vch.m_dConsumeAmount = dConsumAmount;
    vch.m_strConsumeDate = strConsumeDate;
    vch.m_strGoodName = strGoodsName;
    vch.m_strSalesman = strSalesMan;
    vch.m_uiGoodNum = uiGoodsNumber;

    if (!CreateVIPConsumeHistory(strSid, strUserID, strVipID, vch))
    {
        LOG_ERROR_RLD("Create vip consume history failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Create vip consume history info received and session id is " << strSid << " and vip id is " << strVipID << " and user id is " << strUserID
        << " and goods name is " << strGoodsName);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("consume_id", vch.m_strConsumeID));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::DeleteVIPConsumeHistoryHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("consumeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Consume id not found.");
        return blResult;
    }
    const std::string strConsumeID = itFind->second;

    if (!DeleteVIPConsumeHistory(strSid, strUserID, strConsumeID))
    {
        LOG_ERROR_RLD("Delete consume failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete vip consume info received and session id is " << strSid << " and user id is " << strUserID
        << " and consume id  is " << strConsumeID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::ModifyVIPConsumeHistoryHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("consumeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Consume id not found.");
        return blResult;
    }
    const std::string strConsumeID = itFind->second;
    
    std::string strGoodsName;
    itFind = pMsgInfoMap->find("goods_name");
    if (pMsgInfoMap->end() != itFind)
    {
        strGoodsName = itFind->second;
    }
    
    unsigned int uiGoodsNumber = 0xFFFFFFFF;
    itFind = pMsgInfoMap->find("goods_number");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidType<unsigned int>(itFind->second, uiGoodsNumber))
        {
            LOG_ERROR_RLD("Goods number is invalid and value is " << itFind->second);
            return blResult;
        }
    }
    
    std::string strSalesMan;
    itFind = pMsgInfoMap->find("salesman");
    if (pMsgInfoMap->end() != itFind)
    {
        strSalesMan = itFind->second;
    }
    
    double dConsumAmount = -1; //0xFFFFFFFF;
    itFind = pMsgInfoMap->find("consume_amount");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidType<double>(itFind->second, dConsumAmount))
        {
            LOG_ERROR_RLD("Consume amount is invalid and value is " << itFind->second);
            return blResult;
        }
    }    
    
    std::string strConsumeDate;
    itFind = pMsgInfoMap->find("consume_date");
    if (pMsgInfoMap->end() != itFind)
    {
        strConsumeDate = itFind->second;
        if (!ValidDatetime(strConsumeDate))
        {
            LOG_ERROR_RLD("Consume date is invalid and value is " << strConsumeDate);
            return blResult;
        }
    }

    VIP::ConsumeHistory vch;
    vch.m_dConsumeAmount = dConsumAmount;
    vch.m_strConsumeDate = strConsumeDate;
    vch.m_strGoodName = strGoodsName;
    vch.m_strSalesman = strSalesMan;
    vch.m_uiGoodNum = uiGoodsNumber;
    vch.m_strConsumeID = strConsumeID;

    if (!ModifyVIPConsumeHistory(strSid, strUserID, vch))
    {
        LOG_ERROR_RLD("Modify vip consume history failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify vip consume history info received and session id is " << strSid << " and consume id is " << strConsumeID << " and user id is " << strUserID
        << " and goods name is " << strGoodsName);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryVIPConsumeHistoryHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsComHisList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsComHisList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["consume_history"] = jsComHisList;
            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("vipid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Vip id not found.");
        return blResult;
    }
    const std::string strVipID = itFind->second;

    unsigned int uiBeginIndex = 0;
    itFind = pMsgInfoMap->find("beginindex");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiBeginIndex = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Begin index is invalid and error msg is " << e.what() << " and input index is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Begin index is invalid and input index is " << itFind->second);
            return blResult;
        }
    }

    std::list<VIP::ConsumeHistory> vphlist;    
    if (!QueryVIPConsumeHistory(strSid, strUserID, strVipID, uiBeginIndex, vphlist))
    {
        LOG_ERROR_RLD("Query vip consume failed.");
        return blResult;
    }

    unsigned int i = 0;
    for (auto itBegin = vphlist.begin(), itEnd = vphlist.end(); itBegin != itEnd; ++itBegin, ++i)
    {
        Json::Value jsComHis;
        jsComHis["goods_name"] = itBegin->m_strGoodName;
        jsComHis["goods_number"] = itBegin->m_uiGoodNum;
        jsComHis["salesman"] = itBegin->m_strSalesman;
        jsComHis["consume_amount"] = boost::lexical_cast<std::string>(itBegin->m_dConsumeAmount);
        jsComHis["consume_date"] = itBegin->m_strConsumeDate;

        jsComHisList[i] = jsComHis;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query vip consume info received and session id is " << strSid << " and user id is " << strUserID
        << " and vip id  is " << strVipID << " and consume history record size is " << vphlist.size());

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::UserJoinStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;

    itFind = pMsgInfoMap->find("role");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Role not found.");
        return blResult;
    }
    const std::string strRole = itFind->second;

    
    UserOfStore us;
    us.m_strRole = strRole;
    us.m_strStoreID = strStoreID;
    us.m_strUserID = strUserID;

    if (!UserJoinStore(strSid, us))
    {
        LOG_ERROR_RLD("User join store failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("User join store info received and session id is " << strSid << " and store id is " << strStoreID << " and user id is " << strUserID
        << " and role is " << strRole);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::UserQuitStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;

    std::string strAdminID;
    itFind = pMsgInfoMap->find("administratorid");
    if (pMsgInfoMap->end() != itFind)
    {
        strAdminID = itFind->second;
    }
    
    UserOfStore us;
    us.m_strStoreID = strStoreID;
    us.m_strUserID = strUserID;

    if (!UserQuitStore(strSid, strAdminID, us))
    {
        LOG_ERROR_RLD("User quit store failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("User quit store info received and session id is " << strSid << " and store id is " << strStoreID << " and user id is " << strUserID
        << " and admin id is " << strAdminID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryUserOfStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsUserOfStoreList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsUserOfStoreList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["user"] = jsUserOfStoreList;
            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;
    
    UserOfStore us;
    us.m_strUserID = strUserID;
    us.m_strStoreID = strStoreID;

    std::list<UserOfStore> uslist;
    if (!QueryUserOfStore(strSid, us, uslist))
    {
        LOG_ERROR_RLD("Query user of store failed.");
        return blResult;
    }

    unsigned int i = 0;
    for (auto itBegin = uslist.begin(), itEnd = uslist.end(); itBegin != itEnd; ++itBegin, ++i)
    {
        Json::Value jsUserofStore;
        jsUserofStore["user_id"] = itBegin->m_strUserID;
        jsUserofStore["user_name"] = itBegin->m_strUserName;
        jsUserofStore["role"] = itBegin->m_strRole;

        jsUserOfStoreList[i] = jsUserofStore;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query user of store info received and session id is " << strSid << " and user id is " << strUserID
        << " and store id  is " << strStoreID << " and user of store record size is " << uslist.size());

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::CreateEvaluationTemplateHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("evaluation");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Evaluation not found.");
        return blResult;
    }
    const std::string strEvaluation = itFind->second;

    itFind = pMsgInfoMap->find("evaluation_desc");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Evaluation desc not found.");
        return blResult;
    }
    const std::string strEvaluationDesc = itFind->second;

    itFind = pMsgInfoMap->find("evaluation_value");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Evaluation value not found.");
        return blResult;
    }

    double dEvaluationValue = 0;
    if (!ValidType<double>(itFind->second, dEvaluationValue))
    {
        LOG_ERROR_RLD("Evaluation value is invalid and value is " << itFind->second);
        return blResult;
    }

    EvaluationTemplate evt;
    evt.m_dEvaluationValue = dEvaluationValue;
    evt.m_strEvaluation = strEvaluation;
    evt.m_strEvaluationDesc = strEvaluationDesc;
    evt.m_strUserIDOfCreataion = strUserID;    

    if (!CreateEvaluationTemplate(strSid, evt))
    {
        LOG_ERROR_RLD("Create evaluation template failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Create evaluation template info received and session id is " << strSid << " and user id is " << strUserID << " and evaluation is " << strEvaluation
        << " and evaluation desc is " << strEvaluationDesc << " and evaluation value is " << dEvaluationValue);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("evaluation_id", evt.m_strEvaluationTmpID));

    blResult = true;

    return blResult;

}

bool PassengerFlowMsgHandler::DeleteEvaluationTemplateHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("evaluationid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("evaluation id not found.");
        return blResult;
    }
    const std::string strEvaluationID = itFind->second;

    if (!DeleteEvaluationTemplate(strSid, strUserID, strEvaluationID))
    {
        LOG_ERROR_RLD("Delete evaluation template failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete evaluation template info received and session id is " << strSid << " and user id is " << strUserID
        << " and evaluation id  is " << strEvaluationID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::ModifyEvaluationTemplateHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("evaluationid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Evaluation id not found.");
        return blResult;
    }
    const std::string strEvaluationID = itFind->second;

    std::string strEvaluation;
    itFind = pMsgInfoMap->find("evaluation");
    if (pMsgInfoMap->end() != itFind)
    {
        strEvaluation = itFind->second;
    }

    std::string strEvaluationDesc;
    itFind = pMsgInfoMap->find("evaluation_desc");
    if (pMsgInfoMap->end() != itFind)
    {
        strEvaluationDesc = itFind->second;
    }
    
    double dEvaluationValue = -1;
    itFind = pMsgInfoMap->find("evaluation_value");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidType<double>(itFind->second, dEvaluationValue))
        {
            LOG_ERROR_RLD("Evaluation value is invalid and value is " << itFind->second);
            return blResult;
        }
    }

    EvaluationTemplate evt;
    evt.m_dEvaluationValue = dEvaluationValue;
    evt.m_strEvaluation = strEvaluation;
    evt.m_strEvaluationDesc = strEvaluationDesc;
    evt.m_strUserIDOfCreataion = strUserID;
    evt.m_strEvaluationTmpID = strEvaluationID;

    if (!ModifyEvaluationTemplate(strSid, evt))
    {
        LOG_ERROR_RLD("Modify evaluation template failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify evaluation template info received and session id is " << strSid << " and user id is " << strUserID << " and evaluation is " << strEvaluation
        << " and evaluation desc is " << strEvaluationDesc << " and evaluation value is " << dEvaluationValue);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryEvaluationTemplateHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsEvaList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsEvaList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["evaluation_template"] = jsEvaList;
            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;
    
    std::list<EvaluationTemplate> evtlist;
    if (!QueryEvalutaionTemplate(strSid, strUserID, evtlist))
    {
        LOG_ERROR_RLD("Query evaluation template failed.");
        return blResult;
    }

    unsigned int i = 0;
    for (auto itBegin = evtlist.begin(), itEnd = evtlist.end(); itBegin != itEnd; ++itBegin, ++i)
    {
        Json::Value jsEva;
        jsEva["evaluation"] = itBegin->m_strEvaluation;
        jsEva["evaluation_id"] = itBegin->m_strEvaluationTmpID;
        jsEva["evaluation_desc"] = itBegin->m_strEvaluationDesc;
        jsEva["evaluation_value"] = boost::lexical_cast<std::string>(itBegin->m_dEvaluationValue);
        
        jsEvaList[i] = jsEva;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query evaluation template info received and session id is " << strSid << " and user id is " << strUserID
        << " and evaluation record size is " << evtlist.size());

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::CreateEvaluationOfStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("userid_check");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id of checking not found.");
        return blResult;
    }
    const std::string strUserIDCheck = itFind->second;

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;
    
    itFind = pMsgInfoMap->find("evaluation_info");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Evaluation info not found.");
        return blResult;
    }
    const std::string strEvaluationInfo = itFind->second;
    if (strEvaluationInfo.empty())
    {
        LOG_ERROR_RLD("Evaluation info is empty.");
        return blResult;
    }
    
    Evaluation ev;
    ev.m_strUserID = strUserID;
    ev.m_strUserIDOfCheck = strUserIDCheck;
    ev.m_strStoreID = strStoreID;

    auto EvaluationParse = [&](Json::Value jsValue) ->bool
    {
        if (!jsValue.isObject())
        {
            LOG_ERROR_RLD("Evaluation parse failed");
            return blResult;
        }

        auto jsEvaluationID = jsValue["evaluation_id"];
        if (jsEvaluationID.isNull() || !jsEvaluationID.isString() || jsEvaluationID.asString().empty())
        {
            LOG_ERROR_RLD("Evaluation id parse failed");
            return blResult;
        }

        auto jsEvaluationDesc = jsValue["evaluation_desc"];
        if (jsEvaluationDesc.isNull() || !jsEvaluationDesc.isString() || jsEvaluationDesc.asString().empty())
        {
            LOG_ERROR_RLD("Evaluation desc parse failed");
            return blResult;
        }

        auto jsEvaluationValue = jsValue["evaluation_value"];
        if (jsEvaluationValue.isNull() || !jsEvaluationValue.isString() || jsEvaluationValue.asString().empty())
        {
            LOG_ERROR_RLD("Evaluation value parse failed");
            return blResult;
        }

        double dEvaluationValue = 0;
        if (!ValidType<double>(jsEvaluationValue.asString(), dEvaluationValue))
        {
            LOG_ERROR_RLD("Evaluation value is invalid and value is " << jsEvaluationValue.asString());
            return blResult;
        }

        EvaluationTemplate evt;
        evt.m_dEvaluationValue = dEvaluationValue;
        evt.m_strEvaluationTmpID = jsEvaluationID.asString();
        evt.m_strEvaluationDesc = jsEvaluationDesc.asString();

        ev.m_evlist.push_back(std::move(evt));
        
        return true;
    };

    if (!GetValueFromList(strEvaluationInfo, EvaluationParse))
    {
        LOG_ERROR_RLD("Evaluation parse failed and value is " << strEvaluationInfo);
        return blResult;
    }
    
    if (!CreateEvaluation(strSid, ev))
    {
        LOG_ERROR_RLD("Create evaluation failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Create evaluation info received and session id is " << strSid << " and user id is " << strUserID << " and store id is " << strStoreID
        << " and evaluation id is " << ev.m_strEvaluationID << " and evaluation info is " << strEvaluationInfo);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("store_evaluation_id", ev.m_strEvaluationID));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::DeleteEvaluationOfStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("store_evaluation_id");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("evaluation id of store not found.");
        return blResult;
    }
    const std::string strEvaluationIDOfStore = itFind->second;

    if (!DeleteEvaluation(strSid, strUserID, strEvaluationIDOfStore))
    {
        LOG_ERROR_RLD("Delete evaluation failed.");
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete evaluation info received and session id is " << strSid << " and user id is " << strUserID
        << " and evaluation id of store is " << strEvaluationIDOfStore);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::ModifyEvaluationOfStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("store_evaluation_id");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Evaluation id of store not found.");
        return blResult;
    }
    const std::string strEvaluationIDOfStore = itFind->second;
    
    std::string strUserIDCheck;
    itFind = pMsgInfoMap->find("userid_check");
    if (pMsgInfoMap->end() != itFind)
    {
        strUserIDCheck = itFind->second;
    }
    
    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;

    unsigned int uiCheckStatus = 0xFFFFFFFF;
    itFind = pMsgInfoMap->find("check_status");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidType<unsigned int>(itFind->second, uiCheckStatus))
        {
            LOG_ERROR_RLD("Check status  is invalid and value is " << itFind->second);
            return blResult;
        }
    }    

    Evaluation ev;
    ev.m_strUserID = strUserID;
    ev.m_strUserIDOfCheck = strUserIDCheck;
    ev.m_strStoreID = strStoreID;
    ev.m_strEvaluationID = strEvaluationIDOfStore;
    ev.m_uiCheckStatus = uiCheckStatus;

    std::string strEvaluationInfo;
    itFind = pMsgInfoMap->find("evaluation_info");
    if (pMsgInfoMap->end() != itFind)
    {
        strEvaluationInfo = itFind->second;
        if (strEvaluationInfo.empty())
        {
            LOG_ERROR_RLD("Evaluation info is empty.");
            return blResult;
        }

        auto EvaluationParse = [&](Json::Value jsValue) ->bool
        {
            if (!jsValue.isObject())
            {
                LOG_ERROR_RLD("Evaluation parse failed");
                return blResult;
            }

            auto jsEvaluationID = jsValue["evaluation_id"];
            if (jsEvaluationID.isNull() || !jsEvaluationID.isString() || jsEvaluationID.asString().empty())
            {
                LOG_ERROR_RLD("Evaluation id parse failed");
                return blResult;
            }

            auto jsEvaluationDesc = jsValue["evaluation_desc"];
            if (jsEvaluationDesc.isNull() || !jsEvaluationDesc.isString() || jsEvaluationDesc.asString().empty())
            {
                LOG_ERROR_RLD("Evaluation desc parse failed");
                return blResult;
            }

            auto jsEvaluationValue = jsValue["evaluation_value"];
            if (jsEvaluationValue.isNull() || !jsEvaluationValue.isString() || jsEvaluationValue.asString().empty())
            {
                LOG_ERROR_RLD("Evaluation value parse failed");
                return blResult;
            }

            double dEvaluationValue = 0;
            if (!ValidType<double>(jsEvaluationValue.asString(), dEvaluationValue))
            {
                LOG_ERROR_RLD("Evaluation value is invalid and value is " << jsEvaluationValue.asString());
                return blResult;
            }

            EvaluationTemplate evt;
            evt.m_dEvaluationValue = dEvaluationValue;
            evt.m_strEvaluationTmpID = jsEvaluationID.asString();
            evt.m_strEvaluationDesc = jsEvaluationDesc.asString();

            ev.m_evlist.push_back(std::move(evt));

            return true;
        };

        if (!GetValueFromList(strEvaluationInfo, EvaluationParse))
        {
            LOG_ERROR_RLD("Evaluation parse failed and value is " << strEvaluationInfo);
            return blResult;
        }
        
    }
    
    if (!ModifyEvaluation(strSid, ev))
    {
        LOG_ERROR_RLD("Modify evaluation failed.");
        return blResult;
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify evaluation info received and session id is " << strSid << " and user id is " << strUserID << " and store id is " << strStoreID
        << " and evalutaion id is " << strEvaluationIDOfStore << " and evaluation info is " << strEvaluationInfo);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryEvaluationOfStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsEvaList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsEvaList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["evaluation_info"] = jsEvaList;
            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;

    itFind = pMsgInfoMap->find("store_evaluation_id");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store evaluation id not found.");
        return blResult;
    }
    const std::string strEvaluationIDOfStore = itFind->second;
    
    Evaluation ev;
    ev.m_strStoreID = strStoreID;
    ev.m_strEvaluationID = strEvaluationIDOfStore;

    if (!QueryEvaluation(strSid, strUserID, ev))
    {
        LOG_ERROR_RLD("Query evaluation failed.");
        return blResult;
    }

    unsigned int i = 0;
    for (auto itBegin = ev.m_evlist.begin(), itEnd = ev.m_evlist.end(); itBegin != itEnd; ++itBegin, ++i)
    {
        Json::Value jsEva;
        jsEva["evaluation_id"] = itBegin->m_strEvaluationTmpID;
        jsEva["evaluation_template_desc"] = itBegin->m_strEvaluationDesc;
        jsEva["evaluation_template_value"] = boost::lexical_cast<std::string>(itBegin->m_dEvaluationValue);
        jsEva["evaluation"] = itBegin->m_strEvaluation;
        jsEva["evaluation_desc"] = itBegin->m_strEvaDescActive;
        jsEva["evaluation_value"] = boost::lexical_cast<std::string>(itBegin->m_dEvaValueActive);
        jsEvaList[i] = jsEva;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query evaluation info received and session id is " << strSid << " and user id is " << strUserID
        << " and store id is " << strStoreID << " and evaluation id of store is " << strEvaluationIDOfStore);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("check_status", boost::lexical_cast<std::string>(ev.m_uiCheckStatus)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("userid_check", ev.m_strUserIDOfCheck));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("userid", ev.m_strUserID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("evaluation_date", ev.m_strEvaluationDate));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("evaluation_total_value", boost::lexical_cast<std::string>(ev.m_dEvaluationTotalValue)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("store_evaluation_id", ev.m_strEvaluationID));
    
    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryAllEvaluationOfStoreHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsEvaList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsEvaList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["evaluation_info"] = jsEvaList;
            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;

    std::string strBeginDate;
    itFind = pMsgInfoMap->find("begindate");
    if (pMsgInfoMap->end() != itFind)
    {
        strBeginDate = itFind->second;
        if (!ValidDatetime(strBeginDate))
        {
            LOG_ERROR_RLD("Begin data is invalid and value is " << strBeginDate);
            return blResult;
        }
    }
    
    std::string strEndDate;
    itFind = pMsgInfoMap->find("enddate");
    if (pMsgInfoMap->end() != itFind)
    {
        strEndDate = itFind->second;
        if (!ValidDatetime(strEndDate))
        {
            LOG_ERROR_RLD("End data is invalid and value is " << strEndDate);
            return blResult;
        }
    }

    unsigned int uiBeginIndex = 0;
    itFind = pMsgInfoMap->find("beginindex");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiBeginIndex = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Begin index is invalid and error msg is " << e.what() << " and input index is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Begin index is invalid and input index is " << itFind->second);
            return blResult;
        }
    }
    
    std::list<Evaluation> evlist;
    if (!QueryAllEvaluationOfStore(strSid, strUserID, strStoreID, strBeginDate, strEndDate, uiBeginIndex, evlist))
    {
        LOG_ERROR_RLD("Query all evaluation of store failed.");
        return blResult;
    }

    unsigned int i = 0;
    for (auto itBegin = evlist.begin(), itEnd = evlist.end(); itBegin != itEnd; ++itBegin, ++i)
    {
        Json::Value jsEva;
        jsEva["store_evaluation_id"] = itBegin->m_strEvaluationID;
        jsEva["check_status"] = boost::lexical_cast<std::string>(itBegin->m_uiCheckStatus);
        jsEva["userid_check"] = itBegin->m_strUserIDOfCheck;
        jsEva["userid"] = itBegin->m_strUserID;
        jsEva["evaluation_date"] = itBegin->m_strEvaluationDate;
        jsEva["evaluation_total_value"] = boost::lexical_cast<std::string>(itBegin->m_dEvaluationTotalValue);
        
        jsEvaList[i] = jsEva;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query all evaluation info of store received and session id is " << strSid << " and user id is " << strUserID
        << " and store id is " << strStoreID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::UploadPassengerFlowHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Session id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("device_id");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    itFind = pMsgInfoMap->find("flow_data");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strPassengerFlowInfo = itFind->second;
    
    std::list<PassengerFlowInfo> pfinfolist;
    if (!strPassengerFlowInfo.empty())
    {
        if (!GetPassengerFlowInfo(strPassengerFlowInfo, pfinfolist))
        {
            LOG_ERROR_RLD("Get passenger flow info failed");
            return blResult;
        }
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Upload passenger flow info received and session id is " << strSid << " and device id is " << strDevID
        << " and passenger flow info is " << strPassengerFlowInfo);

    if (!UploadPassengerFlow(strSid, strDevID, pfinfolist))
    {
        LOG_ERROR_RLD("Upload passenger flow info handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;


}

bool PassengerFlowMsgHandler::AddStore(const std::string &strSid, const std::string &strUserID, StoreInfo &store)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
        std::string::size_type pos = strCurrentTime.find('T');
        strCurrentTime.replace(pos, 1, std::string(" "));

        PassengerFlowProtoHandler::AddStoreReq AddStoreReq;
        AddStoreReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddStoreReq_T;
        AddStoreReq.m_uiMsgSeq = 1;
        AddStoreReq.m_strSID = strSid;

        AddStoreReq.m_strUserID = strUserID;
        AddStoreReq.m_storeInfo.m_strAddress = store.m_strAddress;
        AddStoreReq.m_storeInfo.m_strCreateDate = strCurrentTime;
        AddStoreReq.m_storeInfo.m_strExtend = store.m_strExtend;
        AddStoreReq.m_storeInfo.m_strGoodsCategory = store.m_strGoodsCategory;
        AddStoreReq.m_storeInfo.m_strStoreName = store.m_strStoreName;
        AddStoreReq.m_storeInfo.m_uiState = 0;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddStoreReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add store req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::AddStoreRsp AddStoreRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddStoreRsp))
        {
            LOG_ERROR_RLD("Add store rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        store.m_strStoreID = AddStoreRsp.m_strStoreID;
        iRet = AddStoreRsp.m_iRetcode;

        LOG_INFO_RLD("Add store and store id is " << store.m_strStoreID << " and return code is " << AddStoreRsp.m_iRetcode <<
            " and return msg is " << AddStoreRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::DelStore(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::DeleteStoreReq DelStoreReq;
        DelStoreReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteStoreReq_T;
        DelStoreReq.m_uiMsgSeq = 1;
        DelStoreReq.m_strSID = strSid;

        DelStoreReq.m_strUserID = strUserID;
        DelStoreReq.m_strStoreID = strStoreID;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelStoreReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete store req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::DeleteStoreRsp DelStoreRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelStoreRsp))
        {
            LOG_ERROR_RLD("Delete store rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DelStoreRsp.m_iRetcode;

        LOG_INFO_RLD("Delete store and store id is " << strStoreID << " and user id is " << strUserID << " and session id is " << strSid <<
            " and return code is " << DelStoreRsp.m_iRetcode <<
            " and return msg is " << DelStoreRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ModifyStore(const std::string &strSid, const std::string &strUserID, const StoreInfo &store)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::ModifyStoreReq ModStoreReq;
        ModStoreReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyStoreReq_T;
        ModStoreReq.m_uiMsgSeq = 1;
        ModStoreReq.m_strSID = strSid;

        ModStoreReq.m_strUserID = strUserID;
        ModStoreReq.m_storeInfo.m_strAddress = store.m_strAddress;
        //ModStoreReq.m_storeInfo.m_strCreateDate = strCurrentTime;
        ModStoreReq.m_storeInfo.m_strExtend = store.m_strExtend;
        ModStoreReq.m_storeInfo.m_strGoodsCategory = store.m_strGoodsCategory;
        ModStoreReq.m_storeInfo.m_strStoreName = store.m_strStoreName;
        ModStoreReq.m_storeInfo.m_uiState = 0;
        ModStoreReq.m_storeInfo.m_strStoreID = store.m_strStoreID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModStoreReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify store req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::ModifyStoreRsp ModStoreRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModStoreRsp))
        {
            LOG_ERROR_RLD("Modify store rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModStoreRsp.m_iRetcode;

        LOG_INFO_RLD("Modify store and store id is " << store.m_strStoreID << " and return code is " << ModStoreRsp.m_iRetcode <<
            " and return msg is " << ModStoreRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryStore(const std::string &strSid, const std::string &strUserID, StoreInfo &store, std::list<EntranceInfo> &entranceInfolist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryStoreInfoReq QueryStoreReq;
        QueryStoreReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryStoreInfoReq_T;
        QueryStoreReq.m_uiMsgSeq = 1;
        QueryStoreReq.m_strSID = strSid;

        QueryStoreReq.m_strUserID = strUserID;
        QueryStoreReq.m_strStoreID = store.m_strStoreID;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryStoreReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query store req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryStoreInfoRsp QueryStoreRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryStoreRsp))
        {
            LOG_ERROR_RLD("Query store rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        store.m_strAddress = QueryStoreRsp.m_storeInfo.m_strAddress;
        store.m_strCreateDate = QueryStoreRsp.m_storeInfo.m_strCreateDate;
        store.m_strExtend = QueryStoreRsp.m_storeInfo.m_strExtend;
        store.m_strGoodsCategory = QueryStoreRsp.m_storeInfo.m_strGoodsCategory;
        store.m_strStoreID = QueryStoreRsp.m_storeInfo.m_strStoreID;
        store.m_strStoreName = QueryStoreRsp.m_storeInfo.m_strStoreName;

        auto itBegin = QueryStoreRsp.m_storeInfo.m_entranceList.begin();
        auto itEnd = QueryStoreRsp.m_storeInfo.m_entranceList.end();
        while (itBegin != itEnd)
        {
            {
                LOG_INFO_RLD("Query store info received and entrance name is " << itBegin->m_strEntranceName
                    << " and entrance id is " << itBegin->m_strEntranceID);
                auto it1 = itBegin->m_strDeviceIDList.begin();
                auto it2 = itBegin->m_strDeviceIDList.end();
                while (it1 != it2)
                {
                    LOG_INFO_RLD("Query store info received and entrance device id is " << *it1);
                    ++it1;
                }
            }

            EntranceInfo einfo;
            einfo.m_strID = itBegin->m_strEntranceID;
            einfo.m_strName = itBegin->m_strEntranceName;
            einfo.m_DeviceIDList.swap(itBegin->m_strDeviceIDList);
            entranceInfolist.push_back(std::move(einfo));

            ++itBegin;
        }

        iRet = QueryStoreRsp.m_iRetcode;

        LOG_INFO_RLD("Query store and store id is " << store.m_strStoreID << " and return code is " << QueryStoreRsp.m_iRetcode <<
            " and return msg is " << QueryStoreRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}


bool PassengerFlowMsgHandler::QueryAllStore(const std::string &strSid, const std::string &strUserID, const unsigned int uiBeginIndex, std::list<StoreInfo> &storelist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryAllStoreReq QueryAllStoreReq;
        QueryAllStoreReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllStoreReq_T;
        QueryAllStoreReq.m_uiMsgSeq = 1;
        QueryAllStoreReq.m_strSID = strSid;

        QueryAllStoreReq.m_strUserID = strUserID;
        QueryAllStoreReq.m_uiBeginIndex = uiBeginIndex;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryAllStoreReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all store req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryAllStoreRsp QueryAllStoreRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryAllStoreRsp))
        {
            LOG_ERROR_RLD("Query all store rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        auto itBegin = QueryAllStoreRsp.m_storeList.begin();
        auto itEnd = QueryAllStoreRsp.m_storeList.end();
        while (itBegin != itEnd)
        {
            StoreInfo store;
            store.m_strStoreID = itBegin->m_strStoreID;
            store.m_strStoreName = itBegin->m_strStoreName;

            LOG_INFO_RLD("Query all store info received and store id is " << itBegin->m_strStoreID <<
                " and store name is " << itBegin->m_strStoreName);

            storelist.push_back(std::move(store));
            ++itBegin;
        }

        iRet = QueryAllStoreRsp.m_iRetcode;

        LOG_INFO_RLD("Query all store and user id is " << strUserID << " and begin index is " << uiBeginIndex << 
            " and return code is " << QueryAllStoreRsp.m_iRetcode <<
            " and return msg is " << QueryAllStoreRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::AddEntrance(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, EntranceInfo &einfo)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::AddEntranceReq AddEntranceReq;
        AddEntranceReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddEntranceReq_T;
        AddEntranceReq.m_uiMsgSeq = 1;
        AddEntranceReq.m_strSID = strSid;

        AddEntranceReq.m_strUserID = strUserID;
        AddEntranceReq.m_strStoreID = strStoreID;
        //AddEntranceReq.m_entranceInfo.m_strEntranceID = einfo.m_strID;
        AddEntranceReq.m_entranceInfo.m_strEntranceName = einfo.m_strName;
        AddEntranceReq.m_entranceInfo.m_strDeviceIDList.swap(einfo.m_DeviceIDList);

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddEntranceReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add entrance req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::AddEntranceRsp AddEntranceRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddEntranceRsp))
        {
            LOG_ERROR_RLD("Add entrance rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        einfo.m_strID = AddEntranceRsp.m_strEntranceID;
        iRet = AddEntranceRsp.m_iRetcode;

        LOG_INFO_RLD("Add entrance and entrance id is " << einfo.m_strID << " and entrance name is " << einfo.m_strName << 
            " and return code is " << AddEntranceRsp.m_iRetcode <<
            " and return msg is " << AddEntranceRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::DelEntrance(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, const std::string &strEntranceID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::DeleteEntranceReq DelEntranceReq;
        DelEntranceReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteEntranceReq_T;
        DelEntranceReq.m_uiMsgSeq = 1;
        DelEntranceReq.m_strSID = strSid;

        DelEntranceReq.m_strUserID = strUserID;
        DelEntranceReq.m_strStoreID = strStoreID;
        DelEntranceReq.m_strEntranceID = strEntranceID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelEntranceReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete entrance req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::DeleteEntranceRsp DelEntranceRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelEntranceRsp))
        {
            LOG_ERROR_RLD("Delete entrance rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DelEntranceRsp.m_iRetcode;

        LOG_INFO_RLD("Delete entrance and entrance id is " << strEntranceID << " and store id is " << strStoreID <<
            " and user id is " << strUserID <<
            " and return code is " << DelEntranceRsp.m_iRetcode <<
            " and return msg is " << DelEntranceRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ModifyEntrance(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, 
    const std::string &strEntranceID, const std::string &strEntranceName, EntranceInfo &einfoadd, EntranceInfo &einfodel)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::ModifyEntranceReq ModEntranceReq;
        ModEntranceReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyEntranceReq_T;
        ModEntranceReq.m_uiMsgSeq = 1;
        ModEntranceReq.m_strSID = strSid;

        ModEntranceReq.m_strUserID = strUserID;
        ModEntranceReq.m_strStoreID = strStoreID;
        ModEntranceReq.m_entranceInfo.m_strEntranceID = strEntranceID;
        ModEntranceReq.m_entranceInfo.m_strEntranceName = strEntranceName;
        ModEntranceReq.m_strAddedDeviceIDList.swap(einfoadd.m_DeviceIDList);
        ModEntranceReq.m_strDeletedDeviceIDList.swap(einfodel.m_DeviceIDList);

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModEntranceReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify entrance req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::ModifyEntranceRsp ModEntranceRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModEntranceRsp))
        {
            LOG_ERROR_RLD("Modify entrance rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModEntranceRsp.m_iRetcode;

        LOG_INFO_RLD("Modify entrance and entrance id is " << strEntranceID << " and entrance name is " << strEntranceName <<
            " and return code is " << ModEntranceRsp.m_iRetcode <<
            " and return msg is " << ModEntranceRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::UploadPassengerFlow(const std::string &strSid, const std::string &strDevID, const std::list<PassengerFlowInfo> &pfinfolist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::ReportCustomerFlowDataReq UploadPassengerFlowReq;
        UploadPassengerFlowReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ReportCustomerFlowDataReq_T;
        UploadPassengerFlowReq.m_uiMsgSeq = 1;
        UploadPassengerFlowReq.m_strSID = strSid;

        UploadPassengerFlowReq.m_strDeviceID = strDevID;

        auto itBegin = pfinfolist.begin();
        auto itEnd = pfinfolist.end();
        while (itBegin != itEnd)
        {
            PassengerFlowProtoHandler::RawCustomerFlow flow;
            flow.m_strDataTime = itBegin->m_strDateTime;
            flow.m_uiEnterNumber = itBegin->m_uiEnterNum;
            flow.m_uiLeaveNumber = itBegin->m_uiLeaveNum;
            flow.m_uiStayNumber = itBegin->m_uiStayNum;

            UploadPassengerFlowReq.m_customerFlowList.push_back(std::move(flow));
            ++itBegin;
        }

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(UploadPassengerFlowReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Upload passenger flow req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::ReportCustomerFlowDataRsp UploadPassengerFlowRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, UploadPassengerFlowRsp))
        {
            LOG_ERROR_RLD("Upload passenger flow rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = UploadPassengerFlowRsp.m_iRetcode;

        LOG_INFO_RLD("Upload passenger flow " <<
            " and return code is " << UploadPassengerFlowRsp.m_iRetcode <<
            " and return msg is " << UploadPassengerFlowRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::BindEntranceDevice(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, 
    const std::string &strEntranceID, const std::string &strDevID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::AddEntranceDeviceReq AddEntranceDevReq;
        AddEntranceDevReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddEntranceDeviceReq_T;
        AddEntranceDevReq.m_uiMsgSeq = 1;
        AddEntranceDevReq.m_strSID = strSid;

        AddEntranceDevReq.m_strUserID = strUserID;
        AddEntranceDevReq.m_strStoreID = strStoreID;
        AddEntranceDevReq.m_strDeviceID = strDevID;
        AddEntranceDevReq.m_strEntranceID = strEntranceID;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddEntranceDevReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Bind entrance device req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::AddEntranceDeviceRsp AddEntranceDevRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddEntranceDevRsp))
        {
            LOG_ERROR_RLD("Bind entrance device rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = AddEntranceDevRsp.m_iRetcode;

        LOG_INFO_RLD("Bind entrance device and store id is " << strStoreID << " and user id is " << strUserID << " and session id is " << strSid <<
            " and return code is " << AddEntranceDevRsp.m_iRetcode <<
            " and return msg is " << AddEntranceDevRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::UnbindEntranceDevice(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, 
    const std::string &strEntranceID, const std::string &strDevID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::DeleteEntranceDeviceReq DelEntranceDevReq;
        DelEntranceDevReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteEntranceDeviceReq_T;
        DelEntranceDevReq.m_uiMsgSeq = 1;
        DelEntranceDevReq.m_strSID = strSid;

        DelEntranceDevReq.m_strUserID = strUserID;
        DelEntranceDevReq.m_strStoreID = strStoreID;
        DelEntranceDevReq.m_strDeviceID = strDevID;
        DelEntranceDevReq.m_strEntranceID = strEntranceID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelEntranceDevReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Unbind entrance device req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::DeleteEntranceDeviceRsp DelEntranceDevRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelEntranceDevRsp))
        {
            LOG_ERROR_RLD("Unbind entrance device rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DelEntranceDevRsp.m_iRetcode;

        LOG_INFO_RLD("Unbind entrance device and store id is " << strStoreID << " and user id is " << strUserID << " and session id is " << strSid <<
            " and return code is " << DelEntranceDevRsp.m_iRetcode <<
            " and return msg is " << DelEntranceDevRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ImportPosData(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, 
    const double dDealAmount, const unsigned int uiOrderAmount, const unsigned int uiGoodsAmount, const std::string &strDealDate)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::ImportPOSDataReq ImportPosDataReq;
        ImportPosDataReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ImportPOSDataReq_T;
        ImportPosDataReq.m_uiMsgSeq = 1;
        ImportPosDataReq.m_strSID = strSid;

        ImportPosDataReq.m_strUserID = strUserID;
        ImportPosDataReq.m_strStoreID = strStoreID;
        ImportPosDataReq.m_strDealDate = strDealDate;
        ImportPosDataReq.m_dDealAmount = dDealAmount;
        ImportPosDataReq.m_uiGoodsAmount = uiGoodsAmount;
        ImportPosDataReq.m_uiOrderAmount = uiOrderAmount;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ImportPosDataReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Import pos data req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::ImportPOSDataRsp ImportPosDataRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ImportPosDataRsp))
        {
            LOG_ERROR_RLD("Import pos data rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ImportPosDataRsp.m_iRetcode;

        LOG_INFO_RLD("Import pos data and store id is " << strStoreID << " and user id is " << strUserID << " and session id is " << strSid <<
            " and return code is " << ImportPosDataRsp.m_iRetcode <<
            " and return msg is " << ImportPosDataRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryPassengerFlowReport(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, 
    const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiTimePrecision, std::string &strChartData)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryCustomerFlowStatisticReq QueryCustomerFlowReq;
        QueryCustomerFlowReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryCustomerFlowStatisticReq_T;
        QueryCustomerFlowReq.m_uiMsgSeq = 1;
        QueryCustomerFlowReq.m_strSID = strSid;

        QueryCustomerFlowReq.m_strUserID = strUserID;
        QueryCustomerFlowReq.m_strStoreID = strStoreID;
        QueryCustomerFlowReq.m_strBeginDate = strBeginDate;
        QueryCustomerFlowReq.m_strEndDate = strEndDate;
        QueryCustomerFlowReq.m_uiTimePrecision = uiTimePrecision;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryCustomerFlowReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query passenger flow report req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryCustomerFlowStatisticRsp QueryCustomerFlowRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryCustomerFlowRsp))
        {
            LOG_ERROR_RLD("Query passenger flow report rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = QueryCustomerFlowRsp.m_iRetcode;

        strChartData = QueryCustomerFlowRsp.m_strChartData;

        LOG_INFO_RLD("Query passenger flow report and store id is " << strStoreID << " and user id is " << strUserID << " and session id is " << strSid <<
            " and chart data is " << strChartData <<
            " and return code is " << QueryCustomerFlowRsp.m_iRetcode <<
            " and return msg is " << QueryCustomerFlowRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ReportEvent(const std::string &strSid, EventInfo &eventinfo)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::AddEventReq AddEventReq;
        AddEventReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddEventReq_T;
        AddEventReq.m_uiMsgSeq = 1;
        AddEventReq.m_strSID = strSid;

        AddEventReq.m_eventInfo.m_strUserID = eventinfo.m_strUserID;
        AddEventReq.m_eventInfo.m_strDeviceID = eventinfo.m_strDevID;
        AddEventReq.m_eventInfo.m_strExpireDate = eventinfo.m_strExpireDate;
        AddEventReq.m_eventInfo.m_strHandlerList.swap(eventinfo.m_strEventHandlerList);
        AddEventReq.m_eventInfo.m_strRemark = eventinfo.m_strRemark;
        AddEventReq.m_eventInfo.m_strSource = eventinfo.m_strSource;
        AddEventReq.m_eventInfo.m_strSubmitDate = eventinfo.m_strSubmitDate;
        AddEventReq.m_eventInfo.m_uiTypeList.swap(eventinfo.m_uiEventTypeList);

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddEventReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Report event req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::AddEventRsp AddEventRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddEventRsp))
        {
            LOG_ERROR_RLD("Report event rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = AddEventRsp.m_iRetcode;

        eventinfo.m_strEventID = AddEventRsp.m_strEventID;

        LOG_INFO_RLD("Report event and event id is " << eventinfo.m_strEventID << " and session id is " << strSid <<
            " and return code is " << AddEventRsp.m_iRetcode <<
            " and return msg is " << AddEventRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::DeleteEvent(const std::string &strSid, const std::string &strUserID, const std::string &strEventID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::DeleteEventReq DelEventReq;
        DelEventReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteEventReq_T;
        DelEventReq.m_uiMsgSeq = 1;
        DelEventReq.m_strSID = strSid;

        DelEventReq.m_strUserID = strUserID;
        DelEventReq.m_strEventID = strEventID;
        

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelEventReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete event req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::DeleteEventRsp DelEventRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelEventRsp))
        {
            LOG_ERROR_RLD("Delete event rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DelEventRsp.m_iRetcode;

        LOG_INFO_RLD("Delete event and event id is " << strEventID << " and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << DelEventRsp.m_iRetcode <<
            " and return msg is " << DelEventRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ModifyEvent(const std::string &strSid, EventInfo &eventinfo)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::ModifyEventReq ModEventReq;
        ModEventReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyEventReq_T;
        ModEventReq.m_uiMsgSeq = 1;
        ModEventReq.m_strSID = strSid;

        ModEventReq.m_strUserID = eventinfo.m_strUserID;
        ModEventReq.m_eventInfo.m_strUserID = eventinfo.m_strUserID;
        ModEventReq.m_eventInfo.m_strDeviceID = eventinfo.m_strDevID;
        ModEventReq.m_eventInfo.m_strEventID = eventinfo.m_strEventID;
        ModEventReq.m_eventInfo.m_strExpireDate = eventinfo.m_strExpireDate;
        ModEventReq.m_eventInfo.m_strHandlerList.swap(eventinfo.m_strEventHandlerList);
        ModEventReq.m_eventInfo.m_strRemark = eventinfo.m_strRemark;
        ModEventReq.m_eventInfo.m_strSource = eventinfo.m_strSource;
        ModEventReq.m_eventInfo.m_strSubmitDate = eventinfo.m_strSubmitDate;
        ModEventReq.m_eventInfo.m_uiTypeList.swap(eventinfo.m_uiEventTypeList);
        ModEventReq.m_eventInfo.m_strProcessState = eventinfo.m_strProcessState;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModEventReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify event req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::ModifyEventRsp ModEventRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModEventRsp))
        {
            LOG_ERROR_RLD("Modify event rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModEventRsp.m_iRetcode;

        LOG_INFO_RLD("Modify event and event id is " << eventinfo.m_strEventID << " and session id is " << strSid <<
            " and return code is " << ModEventRsp.m_iRetcode <<
            " and return msg is " << ModEventRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryEvent(const std::string &strSid, EventInfo &eventinfo)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryEventInfoReq QueryEventReq;
        QueryEventReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryEventInfoReq_T;
        QueryEventReq.m_uiMsgSeq = 1;
        QueryEventReq.m_strSID = strSid;

        QueryEventReq.m_strUserID = eventinfo.m_strUserID;
        QueryEventReq.m_strEventID = eventinfo.m_strEventID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryEventReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query event req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryEventInfoRsp QueryEventRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryEventRsp))
        {
            LOG_ERROR_RLD("Query event rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        eventinfo.m_strCreateDate = QueryEventRsp.m_eventInfo.m_strCreateDate;
        eventinfo.m_strDevID = QueryEventRsp.m_eventInfo.m_strDeviceID;
        eventinfo.m_strEventHandlerList.swap(QueryEventRsp.m_eventInfo.m_strHandlerList);
        eventinfo.m_strEventID = QueryEventRsp.m_eventInfo.m_strEventID;
        eventinfo.m_strExpireDate = QueryEventRsp.m_eventInfo.m_strExpireDate;
        eventinfo.m_strExtend = QueryEventRsp.m_eventInfo.m_strExtend;
        eventinfo.m_strProcessState = QueryEventRsp.m_eventInfo.m_strProcessState;
        eventinfo.m_strRemark = QueryEventRsp.m_eventInfo.m_strRemark;
        eventinfo.m_strSource = QueryEventRsp.m_eventInfo.m_strSource;
        eventinfo.m_strSubmitDate = QueryEventRsp.m_eventInfo.m_strSubmitDate;
        eventinfo.m_strUserID = QueryEventRsp.m_eventInfo.m_strUserID;
        eventinfo.m_uiEventTypeList.swap(QueryEventRsp.m_eventInfo.m_uiTypeList);
        
        iRet = QueryEventRsp.m_iRetcode;

        LOG_INFO_RLD("Query event and event id is " << eventinfo.m_strEventID << " and session id is " << strSid <<
            " and return code is " << QueryEventRsp.m_iRetcode <<
            " and return msg is " << QueryEventRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryAllEvent(const std::string &strSid, const std::string &strUserID, const unsigned int uiBeginIndex, std::list<EventInfo> &eventinfoList)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryAllEventReq QueryAllEventReq;
        QueryAllEventReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllEventReq_T;
        QueryAllEventReq.m_uiMsgSeq = 1;
        QueryAllEventReq.m_strSID = strSid;

        QueryAllEventReq.m_strUserID = strUserID;
        QueryAllEventReq.m_uiBeginIndex = uiBeginIndex;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryAllEventReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all event req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryAllEventRsp QueryAllEventRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryAllEventRsp))
        {
            LOG_ERROR_RLD("Query all event rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        auto itBegin = QueryAllEventRsp.m_eventList.begin();
        auto itEnd = QueryAllEventRsp.m_eventList.end();
        while (itBegin != itEnd)
        {
            EventInfo eventinfo;
            eventinfo.m_strCreateDate = itBegin->m_strCreateDate;
            eventinfo.m_strDevID = itBegin->m_strDeviceID;
            eventinfo.m_strEventHandlerList.swap(itBegin->m_strHandlerList);
            eventinfo.m_strEventID = itBegin->m_strEventID;
            eventinfo.m_strExpireDate = itBegin->m_strExpireDate;
            eventinfo.m_strExtend = itBegin->m_strExtend;
            eventinfo.m_strProcessState = itBegin->m_strProcessState;
            eventinfo.m_strRemark = itBegin->m_strRemark;
            eventinfo.m_strSource = itBegin->m_strSource;
            eventinfo.m_strSubmitDate = itBegin->m_strSubmitDate;
            eventinfo.m_strUserID = itBegin->m_strUserID;
            eventinfo.m_uiEventTypeList.swap(itBegin->m_uiTypeList);
            
            eventinfoList.push_back(std::move(eventinfo));

            ++itBegin;
        }

        iRet = QueryAllEventRsp.m_iRetcode;

        LOG_INFO_RLD("Query all event and user id is " << strUserID << " and session id is " << strSid << " and begin index is " << uiBeginIndex <<
            " and return code is " << QueryAllEventRsp.m_iRetcode <<
            " and return msg is " << QueryAllEventRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::CreateGuardStorePlan(const std::string &strSid, Plan &plan)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::AddSmartGuardStoreReq AddPlanReq;
        AddPlanReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddSmartGuardStoreReq_T;
        AddPlanReq.m_uiMsgSeq = 1;
        AddPlanReq.m_strSID = strSid;

        AddPlanReq.m_strUserID = plan.m_strUserID;
        AddPlanReq.m_smartGuardStore.m_strBeginTime = plan.m_strBeginTime;
        AddPlanReq.m_smartGuardStore.m_strBeginTime2 = plan.m_strBeginTime2;
        AddPlanReq.m_smartGuardStore.m_strEnable = plan.m_strEnable;
        AddPlanReq.m_smartGuardStore.m_strEndTime = plan.m_strEndTime;
        AddPlanReq.m_smartGuardStore.m_strEndTime2 = plan.m_strEndTime2;
        AddPlanReq.m_smartGuardStore.m_strEntranceIDList.swap(plan.m_strEntranceIDList);
        AddPlanReq.m_smartGuardStore.m_strPlanName = plan.m_strPlanName;
        AddPlanReq.m_smartGuardStore.m_strStoreID = plan.m_strStoreID;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddPlanReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Create guard plan req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::AddSmartGuardStoreRsp AddPlanRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddPlanRsp))
        {
            LOG_ERROR_RLD("Create guard plan rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = AddPlanRsp.m_iRetcode;

        plan.m_strPlanID = AddPlanRsp.m_strPlanID;

        LOG_INFO_RLD("Create guard plan and plan id is " << plan.m_strPlanID << " and session id is " << strSid <<
            " and return code is " << AddPlanRsp.m_iRetcode <<
            " and return msg is " << AddPlanRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::DeleteGuardStorePlan(const std::string &strSid, const std::string &strUserID, const std::string &strPlanID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::DeleteSmartGuardStoreReq DelPlanReq;
        DelPlanReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteSmartGuardStoreReq_T;
        DelPlanReq.m_uiMsgSeq = 1;
        DelPlanReq.m_strSID = strSid;

        DelPlanReq.m_strUserID = strUserID;
        DelPlanReq.m_strPlanID = strPlanID;


        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelPlanReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete plan req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::DeleteSmartGuardStoreRsp DelPlanRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelPlanRsp))
        {
            LOG_ERROR_RLD("Delete plan rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DelPlanRsp.m_iRetcode;

        LOG_INFO_RLD("Delete plan and plan id is " << strPlanID << " and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << DelPlanRsp.m_iRetcode <<
            " and return msg is " << DelPlanRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ModifyGuardStorePlan(const std::string &strSid, Plan &plan)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::ModifySmartGuardStoreReq ModPlanReq;
        ModPlanReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifySmartGuardStoreReq_T;
        ModPlanReq.m_uiMsgSeq = 1;
        ModPlanReq.m_strSID = strSid;

        ModPlanReq.m_strUserID = plan.m_strUserID;
        ModPlanReq.m_smartGuardStore.m_strBeginTime = plan.m_strBeginTime;
        ModPlanReq.m_smartGuardStore.m_strBeginTime2 = plan.m_strBeginTime2;
        ModPlanReq.m_smartGuardStore.m_strEnable = plan.m_strEnable;
        ModPlanReq.m_smartGuardStore.m_strEndTime = plan.m_strEndTime;
        ModPlanReq.m_smartGuardStore.m_strEndTime2 = plan.m_strEndTime2;
        ModPlanReq.m_smartGuardStore.m_strEntranceIDList.swap(plan.m_strEntranceIDList);
        ModPlanReq.m_smartGuardStore.m_strPlanName = plan.m_strPlanName;
        ModPlanReq.m_smartGuardStore.m_strPlanID = plan.m_strPlanID;
        ModPlanReq.m_smartGuardStore.m_strStoreID = plan.m_strStoreID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModPlanReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify guard plan req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::ModifySmartGuardStoreRsp ModPlanRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModPlanRsp))
        {
            LOG_ERROR_RLD("Modify guard plan rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModPlanRsp.m_iRetcode;

        LOG_INFO_RLD("Modify guard plan and plan id is " << plan.m_strPlanID << " and session id is " << strSid <<
            " and return code is " << ModPlanRsp.m_iRetcode <<
            " and return msg is " << ModPlanRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryGuardStorePlan(const std::string &strSid, Plan &plan)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QuerySmartGuardStoreInfoReq QueryPlanReq;
        QueryPlanReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QuerySmartGuardStoreInfoReq_T;
        QueryPlanReq.m_uiMsgSeq = 1;
        QueryPlanReq.m_strSID = strSid;

        QueryPlanReq.m_strUserID = plan.m_strUserID;
        QueryPlanReq.m_strPlanID = plan.m_strPlanID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryPlanReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query plan req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QuerySmartGuardStoreInfoRsp QueryPlanRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryPlanRsp))
        {
            LOG_ERROR_RLD("Query plan rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        plan.m_strBeginTime = QueryPlanRsp.m_smartGuardStore.m_strBeginTime;
        plan.m_strBeginTime2 = QueryPlanRsp.m_smartGuardStore.m_strBeginTime2;
        plan.m_strEnable = QueryPlanRsp.m_smartGuardStore.m_strEnable;
        plan.m_strEndTime = QueryPlanRsp.m_smartGuardStore.m_strEndTime;
        plan.m_strEndTime2 = QueryPlanRsp.m_smartGuardStore.m_strEndTime2;
        plan.m_strEntranceIDList.swap(QueryPlanRsp.m_smartGuardStore.m_strEntranceIDList);
        plan.m_strPlanID = QueryPlanRsp.m_smartGuardStore.m_strPlanID;
        plan.m_strPlanName = QueryPlanRsp.m_smartGuardStore.m_strPlanName;
        plan.m_strStoreID = QueryPlanRsp.m_smartGuardStore.m_strStoreID;
        plan.m_strStoreName = QueryPlanRsp.m_smartGuardStore.m_strStoreName;
        
        iRet = QueryPlanRsp.m_iRetcode;

        LOG_INFO_RLD("Query plan and plan id is " << plan.m_strPlanID << " and session id is " << strSid <<
            " and return code is " << QueryPlanRsp.m_iRetcode <<
            " and return msg is " << QueryPlanRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryAllGuardStorePlan(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, std::list<Plan> &planlist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryAllSmartGuardStoreReq QueryAllPlanReq;
        QueryAllPlanReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllSmartGuardStoreReq_T;
        QueryAllPlanReq.m_uiMsgSeq = 1;
        QueryAllPlanReq.m_strSID = strSid;

        QueryAllPlanReq.m_strUserID = strUserID;
        QueryAllPlanReq.m_strDeviceID = strDevID;
        QueryAllPlanReq.m_uiBeginIndex = 0;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryAllPlanReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all plan req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryAllSmartGuardStoreRsp QueryAllPlanRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryAllPlanRsp))
        {
            LOG_ERROR_RLD("Query all plan rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        auto itBegin = QueryAllPlanRsp.m_planList.begin();
        auto itEnd = QueryAllPlanRsp.m_planList.end();
        while (itBegin != itEnd)
        {
            Plan planinfo;
            planinfo.m_strBeginTime = itBegin->m_strBeginTime;
            planinfo.m_strBeginTime2 = itBegin->m_strBeginTime2;
            planinfo.m_strEnable = itBegin->m_strEnable;
            planinfo.m_strEndTime = itBegin->m_strEndTime;
            planinfo.m_strEndTime2 = itBegin->m_strEndTime2;
            planinfo.m_strEntranceIDList.swap(itBegin->m_strEntranceIDList);
            planinfo.m_strPlanID = itBegin->m_strPlanID;
            planinfo.m_strPlanName = itBegin->m_strPlanName;
            planinfo.m_strStoreID = itBegin->m_strStoreID;
            planinfo.m_strStoreName = itBegin->m_strStoreName;
            
            planlist.push_back(std::move(planinfo));

            ++itBegin;
        }

        iRet = QueryAllPlanRsp.m_iRetcode;

        LOG_INFO_RLD("Query all plan and user id is " << strUserID << " and session id is " << strSid << " and device id is " << strDevID <<
            " and return code is " << QueryAllPlanRsp.m_iRetcode <<
            " and return msg is " << QueryAllPlanRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;

}

bool PassengerFlowMsgHandler::CreateRegularPatrol(const std::string &strSid, Patrol &pat)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::AddRegularPatrolReq AddPatrolReq;
        AddPatrolReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddRegularPatrolReq_T;
        AddPatrolReq.m_uiMsgSeq = 1;
        AddPatrolReq.m_strSID = strSid;

        AddPatrolReq.m_strUserID = pat.m_strUserID;
        AddPatrolReq.m_regularPatrol.m_strEnable = pat.m_strEnable;
        AddPatrolReq.m_regularPatrol.m_strPatrolTimeList.swap(pat.m_strPatrolTimeList);
        AddPatrolReq.m_regularPatrol.m_strPlanName = pat.m_strPatrolName;
        AddPatrolReq.m_regularPatrol.m_strStoreIDList.swap(pat.m_strStoreIDList);
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddPatrolReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Create regular patrol req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::AddRegularPatrolRsp AddPatrolRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddPatrolRsp))
        {
            LOG_ERROR_RLD("Create regular patrol rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = AddPatrolRsp.m_iRetcode;

        pat.m_strPatrolID = AddPatrolRsp.m_strPlanID;

        LOG_INFO_RLD("Create regular patrol and patrol id is " << pat.m_strPatrolID << " and session id is " << strSid <<
            " and return code is " << AddPatrolRsp.m_iRetcode <<
            " and return msg is " << AddPatrolRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::DeleteRegularPatrol(const std::string &strSid, const std::string &strUserID, const std::string &strPlanID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::DeleteRegularPatrolReq DelPatrolReq;
        DelPatrolReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteRegularPatrolReq_T;
        DelPatrolReq.m_uiMsgSeq = 1;
        DelPatrolReq.m_strSID = strSid;

        DelPatrolReq.m_strUserID = strUserID;
        DelPatrolReq.m_strPlanID = strPlanID;


        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelPatrolReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete regular patrol req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::DeleteRegularPatrolRsp DelPatrolRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelPatrolRsp))
        {
            LOG_ERROR_RLD("Delete regular patrol rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DelPatrolRsp.m_iRetcode;

        LOG_INFO_RLD("Delete regular patrol and plan id is " << strPlanID << " and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << DelPatrolRsp.m_iRetcode <<
            " and return msg is " << DelPatrolRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ModifyRegularPatrol(const std::string &strSid, Patrol &pat)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::ModifyRegularPatrolReq ModPatrolReq;
        ModPatrolReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyRegularPatrolReq_T;
        ModPatrolReq.m_uiMsgSeq = 1;
        ModPatrolReq.m_strSID = strSid;

        ModPatrolReq.m_strUserID = pat.m_strUserID;
        ModPatrolReq.m_regularPatrol.m_strPlanID = pat.m_strPatrolID;
        ModPatrolReq.m_regularPatrol.m_strEnable = pat.m_strEnable;
        ModPatrolReq.m_regularPatrol.m_strPatrolTimeList.swap(pat.m_strPatrolTimeList);
        ModPatrolReq.m_regularPatrol.m_strPlanName = pat.m_strPatrolName;
        ModPatrolReq.m_regularPatrol.m_strStoreIDList.swap(pat.m_strStoreIDList);

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModPatrolReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify regular patrol req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::ModifyRegularPatrolRsp ModPatrolRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModPatrolRsp))
        {
            LOG_ERROR_RLD("Modify regular patrol rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModPatrolRsp.m_iRetcode;

        LOG_INFO_RLD("Modify regular patrol and patrol id is " << pat.m_strPatrolID << " and session id is " << strSid <<
            " and return code is " << ModPatrolRsp.m_iRetcode <<
            " and return msg is " << ModPatrolRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryRegularPatrolPlan(const std::string &strSid, Patrol &pat)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryRegularPatrolInfoReq QueryPantrolReq;
        QueryPantrolReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryRegularPatrolInfoReq_T;
        QueryPantrolReq.m_uiMsgSeq = 1;
        QueryPantrolReq.m_strSID = strSid;

        QueryPantrolReq.m_strUserID = pat.m_strUserID;
        QueryPantrolReq.m_strPlanID = pat.m_strPatrolID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryPantrolReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query regular patrol req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryRegularPatrolInfoRsp QueryPatrolRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryPatrolRsp))
        {
            LOG_ERROR_RLD("Query regular patrol rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        pat.m_strEnable = QueryPatrolRsp.m_regularPatrol.m_strEnable;
        pat.m_strPatrolID = QueryPatrolRsp.m_regularPatrol.m_strPlanID;
        pat.m_strPatrolInfo = QueryPatrolRsp.m_regularPatrol.m_strStoreInfo;
        pat.m_strPatrolName = QueryPatrolRsp.m_regularPatrol.m_strPlanName;
        pat.m_strPatrolTimeList.swap(QueryPatrolRsp.m_regularPatrol.m_strPatrolTimeList);
        pat.m_strStoreIDList.swap(QueryPatrolRsp.m_regularPatrol.m_strStoreIDList);
        
        iRet = QueryPatrolRsp.m_iRetcode;

        LOG_INFO_RLD("Query regular patrol and plan id is " << pat.m_strPatrolID << " and session id is " << strSid <<
            " and return code is " << QueryPatrolRsp.m_iRetcode <<
            " and return msg is " << QueryPatrolRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryAllRegularPatrolPlan(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, std::list<Patrol> &patlist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryAllRegularPatrolReq QueryAllPatrolReq;
        QueryAllPatrolReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllRegularPatrolReq_T;
        QueryAllPatrolReq.m_uiMsgSeq = 1;
        QueryAllPatrolReq.m_strSID = strSid;

        QueryAllPatrolReq.m_strUserID = strUserID;
        QueryAllPatrolReq.m_strDeviceID = strDevID;
        QueryAllPatrolReq.m_uiBeginIndex = 0;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryAllPatrolReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all regular patrol req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryAllRegularPatrolRsp QueryAllPatrolRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryAllPatrolRsp))
        {
            LOG_ERROR_RLD("Query all regular patrol rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        auto itBegin = QueryAllPatrolRsp.m_planList.begin();
        auto itEnd = QueryAllPatrolRsp.m_planList.end();
        while (itBegin != itEnd)
        {
            Patrol patinfo;
            patinfo.m_strEnable = itBegin->m_strEnable;
            patinfo.m_strPatrolID = itBegin->m_strPlanID;
            patinfo.m_strPatrolInfo = itBegin->m_strStoreInfo;
            patinfo.m_strPatrolName = itBegin->m_strPlanName;
            patinfo.m_strPatrolTimeList.swap(itBegin->m_strPatrolTimeList);
            patinfo.m_strStoreIDList.swap(itBegin->m_strStoreIDList);
            
            patlist.push_back(std::move(patinfo));

            ++itBegin;
        }

        iRet = QueryAllPatrolRsp.m_iRetcode;

        LOG_INFO_RLD("Query all regular patrol and user id is " << strUserID << " and session id is " << strSid << " and device id is " << strDevID <<
            " and return code is " << QueryAllPatrolRsp.m_iRetcode <<
            " and return msg is " << QueryAllPatrolRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::CreateVIP(const std::string &strSid, VIP &vip)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::AddVIPCustomerReq AddVipReq;
        AddVipReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddVIPCustomerReq_T;
        AddVipReq.m_uiMsgSeq = 1;
        AddVipReq.m_strSID = strSid;

        AddVipReq.m_strUserID = vip.m_strVipUserID;

        for (auto itBegin = vip.m_ConsumeHistoryList.begin(), itEnd = vip.m_ConsumeHistoryList.end(); itBegin != itEnd; ++itBegin)
        {
            PassengerFlowProtoHandler::VIPConsumeHistory vph;
            vph.m_dConsumeAmount = itBegin->m_dConsumeAmount;
            vph.m_strConsumeDate = itBegin->m_strConsumeDate;
            vph.m_strGoodsName = itBegin->m_strGoodName;
            vph.m_strSalesman = itBegin->m_strSalesman;
            //vph.m_strVIPID = vip.m_strVipID;
            vph.m_uiGoodsNumber = itBegin->m_uiGoodNum;

            AddVipReq.m_customerInfo.m_consumeHistoryList.push_back(std::move(vph));

        }

        AddVipReq.m_customerInfo.m_strCellphone = vip.m_strCellphone;
        AddVipReq.m_customerInfo.m_strProfilePicture = vip.m_strProfilePicture;
        AddVipReq.m_customerInfo.m_strVIPName = vip.m_strVipName;
        AddVipReq.m_customerInfo.m_strVisitDate = vip.m_strVisitDate;
        AddVipReq.m_customerInfo.m_uiVisitTimes = vip.m_uiVisitTimes;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddVipReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Create vip req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::AddVIPCustomerRsp AddVipRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddVipRsp))
        {
            LOG_ERROR_RLD("Create vip rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = AddVipRsp.m_iRetcode;

        vip.m_strVipID = AddVipRsp.m_strVIPID;

        LOG_INFO_RLD("Create vip and vip id is " << vip.m_strVipID << " and session id is " << strSid <<
            " and return code is " << AddVipRsp.m_iRetcode <<
            " and return msg is " << AddVipRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::DeleteVIP(const std::string &strSid, const std::string &strUserID, const std::string &strVipID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::DeleteVIPCustomerReq DelVipReq;
        DelVipReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteVIPCustomerReq_T;
        DelVipReq.m_uiMsgSeq = 1;
        DelVipReq.m_strSID = strSid;

        DelVipReq.m_strUserID = strUserID;
        DelVipReq.m_strVIPID = strVipID;


        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelVipReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete vip req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::DeleteVIPCustomerRsp DelVipRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelVipRsp))
        {
            LOG_ERROR_RLD("Delete vip rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DelVipRsp.m_iRetcode;

        LOG_INFO_RLD("Delete vip and vip id is " << strVipID << " and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << DelVipRsp.m_iRetcode <<
            " and return msg is " << DelVipRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ModifyVIP(const std::string &strSid, VIP &vip)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::ModifyVIPCustomerReq ModVipReq;
        ModVipReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyVIPCustomerReq_T;
        ModVipReq.m_uiMsgSeq = 1;
        ModVipReq.m_strSID = strSid;

        ModVipReq.m_strUserID = vip.m_strVipUserID;

        for (auto itBegin = vip.m_ConsumeHistoryList.begin(), itEnd = vip.m_ConsumeHistoryList.end(); itBegin != itEnd; ++itBegin)
        {
            PassengerFlowProtoHandler::VIPConsumeHistory vph;
            vph.m_dConsumeAmount = itBegin->m_dConsumeAmount;
            vph.m_strConsumeDate = itBegin->m_strConsumeDate;
            vph.m_strGoodsName = itBegin->m_strGoodName;
            vph.m_strSalesman = itBegin->m_strSalesman;
            //vph.m_strVIPID = vip.m_strVipID;
            vph.m_uiGoodsNumber = itBegin->m_uiGoodNum;

            ModVipReq.m_customerInfo.m_consumeHistoryList.push_back(std::move(vph));

        }

        ModVipReq.m_customerInfo.m_strCellphone = vip.m_strCellphone;
        ModVipReq.m_customerInfo.m_strProfilePicture = vip.m_strProfilePicture;
        ModVipReq.m_customerInfo.m_strVIPName = vip.m_strVipName;
        ModVipReq.m_customerInfo.m_strVisitDate = vip.m_strVisitDate;
        ModVipReq.m_customerInfo.m_uiVisitTimes = vip.m_uiVisitTimes;
        ModVipReq.m_customerInfo.m_strVIPID = vip.m_strVipID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModVipReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify vip req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::ModifyVIPCustomerRsp ModVipRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModVipRsp))
        {
            LOG_ERROR_RLD("Modify vip rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModVipRsp.m_iRetcode;

        LOG_INFO_RLD("Modify vip and vip id is " << vip.m_strVipID << " and session id is " << strSid <<
            " and return code is " << ModVipRsp.m_iRetcode <<
            " and return msg is " << ModVipRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryVIP(const std::string &strSid, VIP &vip)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryVIPCustomerInfoReq QueryVipReq;
        QueryVipReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryVIPCustomerInfoReq_T;
        QueryVipReq.m_uiMsgSeq = 1;
        QueryVipReq.m_strSID = strSid;

        QueryVipReq.m_strUserID = vip.m_strVipUserID;
        QueryVipReq.m_strVIPID = vip.m_strVipID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryVipReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query vip req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryVIPCustomerInfoRsp QueryVipRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryVipRsp))
        {
            LOG_ERROR_RLD("Query vip rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        for (auto itBegin = QueryVipRsp.m_customerInfo.m_consumeHistoryList.begin(), itEnd = QueryVipRsp.m_customerInfo.m_consumeHistoryList.end();
            itBegin != itEnd; ++itBegin)
        {
            VIP::ConsumeHistory vpc;
            vpc.m_dConsumeAmount = itBegin->m_dConsumeAmount;
            vpc.m_strConsumeDate = itBegin->m_strConsumeDate;
            vpc.m_strGoodName = itBegin->m_strGoodsName;
            vpc.m_strSalesman = itBegin->m_strSalesman;
            vpc.m_uiGoodNum = itBegin->m_uiGoodsNumber;
            
            vip.m_ConsumeHistoryList.push_back(std::move(vpc));
        }

        vip.m_strCellphone = QueryVipRsp.m_customerInfo.m_strCellphone;
        vip.m_strProfilePicture = QueryVipRsp.m_customerInfo.m_strProfilePicture;
        vip.m_strRegisterDate = QueryVipRsp.m_customerInfo.m_strRegisterDate;
        //vip.m_strVipID = QueryVipRsp.m_customerInfo.m_strVIPID;
        vip.m_strVipName = QueryVipRsp.m_customerInfo.m_strVIPName;
        //vip.m_strVipUserID = QueryVipRsp.m_customerInfo.
        vip.m_strVisitDate = QueryVipRsp.m_customerInfo.m_strVisitDate;
        vip.m_uiVisitTimes = QueryVipRsp.m_customerInfo.m_uiVisitTimes;        

        iRet = QueryVipRsp.m_iRetcode;

        LOG_INFO_RLD("Query vip and vip id is " << vip.m_strVipID << " and session id is " << strSid << " and vip name is " << vip.m_strVipName <<
            " and return code is " << QueryVipRsp.m_iRetcode <<
            " and return msg is " << QueryVipRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryAllVIP(const std::string &strSid, const std::string &strUserID, const unsigned int uiBeginIndex, std::list<VIP> &viplist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryAllVIPCustomerReq QueryAllVipReq;
        QueryAllVipReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllVIPCustomerReq_T;
        QueryAllVipReq.m_uiMsgSeq = 1;
        QueryAllVipReq.m_strSID = strSid;

        QueryAllVipReq.m_strUserID = strUserID;
        QueryAllVipReq.m_uiBeginIndex = uiBeginIndex;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryAllVipReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all vip req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryAllVIPCustomerRsp QueryAllVipRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryAllVipRsp))
        {
            LOG_ERROR_RLD("Query all vip rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        auto itBegin = QueryAllVipRsp.m_customerList.begin();
        auto itEnd = QueryAllVipRsp.m_customerList.end();
        while (itBegin != itEnd)
        {
            VIP vip;

            for (auto itBegin2 = itBegin->m_consumeHistoryList.begin(), itEnd2 = itBegin->m_consumeHistoryList.end();
                itBegin2 != itEnd2; ++itBegin2)
            {
                VIP::ConsumeHistory vpc;
                vpc.m_dConsumeAmount = itBegin2->m_dConsumeAmount;
                vpc.m_strConsumeDate = itBegin2->m_strConsumeDate;
                vpc.m_strGoodName = itBegin2->m_strGoodsName;
                vpc.m_strSalesman = itBegin2->m_strSalesman;
                vpc.m_uiGoodNum = itBegin2->m_uiGoodsNumber;

                vip.m_ConsumeHistoryList.push_back(std::move(vpc));
            }
            
            vip.m_strCellphone = itBegin->m_strCellphone;
            vip.m_strProfilePicture = itBegin->m_strProfilePicture;
            vip.m_strRegisterDate = itBegin->m_strRegisterDate;
            vip.m_strVipID = itBegin->m_strVIPID;
            vip.m_strVipName = itBegin->m_strVIPName;
            //vip.m_strVipUserID = strUserID;
            vip.m_strVisitDate = itBegin->m_strVisitDate;
            vip.m_uiVisitTimes = itBegin->m_uiVisitTimes;

            viplist.push_back(std::move(vip));

            ++itBegin;
        }

        iRet = QueryAllVipRsp.m_iRetcode;

        LOG_INFO_RLD("Query all vip and user id is " << strUserID << " and session id is " << strSid << " and begin index is " << uiBeginIndex <<
            " and return code is " << QueryAllVipRsp.m_iRetcode <<
            " and return msg is " << QueryAllVipRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::CreateVIPConsumeHistory(const std::string &strSid, const std::string &strUserID, const std::string &strVipID, VIP::ConsumeHistory &vph)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::AddVIPConsumeHistoryReq AddVipConsumeReq;
        AddVipConsumeReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddVIPConsumeHistoryReq_T;
        AddVipConsumeReq.m_uiMsgSeq = 1;
        AddVipConsumeReq.m_strSID = strSid;

        AddVipConsumeReq.m_strUserID = strUserID;
                
        AddVipConsumeReq.m_consumeHistory.m_dConsumeAmount = vph.m_dConsumeAmount;
        AddVipConsumeReq.m_consumeHistory.m_strConsumeDate = vph.m_strConsumeDate;
        AddVipConsumeReq.m_consumeHistory.m_strConsumeID = vph.m_strConsumeID;
        AddVipConsumeReq.m_consumeHistory.m_strGoodsName = vph.m_strGoodName;
        AddVipConsumeReq.m_consumeHistory.m_strSalesman = vph.m_strSalesman;
        AddVipConsumeReq.m_consumeHistory.m_uiGoodsNumber = vph.m_uiGoodNum;
        AddVipConsumeReq.m_consumeHistory.m_strVIPID = strVipID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddVipConsumeReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Create vip consume history req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::AddVIPConsumeHistoryRsp AddVipConsumeRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddVipConsumeRsp))
        {
            LOG_ERROR_RLD("Create vip consume history rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = AddVipConsumeRsp.m_iRetcode;

        vph.m_strConsumeID = AddVipConsumeRsp.m_strConsumeID;

        LOG_INFO_RLD("Create vip consume history and consumer id is " << vph.m_strConsumeID << " and session id is " << strSid <<
            " and return code is " << AddVipConsumeRsp.m_iRetcode <<
            " and return msg is " << AddVipConsumeRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::DeleteVIPConsumeHistory(const std::string &strSid, const std::string &strUserID, const std::string &strConsumeID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::DeleteVIPConsumeHistoryReq DelVipConsumeReq;
        DelVipConsumeReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteVIPConsumeHistoryReq_T;
        DelVipConsumeReq.m_uiMsgSeq = 1;
        DelVipConsumeReq.m_strSID = strSid;

        DelVipConsumeReq.m_strUserID = strUserID;
        DelVipConsumeReq.m_strConsumeID = strConsumeID;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelVipConsumeReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete vip consume req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::DeleteVIPConsumeHistoryRsp DelVipConsumeRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelVipConsumeRsp))
        {
            LOG_ERROR_RLD("Delete vip consume rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DelVipConsumeRsp.m_iRetcode;

        LOG_INFO_RLD("Delete vip consume id is " << strConsumeID << " and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << DelVipConsumeRsp.m_iRetcode <<
            " and return msg is " << DelVipConsumeRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ModifyVIPConsumeHistory(const std::string &strSid, const std::string &strUserID, VIP::ConsumeHistory &vph)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::ModifyVIPConsumeHistoryReq ModVipConsumeReq;
        ModVipConsumeReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyVIPConsumeHistoryReq_T;
        ModVipConsumeReq.m_uiMsgSeq = 1;
        ModVipConsumeReq.m_strSID = strSid;

        ModVipConsumeReq.m_strUserID = strUserID;

        ModVipConsumeReq.m_consumeHistory.m_dConsumeAmount = vph.m_dConsumeAmount;
        ModVipConsumeReq.m_consumeHistory.m_strConsumeDate = vph.m_strConsumeDate;
        ModVipConsumeReq.m_consumeHistory.m_strConsumeID = vph.m_strConsumeID;
        ModVipConsumeReq.m_consumeHistory.m_strGoodsName = vph.m_strGoodName;
        ModVipConsumeReq.m_consumeHistory.m_strSalesman = vph.m_strSalesman;
        ModVipConsumeReq.m_consumeHistory.m_uiGoodsNumber = vph.m_uiGoodNum;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModVipConsumeReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify vip consume history req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::ModifyVIPConsumeHistoryRsp ModVipConsumeRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModVipConsumeRsp))
        {
            LOG_ERROR_RLD("Modify vip consume history rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModVipConsumeRsp.m_iRetcode;

        LOG_INFO_RLD("Modify vip consume history and user id is " << strUserID << " and consume id is " << vph.m_strConsumeID << " and session id is " << strSid <<
            " and return code is " << ModVipConsumeRsp.m_iRetcode <<
            " and return msg is " << ModVipConsumeRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryVIPConsumeHistory(const std::string &strSid, const std::string &strUserID, const std::string &strVipID, 
    const unsigned int uiBeginIndex, std::list<VIP::ConsumeHistory> &vphlist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryAllVIPConsumeHistoryReq QueryVipComsumeHisReq;
        QueryVipComsumeHisReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllVIPConsumeHistoryReq_T;
        QueryVipComsumeHisReq.m_uiMsgSeq = 1;
        QueryVipComsumeHisReq.m_strSID = strSid;

        QueryVipComsumeHisReq.m_strUserID = strUserID;
        QueryVipComsumeHisReq.m_strVIPID = strVipID;
        QueryVipComsumeHisReq.m_uiBeginIndex = uiBeginIndex;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryVipComsumeHisReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query vip consume req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryAllVIPConsumeHistoryRsp QueryVipConsumeHisRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryVipConsumeHisRsp))
        {
            LOG_ERROR_RLD("Query vip consume rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        for (auto itBegin = QueryVipConsumeHisRsp.m_consumeHistoryList.begin(), itEnd = QueryVipConsumeHisRsp.m_consumeHistoryList.end();
            itBegin != itEnd; ++itBegin)
        {
            VIP::ConsumeHistory vpc;
            vpc.m_dConsumeAmount = itBegin->m_dConsumeAmount;
            vpc.m_strConsumeDate = itBegin->m_strConsumeDate;
            vpc.m_strGoodName = itBegin->m_strGoodsName;
            vpc.m_strSalesman = itBegin->m_strSalesman;
            vpc.m_uiGoodNum = itBegin->m_uiGoodsNumber;

            vphlist.push_back(std::move(vpc));

        }

        iRet = QueryVipConsumeHisRsp.m_iRetcode;

        LOG_INFO_RLD("Query vip consume and vip id is " << strVipID << " and session id is " << strSid << " and user id is " << strUserID << 
            " and begin index is " << uiBeginIndex <<
            " and return code is " << QueryVipConsumeHisRsp.m_iRetcode <<
            " and return msg is " << QueryVipConsumeHisRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::UserJoinStore(const std::string &strSid, UserOfStore &us)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::UserJoinStoreReq UserJoinStoreReq;
        UserJoinStoreReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::UserJoinStoreReq_T;
        UserJoinStoreReq.m_uiMsgSeq = 1;
        UserJoinStoreReq.m_strSID = strSid;

        UserJoinStoreReq.m_strRole = us.m_strRole;
        UserJoinStoreReq.m_strStoreID = us.m_strStoreID;
        UserJoinStoreReq.m_strUserID = us.m_strUserID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(UserJoinStoreReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("User join store event req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::UserJoinStoreRsp UserJoinStoreRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, UserJoinStoreRsp))
        {
            LOG_ERROR_RLD("User join store rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = UserJoinStoreRsp.m_iRetcode;

        LOG_INFO_RLD("User join store  and session id is " << strSid << " and user id is " << us.m_strUserID << " and store id is " << us.m_strStoreID <<
            " and role is " << us.m_strRole <<
            " and return code is " << UserJoinStoreRsp.m_iRetcode <<
            " and return msg is " << UserJoinStoreRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::UserQuitStore(const std::string &strSid, const std::string &strAdminID, UserOfStore &us)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::UserQuitStoreReq UserQuitStoreReq;
        UserQuitStoreReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::UserQuitStoreReq_T;
        UserQuitStoreReq.m_uiMsgSeq = 1;
        UserQuitStoreReq.m_strSID = strSid;

        UserQuitStoreReq.m_strAdministratorID = strAdminID;
        UserQuitStoreReq.m_strStoreID = us.m_strStoreID;
        UserQuitStoreReq.m_strUserID = us.m_strUserID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(UserQuitStoreReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("User quit store req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::UserQuitStoreRsp UserQuitStoreRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, UserQuitStoreRsp))
        {
            LOG_ERROR_RLD("User quit store rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = UserQuitStoreRsp.m_iRetcode;

        LOG_INFO_RLD("User quit store and session id is " << strSid << " and user id is " << us.m_strUserID << " and store id is " << us.m_strStoreID <<
            " and admin id is " << strAdminID <<
            " and return code is " << UserQuitStoreRsp.m_iRetcode <<
            " and return msg is " << UserQuitStoreRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryUserOfStore(const std::string &strSid, const UserOfStore &us, std::list<UserOfStore> &uslist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryStoreAllUserReq QueryUserOfStoreReq;
        QueryUserOfStoreReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryStoreAllUserReq_T;
        QueryUserOfStoreReq.m_uiMsgSeq = 1;
        QueryUserOfStoreReq.m_strSID = strSid;

        QueryUserOfStoreReq.m_strStoreID = us.m_strStoreID;
        QueryUserOfStoreReq.m_strUserID = us.m_strUserID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryUserOfStoreReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query user of store req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryStoreAllUserRsp QueryUserOfStoreRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryUserOfStoreRsp))
        {
            LOG_ERROR_RLD("Query user of store rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = QueryUserOfStoreRsp.m_iRetcode;

        for (auto itBegin = QueryUserOfStoreRsp.m_userList.begin(), itEnd = QueryUserOfStoreRsp.m_userList.end(); itBegin != itEnd; ++itBegin)
        {
            UserOfStore us;
            us.m_strRole = itBegin->m_strRole;
            us.m_strUserID = itBegin->m_strUserID;
            us.m_strUserName = itBegin->m_strUserName;

            uslist.push_back(std::move(us));
        }

        LOG_INFO_RLD("Query user of store  and session id is " << strSid << " and user id is " << us.m_strUserID << " and store id is " << us.m_strStoreID <<
            " and return code is " << QueryUserOfStoreRsp.m_iRetcode <<
            " and return msg is " << QueryUserOfStoreRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::CreateEvaluationTemplate(const std::string &strSid, EvaluationTemplate &evt)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::AddEvaluationTemplateReq AddEvaTmpReq;
        AddEvaTmpReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddEvaluationTemplateReq_T;
        AddEvaTmpReq.m_uiMsgSeq = 1;
        AddEvaTmpReq.m_strSID = strSid;

        AddEvaTmpReq.m_strUserID = evt.m_strUserIDOfCreataion;

        AddEvaTmpReq.m_evaluationItem.m_dTotalPoint = evt.m_dEvaluationValue;
        AddEvaTmpReq.m_evaluationItem.m_strDescription = evt.m_strEvaluationDesc;
        AddEvaTmpReq.m_evaluationItem.m_strItemName = evt.m_strEvaluation;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddEvaTmpReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Create evaluation template req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::AddEvaluationTemplateRsp AddEvaTmpRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddEvaTmpRsp))
        {
            LOG_ERROR_RLD("Create evaluation template rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = AddEvaTmpRsp.m_iRetcode;

        evt.m_strEvaluationTmpID = AddEvaTmpRsp.m_strEvaluationID;

        LOG_INFO_RLD("Create evaluation template and evaluation id is " << evt.m_strEvaluationTmpID << " and session id is " << strSid <<
            " and return code is " << AddEvaTmpRsp.m_iRetcode <<
            " and return msg is " << AddEvaTmpRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::DeleteEvaluationTemplate(const std::string &strSid, const std::string &strUserID, const std::string &strEvaluationID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::DeleteEvaluationTemplateReq DelEvaTmpReq;
        DelEvaTmpReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteEvaluationTemplateReq_T;
        DelEvaTmpReq.m_uiMsgSeq = 1;
        DelEvaTmpReq.m_strSID = strSid;

        DelEvaTmpReq.m_strUserID = strUserID;
        DelEvaTmpReq.m_strEvaluationID = strEvaluationID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelEvaTmpReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete evaluation template req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::DeleteEvaluationTemplateRsp DelEvaTmpRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelEvaTmpRsp))
        {
            LOG_ERROR_RLD("Delete evaluation template rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DelEvaTmpRsp.m_iRetcode;

        LOG_INFO_RLD("Delete evaluation template id is " << strEvaluationID << " and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << DelEvaTmpRsp.m_iRetcode <<
            " and return msg is " << DelEvaTmpRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ModifyEvaluationTemplate(const std::string &strSid, EvaluationTemplate &evt)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::ModifyEvaluationTemplateReq ModEvaTmpReq;
        ModEvaTmpReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyEvaluationTemplateReq_T;
        ModEvaTmpReq.m_uiMsgSeq = 1;
        ModEvaTmpReq.m_strSID = strSid;

        ModEvaTmpReq.m_strUserID = evt.m_strUserIDOfCreataion;

        ModEvaTmpReq.m_evaluationItem.m_dTotalPoint = evt.m_dEvaluationValue;
        ModEvaTmpReq.m_evaluationItem.m_strDescription = evt.m_strEvaluationDesc;
        ModEvaTmpReq.m_evaluationItem.m_strItemID = evt.m_strEvaluationTmpID;
        ModEvaTmpReq.m_evaluationItem.m_strItemName = evt.m_strEvaluation;


        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModEvaTmpReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify evaluation template req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::ModifyEvaluationTemplateRsp ModEvaTmpRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModEvaTmpRsp))
        {
            LOG_ERROR_RLD("Modify evaluation template rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModEvaTmpRsp.m_iRetcode;

        LOG_INFO_RLD("Modify evaluation template and evaluation id is " << evt.m_strEvaluationTmpID << " and session id is " << strSid <<
            " and return code is " << ModEvaTmpRsp.m_iRetcode <<
            " and return msg is " << ModEvaTmpRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryEvalutaionTemplate(const std::string &strSid, const std::string &strUserID, std::list<EvaluationTemplate> &evtlist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryAllEvaluationTemplateReq QueryEvaTmpReq;
        QueryEvaTmpReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllEvaluationTemplateReq_T;
        QueryEvaTmpReq.m_uiMsgSeq = 1;
        QueryEvaTmpReq.m_strSID = strSid;

        QueryEvaTmpReq.m_strUserID = strUserID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryEvaTmpReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query evaluation template req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryAllEvaluationTemplateRsp QueryEvaTmpRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryEvaTmpRsp))
        {
            LOG_ERROR_RLD("Query evaluation template rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        for (auto itBegin = QueryEvaTmpRsp.m_evaluationItemList.begin(), itEnd = QueryEvaTmpRsp.m_evaluationItemList.end();
            itBegin != itEnd; ++itBegin)
        {
            EvaluationTemplate evt;
            evt.m_dEvaluationValue = itBegin->m_dTotalPoint;
            evt.m_strEvaluation = itBegin->m_strItemName;
            evt.m_strEvaluationDesc = itBegin->m_strDescription;
            evt.m_strEvaluationTmpID = itBegin->m_strItemID;
            
            evtlist.push_back(std::move(evt));
        }

        iRet = QueryEvaTmpRsp.m_iRetcode;

        LOG_INFO_RLD("Query evaluation template and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << QueryEvaTmpRsp.m_iRetcode <<
            " and return msg is " << QueryEvaTmpRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::CreateEvaluation(const std::string &strSid, Evaluation &ev)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::AddStoreEvaluationReq AddEvaReq;
        AddEvaReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddStoreEvaluationReq_T;
        AddEvaReq.m_uiMsgSeq = 1;
        AddEvaReq.m_strSID = strSid;

        AddEvaReq.m_storeEvaluation.m_strStoreID = ev.m_strStoreID;
        AddEvaReq.m_storeEvaluation.m_strUserIDCheck = ev.m_strUserIDOfCheck;
        AddEvaReq.m_storeEvaluation.m_strUserIDCreate = ev.m_strUserID;

        for (auto itBegin = ev.m_evlist.begin(), itEnd = ev.m_evlist.end(); itBegin != itEnd; ++itBegin)
        {
            PassengerFlowProtoHandler::EvaluationItemScore evs;
            evs.m_dScore = itBegin->m_dEvaluationValue;
            evs.m_evaluationItem.m_strItemID = itBegin->m_strEvaluationTmpID;
            evs.m_strDescription = itBegin->m_strEvaluationDesc;

            AddEvaReq.m_storeEvaluation.m_itemScoreList.push_back(std::move(evs));
        }

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddEvaReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Create evaluation req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::AddStoreEvaluationRsp AddEvaRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddEvaRsp))
        {
            LOG_ERROR_RLD("Create evaluation rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = AddEvaRsp.m_iRetcode;

        ev.m_strEvaluationID = AddEvaRsp.m_strEvaluationID;

        LOG_INFO_RLD("Create evaluation and evaluation id is " << ev.m_strEvaluationID <<
            " and session id is " << strSid << " and user id is " << ev.m_strUserID <<
            " and return code is " << AddEvaRsp.m_iRetcode <<
            " and return msg is " << AddEvaRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
    m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
    CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::DeleteEvaluation(const std::string &strSid, const std::string &strUserID, const std::string &strEvaluationIDOfStore)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::DeleteStoreEvaluationReq DelEvaReq;
        DelEvaReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteStoreEvaluationReq_T;
        DelEvaReq.m_uiMsgSeq = 1;
        DelEvaReq.m_strSID = strSid;

        DelEvaReq.m_strUserID = strUserID;
        DelEvaReq.m_strEvaluationID = strEvaluationIDOfStore;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelEvaReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete evaluation req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::DeleteStoreEvaluationRsp DelEvaTmpRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelEvaTmpRsp))
        {
            LOG_ERROR_RLD("Delete evaluation rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DelEvaTmpRsp.m_iRetcode;

        LOG_INFO_RLD("Delete evaluation id of store is " << strEvaluationIDOfStore << " and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << DelEvaTmpRsp.m_iRetcode <<
            " and return msg is " << DelEvaTmpRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ModifyEvaluation(const std::string &strSid, Evaluation &ev)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::ModifyStoreEvaluationReq ModEvaReq;
        ModEvaReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyStoreEvaluationReq_T;
        ModEvaReq.m_uiMsgSeq = 1;
        ModEvaReq.m_strSID = strSid;

        ModEvaReq.m_strUserID = ev.m_strUserID;
        ModEvaReq.m_storeEvaluation.m_strEvaluationID = ev.m_strEvaluationID;
        ModEvaReq.m_storeEvaluation.m_strStoreID = ev.m_strStoreID;
        ModEvaReq.m_storeEvaluation.m_strUserIDCheck = ev.m_strUserIDOfCheck;
        //ModEvaReq.m_storeEvaluation.m_strUserIDCreate = ev.m_strUserID; //修改人不一定是创建人
        ModEvaReq.m_storeEvaluation.m_uiCheckStatus = ev.m_uiCheckStatus;

        for (auto itBegin = ev.m_evlist.begin(), itEnd = ev.m_evlist.end(); itBegin != itEnd; ++itBegin)
        {
            PassengerFlowProtoHandler::EvaluationItemScore evs;
            evs.m_dScore = itBegin->m_dEvaluationValue;
            evs.m_evaluationItem.m_strItemID = itBegin->m_strEvaluationTmpID;
            evs.m_strDescription = itBegin->m_strEvaluationDesc;

            ModEvaReq.m_storeEvaluation.m_itemScoreList.push_back(std::move(evs));
        }
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModEvaReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify evaluation req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::ModifyStoreEvaluationRsp ModEvaRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModEvaRsp))
        {
            LOG_ERROR_RLD("Modify evaluation rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModEvaRsp.m_iRetcode;

        LOG_INFO_RLD("Modify evaluation and evaluation id is " << ev.m_strEvaluationID <<
        " and session id is " << strSid << " and user id is " << ev.m_strUserID <<
        " and return code is " << ModEvaRsp.m_iRetcode <<
        " and return msg is " << ModEvaRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
    m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
    CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryEvaluation(const std::string &strSid, const std::string &strUserID, Evaluation &ev)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryStoreEvaluationInfoReq QueryEvaReq;
        QueryEvaReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryStoreEvaluationInfoReq_T;
        QueryEvaReq.m_uiMsgSeq = 1;
        QueryEvaReq.m_strSID = strSid;

        QueryEvaReq.m_strUserID = strUserID;
        QueryEvaReq.m_strStoreID = ev.m_strStoreID;
        QueryEvaReq.m_strEvaluationID = ev.m_strEvaluationID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryEvaReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query evaluation req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryStoreEvaluationInfoRsp QueryEvaRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryEvaRsp))
        {
            LOG_ERROR_RLD("Query evaluation rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        ev.m_dEvaluationTotalValue = QueryEvaRsp.m_storeEvaluation.m_dTotalScore;
        ev.m_strEvaluationDate = QueryEvaRsp.m_storeEvaluation.m_strCreateDate;
        ev.m_strEvaluationID = QueryEvaRsp.m_storeEvaluation.m_strEvaluationID;
        ev.m_strStoreID = QueryEvaRsp.m_storeEvaluation.m_strStoreID;
        ev.m_strUserID = QueryEvaRsp.m_storeEvaluation.m_strUserIDCreate;
        ev.m_strUserIDOfCheck = QueryEvaRsp.m_storeEvaluation.m_strUserIDCheck;
        ev.m_uiCheckStatus = QueryEvaRsp.m_storeEvaluation.m_uiCheckStatus;

        for (auto itBegin = QueryEvaRsp.m_storeEvaluation.m_itemScoreList.begin(), itEnd = QueryEvaRsp.m_storeEvaluation.m_itemScoreList.end();
            itBegin != itEnd; ++itBegin)
        {
            EvaluationTemplate evt;
            evt.m_dEvaluationValue = itBegin->m_evaluationItem.m_dTotalPoint;
            evt.m_dEvaValueActive = itBegin->m_dScore;
            evt.m_strEvaDescActive = itBegin->m_strDescription;
            evt.m_strEvaluation = itBegin->m_evaluationItem.m_strItemName;
            evt.m_strEvaluationDesc = itBegin->m_evaluationItem.m_strDescription;
            evt.m_strEvaluationTmpID = itBegin->m_evaluationItem.m_strItemID;
            
            ev.m_evlist.push_back(std::move(evt));
        }

        iRet = QueryEvaRsp.m_iRetcode;

        LOG_INFO_RLD("Query evaluation template and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << QueryEvaRsp.m_iRetcode <<
            " and return msg is " << QueryEvaRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryAllEvaluationOfStore(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, 
    const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiBeginIndex, std::list<Evaluation> &evlist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryAllStoreEvaluationReq QueryEvaReq;
        QueryEvaReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllStoreEvaluationReq_T;
        QueryEvaReq.m_uiMsgSeq = 1;
        QueryEvaReq.m_strSID = strSid;

        QueryEvaReq.m_strUserID = strUserID;
        QueryEvaReq.m_strStoreID = strStoreID;
        QueryEvaReq.m_strBeginDate = strBeginDate;
        QueryEvaReq.m_strEndDate = strEndDate;
        QueryEvaReq.m_uiBeginIndex = uiBeginIndex;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryEvaReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all evaluation req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryAllStoreEvaluationRsp QueryEvaRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryEvaRsp))
        {
            LOG_ERROR_RLD("Query all evaluation rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        for (auto itBegin = QueryEvaRsp.m_storeEvaluationList.begin(), itEnd = QueryEvaRsp.m_storeEvaluationList.end();
            itBegin != itEnd; ++itBegin)
        {
            Evaluation ev;
            ev.m_dEvaluationTotalValue = itBegin->m_dTotalScore;
            ev.m_strEvaluationDate = itBegin->m_strCreateDate;
            ev.m_strEvaluationID = itBegin->m_strEvaluationID;
            ev.m_strStoreID = itBegin->m_strStoreID;
            ev.m_strUserID = itBegin->m_strUserIDCreate;
            ev.m_strUserIDOfCheck = itBegin->m_strUserIDCheck;
            ev.m_uiCheckStatus = itBegin->m_uiCheckStatus;

            evlist.push_back(std::move(ev));
        }

        iRet = QueryEvaRsp.m_iRetcode;

        LOG_INFO_RLD("Query all evaluation template and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << QueryEvaRsp.m_iRetcode <<
            " and return msg is " << QueryEvaRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ParseMsgOfCompact(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter wr)
{
    auto itFind = pMsgInfoMap->find("compact_msg");
    if (pMsgInfoMap->end() != itFind)
    {
        const std::string &strValue = itFind->second;

        LOG_INFO_RLD("Compact msg is " << strValue);

        Json::Reader reader;
        Json::Value root;
        if (!reader.parse(strValue, root, false))
        {
            LOG_ERROR_RLD("Compact msg parse failed beacuse value parsed failed and value is " << strValue);
            return false;
        }

        if (!root.isObject())
        {
            LOG_ERROR_RLD("Compact msg parse failed beacuse json root parsed failed and value is " << strValue);
            return false;
        }

        Json::Value::Members members = root.getMemberNames();
        auto itBegin = members.begin();
        auto itEnd = members.end();
        while (itBegin != itEnd)
        {
            if (root[*itBegin].type() == Json::stringValue) //目前只支持string类型json字段
            {
                pMsgInfoMap->insert(MsgInfoMap::value_type(*itBegin, root[*itBegin].asString()));
            }

            ++itBegin;
        }

        pMsgInfoMap->erase(itFind);

    }

    return true;
}

void PassengerFlowMsgHandler::WriteMsg(const std::map<std::string, std::string> &MsgMap, MsgWriter writer, const bool blResult, boost::function<void(void*)> PostFunc)
{
    Json::Value jsBody;
    auto itBegin = MsgMap.begin();
    auto itEnd = MsgMap.end();
    while (itBegin != itEnd)
    {
        jsBody[itBegin->first] = itBegin->second;

        ++itBegin;
    }

    if (NULL != PostFunc)
    {
        PostFunc((void *)&jsBody);
    }

    //Json::FastWriter fastwriter;
    Json::StyledWriter stylewriter;
    const std::string &strBody = stylewriter.write(jsBody); //fastwriter.write(jsBody);//jsBody.toStyledString();

    //writer(strBody.c_str(), strBody.size(), MsgWriterModel::PRINT_MODEL);

    std::string strOutputMsg;
    if (!blResult)
    {
        strOutputMsg = "Status: 500  Error\r\nContent-Type: text/html\r\n\r\n";
    }
    else
    {
        strOutputMsg = "Status: 200 OK\r\nContent-Type: text/html\r\n\r\n";
    }

    strOutputMsg += strBody;
    strOutputMsg += "\r\n";

    writer(strOutputMsg.c_str(), strOutputMsg.size(), MsgWriterModel::PRINT_MODEL);

    //writer("Content-type: text/*\r\n\r\n", 0, MsgWriterModel::PRINT_MODEL);
    //writer("<title>FastCGI Hello! (C, fcgi_stdio library)</title>\n", 0, MsgWriterModel::PRINT_MODEL);

}


bool PassengerFlowMsgHandler::ValidDatetime(const std::string &strDatetime, const bool IsTime)
{
    if (strDatetime.empty())
    {
        return false;
    }

    if (IsTime)
    {
        boost::regex reg0("([0-9]{2}[0-9]{2}[0-9]{2})"); //HHmmss
        boost::regex reg2("([0-9]{2}:[0-9]{2}:[0-9]{2})"); //HH:mm:ss
        boost::regex reg4("([0-9]{2}:[0-9]{2})"); //HH:mm
        if (!boost::regex_match(strDatetime, reg0) && !boost::regex_match(strDatetime, reg2) && !boost::regex_match(strDatetime, reg4))
        {
            LOG_ERROR_RLD("Time is invalid and input date is " << strDatetime);
            return false;
        }

        return true;
    }

    boost::regex reg0("([0-9]{4}[0-9]{2}[0-9]{2}[0-9]{2}[0-9]{2}[0-9]{2})"); //yyyyMMddHHmmss
    boost::regex reg1("([0-9]{4}[0-9]{2}[0-9]{2})"); //yyyyMMdd
    boost::regex reg2("([0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2})"); //yyyy-MM-dd HH:mm:ss
    boost::regex reg3("([0-9]{4}-[0-9]{2}-[0-9]{2})"); ////yyyy-MM-dd
    boost::regex reg4("([0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2})"); //yyyy-MM-dd  HH:mm

    if (!boost::regex_match(strDatetime, reg0) && !boost::regex_match(strDatetime, reg1) && !boost::regex_match(strDatetime, reg2) &&
        !boost::regex_match(strDatetime, reg3) && !boost::regex_match(strDatetime, reg4))
    {
        LOG_ERROR_RLD("Date time is invalid and input date is " << strDatetime);
        return false;
    }

    return true;
}

bool PassengerFlowMsgHandler::GetEntrance(const std::string &strEntrance, std::list<EntranceInfo> &einfolist)
{
    bool blResult = false;

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(strEntrance, root))
    {
        LOG_ERROR_RLD("Entrance info failed, raw data is : " << strEntrance);
        return blResult;
    }

    if (!root.isArray())
    {
        LOG_ERROR_RLD("Entrance info failed, raw data is : " << strEntrance);
        return blResult;
    }

    for (unsigned int i = 0; i < root.size(); ++i)
    {
        auto jsEntrance = root[i];
        if (jsEntrance.isNull() || !jsEntrance.isObject())
        {
            LOG_ERROR_RLD("Entrance info failed, raw data is: " << strEntrance);
            return blResult;
        }

        auto jsName = jsEntrance["name"];
        if (jsName.isNull() || !jsName.isString() || jsName.asString().empty())
        {
            LOG_ERROR_RLD("Entrance info failed, raw data is: " << strEntrance);
            return blResult;
        }

        EntranceInfo einfo;
        einfo.m_strName = jsName.asString();

        auto jsDevIDList = jsEntrance["device_id"];
        for (unsigned int k = 0; k < jsDevIDList.size(); ++k)
        {
            auto jsDevID = jsDevIDList[k];

            if (jsDevID.isNull() || !jsDevID.isString() || jsDevID.asString().empty())
            {
                LOG_ERROR_RLD("Entrance info failed, raw data is: " << strEntrance);
                return blResult;
            }
            einfo.m_DeviceIDList.push_back(jsDevID.asString());
        }

        einfolist.push_back(einfo);
    }

    return true;
}

bool PassengerFlowMsgHandler::GetEntrance(const std::string &strDeviceIDInfo, EntranceInfo &einfo)
{
    bool blResult = false;

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(strDeviceIDInfo, root))
    {
        LOG_ERROR_RLD("Entrance info failed, raw data is : " << strDeviceIDInfo);
        return blResult;
    }

    if (!root.isArray())
    {
        LOG_ERROR_RLD("Entrance info failed, raw data is : " << strDeviceIDInfo);
        return blResult;
    }

    LOG_INFO_RLD("Entrance device id list size is " << root.size());

    for (unsigned int i = 0; i < root.size(); ++i)
    {
        auto jsDevID = root[i];
        if (jsDevID.isNull() || !jsDevID.isString())
        {
            LOG_ERROR_RLD("Entrance info failed, raw data is: " << strDeviceIDInfo);
            return blResult;
        }

        einfo.m_DeviceIDList.push_back(jsDevID.asString());

        LOG_INFO_RLD("Entrance device id is " << jsDevID.asString());
    }

    return true;
}

bool PassengerFlowMsgHandler::GetPassengerFlowInfo(const std::string &strPassengerFlowInfo, std::list<PassengerFlowInfo> &pfinfolist)
{
    bool blResult = false;

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(strPassengerFlowInfo, root))
    {
        LOG_ERROR_RLD("Passenger flow info failed, raw data is : " << strPassengerFlowInfo);
        return blResult;
    }

    if (!root.isArray())
    {
        LOG_ERROR_RLD("Passenger flow info failed, raw data is : " << strPassengerFlowInfo);
        return blResult;
    }

    LOG_INFO_RLD("Passenger flow list size is " << root.size());

    for (unsigned int i = 0; i < root.size(); ++i)
    {
        auto jsPFInfo = root[i];
        if (jsPFInfo.isNull() || !jsPFInfo.isObject())
        {
            LOG_ERROR_RLD("Passenger flow info failed, raw data is: " << strPassengerFlowInfo);
            return blResult;
        }

        auto jsDateTime = jsPFInfo["date_time"];
        if (jsDateTime.isNull() || !jsDateTime.isString() || jsDateTime.asString().empty())
        {
            LOG_ERROR_RLD("Passenger flow info failed, raw data is: " << strPassengerFlowInfo);
            return blResult;
        }

        if (!ValidDatetime(jsDateTime.asString()))
        {
            LOG_ERROR_RLD("Passenger flow data time is error: " << jsDateTime.asString());
            return blResult;
        }

        PassengerFlowInfo pfinfo;
        pfinfo.m_strDateTime = jsDateTime.asString();

        auto GetNum = [&](const std::string &strKey, unsigned int &uiNum) ->bool
        {
            auto jsNum = jsPFInfo[strKey];
            if (jsNum.isNull() || !jsNum.isString() || jsNum.asString().empty())
            {
                LOG_ERROR_RLD("Passenger flow info failed, raw data is: " << strPassengerFlowInfo << " and key is " << strKey);
                return blResult;
            }

            if (!ValidType<unsigned int>(jsNum.asString(), uiNum))
            {
                LOG_ERROR_RLD("Passenger flow number is error: " << jsNum.asString() << " and key is " << strKey);
                return blResult;
            }

            return true;
        };
        
        if (!GetNum("enter_number", pfinfo.m_uiEnterNum) || !GetNum("leave_number", pfinfo.m_uiLeaveNum)) //|| !GetNum("stay_number"))
        {
            return blResult;
        }
        
        pfinfolist.push_back(std::move(pfinfo));

    }

    return true;
}

template<typename T>
bool PassengerFlowMsgHandler::ValidType(const std::string &strValue, T &ValueT)
{
    try
    {
        ValueT = boost::lexical_cast<T>(strValue);
    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("Type is invalid and error msg is " << e.what() << " and input is " << strValue);
        return false;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Type is invalid and input is " << strValue);
        return false;
    }

    return true;
}

bool PassengerFlowMsgHandler::GetValueList(const std::string &strValue, std::list<std::string> &strValueList)
{
    bool blResult = false;

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(strValue, root))
    {
        LOG_ERROR_RLD("Value info parse failed, raw data is : " << strValue);
        return blResult;
    }

    if (!root.isArray())
    {
        LOG_ERROR_RLD("Value info parse failed, raw data is : " << strValue);
        return blResult;
    }

    LOG_INFO_RLD("Value list size is " << root.size());

    for (unsigned int i = 0; i < root.size(); ++i)
    {
        auto jsValueItem = root[i];
        if (jsValueItem.isNull() || !jsValueItem.isString())
        {
            LOG_ERROR_RLD("Value info type is error, raw data is: " << strValue);
            return blResult;
        }

        strValueList.emplace_back(jsValueItem.asString());

        LOG_INFO_RLD("Value item is " << jsValueItem.asString());
    }

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::GetValueFromList(const std::string &strValue, boost::function<bool(Json::Value &)> ParseFunc)
{
    bool blResult = false;

    if (NULL == ParseFunc)
    {
        LOG_ERROR_RLD("Parse func is null.");
        return blResult;
    }

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(strValue, root))
    {
        LOG_ERROR_RLD("Value info parse failed, raw data is : " << strValue);
        return blResult;
    }

    if (!root.isArray())
    {
        LOG_ERROR_RLD("Value info parse failed, raw data is : " << strValue);
        return blResult;
    }

    LOG_INFO_RLD("Value list size is " << root.size());

    for (unsigned int i = 0; i < root.size(); ++i)
    {
        auto jsValueItem = root[i];
        if (jsValueItem.isNull())
        {
            LOG_ERROR_RLD("Value info type is null, raw data is: " << strValue);
            return blResult;
        }

        if (!ParseFunc(jsValueItem))
        {
            LOG_ERROR_RLD("Value parse failed and row date is " << strValue);
            return blResult;
        }
    }

    blResult = true;

    return blResult;
}
