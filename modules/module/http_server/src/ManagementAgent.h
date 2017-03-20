#ifndef __MANAGEMENT_AGENT__
#define  __MANAGEMENT_AGENT__

#include "NetComm.h"
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include "FCGIManager.h"

typedef boost::function<void(const std::map<std::string, std::string> &, MsgWriter, const bool, boost::function<void(void*)>)> Writer;

class InteractiveProtoHandler;
class InteractiveProtoManagementHandler;


class ManagementAgent
{
public:

    static const std::string ADD_CLUSTER_ACTION;
    static const std::string DELETE_CLUSTER_ACTION;
    static const std::string CLUSTER_SHAKEHAND__ACTION;
    
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

    bool ClusterAgentShakehandHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeleteClusterAgentHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

private:
    bool AddClusterAgent(const std::string &strManagementAddress, const std::string &strClusterID);
    
    bool DeleteClusterAgent(const std::string &strClusterID);

    void CollectClusterInfo(const boost::system::error_code& e);

    template<typename T>
    void AccessedDeviceInfoHandler(boost::shared_ptr<T> DeviceInfoList, const std::string &strPushIPAddress, const std::string &strPushPort);

    template<typename T>
    void AccessedUserInfoHandler(boost::shared_ptr<T> UserInfoList, const std::string &strPushIPAddress, const std::string &strPushPort);


private:
    ParamInfo m_ParamInfo;

    Writer m_Wr;

    static const std::string SUCCESS_CODE;
    static const std::string SUCCESS_MSG;
    static const std::string FAILED_CODE;
    static const std::string FAILED_MSG;

    TimeOutHandler m_Tm;

    boost::mutex m_MgnArMutex;
    std::string m_strManagementAddress;
    std::string m_strClusterID;

    boost::shared_ptr<InteractiveProtoHandler> m_pInteractiveProtoHandler;
    boost::shared_ptr<InteractiveProtoManagementHandler> m_pInteractiveProtoMgrHandler;


    Runner m_PushCollectInfoRunner;
};

#endif


