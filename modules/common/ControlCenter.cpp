#include "ControlCenter.h"
#include "LogRLD.h"

std::string ConvertCharValueToLex(unsigned char *pInValue, const boost::uint32_t uiSize);

ControlCenter::ControlCenter(const ParamInfo &pinfo) : m_ParamInfo(pinfo), 
m_MsgHandlerRunner(pinfo.uiThreadOfWorking), m_MsgWriterRunner(1)
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
    m_MsgWriterRunner.Run();
    m_MsgHandlerRunner.Run(isWaitRunFinished);
        
}

void ControlCenter::Stop()
{
    m_MsgHandlerRunner.Stop();
    m_MsgWriterRunner.Stop();
    m_pClient->Close();
    ClientCommInterface::Stop();
}

void ControlCenter::SetupMsgHandler(const int iMsgType, MsgHandler msghandler)
{
    m_MsgHandlerMap.insert(std::make_pair(iMsgType, msghandler));
}

void ControlCenter::SetupMsgPreHandler(MsgHandler msghandler)
{
    m_MsgPreHandlerList.push_back(msghandler);
}

void ControlCenter::SetupMsgTypeParseHandler(MsgTypeHandler msgtypehdr)
{
    m_MsgTypeHandler = msgtypehdr;
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
        //return;
    }
    

    static bool IsAuthMsg = true;

    if (IsAuthMsg)
    {
        IsAuthMsg = false;
        m_pClient->AsyncRead(pValue);
        return;
    }
    
    m_MsgWriterMutex.unlock();

}

void ControlCenter::ReadCB(const boost::system::error_code &ec, std::list<ClientMsg> *pClientMsgList, void *pValue)
{
    LOG_INFO_RLD("pClientMsgList size is " << pClientMsgList->size());

    if (ec || (NULL == pClientMsgList) || pClientMsgList->empty())
    {
        LOG_ERROR_RLD("Read msg failed, error is " << ec.message() << " pClientMsgList is " << (NULL == pClientMsgList) <<
           " and pClientMsgList size is " << (NULL == pClientMsgList) ? 0 : pClientMsgList->size());

        if (ec)
        {
            Reconnect();
        }
        
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
        
    m_pClient->AsyncRead(pValue);
        
}

void ControlCenter::Reconnect()
{
    LOG_INFO_RLD("Reconnect to " << m_ParamInfo.strRemoteAddress << ":" << m_ParamInfo.strRemotePort);
    boost::this_thread::sleep(boost::posix_time::seconds(2));
    m_pClient->AsyncConnect();
}

void ControlCenter::MsgWrite(const std::string &strDstID, const std::string &strDataToBeWriting)
{
    m_MsgWriterRunner.Post(boost::bind(&ControlCenter::MsgWriteInner, this, strDstID, strDataToBeWriting));
    //m_pClient->AsyncWrite("0", strDstID, "0", strDataToBeWriting.data(), strDataToBeWriting.size(), true);
}

void ControlCenter::MsgWriteInner(const std::string &strDstID, const std::string &strDataToBeWriting)
{
    //LOG_INFO_RLD("MsgWriteInner before =================");
    m_MsgWriterMutex.lock();
    m_pClient->AsyncWrite("0", strDstID, "0", strDataToBeWriting.data(), strDataToBeWriting.size(), true);
    //LOG_INFO_RLD("MsgWriteInner after =================");
}

bool ControlCenter::ReceiveMsgHandler(const std::string &strData, const std::string &strSrcID, void *pValue)
{
    if (NULL == m_MsgTypeHandler)
    {
        LOG_ERROR_RLD("Get msg type handle failed.");
        return false;
    }

    int mtype = 0;
    if (!m_MsgTypeHandler(strData, mtype))
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
    auto itBegin = m_MsgPreHandlerList.begin();
    auto itEnd = m_MsgPreHandlerList.end();
    while (itBegin != itEnd)
    {
        if (!(*itBegin)(strData, strSrcID, boost::bind(&ControlCenter::MsgWrite, this, _1, _2)))
        {
            LOG_ERROR_RLD("Receive msg prehandler failed and type is " << iMsgType << " src id is " << strSrcID);
            return;
        }

        ++itBegin;
    }

    if (!MsgHdr(strData, strSrcID, boost::bind(&ControlCenter::MsgWrite, this, _1, _2)))
    {
        LOG_ERROR_RLD("Receive msg handler failed and type is " << iMsgType << " src id is " << strSrcID);
        //m_pClient->AsyncRead(pValue); //当处理失败时需要考虑是否需要继续接收消息
    }
    else
    {
        LOG_INFO_RLD("Receive msg handler success and type is " << iMsgType << " src id is " << strSrcID);
    }

}

