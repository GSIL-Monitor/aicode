#include "ControlCenter.h"
#include "ProtoHandler.h"
#include "LogRLD.h"
#include "json/json.h"
#include "CommonUtility.h"
#include "uuid/uuid.h"

std::string ConvertCharValueToLex(unsigned char *pInValue, const boost::uint32_t uiSize);

ControlCenter::ControlCenter(const ParamInfo &pinfo) : m_pProtoHandler(new ProtoHandler), m_ParamInfo(pinfo), m_MsgHandlerRunner(1)
{
    m_MsgUnSerializerMap.insert(std::make_pair(ProtoHandler::MsgType::LoginReq_T, boost::bind(&ControlCenter::LoginReqMsgHandler, this, _1, _2)));
    m_MsgUnSerializerMap.insert(std::make_pair(ProtoHandler::MsgType::LoginoutReq_T, boost::bind(&ControlCenter::LoginoutReqMsgHandler, this, _1, _2)));
    m_MsgUnSerializerMap.insert(std::make_pair(ProtoHandler::MsgType::ConfigInfoReq_T, boost::bind(&ControlCenter::ConfigInfoReqMsgHandler, this, _1, _2)));
    m_MsgUnSerializerMap.insert(std::make_pair(ProtoHandler::MsgType::ShakehandReq_T, boost::bind(&ControlCenter::ShakehandReqMsgHandler, this, _1, _2)));
    m_MsgUnSerializerMap.insert(std::make_pair(ProtoHandler::MsgType::GetSyncAddressReq_T, boost::bind(&ControlCenter::GetSyncAddressReqMsgHandler, this, _1, _2)));
    m_MsgUnSerializerMap.insert(std::make_pair(ProtoHandler::MsgType::SyncFileListPendingReq_T, boost::bind(&ControlCenter::SyncFileListPendingReqMsgHandler, this, _1, _2)));
    m_MsgUnSerializerMap.insert(std::make_pair(ProtoHandler::MsgType::ControlCMDReq_T, boost::bind(&ControlCenter::ControlCMDReqMsgHandler, this, _1, _2)));
    m_MsgUnSerializerMap.insert(std::make_pair(ProtoHandler::MsgType::GetFileInfoReq_T, boost::bind(&ControlCenter::GetFileInfoReqMsgHandler, this, _1, _2)));

}

ControlCenter::~ControlCenter()
{

}

void ControlCenter::Run(const bool isWaitRunFinished)
{
    m_TMEX.Run(1);

    ClientCommInterface *pClient = ClientCommInterface::Create(m_ParamInfo.strRemoteAddress.c_str(), 
        m_ParamInfo.strRemotePort.c_str(), 0, m_ParamInfo.uiShakehandOfChannelInterval);

    pClient->SetCallBack
        (
        boost::bind(&ControlCenter::ConnectCB, this, _1),
        boost::bind(&ControlCenter::ReadCB, this, _1, _2, _3),
        boost::bind(&ControlCenter::WriteCB, this, _1, _2)
        );

    ClientCommInterface::Run(m_ParamInfo.uiThreadOfWorking);
    pClient->AsyncConnect();
    m_pClient.reset(pClient);

    LOG_INFO_RLD("Control center begin running...");
    m_MsgHandlerRunner.Run(isWaitRunFinished);
}

void ControlCenter::Stop()
{
    m_TMEX.Stop();

    m_MsgHandlerRunner.Stop();
    m_pClient->Close();
    ClientCommInterface::Stop();
}

void ControlCenter::ConnectCB(const boost::system::error_code &ec)
{
    if (ec)
    {
        LOG_ERROR_RLD("Failed to connect proxy, error is " << ec.message());
        Reconnect();
        return;
    }

    //作为id为0的客户端连接到proxy上，这里发送一个注册消息，类似RW 
    char *cBuffer = new char[128];
    memset(cBuffer, 0, 128);
    snprintf(cBuffer, 128, "RG,%d,%s,0,0,0,0",
        8 + (int)m_ParamInfo.strSelfID.size(), m_ParamInfo.strSelfID.c_str());

    LOG_INFO_RLD("Register msg is " << cBuffer);

    m_pClient->AsyncWrite("0", "0", "0", cBuffer, strlen(cBuffer), false, NULL);

}

void ControlCenter::WriteCB(const boost::system::error_code &ec, void *pValue)
{
    if (ec)
    {
        LOG_ERROR_RLD("Write msg failed, error is " << ec.message());
        Reconnect();
        return;
    }

    m_pClient->AsyncRead(pValue);

}

