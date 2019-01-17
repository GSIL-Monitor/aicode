#include "ProductHandler.h"
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



const std::string ProductHandler::SUCCESS_CODE = "0";
const std::string ProductHandler::SUCCESS_MSG = "Ok";
const std::string ProductHandler::FAILED_CODE = "-1";
const std::string ProductHandler::FAILED_MSG = "Inner failed";


const std::string ProductHandler::ADD_PRODUCT_TYPE_ACTION("add_product_type");

const std::string ProductHandler::REMOVE_PRODUCT_TYPE_ACTION("remove_product_type");

const std::string ProductHandler::MOD_PRODUCT_TYPE_ACTION("modify_product_type");

const std::string ProductHandler::QUERY_PRODUCT_TYPE_ACTION("query_product_type");

const std::string ProductHandler::QUERY_ALL_PRODUCT_TYPE_ACTION("query_all_product_type");

const std::string ProductHandler::ADD_OPEN_REQUEST_ACTION("add_open_request");

const std::string ProductHandler::REMOVE_OPEN_REQUEST_ACTION("remove_open_request");

const std::string ProductHandler::MOD_OPEN_REQUEST_ACTION("modify_open_request");

const std::string ProductHandler::QUERY_OPEN_REQUEST_ACTION("query_open_request");

const std::string ProductHandler::QUERY_ALL_OPEN_REQUEST_ACTION("query_all_open_request");

const std::string ProductHandler::ADD_PRODUCT_ACTION("add_product");

const std::string ProductHandler::REMOVE_PRODUCT_ACTION("remove_product");

const std::string ProductHandler::MOD_PRODUCT_ACTION("modify_product");

const std::string ProductHandler::QUERY_PRODUCT_ACTION("query_product");

const std::string ProductHandler::QUERY_ALL_PRODUCT_ACTION("query_all_product");


const std::string ProductHandler::ADD_PRODUCT_PROPERTY_ACTION("add_product_property");

const std::string ProductHandler::REMOVE_PRODUCT_PROPERTY_ACTION("remove_product_property");

const std::string ProductHandler::ADD_ORDER_ACTION("add_order");

const std::string ProductHandler::REMOVE_ORDER_ACTION("remove_order");

const std::string ProductHandler::MODIFY_ORDER_ACTION("modify_order");

const std::string ProductHandler::ADD_ORDER_DETAIL_ACTION("add_order_detail");

const std::string ProductHandler::REMOVE_ORDER_DETAIL_ACTION("remove_order_detail");

const std::string ProductHandler::QUERY_ORDER_ACTION("query_order");

const std::string ProductHandler::QUERY_ALL_ORDER_ACTION("query_all_order");

ProductHandler::ProductHandler(const ParamInfo &parminfo):
m_ParamInfo(parminfo)
{

}

ProductHandler::~ProductHandler()
{

}

bool ProductHandler::RspFuncCommonActionT(const std::string &strBinaryContent, unsigned int uiRetCode, int *piRetCode, RspFuncCommonT rspfunc)
{
    int iPreCode = 0;
    if (!PreCommonHandler(strBinaryContent, iPreCode))
    {
        ReturnInfo::RetCode(iPreCode);
        return false;
    }

    if (NULL == rspfunc)
    {
        LOG_ERROR_RLD("Rsp function is null");
        return false;
    }

    bool blRspfuncRet = rspfunc(strBinaryContent, uiRetCode);

    ReturnInfo::RetCode(*piRetCode);

    return blRspfuncRet;
}

bool ProductHandler::PreCommonHandler(const std::string &strMsgReceived, int &iRetCode)
{
    ////
    //ProductHandler::CustomerFlowMsgType mtype;
    //if (!m_pInteractiveProtoHandler->GetCustomerFlowMsgType(strMsgReceived, mtype))
    //{
    //    LOG_ERROR_RLD("Get msg type failed.");
    //    return false;
    //}

    //if (ProductHandler::CustomerFlowMsgType::CustomerFlowPreHandleRsp_T == mtype)
    //{
    //    ProductHandler::CustomerFlowPreHandleRsp rsp;
    //    if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, rsp))
    //    {
    //        LOG_ERROR_RLD("Msg prehandler rsp unserialize failed.");
    //        return false;
    //    }

    //    LOG_INFO_RLD("Msg prehandler rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
    //        " and user session id is " << rsp.m_strSID);

    //    if (CommMsgHandler::SUCCEED != (iRetCode = rsp.m_iRetcode))
    //    {
    //        LOG_ERROR_RLD("Msg prehandler rsp return failed.");
    //        return false;
    //    }
    //}

    return true;
}

bool ProductHandler::AddProductTypeHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Product type not found.");
        return blResult;
    }
    int iType = 0;
    if (!ValidType<int>(itFind->second, iType))
    {
        LOG_ERROR_RLD("Product type is error and input value is " << itFind->second);
        return blResult;
    }

    itFind = pMsgInfoMap->find("type_name");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Type name not found.");
        return blResult;
    }
    const std::string strTypeName = itFind->second;

    std::string strPic;
    itFind = pMsgInfoMap->find("pic");
    if (pMsgInfoMap->end() != itFind)
    {
        strPic = itFind->second;
    }

    itFind = pMsgInfoMap->find("index");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Product type index not found.");
        return blResult;
    }
    int iIndex = 0;
    if (!ValidType<int>(itFind->second, iIndex))
    {
        LOG_ERROR_RLD("Product type index is error and input value is " << itFind->second);
        return blResult;
    }

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Add product type info received and session id is " << strSid << " and user id is " << strUserID
        << " and type is " << iType << " and type name is " << strTypeName << " and index is " << iIndex
        << " and extend is " << strExtend << " and pic is " << strPic);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    ProductType pdttype;
    pdttype.__set_iIndex(iIndex);
    pdttype.__set_iType(iType);
    pdttype.__set_strExtend(strExtend);
    pdttype.__set_strPic(strPic);
    pdttype.__set_strTypeName(strTypeName);


    AddProductTypeRT adrt;
    pclient.AddProductType(adrt, strSid, strUserID, pdttype);

    if (adrt.rtcode.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Add product type call failed" << " and return code is " << adrt.rtcode.iRtCode);

        ReturnInfo::RetCode(adrt.rtcode.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("tid", adrt.strTypeID));

    blResult = true;

    return blResult;
}


