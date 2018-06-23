#include "ClientComm.h"
#include "ProtoTxt.h"
#include "LogRLD.h"

typedef struct _StValue
{
    char *pBuffer;
    boost::shared_ptr<ClientComm> pClientComm;
    void *pValueAppend;
} StValue;

TCPClientEx ClientComm::ms_TCPClientEx;

ClientComm::ClientComm() : m_ClientConnectedCB(NULL), m_ClientRCB(NULL), m_ClientWCB(NULL)
{
    memset(m_cReadBuffer, 0, sizeof(m_cReadBuffer));
    m_pRemainBufferPos = m_cReadBuffer + sizeof(unsigned int);
    m_uiRemainBufferSize = sizeof(m_cReadBuffer) - sizeof(unsigned int);
}

ClientComm::~ClientComm()
{
    printf("client comm destruct.\n");
    LOG_INFO_RLD("Client comm destruct.");
}

boost::shared_ptr<ClientComm> ClientComm::Create(const char *pIPAddress, const char *pIPPort, const unsigned int uiSSLEnabled)
{
    boost::shared_ptr<TCPClient> pTcpclient = ms_TCPClientEx.CreateTCPClient(pIPAddress, pIPPort, uiSSLEnabled);

    ClientComm *pClient = new ClientComm();
    pClient->SetTcpClient(pTcpclient);

    pTcpclient->SetCallBack
        (
        boost::bind(&ClientComm::ConnectedCB, pClient, _1),
        boost::bind(&ClientComm::ReadCB, pClient, _1, _2, _3),
        boost::bind(&ClientComm::WriteCB, pClient, _1, _2, _3)
        );


    return boost::shared_ptr<ClientComm>(pClient);
}

void ClientComm::Run(const unsigned int uiThreadNum)
{
    ms_TCPClientEx.Run(uiThreadNum, false);
}

void ClientComm::Stop()
{
    ms_TCPClientEx.Stop();
}

void ClientComm::AsyncConnect()
{
    m_pTcpclient->AsyncConnect();
}

void ClientComm::AsyncWrite(const std::string &strSrcID, const std::string &strDstID, const std::string &strType, 
    const char *pContentBuffer, const unsigned int uiContentBufferLen, const bool IsNeedEncode, void *pValueAppend)
{
    unsigned int uiLen = IsNeedEncode ? 0 : uiContentBufferLen;
    char *pWriteBuffer = IsNeedEncode ? ProtoTxt::Pack(strSrcID, strDstID, strType, pContentBuffer, uiContentBufferLen, uiLen) : (char *)pContentBuffer;
        
    StValue *pValue = new StValue;
    pValue->pBuffer = pWriteBuffer;
    pValue->pClientComm = shared_from_this();
    pValue->pValueAppend = pValueAppend;

    m_pTcpclient->AsyncWrite(pWriteBuffer, uiLen, 0, (void *)pValue);
    
}

void ClientComm::AsyncRead(void *pValueAppend)
{
    StValue *pValue = new StValue;
    pValue->pBuffer = NULL;
    pValue->pClientComm = shared_from_this();
    pValue->pValueAppend = pValueAppend;

    m_pTcpclient->AsyncRead(m_pRemainBufferPos, m_uiRemainBufferSize, 0, (void *)pValue);
}

void ClientComm::Close()
{
    m_pTcpclient->Close();
}

void ClientComm::SetCallBack(ClientConnectedCB cccb, ClientReadCB rdcb, ClientWriteCB wtcb)
{
    m_ClientConnectedCB = cccb;
    m_ClientRCB = rdcb;
    m_ClientWCB = wtcb;

}

void ClientComm::SetTcpClient(boost::shared_ptr<TCPClient> pTcpclient)
{
    m_pTcpclient = pTcpclient;
}

void ClientComm::ConnectedCB(const boost::system::error_code &ec)
{
    if (NULL != m_ClientConnectedCB)
    {
        m_ClientConnectedCB(ec);
    }
}

void ClientComm::ReadCB(const boost::system::error_code &ec, std::size_t bytes_transferred, void *pValue)
{
    std::list<ClientMsg> TxtMsgList;
    if (!ec)
    {
        ProtoTxt::UnPack(TxtMsgList, m_cReadBuffer, sizeof(m_cReadBuffer), bytes_transferred, m_pRemainBufferPos, m_uiRemainBufferSize);
    }
    else
    {
        //error when reading
    }

    StValue *pStValue = (StValue*)pValue;

    if (NULL != m_ClientRCB)
    {
        m_ClientRCB(ec, &TxtMsgList, pStValue->pValueAppend);
    }
    
    delete pStValue;
    pStValue = NULL;

}

void ClientComm::WriteCB(const boost::system::error_code &ec, std::size_t bytes_transferred, void *pValue)
{
    StValue *pStValue = (StValue*)pValue;

    if (NULL != m_ClientWCB)
    {
        m_ClientWCB(ec, pStValue->pValueAppend);
    }
        
    delete[] pStValue->pBuffer;
    pStValue->pBuffer = NULL;

    delete pStValue;
    pStValue = NULL;

}

