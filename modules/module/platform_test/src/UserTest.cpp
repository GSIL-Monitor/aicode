#include "UserTest.h"
#include "LogRLD.h"
#include "NetComm.h"
#include "json/json.h"
#include "CommonUtility.h"
#include "InteractiveProtoHandler.h"

static std::string g_strSessionID;
static std::string g_strUserID;
static std::string g_strUserSessionID;

UserTest::UserTest() : m_pClient(NULL), m_pHandler(new InteractiveProtoHandler), m_pRunner(new Runner(2))
{
    m_pRunner->Run();
}


UserTest::~UserTest()
{
    printf("client obj destruct\n");
    LOG_INFO_RLD("client obj destruct.");

    m_pRunner->Stop();

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

    /////////ѭ��ע���û�
    //if (!g_strSessionID.empty())
    //{
    //    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    //    const std::string &strMsg = RegisterUsrReq();
    //    m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);
    //    LOG_INFO_RLD("=======================");
    //}
    /////////







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
        //����ע���û�
        const std::string &strMsg = RegisterUsrReq();
        if (strMsg.empty())
        {
            LOG_ERROR_RLD("Register user msg is empty.");
            return;
        }
        m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);        
    }
    else
    {
        const std::string &strMsgReceived = std::string(pClientMsgList->front().pContentBuffer.get(), pClientMsgList->front().uiContentBufferLen);

        m_pRunner->Post(boost::bind(&UserTest::MsgProcess, this, strMsgReceived, pValue));

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

std::string UserTest::LoginUsrReq()
{
    InteractiveProtoHandler::LoginReq_USR LgUsrReq;
    LgUsrReq.m_MsgType = InteractiveProtoHandler::MsgType::LoginReq_USR_T;
    LgUsrReq.m_uiMsgSeq = 2;
    LgUsrReq.m_strSID = "xxxxx";
    LgUsrReq.m_strValue = "value";
    LgUsrReq.m_userInfo.m_strUserID = g_strUserID;
    LgUsrReq.m_userInfo.m_strUserName = "yinbin";
    LgUsrReq.m_userInfo.m_strUserPassword = "testpwd";
    LgUsrReq.m_userInfo.m_uiTypeInfo = 2;
    LgUsrReq.m_userInfo.m_strCreatedate = "2016-11-30";
    LgUsrReq.m_userInfo.m_uiStatus = 0;
    LgUsrReq.m_userInfo.m_strExtend = "login_extend";

    std::string strSerializeOutPut;

    if (!m_pHandler->SerializeReq(LgUsrReq, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Login user req serialize failed.");
        return "";
    }

    return strSerializeOutPut;
}

std::string UserTest::ShakehandReq()
{
    InteractiveProtoHandler::ShakehandReq_USR req;
    req.m_MsgType = InteractiveProtoHandler::MsgType::ShakehandReq_USR_T;
    req.m_uiMsgSeq = 3;
    req.m_strSID = g_strUserSessionID;
    req.m_strValue = "shakehandk_value";
    req.m_strUserID = g_strUserID;

    std::string strSerializeOutPut;

    if (!m_pHandler->SerializeReq(req, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Shakehand user req serialize failed.");
        return "";
    }

    return strSerializeOutPut;
}

std::string UserTest::LogoutUsrReq()
{
    InteractiveProtoHandler::LogoutReq_USR req;
    req.m_MsgType = InteractiveProtoHandler::MsgType::LogoutReq_USR_T;
    req.m_uiMsgSeq = 3;
    req.m_strSID = g_strUserSessionID;
    req.m_userInfo.m_strUserID = g_strUserID;
    
    std::string strSerializeOutPut;

    if (!m_pHandler->SerializeReq(req, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Shakehand user req serialize failed.");
        return "";
    }

    return strSerializeOutPut;
}

std::string UserTest::UnregisterUsrReq()
{
    InteractiveProtoHandler::UnRegisterUserReq_USR req;
    req.m_MsgType = InteractiveProtoHandler::MsgType::UnRegisterUserReq_USR_T;
    req.m_uiMsgSeq = 4;
    req.m_strSID = g_strUserSessionID;
    req.m_userInfo.m_strUserID = g_strUserID;

    //LOG_INFO_RLD("======g_strUserSessionID: " << g_strUserSessionID << " ===========g_strUserID: " << g_strUserID);

    std::string strSerializeOutPut;

    if (!m_pHandler->SerializeReq(req, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Unregister user req serialize failed.");
        return "";
    }

    return strSerializeOutPut;
}

std::string UserTest::AddDevReq()
{    
    InteractiveProtoHandler::AddDevReq_USR req;
    req.m_MsgType = InteractiveProtoHandler::MsgType::AddDevReq_USR_T;
    req.m_uiMsgSeq = 3;
    req.m_strSID = g_strUserSessionID;
    req.m_strUserID = g_strUserID;
    req.m_devInfo.m_strCreatedate = "2016-12-14 10:58:00";
    req.m_devInfo.m_strDevID = CreateUUID();
    req.m_devInfo.m_strDevName = CreateUUID();
    req.m_devInfo.m_strDevPassword = "test_devpwd";
    req.m_devInfo.m_strExtend = "test_devextend";
    req.m_devInfo.m_strInnerinfo = "test_inner";
    req.m_devInfo.m_strOwnerUserID = g_strUserID;
    req.m_devInfo.m_uiStatus = 0;
    req.m_devInfo.m_uiTypeInfo = 0;

    std::string strSerializeOutPut;

    if (!m_pHandler->SerializeReq(req, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Add device req serialize failed.");
        return "";
    }

    return strSerializeOutPut;

}

std::string UserTest::DelDevReq()
{
    InteractiveProtoHandler::DelDevReq_USR req;
    req.m_MsgType = InteractiveProtoHandler::MsgType::DelDevReq_USR_T;
    req.m_uiMsgSeq = 4;
    req.m_strSID = g_strUserSessionID;
    req.m_strUserID = g_strUserID;
    req.m_strDevIDList.push_back("20841AD4151B2E4E854D1EC8798D40D2");
    req.m_strDevIDList.push_back("C59D43E07729434492D77B44086B88D2");
    
    std::string strSerializeOutPut;

    if (!m_pHandler->SerializeReq(req, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Delete device req serialize failed.");
        return "";
    }

    return strSerializeOutPut;
}

void UserTest::MsgProcess(const std::string &strMsgReceived, void *pValue)
{
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

        g_strUserID = RegUsrRsp.m_strUserID;

        //�û���¼
        const std::string &strMsg = LoginUsrReq();
        if (strMsg.empty())
        {
            LOG_ERROR_RLD("Login user req msg is empty.");
            return;
        }

        m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);

        LOG_INFO_RLD("Register rsp return code is " << RegUsrRsp.m_iRetcode << " return msg is " << RegUsrRsp.m_strRetMsg <<
            " and user id is " << RegUsrRsp.m_strUserID);

    }
    else if (InteractiveProtoHandler::MsgType::LoginRsp_USR_T == mtype)
    {
        InteractiveProtoHandler::LoginRsp_USR LoginUsrRsp;
        if (!m_pHandler->UnSerializeReq(strMsgReceived, LoginUsrRsp))
        {
            LOG_ERROR_RLD("Login user rsp unserialize failed.");
            return;
        }

        g_strUserSessionID = LoginUsrRsp.m_strSID;

        //�û�����
        const std::string &strMsg = ShakehandReq();
        if (strMsg.empty())
        {
            LOG_ERROR_RLD("Shake user req msg is empty.");
            return;
        }

        m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);


        LOG_INFO_RLD("Login user rsp return code is " << LoginUsrRsp.m_iRetcode << " return msg is " << LoginUsrRsp.m_strRetMsg <<
            " and user session id is " << LoginUsrRsp.m_strSID);

    }
    else if (InteractiveProtoHandler::MsgType::ShakehandRsp_USR_T == mtype)
    {
        InteractiveProtoHandler::ShakehandRsp_USR rsp;
        if (!m_pHandler->UnSerializeReq(strMsgReceived, rsp))
        {
            LOG_ERROR_RLD("Shakehand user rsp unserialize failed.");
            return;
        }

        //�û�ɾ���豸
        const std::string &strMsg = DelDevReq();
        if (strMsg.empty())
        {
            LOG_ERROR_RLD("Delete device req msg is empty.");
            return;
        }


        ////�û������豸
        //const std::string &strMsg = AddDevReq();
        //if (strMsg.empty())
        //{
        //    LOG_ERROR_RLD("Add device req msg is empty.");
        //    return;
        //}

        ////�û��ǳ�
        //const std::string &strMsg = LogoutUsrReq();
        //if (strMsg.empty())
        //{
        //    LOG_ERROR_RLD("Logout user req msg is empty.");
        //    return;
        //}

        ////�û�ע��
        //const std::string &strMsg = UnregisterUsrReq();
        //if (strMsg.empty())
        //{
        //    LOG_ERROR_RLD("Unregister user req msg is empty.");
        //    return;
        //}
        //

        ////�û�����
        //boost::this_thread::sleep(boost::posix_time::seconds(1));
        //const std::string &strMsg = ShakehandReq();
        //if (strMsg.empty())
        //{
        //    LOG_ERROR_RLD("Shake user req msg is empty.");
        //    return;
        //}

        m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);

        LOG_INFO_RLD("Shakehand user rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
            " and user session id is " << rsp.m_strSID);

    }
    else if (InteractiveProtoHandler::MsgType::LogoutRsp_USR_T == mtype)
    {
        InteractiveProtoHandler::LogoutRsp_USR rsp;
        if (!m_pHandler->UnSerializeReq(strMsgReceived, rsp))
        {
            LOG_ERROR_RLD("Logout user rsp unserialize failed.");
            return;
        }

        LOG_INFO_RLD("Logout user rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
            " and user session id is " << rsp.m_strSID);

    }
    else if (InteractiveProtoHandler::MsgType::UnRegisterUserRsp_USR_T == mtype)
    {
        InteractiveProtoHandler::UnRegisterUserRsp_USR rsp;
        if (!m_pHandler->UnSerializeReq(strMsgReceived, rsp))
        {
            LOG_ERROR_RLD("Unregister user rsp unserialize failed.");
            return;
        }

        LOG_INFO_RLD("Unregister user rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
            " and user session id is " << rsp.m_strSID);

    }
    else if (InteractiveProtoHandler::MsgType::AddDevRsp_USR_T == mtype)
    {
        InteractiveProtoHandler::AddDevRsp_USR rsp;
        if (!m_pHandler->UnSerializeReq(strMsgReceived, rsp))
        {
            LOG_ERROR_RLD("Add device rsp unserialize failed.");
            return;
        }

        boost::this_thread::sleep(boost::posix_time::seconds(1));
        //�û������豸
        const std::string &strMsg = AddDevReq();
        if (strMsg.empty())
        {
            LOG_ERROR_RLD("Add device req msg is empty.");
            return;
        }
        m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);


        LOG_INFO_RLD("Add device rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
            " and user session id is " << rsp.m_strSID);
    }
    else if (InteractiveProtoHandler::MsgType::DelDevRsp_USR_T == mtype)
    {
        InteractiveProtoHandler::DelDevRsp_USR rsp;
        if (!m_pHandler->UnSerializeReq(strMsgReceived, rsp))
        {
            LOG_ERROR_RLD("Delete device rsp unserialize failed.");
            return;
        }

        

        LOG_INFO_RLD("Delete device rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
            " and user session id is " << rsp.m_strSID);

    }
    else
    {
        LOG_ERROR_RLD("Unknown message type is " << mtype);
        return;
    }
}
