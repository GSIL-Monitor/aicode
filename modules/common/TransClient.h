#ifndef _TRANS_FUNC_CLIENT_
#define _TRANS_FUNC_CLIENT_

#define FORCE_BOOST_SMART_PTR

#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include "boost/shared_ptr.hpp"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "TransFunc.h"
#include <string>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace Trans::Service;

/************************************************************************/
/* 每个请求/响应都产生一个该对象，代表了一个单独的连接到服务器侧 */
/* 支持单向发送和持续发送 */
/************************************************************************/
class TransClient
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

    typedef boost::function<bool(std::string &strBinaryContent)> ReqHandler;
    typedef boost::function<bool(const std::string &strBinaryContent, unsigned int uiRetCode)>  RspHandler;
    
    TransClient();
    ~TransClient();
    
    bool Init(const Param &paraminfo);
    void Close();

    bool Call(bool blOnlySend = false); //外部循环调用，可以实现连续发送，通过返回值来判断是否需要跳出外部循环

    inline void SetReqHandler(ReqHandler reqhd)
    {
        m_ReqHder = reqhd;
    };
    
    inline void SetRspHandler(RspHandler rsphd)
    {
        m_RspHder = rsphd;
    };

private:
    Param m_paraminfo;

    ReqHandler m_ReqHder;
    RspHandler m_RspHder;

    boost::shared_ptr<TransFuncClient> m_pTransFuncClient;
    boost::shared_ptr<TTransport> m_pTransport;
};

#endif