bool ProductHandler::RemoveProductTypeHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("tid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Product type id not found.");
        return blResult;
    }
    const std::string strTypeID = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete product type info received and session id is " << strSid << " and user id is " << strUserID << " and product type id is " << strTypeID);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    ProductRTInfo pdtrt;
    pclient.RemoveProductType(pdtrt, strSid, strUserID, strTypeID);
    if (pdtrt.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Remove product type call failed" << " and return code is " << pdtrt.iRtCode);

        ReturnInfo::RetCode(pdtrt.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool ProductHandler::ModifyProductTypeHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("tid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Type id not found.");
        return blResult;
    }
    const std::string strTypeID = itFind->second;
    
    int iType = -1;
    itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidType<int>(itFind->second, iType))
        {
            LOG_ERROR_RLD("Product type is error and input value is " << itFind->second);
            return blResult;
        }        
    }
    
    std::string strTypeName;
    itFind = pMsgInfoMap->find("type_name");
    if (pMsgInfoMap->end() != itFind)
    {
        strTypeName = itFind->second;
    }
    
    std::string strPic;
    itFind = pMsgInfoMap->find("pic");
    if (pMsgInfoMap->end() != itFind)
    {
        strPic = itFind->second;
    }

    int iIndex = -1;
    itFind = pMsgInfoMap->find("index");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidType<int>(itFind->second, iIndex))
        {
            LOG_ERROR_RLD("Product type index is error and input value is " << itFind->second);
            return blResult;
        }        
    }
    
    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify product type info received and session id is " << strSid << " and user id is " << strUserID
        << " and type is " << iType << " and type name is " << strTypeName << " and index is " << iIndex
        << " and extend is " << strExtend << " and pic is " << strPic);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    ProductType pdttype;
    pdttype.__set_strPdtTpID(strTypeID);
    pdttype.__set_iIndex(iIndex);
    pdttype.__set_iType(iType);
    pdttype.__set_strExtend(strExtend);
    pdttype.__set_strPic(strPic);
    pdttype.__set_strTypeName(strTypeName);


    ProductRTInfo pdrt;
    pclient.ModifyProductType(pdrt, strSid, strUserID, pdttype);

    if (pdrt.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Modify product type call failed" << " and return code is " << pdrt.iRtCode);

        ReturnInfo::RetCode(pdrt.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool ProductHandler::QueryProductTypeHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("tid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Product type id not found.");
        return blResult;
    }
    const std::string strTypeID = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query product type info received and session id is " << strSid << " and user id is " << strUserID << " and product type id is " << strTypeID);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    QueryProductTypeRT qrypdtrt;
    pclient.QueryProductType(qrypdtrt, strSid, strUserID, strTypeID);
    if (qrypdtrt.rtcode.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Query product type call failed" << " and return code is " << qrypdtrt.rtcode.iRtCode);

        ReturnInfo::RetCode(qrypdtrt.rtcode.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("tid", qrypdtrt.pdttype.strPdtTpID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("type", boost::lexical_cast<std::string>(qrypdtrt.pdttype.iType)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("type_name", qrypdtrt.pdttype.strTypeName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("index", boost::lexical_cast<std::string>(qrypdtrt.pdttype.iIndex)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("extend", qrypdtrt.pdttype.strExtend));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("pic", qrypdtrt.pdttype.strPic));

    blResult = true;

    return blResult;
}

bool ProductHandler::QueryAllProductTypeHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsPdtTypeInfoList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsPdtTypeInfoList)
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
                (*pJsBody)["type_info"] = jsPdtTypeInfoList;
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

    LOG_INFO_RLD("Query all product type info received and session id is " << strSid << " and user id is " << strUserID);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    QueryAllProductTypeRT qryallpdtrt;
    pclient.QueryAllProductType(qryallpdtrt, strSid, strUserID);
    if (qryallpdtrt.rtcode.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Query all product type call failed" << " and return code is " << qryallpdtrt.rtcode.iRtCode);

        ReturnInfo::RetCode(qryallpdtrt.rtcode.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    for (auto itBegin = qryallpdtrt.pdttypelist.begin(), itEnd = qryallpdtrt.pdttypelist.end(); itBegin != itEnd; ++itBegin)
    {
        
        Json::Value jsProduct;
        jsProduct["tid"] = itBegin->strPdtTpID;        
        jsProduct["type"] = boost::lexical_cast<std::string>(itBegin->iType);
        jsProduct["type_name"] = itBegin->strTypeName;
        jsProduct["index"] = boost::lexical_cast<std::string>(itBegin->iIndex);
        jsProduct["extend"] = itBegin->strExtend;
        jsProduct["pic"] = itBegin->strPic;
        
        jsPdtTypeInfoList.append(jsProduct);
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool ProductHandler::AddOpenRequestHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("req_userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Req user id not found.");
        return blResult;
    }
    const std::string strReqUserID = itFind->second;

    itFind = pMsgInfoMap->find("req_username");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Req user name not found.");
        return blResult;
    }
    const std::string strReqUserName = itFind->second;

    itFind = pMsgInfoMap->find("req_info");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Req info not found.");
        return blResult;
    }
    const std::string strReqInfo = itFind->second;

    itFind = pMsgInfoMap->find("req_date");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Req date not found.");
        return blResult;
    }
    const std::string strReqDate = itFind->second;

    if (!ValidDatetime(strReqDate))
    {
        LOG_ERROR_RLD("Req date is invalid and input date is " << strReqDate);
        return blResult;
    }

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Add open request info received and session id is " << strSid << " and user id is " << strUserID
        << " and req user id is " << strReqUserID << " and req user name is " << strReqUserName << " and req info is " << strReqInfo
        << " and req date is " << strReqDate << " and extend is " << strExtend);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    OpenRequest opreq;
    opreq.__set_iReqStatus(1); //处理中状态
    opreq.__set_strExtend(strExtend);
    opreq.__set_strReqDate(strReqDate);
    opreq.__set_strReqInfo(strReqInfo);
    opreq.__set_strReqUserID(strReqUserID);
    opreq.__set_strReqUserName(strReqUserName);

    AddOpenRequestRT adrt;
    pclient.AddOpenRequest(adrt, strSid, strUserID, opreq);

    if (adrt.rtcode.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Add open request call failed" << " and return code is " << adrt.rtcode.iRtCode);

        ReturnInfo::RetCode(adrt.rtcode.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("reqid", adrt.strOpReqID));

    blResult = true;

    return blResult;
}

bool ProductHandler::RemoveOpenRequestHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("reqid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Request id not found.");
        return blResult;
    }
    const std::string strReqID = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete open request info received and session id is " << strSid << " and user id is " << strUserID << " and request id is " << strReqID);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    ProductRTInfo pdtrt;
    pclient.RemoveOpenRequest(pdtrt, strSid, strUserID, strReqID);
    if (pdtrt.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Remove open request call failed" << " and return code is " << pdtrt.iRtCode);

        ReturnInfo::RetCode(pdtrt.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool ProductHandler::ModifyOpenRequestHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("reqid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Request id not found.");
        return blResult;
    }
    const std::string strReqID = itFind->second;
    
    std::string strReqUserID;
    itFind = pMsgInfoMap->find("req_userid");
    if (pMsgInfoMap->end() != itFind)
    {
        strReqUserID = itFind->second;        
    }
    
    std::string strReqUserName;
    itFind = pMsgInfoMap->find("req_username");
    if (pMsgInfoMap->end() != itFind)
    {
        strReqUserName = itFind->second;
    }
    
    std::string strReqInfo;
    itFind = pMsgInfoMap->find("req_info");
    if (pMsgInfoMap->end() != itFind)
    {
        strReqInfo = itFind->second;
    }

    int iReqStatus = -1;
    itFind = pMsgInfoMap->find("status");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidType<int>(itFind->second, iReqStatus))
        {
            LOG_ERROR_RLD("Request status is error and input value is " << itFind->second);
            return blResult;
        }
    }
        
    itFind = pMsgInfoMap->find("req_date");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Req date not found.");
        return blResult;
    }
    const std::string strReqDate = itFind->second;

    if (!ValidDatetime(strReqDate))
    {
        LOG_ERROR_RLD("Req date is invalid and input date is " << strReqDate);
        return blResult;
    }

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify open request info received and session id is " << strSid << " and user id is " << strUserID
        << " and req user id is " << strReqUserID << " and req user name is " << strReqUserName << " and req info is " << strReqInfo
        << " and req date is " << strReqDate << " and extend is " << strExtend);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    OpenRequest opreq;
    opreq.__set_strOpReqID(strReqID);
    opreq.__set_iReqStatus(iReqStatus);
    opreq.__set_strExtend(strExtend);
    opreq.__set_strReqDate(strReqDate);
    opreq.__set_strReqInfo(strReqInfo);
    opreq.__set_strReqUserID(strReqUserID);
    opreq.__set_strReqUserName(strReqUserName);

    ProductRTInfo adrt;
    pclient.ModifyOpenRequest(adrt, strSid, strUserID, opreq);

    if (adrt.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Modify open request call failed" << " and return code is " << adrt.iRtCode);

        ReturnInfo::RetCode(adrt.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool ProductHandler::QueryOpenRequestHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsOpReqInfoList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsOpReqInfoList)
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
                (*pJsBody)["req_info"] = jsOpReqInfoList;

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

    std::string strReqID;
    itFind = pMsgInfoMap->find("reqid");
    if (pMsgInfoMap->end() != itFind)
    {
        strReqID = itFind->second;
    }
    
    std::string strReqUserID;
    itFind = pMsgInfoMap->find("req_userid");
    if (pMsgInfoMap->end() != itFind)
    {
        strReqUserID = itFind->second;
    }

    std::string strReqUserName;
    itFind = pMsgInfoMap->find("req_username");
    if (pMsgInfoMap->end() != itFind)
    {
        strReqUserName = itFind->second;
    }

    if (strReqID.empty() && strReqUserID.empty() && strReqUserName.empty())
    {
        LOG_ERROR_RLD("Request id and user id and user name all empty.");
        return blResult;
    }


    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query open request info received and session id is " << strSid << " and user id is " << strUserID << " and request id is " << strReqID
        << " and request user id is " << strReqUserID << " and request user name is " << strReqUserName);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    QueryOpenRequestRT qrypdtrt;
    pclient.QueryOpenRequest(qrypdtrt, strSid, strUserID, strReqID, strReqUserID, strReqUserName);
    if (qrypdtrt.rtcode.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Query open request call failed" << " and return code is " << qrypdtrt.rtcode.iRtCode);

        ReturnInfo::RetCode(qrypdtrt.rtcode.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    for (auto itBegin = qrypdtrt.opreqlist.begin(), itEnd = qrypdtrt.opreqlist.end(); itBegin != itEnd; ++itBegin)
    {
        Json::Value jsReq;
        jsReq["reqid"] = itBegin->strOpReqID;
        jsReq["info"] = itBegin->strReqInfo;
        jsReq["status"] = boost::lexical_cast<std::string>(itBegin->iReqStatus);
        jsReq["req_date"] = itBegin->strReqDate;
        jsReq["extend"] = itBegin->strExtend;

        jsOpReqInfoList.append(jsReq);
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("req_userid", qrypdtrt.strReqUserID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("req_username", qrypdtrt.strReqUserName));

    blResult = true;

    return blResult;
}

bool ProductHandler::QueryAllOpenRequestHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsOpReqInfoList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsOpReqInfoList)
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
                (*pJsBody)["all_req_info"] = jsOpReqInfoList;

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

    int iReqStatus = -1;
    itFind = pMsgInfoMap->find("status");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidType<int>(itFind->second, iReqStatus))
        {
            LOG_ERROR_RLD("Request status is error and input value is " << itFind->second);
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

    unsigned int uiBeginIndex = 0;
    itFind = pMsgInfoMap->find("beginindex");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidType<unsigned int>(itFind->second, uiBeginIndex))
        {
            LOG_ERROR_RLD("Begin index is error and input value is " << itFind->second);
            return blResult;
        }
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query all open request info received and session id is " << strSid << " and user id is " << strUserID << " and status is " << iReqStatus
        << " and begin date is " << strBeginDate << " and end date is " << strEndDate << " and index is " << uiBeginIndex);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    QueryAllOpenRequestParam param;
    param.__set_iReqStatus(iReqStatus);
    param.__set_strBeginDate(strBeginDate);
    param.__set_strBeginIndex(boost::lexical_cast<std::string>(uiBeginIndex));
    param.__set_strEndDate(strEndDate);

    QueryAllOpenRequestRT qrypdtrt;
    pclient.QueryAllOpenRequest(qrypdtrt, strSid, strUserID, param);
    if (qrypdtrt.rtcode.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Query all open request call failed" << " and return code is " << qrypdtrt.rtcode.iRtCode);

        ReturnInfo::RetCode(qrypdtrt.rtcode.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    for (auto itBegin = qrypdtrt.opreqlist.begin(), itEnd = qrypdtrt.opreqlist.end(); itBegin != itEnd; ++itBegin)
    {
        Json::Value jsReq;
        jsReq["reqid"] = itBegin->strOpReqID;
        jsReq["info"] = itBegin->strReqInfo;
        jsReq["status"] = boost::lexical_cast<std::string>(itBegin->iReqStatus);
        jsReq["req_date"] = itBegin->strReqDate;
        jsReq["extend"] = itBegin->strExtend;
        jsReq["req_userid"] = itBegin->strReqUserID;
        jsReq["req_username"] = itBegin->strReqUserName;

        jsOpReqInfoList.append(jsReq);
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool ProductHandler::AddProductHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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
        LOG_ERROR_RLD("Product name not found.");
        return blResult;
    }
    const std::string strProductName = itFind->second;
        
    itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Product type not found.");
        return blResult;
    }
    int iType = 0;
    if (!ValidType<int>(itFind->second, iType))
    {
        LOG_ERROR_RLD("Product type is error and input value is " << itFind->second);
        return blResult;
    }

    itFind = pMsgInfoMap->find("type_name");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Type name not found.");
        return blResult;
    }
    const std::string strTypeName = itFind->second;

    itFind = pMsgInfoMap->find("price");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Product price not found.");
        return blResult;
    }
    double dlPrice = 0;
    if (!ValidType<double>(itFind->second, dlPrice))
    {
        LOG_ERROR_RLD("Product price is error and input value is " << itFind->second);
        return blResult;
    }
    
    std::string strPic;
    itFind = pMsgInfoMap->find("pic");
    if (pMsgInfoMap->end() != itFind)
    {
        strPic = itFind->second;
    }

    std::string strAliasName;
    itFind = pMsgInfoMap->find("aliasname");
    if (pMsgInfoMap->end() != itFind)
    {
        strAliasName = itFind->second;
    }

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Add product info received and session id is " << strSid << " and user id is " << strUserID << " and product name is " << strProductName 
        << " and type is " << iType << " and type name is " << strTypeName << " and price id is " << dlPrice
        << " and extend is " << strExtend << " and pic is " << strPic << " and alias name is " << strAliasName);
        
    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }
        
    ProductInfo pdt;
    pdt.__set_dlPrice(dlPrice);
    pdt.__set_iType(iType);
    pdt.__set_strTypeName(strTypeName);
    pdt.__set_strAliasName(strAliasName);
    pdt.__set_strExtend(strExtend);
    pdt.__set_strName(strProductName);
    pdt.__set_strPic(strPic);
    
    AddProductRT adrt;
    pclient.AddProduct(adrt, strSid, strUserID, pdt);

    if (adrt.rtcode.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Add product call failed" << " and return code is " << adrt.rtcode.iRtCode);

        ReturnInfo::RetCode(adrt.rtcode.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("pdtid", adrt.strPdtID));

    blResult = true;

    return blResult;
}

