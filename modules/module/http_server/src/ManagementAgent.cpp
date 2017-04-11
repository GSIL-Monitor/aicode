#include "ManagementAgent.h"
#include <boost/lexical_cast.hpp>
#include <boost/scope_exit.hpp>
#include "json/json.h"
#include "LogRLD.h"
#include "CommMsgHandler.h"
#include "InteractiveProtoHandler.h"
#include "InteractiveProtoManagementHandler.h"

const std::string ManagementAgent::ADD_CLUSTER_ACTION("add_cluster_agent");

const std::string ManagementAgent::DELETE_CLUSTER_ACTION("delete_cluster_agent");

const std::string ManagementAgent::CLUSTER_SHAKEHAND__ACTION("cluster_shakehand");

const std::string ManagementAgent::SUCCESS_CODE = "0";
const std::string ManagementAgent::SUCCESS_MSG = "Ok";
const std::string ManagementAgent::FAILED_CODE = "-1";
const std::string ManagementAgent::FAILED_MSG = "Inner failed";

ManagementAgent::ManagementAgent(const ParamInfo &parminfo) : m_ParamInfo(parminfo), 
m_Tm(boost::bind(&ManagementAgent::CollectClusterInfo, this, _1), parminfo.m_uiCollectInfoTimeout),
m_pInteractiveProtoHandler(new InteractiveProtoHandler), m_pInteractiveProtoMgrHandler(new InteractiveProtoManagementHandler), m_PushCollectInfoRunner(1)
{
    m_PushCollectInfoRunner.Run();
    m_Tm.Run();
}


ManagementAgent::~ManagementAgent()
{
    m_Tm.Stop();
    m_PushCollectInfoRunner.Stop();
}

void ManagementAgent::SetMsgWriter(Writer wr)
{
    m_Wr = wr;
}

