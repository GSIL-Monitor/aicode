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

const std::string PassengerFlowMsgHandler::CREATE_DOMAIN("create_domain");

const std::string PassengerFlowMsgHandler::REMOVE_DOMAIN("remove_domain");

const std::string PassengerFlowMsgHandler::MODIFY_DOMAIN("modify_domain");

const std::string PassengerFlowMsgHandler::QUERY_DOMAIN("query_domain");

const std::string PassengerFlowMsgHandler::QUERY_ALL_DOMAIN("query_all_domain");

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

const std::string PassengerFlowMsgHandler::QUERY_ALL_USER_LIST("query_all_userlist");

const std::string PassengerFlowMsgHandler::CREATE_EVALUATION_TEMPLATE("create_evaluation_template");

const std::string PassengerFlowMsgHandler::DELETE_EVALUATION_TEMPLATE("remove_evaluation_template");

const std::string PassengerFlowMsgHandler::MODIFY_EVALUATION_TEMPLATE("modify_evaluation_template");

const std::string PassengerFlowMsgHandler::QUERY_EVALUATION_TEMPLATE("query_evaluation_template");


const std::string PassengerFlowMsgHandler::CREATE_EVALUATION_OF_STORE("create_store_evaluation");

const std::string PassengerFlowMsgHandler::DELETE_EVALUATION_OF_STORE("remove_store_evaluation");

const std::string PassengerFlowMsgHandler::MODIFY_EVALUATION_OF_STORE("modify_store_evaluation");

const std::string PassengerFlowMsgHandler::QUERY_EVALUATION_OF_STORE("query_store_evaluation");

const std::string PassengerFlowMsgHandler::QUERY_ALL_EVALUATION_OF_STORE("query_all_store_evaluation");

const std::string PassengerFlowMsgHandler::CREATE_PATROL_RECORD("create_patrol_record");

const std::string PassengerFlowMsgHandler::DELETE_PATROL_RECORD("remove_patrol_record");

const std::string PassengerFlowMsgHandler::MODIFY_PATROL_RECORD("modify_patrol_record");

const std::string PassengerFlowMsgHandler::QUERY_PATROL_RECORD("query_patrol_record");

const std::string PassengerFlowMsgHandler::QUERY_ALL_PATROL_RECORD("query_all_patrol_record");

const std::string PassengerFlowMsgHandler::CREATE_STORE_SENSOR("add_store_sensor");

const std::string PassengerFlowMsgHandler::DELETE_STORE_SENSOR("remove_store_sensor");

const std::string PassengerFlowMsgHandler::MODIFY_STORE_SENSOR("modify_store_sensor");

const std::string PassengerFlowMsgHandler::QUERY_STORE_SENSOR("query_store_sensor");

const std::string PassengerFlowMsgHandler::QUERY_ALL_STORE_SENSOR("query_all_store_sensor");

const std::string PassengerFlowMsgHandler::REPORT_STORE_SENSOR("report_sensor_info");

const std::string PassengerFlowMsgHandler::QUERY_PATROL_RESULT_REPORT("query_patrol_result_report");

const std::string PassengerFlowMsgHandler::REPORT_STORE_SENSOR_ALARM("report_alarm_info");

const std::string PassengerFlowMsgHandler::QUERY_SENSOR_ALARM_THRESHOLD("query_alarm_threshold");

const std::string PassengerFlowMsgHandler::REMOVE_SENSOR_RECORDS("remove_store_sensor_records");

const std::string PassengerFlowMsgHandler::REMOVE_SENSOR_ALARM_RECORDS("remove_store_sensor_alarm_records");

const std::string PassengerFlowMsgHandler::QUERY_SENSOR_RECORDS("query_store_sensor_records");

const std::string PassengerFlowMsgHandler::QUERY_SENSOR_ALARM_RECORDS("query_store_sensor_alarm_records");

const std::string PassengerFlowMsgHandler::ADD_ROLE("add_role");

const std::string PassengerFlowMsgHandler::REMOVE_ROLE("remove_role");

const std::string PassengerFlowMsgHandler::MODIFY_ROLE("modify_role");

const std::string PassengerFlowMsgHandler::QUERY_ROLE("query_role");

const std::string PassengerFlowMsgHandler::QUERY_ALL_ROLE("query_all_role");