bool ProductHandler::RemoveProductHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("pdtid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Product id not found.");
        return blResult;
    }
    const std::string strPdtID = itFind->second;
        
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete product info received and session id is " << strSid << " and user id is " << strUserID << " and product id is " << strPdtID);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }
        
    ProductRTInfo pdtrt;
    pclient.RemoveProduct(pdtrt, strSid, strUserID, strPdtID);
    if (pdtrt.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Remove product call failed" << " and return code is " << pdtrt.iRtCode);

        ReturnInfo::RetCode(pdtrt.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool ProductHandler::ModifyProductHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("pdtid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Product id not found.");
        return blResult;
    }
    const std::string strPdtID = itFind->second;

    std::string strProductName;
    itFind = pMsgInfoMap->find("name");
    if (pMsgInfoMap->end() != itFind)
    {
        strProductName = itFind->second;
    }

    int iType = -1;
    itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidType<int>(itFind->second, iType))
        {
            LOG_ERROR_RLD("Product type is error and input value is " << itFind->second);
            return blResult;
        }
    }

    std::string strTypeName;
    itFind = pMsgInfoMap->find("type_name");
    if (pMsgInfoMap->end() != itFind)
    {
        strTypeName = itFind->second;
    }
    
    double dlPrice = 0xFFFFFFFF;
    itFind = pMsgInfoMap->find("price");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidType<double>(itFind->second, dlPrice))
        {
            LOG_ERROR_RLD("Product price is error and input value is " << itFind->second);
            return blResult;
        }
    }
    
    std::string strPic;
    itFind = pMsgInfoMap->find("pic");
    if (pMsgInfoMap->end() != itFind)
    {
        strPic = itFind->second;
    }

    std::string strAliasName;
    itFind = pMsgInfoMap->find("aliasname");
    if (pMsgInfoMap->end() != itFind)
    {
        strAliasName = itFind->second;
    }

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify product info received and session id is " << strSid << " and user id is " << strUserID << " and product name is " << strProductName
        << " and type is " << iType << " and type name is " << strTypeName << " and price id is " << dlPrice
        << " and extend is " << strExtend << " and pic is " << strPic << " and alias name is " << strAliasName);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    ProductInfo pdt;
    pdt.__set_dlPrice(dlPrice);
    pdt.__set_iType(iType);
    pdt.__set_strTypeName(strTypeName);
    pdt.__set_strAliasName(strAliasName);
    pdt.__set_strExtend(strExtend);
    pdt.__set_strName(strProductName);
    pdt.__set_strPic(strPic);

    ProductRTInfo pdtrt;
    pclient.ModifyProduct(pdtrt, strSid, strUserID, strPdtID, pdt);
    if (pdtrt.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Modify product call failed" << " and return code is " << pdtrt.iRtCode);

        ReturnInfo::RetCode(pdtrt.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    
    blResult = true;

    return blResult;
}

bool ProductHandler::QueryProductHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsPropertyList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsPropertyList)
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
                (*pJsBody)["property"] = jsPropertyList;
               
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

    itFind = pMsgInfoMap->find("pdtid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Product id not found.");
        return blResult;
    }
    const std::string strPdtID = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query product info received and session id is " << strSid << " and user id is " << strUserID << " and product id is " << strPdtID);
    
    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    QueryProductRT qrypdtrt;
    pclient.QueryProduct(qrypdtrt, strSid, strUserID, strPdtID);
    if (qrypdtrt.rtcode.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Query product call failed" << " and return code is " << qrypdtrt.rtcode.iRtCode);

        ReturnInfo::RetCode(qrypdtrt.rtcode.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    for (auto itBegin = qrypdtrt.pdt.pptList.begin(), itEnd = qrypdtrt.pdt.pptList.end(); itBegin != itEnd; ++itBegin)
    {
        Json::Value jsProperty;
        jsProperty["pdtpptid"] = itBegin->strID;
        jsProperty["p_value"] = itBegin->strValue;
        jsProperty["p_type"] = boost::lexical_cast<std::string>(itBegin->iType);
        jsProperty["p_name"] = itBegin->strName;
        jsProperty["p_extend"] = itBegin->strExtend;

        jsPropertyList.append(jsProperty);
    }

    char cPrice[1024] = { 0 };
    snprintf(cPrice, sizeof(cPrice), "%.2f", qrypdtrt.pdt.dlPrice);
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("pdtid", qrypdtrt.pdt.strID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("name", qrypdtrt.pdt.strName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("type", boost::lexical_cast<std::string>(qrypdtrt.pdt.iType)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("type_name", qrypdtrt.pdt.strTypeName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("aliasname", qrypdtrt.pdt.strAliasName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("price", std::string(cPrice)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("extend", qrypdtrt.pdt.strExtend));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("pic", qrypdtrt.pdt.strPic));
        
    blResult = true;

    return blResult;

}

bool ProductHandler::QueryAllProductHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsProductInfoList;
    
    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsProductInfoList)
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
                (*pJsBody)["product_info"] = jsProductInfoList;
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

    LOG_INFO_RLD("Query all product info received and session id is " << strSid << " and user id is " << strUserID);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    QueryAllProductRT qryallpdtrt;
    pclient.QueryAllProduct(qryallpdtrt, strSid, strUserID);
    if (qryallpdtrt.rtcode.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Query all product call failed" << " and return code is " << qryallpdtrt.rtcode.iRtCode);

        ReturnInfo::RetCode(qryallpdtrt.rtcode.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();
    
    for (auto itBegin = qryallpdtrt.pdtlist.begin(), itEnd = qryallpdtrt.pdtlist.end(); itBegin != itEnd; ++itBegin)
    {
        char cPrice[1024] = { 0 };
        snprintf(cPrice, sizeof(cPrice), "%.2f", itBegin->dlPrice);

        Json::Value jsProduct;
        jsProduct["pdtid"] = itBegin->strID;
        jsProduct["name"] = itBegin->strName;
        jsProduct["type"] = boost::lexical_cast<std::string>(itBegin->iType);
        jsProduct["type_name"] = itBegin->strTypeName;
        jsProduct["aliasname"] = itBegin->strAliasName;
        jsProduct["price"] = std::string(cPrice);
        jsProduct["extend"] = itBegin->strExtend;
        jsProduct["pic"] = itBegin->strPic;

        Json::Value jsPropertyList;
        for (auto itB1 = itBegin->pptList.begin(), itE1 = itBegin->pptList.end(); itB1 != itE1; ++itB1)
        {
            Json::Value jsProperty;
            jsProperty["pdtpptid"] = itB1->strID;
            jsProperty["p_value"] = itB1->strValue;
            jsProperty["p_type"] = boost::lexical_cast<std::string>(itB1->iType);
            jsProperty["p_name"] = itB1->strName;
            jsProperty["p_extend"] = itB1->strExtend;

            jsPropertyList.append(jsProperty);
        }
        jsProduct["property"] = jsPropertyList;

        jsProductInfoList.append(jsProduct);
    }
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}



bool ProductHandler::AddProductPropertyHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("pdtid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Product id not found.");
        return blResult;
    }
    const std::string strPdtID = itFind->second;
    
    itFind = pMsgInfoMap->find("name");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Product property name not found.");
        return blResult;
    }
    const std::string strPdtpptName = itFind->second;

    itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Product property type not found.");
        return blResult;
    }
    int iType = 0;
    if (!ValidType<int>(itFind->second, iType))
    {
        LOG_ERROR_RLD("Product property type is error and input value is " << itFind->second);
        return blResult;
    }

    itFind = pMsgInfoMap->find("value");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Product property value not found.");
        return blResult;
    }
    const std::string strValue = itFind->second;

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Add product property info received and session id is " << strSid << " and user id is " << strUserID << " and product property name is " << strPdtpptName
        << " and type is " << iType << " and value is " << strValue
        << " and extend is " << strExtend);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    ProductProperty pdtppt;
    pdtppt.__set_strName(strPdtpptName);
    pdtppt.__set_iType(iType);
    pdtppt.__set_strPdtID(strPdtID);
    pdtppt.__set_strExtend(strExtend);
    pdtppt.__set_strValue(strValue);
    
    AddProductPropertyRT adrt;
    pclient.AddProductProperty(adrt, strSid, strUserID, strPdtID, pdtppt);
    if (adrt.rtcode.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Add product property call failed" << " and return code is " << adrt.rtcode.iRtCode);

        ReturnInfo::RetCode(adrt.rtcode.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("pdtpptid", adrt.strPdtpptID));

    blResult = true;

    return blResult;
}