bool ManagementAgent::AddClusterAgentHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

        this_->m_Wr(ResultInfoMap, writer, blResult, NULL);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("management_address");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Management address not found.");
        return blResult;
    }
    const std::string strManagementAddress = itFind->second;
    
    itFind = pMsgInfoMap->find("clusterid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Cluster id not found.");
        return blResult;
    }
    const std::string strClusterID = itFind->second;


    if (std::string::npos == strManagementAddress.find(':'))
    {
        LOG_ERROR_RLD("Add cluster agent error because management address format is error and receive mangement address is " << strManagementAddress);
        return blResult;
    }

    LOG_INFO_RLD("Add cluster agent info received and management address is " << strManagementAddress << " and cluster id is " << strClusterID);

    if (!AddClusterAgent(strManagementAddress, strClusterID))
    {
        LOG_ERROR_RLD("Add cluster agent handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}


bool ManagementAgent::ClusterAgentShakehandHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

        this_->m_Wr(ResultInfoMap, writer, blResult, NULL);
    }
    BOOST_SCOPE_EXIT_END
        
    auto itFind = pMsgInfoMap->find("clusterid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Cluster id not found.");
        return blResult;
    }
    const std::string strClusterID = itFind->second;

    LOG_INFO_RLD("Shakehand cluster agent info received and cluster id is " << strClusterID);

    {
        boost::unique_lock<boost::mutex> lock(m_MgnArMutex);
        if (m_strManagementAddress.empty())
        {
            LOG_ERROR_RLD("Current management address is empty, so failed to shakehand.");
            return blResult;
        }
    }
    
        
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool ManagementAgent::DeleteClusterAgentHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

        this_->m_Wr(ResultInfoMap, writer, blResult, NULL);
    }
    BOOST_SCOPE_EXIT_END
        
    auto itFind = pMsgInfoMap->find("clusterid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Cluster id not found.");
        return blResult;
    }
    const std::string strClusterID = itFind->second;

    LOG_INFO_RLD("Delete cluster agent info received and cluster id is " << strClusterID);

    if (!DeleteClusterAgent(strClusterID))
    {
        LOG_ERROR_RLD("Delete cluster agent handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool ManagementAgent::AddClusterAgent(const std::string &strManagementAddress, const std::string &strClusterID)
{
    boost::unique_lock<boost::mutex> lock(m_MgnArMutex);
    m_strManagementAddress = strManagementAddress;
    m_strClusterID = strClusterID;
    
    LOG_INFO_RLD("Receive add cluster agent info and management address is " << strManagementAddress <<
        " and cluster id is " << strClusterID);
    
    return true;
}

bool ManagementAgent::DeleteClusterAgent(const std::string &strClusterID)
{
    boost::unique_lock<boost::mutex> lock(m_MgnArMutex);
    m_strManagementAddress.clear();
    m_strClusterID.clear();
    
    return true;
}

void ManagementAgent::CollectClusterInfo(const boost::system::error_code& e)
{
    if (e)
    {
        LOG_ERROR_RLD("Timer is error: " << e.message());
        return;
    }

    std::string strPushIpAddress;
    std::string strPushPort;
    {
        boost::unique_lock<boost::mutex> lock(m_MgnArMutex);
        if (m_strManagementAddress.empty())
        {
            return;
        }

        std::string::size_type iPos = 0;
        if (std::string::npos == (iPos = m_strManagementAddress.find(':')))
        {
            LOG_ERROR_RLD("Management address format is error and current mangement address is " << m_strManagementAddress);
            return;
        }

        strPushIpAddress = m_strManagementAddress.substr(0, iPos);
        strPushPort = m_strManagementAddress.substr(iPos + 1);

        LOG_INFO_RLD("Current push address is " << strPushIpAddress << " and push port is " << strPushPort);
    }

    LOG_INFO_RLD("Begin collect cluster info.");
    
    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));

    if (CommMsgHandler::SUCCEED != pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval))
    {
        LOG_ERROR_RLD("Connect channel failed and remote address is " << m_ParamInfo.m_strRemoteAddress << 
            "and port is " << m_ParamInfo.m_strRemotePort);
        return;
    }

    unsigned int uiIndex = 0;
    unsigned int uiTotal = 0;
    unsigned int uiReceived = 0;
    do
    {
        auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
        {            
            InteractiveProtoHandler::GetDeviceAccessRecordReq_INNER DeviceAccessedReq;
            DeviceAccessedReq.m_MsgType = InteractiveProtoHandler::MsgType::GetDeviceAccessRecordReq_INNER_T;
            DeviceAccessedReq.m_uiMsgSeq = 1;
            DeviceAccessedReq.m_strSID = "";
            DeviceAccessedReq.m_uiBeginIndex = uiIndex;

            std::string strSerializeOutPut;
            if (!m_pInteractiveProtoHandler->SerializeReq(DeviceAccessedReq, strSerializeOutPut))
            {
                LOG_ERROR_RLD("Get accessed device info req serialize failed.");
                return CommMsgHandler::FAILED;
            }

            return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
        };

        int iRet = CommMsgHandler::FAILED;
        auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
        {
            const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

            ////
            //if (!PreCommonHandler(strMsgReceived))
            //{
            //    return iRet = CommMsgHandler::FAILED;
            //}

            InteractiveProtoHandler::GetDeviceAccessRecordRsp_INNER DeviceAccessedRsp;
            if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DeviceAccessedRsp))
            {
                LOG_ERROR_RLD("Get accessed device info rsp unserialize failed.");
                return iRet = CommMsgHandler::FAILED;
            }

            iRet = DeviceAccessedRsp.m_iRetcode;

            if (CommMsgHandler::SUCCEED != iRet)
            {
                LOG_ERROR_RLD("Get accessed device info return code failed.");
                return iRet = CommMsgHandler::FAILED;
            }

            uiTotal = DeviceAccessedRsp.m_uiRecordTotal;

            boost::shared_ptr<std::list<InteractiveProtoHandler::DeviceAccessRecord> > pDeviceInfoList
                (
                new std::list<InteractiveProtoHandler::DeviceAccessRecord>
                );
            
            pDeviceInfoList->swap(DeviceAccessedRsp.m_deviceAccessRecordList);

            m_PushCollectInfoRunner.Post
                (
                boost::bind(&ManagementAgent::AccessedDeviceInfoHandler<std::list<InteractiveProtoHandler::DeviceAccessRecord> >, this, pDeviceInfoList,
                strPushIpAddress, strPushPort)
                );
            
            uiIndex = uiReceived = uiReceived + pDeviceInfoList->size();
            
            LOG_INFO_RLD("Get accessed device info and total number is " << uiTotal << " and current index is " << uiIndex <<
                " and receive number is " << uiReceived <<
                " and return code is " << DeviceAccessedRsp.m_iRetcode <<
                " and return msg is " << DeviceAccessedRsp.m_strRetMsg);

            return CommMsgHandler::SUCCEED;
        };

        pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);
        
        if (CommMsgHandler::SUCCEED != pCommMsgHdr->StartTask() ||
            CommMsgHandler::SUCCEED != iRet)
        {
            LOG_ERROR_RLD("Get accessed device info failed and current index is " << uiIndex << " and total number is " << uiTotal);
            return;
        }       

    } while (uiReceived < uiTotal);

    LOG_INFO_RLD("Collecting user info.");

    {
        uiIndex = 0;
        uiTotal = 0;
        uiReceived = 0;

        do
        {
            auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
            {
                InteractiveProtoHandler::GetUserAccessRecordReq_INNER UserAccessedReq;
                UserAccessedReq.m_MsgType = InteractiveProtoHandler::MsgType::GetUserAccessRecordReq_INNER_T;
                UserAccessedReq.m_uiMsgSeq = 1;
                UserAccessedReq.m_strSID = "";
                UserAccessedReq.m_uiBeginIndex = uiIndex;

                std::string strSerializeOutPut;
                if (!m_pInteractiveProtoHandler->SerializeReq(UserAccessedReq, strSerializeOutPut))
                {
                    LOG_ERROR_RLD("Get accessed user info req serialize failed.");
                    return CommMsgHandler::FAILED;
                }

                return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
            };

            int iRet = CommMsgHandler::FAILED;
            auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
            {
                const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

                ////
                //if (!PreCommonHandler(strMsgReceived))
                //{
                //    return iRet = CommMsgHandler::FAILED;
                //}

                InteractiveProtoHandler::GetUserAccessRecordRsp_INNER UserAccessedRsp;
                if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, UserAccessedRsp))
                {
                    LOG_ERROR_RLD("Get accessed user info rsp unserialize failed.");
                    return iRet = CommMsgHandler::FAILED;
                }

                iRet = UserAccessedRsp.m_iRetcode;

                if (CommMsgHandler::SUCCEED != iRet)
                {
                    LOG_ERROR_RLD("Get accessed user info return code failed.");
                    return iRet = CommMsgHandler::FAILED;
                }

                uiTotal = UserAccessedRsp.m_uiRecordTotal;

                boost::shared_ptr<std::list<InteractiveProtoHandler::UserAccessRecord> > pUserInfoList
                    (
                    new std::list<InteractiveProtoHandler::UserAccessRecord>
                    );

                pUserInfoList->swap(UserAccessedRsp.m_userAccessRecordList);

                m_PushCollectInfoRunner.Post
                    (
                    boost::bind(&ManagementAgent::AccessedUserInfoHandler<std::list<InteractiveProtoHandler::UserAccessRecord> >, this, pUserInfoList,
                    strPushIpAddress, strPushPort)
                    );

                uiIndex = uiReceived = uiReceived + pUserInfoList->size();

                LOG_INFO_RLD("Get accessed user info and total number is " << uiTotal << " and current index is " << uiIndex <<
                    " and receive number is " << uiReceived <<
                    " and return code is " << UserAccessedRsp.m_iRetcode <<
                    " and return msg is " << UserAccessedRsp.m_strRetMsg);

                return CommMsgHandler::SUCCEED;
            };

            pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);
            
            if (CommMsgHandler::SUCCEED != pCommMsgHdr->StartTask() ||
                CommMsgHandler::SUCCEED != iRet)
            {
                LOG_ERROR_RLD("Get accessed user info failed and current index is " << uiIndex << "and total number is " << uiTotal);
                return;
            }           

        } while (uiReceived < uiTotal);
    }

    LOG_INFO_RLD("End collect cluster info.");

}


