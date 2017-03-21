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

class InteractiveProtoManagementHandler;

class HttpMsgHandler : public boost::noncopyable
{
public:
    static const std::string ADD_CLUSER_ACTION;
    static const std::string DELETE_CLUSER_ACTION;
    static const std::string MODIFY_CLUSER_ACTION;
    static const std::string QUERY_CLUSER_INFO_ACTION;
    static const std::string CLUSER_SHAKEHAND_ACTION;
    static const std::string QUERY_ALL_CLUSER_ACTION;
    static const std::string QUERY_CLUSER_DEVICE_ACTION;
    static const std::string QUERY_CLUSER_USER_ACTION;
    
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

    bool AddCluserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DeleteCluserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyCluserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryCluserInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool CluserShakehandHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryAllCluserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryCluserDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryCluserUserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);



private:

    void WriteMsg(const std::map<std::string, std::string> &MsgMap, MsgWriter writer, const bool blResult = true, boost::function<void(void*)> PostFunc = NULL);

    bool PreCommonHandler(const std::string &strMsgReceived);

    bool AddCluster(const std::string &strClusterAddress, const std::string &strManagementAddress, const std::string &strAliasName, std::string &strClusterID);

    bool DeleteCluster(const std::string &strClusterID);

    bool ModifyCluser(const std::string &strClusterID, const std::string &strAliasName);

    bool QueryCluserInfo(const std::string &strClusterID, std::string &strClusterAddress, std::string &strManagementAddress, std::string &strAliasName,
        std::string &strCreateDate, std::string &strStatus, std::string &strAccessedUserNumber, std::string &strAccessedDeviceNumber);

    bool CluserShakehand(const std::string &strClusterID);

    template<typename T>
    bool QueryAllCluser(const std::string &strManagementAddress, std::list<T> &ClusterInfoList);

    template<typename T>
    bool QueryCluserDevice(const std::string &strClusterID, const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiType, 
        const unsigned int uiBeginIndex, std::list<T> &AccessedDeviceList);

    template<typename T>
    bool QueryCluserUser(const std::string &strClusterID, const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiType,
        const unsigned int uiBeginIndex, std::list<T> &AccessedUserList);


    bool ValidDate(const std::string &strDate);

private:
    ParamInfo m_ParamInfo;
    boost::shared_ptr<InteractiveProtoManagementHandler> m_pInteractiveProtoHandler;

private:
    static const std::string SUCCESS_CODE;
    static const std::string SUCCESS_MSG;
    static const std::string FAILED_CODE;
    static const std::string FAILED_MSG;

};




#endif