bool ProductHandler::RemoveProductPropertyHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("pdtid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Product id not found.");
        return blResult;
    }
    const std::string strPdtID = itFind->second;

    std::string strPdtpptID;
    itFind = pMsgInfoMap->find("pdtpptid");
    if (pMsgInfoMap->end() != itFind)
    {
        strPdtpptID = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete product property info received and session id is " << strSid << " and user id is " << strUserID << " and product id is " << strPdtID
        << " and property id is " << strPdtpptID);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    ProductRTInfo pdtrt;
    pclient.RemoveProductProperty(pdtrt, strSid, strUserID, strPdtID, strPdtpptID);
    if (pdtrt.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Remove product property call failed" << " and return code is " << pdtrt.iRtCode);

        ReturnInfo::RetCode(pdtrt.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}


bool ProductHandler::AddOrderHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("order_userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Order user id not found.");
        return blResult;
    }
    const std::string strOrderUserID = itFind->second;

    itFind = pMsgInfoMap->find("name");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Order name not found.");
        return blResult;
    }
    const std::string strOrdName = itFind->second;

    itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Order type not found.");
        return blResult;
    }
    int iType = 0;
    if (!ValidType<int>(itFind->second, iType))
    {
        LOG_ERROR_RLD("Order type is error and input value is " << itFind->second);
        return blResult;
    }

    itFind = pMsgInfoMap->find("ordstatus");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Order status not found.");
        return blResult;
    }
    int iOrdStatus = 0;
    if (!ValidType<int>(itFind->second, iOrdStatus))
    {
        LOG_ERROR_RLD("Order status is error and input value is " << itFind->second);
        return blResult;
    }

    std::string strExpressInfo;
    itFind = pMsgInfoMap->find("express_info");
    if (pMsgInfoMap->end() != itFind)
    {
        strExpressInfo = itFind->second;
    }
    
    itFind = pMsgInfoMap->find("address");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Address not found.");
        return blResult;
    }
    const std::string strAddress = itFind->second;

    itFind = pMsgInfoMap->find("receiver");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Receiver not found.");
        return blResult;
    }
    const std::string strReceiver = itFind->second;

    itFind = pMsgInfoMap->find("phone");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Phone not found.");
        return blResult;
    }
    const std::string strPhone = itFind->second;

    itFind = pMsgInfoMap->find("create_date");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Create date not found.");
        return blResult;
    }
    const std::string strCreateDate = itFind->second;

    if (!ValidDatetime(strCreateDate))
    {
        LOG_ERROR_RLD("Create date is invalid and value is " << strCreateDate);
        return blResult;
    }
    
    std::list<OrderDetail> OrddtList;
    std::string strOrderDetail;
    itFind = pMsgInfoMap->find("order_detail");
    if (pMsgInfoMap->end() != itFind)
    {
        strOrderDetail = itFind->second;

        auto OrdDtParseFunc = [&](Json::Value &jsValue) -> bool
        {
            if (!jsValue.isObject())
            {
                LOG_ERROR_RLD("Order detail parse failed");
                return false;
            }

            OrderDetail orddt;

            auto jsPtdID = jsValue["pdtid"];
            if (!jsPtdID.isNull())
            {
                if (!jsPtdID.isString() || jsPtdID.asString().empty())
                {
                    LOG_ERROR_RLD("Pdt id parse failed");
                    return false;
                }

                orddt.__set_strPdtID(jsPtdID.asString());
            }
            else
            {
                LOG_ERROR_RLD("Pdt id not found.");
                return false;
            }

            auto jsNumber = jsValue["number"];
            if (!jsNumber.isNull())
            {
                if (!jsNumber.isString() || jsNumber.asString().empty())
                {
                    LOG_ERROR_RLD("Number parse failed");
                    return false;
                }

                int iNumber = 0;
                if (!ValidType<int>(jsNumber.asString(), iNumber))
                {
                    LOG_ERROR_RLD("Number is invalid and value is " << jsNumber.asString());
                    return false;
                }

                if (0 >= iNumber)
                {
                    LOG_ERROR_RLD("Number is invalid and value is " << iNumber);
                    return blResult;
                }

                orddt.__set_iNumber(iNumber);
            }
            else
            {
                LOG_ERROR_RLD("Number not found.");
                return false;
            }

            auto jsPrice = jsValue["price"];
            if (!jsPrice.isNull())
            {
                if (!jsPrice.isString() || jsPrice.asString().empty())
                {
                    LOG_ERROR_RLD("Price parse failed");
                    return false;
                }

                double dlPrice;
                if (!ValidType<double>(jsPrice.asString(), dlPrice))
                {
                    LOG_ERROR_RLD("Price is invalid and value is " << jsPrice.asString());
                    return false;
                }

                if (0 >= dlPrice)
                {
                    LOG_ERROR_RLD("Price is invalid and value is " << dlPrice);
                    return blResult;
                }

                orddt.__set_dlPrice(dlPrice);
            }
            else
            {
                LOG_ERROR_RLD("Price not found.");
                return false;
            }

            double dlTotalPrice = 0xFFFFFFFF;
            auto jsTotalPrice = jsValue["total_price"];
            if (!jsTotalPrice.isNull())
            {
                if (!jsTotalPrice.isString() || jsTotalPrice.asString().empty())
                {
                    LOG_ERROR_RLD("Total price parse failed");
                    return false;
                }

                if (!ValidType<double>(jsTotalPrice.asString(), dlTotalPrice))
                {
                    LOG_ERROR_RLD("Total price is invalid and value is " << jsTotalPrice.asString());
                    return false;
                }
            }
            orddt.__set_dlTotalPrice(dlTotalPrice);

            std::string strExtend;
            auto jsExtend = jsValue["extend"];
            if (!jsExtend.isNull())
            {
                if (!jsExtend.isString() || jsExtend.asString().empty())
                {
                    LOG_ERROR_RLD("Extend parse failed");
                    return false;
                }

                strExtend = jsExtend.asString();
            }
            orddt.__set_strExtend(strExtend);


            OrddtList.push_back(std::move(orddt));

            return true;
        };

        if (!GetValueFromList(strOrderDetail, OrdDtParseFunc))
        {
            LOG_ERROR_RLD("Order detail parse failed and value is " << strOrderDetail);
            return blResult;
        }
    }

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Add order info received and session id is " << strSid << " and user id is " << strUserID << " and order user id is " << strOrderUserID 
        << " and order name is " << strOrdName
        << " and type is " << iType << " and order status is " << iOrdStatus << " and express info is " << strExpressInfo << " and address is " << strAddress
        << " and receiver is " << strReceiver << " and phone is " << strPhone << " and create date is " << strCreateDate << " and extend is " << strExtend
        << " and order detail is " << strOrderDetail);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    OrderInfo ord;
    ord.__set_strName(strOrdName);
    ord.__set_iType(iType);
    ord.__set_iOrdStatus(iOrdStatus);
    ord.__set_strExpressInfo(strExpressInfo);
    ord.__set_strAddress(strAddress);
    ord.__set_strReceiver(strReceiver);
    ord.__set_strPhone(strPhone);
    ord.__set_strCreateDate(strCreateDate);
    ord.__set_strExtend(strExtend);
    ord.__set_strUserID(strOrderUserID);

    AddOrdRT adrt;
    pclient.AddOrd(adrt, strSid, strUserID, ord);

    if (adrt.rtcode.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Add order call failed" << " and return code is " << adrt.rtcode.iRtCode);

        ReturnInfo::RetCode(adrt.rtcode.iRtCode);
        pclient.Close();
        return blResult;
    }

    //pclient.Close();

    ///////////////////////////
    for (auto &ordinfo : OrddtList)
    {
        ordinfo.__set_strOrdID(adrt.strOrdID);

        AddOrdDetailRT adrt;
        pclient.AddOrdDetail(adrt, strSid, strUserID, ordinfo);
        if (adrt.rtcode.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
        {
            LOG_ERROR_RLD("Add order detail call failed" << " and return code is " << adrt.rtcode.iRtCode);

            ReturnInfo::RetCode(adrt.rtcode.iRtCode);
            pclient.Close();
            return blResult;
        }

        LOG_INFO_RLD("Add order detail success and order detail id is " << adrt.strOrddtID);
    }

    pclient.Close();

    ////////////////////////////

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("ordid", adrt.strOrdID));

    blResult = true;

    return blResult;
}

