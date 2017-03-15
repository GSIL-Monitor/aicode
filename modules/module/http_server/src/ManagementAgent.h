#ifndef __MANAGEMENT_AGENT__
#define  __MANAGEMENT_AGENT__

#include "NetComm.h"
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include "FCGIManager.h"

typedef boost::function<void(const std::map<std::string, std::string> &, MsgWriter, const bool, boost::function<void(void*)>)> Writer;

class ManagementAgent
{
public:

    static const std::string ADD_CLUSTER_ACTION;

    typedef struct _ParamInfo
    {
        std::string m_strRemoteAddress;
        std::string m_strRemotePort;
        unsigned int m_uiShakehandOfChannelInterval;
        std::string m_strSelfID;
        unsigned int m_uiCallFuncTimeout;
        unsigned int m_uiThreadOfWorking;
    } ParamInfo;

    ManagementAgent(const ParamInfo &parminfo);
    ~ManagementAgent();

    void SetMsgWriter(Writer wr);

    bool AddClusterAgentHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

private:
    bool AddClusterAgent(const std::string &strManagementAddress);


private:
    ParamInfo m_ParamInfo;

    Writer m_Wr;

    static const std::string SUCCESS_CODE;
    static const std::string SUCCESS_MSG;
    static const std::string FAILED_CODE;
    static const std::string FAILED_MSG;

};

#endif