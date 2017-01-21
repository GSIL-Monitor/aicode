#include <boost/lexical_cast.hpp>
#include <boost/scope_exit.hpp>
#include "CommMsgHandler.h"
#include "CommonUtility.h"
#include "json/json.h"


boost::mutex CommMsgHandler::sm_TimeOutObjMutex;

unsigned int CommMsgHandler::sm_uiTimeOutObjInited = 0;

TimeOutHandlerEx CommMsgHandler::sm_TimeOutObj;

unsigned int CommMsgHandler::sm_uiTimeOutThreadNum = 2;

unsigned int CommMsgHandler::sm_uiCommThreadNum = 4;

CommMsgHandler::CommMsgHandler(const std::string &strID, const int iTimeout) :
m_strID(strID), m_iOperationStatus(SUCCEED), m_nTimeout(iTimeout)
{
    {
        boost::mutex::scoped_lock lock(sm_TimeOutObjMutex);
        if (!sm_uiTimeOutObjInited)
        {
            sm_uiTimeOutObjInited = 1;
            sm_TimeOutObj.Run(sm_uiTimeOutThreadNum);
            ClientCommInterface::Run(sm_uiCommThreadNum);
        }
    }

    //首先装配acc会话处理
    FuncRequest req = boost::bind(&CommMsgHandler::AccRequest, this, _1);
    FuncReply rsp = boost::bind(&CommMsgHandler::AccReply, this, _1);
    m_func.push(boost::bind(&CommMsgHandler::SyncSession, this, req, rsp));
}

CommMsgHandler::~CommMsgHandler()
{
    m_Client->Close();
}

int CommMsgHandler::Start(const std::string& strIpAddr, const std::string& strPort, unsigned int uiSSLEnabled, unsigned int  uiHandShakeInterval)
{
    int errcode = SUCCEED;

    m_Client.reset(ClientCommInterface::Create(strIpAddr.c_str(), strPort.c_str(), uiSSLEnabled, uiHandShakeInterval));

    m_Client->SetCallBack(
        boost::bind(&CommMsgHandler::ConnectCB, this, _1), 
        boost::bind(&CommMsgHandler::ReadCB, this, _1, _2, _3), 
        boost::bind(&CommMsgHandler::WriteCB, this, _1, _2));

    if ((errcode = SyncConnect()) != SUCCEED)
    {
        return errcode;
    }

    return StartTask(); 
}

void CommMsgHandler::SetReqAndRspHandler(FuncRequest req, FuncReply rsp)
{
    m_func.push(boost::bind(&CommMsgHandler::SyncSession, this, req, rsp));
}

void CommMsgHandler::SetTimeoutRunningThreads(const unsigned int uiThdNum)
{
    sm_uiTimeOutThreadNum = uiThdNum;
}

void CommMsgHandler::SetCommRunningThreads(const unsigned int uiThdNum)
{
    sm_uiCommThreadNum = uiThdNum;
}

int CommMsgHandler::AccRequest(SendWriter writer)
{
    Json::Value root;

    root["VER"] = "1.0";
    root["CMD"] = "LOGIN";
    root["SEQ"] = "";
    root["MS"] = "";
    root["MSG"] = "";
    root["UNAME"] = m_strID;
    root["PW"] = "";
    root["ENT"] = "";
    root["ST"] = "0";

    const std::string& strLoginBody = root.toStyledString();
    const std::string& strLoginBodyBase64 = Encode64((unsigned char const*)strLoginBody.c_str(), strLoginBody.length());
    return writer("0", "1", strLoginBodyBase64.c_str(), strLoginBodyBase64.length());
}

int CommMsgHandler::AccReply(Packet& pkt)
{
    Json::Value root;
    Json::Reader reader;
    const std::string& strLoginBody = Decode64((unsigned char const*)(pkt.pBuffer.get()), pkt.buflen);
    if (reader.parse(strLoginBody, root, false))
    {
        std::string key("UUID");
        if (root[key].isString())
        {
            m_strSessionID = root[key].asString();
        }
        else if (root[key].isUInt())
        {
            m_strSessionID = boost::lexical_cast<std::string>(root[key].asUInt());
        }
        else if (root[key].isInt())
        {
            m_strSessionID = boost::lexical_cast<std::string>(root[key].asUInt());
        }
        else
        {
            return FAILED;
        }
    }
    return SUCCEED;
}

int CommMsgHandler::SyncSession(FuncRequest req, FuncReply rsp)
{
    int errcode = req(boost::bind(&CommMsgHandler::SyncWrite, this, (m_strSessionID.empty() ? m_strID : m_strSessionID), _1, _2, _3, _4));
    if (errcode == SUCCEED)
    {
        Packet pkt;
        errcode = SyncRead(pkt);
        if (errcode == SUCCEED)
        {
            errcode = rsp(pkt);
        }
    }

    return errcode;
}