void ControlCenter::ReadCB(const boost::system::error_code &ec, std::list<ClientMsg> *pClientMsgList, void *pValue)
{
    LOG_INFO_RLD("pClientMsgList size is " << pClientMsgList->size());

    if (ec || (NULL == pClientMsgList) || pClientMsgList->empty())
    {
        LOG_ERROR_RLD("Read msg failed, error is " << ec.message() << " pClientMsgList is " << (NULL == pClientMsgList) <<
           " and pClientMsgList size is " << (NULL == pClientMsgList) ? 0 : pClientMsgList->size());
        Reconnect();
        return;
    }

    //
    {
        auto itBegin = pClientMsgList->begin();
        auto itEnd = pClientMsgList->end();
        while (itBegin != itEnd)
        {
            if ((itBegin->strSrcID == "0" || itBegin->strSrcID == m_ParamInfo.strSelfID) && itBegin->strDstID == "0" && itBegin->uiContentBufferLen == 1)
            {
                LOG_INFO_RLD("ClientMsg is shakehand length is " << itBegin->uiContentBufferLen << " and src is " << itBegin->strSrcID << " and dst is " << itBegin->strDstID);
                pClientMsgList->erase(itBegin++);
                continue;
            }

            ++itBegin;
        }
    }

    if (pClientMsgList->empty())
    {
        m_pClient->AsyncRead(pValue);

        LOG_INFO_RLD("Direct async reading.");
        return;
    }

    LOG_INFO_RLD("Begin handle msg of received and msg number is " << pClientMsgList->size());

    auto itBegin = pClientMsgList->begin();
    auto itEnd = pClientMsgList->end();
    while (itBegin != itEnd)
    {
        LOG_INFO_RLD("ClientMsg length is " << itBegin->uiContentBufferLen << " and src is " << itBegin->strSrcID << " and dst is " << itBegin->strDstID);

        if ((itBegin->strSrcID == "0" || itBegin->strSrcID == m_ParamInfo.strSelfID) && itBegin->strDstID == "0" && itBegin->uiContentBufferLen == 1)
        {
            //skip shake hand msg or register msg
            ++itBegin;
            continue;
        }

        std::string strValue;
        strValue.assign(itBegin->pContentBuffer.get(), itBegin->uiContentBufferLen);
        if (!ReceiveMsgHandler(strValue, itBegin->strSrcID, pValue))
        {
            LOG_ERROR_RLD("ReceiveMsgHandler failed, " << " and src is " << itBegin->strSrcID << " and dst is " << itBegin->strDstID);

            ++itBegin;
            continue;
        }

        ++itBegin;
    }
        
}

void ControlCenter::Reconnect()
{
    LOG_INFO_RLD("Reconnect to " << m_ParamInfo.strRemoteAddress << ":" << m_ParamInfo.strRemotePort);
    boost::this_thread::sleep(boost::posix_time::seconds(2));
    m_pClient->AsyncConnect();    
}

bool ControlCenter::ReceiveMsgHandler(const std::string &strData, const std::string &strSrcID, void *pValue)
{
    ProtoHandler::MsgType mtype;
    if (!m_pProtoHandler->GetMsgType(strData, mtype))
    {
        LOG_ERROR_RLD("Get msg type failed.");
        return false;
    }
    
    LOG_INFO_RLD("Receive msg type is " << mtype);

    auto itFind = m_MsgUnSerializerMap.find(mtype);
    if (m_MsgUnSerializerMap.end() == itFind)
    {
        LOG_ERROR_RLD("Not found unserializer handler and type is " << mtype);
        return false;
    }

    ////////
    //if (!(itFind->second)(strData))
    //{
    //    LOG_ERROR_RLD("Unserializer handler failed and type is " << req.m_MsgType);
    //    return false;
    //}

    m_MsgHandlerRunner.Post(boost::bind(&ControlCenter::ReceiveMsgHandlerInner, this, itFind->second, strData, strSrcID, mtype, pValue));

    return true;
}

void ControlCenter::ReceiveMsgHandlerInner(MsgUnSerializer MsgUszr, const std::string &strData, const std::string &strSrcID, const int iMsgType, void *pValue)
{
    if (!MsgUszr(strData, strSrcID))
    {
        LOG_ERROR_RLD("Unserializer handler failed and type is " << iMsgType << " src id is " << strSrcID);
    }
    else
    {
        LOG_INFO_RLD("Unserializer handler success and type is " << iMsgType << " src id is " << strSrcID);
    }

    //m_pClient->AsyncRead(pValue);

}

