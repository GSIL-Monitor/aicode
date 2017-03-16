#include "HttpMsgHandler.h"
#include <boost/algorithm/string.hpp>  
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scope_exit.hpp>
#include "json/json.h"
#include "util.h"
#include "mime_types.h"
#include "LogRLD.h"
#include "InteractiveProtoManagementHandler.h"
#include "CommMsgHandler.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include "boost/regex.hpp"


const std::string HttpMsgHandler::SUCCESS_CODE = "0";
const std::string HttpMsgHandler::SUCCESS_MSG = "Ok";
const std::string HttpMsgHandler::FAILED_CODE = "-1";
const std::string HttpMsgHandler::FAILED_MSG = "Inner failed";



const std::string HttpMsgHandler::ADD_CLUSER_ACTION("add_cluster");

const std::string HttpMsgHandler::DELETE_CLUSER_ACTION("delete_cluster");

const std::string HttpMsgHandler::MODIFY_CLUSER_ACTION("modify_cluster");

const std::string HttpMsgHandler::QUERY_CLUSER_INFO_ACTION("query_cluster_info");

const std::string HttpMsgHandler::CLUSER_SHAKEHAND_ACTION("cluster_shakehand");

const std::string HttpMsgHandler::QUERY_ALL_CLUSER_ACTION("query_all_cluster");

const std::string HttpMsgHandler::QUERY_CLUSER_DEVICE_ACTION("query_cluster_device");

const std::string HttpMsgHandler::QUERY_CLUSER_USER_ACTION("query_cluster_user");

HttpMsgHandler::HttpMsgHandler(const ParamInfo &parminfo):
m_ParamInfo(parminfo),
m_pInteractiveProtoHandler(new InteractiveProtoManagementHandler)
{

}

HttpMsgHandler::~HttpMsgHandler()
{

}


