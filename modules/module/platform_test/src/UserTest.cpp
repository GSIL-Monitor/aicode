#include "UserTest.h"
#include "LogRLD.h"
#include "NetComm.h"
#include "json/json.h"
#include "CommonUtility.h"
#include "InteractiveProtoHandler.h"

static std::string g_strSessionID;

UserTest::UserTest() : m_pClient(NULL), m_pHandler(new InteractiveProtoHandler)
{
}


UserTest::~UserTest()
{
    printf("client obj destruct\n");
    LOG_INFO_RLD("client obj destruct.");

    delete m_pClient;
    m_pClient = NULL;
}

void UserTest::ConnectCB(const boost::system::error_code &ec)
{
    if (ec)
    {
        printf("connect error %s\n", ec.message().c_str());

        LOG_ERROR_RLD("connect error msg is " << ec.message());
        return;
    }

    boost::shared_ptr<UserTest> *pCObj = new boost::shared_ptr<UserTest>(shared_from_this());

    std::string strMsg = "body_yin";
    m_pClient->AsyncWrite("222", "222", "1", strMsg.c_str(), strMsg.size(), true, pCObj);

    printf("first msg of sending: %s\n", strMsg.c_str());
    LOG_INFO_RLD("connected and begin sending msg " << strMsg);

}

void UserTest::WriteCB(const boost::system::error_code &ec, void *pValue)
{
    //printf("write cb.\n");

    if (ec)
    {
        printf("write error %s\n", ec.message().c_str());
        LOG_ERROR_RLD("write cb error " << ec.message());

        //boost::shared_ptr<UserTest> *pInCObj = (boost::shared_ptr<UserTest> *)pValue;
        //delete pInCObj;
        //pInCObj = NULL;
        return;
    }

    //boost::shared_ptr<ClientObj> *pCObj = new boost::shared_ptr<ClientObj>(shared_from_this());

    m_pClient->AsyncRead(pValue);

    if (!g_strSessionID.empty())
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        const std::string &strMsg = RegisterUsrReq();
        m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);
        LOG_INFO_RLD("=======================");
    }

    //boost::shared_ptr<ClientObj> *pInCObj = (boost::shared_ptr<ClientObj> *)pValue;
    //delete pInCObj;
    //pInCObj = NULL;

}

void UserTest::ReadCB(const boost::system::error_code &ec, std::list<ClientMsg> *pClientMsgList, void *pValue)
{
    //printf("read cb.\n");

    if (ec)
    {
        printf("Receive error.\n");
        LOG_ERROR_RLD("Receive error :" << ec.message());

        //boost::shared_ptr<UserTest> *pInCObj = (boost::shared_ptr<UserTest> *)pValue;
        //delete pInCObj;
        //pInCObj = NULL;
        return;
    }

    if (pClientMsgList->empty() || NULL == pClientMsgList)
    {
        printf("client msg is empty.\n");
        LOG_ERROR_RLD("client msg is empty.");

        return;
    }

    bool IsNeedRegisterUsr = false;

    static bool IsSessionMsg = true;

    if (IsSessionMsg)
    {
        IsSessionMsg = false;

        IsNeedRegisterUsr = true;

        Json::Reader rd;
        Json::Value vd;
        ClientMsg &cmsg = pClientMsgList->front();

        const std::string &strMsg = Decode64((const unsigned char *)cmsg.pContentBuffer.get(), cmsg.uiContentBufferLen);

        LOG_INFO_RLD("First get session id msg is " << strMsg);

        if (!rd.parse(strMsg, vd, false))
        {
            printf("parse json error: %s\n", cmsg.pContentBuffer.get());
            return;
        }

        Json::Value sid = vd["UUID"];
        g_strSessionID = sid.asString();

    }

    //boost::shared_ptr<ClientObj> *pCObj = new boost::shared_ptr<ClientObj>(shared_from_this());

    if (IsNeedRegisterUsr)
    {
        const std::string &strMsg = RegisterUsrReq();
        m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);        
    }
    else
    {
        const std::string &strMsgReceived = std::string(pClientMsgList->front().pContentBuffer.get(), pClientMsgList->front().uiContentBufferLen);

        InteractiveProtoHandler::MsgType mtype;
        if (!m_pHandler->GetMsgType(strMsgReceived, mtype))
        {
            LOG_ERROR_RLD("Get msg type failed.");
            return;
        }

        LOG_INFO_RLD("Receive msg type is " << mtype);

        if (InteractiveProtoHandler::MsgType::RegisterUserRsp_USR_T == mtype)
        {
            InteractiveProtoHandler::RegisterUserRsp_USR RegUsrRsp;
            if (!m_pHandler->UnSerializeReq(strMsgReceived, RegUsrRsp))
            {
                LOG_ERROR_RLD("Register user rsp unserialize failed.");
                return;
            }

            LOG_INFO_RLD("Register rsp return code is " << RegUsrRsp.m_iRetcode << " return msg is " << RegUsrRsp.m_strRetMsg);

        }
    }

    



    //boost::shared_ptr<ClientObj> *pInCObj = (boost::shared_ptr<ClientObj> *)pValue;
    //delete pInCObj;
    //pInCObj = NULL;
}

void UserTest::init(const char *pIpAddress, const char *pIpPort, const unsigned int uiShakehandInterval)
{
    ClientCommInterface *pClient = ClientCommInterface::Create(pIpAddress, pIpPort, 0, uiShakehandInterval);

    pClient->SetCallBack
        (
        boost::bind(&UserTest::ConnectCB, this, _1),
        boost::bind(&UserTest::ReadCB, this, _1, _2, _3),
        boost::bind(&UserTest::WriteCB, this, _1, _2)
        );

    ClientCommInterface::Run(2);

    pClient->AsyncConnect();
    m_pClient = pClient;

}

void UserTest::Close()
{
    m_pClient->Close();
    ClientCommInterface::Stop();
}

std::string UserTest::RegisterUsrReq()
{
    InteractiveProtoHandler::RegisterUserReq_USR RegUsrReq;
    RegUsrReq.m_MsgType = InteractiveProtoHandler::MsgType::RegisterUserReq_USR_T;
    RegUsrReq.m_uiMsgSeq = 1;
    RegUsrReq.m_strSID = "ffffeeee";
    RegUsrReq.m_strValue = "value";
    RegUsrReq.m_userInfo.m_uiStatus = 0;
    RegUsrReq.m_userInfo.m_strUserID = "uid_test";
    RegUsrReq.m_userInfo.m_strUserName = "yinbin";
    RegUsrReq.m_userInfo.m_strUserPassword = "testpwd";
    RegUsrReq.m_userInfo.m_uiTypeInfo = 2;
    RegUsrReq.m_userInfo.m_strCreatedate = "2016-11-30";
    RegUsrReq.m_userInfo.m_strExtend = "ext_info";
    
    std::string strSerializeOutPut;
        
    if (!m_pHandler->SerializeReq(RegUsrReq, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Register user req serialize failed.");
        return "";
    }

    return strSerializeOutPut;
}
