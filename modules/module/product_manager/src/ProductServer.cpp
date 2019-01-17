#include "ProductServer.h"
#include "LogRLD.h"
#include "thrift/processor/TMultiplexedProcessor.h"
#include "ProductManager.h"
#include <type_traits>

template <typename T, typename U>
struct decay_equiv : std::is_same<typename std::decay<T>::type, U>::type{};

ProductServerHandler::ProductServerHandler(boost::shared_ptr<ProductServiceIf> pImpl) : m_pImpl(pImpl), m_bh(NULL), m_ah(NULL)
{

}

ProductServerHandler::~ProductServerHandler()
{

}

void ProductServerHandler::SetBeforeHandler(BeforeHandler bh)
{
    m_bh = bh;
}

void ProductServerHandler::SetAfterHandler(AfterHandler ah)
{
    m_ah = ah;
}

void ProductServerHandler::AddProductType(AddProductTypeRT& _return, const std::string& strSid, const std::string& strUserID, const ProductType& pdttype)
{
    BeforeProcess<AddProductTypeRT>(ADD_PRODUCT_TYPE_CMD, strSid, _return, Flag<decay_equiv<AddProductTypeRT, ProductRTInfo>::value>());

    m_pImpl->AddProductType(_return, strSid, strUserID, pdttype);

    AfterProcess(ADD_PRODUCT_TYPE_CMD, strSid);
}

void ProductServerHandler::RemoveProductType(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strTypeID)
{

    BeforeProcess<ProductRTInfo>(REMOVE_PRODUCT_TYPE_CMD, strSid, _return, Flag<decay_equiv<ProductRTInfo, ProductRTInfo>::value>());

    m_pImpl->RemoveProductType(_return, strSid, strUserID, strTypeID);

    AfterProcess(REMOVE_PRODUCT_TYPE_CMD, strSid);
}

void ProductServerHandler::ModifyProductType(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const ProductType& pdttype)
{
    BeforeProcess<ProductRTInfo>(MODIFY_PRODUCT_TYPE_CMD, strSid, _return, Flag<decay_equiv<ProductRTInfo, ProductRTInfo>::value>());

    m_pImpl->ModifyProductType(_return, strSid, strUserID, pdttype);

    AfterProcess(MODIFY_PRODUCT_TYPE_CMD, strSid);
}

void ProductServerHandler::QueryProductType(QueryProductTypeRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strTypeID)
{
    BeforeProcess<QueryProductTypeRT>(QUERY_PRODUCT_TYPE_CMD, strSid, _return, Flag<decay_equiv<QueryProductTypeRT, ProductRTInfo>::value>());

    m_pImpl->QueryProductType(_return, strSid, strUserID, strTypeID);

    AfterProcess(QUERY_PRODUCT_TYPE_CMD, strSid);
}

void ProductServerHandler::QueryAllProductType(QueryAllProductTypeRT& _return, const std::string& strSid, const std::string& strUserID)
{
    BeforeProcess<QueryAllProductTypeRT>(QUERY_ALL_PRODUCT_TYPE_CMD, strSid, _return, Flag<decay_equiv<QueryAllProductTypeRT, ProductRTInfo>::value>());

    m_pImpl->QueryAllProductType(_return, strSid, strUserID);

    AfterProcess(QUERY_ALL_PRODUCT_TYPE_CMD, strSid);
}

void ProductServerHandler::AddOpenRequest(AddOpenRequestRT& _return, const std::string& strSid, const std::string& strUserID, const OpenRequest& opreq)
{
    BeforeProcess<AddOpenRequestRT>(ADD_OPEN_REQUEST_CMD, strSid, _return, Flag<decay_equiv<AddOpenRequestRT, ProductRTInfo>::value>());

    m_pImpl->AddOpenRequest(_return, strSid, strUserID, opreq);

    AfterProcess(ADD_OPEN_REQUEST_CMD, strSid);
}