bool HttpMsgHandler::AddCluserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", FAILED_CODE));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("cluster_address");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Cluster address not found.");
        return blResult;
    }
    const std::string strClusterAddress = itFind->second;


    itFind = pMsgInfoMap->find("management_address");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Mnangement address not found.");
        return blResult;
    }
    const std::string strManagementAddress = itFind->second;


    std::string strAliasName;
    itFind = pMsgInfoMap->find("aliasname");
    if (pMsgInfoMap->end() != itFind)
    {
        strAliasName = itFind->second;
    }

    LOG_INFO_RLD("Add cluser info received and cluster address is " << strClusterAddress << " and manangement address is " << strManagementAddress << 
        " and alias name is " << strAliasName);

    std::string strClusterID;
    if (!AddCluster(strClusterAddress, strManagementAddress, strAliasName, strClusterID))
    {
        LOG_ERROR_RLD("Add cluster handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("clusterid", strClusterID));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::DeleteCluserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", FAILED_CODE));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("clusterid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Cluster id not found.");
        return blResult;
    }
    const std::string strClusterID = itFind->second;

    LOG_INFO_RLD("Delete cluser info received and cluster id is " << strClusterID);

    if (!DeleteCluster(strClusterID))
    {
        LOG_ERROR_RLD("Delete cluster handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::ModifyCluserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", FAILED_CODE));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("clusterid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Cluster id not found.");
        return blResult;
    }
    const std::string strClusterID = itFind->second;


    itFind = pMsgInfoMap->find("aliasname");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Alias name not found.");
        return blResult;
    }
    const std::string strAliasName = itFind->second;
    
    LOG_INFO_RLD("Modify cluser info received and cluster id is " << strClusterID << " and alias name is " << strAliasName);

    if (!ModifyCluser(strClusterID, strAliasName))
    {
        LOG_ERROR_RLD("Modify cluster handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    
    blResult = true;

    return blResult;
}

bool HttpMsgHandler::QueryCluserInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", FAILED_CODE));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("clusterid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Cluster id not found.");
        return blResult;
    }
    const std::string strClusterID = itFind->second;
    
    LOG_INFO_RLD("Query cluser info received and cluster id is " << strClusterID);

    std::string strClusterAddress;
    std::string strManagementAddress;
    std::string strAliasName;
    std::string strCreateDate;
    std::string strStatus;
    if (!QueryCluserInfo(strClusterID, strClusterAddress, strManagementAddress, strAliasName, strCreateDate, strStatus))
    {
        LOG_ERROR_RLD("Query cluster handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::make_pair("cluster_address", strClusterAddress));
    ResultInfoMap.insert(std::make_pair("mananement_address", strManagementAddress));
    ResultInfoMap.insert(std::make_pair("aliasname", strAliasName));
    ResultInfoMap.insert(std::make_pair("createdate", strCreateDate));
    ResultInfoMap.insert(std::make_pair("status", strStatus));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::CluserShakehandHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", FAILED_CODE));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("clusterid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Cluster id not found.");
        return blResult;
    }
    const std::string strClusterID = itFind->second;

    LOG_INFO_RLD("Shakehand cluser info received and cluster id is " << strClusterID);
    
    if (!CluserShakehand(strClusterID))
    {
        LOG_ERROR_RLD("Shakehand cluster handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::QueryAllCluserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsClusterInfoList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsClusterInfoList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", FAILED_CODE));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["data"] = jsClusterInfoList;

            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }

    }
    BOOST_SCOPE_EXIT_END
            
    auto itFind = pMsgInfoMap->find("management_address");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Management address not found.");
        return blResult;
    }
    const std::string strManagementAddress = itFind->second;
    
    LOG_INFO_RLD("Query all cluster info received and  management address is " << strManagementAddress);
        
    std::list<InteractiveProtoManagementHandler::ClusterStatus> ClusterInfoList;
    if (!QueryAllCluser<InteractiveProtoManagementHandler::ClusterStatus>(strManagementAddress, ClusterInfoList))
    {
        LOG_ERROR_RLD("Query all cluster handle failed and management address is " << strManagementAddress);
        return blResult;
    }

    auto itBegin = ClusterInfoList.begin();
    auto itEnd = ClusterInfoList.end();
    while (itBegin != itEnd)
    {
        Json::Value jsRelation;
        jsRelation["aliasname"] = itBegin->m_clusterInfo.m_strAliasname;
        jsRelation["cluster_address"] = itBegin->m_clusterInfo.m_strClusterAddress;
        jsRelation["clusterid"] = itBegin->m_clusterInfo.m_strClusterID;
        jsRelation["createdate"] = itBegin->m_clusterInfo.m_strCreatedate;
        jsRelation["management_address"] = itBegin->m_clusterInfo.m_strManagementAddress;
        jsRelation["status"] = itBegin->m_uiStatus;
        
        jsClusterInfoList.append(jsRelation);

        ++itBegin;
    }
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::QueryCluserDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsClusterInfoList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsClusterInfoList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", FAILED_CODE));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["data"] = jsClusterInfoList;

            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }

    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("clusterid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Cluster id not found.");
        return blResult;
    }
    const std::string strClusterID = itFind->second;

    itFind = pMsgInfoMap->find("begindate");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Begin date not found.");
        return blResult;
    }
    const std::string strBeginDate = itFind->second;

    if (!ValidDate(strBeginDate))
    {
        LOG_ERROR_RLD("Begin date format is error.");
        return blResult;
    }
        
    itFind = pMsgInfoMap->find("enddate");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("End date not found.");
        return blResult;
    }
    const std::string strEndDate = itFind->second;

    if (!ValidDate(strEndDate))
    {
        LOG_ERROR_RLD("End date format is error.");
        return blResult;
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
            LOG_ERROR_RLD("Begin index info is invalid and error msg is " << e.what() << " and input index is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Begin index info is invalid and input index is " << itFind->second);
            return blResult;
        }
    }

    itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Type not found.");
        return blResult;
    }
    const std::string strType = itFind->second;
    unsigned int uiType = 0;
    try
    {
        uiType = boost::lexical_cast<unsigned int>(strType);
    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("Type info is invalid and error msg is " << e.what() << " and input index is " << itFind->second);
        return blResult;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Type info is invalid and input index is " << itFind->second);
        return blResult;
    }

    LOG_INFO_RLD("Query cluster device info and cluster id is " << strClusterID << " and begin date is " << strBeginDate << " and end date is " << strEndDate
        << " and type is " << uiType);

    std::list<InteractiveProtoManagementHandler::AccessedDevice> AccessedDevInfoList;
    if (!QueryCluserDevice<InteractiveProtoManagementHandler::AccessedDevice>(strClusterID, strBeginDate, strEndDate, uiType, uiBeginIndex, AccessedDevInfoList))
    {
        LOG_ERROR_RLD("Query cluster device handle failed");
        return blResult;
    }

    auto itBegin = AccessedDevInfoList.begin();
    auto itEnd = AccessedDevInfoList.end();
    while (itBegin != itEnd)
    {
        Json::Value jsDev;
        jsDev["login_date"] = itBegin->m_strLoginTime;
        jsDev["logout_date"] = itBegin->m_strLogoutTime;
        jsDev["devid"] = itBegin->m_strDeviceID;
        jsDev["devtype"] = itBegin->m_uiDeviceType;
        
        jsClusterInfoList.append(jsDev);

        ++itBegin;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::QueryCluserUserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsClusterInfoList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsClusterInfoList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", FAILED_CODE));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["data"] = jsClusterInfoList;

            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }

    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("clusterid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Cluster id not found.");
        return blResult;
    }
    const std::string strClusterID = itFind->second;

    itFind = pMsgInfoMap->find("begindate");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Begin date not found.");
        return blResult;
    }
    const std::string strBeginDate = itFind->second;

    if (!ValidDate(strBeginDate))
    {
        LOG_ERROR_RLD("Begin date format is error.");
        return blResult;
    }

    itFind = pMsgInfoMap->find("enddate");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("End date not found.");
        return blResult;
    }
    const std::string strEndDate = itFind->second;

    if (!ValidDate(strEndDate))
    {
        LOG_ERROR_RLD("End date format is error.");
        return blResult;
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
            LOG_ERROR_RLD("Begin index info is invalid and error msg is " << e.what() << " and input index is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Begin index info is invalid and input index is " << itFind->second);
            return blResult;
        }
    }

    itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Type not found.");
        return blResult;
    }
    const std::string strType = itFind->second;
    unsigned int uiType = 0;
    try
    {
        uiType = boost::lexical_cast<unsigned int>(strType);
    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("Type info is invalid and error msg is " << e.what() << " and input index is " << itFind->second);
        return blResult;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Type info is invalid and input index is " << itFind->second);
        return blResult;
    }

    LOG_INFO_RLD("Query cluster user info and cluster id is " << strClusterID << " and begin date is " << strBeginDate << " and end date is " << strEndDate
        << " and type is " << uiType);

    std::list<InteractiveProtoManagementHandler::AccessedUser> AccessedDevInfoList;
    if (!QueryCluserUser<InteractiveProtoManagementHandler::AccessedUser>(strClusterID, strBeginDate, strEndDate, uiType, uiBeginIndex, AccessedDevInfoList))
    {
        LOG_ERROR_RLD("Query cluster user handle failed");
        return blResult;
    }

    auto itBegin = AccessedDevInfoList.begin();
    auto itEnd = AccessedDevInfoList.end();
    while (itBegin != itEnd)
    {
        Json::Value jsUsr;
        jsUsr["login_date"] = itBegin->m_strLoginTime;
        jsUsr["logout_date"] = itBegin->m_strLogoutTime;
        jsUsr["userid"] = itBegin->m_strUserID;
        jsUsr["terminaltype"] = itBegin->m_uiClientType;

        jsClusterInfoList.append(jsUsr);

        ++itBegin;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;

}

