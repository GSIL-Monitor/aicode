#ifndef __DEV_COMM_H__
#define __DEV_COMM_H__

#include <string>
#include <queue>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include "ClientCommInterface.h"
#include <boost/shared_array.hpp>
#include "NetComm.h"


class CommMsgHandler : public boost::enable_shared_from_this<CommMsgHandler>
{
public:
    struct Packet {
        boost::shared_array<char> pBuffer;
        int   buflen;
        Packet() : buflen(0){};
        ~Packet(){};
    };

	static const int CONTINUE = 2;
    static const int SUCCEED = 0;
    static const int FAILED = -1;
    static const int TIMEOUT_FAILED = 1;

    typedef boost::function<int(const std::string& dst, const std::string& type, const char* buff, int len)> SendWriter;
    typedef boost::function<int(SendWriter)> FuncRequest;
    typedef boost::function<int(Packet&)> FuncReply;

    CommMsgHandler(const std::string &strID, const int iTimeout);
    ~CommMsgHandler();
    
    int Start(const std::string& strIpAddr, const std::string& strPort, unsigned int uiSSLEnabled, unsigned int  uiHandShakeInterval);

    int StartTask();

    void SetReqAndRspHandler(FuncRequest req, FuncReply rsp);

    static void SetTimeoutRunningThreads(const unsigned int uiThdNum);

    static void SetCommRunningThreads(const unsigned int uiThdNum);

private:
    
    int AccRequest(SendWriter writer);

    int AccReply(Packet& pkt);

    int SyncSession(FuncRequest req, FuncReply rsp);
    
    int SyncConnect();

    int SyncRead(Packet &pt);

    int SyncWrite(const std::string& src, const std::string& dst, const std::string& type, const char* buff, int len);

    void ConnectCB(const boost::system::error_code& e);

    void ReadCB(const boost::system::error_code& e, std::list<ClientMsg> *pClientMsgList, void* pLocalValue);

    void WriteCB(const boost::system::error_code& e, void* pLocalValue);

    void TimeOutCB(const boost::system::error_code& e);


private:
    struct CBValue
    {
        CBValue() :errcode(0){}
        int errcode;
        boost::shared_ptr<TimeOutHandler> pTimer;
    };

    std::string m_strID;
    std::string m_strSessionID;

    boost::mutex m_mtx;
    boost::condition m_cond;

    boost::shared_ptr<ClientCommInterface> m_Client;

    std::queue<std::pair<boost::shared_array<char>, unsigned int> > m_ReadBuff;

    std::queue<boost::function<int()> > m_func;

    boost::atomic_int m_iOperationStatus;

    int m_nTimeout;

    static boost::mutex sm_TimeOutObjMutex;
    static unsigned int sm_uiTimeOutObjInited;
    static TimeOutHandlerEx sm_TimeOutObj;
    static unsigned int sm_uiTimeOutThreadNum;
    static unsigned int sm_uiCommThreadNum;
};

#endif
