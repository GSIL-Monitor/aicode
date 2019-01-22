#ifndef _PRODUCT_SERVER_
#define _PRODUCT_SERVER_

#define FORCE_BOOST_SMART_PTR

#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TNonblockingServerSocket.h>
#include <thrift/transport/TNonblockingServerTransport.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/BoostThreadFactory.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>

#include "boost/shared_ptr.hpp"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "ProductService.h"
#include "OrderService.h"
#include "ProductManager.h"
#include <string>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace ::apache::thrift::server;

using namespace Product::Service;
using namespace ::apache::thrift::concurrency;

typedef boost::function<bool(int, const std::string&, int *)> BeforeHandler; //参数依次是：命令码，会话ID， 返回码
typedef boost::function<void(int, const std::string&)> AfterHandler;

class ProductServerHandler : virtual public ProductServiceIf
{
public:
    ProductServerHandler(boost::shared_ptr<ProductServiceIf> pImpl);
    virtual ~ProductServerHandler();

    static const int ADD_PRODUCT_CMD = 0;
    static const int REMOVE_PRODUCT_CMD = 1;
    static const int MODIFY_PRODUCT_CMD = 2;
    static const int QUERY_PRODUCT_CMD = 3;
    static const int QUERY_ALL_PRODUCT_CMD = 4;
    static const int ADD_PRODUCT_PROPERTY_CMD = 5;
    static const int REMOVE_PRODUCT_PROPERTY_CMD = 6;

    static const int ADD_PRODUCT_TYPE_CMD = 7;
    static const int REMOVE_PRODUCT_TYPE_CMD = 8;
    static const int MODIFY_PRODUCT_TYPE_CMD = 9;
    static const int QUERY_PRODUCT_TYPE_CMD = 10;
    static const int QUERY_ALL_PRODUCT_TYPE_CMD = 11;

    static const int ADD_OPEN_REQUEST_CMD = 12;
    static const int REMOVE_OPEN_REQUEST_CMD = 13;
    static const int MODIFY_OPEN_REQUEST_CMD = 14;
    static const int QUERY_OPEN_REQUEST_CMD = 15;
    static const int QUERY_ALL_OPEN_REQUEST_CMD = 16;


    void SetBeforeHandler(BeforeHandler bh);
    void SetAfterHandler(AfterHandler ah);

    virtual void AddProductType(AddProductTypeRT& _return, const std::string& strSid, const std::string& strUserID, const ProductType& pdttype);
    virtual void RemoveProductType(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strTypeID);
    virtual void ModifyProductType(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const ProductType& pdttype);
    virtual void QueryProductType(QueryProductTypeRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strTypeID);
    virtual void QueryAllProductType(QueryAllProductTypeRT& _return, const std::string& strSid, const std::string& strUserID);
    virtual void AddOpenRequest(AddOpenRequestRT& _return, const std::string& strSid, const std::string& strUserID, const OpenRequest& opreq);
    virtual void RemoveOpenRequest(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOpReqID);
    virtual void ModifyOpenRequest(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const OpenRequest& opreq);
    virtual void QueryOpenRequest(QueryOpenRequestRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strReqID, 
        const std::string& strReqUserID, const std::string& strReqUserName);
    virtual void QueryAllOpenRequest(QueryAllOpenRequestRT& _return, const std::string& strSid, const std::string& strUserID, const QueryAllOpenRequestParam& qryparam);

    virtual void AddProduct(AddProductRT& _return, const std::string& strSid, const std::string& strUserID, const ProductInfo& pdt);
    virtual void RemoveProduct(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID, const int iIncPpty);
    virtual void ModifyProduct(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID, const ProductInfo& pdt);
    virtual void QueryProduct(QueryProductRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID);
    virtual void QueryAllProduct(QueryAllProductRT& _return, const std::string& strSid, const std::string& strUserID);
    virtual void AddProductProperty(AddProductPropertyRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID, 
        const ProductProperty& pdtppt);
    virtual void RemoveProductProperty(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID, 
        const std::string& strPdtpptID);

private:
    template<bool blFlag>
    struct Flag {};

    template<typename T>
    void BeforeProcess(int iCmd, const std::string &strSid, T &_return, Flag<true> flag);

    template<typename T>
    void BeforeProcess(int iCmd, const std::string &strSid, T &_return, Flag<false> flag);


    void AfterProcess(int iCmd, const std::string &strSid);

private:
    boost::shared_ptr<ProductServiceIf> m_pImpl;

    BeforeHandler m_bh;
    AfterHandler m_ah;
};

class OrderServerHandler : virtual public OrderServiceIf
{
public:
    OrderServerHandler(boost::shared_ptr<OrderServiceIf> pImpl);
    virtual ~OrderServerHandler();

    static const int ADD_ORDER = 1000;
    static const int REMOVE_ORDER = 1001;
    static const int MODIFY_ORDER = 1002;
    static const int QUERY_ORDER = 1003;
    static const int QUERY_ALL_ORDER = 1004;

    void SetBeforeHandler(BeforeHandler bh);
    void SetAfterHandler(AfterHandler ah);

    virtual void AddOrd(AddOrdRT& _return, const std::string& strSid, const std::string& strUserID, const OrderInfo& ord);
    virtual void RemoveOrd(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID, const int iIncDtl);
    virtual void ModifyOrd(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID, const OrderInfo& ord);
    virtual void QueryOrd(QueryOrdRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID);
    virtual void QueryAllOrd(QueryAllOrdRT& _return, const std::string& strSid, const std::string& strUserID, const QueryAllOrdParam& qryparam);

    virtual void AddOrdDetail(AddOrdDetailRT& _return, const std::string& strSid, const std::string& strUserID, const OrderDetail& orddt);
    virtual void RemoveOrdDetail(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID, const std::string& strOrddtID);

private:
    void BeforeProcess(int iCmd, const std::string &strSid, int *piRet);
    void AfterProcess(int iCmd, const std::string &strSid);

private:
    boost::shared_ptr<OrderServiceIf> m_pImpl;

    BeforeHandler m_bh;
    AfterHandler m_ah;
};


class ProductServer
{
public:
    struct Param
    {
        std::string m_strServerIp;
        int m_iServerPort;
        int m_iConnectTimeout;
        int m_iReceiveTimeout;
        int m_iSendTimeout;
        unsigned int m_uiThreadNum;
    };
    
    ProductServer(boost::shared_ptr<ProductManager> pImpl);
    ~ProductServer();

    bool Init(const Param &paraminfo);
    
    void Run();
    
    boost::shared_ptr<ProductServerHandler> GetProductServerHandler();
    boost::shared_ptr<OrderServerHandler> GetOrderServerHandler();

private:
    Param m_paraminfo;
    
    boost::shared_ptr<ProductServerHandler> m_pProductServerHandler;
    boost::shared_ptr<OrderServerHandler> m_pOrderServerHandler;

    boost::shared_ptr<TNonblockingServer> m_pServer;

    boost::shared_ptr<ProductManager> m_pImpl;
};


#endif