bool ControlCenter::LoginReqMsgHandler(const std::string &strData, const std::string &strSrcID)
{
    ProtoHandler::LoginReq req;
    if (!m_pProtoHandler->UnSerializeReq(strData, req))
    {
        LOG_ERROR_RLD("Unserialize login req failed and srcid is " << strSrcID);
        return false;
    }

    LOG_INFO_RLD("login req info, syncservice name is " << req.m_strSyncServiceName << ", storage ip is " << req.m_strStorageIP << ", storage port is " 
        << req.m_strStoragePort);

    uuid_t uu;
    uuid_generate(uu);
    const std::string &strUUID = ConvertCharValueToLex((unsigned char *)uu, sizeof(uu));
    
    ProtoHandler::LoginRsp rsp;
    rsp.m_MsgType = ProtoHandler::MsgType::LoginRsp_T;
    rsp.m_uiMsgSeq = req.m_uiMsgSeq;
    rsp.m_strSID = strUUID;
    rsp.m_iRetcode = 0;
    rsp.m_strRetMsg = "success";
    
    rsp.m_strLoginSID = strUUID;


    boost::shared_ptr<SyncService> pSyncService(new SyncService);
    pSyncService->m_strSID = rsp.m_strSID;
    pSyncService->m_strSyncServiceName = req.m_strSyncServiceName;
    pSyncService->m_strPassword = req.m_strPassword;
    pSyncService->m_strStorageIP = req.m_strStorageIP;
    pSyncService->m_strStoragePort = req.m_strStoragePort;
    pSyncService->m_Status = SyncService::SyncServiceStatus::IDLE;
    pSyncService->m_strAddress = strSrcID;
    pSyncService->m_Type = SyncService::NodeType::Sync;
    pSyncService->m_strAreaID = "0";
    pSyncService->m_uiTMTickCount = 0;
    pSyncService->m_pTMHander = m_TMEX.Create(boost::bind(&SyncService::TimeOutCB, pSyncService, _1), m_ParamInfo.uiSyncShakehandTimeout);
    pSyncService->m_uiMaxTickCount = m_ParamInfo.uiSyncShakehandTimeoutCount;
    pSyncService->m_TMFunc = boost::bind(&ControlCenter::DeleteSyncService, this, pSyncService->m_strSID);

    pSyncService->m_pTMHander->Begin();
    
    AddSyncService(pSyncService->m_strSID, pSyncService);

    std::string strOutput;
    m_pProtoHandler->SerializeRsp(rsp, strOutput);

    m_pClient->AsyncWrite("0", strSrcID, "0", strOutput.data(), strOutput.size(), true);

    return true;
}

bool ControlCenter::LoginoutReqMsgHandler(const std::string &strData, const std::string &strSrcID)
{
    ProtoHandler::LogoutReq req;
    if (!m_pProtoHandler->UnSerializeReq(strData, req))
    {
        LOG_ERROR_RLD("Unserialize logout req failed and srcid is " << strSrcID);
        return false;
    }

    LOG_INFO_RLD("logout req strvalue is " << req.m_strValue);

    int iRet = 0;
    std::string strRetMsg("logout success");

    auto itFind = FindSyncService(req.m_strSID);
    if (NULL == itFind.get())
    {
        LOG_ERROR_RLD("Find syncservice failed and srcid is " << strSrcID << " and sid is " << req.m_strSID);
        iRet = -1;
        strRetMsg = "logout req session id not found.";
    }
    else
    {
        //boost::unique_lock<boost::mutex> lock(itFind->m_SyncServiceMutex);
        itFind->m_uiTMTickCount = 0;
        LOG_INFO_RLD("Receive logout req and src id is " << strSrcID);
    }

    ProtoHandler::LogoutRsp rsp;
    rsp.m_MsgType = ProtoHandler::MsgType::LoginoutRsp_T;
    rsp.m_uiMsgSeq = req.m_uiMsgSeq;
    rsp.m_strSID = req.m_strSID;
    rsp.m_iRetcode = iRet;
    rsp.m_strRetMsg = strRetMsg;
    rsp.m_strValue = req.m_strValue;

    DeleteSyncService(req.m_strSID);

    std::string strOutput;
    m_pProtoHandler->SerializeRsp(rsp, strOutput);
    m_pClient->AsyncWrite("0", strSrcID, "0", strOutput.data(), strOutput.size(), true);
    
    return true;
}