void ProductServerHandler::RemoveOpenRequest(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOpReqID)
{
    BeforeProcess<ProductRTInfo>(REMOVE_OPEN_REQUEST_CMD, strSid, _return, Flag<decay_equiv<ProductRTInfo, ProductRTInfo>::value>());

    m_pImpl->RemoveOpenRequest(_return, strSid, strUserID, strOpReqID);

    AfterProcess(REMOVE_OPEN_REQUEST_CMD, strSid);
}

void ProductServerHandler::ModifyOpenRequest(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const OpenRequest& opreq)
{
    BeforeProcess<ProductRTInfo>(MODIFY_OPEN_REQUEST_CMD, strSid, _return, Flag<decay_equiv<ProductRTInfo, ProductRTInfo>::value>());

    m_pImpl->ModifyOpenRequest(_return, strSid, strUserID, opreq);

    AfterProcess(MODIFY_OPEN_REQUEST_CMD, strSid);
}

void ProductServerHandler::QueryOpenRequest(QueryOpenRequestRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strReqID, const std::string& strReqUserID, const std::string& strReqUserName)
{
    BeforeProcess<QueryOpenRequestRT>(QUERY_OPEN_REQUEST_CMD, strSid, _return, Flag<decay_equiv<QueryOpenRequestRT, ProductRTInfo>::value>());

    m_pImpl->QueryOpenRequest(_return, strSid, strUserID, strReqID, strReqUserID, strReqUserName);

    AfterProcess(QUERY_OPEN_REQUEST_CMD, strSid);
}

void ProductServerHandler::QueryAllOpenRequest(QueryAllOpenRequestRT& _return, const std::string& strSid, const std::string& strUserID, const QueryAllOpenRequestParam& qryparam)
{
    BeforeProcess<QueryAllOpenRequestRT>(QUERY_ALL_OPEN_REQUEST_CMD, strSid, _return, Flag<decay_equiv<QueryAllOpenRequestRT, ProductRTInfo>::value>());

    m_pImpl->QueryAllOpenRequest(_return, strSid, strUserID, qryparam);

    AfterProcess(QUERY_ALL_OPEN_REQUEST_CMD, strSid);
}

void ProductServerHandler::AddProduct(AddProductRT& _return, const std::string& strSid, const std::string& strUserID, const ProductInfo& pdt)
{
    
    if (NULL != m_bh)
    {
        int iRet = 0;
        if (!m_bh(ADD_PRODUCT_CMD, strSid, &iRet))
        {
            LOG_ERROR_RLD("Add product before handler failed and sid is " << strSid << " and retcode is " << iRet);

            ProductRTInfo rtd;
            rtd.__set_iRtCode(iRet);
            rtd.__set_strRtMsg("Before handler failed.");
            _return.__set_rtcode(rtd);

            return;
        }
    }

    m_pImpl->AddProduct(_return, strSid, strUserID, pdt);

    if (NULL != m_ah)
    {
        m_ah(ADD_PRODUCT_CMD, strSid);
    }
}

void ProductServerHandler::RemoveProduct(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID)
{
    if (NULL != m_bh)
    {
        int iRet = 0;
        if (!m_bh(REMOVE_PRODUCT_CMD, strSid, &iRet))
        {
            LOG_ERROR_RLD("Remove product before handler failed and sid is " << strSid);

            _return.__set_iRtCode(iRet);
            _return.__set_strRtMsg("Before handler failed.");
            return;
        }
    }

    m_pImpl->RemoveProduct(_return, strSid, strUserID, strPdtID);

    if (NULL != m_ah)
    {
        m_ah(REMOVE_PRODUCT_CMD, strSid);
    }
}

void ProductServerHandler::ModifyProduct(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID, const ProductInfo& pdt)
{
    if (NULL != m_bh)
    {
        int iRet = 0;
        if (!m_bh(MODIFY_PRODUCT_CMD, strSid, &iRet))
        {
            LOG_ERROR_RLD("Modify product before handler failed and sid is " << strSid);

            _return.__set_iRtCode(iRet);
            _return.__set_strRtMsg("Before handler failed.");
            return;
        }
    }

    m_pImpl->ModifyProduct(_return, strSid, strUserID, strPdtID, pdt);

    if (NULL != m_ah)
    {
        m_ah(MODIFY_PRODUCT_CMD, strSid);
    }
}