int CommMsgHandler::StartTask()
{
    int errcode = SUCCEED;
    while (!m_func.empty())
    {
        auto func = m_func.front();
        m_func.pop();

        errcode = func();
        if (errcode != SUCCEED)
        {
            //std::swap(m_func, std::queue<boost::function<int()> >());            
            return errcode;
        }
    }

    return errcode;
}

int CommMsgHandler::SyncConnect()
{
    boost::mutex::scoped_lock lock(m_mtx);
    m_Client->AsyncConnect();
    m_cond.wait(lock);
    return m_iOperationStatus;
}

int CommMsgHandler::SyncRead(Packet &pt)
{
    boost::mutex::scoped_lock lock(m_mtx);
    while (m_ReadBuff.empty())
    {
        boost::shared_ptr<CBValue> pLocalValue(new CBValue);
        if (m_nTimeout != 0)
        {
            pLocalValue->pTimer = sm_TimeOutObj.Create(boost::bind(&CommMsgHandler::TimeOutCB, shared_from_this(), _1), m_nTimeout, false);
            pLocalValue->pTimer->Begin();
        }
        
        m_Client->AsyncRead(pLocalValue.get());
        m_cond.wait(lock);
        if (SUCCEED != pLocalValue->errcode)
        {
            return pLocalValue->errcode;
        }
    }

    auto msg = m_ReadBuff.front();
    m_ReadBuff.pop();
    pt.buflen = msg.second;
    pt.pBuffer = msg.first;

    return SUCCEED;
}

int CommMsgHandler::SyncWrite(const std::string& src, const std::string& dst, const std::string& type, const char* buff, int len)
{
    boost::shared_ptr<CBValue> pLocalValue(new CBValue);
    if (m_nTimeout != 0)
    {
        pLocalValue->pTimer = sm_TimeOutObj.Create(boost::bind(&CommMsgHandler::TimeOutCB, shared_from_this(), _1), m_nTimeout, false);
        pLocalValue->pTimer->Begin();
    }

    boost::mutex::scoped_lock lock(m_mtx);
    m_Client->AsyncWrite(src, dst, type, buff, len, true, pLocalValue.get());
    m_cond.wait(lock);
    return pLocalValue->errcode;
}

void CommMsgHandler::ConnectCB(const boost::system::error_code& e)
{
    m_iOperationStatus = e ? FAILED : SUCCEED;
    
    boost::mutex::scoped_lock lock(m_mtx);
    m_cond.notify_one();
}

void CommMsgHandler::ReadCB(const boost::system::error_code& e, std::list<ClientMsg> *pClientMsgList, void* pLocalValue)
{
    CBValue* pCBValue = static_cast<CBValue*>(pLocalValue);

    if (e && (TIMEOUT_FAILED != m_iOperationStatus)) //通信失败
    {
        if (NULL != pCBValue->pTimer)
        {
            pCBValue->pTimer->End();
        }

        pCBValue->errcode = FAILED;

        boost::mutex::scoped_lock lock(m_mtx);
        m_cond.notify_one();
        return;
    }
    
    if (e && (NULL != pCBValue->pTimer) && (TIMEOUT_FAILED == m_iOperationStatus)) //超时失败
    {        
        pCBValue->errcode = TIMEOUT_FAILED;

        boost::mutex::scoped_lock lock(m_mtx);
        m_cond.notify_one();
        return;
    }
    
    boost::mutex::scoped_lock lock(m_mtx);
    for (auto msg = pClientMsgList->begin(); msg != pClientMsgList->end(); ++msg)
    {
        m_ReadBuff.push(std::make_pair(msg->pContentBuffer, msg->uiContentBufferLen));
    }

    m_iOperationStatus = SUCCEED;
    pCBValue->errcode = SUCCEED;
        
    m_cond.notify_one();
}

void CommMsgHandler::WriteCB(const boost::system::error_code& e, void* pLocalValue)
{
    CBValue* pCBValue = static_cast<CBValue*>(pLocalValue);

    if (e && (TIMEOUT_FAILED != m_iOperationStatus)) //通信失败
    {
        if (NULL != pCBValue->pTimer)
        {
            pCBValue->pTimer->End();
        }

        pCBValue->errcode = FAILED;

        boost::mutex::scoped_lock lock(m_mtx);
        m_cond.notify_one();
        return;
    }
    
    if (e && (NULL != pCBValue->pTimer) && (TIMEOUT_FAILED == m_iOperationStatus)) //超时失败
    {        
        pCBValue->errcode = TIMEOUT_FAILED;

        boost::mutex::scoped_lock lock(m_mtx);
        m_cond.notify_one();
        return;
    }

    m_iOperationStatus = SUCCEED;
    pCBValue->errcode = SUCCEED;

    boost::mutex::scoped_lock lock(m_mtx);
    m_cond.notify_one();
}

void CommMsgHandler::TimeOutCB(const boost::system::error_code& e)
{
    if (e)
    {
        return;
    }

    m_iOperationStatus = TIMEOUT_FAILED;
    m_Client->Close();
}