bool ControlCenter::ConfigInfoReqMsgHandler(const std::string &strData, const std::string &strSrcID)
{
    ProtoHandler::ConfigInfoReq req;
    if (!m_pProtoHandler->UnSerializeReq(strData, req))
    {
        LOG_ERROR_RLD("Unserialize config req failed and srcid is " << strSrcID);
        return false;
    }

    LOG_INFO_RLD("config info req, syncservice name is " << req.m_strSyncServiceName);

    int iRet = 0;
    std::string strRetMsg("get config info success");

    auto itFind = FindSyncService(req.m_strSID);
    if (NULL == itFind.get())
    {
        LOG_ERROR_RLD("Find syncservice failed and srcid is " << strSrcID << " and sid is " << req.m_strSID);
        iRet = -1;
        strRetMsg = "config req session id not found.";
    }
    else
    {
        //boost::unique_lock<boost::mutex> lock(itFind->m_SyncServiceMutex);
        itFind->m_uiTMTickCount = 0;
        LOG_INFO_RLD("Receive config req and src id is " << strSrcID);
    }

    ProtoHandler::ConfigInfoRsp rsp;
    rsp.m_MsgType = ProtoHandler::MsgType::ConfigInfoRsp_T;
    rsp.m_uiMsgSeq = req.m_uiMsgSeq;
    rsp.m_strSID = req.m_strSID;
    rsp.m_iRetcode = iRet;
    rsp.m_strRetMsg = strRetMsg;
    rsp.m_strConfigJson = GetConfigInfo(req.m_strSyncServiceName);

    std::string strOutput;
    m_pProtoHandler->SerializeRsp(rsp, strOutput);
    m_pClient->AsyncWrite("0", strSrcID, "0", strOutput.data(), strOutput.size(), true);

    return true;
}

bool ControlCenter::ShakehandReqMsgHandler(const std::string &strData, const std::string &strSrcID)
{
    ProtoHandler::ShakehandReq req;
    if (!m_pProtoHandler->UnSerializeReq(strData, req))
    {
        LOG_ERROR_RLD("Unserialize shakehand req failed and srcid is " << strSrcID);
        return false;
    }

    int iRet = 0;
    std::string strRetMsg("shakehand success");

    auto itFind = FindSyncService(req.m_strSID);
    if (NULL == itFind.get())
    {
        LOG_ERROR_RLD("Find syncservice failed and srcid is " << strSrcID << " and sid is " << req.m_strSID);
        iRet = -1;
        strRetMsg = "shakehand session id not found.";
    }
    else
    {
        boost::unique_lock<boost::mutex> lock(itFind->m_SyncServiceMutex);
        itFind->m_Status = (SyncService::SyncServiceStatus)req.m_status;
        itFind->m_uiTMTickCount = 0;
        LOG_INFO_RLD("Receive shakehand req and src id is " << strSrcID);
    }
    
    ProtoHandler::ShakehandRsp rsp;
    rsp.m_MsgType = ProtoHandler::MsgType::ShakehandRsp_T;
    rsp.m_uiMsgSeq = req.m_uiMsgSeq;
    rsp.m_strSID = req.m_strSID;
    rsp.m_iRetcode = iRet;
    rsp.m_strRetMsg = strRetMsg;
    rsp.m_strValue = req.m_strValue;

    std::string strOutput;
    m_pProtoHandler->SerializeRsp(rsp, strOutput);
    m_pClient->AsyncWrite("0", strSrcID, "0", strOutput.data(), strOutput.size(), true);

    return true;
}

bool ControlCenter::GetSyncAddressReqMsgHandler(const std::string &strData, const std::string &strSrcID)
{
    ProtoHandler::GetSyncAddressReq req;
    if (!m_pProtoHandler->UnSerializeReq(strData, req))
    {
        LOG_ERROR_RLD("Unserialize getsyncaddress req failed and srcid is " << strSrcID);
        return false;
    }

    int iRet = 0;
    std::string strRetMsg("getsyncaddress success");

    auto itFind = FindSyncService(req.m_strSID);
    if (NULL == itFind.get())
    {
        LOG_ERROR_RLD("Find syncservice failed and srcid is " << strSrcID << " and sid is " << req.m_strSID);
        iRet = -1;
        strRetMsg = "getsyncaddress session id not found.";
    }
    else
    {
        //boost::unique_lock<boost::mutex> lock(itFind->m_SyncServiceMutex);
        itFind->m_uiTMTickCount = 0;
        LOG_INFO_RLD("Receive getsyncaddress req and src id is " << strSrcID << " and strvalue is " << req.m_strValue);
    }

    ProtoHandler::GetSyncAddressRsp rsp;
    rsp.m_MsgType = ProtoHandler::MsgType::GetSyncAddressRsp_T;
    rsp.m_uiMsgSeq = req.m_uiMsgSeq;
    rsp.m_strSID = req.m_strSID;
    rsp.m_iRetcode = iRet;
    rsp.m_strRetMsg = strRetMsg;
    
    if (0 == iRet) //if success
    {
        boost::unique_lock<boost::mutex> lock(m_SyncServiceMapMutex);
        auto itBegin = m_SyncServiceMap.begin();
        auto itEnd = m_SyncServiceMap.end();
        while (itBegin != itEnd)
        {
            ProtoHandler::Address addr;
            addr.strAddress = itBegin->second->m_strAddress;
            addr.type = ProtoHandler::Address::NodeType::Sync;
            addr.AreaID = itBegin->second->m_strAreaID;
            addr.m_strStorageIP = itBegin->second->m_strStorageIP;
            addr.m_strStoragePort = itBegin->second->m_strStoragePort;
            addr.status = (ProtoHandler::SyncServiceStatus)itBegin->second->m_Status;

            rsp.m_AddressList.push_back(addr);

            ++itBegin;
        }
    }

    std::string strOutput;
    m_pProtoHandler->SerializeRsp(rsp, strOutput);
    m_pClient->AsyncWrite("0", strSrcID, "0", strOutput.data(), strOutput.size(), true);

    return true;
}