template<typename T>
bool HttpMsgHandler::QueryCluserDevice(const std::string &strClusterID, const std::string &strBeginDate, const std::string &strEndDate, 
    const unsigned int uiType, const unsigned int uiBeginIndex, std::list<T> &AccessedDeviceList)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoManagementHandler::QueryClusterDeviceReq QueryClusterDevReq;
        QueryClusterDevReq.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::QueryClusterDeviceReq_T;
        QueryClusterDevReq.m_uiMsgSeq = 1;
        QueryClusterDevReq.m_strSID = "";
        QueryClusterDevReq.m_strBegindate = strBeginDate;
        QueryClusterDevReq.m_strClusterID = strClusterID;
        QueryClusterDevReq.m_strEnddate = strEndDate;
        QueryClusterDevReq.m_uiRecordType = uiType;
        QueryClusterDevReq.m_uiBeginIndex = uiBeginIndex;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryClusterDevReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query cluster device req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

        InteractiveProtoManagementHandler::QueryClusterDeviceRsp QueryClusterDevRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryClusterDevRsp))
        {
            LOG_ERROR_RLD("Query cluster device rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        AccessedDeviceList.clear();
        AccessedDeviceList.swap(QueryClusterDevRsp.m_accessedDeviceInfoList);

        iRet = QueryClusterDevRsp.m_iRetcode;

        LOG_INFO_RLD("Query cluster device  and return code is " << QueryClusterDevRsp.m_iRetcode <<
            " and return msg is " << QueryClusterDevRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

template<typename T>
bool HttpMsgHandler::QueryCluserUser(const std::string &strClusterID, const std::string &strBeginDate, const std::string &strEndDate,
    const unsigned int uiType, const unsigned int uiBeginIndex, std::list<T> &AccessedUserList)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoManagementHandler::QueryClusterUserReq QueryClusterUserReq;
        QueryClusterUserReq.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::QueryClusterUserReq_T;
        QueryClusterUserReq.m_uiMsgSeq = 1;
        QueryClusterUserReq.m_strSID = "";
        QueryClusterUserReq.m_strBegindate = strBeginDate;
        QueryClusterUserReq.m_strClusterID = strClusterID;
        QueryClusterUserReq.m_strEnddate = strEndDate;
        QueryClusterUserReq.m_uiRecordType = uiType;
        QueryClusterUserReq.m_uiBeginIndex = uiBeginIndex;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryClusterUserReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query cluster user req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

        InteractiveProtoManagementHandler::QueryClusterUserRsp QueryClusterUserRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryClusterUserRsp))
        {
            LOG_ERROR_RLD("Query cluster user rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        AccessedUserList.clear();
        AccessedUserList.swap(QueryClusterUserRsp.m_accessedUserInfoList);

        iRet = QueryClusterUserRsp.m_iRetcode;

        LOG_INFO_RLD("Query cluster user  and return code is " << QueryClusterUserRsp.m_iRetcode <<
            " and return msg is " << QueryClusterUserRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

void HttpMsgHandler::WriteMsg(const std::map<std::string, std::string> &MsgMap, MsgWriter writer, const bool blResult, boost::function<void(void*)> PostFunc)
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

bool HttpMsgHandler::AddCluster(const std::string &strClusterAddress, const std::string &strManagementAddress, const std::string &strAliasName, std::string &strClusterID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {        
        std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
        std::string::size_type pos = strCurrentTime.find('T');
        strCurrentTime.replace(pos, 1, std::string(" "));

        InteractiveProtoManagementHandler::AddClusterReq AddClusterReq;
        AddClusterReq.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::AddClusterReq_T;
        AddClusterReq.m_uiMsgSeq = 1;
        AddClusterReq.m_strSID = "";
        AddClusterReq.m_clusterInfo.m_strAliasname = strAliasName;
        AddClusterReq.m_clusterInfo.m_strClusterAddress = strClusterAddress;
        AddClusterReq.m_clusterInfo.m_strClusterID = "";
        AddClusterReq.m_clusterInfo.m_strCreatedate = strCurrentTime;
        AddClusterReq.m_clusterInfo.m_strManagementAddress = strManagementAddress;
        AddClusterReq.m_clusterInfo.m_uiStatus = 0;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddClusterReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add cluster req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

        InteractiveProtoManagementHandler::AddClusterRsp AddClusterRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddClusterRsp))
        {
            LOG_ERROR_RLD("Add cluster rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        strClusterID = AddClusterRsp.m_strClusterID;
        iRet = AddClusterRsp.m_iRetcode;

        LOG_INFO_RLD("Add cluster and cluser id is " << strClusterID << " and return code is " << AddClusterRsp.m_iRetcode <<
            " and return msg is " << AddClusterRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::DeleteCluster(const std::string &strClusterID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
        std::string::size_type pos = strCurrentTime.find('T');
        strCurrentTime.replace(pos, 1, std::string(" "));

        InteractiveProtoManagementHandler::DeleteClusterReq DelClusterReq;
        DelClusterReq.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::DeleteClusterReq_T;
        DelClusterReq.m_uiMsgSeq = 1;
        DelClusterReq.m_strSID = "";
        DelClusterReq.m_strClusterID = strClusterID;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelClusterReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete cluster req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

        InteractiveProtoManagementHandler::DeleteClusterRsp DelClusterRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelClusterRsp))
        {
            LOG_ERROR_RLD("Delete cluster rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DelClusterRsp.m_iRetcode;

        LOG_INFO_RLD("Delete cluster and cluser id is " << strClusterID << " and return code is " << DelClusterRsp.m_iRetcode <<
            " and return msg is " << DelClusterRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;


}

bool HttpMsgHandler::ModifyCluser(const std::string &strClusterID, const std::string &strAliasName)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
        std::string::size_type pos = strCurrentTime.find('T');
        strCurrentTime.replace(pos, 1, std::string(" "));

        InteractiveProtoManagementHandler::ModifyClusterReq ModClusterReq;
        ModClusterReq.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::ModifyClusterReq_T;
        ModClusterReq.m_uiMsgSeq = 1;
        ModClusterReq.m_strSID = "";
        ModClusterReq.m_clusterInfo.m_strAliasname = strAliasName;
        ModClusterReq.m_clusterInfo.m_strClusterAddress = "";
        ModClusterReq.m_clusterInfo.m_strClusterID = strClusterID;
        ModClusterReq.m_clusterInfo.m_strCreatedate = strCurrentTime;
        ModClusterReq.m_clusterInfo.m_strManagementAddress = "";
        ModClusterReq.m_clusterInfo.m_uiStatus = 0;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModClusterReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify cluster req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

        InteractiveProtoManagementHandler::ModifyClusterRsp ModClusterRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModClusterRsp))
        {
            LOG_ERROR_RLD("Modify cluster rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModClusterRsp.m_iRetcode;

        LOG_INFO_RLD("Modify cluster and cluser id is " << strClusterID << " and alias name is " << strAliasName <<
            " and return code is " << ModClusterRsp.m_iRetcode <<
            " and return msg is " << ModClusterRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::QueryCluserInfo(const std::string &strClusterID, std::string &strClusterAddress, std::string &strManagementAddress, std::string &strAliasName,
    std::string &strCreateDate, std::string &strStatus)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
        std::string::size_type pos = strCurrentTime.find('T');
        strCurrentTime.replace(pos, 1, std::string(" "));

        InteractiveProtoManagementHandler::QueryClusterInfoReq QueryClusterReq;
        QueryClusterReq.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::QueryClusterInfoReq_T;
        QueryClusterReq.m_uiMsgSeq = 1;
        QueryClusterReq.m_strSID = "";
        QueryClusterReq.m_strClusterID = strClusterID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryClusterReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query cluster req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

        InteractiveProtoManagementHandler::QueryClusterInfoRsp QueryClusterRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryClusterRsp))
        {
            LOG_ERROR_RLD("Query cluster rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        strClusterAddress = QueryClusterRsp.m_clusterInfo.m_strClusterAddress;
        strManagementAddress = QueryClusterRsp.m_clusterInfo.m_strManagementAddress;
        strAliasName = QueryClusterRsp.m_clusterInfo.m_strAliasname;
        strCreateDate = QueryClusterRsp.m_clusterInfo.m_strCreatedate;
        strStatus = boost::lexical_cast<std::string>(QueryClusterRsp.m_uiStatus);

        iRet = QueryClusterRsp.m_iRetcode;

        LOG_INFO_RLD("Query cluster and cluser id is " << strClusterID <<
            " and cluster address is " << strClusterAddress <<
            " and management address is " << strManagementAddress <<
            " and alias name is " << strAliasName <<
            " and create date is " << strCreateDate <<
            " and status is " << strStatus <<
            " and return code is " << QueryClusterRsp.m_iRetcode <<
            " and return msg is " << QueryClusterRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::CluserShakehand(const std::string &strClusterID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
        std::string::size_type pos = strCurrentTime.find('T');
        strCurrentTime.replace(pos, 1, std::string(" "));

        InteractiveProtoManagementHandler::ShakehandClusterReq SkClusterReq;
        SkClusterReq.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::ShakehandClusterReq_T;
        SkClusterReq.m_uiMsgSeq = 1;
        SkClusterReq.m_strSID = "";
        SkClusterReq.m_strClusterID = strClusterID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(SkClusterReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Shakehand cluster req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

        InteractiveProtoManagementHandler::ShakehandClusterRsp SkClusterRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, SkClusterRsp))
        {
            LOG_ERROR_RLD("Shakehand cluster rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = SkClusterRsp.m_iRetcode;

        LOG_INFO_RLD("Shakehand cluster and cluser id is " << strClusterID <<            
            " and return code is " << SkClusterRsp.m_iRetcode <<
            " and return msg is " << SkClusterRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;

}


template<typename T>
bool HttpMsgHandler::QueryAllCluser(const std::string &strManagementAddress, std::list<T> &ClusterInfoList)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoManagementHandler::QueryAllClusterReq QueryAllClsReq;
        QueryAllClsReq.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::QueryAllClusterReq_T;
        QueryAllClsReq.m_uiMsgSeq = 1;
        QueryAllClsReq.m_strSID = "";
        QueryAllClsReq.m_strManagementAddress = strManagementAddress;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryAllClsReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all cluster req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

        InteractiveProtoManagementHandler::QueryAllClusterRsp QueryAllClsRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryAllClsRsp))
        {
            LOG_ERROR_RLD("Query all cluster rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        ClusterInfoList.clear();
        ClusterInfoList.swap(QueryAllClsRsp.m_clusterStatusList);

        iRet = QueryAllClsRsp.m_iRetcode;

        LOG_INFO_RLD("Query all cluster and management address is " << strManagementAddress <<
            " and return code is " << QueryAllClsRsp.m_iRetcode <<
            " and return msg is " << QueryAllClsRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::ValidDate(const std::string &strDate)
{
    boost::regex reg0("([0-9]{4}[0-9]{2}[0-9]{2}[0-9]{2}[0-9]{2}[0-9]{2})"); //yyyyMMddHHmmss
    boost::regex reg1("([0-9]{4}[0-9]{2}[0-9]{2})"); //yyyyMMdd
    boost::regex reg2("([0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2})"); //yyyy-MM-dd HH:mm:ss
    boost::regex reg3("([0-9]{4}-[0-9]{2}-[0-9]{2})"); ////yyyy-MM-dd
    boost::regex reg4("([0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2})"); //yyyy-MM-dd  HH:mm

    if (!boost::regex_match(strDate, reg0) && !boost::regex_match(strDate, reg1) && !boost::regex_match(strDate, reg2) &&
        !boost::regex_match(strDate, reg3) && !boost::regex_match(strDate, reg4))
    {
        LOG_ERROR_RLD("File begin date is invalid and input date is " << strDate);
        return false;
    }

    return true;
}

//
bool HttpMsgHandler::PreCommonHandler(const std::string &strMsgReceived)
{
    ////
    //InteractiveProtoHandler::MsgType mtype;
    //if (!m_pInteractiveProtoHandler->GetMsgType(strMsgReceived, mtype))
    //{
    //    LOG_ERROR_RLD("Get msg type failed.");
    //    return false;
    //}

    //if (InteractiveProtoHandler::MsgType::MsgPreHandlerRsp_USR_T == mtype)
    //{
    //    InteractiveProtoHandler::MsgPreHandlerRsp_USR rsp;
    //    if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, rsp))
    //    {
    //        LOG_ERROR_RLD("Msg prehandler rsp unserialize failed.");
    //        return false;
    //    }

    //    LOG_INFO_RLD("Msg prehandler rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
    //        " and user session id is " << rsp.m_strSID);

    //    if (CommMsgHandler::SUCCEED != rsp.m_iRetcode)
    //    {
    //        LOG_ERROR_RLD("Msg prehandler rsp return failed.");
    //        return false;
    //    }
    //}

    return true;
}

