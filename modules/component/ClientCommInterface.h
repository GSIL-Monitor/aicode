#ifndef CLIENT_INTERFACE
#define CLIENT_INTERFACE
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <list>
#include <string>
#include <boost/thread/mutex.hpp>
#include <boost/weak_ptr.hpp>

class ClientComm;
class TimeOutHandler;
class TimeOutHandlerEx;

typedef boost::function<void(const boost::system::error_code&)> ClientConnectedCB;
typedef boost::function<void(const boost::system::error_code&, void *)> ClientWriteCB;

typedef struct
{
    std::string strSrcID;
    std::string strDstID;
    std::string strType;
    boost::shared_ptr<char> pContentBuffer;
    unsigned int uiContentBufferLen;
} ClientMsg;
typedef boost::function<void(const boost::system::error_code&, std::list<ClientMsg> *pClientMsgList, void *)> ClientReadCB;

class ClientCommInterface
{
public:
    ~ClientCommInterface();

    static ClientCommInterface *Create(const char *pIPAddress, const char *pIPPort, const unsigned int uiSSLEnabled = 0, const unsigned int uiShakehandInterval = 0);

    static void Run(const unsigned int uiThreadNum);

    static void Stop();

    void AsyncConnect();

    void AsyncWrite(const std::string &strSrcID, const std::string &strDstID, const std::string &strType,
        const char *pContentBuffer, const unsigned int uiContentBufferLen, const bool IsNeedEncode = false, void *pValue = NULL);

    void AsyncRead(void *pValue = NULL);

    void Close();

    void SetCallBack(ClientConnectedCB cccb, ClientReadCB rdcb, ClientWriteCB wtcb);

    
private:
    ClientCommInterface(boost::shared_ptr<ClientComm> pClientComm, const unsigned int uiShakehandInterval = 0);

    boost::shared_ptr<ClientComm> m_pClientComm;

    struct ShakehandHandler
    {
        ShakehandHandler();
        ~ShakehandHandler();
        
        ClientConnectedCB m_ClientConnectedCB;
        ClientWriteCB m_ClientWriteCB;
        ClientReadCB m_ClientReadCB;

        boost::shared_ptr<TimeOutHandler> m_pShakehandTM;
        boost::mutex m_ChannelCommMutex;
        boost::uint32_t m_uiDoShakehanding;
        //boost::shared_ptr<ClientComm> m_pClientComm;
        boost::weak_ptr<ClientComm> m_pClientComm;

        void ShakeHand(const boost::system::error_code &ec);

        void ConnectedCBInner(const boost::system::error_code &ec);

        void WriteCBInner(const boost::system::error_code &ec, void *pValue);

        void ReadCBInner(const boost::system::error_code &ec, std::list<ClientMsg> *pClientMsgList, void *pValue);
    };

    boost::shared_ptr<ShakehandHandler> m_Shakehandler;

    static boost::shared_ptr<TimeOutHandlerEx> sm_pTmEx;
    static boost::mutex sm_TmExMutex;
    
};


#endif