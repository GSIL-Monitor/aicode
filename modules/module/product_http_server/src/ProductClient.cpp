#include "ProductClient.h"
#include "LogRLD.h"
#include "thrift/protocol/TMultiplexedProtocol.h"

ProductClient::ProductClient()
{
}


ProductClient::~ProductClient()
{
}

bool ProductClient::Init(const Param &paraminfo)
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
    boost::shared_ptr<TMultiplexedProtocol> mp1(new TMultiplexedProtocol(protocol, "ProductService"));
    m_pProductClient.reset(new ProductServiceClient(mp1));

    boost::shared_ptr<TMultiplexedProtocol> mp2(new TMultiplexedProtocol(protocol, "OrderService"));
    m_pOrderClient.reset(new OrderServiceClient(mp2));
    
    try
    {
        m_pTransport->open();

        return true;
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("Product client init was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return false;
    }
    catch (...)
    {
        LOG_INFO_RLD("Product client init was failed and error is unknown");
        return false;
    }

}

void ProductClient::Close()
{
    m_pTransport->flush();
    m_pTransport->close();
}

void ProductClient::AddProductType(AddProductTypeRT& _return, const std::string& strSid, const std::string& strUserID, const ProductType& pdttype)
{
    try
    {
        m_pProductClient->AddProductType(_return, strSid, strUserID, pdttype);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("AddProductType call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("AddProductType call was failed and error is unknown");
        return;
    }
}

void ProductClient::RemoveProductType(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strTypeID)
{
    try
    {
        m_pProductClient->RemoveProductType(_return, strSid, strUserID, strTypeID);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("RemoveProductType call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("RemoveProductType call was failed and error is unknown");
        return;
    }
}

void ProductClient::ModifyProductType(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const ProductType& pdttype)
{
    try
    {
        m_pProductClient->ModifyProductType(_return, strSid, strUserID, pdttype);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("ModifyProductType call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("ModifyProductType call was failed and error is unknown");
        return;
    }
}

void ProductClient::QueryProductType(QueryProductTypeRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strTypeID)
{
    try
    {
        m_pProductClient->QueryProductType(_return, strSid, strUserID, strTypeID);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("QueryProductType call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("QueryProductType call was failed and error is unknown");
        return;
    }
}

void ProductClient::QueryAllProductType(QueryAllProductTypeRT& _return, const std::string& strSid, const std::string& strUserID)
{
    try
    {
        m_pProductClient->QueryAllProductType(_return, strSid, strUserID);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("QueryAllProductType call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("QueryAllProductType call was failed and error is unknown");
        return;
    }
}

void ProductClient::AddOpenRequest(AddOpenRequestRT& _return, const std::string& strSid, const std::string& strUserID, const OpenRequest& opreq)
{
    try
    {
        m_pProductClient->AddOpenRequest(_return, strSid, strUserID, opreq);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("AddOpenRequest call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("AddOpenRequest call was failed and error is unknown");
        return;
    }
}

void ProductClient::RemoveOpenRequest(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOpReqID)
{
    try
    {
        m_pProductClient->RemoveOpenRequest(_return, strSid, strUserID, strOpReqID);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("RemoveOpenRequest call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("RemoveOpenRequest call was failed and error is unknown");
        return;
    }
}

void ProductClient::ModifyOpenRequest(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const OpenRequest& opreq)
{
    try
    {
        m_pProductClient->ModifyOpenRequest(_return, strSid, strUserID, opreq);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("ModifyOpenRequest call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("ModifyOpenRequest call was failed and error is unknown");
        return;
    }
}

void ProductClient::QueryOpenRequest(QueryOpenRequestRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strReqID,
    const std::string& strReqUserID, const std::string& strReqUserName)
{
    try
    {
        m_pProductClient->QueryOpenRequest(_return, strSid, strUserID, strReqID, strReqUserID, strReqUserName);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("QueryOpenRequest call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("QueryOpenRequest call was failed and error is unknown");
        return;
    }
}

void ProductClient::QueryAllOpenRequest(QueryAllOpenRequestRT& _return, const std::string& strSid, const std::string& strUserID, const QueryAllOpenRequestParam& qryparam)
{
    try
    {
        m_pProductClient->QueryAllOpenRequest(_return, strSid, strUserID, qryparam);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("QueryAllOpenRequest call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("QueryAllOpenRequest call was failed and error is unknown");
        return;
    }
}

void ProductClient::AddProduct(AddProductRT& _return, const std::string& strSid, const std::string& strUserID, const ProductInfo& pdt)
{
    try
    {
        m_pProductClient->AddProduct(_return, strSid, strUserID, pdt);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("AddProduct call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("AddProduct call was failed and error is unknown");
        return;
    }
}

void ProductClient::RemoveProduct(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID, const int iIncPpty)
{
    try
    {
        m_pProductClient->RemoveProduct(_return, strSid, strUserID, strPdtID, iIncPpty);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("RemoveProduct call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("RemoveProduct call was failed and error is unknown");
        return;
    }
}

void ProductClient::ModifyProduct(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID, const ProductInfo& pdt)
{
    try
    {
        m_pProductClient->ModifyProduct(_return, strSid, strUserID, strPdtID, pdt);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("ModifyProduct call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("ModifyProduct call was failed and error is unknown");
        return;
    }
}

void ProductClient::QueryProduct(QueryProductRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID)
{
    try
    {
        m_pProductClient->QueryProduct(_return, strSid, strUserID, strPdtID);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("QueryProduct call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("QueryProduct call was failed and error is unknown");
        return;
    }
}

void ProductClient::QueryAllProduct(QueryAllProductRT& _return, const std::string& strSid, const std::string& strUserID)
{
    try
    {
        m_pProductClient->QueryAllProduct(_return, strSid, strUserID);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("QueryAllProduct call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("QueryAllProduct call was failed and error is unknown");
        return;
    }
}

void ProductClient::AddProductProperty(AddProductPropertyRT& _return, const std::string& strSid, const std::string& strUserID, 
    const std::string& strPdtID, const ProductProperty& pdtppt)
{
    try
    {
        m_pProductClient->AddProductProperty(_return, strSid, strUserID, strPdtID, pdtppt);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("AddProductProperty call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("AddProductProperty call was failed and error is unknown");
        return;
    }
}

void ProductClient::RemoveProductProperty(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID, 
    const std::string& strPdtpptID)
{
    try
    {
        m_pProductClient->RemoveProductProperty(_return, strSid, strUserID, strPdtID, strPdtpptID);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("RemoveProductProperty call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("RemoveProductProperty call was failed and error is unknown");
        return;
    }
}



void ProductClient::AddOrd(AddOrdRT& _return, const std::string& strSid, const std::string& strUserID, const OrderInfo& ord)
{
    try
    {
        m_pOrderClient->AddOrd(_return, strSid, strUserID, ord);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("AddOrd call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("AddOrd call was failed and error is unknown");
        return;
    }
}

void ProductClient::RemoveOrd(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID, const int iIncDtl)
{
    try
    {
        m_pOrderClient->RemoveOrd(_return, strSid, strUserID, strOrdID, iIncDtl);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("RemoveOrd call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("RemoveOrd call was failed and error is unknown");
        return;
    }
}

void ProductClient::ModifyOrd(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID, const OrderInfo& ord)
{
    try
    {
        m_pOrderClient->ModifyOrd(_return, strSid, strUserID, strOrdID, ord);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("ModifyOrd call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("ModifyOrd call was failed and error is unknown");
        return;
    }
}

void ProductClient::QueryOrd(QueryOrdRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID)
{
    try
    {
        m_pOrderClient->QueryOrd(_return, strSid, strUserID, strOrdID);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("QueryOrd call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("QueryOrd call was failed and error is unknown");
        return;
    }
}

void ProductClient::QueryAllOrd(QueryAllOrdRT& _return, const std::string& strSid, const std::string& strUserID, const QueryAllOrdParam& qryparam)
{
    try
    {
        m_pOrderClient->QueryAllOrd(_return, strSid, strUserID, qryparam);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("QueryAllOrd call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("QueryAllOrd call was failed and error is unknown");
        return;
    }
}

void ProductClient::AddOrdDetail(AddOrdDetailRT& _return, const std::string& strSid, const std::string& strUserID, const OrderDetail& orddt)
{
    try
    {
        m_pOrderClient->AddOrdDetail(_return, strSid, strUserID, orddt);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("AddOrdDetail call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("AddOrdDetail call was failed and error is unknown");
        return;
    }
}

void ProductClient::RemoveOrdDetail(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID, const std::string& strOrddtID)
{
    try
    {
        m_pOrderClient->RemoveOrdDetail(_return, strSid, strUserID, strOrdID, strOrddtID);
    }
    catch (TTransportException te)
    {
        LOG_INFO_RLD("RemoveOrdDetail call was failed and error code is " << te.getType() << " and error msg is " << te.what());
        return;
    }
    catch (...)
    {
        LOG_INFO_RLD("RemoveOrdDetail call was failed and error is unknown");
        return;
    }
}
