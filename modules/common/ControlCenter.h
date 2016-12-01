#ifndef _CONTROL_CENTER_
#define _CONTROL_CENTER_

#include <string>
#include "boost/thread.hpp"
#include "ClientCommInterface.h"
#include "boost/shared_ptr.hpp"
#include "NetComm.h"
#include <map>
#include "boost/atomic.hpp"
#include "LogRLD.h"

/************************************************************************/
/* ʵ����ͨ�õ�Э�����ݰ��ķ��͡�������Ϣ�¼�����ѭ����
 * ͨ���ṩע����Ϣ���������ӿ������ⲿҵ����н�����
 * Author������
 * Date��2016-12-1*/
/************************************************************************/

typedef boost::function<void(const std::string &strDstID, const std::string &strDataToBeWriting)> MsgWriter;

typedef boost::function<bool(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)> MsgHandler;

class InteractiveProtoHandler;

class ControlCenter
{
public:
    typedef struct _ParamInfo
    {        
        std::string strRemoteAddress;
        std::string strRemotePort;
        unsigned int uiShakehandOfChannelInterval;

        std::string strSelfID;
        unsigned int uiSyncShakehandTimeoutCount;
        unsigned int uiSyncShakehandTimeout;
        unsigned int uiSyncAddressRspInvalidTimeout;
        unsigned int uiThreadOfWorking;

    } ParamInfo;

    ControlCenter(const ParamInfo &pinfo);
    ~ControlCenter();

    void Run(const bool isWaitRunFinished);

    void Stop();


    inline void SetParamInfo(const ParamInfo &pinfo)
    {
        m_ParamInfo = pinfo;
    };

    void SetupMsgHandler(const int iMsgType, MsgHandler msghandler);

private:    
    void ConnectCB(const boost::system::error_code &ec);
    void WriteCB(const boost::system::error_code &ec, void *pValue);
    void ReadCB(const boost::system::error_code &ec, std::list<ClientMsg> *pClientMsgList, void *pValue);    
    void Reconnect();

    void MsgWrite(const std::string &strDstID, const std::string &strDataToBeWriting);

private:
    boost::shared_ptr<InteractiveProtoHandler> m_pProtoHandler;

    ParamInfo m_ParamInfo;

    boost::shared_ptr<ClientCommInterface> m_pClient;

    Runner m_MsgHandlerRunner;
    
    std::map<int, MsgHandler> m_MsgHandlerMap;

private:
    bool ReceiveMsgHandler(const std::string &strData, const std::string &strSrcID, void *pValue);
    void ReceiveMsgHandlerInner(MsgHandler MsgHdr, const std::string &strData, const std::string &strSrcID, const int iMsgType, void *pValue);
        


};

#endif