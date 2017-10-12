#include "PassengerFlowMsgHandler.h"
#include <boost/algorithm/string.hpp>  
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scope_exit.hpp>
#include "json/json.h"
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

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("chart_data", strChartData));
    
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
    
    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsEventTypeList, &jsEventHandlerList)
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
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("remark", einfo.m_strRemark));
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
        Json::Value jsEventInfo;
        jsEventInfo["event_id"] = itBegin->m_strEventID;
        jsEventInfo["source"] = itBegin->m_strSource;
        jsEventInfo["submit_date"] = itBegin->m_strSource;
        jsEventInfo["expire_date"] = itBegin->m_strExpireDate;
        jsEventInfo["process_state"] = itBegin->m_strProcessState;
        jsEventInfo["create_date"] = itBegin->m_strCreateDate;
        jsEventInfo["remark"] = itBegin->m_strRemark;        
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


bool PassengerFlowMsgHandler::ValidDatetime(const std::string &strDatetime)
{
    if (strDatetime.empty())
    {
        return false;
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
