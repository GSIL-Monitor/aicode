#ifndef CLIENT_COMM
#define CLIENT_COMM

#include "NetComm.h"
#include <string>
#include <boost/weak_ptr.hpp>
#include <list>
#include "ClientCommInterface.h"

/************************************************************************/
/* ClientComm�ͻ���ͨѶ����һ��ʵ��������һ�������ϵ�tcp����
 * ��Ҫ����ʵ�ֵײ�������ͨѶ�����Ҹ����շ��ı���ʽЭ���
 * ���ı�Э����е�body�����ڸ����д����ɾ���ҵ����*/
/************************************************************************/


class ClientComm : public boost::enable_shared_from_this<ClientComm>
{
public:
    ClientComm();
    ~ClientComm();

    static boost::shared_ptr<ClientComm> Create(const char *pIPAddress, const char *pIPPort, const unsigned int uiSSLEnabled = 0);

    static void Run(const unsigned int uiThreadNum);

    static void Stop();

    void AsyncConnect();

    void AsyncWrite(const std::string &strSrcID, const std::string &strDstID, const std::string &strType,
        const char *pContentBuffer, const unsigned int uiContentBufferLen, const bool IsNeedEncode = false, void *pValueAppend = NULL);
    
    void AsyncRead(void *pValueAppend = NULL);

    void Close();

    void SetCallBack(ClientConnectedCB cccb, ClientReadCB rdcb, ClientWriteCB wtcb);

    void SetTcpClient(boost::shared_ptr<TCPClient> pTcpclient);
    
private:
    void ConnectedCB(const boost::system::error_code &ec);

    void ReadCB(const boost::system::error_code &ec, std::size_t bytes_transferred, void *pValue);

    void WriteCB(const boost::system::error_code &ec, std::size_t bytes_transferred, void *pValue);

private:

    boost::shared_ptr<TCPClient> m_pTcpclient;

    ClientConnectedCB m_ClientConnectedCB;
    ClientReadCB m_ClientRCB;
    ClientWriteCB m_ClientWCB;
        
    char m_cReadBuffer[64 * 1024];
    char *m_pRemainBufferPos;
    unsigned int m_uiRemainBufferSize;

    static TCPClientEx ms_TCPClientEx;
};


#endif