void ProductServerHandler::QueryProduct(QueryProductRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID)
{
    if (NULL != m_bh)
    {
        int iRet = 0;
        if (!m_bh(QUERY_PRODUCT_CMD, strSid, &iRet))
        {
            LOG_ERROR_RLD("Query product before handler failed and sid is " << strSid);

            ProductRTInfo rtd;
            rtd.__set_iRtCode(iRet);
            rtd.__set_strRtMsg("Before handler failed.");
            _return.__set_rtcode(rtd);

            return;
        }
    }

    m_pImpl->QueryProduct(_return, strSid, strUserID, strPdtID);

    if (NULL != m_ah)
    {
        m_ah(QUERY_PRODUCT_CMD, strSid);
    }
}

void ProductServerHandler::QueryAllProduct(QueryAllProductRT& _return, const std::string& strSid, const std::string& strUserID)
{
    if (NULL != m_bh)
    {
        int iRet = 0;
        if (!m_bh(QUERY_ALL_PRODUCT_CMD, strSid, &iRet))
        {
            LOG_ERROR_RLD("Query all product before handler failed and sid is " << strSid);

            ProductRTInfo rtd;
            rtd.__set_iRtCode(iRet);
            rtd.__set_strRtMsg("Before handler failed.");
            _return.__set_rtcode(rtd);

            return;
        }
    }

    m_pImpl->QueryAllProduct(_return, strSid, strUserID);

    if (NULL != m_ah)
    {
        m_ah(QUERY_ALL_PRODUCT_CMD, strSid);
    }
}

void ProductServerHandler::AddProductProperty(AddProductPropertyRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID, 
    const ProductProperty& pdtppt)
{
    if (NULL != m_bh)
    {
        int iRet = 0;
        if (!m_bh(ADD_PRODUCT_PROPERTY_CMD, strSid, &iRet))
        {
            LOG_ERROR_RLD("Add product property before handler failed and sid is " << strSid);

            ProductRTInfo rtd;
            rtd.__set_iRtCode(iRet);
            rtd.__set_strRtMsg("Before handler failed.");
            _return.__set_rtcode(rtd);

            return;
        }
    }

    m_pImpl->AddProductProperty(_return, strSid, strUserID, strPdtID, pdtppt);

    if (NULL != m_ah)
    {
        m_ah(ADD_PRODUCT_PROPERTY_CMD, strSid);
    }
}

void ProductServerHandler::RemoveProductProperty(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID, 
    const std::string& strPdtpptID)
{
    if (NULL != m_bh)
    {
        int iRet = 0;
        if (!m_bh(REMOVE_PRODUCT_PROPERTY_CMD, strSid, &iRet))
        {
            LOG_ERROR_RLD("Remove product property before handler failed and sid is " << strSid);

            _return.__set_iRtCode(iRet);
            _return.__set_strRtMsg("Before handler failed.");
            return;
        }
    }

    m_pImpl->RemoveProductProperty(_return, strSid, strUserID, strPdtID, strPdtpptID);

    if (NULL != m_ah)
    {
        m_ah(REMOVE_PRODUCT_PROPERTY_CMD, strSid);
    }
}

template<typename T>
void ProductServerHandler::BeforeProcess(int iCmd, const std::string &strSid, T &_return, Flag<true> flag)
{
    if (NULL != m_bh)
    {
        int iRet = 0;
        if (!m_bh(iCmd, strSid, &iRet))
        {
            LOG_ERROR_RLD("Before handler failed and sid is " << strSid << " and retcode is " << iRet << " and cmd is " << iCmd);
            
            _return.__set_iRtCode(iRet);
            _return.__set_strRtMsg("Before handler failed.");
            
            return;
        }
    }
}