const std::string PassengerFlowMsgHandler::USER_BIND_ROLE("user_bind_role");

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

    if (PassengerFlowProtoHandler::CustomerFlowMsgType::CustomerFlowPreHandleRsp_T == mtype)
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

    std::list<std::string> strPhoneList;
    std::string strPhone;
    itFind = pMsgInfoMap->find("phone");
    if (pMsgInfoMap->end() != itFind)
    {
        strPhone = itFind->second;

        if (!GetValueList(strPhone, strPhoneList))
        {
            LOG_ERROR_RLD("Parse phone info failed and value is " << strPhone);
            return blResult;
        }
    }

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    itFind = pMsgInfoMap->find("domainid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Domain id not found.");
        return blResult;
    }
    const std::string strDomainID = itFind->second;

    std::string strOpenState = "1";
    unsigned int uiOpenState = 1;
    itFind = pMsgInfoMap->find("open_state");
    if (pMsgInfoMap->end() != itFind)
    {
        strOpenState = itFind->second;
        if (!ValidType<unsigned int>(strOpenState, uiOpenState))
        {
            LOG_ERROR_RLD("Open state is error and input value is " << strOpenState);
            return blResult;
        }
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Add store info received and session id is " << strSid << " and user id is " << strUserID << " and store name is " << strStoreName 
        << " and goods category is " << strGoodsCategory << " and domain id is " << strDomainID
        << " and extend is " << strExtend << " and address is " << strAddress << " and open state is " << uiOpenState);

    StoreInfo store;
    store.m_strAddress = strAddress;
    store.m_strPhoneList.swap(strPhoneList);
    store.m_strExtend = strExtend;
    store.m_strGoodsCategory = strGoodsCategory;
    store.m_strStoreName = strStoreName;
    store.m_strDomainID = strDomainID;
    store.m_uiOpenState = uiOpenState;

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

    std::list<std::string> strPhoneList;
    std::string strPhone;
    itFind = pMsgInfoMap->find("phone");
    if (pMsgInfoMap->end() != itFind)
    {
        strPhone = itFind->second;

        if (!GetValueList(strPhone, strPhoneList))
        {
            LOG_ERROR_RLD("Parse phone info failed and value is " << strPhone);
            return blResult;
        }
    }

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    std::string strDomainID;
    itFind = pMsgInfoMap->find("domainid");
    if (pMsgInfoMap->end() != itFind)
    {
        strDomainID = itFind->second;
    }

    std::string strOpenState = "1";
    unsigned int uiOpenState = 1;
    itFind = pMsgInfoMap->find("open_state");
    if (pMsgInfoMap->end() != itFind)
    {
        strOpenState = itFind->second;
        if (!ValidType<unsigned int>(strOpenState, uiOpenState))
        {
            LOG_ERROR_RLD("Open state is error and input value is " << strOpenState);
            return blResult;
        }
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify store info received and session id is " << strSid << " and user id is " << strUserID << " and store id is " << strStoreID
        << " and store name is " << strStoreName << " and goods category is " << strGoodsCategory << " and domain id is " << strDomainID
        << " and extend is " << strExtend << " and address is " << strAddress << " and open state is " << uiOpenState);

    StoreInfo store;
    store.m_strAddress = strAddress;
    store.m_strPhoneList.swap(strPhoneList);
    store.m_strExtend = strExtend;
    store.m_strGoodsCategory = strGoodsCategory;
    store.m_strStoreName = strStoreName;
    store.m_strStoreID = strStoreID;
    store.m_strDomainID = strDomainID;
    store.m_uiOpenState = uiOpenState;

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
    Json::Value jsEtrList;
    Json::Value jsDmiList;
    Json::Value jsPhoneList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsEtrList, &jsDmiList, &jsPhoneList)
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
                (*pJsBody)["entrance"] = jsEtrList;
                (*pJsBody)["domain"] = jsDmiList;
                (*pJsBody)["phone"] = jsPhoneList;
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

    std::list<std::string> strPhoneList;
    std::list<DomainInfo> dmilist;
    std::list<EntranceInfo> etilist;
    StoreInfo sinfo;
    sinfo.m_strStoreID = strStoreID;

    if (!QueryStore(strSid, strUserID, sinfo, etilist, dmilist, strPhoneList))
    {
        LOG_ERROR_RLD("Query store handle failed");
        return blResult;
    }

    //jsDmiList["domainid"] = sinfo.m_strDomainID;
    //jsDmiList["name"] = sinfo.m_strDomainName;

    Json::Value jsPhoneInfo;
    unsigned int uiPhoneK = 0;
    for (auto itBegin = strPhoneList.begin(), itEnd = strPhoneList.end(); itBegin != itEnd; ++itBegin, ++uiPhoneK)
    {
        jsPhoneList[uiPhoneK] = *itBegin;
    }

    for (auto itBegin = dmilist.begin(), itEnd = dmilist.end(); itBegin != itEnd; ++itBegin)
    {
        Json::Value jsDmi;
        jsDmi["domainid"] = itBegin->m_strDomainID;
        jsDmi["name"] = itBegin->m_strDomainName;
        jsDmiList.append(jsDmi);
    }
    
    auto itBegin = etilist.begin();
    auto itEnd = etilist.end();
    while (itBegin != itEnd)
    {
        Json::Value jsEntrance;
        jsEntrance["entrance_name"] = itBegin->m_strName;
        jsEntrance["entrance_id"] = itBegin->m_strID;
        jsEntrance["picture"] = itBegin->m_strPicture;

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

        jsEtrList.append(jsEntrance);

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
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("open_state", boost::lexical_cast<std::string>(sinfo.m_uiOpenState)));

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

    std::string strDomainID;
    itFind = pMsgInfoMap->find("domainid");
    if (pMsgInfoMap->end() != itFind)
    {
        strDomainID = itFind->second;
    }
    
    unsigned int uiOpenState = 0xFFFFFFFF;
    std::string strOpenState;
    itFind = pMsgInfoMap->find("open_state");
    if (pMsgInfoMap->end() != itFind)
    {
        strOpenState = itFind->second;

        if (!ValidType<unsigned int>(strOpenState, uiOpenState))
        {
            LOG_ERROR_RLD("Open state is invalid and value is " << strOpenState);
            return blResult;
        }
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query all store info received and session id is " << strSid << " and user id is " << strUserID << " and begin index is " << uiBeginIndex
        << " and domian id is " << strDomainID << " and open state is " << uiOpenState);

    std::list<StoreAndEntranceInfo> storelist;

    if (!QueryAllStore(strSid, strUserID, uiBeginIndex, strDomainID, uiOpenState, storelist))
    {
        LOG_ERROR_RLD("Query all store handle failed");
        return blResult;
    }

    auto itBegin = storelist.begin();
    auto itEnd = storelist.end();
    while (itBegin != itEnd)
    {
        Json::Value jsStore;
        jsStore["store_id"] = itBegin->stinfo.m_strStoreID;
        jsStore["store_name"] = itBegin->stinfo.m_strStoreName;
        jsStore["address"] = itBegin->stinfo.m_strAddress;
        jsStore["open_state"] = boost::lexical_cast<std::string>(itBegin->stinfo.m_uiOpenState);
        
        Json::Value jsEntranceInfoList;
        for (auto itB1 = itBegin->etinfolist.begin(), itE1 = itBegin->etinfolist.end(); itB1 != itE1; ++itB1)
        {
            Json::Value jsEntranceInfo;
            jsEntranceInfo["entrance_id"] = itB1->m_strID;
            jsEntranceInfo["entrance_name"] = itB1->m_strName;
            jsEntranceInfo["picture"] = itB1->m_strPicture;

            Json::Value jsDevid;
            unsigned int i = 0;
            auto itB2 = itB1->m_DeviceIDList.begin();
            auto itE2 = itB1->m_DeviceIDList.end();
            while (itB2 != itE2)
            {
                jsDevid[i] = *itB2;

                ++itB2;
                ++i;
            }

            jsEntranceInfo["device_id"] = jsDevid;

            jsEntranceInfoList.append(jsEntranceInfo);
        }

        jsStore["entrance"] = jsEntranceInfoList;

        Json::Value jsDmi;
        jsDmi["domainid"] = itBegin->stinfo.m_strDomainID;
        jsDmi["name"] = itBegin->stinfo.m_strDomainName;
        jsStore["domain"] = jsDmi;

        Json::Value jsPhoneInfo;
        unsigned int i = 0;
        auto itB2 = itBegin->stinfo.m_strPhoneList.begin();
        auto itE2 = itBegin->stinfo.m_strPhoneList.end();
        while (itB2 != itE2)
        {
            jsPhoneInfo[i] = *itB2;

            ++itB2;
            ++i;
        }
        jsStore["phone"] = jsPhoneInfo;

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

    std::string strPicture;
    itFind = pMsgInfoMap->find("picture");
    if (pMsgInfoMap->end() != itFind)
    {
        strPicture = itFind->second;
    }

    std::string strDevIDInfo;
    itFind = pMsgInfoMap->find("deviceid");
    if (pMsgInfoMap->end() != itFind)
    {
        strDevIDInfo = itFind->second;
    }

    EntranceInfo einfo;
    einfo.m_strName = strEntranceName;
    einfo.m_strPicture = strPicture;

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

    std::string strPicture;
    itFind = pMsgInfoMap->find("picture");
    if (pMsgInfoMap->end() != itFind)
    {
        strPicture = itFind->second;
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

    if (!ModifyEntrance(strSid, strUserID, strStoreID, strEntranceID, strEntranceName, strPicture, einfoadd, einfodel))
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
    if (pMsgInfoMap->end() != itFind)
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

    std::string strRemark;
    itFind = pMsgInfoMap->find("remark");
    if (pMsgInfoMap->end() != itFind)
    {
        strRemark = itFind->second;

        ////
        //Json::Value jsRemark;
        //Json::Reader reader;
        //if (!reader.parse(strRemark, jsRemark, false))
        //{
        //    LOG_ERROR_RLD("Parsed failed and value is " << strRemark);
        //    return blResult;
        //}
    }
    
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
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    if (!ReportEvent(strSid, einfo))
    {
        LOG_ERROR_RLD("Report event failed.");
        return blResult;
    }

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
   
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete event info received and session id is " << strSid << " and user id is " << strUserID
        << " and event id  is " << strEventID);

    if (!DeleteEvent(strSid, strUserID, strEventID))
    {
        LOG_ERROR_RLD("Delete event failed.");
        return blResult;
    }

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

    unsigned int uiViewState = 0xFFFFFFFF;
    std::string strViewState;
    itFind = pMsgInfoMap->find("view_state");
    if (pMsgInfoMap->end() != itFind)
    {
        strViewState = itFind->second;        
        if (!ValidType<unsigned int>(strViewState, uiViewState))
        {
            LOG_ERROR_RLD("View state value parse failed.");
            return blResult;
        }
    }

    std::string strRemark;
    itFind = pMsgInfoMap->find("remark");
    if (pMsgInfoMap->end() != itFind)
    {
        strRemark = itFind->second;

        ////
        //Json::Value jsRemark;
        //Json::Reader reader;
        //if (!reader.parse(strRemark, jsRemark, false))
        //{
        //    LOG_ERROR_RLD("Parsed failed and value is " << strRemark);
        //    return blResult;
        //}
    }

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
    einfo.m_strViewState = boost::lexical_cast<std::string>(uiViewState);

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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify event info received and session id is " << strSid << " and user id is " << strUserID
        << " and event id  is " << einfo.m_strEventID);

    if (!ModifyEvent(strSid, einfo))
    {
        LOG_ERROR_RLD("Modify event failed.");
        return blResult;
    }

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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query event info received and session id is " << strSid << " and user id is " << strUserID
        << " and event id  is " << strEventID);

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
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("view_state", einfo.m_strViewState));

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("store_id", einfo.m_strStoreID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("store_name", einfo.m_strStoreName));

            
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

    unsigned int uiRelation = 0xFFFFFFFF;
    std::string strRelation;
    itFind = pMsgInfoMap->find("relation");
    if (pMsgInfoMap->end() != itFind)
    {
        strRelation = itFind->second;
        if (!ValidType<unsigned int>(strRelation, uiRelation))
        {
            LOG_ERROR_RLD("Relation parse failed.");
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

    unsigned int uiProcessState = 0xFFFFFFFF;
    std::string strProcessState;
    itFind = pMsgInfoMap->find("process_state");
    if (pMsgInfoMap->end() != itFind)
    {
        strProcessState = itFind->second;        
        if (!ValidType<unsigned int>(strProcessState, uiProcessState))
        {
            LOG_ERROR_RLD("Process state parse failed.");
            return blResult;
        }
    }

    unsigned int uiEventType = 0xFFFFFFFF;
    std::string strEventType;
    itFind = pMsgInfoMap->find("event_type");
    if (pMsgInfoMap->end() != itFind)
    {
        strEventType = itFind->second;
        if (!ValidType<unsigned int>(strEventType, uiEventType))
        {
            LOG_ERROR_RLD("Event type parse failed.");
            return blResult;
        }
    }
    
    std::string strBeginDate;
    itFind = pMsgInfoMap->find("begindate");
    if (pMsgInfoMap->end() != itFind)
    {
        strBeginDate = itFind->second;
        if (!ValidDatetime(strBeginDate))
        {
            LOG_ERROR_RLD("Begin date is invalid and value is " << strBeginDate);
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
            LOG_ERROR_RLD("End date is invalid and value is " << strEndDate);
            return blResult;
        }
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query all event info received and session id is " << strSid << " and user id is " << strUserID << " and begin index is " << uiBeginIndex <<
        " and process state is " << uiProcessState << " and begin date is " << strBeginDate << " and end date is " << strEndDate);

    std::list<EventInfo> einfoList;    
    if (!QueryAllEvent(strSid, strUserID, uiRelation, uiBeginIndex, uiProcessState, uiEventType, strBeginDate, strEndDate, einfoList))
    {
        LOG_ERROR_RLD("Query all event handle failed");
        return blResult;
    }

    auto itBegin = einfoList.begin();
    auto itEnd = einfoList.end();
    while (itBegin != itEnd)
    {
        Json::Value jsRemark;
        if (!itBegin->m_strRemark.empty())
        {
            
            Json::Reader reader;
            if (!reader.parse(itBegin->m_strRemark, jsRemark, false))
            {
                LOG_ERROR_RLD("Parsed failed and value is " << itBegin->m_strRemark);
                return blResult;
            }
        }
        
        Json::Value jsEventInfo;
        jsEventInfo["store_id"] = itBegin->m_strStoreID;
        jsEventInfo["store_name"] = itBegin->m_strStoreName;

        jsEventInfo["event_id"] = itBegin->m_strEventID;
        jsEventInfo["source"] = itBegin->m_strSource;
        jsEventInfo["submit_date"] = itBegin->m_strSubmitDate;
        jsEventInfo["expire_date"] = itBegin->m_strExpireDate;
        jsEventInfo["process_state"] = itBegin->m_strProcessState;
        jsEventInfo["create_date"] = itBegin->m_strCreateDate;
        
        jsEventInfo["userid"] = itBegin->m_strUserID;
        jsEventInfo["deviceid"] = itBegin->m_strDevID;
        jsEventInfo["view_state"] = itBegin->m_strViewState;

        //if (!itBegin->m_strRemark.empty())
        {
            jsEventInfo["remark"] = jsRemark;
        }
        
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
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    if (!CreateGuardStorePlan(strSid, plan))
    {
        LOG_ERROR_RLD("Create guard store plan failed.");
        return blResult;
    }

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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete guard plan info received and session id is " << strSid << " and user id is " << strUserID
        << " and plan id  is " << strPlanID);

    if (!DeleteGuardStorePlan(strSid, strUserID, strPlanID))
    {
        LOG_ERROR_RLD("Delete plan failed.");
        return blResult;
    }

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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify guard store plan info received and session id is " << strSid << " and plan id is " << strPlanID << " and user id is " << strUserID
        << " and plan name is " << strPlanName);

    if (!ModifyGuardStorePlan(strSid, plan))
    {
        LOG_ERROR_RLD("Modify guard store plan failed.");
        return blResult;
    }

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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query plan info received and session id is " << strSid << " and user id is " << strUserID
        << " and plan id  is " << strPlanID);

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

    itFind = pMsgInfoMap->find("entrance");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Entrance not found.");
        return blResult;
    }
    const std::string strEntrance = itFind->second;

    std::list<StoreAndEntranceInfo> saelist;
    if (!GetEntrance(strEntrance, saelist))
    {
        LOG_ERROR_RLD("Entrance parse failed.");
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

    itFind = pMsgInfoMap->find("handler");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Patrol handler not found.");
        return blResult;
    }

    std::list<std::string> strPatrolHandlerList;
    std::string strPatrolHandler;
    strPatrolHandler = itFind->second;
    if (!GetValueList(strPatrolHandler, strPatrolHandlerList))
    {
        LOG_ERROR_RLD("Patrol handler list parse failed.");
        return blResult;
    }

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    Patrol pat;
    pat.m_strEnable = strEnable;
    pat.m_strPatrolName = strPatrolName;
    pat.m_strPatrolTimeList.swap(strPatrolTimeList);
    pat.m_strUserID = strUserID;
    pat.m_strPatrolHandlerList.swap(strPatrolHandlerList);
    pat.m_strExtend = strExtend;
    pat.m_SAEList.swap(saelist);

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    if (!CreateRegularPatrol(strSid, pat))
    {
        LOG_ERROR_RLD("Create regular patrol failed.");
        return blResult;
    }

    LOG_INFO_RLD("Create regular patrol info received and session id is " << strSid << " and patrol id is " << pat.m_strPatrolID << " and user id is " << strUserID
        << " and patrol name is " << strPatrolName << " and extend is " << strExtend);

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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete regular patrol info received and session id is " << strSid << " and user id is " << strUserID
        << " and plan id  is " << strPlanID);

    if (!DeleteRegularPatrol(strSid, strUserID, strPlanID))
    {
        LOG_ERROR_RLD("Delete regular patrol failed.");
        return blResult;
    }

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
    
    std::list<StoreAndEntranceInfo> saelist;
    std::string strEntrance;
    itFind = pMsgInfoMap->find("entrance");
    if (pMsgInfoMap->end() != itFind)
    {
        strEntrance = itFind->second;        
        if (!GetEntrance(strEntrance, saelist))
        {
            LOG_ERROR_RLD("Entrance parse failed.");
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

    std::list<std::string> strPatrolHandlerList;
    std::string strPatrolHandler;
    itFind = pMsgInfoMap->find("handler");
    if (pMsgInfoMap->end() != itFind)
    {
        strPatrolHandler = itFind->second;
        if (!GetValueList(strPatrolHandler, strPatrolHandlerList))
        {
            LOG_ERROR_RLD("Patrol handler list parse failed.");
            return blResult;
        }
    }

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }
    
    Patrol pat;
    pat.m_strEnable = strEnable;
    pat.m_strPatrolName = strPatrolName;
    pat.m_strPatrolTimeList.swap(strPatrolTimeList);
    pat.m_strUserID = strUserID;
    pat.m_strPatrolID = strPatrolID;
    pat.m_strPatrolHandlerList.swap(strPatrolHandlerList);
    pat.m_strExtend = strExtend;
    pat.m_SAEList.swap(saelist);

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify regular patrol info received and session id is " << strSid << " and patrol id is " << pat.m_strPatrolID << " and user id is " << strUserID
        << " and patrol name is " << strPatrolName << " and extend is " << strExtend);

    if (!ModifyRegularPatrol(strSid, pat))
    {
        LOG_ERROR_RLD("Modify regular patrol failed.");
        return blResult;
    }

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
    Json::Value jsPatrolHandlerList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsStoreList, &jsPatrolTimeList, &jsPatrolHandlerList)
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
                (*pJsBody)["handler"] = jsPatrolHandlerList;
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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query regulan patrol info received and session id is " << strSid << " and user id is " << strUserID
        << " and plan id  is " << strPlanID);

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

    for (auto itBegin = pat.m_SAEList.begin(), itEnd = pat.m_SAEList.end(); itBegin != itEnd; ++itBegin)
    {
        Json::Value jsStoreInfo;
        jsStoreInfo["store_id"] = itBegin->stinfo.m_strStoreID;
        jsStoreInfo["store_name"] = itBegin->stinfo.m_strStoreName;
        
        
        Json::Value jsEntranceInfoList;        
        for (auto itB1 = itBegin->etinfolist.begin(), itE1 = itBegin->etinfolist.end(); itB1 != itE1; ++itB1)
        {
            Json::Value jsEntranceInfo;
            jsEntranceInfo["entrance_id"] = itB1->m_strID;
            jsEntranceInfo["entrance_name"] = itB1->m_strName;
            jsEntranceInfoList.append(jsEntranceInfo);
        }

        jsStoreInfo["entrance"] = jsEntranceInfoList;

        jsStoreList.append(jsStoreInfo);
    }


    i = 0;
    for (auto itBegin = pat.m_strPatrolHandlerList.begin(), itEnd = pat.m_strPatrolHandlerList.end(); itBegin != itEnd; ++itBegin, ++i)
    {
        jsPatrolHandlerList[i] = *itBegin;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("plan_id", pat.m_strPatrolID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("plan_name", pat.m_strPatrolName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("enable", pat.m_strEnable));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("extend", pat.m_strExtend));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("create_date", pat.m_strCreateDate));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("modify_date", pat.m_strModifyDate));

    
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
        jsPlanInfo["create_date"] = itBegin->m_strCreateDate;
        jsPlanInfo["modify_date"] = itBegin->m_strModifyDate;
        jsPlanInfo["enable"] = itBegin->m_strEnable;
        jsPlanInfo["extend"] = itBegin->m_strExtend;
                
        Json::Value jsPatrolTimeList;
        unsigned int i = 0;
        for (auto itB = itBegin->m_strPatrolTimeList.begin(), itE = itBegin->m_strPatrolTimeList.end(); itB != itE; ++itB, ++i)
        {
            jsPatrolTimeList[i] = *itB;
        }

        jsPlanInfo["patrol_time"] = jsPatrolTimeList;

        Json::Value jsPatrolHandlerList;
        i = 0;
        for (auto itB = itBegin->m_strPatrolHandlerList.begin(), itE = itBegin->m_strPatrolHandlerList.end(); itB != itE; ++itB, ++i)
        {
            jsPatrolHandlerList[i] = *itB;
        }

        jsPlanInfo["handler"] = jsPatrolHandlerList;
                
        Json::Value jsStoreList;        
        for (auto itB = itBegin->m_SAEList.begin(), itE = itBegin->m_SAEList.end(); itB != itE; ++itB)
        {
            Json::Value jsStoreInfo;
            jsStoreInfo["store_id"] = itB->stinfo.m_strStoreID;
            jsStoreInfo["store_name"] = itB->stinfo.m_strStoreName;

            jsStoreList.append(jsStoreInfo);
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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    if (!CreateVIP(strSid, vip))
    {
        LOG_ERROR_RLD("Create vip failed.");
        return blResult;
    }

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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete vip info received and session id is " << strSid << " and user id is " << strUserID
        << " and vip id  is " << strVipID);

    if (!DeleteVIP(strSid, strUserID, strVipID))
    {
        LOG_ERROR_RLD("Delete vip failed.");
        return blResult;
    }
    
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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify vip info received and session id is " << strSid << " and vip id is " << strVipID << " and user id is " << strUserID
        << " and vip name is " << strVipName);

    if (!ModifyVIP(strSid, vip))
    {
        LOG_ERROR_RLD("Modify vip failed.");
        return blResult;
    }

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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query vip info received and session id is " << strSid << " and user id is " << strUserID
        << " and vip id  is " << strVipID);

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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Create vip consume history info received and session id is " << strSid << " and vip id is " << strVipID << " and user id is " << strUserID
        << " and goods name is " << strGoodsName);

    if (!CreateVIPConsumeHistory(strSid, strUserID, strVipID, vch))
    {
        LOG_ERROR_RLD("Create vip consume history failed.");
        return blResult;
    }

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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete vip consume info received and session id is " << strSid << " and user id is " << strUserID
        << " and consume id  is " << strConsumeID);

    if (!DeleteVIPConsumeHistory(strSid, strUserID, strConsumeID))
    {
        LOG_ERROR_RLD("Delete consume failed.");
        return blResult;
    }

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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify vip consume history info received and session id is " << strSid << " and consume id is " << strConsumeID << " and user id is " << strUserID
        << " and goods name is " << strGoodsName);

    if (!ModifyVIPConsumeHistory(strSid, strUserID, vch))
    {
        LOG_ERROR_RLD("Modify vip consume history failed.");
        return blResult;
    }

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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

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

    itFind = pMsgInfoMap->find("administratorid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Admin id not found.");
        return blResult;
    }
    const std::string strAdminID = itFind->second;
        
    UserOfStore us;
    us.m_strRole = strRole;
    us.m_strStoreID = strStoreID;
    us.m_strUserID = strUserID;
    us.m_strAdminID = strAdminID;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("User join store info received and session id is " << strSid << " and store id is " << strStoreID << " and user id is " << strUserID
        << " and role is " << strRole);

    if (!UserJoinStore(strSid, us))
    {
        LOG_ERROR_RLD("User join store failed.");
        return blResult;
    }

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
    us.m_strAdminID = strAdminID;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("User quit store info received and session id is " << strSid << " and store id is " << strStoreID << " and user id is " << strUserID
        << " and admin id is " << strAdminID);

    if (!UserQuitStore(strSid, strAdminID, us))
    {
        LOG_ERROR_RLD("User quit store failed.");
        return blResult;
    }

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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

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
        jsUserofStore["aliasname"] = itBegin->m_strAliasName;
        jsUserofStore["role"] = itBegin->m_strRole;

        jsUserOfStoreList[i] = jsUserofStore;
    }

    LOG_INFO_RLD("Query user of store info received and session id is " << strSid << " and user id is " << strUserID
        << " and store id  is " << strStoreID << " and user of store record size is " << uslist.size());

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryAllUserListHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsAllUserList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsAllUserList)
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
                (*pJsBody)["user_list"] = jsAllUserList;
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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    std::list<UserOfStore> uslist;
    if (!QueryAllUserList(strSid, strUserID, uslist))
    {
        LOG_ERROR_RLD("Query all user list failed.");
        return blResult;
    }

    unsigned int i = 0;
    for (auto itBegin = uslist.begin(), itEnd = uslist.end(); itBegin != itEnd; ++itBegin, ++i)
    {
        Json::Value jsUser;
        jsUser["user_id"] = itBegin->m_strUserID;
        jsUser["user_name"] = itBegin->m_strUserName;
        jsUser["aliasname"] = itBegin->m_strAliasName;
        jsUser["roleid"] = itBegin->m_strRole;

        jsAllUserList[i] = jsUser;
    }

    LOG_INFO_RLD("Query all user list info received and session id is " << strSid << " and user id is " << strUserID
        << " and user of list size is " << uslist.size());

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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Create evaluation template info received and session id is " << strSid << " and user id is " << strUserID << " and evaluation is " << strEvaluation
        << " and evaluation desc is " << strEvaluationDesc << " and evaluation value is " << dEvaluationValue);

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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete evaluation template info received and session id is " << strSid << " and user id is " << strUserID
        << " and evaluation id  is " << strEvaluationID);

    if (!DeleteEvaluationTemplate(strSid, strUserID, strEvaluationID))
    {
        LOG_ERROR_RLD("Delete evaluation template failed.");
        return blResult;
    }

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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify evaluation template info received and session id is " << strSid << " and user id is " << strUserID << " and evaluation is " << strEvaluation
        << " and evaluation desc is " << strEvaluationDesc << " and evaluation value is " << dEvaluationValue);

    if (!ModifyEvaluationTemplate(strSid, evt))
    {
        LOG_ERROR_RLD("Modify evaluation template failed.");
        return blResult;
    }

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
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

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
    
    std::list<std::string> strFileIDList;
    itFind = pMsgInfoMap->find("picture");
    if (pMsgInfoMap->end() != itFind)
    {
        std::string strPic = itFind->second;
        if (!GetValueList(strPic, strFileIDList))
        {
            LOG_ERROR_RLD("Pictcure info error");
            return blResult;
        }
    }

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
    ev.m_strFileIDList.swap(strFileIDList);

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

        std::list<std::string> strFileIDList;
        auto jsPic = jsValue["picture"];
        if (!jsPic.isNull() && !jsPic.isArray())
        {
            LOG_ERROR_RLD("Picture info parse failed");
            return blResult;
        }
        
        Json::StyledWriter stylewriter;
        const std::string &strBody = stylewriter.write(jsPic);

        if (!GetValueList(strBody, strFileIDList))
        {
            LOG_ERROR_RLD("Pictcure info error");
            return blResult;
        }
        
        EvaluationTemplate evt;
        evt.m_dEvaluationValue = dEvaluationValue;
        evt.m_strEvaluationTmpID = jsEvaluationID.asString();
        evt.m_strEvaluationDesc = jsEvaluationDesc.asString();
        evt.m_strFileIDList.swap(strFileIDList);

        ev.m_evlist.push_back(std::move(evt));
        
        return true;
    };

    if (!GetValueFromList(strEvaluationInfo, EvaluationParse))
    {
        LOG_ERROR_RLD("Evaluation parse failed and value is " << strEvaluationInfo);
        return blResult;
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    if (!CreateEvaluation(strSid, ev))
    {
        LOG_ERROR_RLD("Create evaluation failed.");
        return blResult;
    }

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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete evaluation info received and session id is " << strSid << " and user id is " << strUserID
        << " and evaluation id of store is " << strEvaluationIDOfStore);

    if (!DeleteEvaluation(strSid, strUserID, strEvaluationIDOfStore))
    {
        LOG_ERROR_RLD("Delete evaluation failed.");
        return blResult;
    }

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

    std::list<std::string> strFileIDList;
    itFind = pMsgInfoMap->find("picture");
    if (pMsgInfoMap->end() != itFind)
    {
        std::string strPic = itFind->second;
        if (!GetValueList(strPic, strFileIDList))
        {
            LOG_ERROR_RLD("Pictcure info error");
            return blResult;
        }
    }

    Evaluation ev;
    ev.m_strUserID = strUserID;
    ev.m_strUserIDOfCheck = strUserIDCheck;
    ev.m_strStoreID = strStoreID;
    ev.m_strEvaluationID = strEvaluationIDOfStore;
    ev.m_uiCheckStatus = uiCheckStatus;
    ev.m_strFileIDList.swap(strFileIDList);

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

            std::list<std::string> strFileIDList;
            auto jsPic = jsValue["picture"];
            if (jsPic.isNull() || !jsPic.isArray())
            {
                LOG_ERROR_RLD("Picture info parse failed");
                return blResult;
            }

            Json::StyledWriter stylewriter;
            const std::string &strBody = stylewriter.write(jsPic);

            if (!GetValueList(strBody, strFileIDList))
            {
                LOG_ERROR_RLD("Pictcure info error");
                return blResult;
            }

            EvaluationTemplate evt;
            evt.m_dEvaluationValue = dEvaluationValue;
            evt.m_strEvaluationTmpID = jsEvaluationID.asString();
            evt.m_strEvaluationDesc = jsEvaluationDesc.asString();
            evt.m_strFileIDList.swap(strFileIDList);

            ev.m_evlist.push_back(std::move(evt));

            return true;
        };

        if (!GetValueFromList(strEvaluationInfo, EvaluationParse))
        {
            LOG_ERROR_RLD("Evaluation parse failed and value is " << strEvaluationInfo);
            return blResult;
        }
        
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify evaluation info received and session id is " << strSid << " and user id is " << strUserID << " and store id is " << strStoreID
        << " and evalutaion id is " << strEvaluationIDOfStore << " and evaluation info is " << strEvaluationInfo);

    if (!ModifyEvaluation(strSid, ev))
    {
        LOG_ERROR_RLD("Modify evaluation failed.");
        return blResult;
    }
    
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
    Json::Value jsFileIDList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsEvaList, &jsFileIDList)
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
                (*pJsBody)["picture"] = jsFileIDList;
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

    ////去除查询门店考评记录详情中门店ID字段。
    //itFind = pMsgInfoMap->find("storeid");
    //if (pMsgInfoMap->end() == itFind)
    //{
    //    LOG_ERROR_RLD("Store id not found.");
    //    return blResult;
    //}
    //const std::string strStoreID = itFind->second;

    itFind = pMsgInfoMap->find("store_evaluation_id");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store evaluation id not found.");
        return blResult;
    }
    const std::string strEvaluationIDOfStore = itFind->second;
    
    Evaluation ev;
    //ev.m_strStoreID = strStoreID;
    ev.m_strEvaluationID = strEvaluationIDOfStore;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query evaluation info received and session id is " << strSid << " and user id is " << strUserID
        //<< " and store id is " << strStoreID 
        << " and evaluation id of store is " << strEvaluationIDOfStore);

    if (!QueryEvaluation(strSid, strUserID, ev))
    {
        LOG_ERROR_RLD("Query evaluation failed.");
        return blResult;
    }

    unsigned int k = 0;
    for (auto itBegin = ev.m_strFileIDList.begin(), itEnd = ev.m_strFileIDList.end(); itBegin != itEnd; ++itBegin, ++k)
    {
        jsFileIDList[k] = *itBegin;
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

        Json::Value jsFileURL;
        unsigned int kk = 0;
        for (const auto& strFileURL : itBegin->m_strFileIDList)
        {
            jsFileURL[kk++] = strFileURL;
        }
        jsEva["picture"] = jsFileURL;

        jsEvaList[i] = jsEva;
    }

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
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query all evaluation info of store received and session id is " << strSid << " and user id is " << strUserID
        << " and store id is " << strStoreID);

    std::list<Evaluation> evlist;
    if (!QueryAllEvaluationOfStore(strSid, strUserID, strStoreID, uiCheckStatus, strBeginDate, strEndDate, uiBeginIndex, evlist))
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
        jsEva["evaluation_template_total_value"] = boost::lexical_cast<std::string>(itBegin->m_dEvaluationTemplateTotalValue);

        jsEvaList[i] = jsEva;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::CreatePatrolRecordHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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
    if (pMsgInfoMap->end() != itFind)
    {
        strDevID = itFind->second;
    }

    if (strUserID.empty() && strDevID.empty())
    {
        LOG_ERROR_RLD("User id and device id all empty.");
        return blResult;
    }

    std::string strPlanID;
    if (!strDevID.empty())
    {
        itFind = pMsgInfoMap->find("planid");
        if (pMsgInfoMap->end() == itFind)
        {
            LOG_ERROR_RLD("Plan id not found.");
            return blResult;
        }

        strPlanID = itFind->second;
    }

    std::list<std::string> strEntranceIDList;
    if (!strUserID.empty())
    {
        itFind = pMsgInfoMap->find("entranceid");
        if (pMsgInfoMap->end() == itFind)
        {
            LOG_ERROR_RLD("Entrance id not found.");
            return blResult;
        }

        if (!GetValueList(itFind->second, strEntranceIDList))
        {
            LOG_ERROR_RLD("Entrance id list parse failed.");
            return blResult;
        }
    }

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;

    itFind = pMsgInfoMap->find("patrol_date");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Patrol date not found.");
        return blResult;
    }
    const std::string strPatrolDate = itFind->second;
    if (!ValidDatetime(strPatrolDate))
    {
        LOG_ERROR_RLD("Patrol date is invalid and value is " << strPatrolDate);
        return blResult;
    }

    itFind = pMsgInfoMap->find("patrol_picture");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Patrol picture info not found.");
        return blResult;
    }
    const std::string strPatrolPicInfo = itFind->second;

    std::list<PatrolRecord::PatrolPic> PicList;
    std::list<std::string> strPicIDList;
    if (!strDevID.empty())
    {
        if (!GetValueList(strPatrolPicInfo, strPicIDList))
        {
            LOG_ERROR_RLD("Patrol picture list parse failed and value is " << strPatrolPicInfo);
            return blResult;
        }
    }
    else
    {
        auto PicParse = [&](Json::Value jsValue) ->bool
        {
            if (!jsValue.isObject())
            {
                LOG_ERROR_RLD("Pic info parse failed");
                return blResult;
            }

            auto jsEid = jsValue["eid"];
            if (jsEid.isNull() || !jsEid.isString() || jsEid.asString().empty())
            {
                LOG_ERROR_RLD("Entrance id parse failed");
                return blResult;
            }

            PatrolRecord::PatrolPic pic;
            pic.m_strEntranceID = jsEid.asString();

            auto jsPicValue = jsValue["pic"];
            if (jsPicValue.isNull() || !jsPicValue.isArray())
            {
                LOG_ERROR_RLD("Pic parse failed");
                return blResult;
            }

            for (unsigned int i = 0; i < jsPicValue.size(); ++i)
            {
                if (jsPicValue[i].isNull() || !jsPicValue[i].isString() || jsPicValue[i].asString().empty())
                {
                    LOG_ERROR_RLD("Pic parse failed");
                    return blResult;
                }
                pic.m_strPicIDList.push_back(jsPicValue[i].asString());
            }

            PicList.push_back(std::move(pic));

            return true;
        };

        if (!GetValueFromList(strPatrolPicInfo, PicParse))
        {
            LOG_ERROR_RLD("Pic info parse failed and value is " << strPatrolPicInfo);
            return blResult;
        }
    }

    itFind = pMsgInfoMap->find("patrol_result");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Patrol result not found.");
        return blResult;
    }
    const std::string strPatrolResult = itFind->second;

    unsigned int uiPatrolResult = 0;
    if (!ValidType<unsigned int>(strPatrolResult, uiPatrolResult))
    {
        LOG_ERROR_RLD("Value is invalid and value is " << strPatrolResult);
        return blResult;
    }

    itFind = pMsgInfoMap->find("patrol_desc");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Patrol desc not found.");
        return blResult;
    }
    const std::string strPatrolDesc = itFind->second;

    PatrolRecord pr;
    pr.m_strDevID = strDevID;
    pr.m_strPatrolDate = strPatrolDate;
    pr.m_strPatrolDesc = strPatrolDesc;
    pr.m_strPatrolPicIDList.swap(strPicIDList);
    pr.m_strStoreID = strStoreID;
    pr.m_strUserID = strUserID;
    pr.m_uiPatrolResult = uiPatrolResult;
    pr.m_strPlanID = strPlanID;
    pr.m_strEntranceIDList.swap(strEntranceIDList);
    pr.m_PicList.swap(PicList);
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    if (!CreatePatrolRecord(strSid, pr))
    {
        LOG_ERROR_RLD("Create patrol record failed.");
        return blResult;
    }

    LOG_INFO_RLD("Create patrol record info received and session id is " << strSid << " and user id is " << strUserID << " and store id is " << strStoreID
        << " and ptrol record id is " << pr.m_strPatrolID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("patrol_id", pr.m_strPatrolID));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::DeletePatrolRecordHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("patrol_id");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Patrol id of store not found.");
        return blResult;
    }
    const std::string strPatrolID = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete patrol record info received and session id is " << strSid << " and user id is " << strUserID
        << " and patrol id of store is " << strPatrolID);

    if (!DeletePatrolRecord(strSid, strUserID, strPatrolID))
    {
        LOG_ERROR_RLD("Delete patrol record failed.");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::ModifyPatrolRecordHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("patrol_id");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strPatrolID = itFind->second;
        
    std::list<PatrolRecord::PatrolPic> PicList;
    itFind = pMsgInfoMap->find("patrol_picture");
    if (pMsgInfoMap->end() != itFind)
    {
        const std::string strPatrolPicInfo = itFind->second;
        auto PicParse = [&](Json::Value jsValue) ->bool
        {
            if (!jsValue.isObject())
            {
                LOG_ERROR_RLD("Pic info parse failed");
                return blResult;
            }

            auto jsEid = jsValue["eid"];
            if (jsEid.isNull() || !jsEid.isString() || jsEid.asString().empty())
            {
                LOG_ERROR_RLD("Entrance id parse failed");
                return blResult;
            }

            PatrolRecord::PatrolPic pic;
            pic.m_strEntranceID = jsEid.asString();

            auto jsPicValue = jsValue["pic"];
            if (jsPicValue.isNull() || !jsPicValue.isArray())
            {
                LOG_ERROR_RLD("Pic parse failed");
                return blResult;
            }

            for (unsigned int i = 0; i < jsPicValue.size(); ++i)
            {
                if (jsPicValue[i].isNull() || !jsPicValue[i].isString() || jsPicValue[i].asString().empty())
                {
                    LOG_ERROR_RLD("Pic parse failed");
                    return blResult;
                }
                pic.m_strPicIDList.push_back(jsPicValue[i].asString());
            }

            PicList.push_back(std::move(pic));

            return true;
        };

        if (!GetValueFromList(strPatrolPicInfo, PicParse))
        {
            LOG_ERROR_RLD("Pic info parse failed and value is " << strPatrolPicInfo);
            return blResult;
        }        
    }

    unsigned int uiPatrolResult = 0xFFFFFFFF;
    itFind = pMsgInfoMap->find("patrol_result");
    if (pMsgInfoMap->end() != itFind)
    {
        const std::string strPatrolResult = itFind->second;
        
        if (!ValidType<unsigned int>(strPatrolResult, uiPatrolResult))
        {
            LOG_ERROR_RLD("Value is invalid and value is " << strPatrolResult);
            return blResult;
        }
    }
       
    std::string strPatrolDesc;
    itFind = pMsgInfoMap->find("patrol_desc");
    if (pMsgInfoMap->end() != itFind)
    {
        strPatrolDesc = itFind->second;
    }
    

    PatrolRecord pr;
    pr.m_strPatrolID = strPatrolID;
    pr.m_strPatrolDesc = strPatrolDesc;
    pr.m_PicList.swap(PicList);
    pr.m_strUserID = strUserID;
    pr.m_uiPatrolResult = uiPatrolResult;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify patrol record info received and session id is " << strSid << " and user id is " << strUserID << " and patrol id is " << strPatrolID);

    if (!ModifyPatrolRecord(strSid, pr))
    {
        LOG_ERROR_RLD("Modify patrol record failed.");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryPatrolRecordHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsPatrolPicURLList;
    Json::Value jsEidList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsPatrolPicURLList, &jsEidList)
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
                (*pJsBody)["patrol_picture"] = jsPatrolPicURLList;
                (*pJsBody)["entranceid"] = jsEidList;
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

    itFind = pMsgInfoMap->find("patrol_id");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Patrol id not found.");
        return blResult;
    }
    const std::string strPatrolID = itFind->second;

    PatrolRecord pr;
    pr.m_strUserID = strUserID;
    pr.m_strPatrolID = strPatrolID;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query patrol record info received and session id is " << strSid << " and user id is " << strUserID
        << " and patrol id is " << strPatrolID);

    if (!QueryPatrolRecord(strSid, pr))
    {
        LOG_ERROR_RLD("Query patrol record failed.");
        return blResult;
    }

    for (auto itBegin = pr.m_PicList.begin(), itEnd = pr.m_PicList.end(); itBegin != itEnd; ++itBegin)
    {
        Json::Value jsPicInfo;
        jsPicInfo["eid"] = itBegin->m_strEntranceID;

        Json::Value jsURL;
        unsigned int i = 0;
        auto itB1 = itBegin->m_strPicIDList.begin();
        auto itE1 = itBegin->m_strPicIDList.end();
        while (itB1 != itE1)
        {
            jsURL[i] = *itB1;

            ++itB1;
            ++i;
        }

        jsPicInfo["pic"] = jsURL;

        jsPatrolPicURLList.append(jsPicInfo);
    }

    unsigned int i = 0;
    for (auto itBegin = pr.m_strEntranceIDList.begin(), itEnd = pr.m_strEntranceIDList.end(); itBegin != itEnd; ++itBegin, ++i)
    {        
        jsEidList[i] = *itBegin;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("patrol_result", boost::lexical_cast<std::string>(pr.m_uiPatrolResult)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("userid", pr.m_strUserID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("patrol_desc", pr.m_strPatrolDesc));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("deviceid", pr.m_strDevID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("storeid", pr.m_strStoreID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("patrol_date", pr.m_strPatrolDate));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("planid", pr.m_strPlanID));
    //ResultInfoMap.insert(std::map<std::string, std::string>::value_type("entranceid", pr.m_strEntranceID));
        
    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryAllPatrolRecordHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsPatrolRecordList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsPatrolRecordList)
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
                (*pJsBody)["patrol_record_info"] = jsPatrolRecordList;
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

    std::string strStoreID;
    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() != itFind)
    {
        strStoreID = itFind->second;
    }

    unsigned int uiPatrolResult = 0xFFFFFFFF;
    std::string strPatrolResult;
    itFind = pMsgInfoMap->find("patrol_result");
    if (pMsgInfoMap->end() != itFind)
    {
        strPatrolResult = itFind->second;
        if (!ValidType<unsigned int>(strPatrolResult, uiPatrolResult))
        {
            LOG_ERROR_RLD("Patrol result type is incorrect.");
            return blResult;
        }
    }

    unsigned int uiPlanFlag = 0xFFFFFFFF;
    std::string strPlanFlag;
    itFind = pMsgInfoMap->find("plan_flag");
    if (pMsgInfoMap->end() != itFind)
    {
        strPlanFlag = itFind->second;
        if (!ValidType<unsigned int>(strPlanFlag, uiPlanFlag))
        {
            LOG_ERROR_RLD("Plan flag type is incorrect.");
            return blResult;
        }
    }
        
    std::string strPlanID;
    itFind = pMsgInfoMap->find("planid");
    if (pMsgInfoMap->end() != itFind)
    {
        strPlanID = itFind->second;
    }
    
    ////
    //if (strStoreID.empty() && strPlanID.empty())
    //{
    //    LOG_ERROR_RLD("Store id and pland id all empty.");
    //    return blResult;
    //}
    
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

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query all patrol info of store received and session id is " << strSid << " and user id is " << strUserID
        << " and store id is " << strStoreID);

    std::list<PatrolRecord> prlist;
    if (!QueryAllPatrolRecord(strSid, strUserID, strStoreID, uiPatrolResult, uiPlanFlag, strPlanID, strBeginDate, strEndDate, uiBeginIndex, prlist))
    {
        LOG_ERROR_RLD("Query all evaluation of store failed.");
        return blResult;
    }

    unsigned int ii = 0;
    for (auto itBegin = prlist.begin(), itEnd = prlist.end(); itBegin != itEnd; ++itBegin, ++ii)
    {
        Json::Value jsPr;
        jsPr["userid"] = itBegin->m_strUserID;
        jsPr["patrol_result"] = boost::lexical_cast<std::string>(itBegin->m_uiPatrolResult);
        jsPr["patrol_desc"] = itBegin->m_strPatrolDesc;
        jsPr["deviceid"] = itBegin->m_strDevID;
        jsPr["storeid"] = itBegin->m_strStoreID;
        jsPr["patrol_date"] = itBegin->m_strPatrolDate;
        jsPr["planid"] = itBegin->m_strPlanID;
        //jsPr["entranceid"] = itBegin->m_strEntranceID;
        jsPr["patrol_id"] = itBegin->m_strPatrolID;
        
        Json::Value jsPatrolPicURLList;
        for (auto itBeginPic = itBegin->m_PicList.begin(), itEndPic = itBegin->m_PicList.end(); itBeginPic != itEndPic; ++itBeginPic)
        {
            Json::Value jsPicInfo;
            jsPicInfo["eid"] = itBeginPic->m_strEntranceID;

            Json::Value jsURL;
            unsigned int i = 0;
            auto itB1 = itBeginPic->m_strPicIDList.begin();
            auto itE1 = itBeginPic->m_strPicIDList.end();
            while (itB1 != itE1)
            {
                jsURL[i] = *itB1;

                ++itB1;
                ++i;
            }

            jsPicInfo["pic"] = jsURL;

            jsPatrolPicURLList.append(jsPicInfo);
        }

        jsPr["patrol_picture"] = jsPatrolPicURLList;

        Json::Value jsEid;
        unsigned int i = 0;
        for (auto itBeginEid = itBegin->m_strEntranceIDList.begin(), itEndEid = itBegin->m_strEntranceIDList.end(); itBeginEid != itEndEid; ++itBeginEid, ++i)
        {            
            jsEid[i] = *itBeginEid;
        }

        jsPr["entranceid"] = jsEid;

        jsPatrolRecordList[ii] = jsPr;
    }

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

