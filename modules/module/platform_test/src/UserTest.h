#ifndef _USER_TEST_
#define _USER_TEST_

#include "boost/enable_shared_from_this.hpp"
#include "ClientCommInterface.h"

class InteractiveProtoHandler;

class UserTest : public boost::enable_shared_from_this<UserTest>
{
public:
    UserTest();
    ~UserTest();

    void ConnectCB(const boost::system::error_code &ec);

    void WriteCB(const boost::system::error_code &ec, void *pValue);


    void ReadCB(const boost::system::error_code &ec, std::list<ClientMsg> *pClientMsgList, void *pValue);

    void init(const char *pIpAddress, const char *pIpPort, const unsigned int uiShakehandInterval);
    void Close();

    std::string RegisterUsrReq();
    bool RegisterUsrRsp(const std::string &strMsg);

private:
    ClientCommInterface *m_pClient;

    boost::shared_ptr<InteractiveProtoHandler> m_pHandler;

};


#endif
