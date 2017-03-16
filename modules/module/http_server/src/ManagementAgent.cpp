#include "ManagementAgent.h"
#include <boost/lexical_cast.hpp>
#include <boost/scope_exit.hpp>
#include "json/json.h"
#include "LogRLD.h"

const std::string ManagementAgent::ADD_CLUSTER_ACTION("add_cluster_agent");

const std::string ManagementAgent::CLUSTER_SHAKEHAND__ACTION("cluster_shakehand");

const std::string ManagementAgent::SUCCESS_CODE = "0";
const std::string ManagementAgent::SUCCESS_MSG = "Ok";
const std::string ManagementAgent::FAILED_CODE = "-1";
const std::string ManagementAgent::FAILED_MSG = "Inner failed";

ManagementAgent::ManagementAgent(const ParamInfo &parminfo) : m_ParamInfo(parminfo), m_Tm(boost::bind(&ManagementAgent::CollectClusterInfo, this, _1), 10)
{
    m_Tm.Run();
}


ManagementAgent::~ManagementAgent()
{
    m_Tm.Stop();
}

void ManagementAgent::SetMsgWriter(Writer wr)
{
    m_Wr = wr;
}

bool ManagementAgent::AddClusterAgentHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", FAILED_CODE));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->m_Wr(ResultInfoMap, writer, blResult, NULL);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("management_address");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Management address not found.");
        return blResult;
    }
    const std::string strManagementAddress = itFind->second;
    
    itFind = pMsgInfoMap->find("clusterid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Cluster id not found.");
        return blResult;
    }
    const std::string strClusterID = itFind->second;

    LOG_INFO_RLD("Add cluster agent info received and management address is " << strManagementAddress << " and cluster id is " << strClusterID);

    if (!AddClusterAgent(strManagementAddress, strClusterID))
    {
        LOG_ERROR_RLD("Add cluster agent handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}


bool ManagementAgent::ClusterAgentShakehandHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", FAILED_CODE));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->m_Wr(ResultInfoMap, writer, blResult, NULL);
    }
    BOOST_SCOPE_EXIT_END
        
    auto itFind = pMsgInfoMap->find("clusterid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Cluster id not found.");
        return blResult;
    }
    const std::string strClusterID = itFind->second;

    LOG_INFO_RLD("Shakehand cluster agent info received and cluster id is " << strClusterID);
        
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool ManagementAgent::AddClusterAgent(const std::string &strManagementAddress, const std::string &strClusterID)
{


    return true;
}

void ManagementAgent::CollectClusterInfo(const boost::system::error_code& e)
{
    if (e)
    {
        LOG_ERROR_RLD("Timer is error: " << e.message());
        return;
    }

    LOG_INFO_RLD("Begin collect cluster info.");
}