bool ProductHandler::RemoveOrderHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("ordid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Order id not found.");
        return blResult;
    }
    const std::string strOrdID = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete order info received and session id is " << strSid << " and user id is " << strUserID << " and order id is " << strOrdID);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    ProductRTInfo pdtrt;
    pclient.RemoveOrd(pdtrt, strSid, strUserID, strOrdID);
    if (pdtrt.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Remove order call failed" << " and return code is " << pdtrt.iRtCode);

        ReturnInfo::RetCode(pdtrt.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool ProductHandler::ModifyOrderHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("ordid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Order id not found.");
        return blResult;
    }
    const std::string strOrdID = itFind->second;
    
    std::string strOrderUserID;
    itFind = pMsgInfoMap->find("order_userid");
    if (pMsgInfoMap->end() != itFind)
    {
        strOrderUserID = itFind->second;
    }

    std::string strOrdName;
    itFind = pMsgInfoMap->find("name");
    if (pMsgInfoMap->end() != itFind)
    {
        strOrdName = itFind->second;
    }
    
    int iType = -1;
    itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidType<int>(itFind->second, iType))
        {
            LOG_ERROR_RLD("Order type is error and input value is " << itFind->second);
            return blResult;
        }
    }

    int iOrdStatus = -1;
    itFind = pMsgInfoMap->find("ordstatus");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidType<int>(itFind->second, iOrdStatus))
        {
            LOG_ERROR_RLD("Order status is error and input value is " << itFind->second);
            return blResult;
        }
    }

    std::string strExpressInfo;
    itFind = pMsgInfoMap->find("express_info");
    if (pMsgInfoMap->end() != itFind)
    {
        strExpressInfo = itFind->second;
    }

    std::string strAddress;
    itFind = pMsgInfoMap->find("address");
    if (pMsgInfoMap->end() != itFind)
    {
        strAddress = itFind->second;
    }
    
    std::string strReceiver;
    itFind = pMsgInfoMap->find("receiver");
    if (pMsgInfoMap->end() != itFind)
    {
        strReceiver = itFind->second;
    }
    
    std::string strPhone;
    itFind = pMsgInfoMap->find("phone");
    if (pMsgInfoMap->end() != itFind)
    {
        strPhone = itFind->second;
    }
    
    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify order info received and session id is " << strSid << " and user id is " << strUserID << " and order user id is " << strOrderUserID
        << " and order id is " << strOrdID 
        << " and order name is " << strOrdName << " and type is " << iType << " and order status is " << iOrdStatus << " and express info is " << strExpressInfo 
        << " and address is " << strAddress << " and receiver is " << strReceiver << " and phone is " << strPhone << " and extend is " << strExtend);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    OrderInfo ord;
    ord.__set_strName(strOrdName);
    ord.__set_iType(iType);
    ord.__set_iOrdStatus(iOrdStatus);
    ord.__set_strExpressInfo(strExpressInfo);
    ord.__set_strAddress(strAddress);
    ord.__set_strReceiver(strReceiver);
    ord.__set_strPhone(strPhone);
    //ord.__set_strCreateDate(strCreateDate);
    ord.__set_strExtend(strExtend);
    ord.__set_strUserID(strOrderUserID);

    ord.__set_strID(strOrdID);

    ProductRTInfo pdtrt;
    pclient.ModifyOrd(pdtrt, strSid, strUserID, strOrdID, ord);

    if (pdtrt.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Modify order call failed" << " and return code is " << pdtrt.iRtCode);

        ReturnInfo::RetCode(pdtrt.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool ProductHandler::QueryOrderHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsOrderDetailList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsOrderDetailList)
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
                (*pJsBody)["order_detail"] = jsOrderDetailList;

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

    itFind = pMsgInfoMap->find("ordid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Order id not found.");
        return blResult;
    }
    const std::string strOrdID = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query order info received and session id is " << strSid << " and user id is " << strUserID << " and order id is " << strOrdID);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    QueryOrdRT qryordrt;
    pclient.QueryOrd(qryordrt, strSid, strUserID, strOrdID);
    if (qryordrt.rtcode.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Query order call failed" << " and return code is " << qryordrt.rtcode.iRtCode);

        ReturnInfo::RetCode(qryordrt.rtcode.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    for (auto itBegin = qryordrt.ord.orddtList.begin(), itEnd = qryordrt.ord.orddtList.end(); itBegin != itEnd; ++itBegin)
    {
        char cPrice[1024] = { 0 };
        snprintf(cPrice, sizeof(cPrice), "%.2f", itBegin->dlPrice);

        char cPriceTotal[1024] = { 0 };
        snprintf(cPriceTotal, sizeof(cPriceTotal), "%.2f", itBegin->dlTotalPrice);
        
        Json::Value jsOrderDetail;
        jsOrderDetail["orddtid"] = itBegin->strID;
        jsOrderDetail["pdtid"] = itBegin->strPdtID;
        jsOrderDetail["number"] = boost::lexical_cast<std::string>(itBegin->iNumber);
        jsOrderDetail["price"] = std::string(cPrice);
        jsOrderDetail["totalprice"] = std::string(cPriceTotal);
        jsOrderDetail["extend"] = itBegin->strExtend;

        jsOrderDetailList.append(jsOrderDetail);
    }

    char cTotalPrice[1024] = { 0 };
    snprintf(cTotalPrice, sizeof(cTotalPrice), "%.2f", qryordrt.ord.dlTotalPrice);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("ordid", qryordrt.ord.strID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("name", qryordrt.ord.strName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("userid", qryordrt.ord.strUserID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("type", boost::lexical_cast<std::string>(qryordrt.ord.iType)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("ordstatus", boost::lexical_cast<std::string>(qryordrt.ord.iOrdStatus)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("totalprice", std::string(cTotalPrice)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("express_info", qryordrt.ord.strExpressInfo));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("address", qryordrt.ord.strAddress));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("receiver", qryordrt.ord.strReceiver));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("phone", qryordrt.ord.strPhone));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("create_date", qryordrt.ord.strCreateDate));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("extend", qryordrt.ord.strExtend));

    blResult = true;

    return blResult;
}

bool ProductHandler::QueryAllOrderHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsOrderInfoList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsOrderInfoList)
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
                (*pJsBody)["order_info"] = jsOrderInfoList;
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

    std::string strOrderUserID;
    itFind = pMsgInfoMap->find("order_userid");
    if (pMsgInfoMap->end() != itFind)
    {
        strOrderUserID = itFind->second;
    }

    int iType = -1;
    itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidType<int>(itFind->second, iType))
        {
            LOG_ERROR_RLD("Order type is error and input value is " << itFind->second);
            return blResult;
        }
    }

    int iOrdStatus = -1;
    itFind = pMsgInfoMap->find("ordstatus");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidType<int>(itFind->second, iOrdStatus))
        {
            LOG_ERROR_RLD("Order status is error and input value is " << itFind->second);
            return blResult;
        }
    }

    std::string strReceiver;
    itFind = pMsgInfoMap->find("receiver");
    if (pMsgInfoMap->end() != itFind)
    {
        strReceiver = itFind->second;
    }

    std::string strPhone;
    itFind = pMsgInfoMap->find("phone");
    if (pMsgInfoMap->end() != itFind)
    {
        strPhone = itFind->second;
    }

    std::string strPdtID;
    itFind = pMsgInfoMap->find("pdtid");
    if (pMsgInfoMap->end() != itFind)
    {
        strPdtID = itFind->second;
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

    LOG_INFO_RLD("Query all order info received and session id is " << strSid << " and user id is " << strUserID << " and order user id is " << strOrderUserID << 
            " and type is " << iType << " and order status is " << iOrdStatus << " and receiver is " << strReceiver << " and phone is " << strPhone <<
            " and product id is " << strPdtID << " and begin date is " << strBeginDate << " and end date is " << strEndDate << " and begin index is " << uiBeginIndex);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    QueryAllOrdParam qpam;
    qpam.__set_iOrdStatus(iOrdStatus);
    qpam.__set_iType(iType);
    qpam.__set_strBeginDate(strBeginDate);
    qpam.__set_strBeginIndex(boost::lexical_cast<std::string>(uiBeginIndex));
    qpam.__set_strEndDate(strEndDate);
    qpam.__set_strPdtID(strPdtID);
    qpam.__set_strPhone(strPhone);
    qpam.__set_strReceiver(strReceiver);

    QueryAllOrdRT qryallordrt;
    pclient.QueryAllOrd(qryallordrt, strSid, strUserID, qpam);
    if (qryallordrt.rtcode.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Query all order call failed" << " and return code is " << qryallordrt.rtcode.iRtCode);

        ReturnInfo::RetCode(qryallordrt.rtcode.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    for (auto itBegin = qryallordrt.ordlist.begin(), itEnd = qryallordrt.ordlist.end(); itBegin != itEnd; ++itBegin)
    {
        char cTotalPrice[1024] = { 0 };
        snprintf(cTotalPrice, sizeof(cTotalPrice), "%.2f", itBegin->dlTotalPrice);

        Json::Value jsOrder;
        jsOrder["ordid"] = itBegin->strID;
        jsOrder["name"] = itBegin->strName;
        jsOrder["type"] = boost::lexical_cast<std::string>(itBegin->iType);
        jsOrder["userid"] = itBegin->strUserID;
        jsOrder["ordstatus"] = boost::lexical_cast<std::string>(itBegin->iOrdStatus);
        jsOrder["totalprice"] = std::string(cTotalPrice);
        jsOrder["express_info"] = itBegin->strExpressInfo;
        jsOrder["address"] = itBegin->strAddress;
        jsOrder["receiver"] = itBegin->strReceiver;
        jsOrder["phone"] = itBegin->strPhone;
        jsOrder["create_date"] = itBegin->strCreateDate;
        jsOrder["extend"] = itBegin->strExtend;

        Json::Value jsOrderDetailList;
        for (auto itB1 = itBegin->orddtList.begin(), itE1 = itBegin->orddtList.end(); itB1 != itE1; ++itB1)
        {
            char cPrice[1024] = { 0 };
            snprintf(cPrice, sizeof(cPrice), "%.2f", itB1->dlPrice);

            char cPriceTotal[1024] = { 0 };
            snprintf(cPriceTotal, sizeof(cPriceTotal), "%.2f", itB1->dlTotalPrice);

            Json::Value jsOrderDetail;
            jsOrderDetail["orddtid"] = itB1->strID;
            jsOrderDetail["pdtid"] = itB1->strPdtID;
            jsOrderDetail["number"] = boost::lexical_cast<std::string>(itB1->iNumber);
            jsOrderDetail["price"] = std::string(cPrice);
            jsOrderDetail["totalprice"] = std::string(cPriceTotal);
            jsOrderDetail["extend"] = itB1->strExtend;

            jsOrderDetailList.append(jsOrderDetail);
        }

        jsOrder["order_detail"] = jsOrderDetailList;

        jsOrderInfoList.append(jsOrder);
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool ProductHandler::AddOrderDetailHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("pdtid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Product id not found.");
        return blResult;
    }
    const std::string strPdtID = itFind->second;

    itFind = pMsgInfoMap->find("ordid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Order id not found.");
        return blResult;
    }
    const std::string strOrdID = itFind->second;

    itFind = pMsgInfoMap->find("number");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Number not found.");
        return blResult;
    }
    int iNumber = 0;
    if (!ValidType<int>(itFind->second, iNumber))
    {
        LOG_ERROR_RLD("Number is error and input value is " << itFind->second);
        return blResult;
    }

    if (0 >= iNumber)
    {
        LOG_ERROR_RLD("Number is invalid and value is " << iNumber);
        return blResult;
    }

    itFind = pMsgInfoMap->find("price");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Product price not found.");
        return blResult;
    }
    double dlPrice = 0;
    if (!ValidType<double>(itFind->second, dlPrice))
    {
        LOG_ERROR_RLD("Product price is error and input value is " << itFind->second);
        return blResult;
    }

    if (0 >= dlPrice)
    {
        LOG_ERROR_RLD("Price is invalid and value is " << dlPrice);
        return blResult;
    }

    double dlTotalPrice = 0xFFFFFFFF;
    itFind = pMsgInfoMap->find("total_price");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidType<double>(itFind->second, dlTotalPrice))
        {
            LOG_ERROR_RLD("Total price is error and input value is " << itFind->second);
            return blResult;
        }

        ////暂不约束，允许为小于等于0的情况
        //if (0 >= dlTotalPrice)
        //{
        //    LOG_ERROR_RLD("Price is invalid and value is " << dlTotalPrice);
        //    return blResult;
        //}
        
    }
    
    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Add order detail info received and session id is " << strSid << " and user id is " << strUserID << " and order id is " << strOrdID
        << " and product id is " << strPdtID << " and number is " << iNumber << " and price is " << dlPrice << " and total price is " << dlTotalPrice
        << " and extend is " << strExtend);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    OrderDetail orddt;
    orddt.__set_dlPrice(dlPrice);
    orddt.__set_dlTotalPrice(dlTotalPrice);
    orddt.__set_iNumber(iNumber);
    orddt.__set_strExtend(strExtend);
    orddt.__set_strOrdID(strOrdID);
    orddt.__set_strPdtID(strPdtID);

    AddOrdDetailRT adrt;
    pclient.AddOrdDetail(adrt, strSid, strUserID, orddt);
    if (adrt.rtcode.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Add order detail call failed" << " and return code is " << adrt.rtcode.iRtCode);

        ReturnInfo::RetCode(adrt.rtcode.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("orddtid", adrt.strOrddtID));

    blResult = true;

    return blResult;
}