template<typename T>
void ProductServerHandler::BeforeProcess(int iCmd, const std::string &strSid, T &_return, Flag<false> flag)
{
    if (NULL != m_bh)
    {
        int iRet = 0;
        if (!m_bh(iCmd, strSid, &iRet))
        {
            LOG_ERROR_RLD("Before handler failed and sid is " << strSid << " and retcode is " << iRet << " and cmd is " << iCmd);
                        
            ProductRTInfo rtd;
            rtd.__set_iRtCode(iRet);
            rtd.__set_strRtMsg("Before handler failed.");
            _return.__set_rtcode(rtd);
            
            return;
        }
    }
}

void ProductServerHandler::AfterProcess(int iCmd, const std::string &strSid)
{
    if (NULL != m_ah)
    {
        m_ah(iCmd, strSid);
    }
}

OrderServerHandler::OrderServerHandler(boost::shared_ptr<OrderServiceIf> pImpl) : m_pImpl(pImpl), m_bh(NULL), m_ah(NULL)
{

}

OrderServerHandler::~OrderServerHandler()
{

}

void OrderServerHandler::SetBeforeHandler(BeforeHandler bh)
{
    m_bh = bh;
}

void OrderServerHandler::SetAfterHandler(AfterHandler ah)
{
    m_ah = ah;
}

void OrderServerHandler::AddOrd(AddOrdRT& _return, const std::string& strSid, const std::string& strUserID, const OrderInfo& ord)
{
    if (NULL != m_bh)
    {
        int iRet = 0;
        if (!m_bh(ADD_ORDER, strSid, &iRet))
        {
            LOG_ERROR_RLD("Add order before handler failed and sid is " << strSid);

            ProductRTInfo rtd;
            rtd.__set_iRtCode(iRet);
            rtd.__set_strRtMsg("Before handler failed.");
            _return.__set_rtcode(rtd);

            return;
        }
    }

    m_pImpl->AddOrd(_return, strSid, strUserID, ord);

    if (NULL != m_ah)
    {
        m_ah(ADD_ORDER, strSid);
    }
}

void OrderServerHandler::RemoveOrd(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID)
{
    if (NULL != m_bh)
    {
        int iRet = 0;
        if (!m_bh(REMOVE_ORDER, strSid, &iRet))
        {
            LOG_ERROR_RLD("Remove order before handler failed and sid is " << strSid);

            _return.__set_iRtCode(iRet);
            _return.__set_strRtMsg("Before handler failed.");
            return;
        }
    }

    m_pImpl->RemoveOrd(_return, strSid, strUserID, strOrdID);

    if (NULL != m_ah)
    {
        m_ah(REMOVE_ORDER, strSid);
    }
}

void OrderServerHandler::ModifyOrd(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID, const OrderInfo& ord)
{
    if (NULL != m_bh)
    {
        int iRet = 0;
        if (!m_bh(MODIFY_ORDER, strSid, &iRet))
        {
            LOG_ERROR_RLD("Modify order before handler failed and sid is " << strSid);

            _return.__set_iRtCode(iRet);
            _return.__set_strRtMsg("Before handler failed.");
            return;
        }
    }

    m_pImpl->ModifyOrd(_return, strSid, strUserID, strOrdID, ord);

    if (NULL != m_ah)
    {
        m_ah(MODIFY_ORDER, strSid);
    }
}

void OrderServerHandler::QueryOrd(QueryOrdRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID)
{
    if (NULL != m_bh)
    {
        int iRet = 0;
        if (!m_bh(QUERY_ORDER, strSid, &iRet))
        {
            LOG_ERROR_RLD("Query order before handler failed and sid is " << strSid);

            ProductRTInfo rtd;
            rtd.__set_iRtCode(iRet);
            rtd.__set_strRtMsg("Before handler failed.");
            _return.__set_rtcode(rtd);

            return;
        }
    }

    m_pImpl->QueryOrd(_return, strSid, strUserID, strOrdID);

    if (NULL != m_ah)
    {
        m_ah(QUERY_ORDER, strSid);
    }
}

