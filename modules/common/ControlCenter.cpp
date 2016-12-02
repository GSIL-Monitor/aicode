#include "ControlCenter.h"
#include "InteractiveProtoHandler.h"
#include "LogRLD.h"

std::string ConvertCharValueToLex(unsigned char *pInValue, const boost::uint32_t uiSize);

ControlCenter::ControlCenter(const ParamInfo &pinfo) : m_pProtoHandler(new InteractiveProtoHandler), m_ParamInfo(pinfo), m_MsgHandlerRunner(1)
{


}

ControlCenter::~ControlCenter()
{

}

void ControlCenter::Run(const bool isWaitRunFinished)
{
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
    m_MsgHandlerRunner.Stop();
    m_pClient->Close();
    ClientCommInterface::Stop();
}

void ControlCenter::SetupMsgHandler(const int iMsgType, MsgHandler msghandler)
{
    m_MsgHandlerMap.insert(std::make_pair(iMsgType, msghandler));
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

void ControlCenter::MsgWrite(const std::string &strDstID, const std::string &strDataToBeWriting)
{
    //char *pBuffer = new char[strDataToBeWriting.size()];
    //memcpy(pBuffer, strDataToBeWriting.data(), strDataToBeWriting.size());
    m_pClient->AsyncWrite("0", strDstID, "0", strDataToBeWriting.data(), strDataToBeWriting.size(), true);
}

bool ControlCenter::ReceiveMsgHandler(const std::string &strData, const std::string &strSrcID, void *pValue)
{
    InteractiveProtoHandler::MsgType mtype;
    if (!m_pProtoHandler->GetMsgType(strData, mtype))
    {
        LOG_ERROR_RLD("Get msg type failed.");
        return false;
    }
    
    LOG_INFO_RLD("Receive msg type is " << mtype);

    auto itFind = m_MsgHandlerMap.find(mtype);
    if (m_MsgHandlerMap.end() == itFind)
    {
        LOG_ERROR_RLD("Not found msg handler and type is " << mtype);
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

void ControlCenter::ReceiveMsgHandlerInner(MsgHandler MsgHdr, const std::string &strData, const std::string &strSrcID, const int iMsgType, void *pValue)
{
    if (!MsgHdr(strData, strSrcID, boost::bind(&ControlCenter::MsgWrite, this, _1, _2)))
    {
        LOG_ERROR_RLD("Unserializer handler failed and type is " << iMsgType << " src id is " << strSrcID);
        //m_pClient->AsyncRead(pValue); //当处理失败时需要考虑是否需要继续接收消息
    }
    else
    {
        LOG_INFO_RLD("Unserializer handler success and type is " << iMsgType << " src id is " << strSrcID);
    }

    //m_pClient->AsyncRead(pValue);

}

//bool ControlCenter::LoginReqMsgHandler(const std::string &strData, const std::string &strSrcID)
//{
//    ProtoHandler::LoginReq req;
//    if (!m_pProtoHandler->UnSerializeReq(strData, req))
//    {
//        LOG_ERROR_RLD("Unserialize login req failed and srcid is " << strSrcID);
//        return false;
//    }
//
//    LOG_INFO_RLD("login req info, syncservice name is " << req.m_strSyncServiceName << ", storage ip is " << req.m_strStorageIP << ", storage port is " 
//        << req.m_strStoragePort);
//
//    uuid_t uu;
//    uuid_generate(uu);
//    const std::string &strUUID = ConvertCharValueToLex((unsigned char *)uu, sizeof(uu));
//    
//    ProtoHandler::LoginRsp rsp;
//    rsp.m_MsgType = ProtoHandler::MsgType::LoginRsp_T;
//    rsp.m_uiMsgSeq = req.m_uiMsgSeq;
//    rsp.m_strSID = strUUID;
//    rsp.m_iRetcode = 0;
//    rsp.m_strRetMsg = "success";
//    
//    rsp.m_strLoginSID = strUUID;
//
//
//    boost::shared_ptr<SyncService> pSyncService(new SyncService);
//    pSyncService->m_strSID = rsp.m_strSID;
//    pSyncService->m_strSyncServiceName = req.m_strSyncServiceName;
//    pSyncService->m_strPassword = req.m_strPassword;
//    pSyncService->m_strStorageIP = req.m_strStorageIP;
//    pSyncService->m_strStoragePort = req.m_strStoragePort;
//    pSyncService->m_Status = SyncService::SyncServiceStatus::IDLE;
//    pSyncService->m_strAddress = strSrcID;
//    pSyncService->m_Type = SyncService::NodeType::Sync;
//    pSyncService->m_strAreaID = "0";
//    pSyncService->m_uiTMTickCount = 0;
//    pSyncService->m_pTMHander = m_TMEX.Create(boost::bind(&SyncService::TimeOutCB, pSyncService, _1), m_ParamInfo.uiSyncShakehandTimeout);
//    pSyncService->m_uiMaxTickCount = m_ParamInfo.uiSyncShakehandTimeoutCount;
//    pSyncService->m_TMFunc = boost::bind(&ControlCenter::DeleteSyncService, this, pSyncService->m_strSID);
//
//    pSyncService->m_pTMHander->Begin();
//    
//    AddSyncService(pSyncService->m_strSID, pSyncService);
//
//    std::string strOutput;
//    m_pProtoHandler->SerializeRsp(rsp, strOutput);
//
//    m_pClient->AsyncWrite("0", strSrcID, "0", strOutput.data(), strOutput.size(), true);
//
//    return true;
//}
