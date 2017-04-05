#ifndef FILE_HANDLER
#define FILE_HANDLER

#include "NetComm.h"
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include "FCGIManager.h"

/************************************************************************/
/*负责实现消息处理具体细节，向FCGIManager注册处理函数，
 *其中包括实际的业务消息处理，例如：将用户注册的信息汇总，
 *调用接口实现用户注册的业务动作。*/
/************************************************************************/

class InteractiveProtoHandler;

class HttpMsgHandler : public boost::noncopyable
{
public:
    static const std::string UPLOAD_FILE_ACTION;
    static const std::string REGISTER_USER_ACTION;
    static const std::string USER_SHAKEHAND_ACTION;
    
    typedef struct _ParamInfo
    {
        std::string m_strRemoteAddress;
        std::string m_strRemotePort;
        unsigned int m_uiShakehandOfChannelInterval;
        std::string m_strSelfID;
        unsigned int m_uiCallFuncTimeout;
        unsigned int m_uiThreadOfWorking;
    } ParamInfo;

    HttpMsgHandler(const ParamInfo &parminfo);
    ~HttpMsgHandler();

    bool UploadFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool RegisterUserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ShakehandHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    void WriteMsg(const std::map<std::string, std::string> &MsgMap, MsgWriter writer, const bool blResult = true, boost::function<void(void*)> PostFunc = NULL);
    
private:

    bool PreCommonHandler(const std::string &strMsgReceived);

    bool RegisterUser(const std::string &strUserName, const std::string &strUserPwd, 
        const std::string &strType, const std::string &strExtend, const std::string &strAliasName, const std::string &strEmail, std::string &strUserID);

private:
    ParamInfo m_ParamInfo;
    boost::shared_ptr<InteractiveProtoHandler> m_pInteractiveProtoHandler;

private:
    static const std::string SUCCESS_CODE;
    static const std::string SUCCESS_MSG;
    static const std::string FAILED_CODE;
    static const std::string FAILED_MSG;

};

#endif