void OrderServerHandler::QueryAllOrd(QueryAllOrdRT& _return, const std::string& strSid, const std::string& strUserID, const QueryAllOrdParam& qryparam)
{
    if (NULL != m_bh)
    {
        int iRet = 0;
        if (!m_bh(QUERY_ALL_ORDER, strSid, &iRet))
        {
            LOG_ERROR_RLD("Query all order before handler failed and sid is " << strSid);

            ProductRTInfo rtd;
            rtd.__set_iRtCode(iRet);
            rtd.__set_strRtMsg("Before handler failed.");
            _return.__set_rtcode(rtd);

            return;
        }
    }

    m_pImpl->QueryAllOrd(_return, strSid, strUserID, qryparam);

    if (NULL != m_ah)
    {
        m_ah(QUERY_ALL_ORDER, strSid);
    }
}


void OrderServerHandler::AddOrdDetail(AddOrdDetailRT& _return, const std::string& strSid, const std::string& strUserID, const OrderDetail& orddt)
{
    if (NULL != m_bh)
    {
        int iRet = 0;
        if (!m_bh(QUERY_ALL_ORDER, strSid, &iRet))
        {
            LOG_ERROR_RLD("Add order detail before handler failed and sid is " << strSid);

            ProductRTInfo rtd;
            rtd.__set_iRtCode(iRet);
            rtd.__set_strRtMsg("Before handler failed.");
            _return.__set_rtcode(rtd);

            return;
        }
    }

    m_pImpl->AddOrdDetail(_return, strSid, strUserID, orddt);

    if (NULL != m_ah)
    {
        m_ah(QUERY_ALL_ORDER, strSid);
    }
}

void OrderServerHandler::RemoveOrdDetail(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID, const std::string& strOrddtID)
{
    if (NULL != m_bh)
    {
        int iRet = 0;
        if (!m_bh(QUERY_ALL_ORDER, strSid, &iRet))
        {
            LOG_ERROR_RLD("Add order detail before handler failed and sid is " << strSid);

            _return.__set_iRtCode(iRet);
            _return.__set_strRtMsg("Before handler failed.");
            return;
        }
    }

    m_pImpl->RemoveOrdDetail(_return, strSid, strUserID, strOrdID, strOrddtID);

    if (NULL != m_ah)
    {
        m_ah(QUERY_ALL_ORDER, strSid);
    }
}

void OrderServerHandler::BeforeProcess(int iCmd, const std::string &strSid, int *piRet)
{

}

void OrderServerHandler::AfterProcess(int iCmd, const std::string &strSid)
{

}

ProductServer::ProductServer(boost::shared_ptr<ProductManager> pImpl) : m_pImpl(pImpl)
{

}

ProductServer::~ProductServer()
{

}

bool ProductServer::Init(const Param &paraminfo)
{
    m_pOrderServerHandler.reset(new OrderServerHandler(m_pImpl));
    m_pProductServerHandler.reset(new ProductServerHandler(m_pImpl));
    //boost::shared_ptr<TProcessor> processor(new ProductServiceProcessor(m_pProductServerHandler));

    boost::shared_ptr<TMultiplexedProcessor> processor(new TMultiplexedProcessor());
    processor->registerProcessor("ProductService", boost::shared_ptr<TProcessor>(new ProductServiceProcessor(m_pProductServerHandler)));
    processor->registerProcessor("OrderService", boost::shared_ptr<TProcessor>(new OrderServiceProcessor(m_pOrderServerHandler)));


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

void ProductServer::Run()
{
    try
    {
        m_pServer->serve();
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("Product server listen was failed and error code is " << te.getType() << " and error msg is " << te.what());
    }
    catch (...)
    {
        LOG_INFO_RLD("Product client init was failed and error is unknown");
    }
    
}

boost::shared_ptr<ProductServerHandler> ProductServer::GetProductServerHandler()
{
    return m_pProductServerHandler;
}

boost::shared_ptr<OrderServerHandler> ProductServer::GetOrderServerHandler()
{
    return m_pOrderServerHandler;
}

