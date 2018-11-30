#include "TransServer.h"
#include "LogRLD.h"

TransFuncHandler::TransFuncHandler() : m_ReqHder(NULL)
{

}

TransFuncHandler::~TransFuncHandler()
{

}

void TransFuncHandler::ProcessMsg(TReturnInfo& _return, const TReqInfo& reqinfo)
{
    if (NULL == m_ReqHder)
    {
        LOG_ERROR_RLD("Req handler is null.");
        return;
    }

    std::string strResult;    
    bool blResult = m_ReqHder(reqinfo.req, strResult);
    if (!blResult)
    {
        LOG_ERROR_RLD("Process msg failed.");
    }

    TRspInfo rsp;
    rsp.__set_rsp(strResult);

    _return.__set_rspinfo(rsp);
    _return.__set_rtcode(blResult ? TReturnCode::type::Success : TReturnCode::type::Failed);

}

void TransFuncHandler::SendMsg(const TReqInfo& reqinfo)
{
    if (NULL == m_ReqHder)
    {
        LOG_ERROR_RLD("Req handler is null.");
        return;
    }

    std::string strResult;
    bool blResult = m_ReqHder(reqinfo.req, strResult);
    if (!blResult)
    {
        LOG_ERROR_RLD("Process msg failed.");
    }

}

TransServer::TransServer()
{
}


TransServer::~TransServer()
{
}

bool TransServer::Init(const Param &paraminfo)
{
    m_pTransFuncHandler.reset(new TransFuncHandler());
    boost::shared_ptr<TProcessor> processor(new TransFuncProcessor(m_pTransFuncHandler));

    boost::shared_ptr<TNonblockingServerTransport> serverTransport(new TNonblockingServerSocket(paraminfo.m_iServerPort));

    //boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    
    boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
    boost::shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(paraminfo.m_uiThreadNum); //线程数量
    boost::shared_ptr<BoostThreadFactory> threadFactory(new BoostThreadFactory()); //使用posix线程模型
    threadManager->threadFactory(threadFactory);
    threadManager->start();
    
    m_pServer.reset(new TNonblockingServer(processor, protocolFactory, serverTransport, threadManager));

    //m_pServer.reset(new TThreadPoolServer(processor, serverTransport, transportFactory, protocolFactory, threadManager));
    //server.serve();

    LOG_INFO_RLD("Server init complete and thread num is " << paraminfo.m_uiThreadNum << " and port is " << paraminfo.m_iServerPort);

    return true;
}

void TransServer::Run()
{
    m_pServer->serve();
}

