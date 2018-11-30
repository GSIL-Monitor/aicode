#include "TransClient.h"
#include "LogRLD.h"

TransClient::TransClient() : m_ReqHder(NULL), m_RspHder(NULL)
{
}


TransClient::~TransClient()
{
}

bool TransClient::Init(const Param &paraminfo)
{
    m_paraminfo = paraminfo;

    boost::shared_ptr<TSocket> socket(new TSocket(paraminfo.m_strServerIp, paraminfo.m_iServerPort));
    
    //设置发送、接收、连接超时
    socket->setConnTimeout(paraminfo.m_iConnectTimeout);
    socket->setRecvTimeout(paraminfo.m_iReceiveTimeout);
    socket->setSendTimeout(paraminfo.m_iSendTimeout);

    //对接nonblockingServer时必须的，对普通server端时用boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    //boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
    //boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));    
    m_pTransport.reset(new TFramedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(m_pTransport));
    
    m_pTransFuncClient.reset(new TransFuncClient(protocol));    

    try
    {        
        m_pTransport->open();

        return true;
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("TransClient init was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return false;
    }
    catch (...)
    {
        LOG_INFO_RLD("TransClient init was failed and error is unknown");
        return false;
    }

}

void TransClient::Close()
{
    m_pTransport->flush();
    m_pTransport->close();
}

bool TransClient::Call(bool blOnlySend)
{
    if (NULL == m_ReqHder)
    {
        LOG_ERROR_RLD("Req handler is null.");
        return false;
    }
    
    bool blResult = false;

    TReqInfo treq;
    std::string strContent;
    if (!(blResult = m_ReqHder(strContent)))
    {
        LOG_ERROR_RLD("Req handler call failed.");
        return false;
    }
    treq.__set_req(strContent);
    
    try
    {
        if (blOnlySend)
        {
            m_pTransFuncClient->SendMsg(treq);
        }
        else
        {
            TReturnInfo tretinfo;
            m_pTransFuncClient->ProcessMsg(tretinfo, treq);

            if (NULL != m_RspHder)
            {
                blResult = m_RspHder(tretinfo.rspinfo.rsp, (unsigned int)tretinfo.rtcode); //直接使用业务响应函数的返回值
            }
        }
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("TransClient call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return false;
    }
    catch (...)
    {
        LOG_INFO_RLD("TransClient call was failed and error is unknown");
        return false;
    }

    return blResult;
}
