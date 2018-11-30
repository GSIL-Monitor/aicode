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

#include "TransServer.h"

/************************************************************************/
/* 实现了通用的协议数据包的发送、接收消息事件处理循环，
 * 通过提供注册消息处理函数接口来与外部业务进行交互。
 * Author：尹宾
 * Date：2016-12-1*/
/************************************************************************/

typedef boost::function<void(const std::string &strDstID, const std::string &strDataToBeWriting)> MsgWriter;

typedef boost::function<bool(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)> MsgHandler;

typedef boost::function<bool(const std::string &strMsg, int &iMsgType)> MsgTypeHandler;


class ControlCenter
{
public:
    typedef struct _ParamInfo
    {        
        std::string strRemoteAddress;
        std::string strRemotePort;
        unsigned int uiShakehandOfChannelInterval;

        std::string strSelfID;
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

    void SetupMsgPreHandler(MsgHandler msghandler);

    void SetupMsgTypeParseHandler(MsgTypeHandler msgtypehdr);

private:    
    void ConnectCB(const boost::system::error_code &ec);
    void WriteCB(const boost::system::error_code &ec, void *pValue);
    void ReadCB(const boost::system::error_code &ec, std::list<ClientMsg> *pClientMsgList, void *pValue);    
    void Reconnect();

    void MsgWrite(const std::string &strDstID, const std::string &strDataToBeWriting);

    void MsgWriteInner(const std::string &strDstID, const std::string &strDataToBeWriting);

private:

    ParamInfo m_ParamInfo;

    boost::shared_ptr<ClientCommInterface> m_pClient;

    Runner m_MsgHandlerRunner;
    
    std::map<int, MsgHandler> m_MsgHandlerMap;

    std::list<MsgHandler> m_MsgPreHandlerList;

    Runner m_MsgWriterRunner;
    boost::mutex m_MsgWriterMutex;

    MsgTypeHandler m_MsgTypeHandler;

    TransServer m_pServer;

private:
    bool ReceiveMsgHandler(const std::string &strData, const std::string &strSrcID, void *pValue);
    void ReceiveMsgHandlerInner(MsgHandler MsgHdr, const std::string &strData, const std::string &strSrcID, const int iMsgType, void *pValue);

    void MsgProcess(const std::string &strData, MsgWriter msgwriter);

};

#endif
