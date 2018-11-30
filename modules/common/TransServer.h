#ifndef _TRANS_FUNC_SERVER_
#define _TRANS_FUNC_SERVER_

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
#include "TransFunc.h"
#include <string>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace ::apache::thrift::server;

using namespace Trans::Service;
using namespace ::apache::thrift::concurrency;


class TransFuncHandler : virtual public TransFuncIf
{
public:
    typedef boost::function<bool(const std::string &strBinaryContent, std::string &strResult)> ReqHandler;
    
    TransFuncHandler();
    virtual ~TransFuncHandler();

    virtual void ProcessMsg(TReturnInfo& _return, const TReqInfo& reqinfo);
    virtual void SendMsg(const TReqInfo& reqinfo);
    
    inline void SetReqHandler(ReqHandler reqhd)
    {
        m_ReqHder = reqhd;
    };

private:
    ReqHandler m_ReqHder;

};

class TransServer
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
    
    TransServer();
    ~TransServer();

    bool Init(const Param &paraminfo);
    
    void Run();
    
    inline boost::shared_ptr<TransFuncHandler> GetHandler()
    {
        return m_pTransFuncHandler;
    };

private:
    Param m_paraminfo;

    boost::shared_ptr<TransFuncHandler> m_pTransFuncHandler;
    boost::shared_ptr<TNonblockingServer> m_pServer;
};


#endif