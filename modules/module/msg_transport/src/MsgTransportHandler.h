#ifndef FILE_HANDLER
#define FILE_HANDLER

#include "NetComm.h"
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#include "CommMsgHandler.h"


/************************************************************************/
/*负责实现消息传输处理，
 *其中包括实际的业务消息处理，例如：将用户注册的信息汇总，
 *调用接口实现用户注册的业务动作。*/
/************************************************************************/

class CacheMgr;

class InteractiveProtoHandler;

typedef std::map<std::string, std::string> MsgInfoMap;


class MsgTransportHandler : public boost::noncopyable
{
public:
    static const std::string REGISTER_USER_ACTION;
    

    typedef struct _ParamInfo
    {
        std::string m_strRemoteAddress;
        std::string m_strRemotePort;
        unsigned int m_uiShakehandOfChannelInterval;
        std::string m_strSelfID;
        unsigned int m_uiCallFuncTimeout;
        unsigned int m_uiThreadOfWorking;
    } ParamInfo;

    MsgTransportHandler(const ParamInfo &parminfo, CacheMgr &sgr);
    ~MsgTransportHandler();


    bool CtrlMsgHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap);
    
    inline void SetID(const std::string &strID)
    {
        m_strID = strID;
    };
    
private:
    
    bool CtrlMsg(const std::string &strDstSid);
    
private:
    bool ValidDatetime(const std::string &strDatetime);

    bool GetValueList(const std::string &strValue, std::list<std::string> &strValueList);

private:
    ParamInfo m_ParamInfo;
    boost::shared_ptr<InteractiveProtoHandler> m_pInteractiveProtoHandler;

    Runner m_MsgReceiveRunner;

    CacheMgr &m_SessionMgr;

    std::string m_strID;

    TimeOutHandler m_SidTimer;

    boost::mutex m_ReceiveSidMutex;
    std::string m_strReceiveSid;
private:
    static const std::string SUCCESS_CODE;
    static const std::string SUCCESS_MSG;
    static const std::string FAILED_CODE;
    static const std::string FAILED_MSG;

};

#endif