bool PassengerFlowMsgHandler::CreateStoreSensorHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("deviceid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store id not found.");
        return blResult;
    }
    const std::string strStoreID = itFind->second;

    itFind = pMsgInfoMap->find("name");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Name not found.");
        return blResult;
    }
    const std::string strName = itFind->second;

    itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Type not found.");
        return blResult;
    }
    const std::string strType = itFind->second;

    unsigned int uiType = 0;
    if (!ValidType<unsigned int>(strType, uiType))
    {
        LOG_ERROR_RLD("Value is invalid and value is " << strType);
        return blResult;
    }

    std::string strAlarmThreshold;
    itFind = pMsgInfoMap->find("alarm_threshold");
    if (pMsgInfoMap->end() != itFind)
    {
        strAlarmThreshold = itFind->second;

    }

    itFind = pMsgInfoMap->find("sensor_key");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sensor key not found.");
        return blResult;
    }
    const std::string strSensorKey = itFind->second;

    std::string strLocation;
    itFind = pMsgInfoMap->find("location");
    if (pMsgInfoMap->end() != itFind)
    {
        strLocation = itFind->second;
    }
    
    Sensor sr;
    sr.m_strDevID = strDevID;
    sr.m_strName = strName;
    sr.m_strStoreID = strStoreID;
    sr.m_uiType = uiType;
    sr.m_strAlarmThreshold = strAlarmThreshold;
    sr.m_strSensorKey = strSensorKey;
    sr.m_strLocation = strLocation;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    if (!CreateStoreSensor(strSid, strUserID, sr))
    {
        LOG_ERROR_RLD("Create store sensor failed.");
        return blResult;
    }

    LOG_INFO_RLD("Create store sensor info received and session id is " << strSid << " and user id is " << strUserID << " and store id is " << strStoreID
        << " and sensor id is " << sr.m_strID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("sensorid", sr.m_strID));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::DeleteStoreSensorHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("sensorid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sensor id not found.");
        return blResult;
    }
    const std::string strSensorID = itFind->second;
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete store sensor info received and session id is " << strSid << " and user id is " << strUserID
        << " and sensor id of store is " << strSensorID);

    if (!DeleteStoreSensor(strSid, strUserID, strStoreID, strSensorID))
    {
        LOG_ERROR_RLD("Delete store sensor failed.");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::ModifyStoreSensorHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("sensorid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sensor id not found.");
        return blResult;
    }
    const std::string strSensorID = itFind->second;
    
    std::string strName;
    itFind = pMsgInfoMap->find("name");
    if (pMsgInfoMap->end() != itFind)
    {
        strName = itFind->second;
    }

    unsigned int uiType = 0xFFFFFFFF;
    std::string strType;
    itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() != itFind)
    {
        strType = itFind->second;
        if (!ValidType<unsigned int>(strType, uiType))
        {
            LOG_ERROR_RLD("Value is invalid and value is " << strType);
            return blResult;
        }
    }
    
    std::string strStoreID;
    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() != itFind)
    {
        strStoreID = itFind->second;
    }

    std::string strDevID;
    itFind = pMsgInfoMap->find("deviceid");
    if (pMsgInfoMap->end() != itFind)
    {
        strDevID = itFind->second;
    }
    
    std::string strAlarmThreshold;
    itFind = pMsgInfoMap->find("alarm_threshold");
    if (pMsgInfoMap->end() != itFind)
    {
        strAlarmThreshold = itFind->second;

    }

    std::string strSensorKey;
    itFind = pMsgInfoMap->find("sensor_key");
    if (pMsgInfoMap->end() != itFind)
    {
        strSensorKey = itFind->second;
    }

    std::string strLocation;
    itFind = pMsgInfoMap->find("location");
    if (pMsgInfoMap->end() != itFind)
    {
        strLocation = itFind->second;
    }

    Sensor sr;
    sr.m_strDevID = strDevID;
    sr.m_strName = strName;
    sr.m_strStoreID = strStoreID;
    sr.m_uiType = uiType;
    sr.m_strID = strSensorID;
    sr.m_strAlarmThreshold = strAlarmThreshold;
    sr.m_strSensorKey = strSensorKey;
    sr.m_strLocation = strLocation;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify store sensor info received and session id is " << strSid << " and user id is " << strUserID << " and store id is " << strStoreID
        << " and sensor id is " << sr.m_strID);

    if (!ModifyStoreSensor(strSid, strUserID, sr))
    {
        LOG_ERROR_RLD("Modify store sensor failed.");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryStoreSensorHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("sensorid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sensor id not found.");
        return blResult;
    }
    const std::string strSensorID = itFind->second;

    Sensor sr;
    sr.m_strID = strSensorID;
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query sensor info received and session id is " << strSid << " and user id is " << strUserID
        << " and sensor id is " << strSensorID);

    if (!QueryStoreSensor(strSid, strUserID, sr))
    {
        LOG_ERROR_RLD("Query sensor info failed.");
        return blResult;
    }
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("type", boost::lexical_cast<std::string>(sr.m_uiType)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("value", sr.m_strValue));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("name", sr.m_strName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("storeid", sr.m_strStoreID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("deviceid", sr.m_strDevID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("alarm_threshold", sr.m_strAlarmThreshold));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("sensor_key", sr.m_strSensorKey));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("location", sr.m_strLocation));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryAllStoreSensorHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsSensorInfoList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsSensorInfoList)
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
                (*pJsBody)["store_sensor"] = jsSensorInfoList;
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

    LOG_INFO_RLD("Query all sensor info of store received and session id is " << strSid << " and user id is " << strUserID
        << " and store id is " << strStoreID);

    std::list<Sensor> srlist;
    if (!QueryAllStoreSensor(strSid, strUserID, strStoreID, srlist))
    {
        LOG_ERROR_RLD("Query all sensor info of store failed.");
        return blResult;
    }

    unsigned int i = 0;
    for (auto itBegin = srlist.begin(), itEnd = srlist.end(); itBegin != itEnd; ++itBegin, ++i)
    {
        Json::Value jsSensor;
        jsSensor["deviceid"] = itBegin->m_strDevID;
        jsSensor["storeid"] = itBegin->m_strStoreID;
        jsSensor["name"] = itBegin->m_strName;
        jsSensor["type"] = boost::lexical_cast<std::string>(itBegin->m_uiType);
        jsSensor["value"] = itBegin->m_strValue;
        jsSensor["sensorid"] = itBegin->m_strID;
        jsSensor["alarm_threshold"] = itBegin->m_strAlarmThreshold;
        jsSensor["sensor_key"] = itBegin->m_strSensorKey;
        jsSensor["location"] = itBegin->m_strLocation;

        jsSensorInfoList[i] = jsSensor;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::ReportSensorInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("deviceid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    itFind = pMsgInfoMap->find("info");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Info not found.");
        return blResult;
    }
    const std::string strSensorInfo = itFind->second;
    
    std::list<Sensor> srlist;
    auto SensorInfoParse = [&](Json::Value jsValue) ->bool
    {
        if (!jsValue.isObject())
        {
            LOG_ERROR_RLD("Sensor info parse failed");
            return blResult;
        }

        auto jsSensorType = jsValue["type"];
        if (jsSensorType.isNull() || !jsSensorType.isString() || jsSensorType.asString().empty())
        {
            LOG_ERROR_RLD("Sensor type parse failed");
            return blResult;
        }

        unsigned int uiSensorType = 0;
        if (!ValidType<unsigned int>(jsSensorType.asString(), uiSensorType))
        {
            LOG_ERROR_RLD("Sensor type is invalid and value is " << jsSensorType.asString());
            return blResult;
        }

        auto jsSensorValue = jsValue["value"];
        if (jsSensorValue.isNull() || !jsSensorValue.isString() || jsSensorValue.asString().empty())
        {
            LOG_ERROR_RLD("Sensor value parse failed");
            return blResult;
        }

        std::string strSensorValue = jsSensorValue.asString();
        std::list<std::string> SensorValueList;
        boost::split(SensorValueList, strSensorValue, boost::is_any_of("|"));
        if (SensorValueList.empty())
        {
            SensorValueList.push_back(strSensorValue);
        }

        for (const auto &svalue : SensorValueList)
        {
            auto ipos = svalue.find(':');
            if (std::string::npos == ipos)
            {
                LOG_ERROR_RLD("Sensor value info is invalid");
                return blResult;
            }
            const auto &strSensorKey = svalue.substr(0, ipos);
            const auto &strSensorValue = svalue.substr(ipos + 1);

            Sensor sr;
            sr.m_uiType = uiSensorType;
            sr.m_strValue = strSensorValue;
            sr.m_strSensorKey = strSensorKey;

            LOG_INFO_RLD("Report sensor type is " << sr.m_uiType << " and value is " << sr.m_strValue << " and sensor key is " << sr.m_strSensorKey);

            srlist.push_back(std::move(sr));
        }

        return true;
    };

    if (!GetValueFromList(strSensorInfo, SensorInfoParse))
    {
        LOG_ERROR_RLD("Sensor info parse failed and value is " << strSensorInfo);
        return blResult;
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Report sensor info received and session id is " << strSid << " and device id is " << strDevID);

    if (!ReportSensorInfo(strSid, strDevID, srlist))
    {
        LOG_ERROR_RLD("Report sensor info failed.");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryPatrolResultReportHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("begindate");
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

    itFind = pMsgInfoMap->find("enddate");
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

    std::string strStoreID;
    itFind = pMsgInfoMap->find("storeid");
    if (pMsgInfoMap->end() != itFind)
    {
        strStoreID = itFind->second;
    }

    unsigned int uiPatrolResult = 0xFFFFFFFF;
    std::string strPatrolResult;
    itFind = pMsgInfoMap->find("patrol_result");
    if (pMsgInfoMap->end() != itFind)
    {
        strPatrolResult = itFind->second;

        if (!ValidType<unsigned int>(strPatrolResult, uiPatrolResult))
        {
            LOG_ERROR_RLD("Patrol result value is error and value is " << strPatrolResult);
            return blResult;
        }
    }

    std::string strPatrolUserID;
    itFind = pMsgInfoMap->find("patrol_userid");
    if (pMsgInfoMap->end() != itFind)
    {
        strPatrolUserID = itFind->second;
    }

    PatrolResultReportQueryParam prqm;
    prqm.m_strBeginDate = strBeginDate;
    prqm.m_strEndDate = strEndDate;
    prqm.m_strPatrolUserID = strPatrolUserID;
    prqm.m_strStoreID = strStoreID;
    prqm.m_uiPatrolResult = uiPatrolResult;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query patrol result report info received and session id is " << strSid << " and user id is " << strUserID << " and begin date is " << strBeginDate
        << " and end date is " << strEndDate << " and store id is " << strStoreID << " and patrol result is " << uiPatrolResult << " and patrol user id is " << strPatrolUserID);

    std::string strReport;
    if (!QueryPatrolResult(strSid, strUserID, prqm, strReport))
    {
        LOG_ERROR_RLD("Query patrol result report info failed.");
        return blResult;
    }

    Json::Reader reader;
    if (!reader.parse(strReport, jsChartData, false))
    {
        LOG_ERROR_RLD("Parsed patrol result report failed and value is " << strReport);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("chart_data", strReport));


    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::ReportSensorAlarmInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("deviceid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    itFind = pMsgInfoMap->find("sensor_key");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sensor key not found.");
        return blResult;
    }
    const std::string strSensorKey = itFind->second;

    itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Type not found.");
        return blResult;
    }
    const std::string strType = itFind->second;
    unsigned int uiType = 0;

    if (!ValidType<unsigned int>(strType, uiType))
    {
        LOG_ERROR_RLD("Type is invalid and value is " << strType);
        return blResult;
    }
    
    itFind = pMsgInfoMap->find("current_value");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Current value not found.");
        return blResult;
    }
    const std::string strCurrentValue = itFind->second;

    itFind = pMsgInfoMap->find("threshold_value");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Threshold value not found.");
        return blResult;
    }
    const std::string strThresholdValue = itFind->second;

    itFind = pMsgInfoMap->find("recover");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Recover not found.");
        return blResult;
    }
    const std::string strRecover = itFind->second;
    unsigned int uiRecover = 0;

    if (!ValidType<unsigned int>(strRecover, uiRecover))
    {
        LOG_ERROR_RLD("Recover is invalid and value is " << strRecover);
        return blResult;
    }

    std::string strFileID;
    itFind = pMsgInfoMap->find("fileid");
    if (pMsgInfoMap->end() != itFind)
    {
        strFileID = itFind->second;
    }
    
    Sensor sr;
    sr.m_strDevID = strDevID;
    //sr.m_strName = strName;
    //sr.m_strStoreID = strStoreID;
    sr.m_uiType = uiType;
    //sr.m_strID = strSensorID;
    sr.m_strAlarmThreshold = strThresholdValue;
    sr.m_strValue = strCurrentValue;
    sr.m_strSensorKey = strSensorKey;
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Report sensor alarm info received and session id is " << strSid << " and device id is " << strDevID
        << " and type is " << uiType << " and recover flag is " << uiRecover << " and file id is " << strFileID);

    if (!ReportSensorAlarmInfo(strSid, sr, uiRecover, strFileID))
    {
        LOG_ERROR_RLD("Report sensor alarm info failed.");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QuerySensorAlarmThresholdHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsAlarmThresholdList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsAlarmThresholdList)
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
                (*pJsBody)["alarm_threshold"] = jsAlarmThresholdList;                
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

    itFind = pMsgInfoMap->find("deviceid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query sensor alarm threshold info received and session id is " << strSid << " and device id is " << strDevID);

    std::list<Sensor> srlist;
    if (!QuerySensorAlarmThreshold(strSid, strDevID, srlist))
    {
        LOG_ERROR_RLD("Query sensor alarm failed and device id is " << strDevID);
        return blResult;
    }

    unsigned int i = 0;
    for (auto itBegin = srlist.begin(), itEnd = srlist.end(); itBegin != itEnd; ++itBegin, ++i)
    {
        Json::Value jsAlarmThreshold;
        jsAlarmThreshold["type"] = boost::lexical_cast<std::string>(itBegin->m_uiType);
        jsAlarmThreshold["threshold_value"] = itBegin->m_strAlarmThreshold;
        jsAlarmThreshold["sensor_key"] = itBegin->m_strSensorKey;
        jsAlarmThreshold["sensorid"] = itBegin->m_strID;
        
        jsAlarmThresholdList[i] = jsAlarmThreshold;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::RemoveSensorRecordsHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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
        LOG_ERROR_RLD("Sid not found.");
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

    itFind = pMsgInfoMap->find("recordid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Record id not found.");
        return blResult;
    }
    const std::string strRecordID = itFind->second;

    std::list<std::string> strRecordIDList;
    if (GetValueList(strRecordID, strRecordIDList))
    {
        LOG_INFO_RLD("Multiple record id is received.");
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Remove sensor records info received and sid is " << strSid << " and record id is " << strRecordID);

    if (strRecordIDList.empty())
    {
        strRecordIDList.push_back(strRecordID);
    }

    if (!RemoveSensorRecords(strSid, strUserID, strRecordIDList))
    {
        LOG_ERROR_RLD("Remove sensor records handle failed and user id is " << strUserID);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::RemoveSensorAlarmRecordsHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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
        LOG_ERROR_RLD("Sid not found.");
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

    itFind = pMsgInfoMap->find("recordid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Record id not found.");
        return blResult;
    }
    const std::string strRecordID = itFind->second;

    std::list<std::string> strRecordIDList;
    if (GetValueList(strRecordID, strRecordIDList))
    {
        LOG_INFO_RLD("Multiple record id is received.");
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Remove sensor alarm records info received and sid is " << strSid << " and record id is " << strRecordID);

    if (strRecordIDList.empty())
    {
        strRecordIDList.push_back(strRecordID);
    }

    if (!RemoveSensorAlarmRecords(strSid, strUserID, strRecordIDList))
    {
        LOG_ERROR_RLD("Remove sensor alarm records handle failed and user id is " << strUserID);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QuerySensorRecordsHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsSensorRecordsList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsSensorRecordsList)
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
                (*pJsBody)["store_sensor"] = jsSensorRecordsList;

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
    
    std::string strSensorID;
    itFind = pMsgInfoMap->find("sensorid");
    if (pMsgInfoMap->end() != itFind)
    {
        strSensorID = itFind->second;
    }
    
    unsigned int uiType = 0xFFFFFFFF;
    std::string strType;
    itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() != itFind)
    {
        strType = itFind->second;
        if (!ValidType<unsigned int>(strType, uiType))
        {
            LOG_ERROR_RLD("Type is invalid and value is " << strType);
            return blResult;
        }
    }
    
    std::string strBeginDate;
    itFind = pMsgInfoMap->find("begindate");
    if (pMsgInfoMap->end() != itFind)
    {
        strBeginDate = itFind->second;
        if (!ValidDatetime(strBeginDate))
        {
            LOG_ERROR_RLD("Begin date is invalid and value is " << strBeginDate);
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
            LOG_ERROR_RLD("End date is invalid and value is " << strEndDate);
            return blResult;
        }
    }

    std::string strBeginIndex;
    unsigned int uiBeginIndex = 0xFFFFFFFF;
    itFind = pMsgInfoMap->find("beginindex");
    if (pMsgInfoMap->end() != itFind)
    {
        strBeginIndex = itFind->second;
        if (!ValidType<unsigned int>(strBeginIndex, uiBeginIndex))
        {
            LOG_ERROR_RLD("Begin index is invalid and value is " << strBeginIndex);
            return blResult;
        }
    }

	std::string strRangeType;
	unsigned int uiRangeType = 0xFFFFFFFF;
	itFind = pMsgInfoMap->find("range_type");
	if (pMsgInfoMap->end() != itFind)
	{
		strRangeType = itFind->second;
		if (!ValidType<unsigned int>(strRangeType, uiRangeType))
		{
			LOG_ERROR_RLD("Range type is invalid and value is " << strRangeType);
			return blResult;
		}
	}

	std::string strRangeBase;
	unsigned int uiRangeBase = 0xFFFFFFFF;
	itFind = pMsgInfoMap->find("range_base");
	if (pMsgInfoMap->end() != itFind)
	{
		strRangeBase = itFind->second;
		if (!ValidType<unsigned int>(strRangeBase, uiRangeBase))
		{
			LOG_ERROR_RLD("Range base is invalid and value is " << strRangeBase);
			return blResult;
		}

		if (64 < uiRangeBase)
		{
			LOG_ERROR_RLD("Range base value is too large " << uiRangeBase);
			return blResult;
		}
	}
		    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query sensor records info received and session id is " << strSid << " and user id is " << strUserID
        << " and store id is " << strStoreID << " and type is " << strType << " and begin date is " << strBeginDate
        << " and end date is " << strEndDate << " and begin index is " << strBeginIndex 
		<< " and range type is " << strRangeType  << " and range base is " << strRangeBase);

    QuerySensorParam qsr;
    qsr.m_strBeginDate = strBeginDate;
    qsr.m_strEndDate = strEndDate;
    qsr.m_strSensorID = strSensorID;
    qsr.m_strStoreID = strStoreID;
    qsr.m_strType = strType;
    qsr.m_strUserID = strUserID;
    qsr.m_uiBeginIndex = uiBeginIndex;
	qsr.m_uiRangeType = uiRangeType;
	qsr.m_uiRangeBase = uiRangeBase;

	unsigned int uiRealRecordNum = 0;
    std::list<SensorRecord> srdlist;
    if (!QuerySensorRecords(strSid, qsr, srdlist, uiRealRecordNum))
    {
        LOG_ERROR_RLD("Query sensor records failed.");
        return blResult;
    }

    auto itBegin = srdlist.begin();
    auto itEnd = srdlist.end();
    while (itBegin != itEnd)
    {
        Json::Value jsSensorRecordInfo;
        jsSensorRecordInfo["recordid"] = itBegin->m_strRecordID;
        jsSensorRecordInfo["deviceid"] = itBegin->sr.m_strDevID;
        jsSensorRecordInfo["storeid"] = itBegin->sr.m_strStoreID;
        jsSensorRecordInfo["name"] = itBegin->sr.m_strName;
        jsSensorRecordInfo["type"] = boost::lexical_cast<std::string>(itBegin->sr.m_uiType);
        jsSensorRecordInfo["value"] = itBegin->sr.m_strValue;
        jsSensorRecordInfo["sensorid"] = itBegin->sr.m_strID;
        jsSensorRecordInfo["alarm_threshold"] = itBegin->sr.m_strAlarmThreshold;
        jsSensorRecordInfo["create_date"] = itBegin->m_strCreateDate;
        jsSensorRecordInfo["sensor_key"] = itBegin->sr.m_strSensorKey;
        jsSensorRecordInfo["location"] = itBegin->sr.m_strLocation;
        
        jsSensorRecordsList.append(jsSensorRecordInfo);

        ++itBegin;
    }

	if (!strRangeType.empty()) //启用了范围过滤查询
	{
		ResultInfoMap.insert(std::map<std::string, std::string>::value_type("real_record_num", boost::lexical_cast<std::string>(uiRealRecordNum)));
	}
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QuerySensorAlarmRecordsHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsSensorAlarmRecordsList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsSensorAlarmRecordsList)
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
                (*pJsBody)["store_alarm"] = jsSensorAlarmRecordsList;

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

    std::string strSensorID;
    itFind = pMsgInfoMap->find("sensorid");
    if (pMsgInfoMap->end() != itFind)
    {
        strSensorID = itFind->second;
    }

    unsigned int uiType = 0xFFFFFFFF;
    std::string strType;
    itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() != itFind)
    {
        strType = itFind->second;
        if (!ValidType<unsigned int>(strType, uiType))
        {
            LOG_ERROR_RLD("Type is invalid and value is " << strType);
            return blResult;
        }
    }

    unsigned int uiRecover = 0xFFFFFFFF;
    std::string strRecover;
    itFind = pMsgInfoMap->find("recover");
    if (pMsgInfoMap->end() != itFind)
    {
        strRecover = itFind->second;
        if (!ValidType<unsigned int>(strRecover, uiRecover))
        {
            LOG_ERROR_RLD("Recover is invalid and value is " << strType);
            return blResult;
        }
    }
    
    std::string strBeginDate;
    itFind = pMsgInfoMap->find("begindate");
    if (pMsgInfoMap->end() != itFind)
    {
        strBeginDate = itFind->second;
        if (!ValidDatetime(strBeginDate))
        {
            LOG_ERROR_RLD("Begin date is invalid and value is " << strBeginDate);
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
            LOG_ERROR_RLD("End date is invalid and value is " << strEndDate);
            return blResult;
        }
    }

    std::string strBeginIndex;
    unsigned int uiBeginIndex = 0xFFFFFFFF;
    itFind = pMsgInfoMap->find("beginindex");
    if (pMsgInfoMap->end() != itFind)
    {
        strBeginIndex = itFind->second;
        if (!ValidType<unsigned int>(strBeginIndex, uiBeginIndex))
        {
            LOG_ERROR_RLD("Begin index is invalid and value is " << strBeginIndex);
            return blResult;
        }
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query sensor alarm records info received and session id is " << strSid << " and user id is " << strUserID
        << " and store id is " << strStoreID << " and type is " << strType << " and recover is " << strRecover << " and begin date is " << strBeginDate
        << " and end date is " << strEndDate << " and begin index is " << strBeginIndex);

    QuerySensorParam qsr;
    qsr.m_strBeginDate = strBeginDate;
    qsr.m_strEndDate = strEndDate;
    qsr.m_strSensorID = strSensorID;
    qsr.m_strStoreID = strStoreID;
    qsr.m_strType = strType;
    qsr.m_uiRecover = uiRecover;
    qsr.m_strUserID = strUserID;
    qsr.m_uiBeginIndex = uiBeginIndex;

    std::list<SensorAlarmRecord> srdlist;
    if (!QuerySensorAlarmRecords(strSid, qsr, srdlist))
    {
        LOG_ERROR_RLD("Query sensor alarm records failed.");
        return blResult;
    }

    auto itBegin = srdlist.begin();
    auto itEnd = srdlist.end();
    while (itBegin != itEnd)
    {
        Json::Value jsSensorAlarmRecordInfo;
        jsSensorAlarmRecordInfo["recordid"] = itBegin->m_strRecordID;
        jsSensorAlarmRecordInfo["deviceid"] = itBegin->m_sr.m_strDevID;
        jsSensorAlarmRecordInfo["storeid"] = itBegin->m_sr.m_strStoreID;
        jsSensorAlarmRecordInfo["name"] = itBegin->m_sr.m_strName;
        jsSensorAlarmRecordInfo["type"] = boost::lexical_cast<std::string>(itBegin->m_sr.m_uiType);
        jsSensorAlarmRecordInfo["value"] = itBegin->m_sr.m_strValue;
        jsSensorAlarmRecordInfo["sensorid"] = itBegin->m_sr.m_strID;
        jsSensorAlarmRecordInfo["alarm_threshold"] = itBegin->m_sr.m_strAlarmThreshold;
        jsSensorAlarmRecordInfo["create_date"] = itBegin->m_strCreateDate;
        jsSensorAlarmRecordInfo["recover"] = boost::lexical_cast<std::string>(itBegin->m_uiRecover);
        jsSensorAlarmRecordInfo["fileid"] = itBegin->m_strFileID;
        jsSensorAlarmRecordInfo["sensor_key"] = itBegin->m_sr.m_strSensorKey;
        jsSensorAlarmRecordInfo["location"] = itBegin->m_sr.m_strLocation;
        
        jsSensorAlarmRecordsList.append(jsSensorAlarmRecordInfo);

        ++itBegin;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::AddRoleHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("roleid_new");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Role id of new not found.");
        return blResult;
    }
    const std::string strRoleIDNew = itFind->second;

    itFind = pMsgInfoMap->find("roleid_old");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Role id of old not found.");
        return blResult;
    }
    const std::string strRoleIDOld = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    if (!AddRole(strSid, strUserID, strRoleIDNew, strRoleIDOld))
    {
        LOG_ERROR_RLD("Add role failed.");
        return blResult;
    }

    LOG_INFO_RLD("Add role info received and session id is " << strSid << " and user id is " << strUserID << " and new role id is " << strRoleIDNew
        << " and old role id is " << strRoleIDOld);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::RemoveRoleHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("roleid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Role id not found.");
        return blResult;
    }
    const std::string strRoleID = itFind->second;
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    if (!RemoveRole(strSid, strUserID, strRoleID))
    {
        LOG_ERROR_RLD("Remove role failed.");
        return blResult;
    }

    LOG_INFO_RLD("Remove role info received and session id is " << strSid << " and user id is " << strUserID << " and role id is " << strRoleID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::ModifyRoleHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("roleid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Role id not found.");
        return blResult;
    }
    const std::string strRoleID = itFind->second;

    bool blFlag = false;
    std::list<unsigned int> uiAllowFuncIDList;
    std::list<std::string> strAllowFuncIDList;
    itFind = pMsgInfoMap->find("allow_func");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!GetValueList(itFind->second, strAllowFuncIDList))
        {
            LOG_ERROR_RLD("Allow func info is error.");
            return blResult;
        }

        for (auto itBegin = strAllowFuncIDList.begin(), itEnd = strAllowFuncIDList.end(); itBegin != itEnd; ++itBegin)
        {
            unsigned int uiValue = 0;
            if (!ValidType<unsigned int>(*itBegin, uiValue))
            {
                LOG_ERROR_RLD("Allow func info is invalid.");
                return blResult;
            }

            uiAllowFuncIDList.push_back(uiValue);
        }

        blFlag = true;
    }
    
    std::list<unsigned int> uiDisallowFuncIDList;
    std::list<std::string> strDisallowFuncIDList;
    itFind = pMsgInfoMap->find("disallow_func");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!GetValueList(itFind->second, strDisallowFuncIDList))
        {
            LOG_ERROR_RLD("Disallow func info is error.");
            return blResult;
        }

        for (auto itBegin = strDisallowFuncIDList.begin(), itEnd = strDisallowFuncIDList.end(); itBegin != itEnd; ++itBegin)
        {
            unsigned int uiValue = 0;
            if (!ValidType<unsigned int>(*itBegin, uiValue))
            {
                LOG_ERROR_RLD("Disallow func info is invalid.");
                return blResult;
            }

            uiDisallowFuncIDList.push_back(uiValue);
        }

        blFlag = true;
    }


    if (!blFlag)
    {
        LOG_ERROR_RLD("Allow and disallow info not found and user id is " << strUserID << " role id is " << strRoleID);
        return blResult;
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    if (!ModifyRole(strSid, strUserID, strRoleID, uiAllowFuncIDList, uiDisallowFuncIDList))
    {
        LOG_ERROR_RLD("Modify role failed.");
        return blResult;
    }

    LOG_INFO_RLD("Modify role info received and session id is " << strSid << " and user id is " << strUserID << " and role id is " << strRoleID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryRoleHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsPermissionInfo;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsPermissionInfo)
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
                (*pJsBody)["permission_info"] = jsPermissionInfo;

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

    itFind = pMsgInfoMap->find("roleid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Role id not found.");
        return blResult;
    }
    const std::string strRoleID = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    PassengerFlowProtoHandler::Role role;
    if (!QueryRole<PassengerFlowProtoHandler::Role>(strSid, strUserID, strRoleID, role))
    {
        LOG_ERROR_RLD("Query role failed.");
        return blResult;
    }

    for (auto itBegin = role.m_pmlist.begin(), itEnd = role.m_pmlist.end(); itBegin != itEnd; ++itBegin)
    {
        Json::Value jsPm;
        jsPm["func_id"] = boost::lexical_cast<std::string>(itBegin->m_uiFuncID);
        jsPm["access"] = itBegin->m_strAccess;

        jsPermissionInfo.append(jsPm);
    }

    LOG_INFO_RLD("Query role info received and session id is " << strSid << " and user id is " << strUserID << " and role id is " << strRoleID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("roleid", role.m_strRoleID));
    
    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryAllRoleHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsAllRoleInfo;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsAllRoleInfo)
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
                (*pJsBody)["all_role_info"] = jsAllRoleInfo;

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
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    std::list<PassengerFlowProtoHandler::Role> rolelist;
    if (!QueryAllRole<PassengerFlowProtoHandler::Role>(strSid, strUserID, rolelist))
    {
        LOG_ERROR_RLD("Query all role failed.");
        return blResult;
    }

    for (auto itB = rolelist.begin(), itE = rolelist.end(); itB != itE; ++itB)
    {
        Json::Value jsRoleInfo;
        jsRoleInfo["role_id"] = itB->m_strRoleID;

        Json::Value jsPermissionInfoList;
        for (auto itBegin = itB->m_pmlist.begin(), itEnd = itB->m_pmlist.end(); itBegin != itEnd; ++itBegin)
        {
            Json::Value jsPm;
            jsPm["func_id"] = boost::lexical_cast<std::string>(itBegin->m_uiFuncID);
            jsPm["access"] = itBegin->m_strAccess;

            jsPermissionInfoList.append(jsPm);
        }

        jsRoleInfo["permission_info"] = jsPermissionInfoList;

        jsAllRoleInfo.append(jsRoleInfo);
    }
    
    LOG_INFO_RLD("Query all role info received and session id is " << strSid << " and user id is " << strUserID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::UserBindRoleHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("roleid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Role id not found.");
        return blResult;
    }
    const std::string strRoleID = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    if (!UserBindRole(strSid, strUserID, strRoleID))
    {
        LOG_ERROR_RLD("User bind role failed.");
        return blResult;
    }

    LOG_INFO_RLD("User bind role info received and session id is " << strSid << " and user id is " << strUserID << " and role id is " << strRoleID);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::CreateDomain(const std::string &strSid, const std::string &strUserID, DomainInfo &dmi)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::AddAreaReq AddAreaReq;
        AddAreaReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddAreaReq_T;
        AddAreaReq.m_uiMsgSeq = 1;
        AddAreaReq.m_strSID = strSid;

        AddAreaReq.m_strUserID = strUserID;
        AddAreaReq.m_areaInfo.m_strAreaName = dmi.m_strDomainName;
        AddAreaReq.m_areaInfo.m_strExtend = dmi.m_strExtend;
        AddAreaReq.m_areaInfo.m_strParentAreaID = dmi.m_strParentDomainID;
        AddAreaReq.m_areaInfo.m_uiLevel = dmi.m_uiLevel;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddAreaReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Create domain req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::AddAreaRsp AddAreaRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddAreaRsp))
        {
            LOG_ERROR_RLD("Create domain rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        dmi.m_strDomainID = AddAreaRsp.m_strAreaID;

        iRet = AddAreaRsp.m_iRetcode;

        LOG_INFO_RLD("Create domain and domain id is " << dmi.m_strDomainID << " and return code is " << AddAreaRsp.m_iRetcode <<
            " and return msg is " << AddAreaRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::RemoveDomain(const std::string &strSid, const std::string &strUserID, const std::string &strDomainID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::DeleteAreaReq DelAreaReq;
        DelAreaReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteAreaReq_T;
        DelAreaReq.m_uiMsgSeq = 1;
        DelAreaReq.m_strSID = strSid;

        DelAreaReq.m_strUserID = strUserID;
        DelAreaReq.m_strAreaID = strDomainID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelAreaReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Remove domain req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::DeleteAreaRsp DelAreaRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelAreaRsp))
        {
            LOG_ERROR_RLD("Remove domain rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DelAreaRsp.m_iRetcode;

        LOG_INFO_RLD("Remove domain and domain id is " << strDomainID << " and user id is " << strUserID << " and session id is " << strSid <<
            " and return code is " << DelAreaRsp.m_iRetcode <<
            " and return msg is " << DelAreaRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ModifyDomain(const std::string &strSid, const std::string &strUserID, DomainInfo &dmi)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::ModifyAreaReq ModifyAreaReq;
        ModifyAreaReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyAreaReq_T;
        ModifyAreaReq.m_uiMsgSeq = 1;
        ModifyAreaReq.m_strSID = strSid;

        ModifyAreaReq.m_strUserID = strUserID;
        ModifyAreaReq.m_areaInfo.m_strAreaName = dmi.m_strDomainName;
        ModifyAreaReq.m_areaInfo.m_strExtend = dmi.m_strExtend;
        ModifyAreaReq.m_areaInfo.m_strAreaID = dmi.m_strDomainID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModifyAreaReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify domain req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::ModifyAreaRsp ModifyAreaRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModifyAreaRsp))
        {
            LOG_ERROR_RLD("Modify domain rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModifyAreaRsp.m_iRetcode;

        LOG_INFO_RLD("Modify domain and domain id is " << dmi.m_strDomainID << " and return code is " << ModifyAreaRsp.m_iRetcode <<
            " and return msg is " << ModifyAreaRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryDomain(const std::string &strSid, const std::string &strUserID, const std::string &strDomainID, DomainInfo &dmi)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryAreaInfoReq QueryAreaReq;
        QueryAreaReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAreaInfoReq_T;
        QueryAreaReq.m_uiMsgSeq = 1;
        QueryAreaReq.m_strSID = strSid;

        QueryAreaReq.m_strUserID = strUserID;
        QueryAreaReq.m_strAreaID = strDomainID;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryAreaReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query domain req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryAreaInfoRsp QueryAreaRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryAreaRsp))
        {
            LOG_ERROR_RLD("Query domain rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        dmi.m_strDomainID = QueryAreaRsp.m_areaInfo.m_strAreaID;
        dmi.m_strDomainName = QueryAreaRsp.m_areaInfo.m_strAreaName;
        dmi.m_strExtend = QueryAreaRsp.m_areaInfo.m_strExtend;
        dmi.m_strParentDomainID = QueryAreaRsp.m_areaInfo.m_strParentAreaID;
        dmi.m_uiLevel = QueryAreaRsp.m_areaInfo.m_uiLevel;

        iRet = QueryAreaRsp.m_iRetcode;

        LOG_INFO_RLD("Query domain and domain id is " << dmi.m_strDomainID << " and return code is " << QueryAreaRsp.m_iRetcode <<
            " and return msg is " << QueryAreaRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryAllDomain(const std::string &strSid, const std::string &strUserID, std::list<DomainInfo> &dmilist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryAllAreaReq QueryAllAreaReq;
        QueryAllAreaReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllAreaReq_T;
        QueryAllAreaReq.m_uiMsgSeq = 1;
        QueryAllAreaReq.m_strSID = strSid;

        QueryAllAreaReq.m_strUserID = strUserID;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryAllAreaReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all domain req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryAllAreaRsp QueryAllAreaRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryAllAreaRsp))
        {
            LOG_ERROR_RLD("Query all domain rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        auto itBegin = QueryAllAreaRsp.m_areaList.begin();
        auto itEnd = QueryAllAreaRsp.m_areaList.end();
        while (itBegin != itEnd)
        {
            DomainInfo dmi;
            dmi.m_strDomainID = itBegin->m_strAreaID;
            dmi.m_strDomainName = itBegin->m_strAreaName;
            dmi.m_strExtend = itBegin->m_strExtend;
            dmi.m_strParentDomainID = itBegin->m_strParentAreaID;
            dmi.m_uiLevel = itBegin->m_uiLevel;

            dmilist.push_back(std::move(dmi));

            ++itBegin;
        }

        iRet = QueryAllAreaRsp.m_iRetcode;

        LOG_INFO_RLD("Query all domain and return code is " << QueryAllAreaRsp.m_iRetcode <<
            " and return msg is " << QueryAllAreaRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
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
        AddStoreReq.m_storeInfo.m_strTelephoneList.swap(store.m_strPhoneList);
        AddStoreReq.m_storeInfo.m_strCreateDate = strCurrentTime;
        AddStoreReq.m_storeInfo.m_strExtend = store.m_strExtend;
        AddStoreReq.m_storeInfo.m_strGoodsCategory = store.m_strGoodsCategory;
        AddStoreReq.m_storeInfo.m_strStoreName = store.m_strStoreName;
        AddStoreReq.m_storeInfo.m_uiState = 0;
        AddStoreReq.m_storeInfo.m_area.m_strAreaID = store.m_strDomainID;
        AddStoreReq.m_storeInfo.m_uiOpenState = store.m_uiOpenState;

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

bool PassengerFlowMsgHandler::ModifyStore(const std::string &strSid, const std::string &strUserID, StoreInfo &store)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::ModifyStoreReq ModStoreReq;
        ModStoreReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyStoreReq_T;
        ModStoreReq.m_uiMsgSeq = 1;
        ModStoreReq.m_strSID = strSid;

        ModStoreReq.m_strUserID = strUserID;
        ModStoreReq.m_storeInfo.m_strAddress = store.m_strAddress;
        ModStoreReq.m_storeInfo.m_strTelephoneList.swap(store.m_strPhoneList);
        //ModStoreReq.m_storeInfo.m_strCreateDate = strCurrentTime;
        ModStoreReq.m_storeInfo.m_strExtend = store.m_strExtend;
        ModStoreReq.m_storeInfo.m_strGoodsCategory = store.m_strGoodsCategory;
        ModStoreReq.m_storeInfo.m_strStoreName = store.m_strStoreName;
        ModStoreReq.m_storeInfo.m_uiState = 0;
        ModStoreReq.m_storeInfo.m_strStoreID = store.m_strStoreID;
        ModStoreReq.m_storeInfo.m_area.m_strAreaID = store.m_strDomainID;
        ModStoreReq.m_storeInfo.m_uiOpenState = store.m_uiOpenState;

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

bool PassengerFlowMsgHandler::QueryStore(const std::string &strSid, const std::string &strUserID, StoreInfo &store, std::list<EntranceInfo> &entranceInfolist,
    std::list<DomainInfo> &dmilist, std::list<std::string> &strPhoneList)
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
        store.m_strDomainID = QueryStoreRsp.m_storeInfo.m_area.m_strAreaID;
        store.m_strDomainName = QueryStoreRsp.m_storeInfo.m_area.m_strAreaName;
        store.m_uiOpenState = QueryStoreRsp.m_storeInfo.m_uiOpenState;

        strPhoneList.swap(QueryStoreRsp.m_storeInfo.m_strTelephoneList);

        for (auto itBegin = QueryStoreRsp.m_areaList.begin(), itEnd = QueryStoreRsp.m_areaList.end(); itBegin != itEnd; ++itBegin)
        {
            DomainInfo dmi;
            dmi.m_strDomainID = itBegin->m_strAreaID;
            dmi.m_strDomainName = itBegin->m_strAreaName;
            dmilist.push_back(std::move(dmi));
        }

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
            einfo.m_strPicture = itBegin->m_strPicture;
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


bool PassengerFlowMsgHandler::QueryAllStore(const std::string &strSid, const std::string &strUserID, const unsigned int uiBeginIndex, 
    const std::string &strDomainID, const unsigned int uiOpenState, std::list<StoreAndEntranceInfo> &storelist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryAllStoreReq QueryAllStoreReq;
        QueryAllStoreReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllStoreReq_T;
        QueryAllStoreReq.m_uiMsgSeq = 1;
        QueryAllStoreReq.m_strSID = strSid;

        QueryAllStoreReq.m_strUserID = strUserID;
        QueryAllStoreReq.m_uiBeginIndex = uiBeginIndex;
        QueryAllStoreReq.m_strAreaID = strDomainID;
        QueryAllStoreReq.m_uiOpenState = uiOpenState;

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
            StoreAndEntranceInfo store;
            store.stinfo.m_strStoreID = itBegin->m_strStoreID;
            store.stinfo.m_strStoreName = itBegin->m_strStoreName;
            store.stinfo.m_uiOpenState = itBegin->m_uiOpenState;
            store.stinfo.m_strAddress = itBegin->m_strAddress;
            store.stinfo.m_strDomainID = itBegin->m_area.m_strAreaID;
            store.stinfo.m_strDomainName = itBegin->m_area.m_strAreaName;

            store.stinfo.m_strPhoneList.swap(itBegin->m_strTelephoneList);

            for (auto itB = itBegin->m_entranceList.begin(), itE = itBegin->m_entranceList.end(); itB != itE; ++itB)
            {
                EntranceInfo einfo;
                einfo.m_strID = itB->m_strEntranceID;
                einfo.m_strName = itB->m_strEntranceName;
                einfo.m_strPicture = itB->m_strPicture;

                auto itB2 = itB->m_strDeviceIDList.begin();
                auto itE2 = itB->m_strDeviceIDList.end();
                while (itB2 != itE2)
                {
                    einfo.m_DeviceIDList.push_back(*itB2);
                    ++itB2;
                }

                store.etinfolist.push_back(std::move(einfo));
            }

            storelist.push_back(std::move(store));

            LOG_INFO_RLD("Query all store info received and store id is " << itBegin->m_strStoreID <<
                " and store name is " << itBegin->m_strStoreName << " and open state is " << itBegin->m_uiOpenState << " and address is " <<
                itBegin->m_strAddress << " and entrance list size is " << itBegin->m_entranceList.size());

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
        AddEntranceReq.m_entranceInfo.m_strPicture = einfo.m_strPicture;

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
    const std::string &strEntranceID, const std::string &strEntranceName, const std::string &strPicture, EntranceInfo &einfoadd, EntranceInfo &einfodel)
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
        ModEntranceReq.m_entranceInfo.m_strPicture = strPicture;

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
        ModEventReq.m_eventInfo.m_uiViewState = boost::lexical_cast<unsigned int>(eventinfo.m_strViewState);

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
        eventinfo.m_strViewState = boost::lexical_cast<std::string>(QueryEventRsp.m_eventInfo.m_uiViewState);
        eventinfo.m_strStoreID = QueryEventRsp.m_eventInfo.m_strStoreID;
        eventinfo.m_strStoreName = QueryEventRsp.m_eventInfo.m_strStoreName;
        
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

bool PassengerFlowMsgHandler::QueryAllEvent(const std::string &strSid, const std::string &strUserID, const unsigned int uiRelation, const unsigned int uiBeginIndex,
    const unsigned int uiProcessState, const unsigned int uiEventType,
    const std::string &strBeginDate, const std::string &strEndDate, std::list<EventInfo> &eventinfoList)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryAllEventReq QueryAllEventReq;
        QueryAllEventReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllEventReq_T;
        QueryAllEventReq.m_uiMsgSeq = 1;
        QueryAllEventReq.m_strSID = strSid;

        QueryAllEventReq.m_strUserID = strUserID;
        QueryAllEventReq.m_uiBeginIndex = uiBeginIndex;

        QueryAllEventReq.m_strBeginDate = strBeginDate;
        QueryAllEventReq.m_uiProcessState = uiProcessState;
        QueryAllEventReq.m_strEndDate = strEndDate;
        QueryAllEventReq.m_uiRelation = uiRelation;
        QueryAllEventReq.m_uiEventType = uiEventType;

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
            eventinfo.m_strViewState = boost::lexical_cast<std::string>(itBegin->m_uiViewState);

            eventinfo.m_strStoreID = itBegin->m_strStoreID;
            eventinfo.m_strStoreName = itBegin->m_strStoreName;
            
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
        AddPatrolReq.m_regularPatrol.m_strExtend = pat.m_strExtend;
        AddPatrolReq.m_regularPatrol.m_strHandlerList.swap(pat.m_strPatrolHandlerList);

        for (auto itBegin = pat.m_SAEList.begin(), itEnd = pat.m_SAEList.end(); itBegin != itEnd; ++itBegin)
        {
            PassengerFlowProtoHandler::PatrolStoreEntrance pse;
            pse.m_strStoreID = itBegin->stinfo.m_strStoreID;
            pse.m_strStoreName = itBegin->stinfo.m_strStoreName;

            for (auto itB = itBegin->etinfolist.begin(), itE = itBegin->etinfolist.end(); itB != itE; ++itB)
            {
                PassengerFlowProtoHandler::Entrance et;
                et.m_strEntranceID = itB->m_strID;
                et.m_strEntranceName = itB->m_strName;
                pse.m_entranceList.push_back(std::move(et));
            }

            AddPatrolReq.m_regularPatrol.m_storeEntranceList.push_back(std::move(pse));
        }
        
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
        ModPatrolReq.m_regularPatrol.m_strExtend = pat.m_strExtend;
        ModPatrolReq.m_regularPatrol.m_strHandlerList.swap(pat.m_strPatrolHandlerList);

        for (auto itBegin = pat.m_SAEList.begin(), itEnd = pat.m_SAEList.end(); itBegin != itEnd; ++itBegin)
        {
            PassengerFlowProtoHandler::PatrolStoreEntrance pse;
            pse.m_strStoreID = itBegin->stinfo.m_strStoreID;
            pse.m_strStoreName = itBegin->stinfo.m_strStoreName;

            for (auto itB = itBegin->etinfolist.begin(), itE = itBegin->etinfolist.end(); itB != itE; ++itB)
            {
                PassengerFlowProtoHandler::Entrance et;
                et.m_strEntranceID = itB->m_strID;
                et.m_strEntranceName = itB->m_strName;
                pse.m_entranceList.push_back(std::move(et));
            }

            ModPatrolReq.m_regularPatrol.m_storeEntranceList.push_back(std::move(pse));
        }

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
        pat.m_strPatrolName = QueryPatrolRsp.m_regularPatrol.m_strPlanName;
        pat.m_strCreateDate = QueryPatrolRsp.m_regularPatrol.m_strCreateDate;
        pat.m_strModifyDate = QueryPatrolRsp.m_regularPatrol.m_strUpdateDate;
        pat.m_strPatrolTimeList.swap(QueryPatrolRsp.m_regularPatrol.m_strPatrolTimeList);
        pat.m_strExtend = QueryPatrolRsp.m_regularPatrol.m_strExtend;
        pat.m_strPatrolHandlerList.swap(QueryPatrolRsp.m_regularPatrol.m_strHandlerList);

        for (auto itBegin = QueryPatrolRsp.m_regularPatrol.m_storeEntranceList.begin(), itEnd = QueryPatrolRsp.m_regularPatrol.m_storeEntranceList.end();
            itBegin != itEnd; ++itBegin)
        {
            StoreAndEntranceInfo sae;
            sae.stinfo.m_strStoreID = itBegin->m_strStoreID;
            sae.stinfo.m_strStoreName = itBegin->m_strStoreName;
            
            for (auto itB = itBegin->m_entranceList.begin(), itE = itBegin->m_entranceList.end(); itB != itE; ++itB)
            {
                EntranceInfo einfo;
                einfo.m_strID = itB->m_strEntranceID;
                einfo.m_strName = itB->m_strEntranceName;

                sae.etinfolist.push_back(std::move(einfo));
            }

            pat.m_SAEList.push_back(std::move(sae));
        }
        
        iRet = QueryPatrolRsp.m_iRetcode;

        LOG_INFO_RLD("Query regular patrol and plan id is " << pat.m_strPatrolID << " and session id is " << strSid << 
            " and create date is " << QueryPatrolRsp.m_regularPatrol.m_strCreateDate << " and modify date is " << QueryPatrolRsp.m_regularPatrol.m_strUpdateDate <<
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
            patinfo.m_strPatrolName = itBegin->m_strPlanName;
            patinfo.m_strCreateDate = itBegin->m_strCreateDate;
            patinfo.m_strModifyDate = itBegin->m_strUpdateDate;
            patinfo.m_strPatrolTimeList.swap(itBegin->m_strPatrolTimeList);
            patinfo.m_strExtend = itBegin->m_strExtend;
            patinfo.m_strPatrolHandlerList.swap(itBegin->m_strHandlerList);

            for (auto itBegin2 = itBegin->m_storeEntranceList.begin(), itEnd2 = itBegin->m_storeEntranceList.end();
                itBegin2 != itEnd2; ++itBegin2)
            {
                StoreAndEntranceInfo sae;
                sae.stinfo.m_strStoreID = itBegin2->m_strStoreID;
                sae.stinfo.m_strStoreName = itBegin2->m_strStoreName;

                for (auto itB = itBegin2->m_entranceList.begin(), itE = itBegin2->m_entranceList.end(); itB != itE; ++itB)
                {
                    EntranceInfo einfo;
                    einfo.m_strID = itB->m_strEntranceID;
                    einfo.m_strName = itB->m_strEntranceName;

                    sae.etinfolist.push_back(std::move(einfo));
                }

                patinfo.m_SAEList.push_back(std::move(sae));
            }
            
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
        UserJoinStoreReq.m_strAdministratorID = us.m_strAdminID;

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
            us.m_strAliasName = itBegin->m_strAliasName;

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

bool PassengerFlowMsgHandler::QueryAllUserList(const std::string &strSid, const std::string &strUserID, std::list<UserOfStore> &uslist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryCompanyAllUserReq QueryAllUserListReq;
        QueryAllUserListReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryCompanyAllUserReq_T;
        QueryAllUserListReq.m_uiMsgSeq = 1;
        QueryAllUserListReq.m_strSID = strSid;

        QueryAllUserListReq.m_strUserID = strUserID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryAllUserListReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all user list req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryCompanyAllUserRsp QueryAllUserListRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryAllUserListRsp))
        {
            LOG_ERROR_RLD("Query all user list rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = QueryAllUserListRsp.m_iRetcode;

        for (auto itBegin = QueryAllUserListRsp.m_userList.begin(), itEnd = QueryAllUserListRsp.m_userList.end(); itBegin != itEnd; ++itBegin)
        {
            UserOfStore us;
            us.m_strRole = itBegin->m_strRole;
            us.m_strUserID = itBegin->m_strUserID;
            us.m_strUserName = itBegin->m_strUserName;
            us.m_strAliasName = itBegin->m_strAliasName;
            us.m_strRole = itBegin->m_strRole;

            uslist.push_back(std::move(us));
        }

        LOG_INFO_RLD("Query all user list and session id is " << strSid << " and user id is " << strUserID << " and store id is " <<
            " and return code is " << QueryAllUserListRsp.m_iRetcode <<
            " and return msg is " << QueryAllUserListRsp.m_strRetMsg);

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
        AddEvaReq.m_storeEvaluation.m_strPictureList.swap(ev.m_strFileIDList);

        for (auto itBegin = ev.m_evlist.begin(), itEnd = ev.m_evlist.end(); itBegin != itEnd; ++itBegin)
        {
            PassengerFlowProtoHandler::EvaluationItemScore evs;
            evs.m_dScore = itBegin->m_dEvaluationValue;
            evs.m_evaluationItem.m_strItemID = itBegin->m_strEvaluationTmpID;
            evs.m_strDescription = itBegin->m_strEvaluationDesc;
            evs.m_strPictureList.swap(itBegin->m_strFileIDList);

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
        ModEvaReq.m_storeEvaluation.m_strPictureList.swap(ev.m_strFileIDList);

        for (auto itBegin = ev.m_evlist.begin(), itEnd = ev.m_evlist.end(); itBegin != itEnd; ++itBegin)
        {
            PassengerFlowProtoHandler::EvaluationItemScore evs;
            evs.m_dScore = itBegin->m_dEvaluationValue;
            evs.m_evaluationItem.m_strItemID = itBegin->m_strEvaluationTmpID;
            evs.m_strDescription = itBegin->m_strEvaluationDesc;
            evs.m_strPictureList.swap(itBegin->m_strFileIDList);

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
            evt.m_strFileIDList.swap(itBegin->m_strPictureList);
            
            ev.m_evlist.push_back(std::move(evt));
        }

        ev.m_strFileIDList.swap(QueryEvaRsp.m_storeEvaluation.m_strPictureList);

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

bool PassengerFlowMsgHandler::QueryAllEvaluationOfStore(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, const unsigned int uiCheckStatus,
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
        QueryEvaReq.m_uiCheckStatus = uiCheckStatus;
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
            ev.m_dEvaluationTemplateTotalValue = itBegin->m_dTotalPoint;
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

bool PassengerFlowMsgHandler::CreatePatrolRecord(const std::string &strSid, PatrolRecord &pr)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::AddRemotePatrolStoreReq AddRemotePRReq;
        AddRemotePRReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddRemotePatrolStoreReq_T;
        AddRemotePRReq.m_uiMsgSeq = 1;
        AddRemotePRReq.m_strSID = strSid;

        AddRemotePRReq.m_patrolStore.m_strDescription = pr.m_strPatrolDesc;
        AddRemotePRReq.m_patrolStore.m_strDeviceID = pr.m_strDevID;
        AddRemotePRReq.m_patrolStore.m_strPatrolDate = pr.m_strPatrolDate;
        AddRemotePRReq.m_patrolStore.m_strPatrolPictureList.swap(pr.m_strPatrolPicIDList);
        AddRemotePRReq.m_patrolStore.m_strStoreID = pr.m_strStoreID;
        AddRemotePRReq.m_patrolStore.m_strUserID = pr.m_strUserID;
        AddRemotePRReq.m_patrolStore.m_uiPatrolResult = pr.m_uiPatrolResult;
        AddRemotePRReq.m_patrolStore.m_strPlanID = pr.m_strPlanID;
        AddRemotePRReq.m_patrolStore.m_strEntranceIDList.swap(pr.m_strEntranceIDList);
        
        for (auto itBegin = pr.m_PicList.begin(), itEnd = pr.m_PicList.end(); itBegin != itEnd; ++itBegin)
        {
            PassengerFlowProtoHandler::EntrancePicture pic;
            pic.m_strEntranceID = itBegin->m_strEntranceID;
            pic.m_strPatrolPictureList.swap(itBegin->m_strPicIDList);

            AddRemotePRReq.m_patrolStore.m_patrolPictureList.push_back(std::move(pic));
        }

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddRemotePRReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Create patrol record req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::AddRemotePatrolStoreRsp AddRemotePRRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddRemotePRRsp))
        {
            LOG_ERROR_RLD("Create patrol record rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = AddRemotePRRsp.m_iRetcode;

        pr.m_strPatrolID = AddRemotePRRsp.m_strPatrolID;

        LOG_INFO_RLD("Create patrol record and patrol id is " << pr.m_strPatrolID <<
            " and session id is " << strSid <<
            " and return code is " << AddRemotePRRsp.m_iRetcode <<
            " and return msg is " << AddRemotePRRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::DeletePatrolRecord(const std::string &strSid, const std::string &strUserID, const std::string &strPatrolID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::DeleteRemotePatrolStoreReq DelRemotePRReq;
        DelRemotePRReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteRemotePatrolStoreReq_T;
        DelRemotePRReq.m_uiMsgSeq = 1;
        DelRemotePRReq.m_strSID = strSid;

        DelRemotePRReq.m_strUserID = strUserID;
        DelRemotePRReq.m_strPatrolID = strPatrolID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelRemotePRReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete patrol record req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::DeleteRemotePatrolStoreRsp DelRemotePRRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelRemotePRRsp))
        {
            LOG_ERROR_RLD("Delete patrol record unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DelRemotePRRsp.m_iRetcode;

        LOG_INFO_RLD("Delete patrol record id is " << strPatrolID << " and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << DelRemotePRRsp.m_iRetcode <<
            " and return msg is " << DelRemotePRRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ModifyPatrolRecord(const std::string &strSid, PatrolRecord &pr)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::ModifyRemotePatrolStoreReq ModRemotePRReq;
        ModRemotePRReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyRemotePatrolStoreReq_T;
        ModRemotePRReq.m_uiMsgSeq = 1;
        ModRemotePRReq.m_strSID = strSid;

        ModRemotePRReq.m_strUserID = pr.m_strUserID;

        ModRemotePRReq.m_patrolStore.m_strPatrolID = pr.m_strPatrolID;
        ModRemotePRReq.m_patrolStore.m_strDescription = pr.m_strPatrolDesc;
        ModRemotePRReq.m_patrolStore.m_strDeviceID = pr.m_strDevID;
        ModRemotePRReq.m_patrolStore.m_strPatrolDate = pr.m_strPatrolDate;
        ModRemotePRReq.m_patrolStore.m_strPatrolPictureList.swap(pr.m_strPatrolPicIDList);
        ModRemotePRReq.m_patrolStore.m_strStoreID = pr.m_strStoreID;
        ModRemotePRReq.m_patrolStore.m_strUserID = pr.m_strUserID;
        ModRemotePRReq.m_patrolStore.m_uiPatrolResult = pr.m_uiPatrolResult;
        
        for (auto itBegin = pr.m_PicList.begin(), itEnd = pr.m_PicList.end(); itBegin != itEnd; ++itBegin)
        {
            PassengerFlowProtoHandler::EntrancePicture pic;
            pic.m_strEntranceID = itBegin->m_strEntranceID;
            pic.m_strPatrolPictureList.swap(itBegin->m_strPicIDList);

            ModRemotePRReq.m_patrolStore.m_patrolPictureList.push_back(std::move(pic));
        }

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModRemotePRReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify patrol record req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::ModifyRemotePatrolStoreRsp ModRemotePRRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModRemotePRRsp))
        {
            LOG_ERROR_RLD("Modify patrol record rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModRemotePRRsp.m_iRetcode;

        LOG_INFO_RLD("Modify patrol record and patrol id is " << pr.m_strPatrolID <<
            " and session id is " << strSid <<
            " and return code is " << ModRemotePRRsp.m_iRetcode <<
            " and return msg is " << ModRemotePRRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryPatrolRecord(const std::string &strSid, PatrolRecord &pr)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryRemotePatrolStoreInfoReq QueryRemotePRReq;
        QueryRemotePRReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryRemotePatrolStoreInfoReq_T;
        QueryRemotePRReq.m_uiMsgSeq = 1;
        QueryRemotePRReq.m_strSID = strSid;

        QueryRemotePRReq.m_strPatrolID = pr.m_strPatrolID;
        QueryRemotePRReq.m_strUserID = pr.m_strUserID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryRemotePRReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query patrol record req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryRemotePatrolStoreInfoRsp QueryRemotePRRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryRemotePRRsp))
        {
            LOG_ERROR_RLD("Query patrol record rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        pr.m_strDevID = QueryRemotePRRsp.m_patrolStore.m_strDeviceID;
        pr.m_strPatrolDate = QueryRemotePRRsp.m_patrolStore.m_strPatrolDate;
        pr.m_strPatrolDesc = QueryRemotePRRsp.m_patrolStore.m_strDescription;
        pr.m_strStoreID = QueryRemotePRRsp.m_patrolStore.m_strStoreID;
        pr.m_strUserID = QueryRemotePRRsp.m_patrolStore.m_strUserID;
        pr.m_uiPatrolResult = QueryRemotePRRsp.m_patrolStore.m_uiPatrolResult;
        pr.m_strPlanID = QueryRemotePRRsp.m_patrolStore.m_strPlanID;
        //pr.m_strEntranceID = QueryRemotePRRsp.m_patrolStore.m_strEntranceID;
        pr.m_strEntranceIDList.swap(QueryRemotePRRsp.m_patrolStore.m_strEntranceIDList);

        //pr.m_strPatrolPicURLList.swap(QueryRemotePRRsp.m_patrolStore.m_strPatrolPictureList);

        for (auto itBegin = QueryRemotePRRsp.m_patrolStore.m_patrolPictureList.begin(), 
            itEnd = QueryRemotePRRsp.m_patrolStore.m_patrolPictureList.end(); itBegin != itEnd; ++ itBegin)
        {
            PatrolRecord::PatrolPic pic;
            pic.m_strEntranceID = itBegin->m_strEntranceID;
            pic.m_strPicIDList.swap(itBegin->m_strPatrolPictureList);

            pr.m_PicList.push_back(std::move(pic));
        }
        

        iRet = QueryRemotePRRsp.m_iRetcode;

        LOG_INFO_RLD("Query patrol record and patrol id is " << pr.m_strPatrolID <<
            " and session id is " << strSid <<
            " and return code is " << QueryRemotePRRsp.m_iRetcode <<
            " and return msg is " << QueryRemotePRRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryAllPatrolRecord(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, 
    const unsigned int uiPatrolResult, const unsigned int uiPlanFlag, const std::string &strPlanID,
    const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiBeginIndex, std::list<PatrolRecord> &prlist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryAllRemotePatrolStoreReq QueryAllRemotePRReq;
        QueryAllRemotePRReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllRemotePatrolStoreReq_T;
        QueryAllRemotePRReq.m_uiMsgSeq = 1;
        QueryAllRemotePRReq.m_strSID = strSid;

        QueryAllRemotePRReq.m_strBeginDate = strBeginDate;
        QueryAllRemotePRReq.m_strEndDate = strEndDate;
        QueryAllRemotePRReq.m_strStoreID = strStoreID;
        QueryAllRemotePRReq.m_strPlanID = strPlanID;
        QueryAllRemotePRReq.m_strUserID = strUserID;
        QueryAllRemotePRReq.m_uiBeginIndex = uiBeginIndex;
        QueryAllRemotePRReq.m_uiPatrolResult = uiPatrolResult;
        QueryAllRemotePRReq.m_uiPlanFlag = uiPlanFlag;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryAllRemotePRReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all patrol record req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryAllRemotePatrolStoreRsp QueryAllRemotePRRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryAllRemotePRRsp))
        {
            LOG_ERROR_RLD("Query patrol record rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        for (auto itBegin = QueryAllRemotePRRsp.m_patrolStoreList.begin(), itEnd = QueryAllRemotePRRsp.m_patrolStoreList.end(); itBegin != itEnd; ++itBegin)
        {
            PatrolRecord pr;
            pr.m_strDevID = itBegin->m_strDeviceID;
            pr.m_strPatrolDate = itBegin->m_strPatrolDate;
            pr.m_strPatrolDesc = itBegin->m_strDescription;
            pr.m_strStoreID = itBegin->m_strStoreID;
            pr.m_strUserID = itBegin->m_strUserID;
            pr.m_uiPatrolResult = itBegin->m_uiPatrolResult;
            pr.m_strPlanID = itBegin->m_strPlanID;
            //pr.m_strEntranceID = itBegin->m_strEntranceID;
            pr.m_strEntranceIDList.swap(itBegin->m_strEntranceIDList);
            pr.m_strPatrolID = itBegin->m_strPatrolID;

            //pr.m_strPatrolPicURLList.swap(itBegin->m_strPatrolPictureList);

            for (auto itBeginPic = itBegin->m_patrolPictureList.begin(),
                itEndPic = itBegin->m_patrolPictureList.end(); itBeginPic != itEndPic; ++itBeginPic)
            {
                PatrolRecord::PatrolPic pic;
                pic.m_strEntranceID = itBeginPic->m_strEntranceID;
                pic.m_strPicIDList.swap(itBeginPic->m_strPatrolPictureList);

                pr.m_PicList.push_back(std::move(pic));
            }


            prlist.push_back(std::move(pr));
        }

        iRet = QueryAllRemotePRRsp.m_iRetcode;

        LOG_INFO_RLD("Query all patrol record and store id is " << strStoreID <<
            " and session id is " << strSid <<
            " and return code is " << QueryAllRemotePRRsp.m_iRetcode <<
            " and return msg is " << QueryAllRemotePRRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::CreateStoreSensor(const std::string &strSid, const std::string &strUserID, Sensor &sr)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::AddStoreSensorReq AddStoreSensorReq;
        AddStoreSensorReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddStoreSensorReq_T;
        AddStoreSensorReq.m_uiMsgSeq = 1;
        AddStoreSensorReq.m_strSID = strSid;

        AddStoreSensorReq.m_strUserID = strUserID;
        AddStoreSensorReq.m_sensorInfo.m_strDeviceID = sr.m_strDevID;
        AddStoreSensorReq.m_sensorInfo.m_strSensorName = sr.m_strName;
        AddStoreSensorReq.m_sensorInfo.m_strSensorType = boost::lexical_cast<std::string>(sr.m_uiType);
        AddStoreSensorReq.m_sensorInfo.m_strSensorAlarmThreshold = sr.m_strAlarmThreshold;
        AddStoreSensorReq.m_sensorInfo.m_strStoreID = sr.m_strStoreID;
        AddStoreSensorReq.m_sensorInfo.m_strSensorKey = sr.m_strSensorKey;
        AddStoreSensorReq.m_sensorInfo.m_strLocation = sr.m_strLocation;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddStoreSensorReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Create store sensor req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::AddStoreSensorRsp AddStoreSensorRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddStoreSensorRsp))
        {
            LOG_ERROR_RLD("Create store sensor rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = AddStoreSensorRsp.m_iRetcode;

        sr.m_strID = AddStoreSensorRsp.m_strSensorID;

        LOG_INFO_RLD("Create store sensor and id is " << sr.m_strID <<
            " and session id is " << strSid <<
            " and return code is " << AddStoreSensorRsp.m_iRetcode <<
            " and return msg is " << AddStoreSensorRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::DeleteStoreSensor(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, const std::string &strSensorID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::DeleteStoreSensorReq DelStoreSensorReq;
        DelStoreSensorReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteStoreSensorReq_T;
        DelStoreSensorReq.m_uiMsgSeq = 1;
        DelStoreSensorReq.m_strSID = strSid;

        DelStoreSensorReq.m_strUserID = strUserID;
        DelStoreSensorReq.m_strSensorID = strSensorID;
        DelStoreSensorReq.m_strStoreID = strStoreID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelStoreSensorReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete store sensor req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::DeleteStoreSensorRsp DelStoreSensorRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelStoreSensorRsp))
        {
            LOG_ERROR_RLD("Delete store sensor unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DelStoreSensorRsp.m_iRetcode;

        LOG_INFO_RLD("Delete store sensor id is " << strSensorID << " and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << DelStoreSensorRsp.m_iRetcode <<
            " and return msg is " << DelStoreSensorRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ModifyStoreSensor(const std::string &strSid, const std::string &strUserID, Sensor &sr)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::ModifyStoreSensorReq ModifyStoreSensorReq;
        ModifyStoreSensorReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyStoreSensorReq_T;
        ModifyStoreSensorReq.m_uiMsgSeq = 1;
        ModifyStoreSensorReq.m_strSID = strSid;

        ModifyStoreSensorReq.m_strUserID = strUserID;
        ModifyStoreSensorReq.m_sensorInfo.m_strDeviceID = sr.m_strDevID;
        ModifyStoreSensorReq.m_sensorInfo.m_strSensorName = sr.m_strName;
        //ModifyStoreSensorReq.m_sensorInfo.m_strSensorType = 0xFFFFFFFF == sr.m_uiType ? "" : boost::lexical_cast<std::string>(sr.m_uiType);
        ModifyStoreSensorReq.m_sensorInfo.m_strSensorType = boost::lexical_cast<std::string>(sr.m_uiType);
        ModifyStoreSensorReq.m_sensorInfo.m_strSensorAlarmThreshold = sr.m_strAlarmThreshold;
        ModifyStoreSensorReq.m_sensorInfo.m_strStoreID = sr.m_strStoreID;
        ModifyStoreSensorReq.m_sensorInfo.m_strSensorID = sr.m_strID;
        ModifyStoreSensorReq.m_sensorInfo.m_strSensorKey = sr.m_strSensorKey;
        ModifyStoreSensorReq.m_sensorInfo.m_strLocation = sr.m_strLocation;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModifyStoreSensorReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify store sensor req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::ModifyStoreSensorRsp ModifyStoreSensorRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModifyStoreSensorRsp))
        {
            LOG_ERROR_RLD("Modify store sensor rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModifyStoreSensorRsp.m_iRetcode;

        LOG_INFO_RLD("Modify store sensor and id is " << sr.m_strID <<
            " and session id is " << strSid <<
            " and return code is " << ModifyStoreSensorRsp.m_iRetcode <<
            " and return msg is " << ModifyStoreSensorRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryStoreSensor(const std::string &strSid, const std::string &strUserID, Sensor &sr)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryStoreSensorInfoReq QuerySensorInfoReq;
        QuerySensorInfoReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryStoreSensorInfoReq_T;
        QuerySensorInfoReq.m_uiMsgSeq = 1;
        QuerySensorInfoReq.m_strSID = strSid;

        QuerySensorInfoReq.m_strSensorID = sr.m_strID;
        QuerySensorInfoReq.m_strUserID = strUserID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QuerySensorInfoReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query sensor info req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryStoreSensorInfoRsp QuerySensorInfoRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QuerySensorInfoRsp))
        {
            LOG_ERROR_RLD("Query sensor info rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        sr.m_strDevID = QuerySensorInfoRsp.m_sensorInfo.m_strDeviceID;
        sr.m_strName = QuerySensorInfoRsp.m_sensorInfo.m_strSensorName;
        sr.m_strStoreID = QuerySensorInfoRsp.m_sensorInfo.m_strStoreID;
        sr.m_strValue = QuerySensorInfoRsp.m_sensorInfo.m_strValue;
        sr.m_uiType = QuerySensorInfoRsp.m_sensorInfo.m_strSensorType.empty() ? 0xFFFFFFFF : boost::lexical_cast<unsigned int>(QuerySensorInfoRsp.m_sensorInfo.m_strSensorType);
        sr.m_strAlarmThreshold = QuerySensorInfoRsp.m_sensorInfo.m_strSensorAlarmThreshold;
        sr.m_strSensorKey = QuerySensorInfoRsp.m_sensorInfo.m_strSensorKey;
        sr.m_strLocation = QuerySensorInfoRsp.m_sensorInfo.m_strLocation;

        iRet = QuerySensorInfoRsp.m_iRetcode;

        LOG_INFO_RLD("Query sensor info and sensor id is " << sr.m_strID <<
            " and session id is " << strSid <<
            " and return code is " << QuerySensorInfoRsp.m_iRetcode <<
            " and return msg is " << QuerySensorInfoRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryAllStoreSensor(const std::string &strSid, const std::string &strUserID, const std::string &strStoreID, std::list<Sensor> &srlist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryAllStoreSensorReq QueryAllSensorReq;
        QueryAllSensorReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllStoreSensorReq_T;
        QueryAllSensorReq.m_uiMsgSeq = 1;
        QueryAllSensorReq.m_strSID = strSid;

        QueryAllSensorReq.m_strStoreID = strStoreID;
        QueryAllSensorReq.m_strUserID = strUserID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryAllSensorReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all sensor info req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryAllStoreSensorRsp QueryAllSensorRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryAllSensorRsp))
        {
            LOG_ERROR_RLD("Query all sensor info rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        for (auto itBegin = QueryAllSensorRsp.m_sensorList.begin(), itEnd = QueryAllSensorRsp.m_sensorList.end(); itBegin != itEnd; ++itBegin)
        {
            Sensor sr;
            sr.m_strDevID = itBegin->m_strDeviceID;
            sr.m_strID = itBegin->m_strSensorID;
            sr.m_strName = itBegin->m_strSensorName;
            sr.m_strStoreID = itBegin->m_strStoreID;
            sr.m_strValue = itBegin->m_strValue;
            sr.m_uiType = itBegin->m_strSensorType.empty() ? 0xFFFFFFFF : boost::lexical_cast<unsigned int>(itBegin->m_strSensorType);
            sr.m_strAlarmThreshold = itBegin->m_strSensorAlarmThreshold;
            sr.m_strSensorKey = itBegin->m_strSensorKey;
            sr.m_strLocation = itBegin->m_strLocation;

            srlist.push_back(std::move(sr));
        }

        iRet = QueryAllSensorRsp.m_iRetcode;

        LOG_INFO_RLD("Query all sensor info record and store id is " << strStoreID <<
            " and session id is " << strSid <<
            " and return code is " << QueryAllSensorRsp.m_iRetcode <<
            " and return msg is " << QueryAllSensorRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ReportSensorInfo(const std::string &strSid, const std::string &strDevID, std::list<Sensor> &srlist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::ReportSensorInfoReq ReportSensorReq;
        ReportSensorReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ReportSensorInfoReq_T;
        ReportSensorReq.m_uiMsgSeq = 1;
        ReportSensorReq.m_strSID = strSid;

        ReportSensorReq.m_strDeviceID = strDevID;

        std::list<PassengerFlowProtoHandler::Sensor> psrlist;
        for (auto itBegin = srlist.begin(), itEnd = srlist.end(); itBegin != itEnd; ++itBegin)
        {
            PassengerFlowProtoHandler::Sensor psr;
            psr.m_strDeviceID = strDevID;
            psr.m_strValue = itBegin->m_strValue;
            psr.m_strSensorType = boost::lexical_cast<std::string>(itBegin->m_uiType);
            psr.m_strSensorKey = itBegin->m_strSensorKey;
            psrlist.push_back(std::move(psr));
        }
        ReportSensorReq.m_sensorList.swap(psrlist);

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ReportSensorReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Report sensor req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::ReportSensorInfoRsp ReportSensorRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ReportSensorRsp))
        {
            LOG_ERROR_RLD("Report sensor unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ReportSensorRsp.m_iRetcode;

        LOG_INFO_RLD("Report sensor and session id is " << strSid << " and device id is " << strDevID <<
            " and return code is " << ReportSensorRsp.m_iRetcode <<
            " and return msg is " << ReportSensorRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ReportSensorAlarmInfo(const std::string &strSid, const Sensor &sr, const unsigned int uiRecover, const std::string &strFileID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::ReportSensorAlarmInfoReq ReportSensorAlarmReq;
        ReportSensorAlarmReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ReportSensorAlarmInfoReq_T;
        ReportSensorAlarmReq.m_uiMsgSeq = 1;
        ReportSensorAlarmReq.m_strSID = strSid;

        ReportSensorAlarmReq.m_strFileID = strFileID;
        ReportSensorAlarmReq.m_uiRecover = uiRecover;


        ReportSensorAlarmReq.m_sensorInfo.m_strDeviceID = sr.m_strDevID;
        //sr.m_strName = strName;
        //sr.m_strStoreID = strStoreID;
        ReportSensorAlarmReq.m_sensorInfo.m_strSensorType = boost::lexical_cast<std::string>(sr.m_uiType);
        //sr.m_strID = strSensorID;
        ReportSensorAlarmReq.m_sensorInfo.m_strSensorAlarmThreshold = sr.m_strAlarmThreshold;
        ReportSensorAlarmReq.m_sensorInfo.m_strValue = sr.m_strValue;
        ReportSensorAlarmReq.m_sensorInfo.m_strSensorKey = sr.m_strSensorKey;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ReportSensorAlarmReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Report sensor alarm req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::ReportSensorAlarmInfoRsp ReportSensorAlarmRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ReportSensorAlarmRsp))
        {
            LOG_ERROR_RLD("Report sensor alarm unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ReportSensorAlarmRsp.m_iRetcode;

        LOG_INFO_RLD("Report sensor alarm and session id is " << strSid << " and device id is " << sr.m_strDevID <<
            " and return code is " << ReportSensorAlarmRsp.m_iRetcode <<
            " and return msg is " << ReportSensorAlarmRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QueryPatrolResult(const std::string &strSid, const std::string &strUserID, PatrolResultReportQueryParam &prqm, std::string &strReport)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryPatrolResultReportReq QueryPatrolResultReq;
        QueryPatrolResultReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryPatrolResultReportReq_T;
        QueryPatrolResultReq.m_uiMsgSeq = 1;
        QueryPatrolResultReq.m_strSID = strSid;

        QueryPatrolResultReq.m_strBeginDate = prqm.m_strBeginDate;
        QueryPatrolResultReq.m_strUserID = strUserID;
        QueryPatrolResultReq.m_strEndDate = prqm.m_strEndDate;
        QueryPatrolResultReq.m_strPatrolUserID = prqm.m_strPatrolUserID;
        QueryPatrolResultReq.m_strStoreID = prqm.m_strStoreID;
        QueryPatrolResultReq.m_uiPatrolResult = prqm.m_uiPatrolResult;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryPatrolResultReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query patrol result info req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryPatrolResultReportRsp QuerySensorInfoRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QuerySensorInfoRsp))
        {
            LOG_ERROR_RLD("Query patrol result info rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        strReport = QuerySensorInfoRsp.m_strChartData;
        
        iRet = QuerySensorInfoRsp.m_iRetcode;

        LOG_INFO_RLD("Query sensor info and report is " << strReport <<
            " and session id is " << strSid <<
            " and return code is " << QuerySensorInfoRsp.m_iRetcode <<
            " and return msg is " << QuerySensorInfoRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QuerySensorAlarmThreshold(const std::string &strSid, const std::string &strDevID, std::list<Sensor> &srlist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QuerySensorAlarmThresholdReq QuerySensorAlarmThresholdReq;
        QuerySensorAlarmThresholdReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QuerySensorAlarmThresholdReq_T;
        QuerySensorAlarmThresholdReq.m_uiMsgSeq = 1;
        QuerySensorAlarmThresholdReq.m_strSID = strSid;

        QuerySensorAlarmThresholdReq.m_strDeviceID = strDevID;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QuerySensorAlarmThresholdReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query sensor alarm threshold req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QuerySensorAlarmThresholdRsp QuerySensorAlarmThresholdRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QuerySensorAlarmThresholdRsp))
        {
            LOG_ERROR_RLD("Query sensor alarm threshold rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        for (auto itBegin = QuerySensorAlarmThresholdRsp.m_sensorList.begin(), itEnd = QuerySensorAlarmThresholdRsp.m_sensorList.end(); itBegin != itEnd; ++itBegin)
        {
            Sensor sr;
            sr.m_strAlarmThreshold = itBegin->m_strSensorAlarmThreshold;
            sr.m_strDevID = itBegin->m_strDeviceID;
            sr.m_strID = itBegin->m_strSensorID;
            sr.m_strName = itBegin->m_strSensorName;
            sr.m_strStoreID = itBegin->m_strStoreID;
            sr.m_strValue = itBegin->m_strValue;
            sr.m_uiType = boost::lexical_cast<unsigned int>(itBegin->m_strSensorType);
            sr.m_strSensorKey = itBegin->m_strSensorKey;

            srlist.push_back(std::move(sr));
        }

        iRet = QuerySensorAlarmThresholdRsp.m_iRetcode;

        LOG_INFO_RLD("Query sensor alarm threshold and session id is " << strSid << " and device id is " << strDevID <<
            " and return code is " << QuerySensorAlarmThresholdRsp.m_iRetcode <<
            " and return msg is " << QuerySensorAlarmThresholdRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::RemoveSensorRecords(const std::string &strSid, const std::string &strUserID, std::list<std::string> &strRecordIDList)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::RemoveSensorRecordsReq RemoveSensorRecordsReq;
        RemoveSensorRecordsReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::RemoveSensorRecordsReq_T;
        RemoveSensorRecordsReq.m_uiMsgSeq = 1;
        RemoveSensorRecordsReq.m_strSID = strSid;

        RemoveSensorRecordsReq.m_strUserID = strUserID;
        
        RemoveSensorRecordsReq.m_strRecordIDList.swap(strRecordIDList);

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(RemoveSensorRecordsReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Remove sensor records req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::RemoveSensorRecordsRsp RemoveSensorRecordsRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, RemoveSensorRecordsRsp))
        {
            LOG_ERROR_RLD("Remove sensor records rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = RemoveSensorRecordsRsp.m_iRetcode;

        LOG_INFO_RLD("Remove sensor records and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << RemoveSensorRecordsRsp.m_iRetcode <<
            " and return msg is " << RemoveSensorRecordsRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::RemoveSensorAlarmRecords(const std::string &strSid, const std::string &strUserID, std::list<std::string> &strRecordIDList)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::RemoveSensorAlarmRecordsReq RemoveSensorAlarmRecordsReq;
        RemoveSensorAlarmRecordsReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::RemoveSensorAlarmRecordsReq_T;
        RemoveSensorAlarmRecordsReq.m_uiMsgSeq = 1;
        RemoveSensorAlarmRecordsReq.m_strSID = strSid;

        RemoveSensorAlarmRecordsReq.m_strUserID = strUserID;

        RemoveSensorAlarmRecordsReq.m_strRecordIDList.swap(strRecordIDList);

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(RemoveSensorAlarmRecordsReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Remove sensor alarm records req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::RemoveSensorAlarmRecordsRsp RemoveSensorAlarmRecordsRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, RemoveSensorAlarmRecordsRsp))
        {
            LOG_ERROR_RLD("Remove sensor alarm records rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = RemoveSensorAlarmRecordsRsp.m_iRetcode;

        LOG_INFO_RLD("Remove sensor alarm records and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << RemoveSensorAlarmRecordsRsp.m_iRetcode <<
            " and return msg is " << RemoveSensorAlarmRecordsRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QuerySensorRecords(const std::string &strSid, const QuerySensorParam &pm, std::list<SensorRecord> &srdlist,
	unsigned int &uiRealRecordNum)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QuerySensorRecordsReq QuerySensorRecordsReq;
        QuerySensorRecordsReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QuerySensorRecordsReq_T;
        QuerySensorRecordsReq.m_uiMsgSeq = 1;
        QuerySensorRecordsReq.m_strSID = strSid;

        QuerySensorRecordsReq.m_strBeginDate = pm.m_strBeginDate;
        QuerySensorRecordsReq.m_strEndDate = pm.m_strEndDate;
        QuerySensorRecordsReq.m_strSensorID = pm.m_strSensorID;
        QuerySensorRecordsReq.m_strSensorType = pm.m_strType;
        QuerySensorRecordsReq.m_strStoreID = pm.m_strStoreID;
        QuerySensorRecordsReq.m_strUserID = pm.m_strUserID;
        QuerySensorRecordsReq.m_uiBeginIndex = pm.m_uiBeginIndex;
		QuerySensorRecordsReq.m_uiTimeRangeType = pm.m_uiRangeType;
		QuerySensorRecordsReq.m_uiTimeRangeBase = pm.m_uiRangeBase;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QuerySensorRecordsReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query sensor records req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QuerySensorRecordsRsp QuerySensorRecordsRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QuerySensorRecordsRsp))
        {
            LOG_ERROR_RLD("Query sensor records rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        if (QuerySensorRecordsRsp.m_sensorList.size() != QuerySensorRecordsRsp.m_strRecordIDList.size())
        {
            LOG_ERROR_RLD("Query sensor records rsp data is invalid and sensor list size is " << QuerySensorRecordsRsp.m_sensorList.size() <<
                " and record id list size is " << QuerySensorRecordsRsp.m_strRecordIDList.size());
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = QuerySensorRecordsRsp.m_iRetcode;
        
        auto itBegin2 = QuerySensorRecordsRsp.m_strRecordIDList.begin();
        for (auto itBegin = QuerySensorRecordsRsp.m_sensorList.begin(), itEnd = QuerySensorRecordsRsp.m_sensorList.end();
            itBegin != itEnd; ++itBegin)
        {
            SensorRecord srd;
            srd.m_strCreateDate = itBegin->m_strCreateDate;
            srd.m_strRecordID = *itBegin2++;
            srd.sr.m_strAlarmThreshold = itBegin->m_strSensorAlarmThreshold;
            srd.sr.m_strDevID = itBegin->m_strDeviceID;
            srd.sr.m_strID = itBegin->m_strSensorID;
            srd.sr.m_strName = itBegin->m_strSensorName;
            srd.sr.m_strStoreID = itBegin->m_strStoreID;
            srd.sr.m_strValue = itBegin->m_strValue;
            srd.sr.m_uiType = boost::lexical_cast<unsigned int>(itBegin->m_strSensorType);
            srd.sr.m_strSensorKey = itBegin->m_strSensorKey;
            srd.sr.m_strLocation = itBegin->m_strLocation;

            srdlist.push_back(std::move(srd));
        }

		uiRealRecordNum = QuerySensorRecordsRsp.m_uiRealRecordNum;

        LOG_INFO_RLD("Query sensor records and session id is " << strSid << " and user id is " << pm.m_strUserID <<
			" and real record number is " << uiRealRecordNum <<
            " and return code is " << QuerySensorRecordsRsp.m_iRetcode <<
            " and return msg is " << QuerySensorRecordsRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::QuerySensorAlarmRecords(const std::string &strSid, const QuerySensorParam &pm, std::list<SensorAlarmRecord> &srdlist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QuerySensorAlarmRecordsReq QuerySensorAlarmRecordsReq;
        QuerySensorAlarmRecordsReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QuerySensorAlarmRecordsReq_T;
        QuerySensorAlarmRecordsReq.m_uiMsgSeq = 1;
        QuerySensorAlarmRecordsReq.m_strSID = strSid;

        QuerySensorAlarmRecordsReq.m_strBeginDate = pm.m_strBeginDate;
        QuerySensorAlarmRecordsReq.m_strEndDate = pm.m_strEndDate;
        QuerySensorAlarmRecordsReq.m_strSensorID = pm.m_strSensorID;
        QuerySensorAlarmRecordsReq.m_strSensorType = pm.m_strType;
        QuerySensorAlarmRecordsReq.m_uiRecover = pm.m_uiRecover;
        QuerySensorAlarmRecordsReq.m_strStoreID = pm.m_strStoreID;
        QuerySensorAlarmRecordsReq.m_strUserID = pm.m_strUserID;
        QuerySensorAlarmRecordsReq.m_uiBeginIndex = pm.m_uiBeginIndex;


        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QuerySensorAlarmRecordsReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query sensor alarm records req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QuerySensorAlarmRecordsRsp QuerySensorAlarmRecordsRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QuerySensorAlarmRecordsRsp))
        {
            LOG_ERROR_RLD("Query sensor alarm records rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = QuerySensorAlarmRecordsRsp.m_iRetcode;

        for (auto itBegin = QuerySensorAlarmRecordsRsp.m_sardList.begin(), itEnd = QuerySensorAlarmRecordsRsp.m_sardList.end();
            itBegin != itEnd; ++itBegin)
        {
            SensorAlarmRecord srd;
            srd.m_strCreateDate = itBegin->m_sensorInfo.m_strCreateDate;
            srd.m_strRecordID = itBegin->m_strRecordID;
            srd.m_strFileID = itBegin->m_strFileID;
            srd.m_uiRecover = itBegin->m_uiRecover;
            srd.m_sr.m_strAlarmThreshold = itBegin->m_sensorInfo.m_strSensorAlarmThreshold;
            srd.m_sr.m_strDevID = itBegin->m_sensorInfo.m_strDeviceID;
            srd.m_sr.m_strID = itBegin->m_sensorInfo.m_strSensorID;
            srd.m_sr.m_strName = itBegin->m_sensorInfo.m_strSensorName;
            srd.m_sr.m_strStoreID = itBegin->m_sensorInfo.m_strStoreID;
            srd.m_sr.m_strValue = itBegin->m_sensorInfo.m_strValue;
            srd.m_sr.m_uiType = boost::lexical_cast<unsigned int>(itBegin->m_sensorInfo.m_strSensorType);
            srd.m_sr.m_strSensorKey = itBegin->m_sensorInfo.m_strSensorKey;
            srd.m_sr.m_strLocation = itBegin->m_sensorInfo.m_strLocation;

            srdlist.push_back(std::move(srd));
        }

        LOG_INFO_RLD("Query sensor alarm records and session id is " << strSid << " and user id is " << pm.m_strUserID <<
            " and return code is " << QuerySensorAlarmRecordsRsp.m_iRetcode <<
            " and return msg is " << QuerySensorAlarmRecordsRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::AddRole(const std::string &strSid, const std::string &strUserID, const std::string &strRoleIDNew, const std::string &strRoleIDOld)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::AddRoleReq AddRoleReq;
        AddRoleReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::AddRoleReq_T;
        AddRoleReq.m_uiMsgSeq = 1;
        AddRoleReq.m_strSID = strSid;

        AddRoleReq.m_strUserID = strUserID;
        AddRoleReq.m_strRoleIDNew = strRoleIDNew;
        AddRoleReq.m_strRoleIDOld = strRoleIDOld;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddRoleReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add role req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::AddRoleRsp AddRoleRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddRoleRsp))
        {
            LOG_ERROR_RLD("Add role rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = AddRoleRsp.m_iRetcode;

        LOG_INFO_RLD("Add role and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << AddRoleRsp.m_iRetcode <<
            " and return msg is " << AddRoleRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::RemoveRole(const std::string &strSid, const std::string &strUserID, const std::string &strRoleID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::RemoveRoleReq RemoveRoleReq;
        RemoveRoleReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::RemoveRoleReq_T;
        RemoveRoleReq.m_uiMsgSeq = 1;
        RemoveRoleReq.m_strSID = strSid;

        RemoveRoleReq.m_strUserID = strUserID;
        RemoveRoleReq.m_strRoleID = strRoleID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(RemoveRoleReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Remove role req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::RemoveRoleRsp RemoveRoleRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, RemoveRoleRsp))
        {
            LOG_ERROR_RLD("Remove role rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = RemoveRoleRsp.m_iRetcode;

        LOG_INFO_RLD("Remove role and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << RemoveRoleRsp.m_iRetcode <<
            " and return msg is " << RemoveRoleRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ModifyRole(const std::string &strSid, const std::string  &strUserID, const std::string &strRoleID, 
    const std::list<unsigned int> &uiAllowFuncList, const std::list<unsigned int> &uiDisallowFuncList)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::ModifyRoleReq ModifyRoleReq;
        ModifyRoleReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyRoleReq_T;
        ModifyRoleReq.m_uiMsgSeq = 1;
        ModifyRoleReq.m_strSID = strSid;

        ModifyRoleReq.m_strUserID = strUserID;
        ModifyRoleReq.m_role.m_strRoleID = strRoleID;
        
        for (auto itBegin = uiAllowFuncList.begin(), itEnd = uiAllowFuncList.end(); itBegin != itEnd; ++itBegin)
        {
            PassengerFlowProtoHandler::Permission pm;
            pm.m_strAccess = "allow";
            pm.m_uiFuncID = *itBegin;
            
            ModifyRoleReq.m_role.m_pmlist.push_back(std::move(pm));
        }

        for (auto itBegin = uiDisallowFuncList.begin(), itEnd = uiDisallowFuncList.end(); itBegin != itEnd; ++itBegin)
        {
            PassengerFlowProtoHandler::Permission pm;
            pm.m_strAccess = "disallow";
            pm.m_uiFuncID = *itBegin;

            ModifyRoleReq.m_role.m_pmlist.push_back(std::move(pm));
        }

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModifyRoleReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify role req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::ModifyRoleRsp ModifyRoleRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModifyRoleRsp))
        {
            LOG_ERROR_RLD("Modify role rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModifyRoleRsp.m_iRetcode;

        LOG_INFO_RLD("Modify role and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << ModifyRoleRsp.m_iRetcode <<
            " and return msg is " << ModifyRoleRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

template<typename T>
bool PassengerFlowMsgHandler::QueryRole(const std::string &strSid, const std::string &strUserID, const std::string &strRoleID, T &Role)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryRoleReq QueryRoleReq;
        QueryRoleReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryRoleReq_T;
        QueryRoleReq.m_uiMsgSeq = 1;
        QueryRoleReq.m_strSID = strSid;

        QueryRoleReq.m_strUserID = strUserID;
        QueryRoleReq.m_strRoleID = strRoleID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryRoleReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query role req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryRoleRsp QueryRoleRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryRoleRsp))
        {
            LOG_ERROR_RLD("Query role rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = QueryRoleRsp.m_iRetcode;

        Role.m_strCreateDate = QueryRoleRsp.m_role.m_strCreateDate;
        Role.m_strRoleID = QueryRoleRsp.m_role.m_strRoleID;
        Role.m_strUpdateDate = QueryRoleRsp.m_role.m_strUpdateDate;
        Role.m_uiState = QueryRoleRsp.m_role.m_uiState;
        Role.m_pmlist.swap(QueryRoleRsp.m_role.m_pmlist);

        for (auto itBegin = Role.m_pmlist.begin(), itEnd = Role.m_pmlist.end(); itBegin != itEnd; ++itBegin)
        {
            LOG_INFO_RLD("Role func id is " << itBegin->m_uiFuncID << " and access is " << itBegin->m_strAccess);
        }

        LOG_INFO_RLD("Query role and session id is " << strSid << " and user id is " << strUserID << " and role id is " << strRoleID <<
            " and return code is " << QueryRoleRsp.m_iRetcode <<
            " and return msg is " << QueryRoleRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}


template<typename T>
bool PassengerFlowMsgHandler::QueryAllRole(const std::string &strSid, const std::string &strUserID, std::list<T> &RoleList)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::QueryAllRoleReq QueryAllRoleReq;
        QueryAllRoleReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllRoleReq_T;
        QueryAllRoleReq.m_uiMsgSeq = 1;
        QueryAllRoleReq.m_strSID = strSid;

        QueryAllRoleReq.m_strUserID = strUserID;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryAllRoleReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all role req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::QueryAllRoleRsp QueryAllRoleRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryAllRoleRsp))
        {
            LOG_ERROR_RLD("Query all role rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = QueryAllRoleRsp.m_iRetcode;

        RoleList.swap(QueryAllRoleRsp.m_rolelist);
        
        LOG_INFO_RLD("Query all role and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << QueryAllRoleRsp.m_iRetcode <<
            " and return msg is " << QueryAllRoleRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}


bool PassengerFlowMsgHandler::UserBindRole(const std::string &strSid, const std::string &strUserID, const std::string &strRoleID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        PassengerFlowProtoHandler::UserBindRoleReq UserBindRoleReq;
        UserBindRoleReq.m_MsgType = PassengerFlowProtoHandler::CustomerFlowMsgType::UserBindRoleReq_T;
        UserBindRoleReq.m_uiMsgSeq = 1;
        UserBindRoleReq.m_strSID = strSid;

        UserBindRoleReq.m_strUserID = strUserID;
        UserBindRoleReq.m_strRoleID = strRoleID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(UserBindRoleReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("User bind role req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        PassengerFlowProtoHandler::UserBindRoleRsp UserBindRoleRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, UserBindRoleRsp))
        {
            LOG_ERROR_RLD("User bind role rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = UserBindRoleRsp.m_iRetcode;

        LOG_INFO_RLD("User bind role and session id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << UserBindRoleRsp.m_iRetcode <<
            " and return msg is " << UserBindRoleRsp.m_strRetMsg);

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

bool PassengerFlowMsgHandler::CreateDomainHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("name");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Store name not found.");
        return blResult;
    }
    const std::string strDomainName = itFind->second;

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    itFind = pMsgInfoMap->find("level");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Level name not found.");
        return blResult;
    }
    const std::string strLevelName = itFind->second;

    if (!ValidNumber(strLevelName))
    {
        LOG_ERROR_RLD("Level name is not number.");
        return blResult;
    }

    std::string strParentDomainID;
    const unsigned int uiLevel = boost::lexical_cast<unsigned int>(strLevelName);
    if (1 != uiLevel)
    {
        itFind = pMsgInfoMap->find("parent_domainid");
        if (pMsgInfoMap->end() == itFind)
        {
            LOG_ERROR_RLD("Parent domain id not found.");
            return blResult;
        }

        strParentDomainID = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Create domain info received and session id is " << strSid << " and user id is " << strUserID << " and domain name is " << strDomainName
        << " and extend is " << strExtend << " and parent domain id is " << strParentDomainID << " and level is " << uiLevel);

    DomainInfo dmi;
    dmi.m_strDomainName = strDomainName;
    dmi.m_strExtend = strExtend;
    dmi.m_strParentDomainID = strParentDomainID;
    dmi.m_uiLevel = uiLevel;

    if (!CreateDomain(strSid, strUserID, dmi))
    {
        LOG_ERROR_RLD("Create domain handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("domainid", dmi.m_strDomainID));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::RemoveDomainHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("domainid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Domain id not found.");
        return blResult;
    }
    const std::string strDomainID = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Remove domain info received and session id is " << strSid << " and user id is " << strUserID << " and domain id is " << strDomainID);

    if (!RemoveDomain(strSid, strUserID, strDomainID))
    {
        LOG_ERROR_RLD("Remove domain handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::ModifyDomainHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("domainid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Domain id not found.");
        return blResult;
    }
    const std::string strDomainID = itFind->second;
    
    std::string strDomainName;
    itFind = pMsgInfoMap->find("name");
    if (pMsgInfoMap->end() != itFind)
    {
        strDomainName = itFind->second;
    }
    
    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify domain info received and session id is " << strSid << " and user id is " << strUserID << " and domain name is " << strDomainName
        << " and extend is " << strExtend);

    DomainInfo dmi;
    dmi.m_strDomainName = strDomainName;
    dmi.m_strExtend = strExtend;
    dmi.m_strDomainID = strDomainID;

    if (!ModifyDomain(strSid, strUserID, dmi))
    {
        LOG_ERROR_RLD("Modify domain handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool PassengerFlowMsgHandler::QueryDomainHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("domainid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Domain id not found.");
        return blResult;
    }
    const std::string strDomainID = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query domain info received and session id is " << strSid << " and user id is " << strUserID << " and domain id is " << strDomainID);

    DomainInfo dmi;

    if (!QueryDomain(strSid, strUserID, strDomainID, dmi))
    {
        LOG_ERROR_RLD("Query domain handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("parent_domainid", dmi.m_strParentDomainID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("domainid", dmi.m_strDomainID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("name", dmi.m_strDomainName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("extend", dmi.m_strExtend));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("level", boost::lexical_cast<std::string>(dmi.m_uiLevel)));

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;

}

bool PassengerFlowMsgHandler::QueryAllDomainHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsDomainInfoList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsDomainInfoList)
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
                (*pJsBody)["domain_info"] = jsDomainInfoList;

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
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query all domain info received and session id is " << strSid << " and user id is " << strUserID);

    std::list<DomainInfo> dmilist;

    if (!QueryAllDomain(strSid, strUserID, dmilist))
    {
        LOG_ERROR_RLD("Query all domain handle failed");
        return blResult;
    }

    auto itBegin = dmilist.begin();
    auto itEnd = dmilist.end();
    while (itBegin != itEnd)
    {
        Json::Value jsDmi;
        jsDmi["domainid"] = itBegin->m_strDomainID;
        jsDmi["name"] = itBegin->m_strDomainName;
        jsDmi["extend"] = itBegin->m_strExtend;
        jsDmi["parent_domainid"] = itBegin->m_strParentDomainID;
        jsDmi["level"] = boost::lexical_cast<std::string>(itBegin->m_uiLevel);

        jsDomainInfoList.append(jsDmi);

        ++itBegin;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
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

bool PassengerFlowMsgHandler::ValidNumber(const std::string &strNumber, const unsigned int uiCount)
{
    char rep[16] = { 0 };
    const char* repfmt = "([0-9]{%u})";
    snprintf(rep, sizeof(rep), repfmt, uiCount);
    std::string strRepFmt(rep);

    boost::regex reg0(strRepFmt.c_str()); //

    if (!boost::regex_match(strNumber, reg0))
    {
        LOG_ERROR_RLD("Number is invalid and input date is " << strNumber);
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

bool PassengerFlowMsgHandler::GetEntrance(const std::string &strValue, std::list<StoreAndEntranceInfo> &saelist)
{
    bool blResult = false;

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(strValue, root))
    {
        LOG_ERROR_RLD("Value info parse failed, raw data is : " << strValue);
        return blResult;
    }

    if (!root.isObject())
    {
        LOG_ERROR_RLD("Value info parse failed, raw data is : " << strValue);
        return blResult;
    }
        
    Json::Value::Members mem = root.getMemberNames();
    for (auto itBegin = mem.begin(), itEnd = mem.end(); itBegin != itEnd; ++itBegin)
    {
        auto &jsValue = root[*itBegin];
        if (jsValue.isNull() || !jsValue.isArray())
        {
            LOG_ERROR_RLD("Value info type is error, raw data is: " << strValue);
            return blResult;
        }
        StoreAndEntranceInfo sae;
        sae.stinfo.m_strStoreID = *itBegin;

        std::list<std::string> ValueList;
        for (unsigned int i = 0; i < jsValue.size(); ++i)
        {
            auto jsValueItem = jsValue[i];
            if (jsValueItem.isNull() || !jsValueItem.isString())
            {
                LOG_ERROR_RLD("Value info type is error, raw data is: " << strValue);
                return blResult;
            }

            EntranceInfo einfo;
            einfo.m_strID = jsValueItem.asString();
            sae.etinfolist.emplace_back(einfo);

            LOG_INFO_RLD("Value item is " << jsValueItem.asString());
        }

        saelist.push_back(std::move(sae));
    }

    blResult = true;

    return blResult;
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
