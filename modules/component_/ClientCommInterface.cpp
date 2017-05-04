#include "ClientCommInterface.h"
#include "ClientComm.h"

boost::shared_ptr<TimeOutHandlerEx> ClientCommInterface::sm_pTmEx;
boost::mutex ClientCommInterface::sm_TmExMutex;

ClientCommInterface::~ClientCommInterface()
{
    printf("client interface destruct.\n");
}

ClientCommInterface::ClientCommInterface(boost::shared_ptr<ClientComm> pClientComm, const unsigned int uiShakehandInterval) :
m_pClientComm(pClientComm)
{
    if (0 != uiShakehandInterval && (NULL != sm_pTmEx.get()))
    {
        m_Shakehandler.reset(new ShakehandHandler);
        m_Shakehandler->m_pClientComm = m_pClientComm;
        m_Shakehandler->m_uiDoShakehanding = 0;

        m_Shakehandler->m_pShakehandTM = sm_pTmEx->Create(boost::bind(&ShakehandHandler::ShakeHand, m_Shakehandler.get(), _1), uiShakehandInterval);
    }
}

ClientCommInterface * ClientCommInterface::Create(const char *pIPAddress, const char *pIPPort, const unsigned int uiSSLEnabled, const unsigned int uiShakehandInterval)
{
    if (0 < uiShakehandInterval && NULL == sm_pTmEx.get())
    {
        boost::unique_lock<boost::mutex> lock(sm_TmExMutex);
        if (NULL == sm_pTmEx.get())
        {
            sm_pTmEx.reset(new TimeOutHandlerEx);
        }
    }

    boost::shared_ptr<ClientComm> pClientComm = ClientComm::Create(pIPAddress, pIPPort, uiSSLEnabled);

    return new ClientCommInterface(pClientComm, uiShakehandInterval);
}

void ClientCommInterface::Run(const unsigned int uiThreadNum)
{
    ClientComm::Run(uiThreadNum);

    if (NULL != sm_pTmEx.get())
    {
        sm_pTmEx->Run(1);
    }
}

void ClientCommInterface::Stop()
{
    if (NULL != sm_pTmEx.get())
    {
        sm_pTmEx->Stop();
    }

    ClientComm::Stop();
}

void ClientCommInterface::AsyncConnect()
{
    m_pClientComm->AsyncConnect();
}

void ClientCommInterface::AsyncWrite(const std::string &strSrcID, const std::string &strDstID, const std::string &strType, 
    const char *pContentBuffer, const unsigned int uiContentBufferLen, const bool IsNeedEncode, void *pValue)
{
    if (NULL != m_Shakehandler.get())
    {
        m_Shakehandler->m_ChannelCommMutex.lock();
    }

    m_pClientComm->AsyncWrite(strSrcID, strDstID, strType, pContentBuffer, uiContentBufferLen, IsNeedEncode, pValue);
}

void ClientCommInterface::AsyncRead(void *pValue)
{
    m_pClientComm->AsyncRead(pValue);
}

void ClientCommInterface::Close()
{
    if (NULL != m_Shakehandler.get())
    {
        m_Shakehandler->m_pShakehandTM->End();
    }

    m_pClientComm->Close();
}

void ClientCommInterface::SetCallBack(ClientConnectedCB cccb, ClientReadCB rdcb, ClientWriteCB wtcb)
{
    if (NULL != m_Shakehandler.get())
    {
        m_Shakehandler->m_ClientConnectedCB = cccb;
        m_Shakehandler->m_ClientWriteCB = wtcb;
        m_Shakehandler->m_ClientReadCB = rdcb;
        m_pClientComm->SetCallBack
            (
            boost::bind(&ClientCommInterface::ShakehandHandler::ConnectedCBInner, m_Shakehandler, _1),
            boost::bind(&ClientCommInterface::ShakehandHandler::ReadCBInner, m_Shakehandler, _1, _2, _3),
            boost::bind(&ClientCommInterface::ShakehandHandler::WriteCBInner, m_Shakehandler, _1, _2)
            );
    }
    else
    {
        m_pClientComm->SetCallBack(cccb, rdcb, wtcb);
    }

}


ClientCommInterface::ShakehandHandler::ShakehandHandler()
{

}

ClientCommInterface::ShakehandHandler::~ShakehandHandler()
{    
    printf("shake hanler destruct.\n");
    
}

void ClientCommInterface::ShakehandHandler::ShakeHand(const boost::system::error_code &ec)
{
    if (ec)
    {
        printf("shake hand timer error : %s\n", ec.message().c_str());
        return;
    }

    const std::string strShakehandMsg("RG,9,0,0,0,0,0");
    char *pShakeMsg = new char[strShakehandMsg.size()];
    memcpy(pShakeMsg, strShakehandMsg.data(), strShakehandMsg.size());

    m_ChannelCommMutex.lock();
    m_uiDoShakehanding = 1;

    if (!m_pClientComm.expired())
    {
        boost::shared_ptr<ClientComm> pClientComm = m_pClientComm.lock();
        if (NULL != pClientComm.get())
        {
            pClientComm->AsyncWrite("0", "0", "0", pShakeMsg, strShakehandMsg.size(), false);
        }
    }
}

void ClientCommInterface::ShakehandHandler::ConnectedCBInner(const boost::system::error_code &ec)
{
    if (!ec && (NULL != m_pShakehandTM.get()))
    {
        //启动握手定时器
        m_pShakehandTM->Begin();
    }

    if (NULL != m_ClientConnectedCB)
    {
        m_ClientConnectedCB(ec);
    }
}

void ClientCommInterface::ShakehandHandler::WriteCBInner(const boost::system::error_code &ec, void *pValue)
{
    if (NULL != m_pShakehandTM.get())
    {
        if (m_uiDoShakehanding)
        {
            m_uiDoShakehanding = 0;
            m_ChannelCommMutex.unlock();
            return;
        }

        m_ChannelCommMutex.unlock();
    }

    if (NULL != m_ClientWriteCB)
    {
        m_ClientWriteCB(ec, pValue);
    }
}

void ClientCommInterface::ShakehandHandler::ReadCBInner(const boost::system::error_code &ec, std::list<ClientMsg> *pClientMsgList, void *pValue)
{
    if (NULL != m_pShakehandTM.get() && !ec && (NULL != pClientMsgList) && !pClientMsgList->empty()) //fileter shakehand msg
    {
        auto itBegin = pClientMsgList->begin();
        auto itEnd = pClientMsgList->end();
        while (itBegin != itEnd)
        {
            if (itBegin->strSrcID == "0" && itBegin->strDstID == "0" && itBegin->strType == "0" &&
                itBegin->uiContentBufferLen == 1 && (itBegin->pContentBuffer[0] == '0'))
            {                    
                pClientMsgList->erase(itBegin++);
                continue;
            }

            ++itBegin;
        }

        if (pClientMsgList->empty())
        {
            if (!m_pClientComm.expired())
            {
                boost::shared_ptr<ClientComm> pClientComm = m_pClientComm.lock();
                if (NULL != pClientComm.get())
                {
                    pClientComm->AsyncRead(pValue);
                }
            }

            return;
        }
    }

    if (NULL != m_ClientReadCB)
    {
        m_ClientReadCB(ec, pClientMsgList, pValue);
    }

}
