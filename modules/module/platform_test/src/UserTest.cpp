#include "UserTest.h"
#include "LogRLD.h"
#include "NetComm.h"
#include "json/json.h"
#include "CommonUtility.h"
#include "InteractiveProtoHandler.h"
#include "boost/date_time.hpp"

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

    /////////循环注册用户
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
        ////首先注册用户
        //const std::string &strMsg = RegisterUsrReq();
        //if (strMsg.empty())
        //{
        //    LOG_ERROR_RLD("Register user msg is empty.");
        //    return;
        //}
        //m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);

        ////////////////////////////////////////////////////////
        g_strUserID = "A087FEC87BE5DF4784394F853F0EDF69";

        //用户登录
        const std::string &strMsg = LoginUsrReq();
        if (strMsg.empty())
        {
            LOG_ERROR_RLD("Login user req msg is empty.");
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

std::string UserTest::ModDevReq()
{
    InteractiveProtoHandler::ModifyDevReq_USR req;
    req.m_MsgType = InteractiveProtoHandler::MsgType::ModifyDevReq_USR_T;
    req.m_uiMsgSeq = 5;
    req.m_strSID = g_strUserSessionID;
    req.m_strUserID = g_strUserID;
    req.m_devInfo.m_strDevID = "5D1C115660F6704CB048DAB53B845D45";
    req.m_devInfo.m_strDevName = "ModifyDevName";
    req.m_devInfo.m_strDevPassword = "ModifyPwd";
    req.m_devInfo.m_uiStatus = 0xFFFFFFFF;
    req.m_devInfo.m_uiTypeInfo = 0xFFFFFFFF;
    
    std::string strSerializeOutPut;

    if (!m_pHandler->SerializeReq(req, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Modify device req serialize failed.");
        return "";
    }

    return strSerializeOutPut;

}

std::string UserTest::QueryDevReq()
{
    InteractiveProtoHandler::QueryDevReq_USR req;
    req.m_MsgType = InteractiveProtoHandler::MsgType::QueryDevReq_USR_T;
    req.m_uiMsgSeq = 5;
    req.m_strSID = g_strUserSessionID;
    req.m_strUserID = g_strUserID;
    req.m_uiBeginIndex = 0;
    req.m_strValue = "value";

    std::string strSerializeOutPut;

    if (!m_pHandler->SerializeReq(req, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Query device req serialize failed.");
        return "";
    }

    return strSerializeOutPut;
}

std::string UserTest::QueryUsrReq()
{
    InteractiveProtoHandler::QueryUserReq_USR req;
    req.m_MsgType = InteractiveProtoHandler::MsgType::QueryUserReq_USR_T;
    req.m_uiMsgSeq = 5;
    req.m_strSID = g_strUserSessionID;
    req.m_strDevID = "5D1C115660F6704CB048DAB53B845D45";
    req.m_uiBeginIndex = 0;
    req.m_strValue = "value";

    std::string strSerializeOutPut;

    if (!m_pHandler->SerializeReq(req, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Query user req serialize failed.");
        return "";
    }

    return strSerializeOutPut;

}

std::string UserTest::SharingDevReq()
{
    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));
    
    InteractiveProtoHandler::SharingDevReq_USR req;
    req.m_MsgType = InteractiveProtoHandler::MsgType::SharingDevReq_USR_T;
    req.m_uiMsgSeq = 6;
    req.m_strSID = g_strUserSessionID;
    
    req.m_relationInfo.m_strBeginDate = strCurrentTime;
    req.m_relationInfo.m_strEndDate = "2199-01-01 00:00:00";
    req.m_relationInfo.m_strDevID = "5D1C115660F6704CB048DAB53B845D45";
    req.m_relationInfo.m_strUserID = "DECC8BC2FF45F64DA223928B3FA14727";
    req.m_relationInfo.m_strValue = "value";
    req.m_relationInfo.m_uiRelation = 1;
    
    std::string strSerializeOutPut;

    if (!m_pHandler->SerializeReq(req, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Sharing device req serialize failed.");
        return "";
    }

    return strSerializeOutPut;

}

std::string UserTest::CancelSharedDevReq()
{
    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));

    InteractiveProtoHandler::CancelSharedDevReq_USR req;
    req.m_MsgType = InteractiveProtoHandler::MsgType::CancelSharedDevReq_USR_T;
    req.m_uiMsgSeq = 7;
    req.m_strSID = g_strUserSessionID;

    req.m_relationInfo.m_strBeginDate = strCurrentTime;
    req.m_relationInfo.m_strEndDate = "2199-01-01 00:00:00";
    req.m_relationInfo.m_strDevID = "5D1C115660F6704CB048DAB53B845D45";
    req.m_relationInfo.m_strUserID = "DECC8BC2FF45F64DA223928B3FA14727";
    req.m_relationInfo.m_strValue = "value";
    req.m_relationInfo.m_uiRelation = 1;

    std::string strSerializeOutPut;

    if (!m_pHandler->SerializeReq(req, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Cancel shared device req serialize failed.");
        return "";
    }

    return strSerializeOutPut;
}

std::string UserTest::QueryUsrInfoReq()
{
    InteractiveProtoHandler::QueryUsrInfoReq_USR req;
    req.m_MsgType = InteractiveProtoHandler::MsgType::QueryUsrInfoReq_USR_T;
    req.m_uiMsgSeq = 8;
    req.m_strSID = g_strUserSessionID;
    req.m_strUserID = "A087FEC87BE5DF4784394F853F0EDF69";
    req.m_strValue = "value";

    std::string strSerializeOutPut;

    if (!m_pHandler->SerializeReq(req, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Query user info req serialize failed.");
        return "";
    }

    return strSerializeOutPut;
}

std::string UserTest::QueryDevInfoReq()
{
    InteractiveProtoHandler::QueryDevInfoReq_USR req;
    req.m_MsgType = InteractiveProtoHandler::MsgType::QueryDevInfoReq_USR_T;
    req.m_uiMsgSeq = 8;
    req.m_strSID = g_strUserSessionID;
    req.m_strDevID = "5D1C115660F6704CB048DAB53B845D45";
    req.m_strValue = "value";

    std::string strSerializeOutPut;

    if (!m_pHandler->SerializeReq(req, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Query device info req serialize failed.");
        return "";
    }

    return strSerializeOutPut;

}

std::string UserTest::AddFriendsReq()
{
    InteractiveProtoHandler::AddFriendsReq_USR req;
    req.m_MsgType = InteractiveProtoHandler::MsgType::AddFriendsReq_USR_T;
    req.m_uiMsgSeq = 8;
    req.m_strSID = g_strUserSessionID;
    req.m_strUserID = g_strUserID;
    req.m_strFriendUserID = "950670E8CB0EF34F9FCA9C4C099971E5";
    
    std::string strSerializeOutPut;

    if (!m_pHandler->SerializeReq(req, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Add friends req serialize failed.");
        return "";
    }

    return strSerializeOutPut;

}

std::string UserTest::DelFriendsReq()
{
    InteractiveProtoHandler::DelFriendsReq_USR req;
    req.m_MsgType = InteractiveProtoHandler::MsgType::DelFriendsReq_USR_T;
    req.m_uiMsgSeq = 8;
    req.m_strSID = g_strUserSessionID;
    req.m_strUserID = g_strUserID;
    req.m_strFriendUserIDList.push_back("DECC8BC2FF45F64DA223928B3FA14727");
    req.m_strFriendUserIDList.push_back("950670E8CB0EF34F9FCA9C4C099971E5");


    std::string strSerializeOutPut;

    if (!m_pHandler->SerializeReq(req, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Delete friends req serialize failed.");
        return "";
    }

    return strSerializeOutPut;

}

std::string UserTest::QueryFriendsReq()
{
    InteractiveProtoHandler::QueryFriendsReq_USR req;
    req.m_MsgType = InteractiveProtoHandler::MsgType::QueryFriendsReq_USR_T;
    req.m_uiMsgSeq = 8;
    req.m_strSID = g_strUserSessionID;
    req.m_strUserID = g_strUserID;
    req.m_uiBeginIndex = 0;
    req.m_strValue = "value";


    std::string strSerializeOutPut;

    if (!m_pHandler->SerializeReq(req, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Query friends req serialize failed.");
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

        //用户登录
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

        //解析登录之后获得的该用户关联的设备列表
        unsigned int iLoop = 0;
        auto itBegin = LoginUsrRsp.m_reInfoList.begin();
        auto itEnd = LoginUsrRsp.m_reInfoList.end();
        while (itBegin != itEnd)
        {
            
            LOG_INFO_RLD("Device " << iLoop << "============");
            LOG_INFO_RLD("Query user id is " << itBegin->m_strUserID << " and device id is " << itBegin->m_strDevID <<
                " and begin date is " << itBegin->m_strBeginDate << " and end date is " << itBegin->m_strEndDate <<
                " and value is " << itBegin->m_strValue);
            
            ++iLoop;
            ++itBegin;
        }

        //查询好友
        const std::string &strMsg = QueryFriendsReq();
        if (strMsg.empty())
        {
            LOG_ERROR_RLD("Query friends req msg is empty.");
            return;
        }

        m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);

        ////删除好友
        //const std::string &strMsg = DelFriendsReq();
        //if (strMsg.empty())
        //{
        //    LOG_ERROR_RLD("Delete friends req msg is empty.");
        //    return;
        //}

        //m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);

        //// 添加好友
        //const std::string &strMsg = AddFriendsReq();
        //if (strMsg.empty())
        //{
        //    LOG_ERROR_RLD("Add friends req msg is empty.");
        //    return;
        //}

        //m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);

        //// 查询设备信息
        //const std::string &strMsg = QueryDevInfoReq();
        //if (strMsg.empty())
        //{
        //    LOG_ERROR_RLD("Query device info req msg is empty.");
        //    return;
        //}

        //m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);

        ////查询用户信息
        //const std::string &strMsg = QueryUsrInfoReq();
        //if (strMsg.empty())
        //{
        //    LOG_ERROR_RLD("Query user info req msg is empty.");
        //    return;
        //}

        //m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);


        ////设备关联用户查询
        //const std::string &strMsg = QueryUsrReq();
        //if (strMsg.empty())
        //{
        //    LOG_ERROR_RLD("Query user req msg is empty.");
        //    return;
        //}

        //m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);

        ////用户分享设备
        //const std::string &strMsg = SharingDevReq();
        //if (strMsg.empty())
        //{
        //    LOG_ERROR_RLD("Sharing device req msg is empty.");
        //    return;
        //}

        //m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);

        //{
        //    //用户取消分享设备
        //    const std::string &strMsg = CancelSharedDevReq();
        //    if (strMsg.empty())
        //    {
        //        LOG_ERROR_RLD("Sharing device req msg is empty.");
        //        return;
        //    }

        //    m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);
        //}
       

        ////用户查询设备
        //const std::string &strMsg = QueryDevReq();
        //if (strMsg.empty())
        //{
        //    LOG_ERROR_RLD("Query device req msg is empty.");
        //    return;
        //}

        //m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);


        ////用户修改设备
        //const std::string &strMsg = ModDevReq();
        //if (strMsg.empty())
        //{
        //    LOG_ERROR_RLD("Modify device req msg is empty.");
        //    return;
        //}

        //m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);
        
        ////用户握手
        //const std::string &strMsg = ShakehandReq();
        //if (strMsg.empty())
        //{
        //    LOG_ERROR_RLD("Shake user req msg is empty.");
        //    return;
        //}

        //m_pClient->AsyncWrite(g_strSessionID, "0", "0", strMsg.c_str(), strMsg.size(), true, pValue);


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

        ////用户删除设备
        //const std::string &strMsg = DelDevReq();
        //if (strMsg.empty())
        //{
        //    LOG_ERROR_RLD("Delete device req msg is empty.");
        //    return;
        //}


        //用户增加设备
        const std::string &strMsg = AddDevReq();
        if (strMsg.empty())
        {
            LOG_ERROR_RLD("Add device req msg is empty.");
            return;
        }

        ////用户登出
        //const std::string &strMsg = LogoutUsrReq();
        //if (strMsg.empty())
        //{
        //    LOG_ERROR_RLD("Logout user req msg is empty.");
        //    return;
        //}

        ////用户注销
        //const std::string &strMsg = UnregisterUsrReq();
        //if (strMsg.empty())
        //{
        //    LOG_ERROR_RLD("Unregister user req msg is empty.");
        //    return;
        //}
        //

        ////用户握手
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
        //用户增加设备
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
    else if (InteractiveProtoHandler::MsgType::ModifyDevRsp_USR_T == mtype)
    {
        InteractiveProtoHandler::ModifyDevRsp_USR rsp;
        if (!m_pHandler->UnSerializeReq(strMsgReceived, rsp))
        {
            LOG_ERROR_RLD("Modify device rsp unserialize failed.");
            return;
        }
        
        LOG_INFO_RLD("Modify device rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
            " and user session id is " << rsp.m_strSID);
    }
    else if (InteractiveProtoHandler::MsgType::QueryDevRsp_USR_T == mtype)
    {
        InteractiveProtoHandler::QueryDevRsp_USR rsp;
        if (!m_pHandler->UnSerializeReq(strMsgReceived, rsp))
        {
            LOG_ERROR_RLD("Query device rsp unserialize failed.");
            return;
        }

        //解析查询获得的该用户关联的设备列表
        unsigned int iLoop = 0;
        auto itBegin = rsp.m_allRelationInfoList.begin();
        auto itEnd = rsp.m_allRelationInfoList.end();
        while (itBegin != itEnd)
        {

            LOG_INFO_RLD("Query Device " << iLoop << "+++++++++++++++++++++++++++");
            LOG_INFO_RLD("Query user id is " << itBegin->m_strUserID << " and device id is " << itBegin->m_strDevID <<
                " and begin date is " << itBegin->m_strBeginDate << " and end date is " << itBegin->m_strEndDate <<
                " and value is " << itBegin->m_strValue);
            
            ++iLoop;
            ++itBegin;
        }
        
        LOG_INFO_RLD("Query device rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
            " and user session id is " << rsp.m_strSID);

    }
    else if (InteractiveProtoHandler::MsgType::QueryUserRsp_USR_T == mtype)
    {
        InteractiveProtoHandler::QueryUserRsp_USR rsp;
        if (!m_pHandler->UnSerializeReq(strMsgReceived, rsp))
        {
            LOG_ERROR_RLD("Query device rsp unserialize failed.");
            return;
        }

        //解析查询获得的该设备关联的关系列表
        unsigned int iLoop = 0;
        auto itBegin = rsp.m_allRelationInfoList.begin();
        auto itEnd = rsp.m_allRelationInfoList.end();
        while (itBegin != itEnd)
        {

            LOG_INFO_RLD("Query user " << iLoop << "*********************************");
            LOG_INFO_RLD("Query user id is " << itBegin->m_strUserID << " and device id is " << itBegin->m_strDevID <<
                " and begin date is " << itBegin->m_strBeginDate << " and end date is " << itBegin->m_strEndDate <<
                " and value is " << itBegin->m_strValue);

            ++iLoop;
            ++itBegin;
        }

        LOG_INFO_RLD("Query user rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
            " and user session id is " << rsp.m_strSID);

    }
    else if (InteractiveProtoHandler::MsgType::SharingDevRsp_USR_T == mtype)
    {
        InteractiveProtoHandler::SharingDevRsp_USR rsp;
        if (!m_pHandler->UnSerializeReq(strMsgReceived, rsp))
        {
            LOG_ERROR_RLD("Sharing device rsp unserialize failed.");
            return;
        }

        LOG_INFO_RLD("Sharing device rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
            " and user session id is " << rsp.m_strSID);

    }
    else if (InteractiveProtoHandler::MsgType::CancelSharedDevRsp_USR_T == mtype)
    {
        InteractiveProtoHandler::CancelSharedDevRsp_USR rsp;
        if (!m_pHandler->UnSerializeReq(strMsgReceived, rsp))
        {
            LOG_ERROR_RLD("Cancel shared device rsp unserialize failed.");
            return;
        }

        LOG_INFO_RLD("Cancel shared device rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
            " and user session id is " << rsp.m_strSID);
    }
    else if (InteractiveProtoHandler::MsgType::QueryDevInfoRsp_USR_T == mtype)
    {
        InteractiveProtoHandler::QueryDevInfoRsp_USR rsp;

        if (!m_pHandler->UnSerializeReq(strMsgReceived, rsp))
        {
            LOG_ERROR_RLD("Query device info rsp unserialize failed.");
            return;
        }

        LOG_INFO_RLD("Query device id is " << rsp.m_devInfo.m_strDevID << " device name is " << rsp.m_devInfo.m_strDevName <<
            " device pwd is " << rsp.m_devInfo.m_strDevPassword << " type info is " << rsp.m_devInfo.m_uiTypeInfo <<
            " create date is " << rsp.m_devInfo.m_strCreatedate << " status is " << rsp.m_devInfo.m_uiStatus <<
            " extend is " << rsp.m_devInfo.m_strExtend << " and inner info is " << rsp.m_devInfo.m_strInnerinfo);

        LOG_INFO_RLD("Query device info rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
            " and user session id is " << rsp.m_strSID);
    }
    else if (InteractiveProtoHandler::MsgType::QueryUsrInfoRsp_USR_T == mtype)
    {
        InteractiveProtoHandler::QueryUsrInfoRsp_USR rsp;

        if (!m_pHandler->UnSerializeReq(strMsgReceived, rsp))
        {
            LOG_ERROR_RLD("Query user info rsp unserialize failed.");
            return;
        }

        LOG_INFO_RLD("Query user id is " << rsp.m_userInfo.m_strUserID << " user name is " << rsp.m_userInfo.m_strUserName <<
            " user pwd is " << rsp.m_userInfo.m_strUserPassword << " type info is " << rsp.m_userInfo.m_uiTypeInfo <<
            " create date is " << rsp.m_userInfo.m_strCreatedate << " status is " << rsp.m_userInfo.m_uiStatus <<
            " extend is " << rsp.m_userInfo.m_strExtend);

        LOG_INFO_RLD("Query user info rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
            " and user session id is " << rsp.m_strSID);
    }
    else if (InteractiveProtoHandler::MsgType::MsgPreHandlerRsp_USR_T == mtype)
    {
        InteractiveProtoHandler::MsgPreHandlerRsp_USR rsp;
        if (!m_pHandler->UnSerializeReq(strMsgReceived, rsp))
        {
            LOG_ERROR_RLD("Msg prehandler rsp unserialize failed.");
            return;
        }

        LOG_INFO_RLD("Msg prehandler rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
            " and user session id is " << rsp.m_strSID);

    }
    else if (InteractiveProtoHandler::MsgType::AddFriendsRsp_USR_T == mtype)
    {
        InteractiveProtoHandler::AddFriendsRsp_USR rsp;
        if (!m_pHandler->UnSerializeReq(strMsgReceived, rsp))
        {
            LOG_ERROR_RLD("Add friends rsp unserialize failed.");
            return;
        }

        LOG_INFO_RLD("Add friends rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
            " and user session id is " << rsp.m_strSID);
    }
    else if (InteractiveProtoHandler::MsgType::DelFriendsRsp_USR_T == mtype)
    {
        InteractiveProtoHandler::DelFriendsRsp_USR rsp;
        if (!m_pHandler->UnSerializeReq(strMsgReceived, rsp))
        {
            LOG_ERROR_RLD("Delete friends rsp unserialize failed.");
            return;
        }

        LOG_INFO_RLD("Delete friends rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
            " and user session id is " << rsp.m_strSID);
    }
    else if (InteractiveProtoHandler::MsgType::QueryFriendsRsp_USR_T == mtype)
    {
        InteractiveProtoHandler::QueryFriendsRsp_USR rsp;

        if (!m_pHandler->UnSerializeReq(strMsgReceived, rsp))
        {
            LOG_ERROR_RLD("Query friends rsp unserialize failed.");
            return;
        }

        auto itBegin = rsp.m_allFriendUserIDList.begin();
        auto itEnd = rsp.m_allFriendUserIDList.end();
        while (itBegin != itEnd)
        {

            LOG_INFO_RLD("Query friend id is " << *itBegin);

            ++itBegin;
        }

        LOG_INFO_RLD("Query friends rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
            " and user session id is " << rsp.m_strSID);
    }    
    else
    {
        LOG_ERROR_RLD("Unknown message type is " << mtype);
        return;
    }
}