template<typename T>
void ManagementAgent::AccessedDeviceInfoHandler(boost::shared_ptr<T> DeviceInfoList, const std::string &strPushIPAddress, const std::string &strPushPort)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoManagementHandler::PushClusterDeviceReq PushDevReq;
        PushDevReq.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::PushClusterDeviceReq_T;
        PushDevReq.m_uiMsgSeq = 1;
        PushDevReq.m_strSID = "";
        PushDevReq.m_strClusterID = m_strClusterID;

        for (auto &DevInfo : (*DeviceInfoList))
        {
            InteractiveProtoManagementHandler::DeviceAccessRecord dar;
            dar.m_strAccessID = DevInfo.m_strAccessID;
            dar.m_strClusterID = m_strClusterID; //DevInfo.m_strClusterID;
            dar.m_strCreateDate = DevInfo.m_strCreateDate;
            dar.m_uiStatus = DevInfo.m_uiStatus;
            dar.m_accessedDevice.m_strDeviceID = DevInfo.m_strDeviceID;
            dar.m_accessedDevice.m_strDeviceName = DevInfo.m_strDeviceName;
            dar.m_accessedDevice.m_strLoginTime = DevInfo.m_strLoginTime;
            dar.m_accessedDevice.m_strLogoutTime = DevInfo.m_strLogoutTime;
            dar.m_accessedDevice.m_uiDeviceType = DevInfo.m_uiDeviceType;
            dar.m_accessedDevice.m_uiOnlineDuration = DevInfo.m_uiOnlineDuration;
            
            PushDevReq.m_deviceAccessRecordList.push_back(std::move(dar));
        }

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoMgrHandler->SerializeReq(PushDevReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Push accessed device req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        ////
        //if (!PreCommonHandler(strMsgReceived))
        //{
        //    return iRet = CommMsgHandler::FAILED;
        //}

        InteractiveProtoManagementHandler::PushClusterDeviceRsp PushDevRsp;
        if (!m_pInteractiveProtoMgrHandler->UnSerializeReq(strMsgReceived, PushDevRsp))
        {
            LOG_ERROR_RLD("Push accessed device rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = PushDevRsp.m_iRetcode;

        LOG_INFO_RLD("Push accessed device" <<
            " and return code is " << PushDevRsp.m_iRetcode <<
            " and return msg is " << PushDevRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    bool blResult = CommMsgHandler::SUCCEED == pCommMsgHdr->Start(strPushIPAddress,
        strPushPort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;

    if (blResult)
    {
        LOG_INFO_RLD("Push accessed device success.");
    }
    else
    {
        LOG_ERROR_RLD("Push accessed device failed.");
    }

}

template<typename T>
void ManagementAgent::AccessedUserInfoHandler(boost::shared_ptr<T> UserInfoList, const std::string &strPushIPAddress, const std::string &strPushPort)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoManagementHandler::PushClusterUserReq PushUsrReq;
        PushUsrReq.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::PushClusterUserReq_T;
        PushUsrReq.m_uiMsgSeq = 1;
        PushUsrReq.m_strSID = "";
        PushUsrReq.m_strClusterID = m_strClusterID;

        for (auto &UsrInfo : (*UserInfoList))
        {
            InteractiveProtoManagementHandler::UserAccessRecord dar;
            dar.m_strAccessID = UsrInfo.m_strAccessID;
            dar.m_strClusterID = m_strClusterID; //UsrInfo.m_strClusterID;
            dar.m_strCreateDate = UsrInfo.m_strCreateDate;
            dar.m_uiStatus = UsrInfo.m_uiStatus;
            dar.m_accessedUser.m_strUserID = UsrInfo.m_strUserID;
            dar.m_accessedUser.m_strUserName = UsrInfo.m_strUserName;
            dar.m_accessedUser.m_strUserAliasname = UsrInfo.m_strUserAliasname;
            dar.m_accessedUser.m_strLoginTime = UsrInfo.m_strLoginTime;
            dar.m_accessedUser.m_strLogoutTime = UsrInfo.m_strLogoutTime;
            dar.m_accessedUser.m_uiClientType = UsrInfo.m_uiClientType;
            dar.m_accessedUser.m_uiOnlineDuration = UsrInfo.m_uiOnlineDuration;

            PushUsrReq.m_userAccessRecordList.push_back(std::move(dar));
        }

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoMgrHandler->SerializeReq(PushUsrReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Push accessed user req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        ////
        //if (!PreCommonHandler(strMsgReceived))
        //{
        //    return iRet = CommMsgHandler::FAILED;
        //}

        InteractiveProtoManagementHandler::PushClusterUserRsp PushUsrRsp;
        if (!m_pInteractiveProtoMgrHandler->UnSerializeReq(strMsgReceived, PushUsrRsp))
        {
            LOG_ERROR_RLD("Push accessed user rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = PushUsrRsp.m_iRetcode;

        LOG_INFO_RLD("Push accessed user" <<
            " and return code is " << PushUsrRsp.m_iRetcode <<
            " and return msg is " << PushUsrRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    bool blResult = CommMsgHandler::SUCCEED == pCommMsgHdr->Start(strPushIPAddress,
        strPushPort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;

    if (blResult)
    {
        LOG_INFO_RLD("Push accessed user success.");
    }
    else
    {
        LOG_ERROR_RLD("Push accessed user failed.");
    }
}