bool ProductHandler::RemoveOrderDetailHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("ordid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Order id not found.");
        return blResult;
    }
    const std::string strOrdID = itFind->second;

    std::string strOrddtID;
    itFind = pMsgInfoMap->find("orddtid");
    if (pMsgInfoMap->end() != itFind)
    {
        strOrddtID = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete order detail info received and session id is " << strSid << " and user id is " << strUserID << " and order id is " << strOrdID
        << " and detail id is " << strOrddtID);

    ProductClient pclient;
    ProductClient::Param pam;
    pam.m_iConnectTimeout = pam.m_iReceiveTimeout = pam.m_iSendTimeout = m_ParamInfo.m_uiCallFuncTimeout * 1000;
    pam.m_iServerPort = boost::lexical_cast<int>(m_ParamInfo.m_strRemotePort);
    pam.m_strServerIp = m_ParamInfo.m_strRemoteAddress;
    if (!pclient.Init(pam))
    {
        LOG_ERROR_RLD("Product client init failed and server ip is " << pam.m_strServerIp << " and server port is " << pam.m_iServerPort);
        return false;
    }

    ProductRTInfo pdtrt;
    pclient.RemoveOrdDetail(pdtrt, strSid, strUserID, strOrdID, strOrddtID);
    if (pdtrt.iRtCode != g_Product_constants.PDT_SUCCESS_CODE)
    {
        LOG_ERROR_RLD("Remove order detail call failed" << " and return code is " << pdtrt.iRtCode);

        ReturnInfo::RetCode(pdtrt.iRtCode);
        pclient.Close();
        return blResult;
    }
    pclient.Close();

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool ProductHandler::ParseMsgOfCompact(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter wr)
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



void ProductHandler::WriteMsg(const std::map<std::string, std::string> &MsgMap, MsgWriter writer, const bool blResult, boost::function<void(void*)> PostFunc)
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


bool ProductHandler::ValidDatetime(const std::string &strDatetime, const bool IsTime)
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

bool ProductHandler::ValidNumber(const std::string &strNumber, const unsigned int uiCount)
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

template<typename T>
bool ProductHandler::ValidType(const std::string &strValue, T &ValueT)
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

bool ProductHandler::GetValueList(const std::string &strValue, std::list<std::string> &strValueList)
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

bool ProductHandler::GetValueFromList(const std::string &strValue, boost::function<bool(Json::Value &)> ParseFunc)
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
