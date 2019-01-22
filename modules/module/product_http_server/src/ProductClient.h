#ifndef _PRODUCT_CLIENT_
#define _PRODUCT_CLIENT_

#define FORCE_BOOST_SMART_PTR

#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include "boost/shared_ptr.hpp"
#include "boost/bind.hpp"
#include "boost/function.hpp"

#include "ProductService.h"
#include "OrderService.h"
#include "Product_constants.h"
#include "Product_types.h"
#include <string>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace Product::Service;

/************************************************************************/
/* 每个请求/响应都产生一个该对象，代表了一个单独的连接到服务器侧 */
/* 支持单向发送和持续发送 */
/************************************************************************/
class ProductClient
{
public:
    struct Param 
    {
        std::string m_strServerIp;
        int m_iServerPort;
        int m_iConnectTimeout;
        int m_iReceiveTimeout;
        int m_iSendTimeout;
    };

    ProductClient();
    ~ProductClient();
    
    bool Init(const Param &paraminfo);
    void Close();

    void AddProductType(AddProductTypeRT& _return, const std::string& strSid, const std::string& strUserID, const ProductType& pdttype);
    void RemoveProductType(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strTypeID);
    void ModifyProductType(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const ProductType& pdttype);
    void QueryProductType(QueryProductTypeRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strTypeID);
    void QueryAllProductType(QueryAllProductTypeRT& _return, const std::string& strSid, const std::string& strUserID);

    void AddOpenRequest(AddOpenRequestRT& _return, const std::string& strSid, const std::string& strUserID, const OpenRequest& opreq);
    void RemoveOpenRequest(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOpReqID);
    void ModifyOpenRequest(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const OpenRequest& opreq);
    void QueryOpenRequest(QueryOpenRequestRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strReqID, const std::string& strReqUserID, const std::string& strReqUserName);
    void QueryAllOpenRequest(QueryAllOpenRequestRT& _return, const std::string& strSid, const std::string& strUserID, const QueryAllOpenRequestParam& qryparam);
    

    void AddProduct(AddProductRT& _return, const std::string& strSid, const std::string& strUserID, const ProductInfo& pdt);
    void RemoveProduct(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID, const int iIncPpty);
    void ModifyProduct(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID, const ProductInfo& pdt);
    void QueryProduct(QueryProductRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID);
    void QueryAllProduct(QueryAllProductRT& _return, const std::string& strSid, const std::string& strUserID);
    
    void AddProductProperty(AddProductPropertyRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID, const ProductProperty& pdtppt);
    void RemoveProductProperty(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID, const std::string& strPdtpptID);


    void AddOrd(AddOrdRT& _return, const std::string& strSid, const std::string& strUserID, const OrderInfo& ord);
    void RemoveOrd(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID, const int iIncDtl);
    void ModifyOrd(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID, const OrderInfo& ord);
    void QueryOrd(QueryOrdRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID);
    void QueryAllOrd(QueryAllOrdRT& _return, const std::string& strSid, const std::string& strUserID, const QueryAllOrdParam& qryparam);
    void AddOrdDetail(AddOrdDetailRT& _return, const std::string& strSid, const std::string& strUserID, const OrderDetail& orddt);
    void RemoveOrdDetail(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID, const std::string& strOrddtID);

private:
    Param m_paraminfo;

    boost::shared_ptr<ProductServiceClient> m_pProductClient;
    boost::shared_ptr<OrderServiceClient> m_pOrderClient;

    boost::shared_ptr<TTransport> m_pTransport;
};

#endif