bool ControlCenter::SyncFileListPendingReqMsgHandler(const std::string &strData, const std::string &strSrcID)
{

    return true;
}

bool ControlCenter::ControlCMDReqMsgHandler(const std::string &strData, const std::string &strSrcID)
{

    return true;
}

bool ControlCenter::GetFileInfoReqMsgHandler(const std::string &strData, const std::string &strSrcID)
{

    return true;
}

void ControlCenter::AddSyncService(const std::string &strSID, boost::shared_ptr<SyncService> pSyncservice)
{
    boost::unique_lock<boost::mutex> lock(m_SyncServiceMapMutex);

    auto itFind = m_SyncServiceMap.find(strSID);
    if (m_SyncServiceMap.end() != itFind)
    {
        LOG_ERROR_RLD("SyncService map already exist sid " << strSID);
        return;
    }

    auto itBegin = m_SyncServiceMap.begin();
    auto itEnd = m_SyncServiceMap.end();
    while (itBegin != itEnd)
    {
        if (itBegin->second->m_strSyncServiceName == pSyncservice->m_strSyncServiceName)
        {
            itBegin->second->m_pTMHander->End();
            m_SyncServiceMap.erase(itBegin);
            break;
        }

        ++itBegin;
    }


    m_SyncServiceMap.insert(std::make_pair(strSID, pSyncservice));
    LOG_INFO_RLD("SyncService map add " << strSID);
}

void ControlCenter::DeleteSyncService(const std::string &strSID)
{
    boost::unique_lock<boost::mutex> lock(m_SyncServiceMapMutex);
    
    auto itFind = m_SyncServiceMap.find(strSID);
    if (m_SyncServiceMap.end() == itFind)
    {
        LOG_ERROR_RLD("SyncService map not exist sid " << strSID);
        return;
    }
    
    itFind->second->m_pTMHander->End();

    LOG_INFO_RLD("SyncService map delete " << strSID);

    m_SyncServiceMap.erase(itFind);

}

boost::shared_ptr<ControlCenter::SyncService> ControlCenter::FindSyncService(const std::string &strSID)
{
    boost::shared_ptr<ControlCenter::SyncService> pSyncService;

    boost::unique_lock<boost::mutex> lock(m_SyncServiceMapMutex);
    auto itFind = m_SyncServiceMap.find(strSID);
    if (m_SyncServiceMap.end() == itFind)
    {
        LOG_ERROR_RLD("SyncService map not find sid " << strSID);
        return pSyncService;
    }

    pSyncService = itFind->second;
    return pSyncService;
}

std::string ControlCenter::GetConfigInfo(const std::string &strSyncServiceName)
{
        
    Json::Value jsBody;
    jsBody["Test"] = "configvalue1";


    Json::FastWriter fastwriter;
    const std::string &strBody = fastwriter.write(jsBody);//jsBody.toStyledString();

    return strBody;
}

void ControlCenter::SyncService::TimeOutCB(const boost::system::error_code &ec)
{
    if (ec)
    {
        LOG_ERROR_RLD("Sync service timeout error, msg is " << ec.message());
        m_pTMHander.reset();
        return;
    }

    ++m_uiTMTickCount;

    if (m_uiMaxTickCount <= m_uiTMTickCount)
    {
        LOG_ERROR_RLD("SyncService timeout sid is " << m_strSID << " name is " << m_strSyncServiceName
            << " address is " << m_strAddress);

        m_TMFunc(); //call timeout handler

        m_pTMHander.reset();
    }

}
