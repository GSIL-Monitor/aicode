#include "ManagementCenter.h"
#include <boost/scope_exit.hpp>
#include "CommonUtility.h"
#include "ReturnCode.h"
#include "boost/lexical_cast.hpp"
#include "json/json.h"
#include "HttpClient.h"
#include "mysql_impl.h"

const std::string ManagementCenter::MAX_DATE = "2199-01-01 00:00:00";

ManagementCenter::ManagementCenter(const ParamInfo &pinfo) : m_ParamInfo(pinfo), m_DBRuner(1), m_pProtoHandler(new InteractiveProtoManagementHandler),
m_pMysql(new MysqlImpl), m_DBCache(m_pMysql), m_uiMsgSeq(0), m_timer(boost::bind(&ManagementCenter::ShakehandCluster, this), 30)
{

}


ManagementCenter::~ManagementCenter()
{
    m_DBRuner.Stop();

    delete m_pMysql;
    m_pMysql = NULL;

    m_timer.Stop();
}

bool ManagementCenter::Init()
{
    if (!m_pMysql->Init(m_ParamInfo.m_strDBHost.c_str(), m_ParamInfo.m_strDBUser.c_str(), m_ParamInfo.m_strDBPassword.c_str(), m_ParamInfo.m_strDBName.c_str()))
    {
        LOG_ERROR_RLD("Init db failed, db host is " << m_ParamInfo.m_strDBHost << " db user is " << m_ParamInfo.m_strDBUser << " db pwd is " <<
            m_ParamInfo.m_strDBPassword << " db name is " << m_ParamInfo.m_strDBName);
        return false;
    }


    if (!m_pMysql->QueryExec(std::string("SET NAMES utf8")))
    {
        LOG_ERROR_RLD("Init charset to utf8 failed, sql is SET NAMES utf8");
        return false;
    }

    if (!InitClusterSession())
    {
        LOG_ERROR_RLD("Init cluster session failed");
        return false;
    }

    //m_DBCache.SetSqlCB(boost::bind(&ManagementCenter::UserInfoSqlCB, this, _1, _2, _3, _4));

    m_DBRuner.Run();

    m_timer.Run(true);

    LOG_INFO_RLD("Management center init success");

    return true;
}

bool ManagementCenter::GetMsgType(const std::string &strMsg, int &iMsgType)
{
    InteractiveProtoManagementHandler::ManagementMsgType mtype;
    if (!m_pProtoHandler->GetManagementMsgType(strMsg, mtype))
    {
        LOG_ERROR_RLD("Get msg type failed");
        return false;
    }

    iMsgType = mtype;
    return true;
}

bool ManagementCenter::AddClusterReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    std::string strClusterID;
    InteractiveProtoManagementHandler::AddClusterReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &strClusterID)
    {
        InteractiveProtoManagementHandler::AddClusterRsp rsp;
        rsp.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::AddClusterRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strClusterID = blResult ? strClusterID : "";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add cluster rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Add cluster rsp already send, dst id is " << strSrcID <<
            " and cluster id is " << strClusterID <<
            " and address is " << req.m_clusterInfo.m_strClusterAddress <<
            " and management address is " << req.m_clusterInfo.m_strManagementAddress <<
            " and aliasname is " << req.m_clusterInfo.m_strAliasname <<
            " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Add cluster req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!IsValidCluster(req.m_clusterInfo.m_strClusterAddress))
    {
        LOG_ERROR_RLD("Add cluster failed, the cluster address is not available, address is " << req.m_clusterInfo.m_strClusterAddress);
        return false;
    }

    //这里是异步执行sql，防止阻塞，后续可以使用其他方式比如MQ来消除数据库瓶颈
    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));

    InteractiveProtoManagementHandler::Cluster clusterInfo;
    clusterInfo.m_strClusterID = strClusterID = CreateUUID();
    clusterInfo.m_strClusterAddress = req.m_clusterInfo.m_strClusterAddress;
    clusterInfo.m_strManagementAddress = req.m_clusterInfo.m_strManagementAddress;
    clusterInfo.m_strAliasname = req.m_clusterInfo.m_strAliasname;
    clusterInfo.m_strCreatedate = strCurrentTime;
    clusterInfo.m_uiStatus = NORMAL_STATUS;

    m_DBRuner.Post(boost::bind(&ManagementCenter::AddCluster, this, clusterInfo));

    m_DBRuner.Post(boost::bind(&ManagementCenter::AddClusterAgent, this, clusterInfo.m_strClusterAddress, clusterInfo.m_strClusterID,
        m_ParamInfo.m_strManagementAddress));

    blResult = true;

    return blResult;
}

bool ManagementCenter::DeleteClusterReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoManagementHandler::DeleteClusterReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &m_mutex, &m_clusterSessionMap)
    {
        InteractiveProtoManagementHandler::DeleteClusterRsp rsp;
        rsp.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::DeleteClusterRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        if (blResult)
        {
            m_mutex.lock();
            auto itPos = m_clusterSessionMap.find(req.m_strClusterID);
            if (itPos != m_clusterSessionMap.end())
            {
                m_clusterSessionMap.erase(itPos);
            }
            m_mutex.unlock();
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete cluster rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Delete cluster rsp already send, dst id is " << strSrcID <<
            " and cluster id is " << req.m_strClusterID <<
            " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Delete cluster req unserialize failed, src id is " << strSrcID);
        return false;
    }

    m_DBRuner.Post(boost::bind(&ManagementCenter::DeleteCluster, this, req.m_strClusterID));

    blResult = true;

    return blResult;
}

bool ManagementCenter::ModifyClusterReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoManagementHandler::ModifyClusterReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        InteractiveProtoManagementHandler::ModifyClusterRsp rsp;
        rsp.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::ModifyClusterRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify cluster rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Modify cluster rsp already send, dst id is " << strSrcID <<
            " and cluster id is " << req.m_clusterInfo.m_strClusterID <<
            " and aliasname is " << req.m_clusterInfo.m_strAliasname <<
            " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Modify cluster req unserialize failed, src id is " << strSrcID);
        return false;
    }

    m_DBRuner.Post(boost::bind(&ManagementCenter::ModifyCluster, this, req.m_clusterInfo));

    blResult = true;

    return blResult;
}

bool ManagementCenter::QueryClusterInfoReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoManagementHandler::QueryClusterInfoReq req;
    InteractiveProtoManagementHandler::Cluster clusterInfo;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &clusterInfo, &m_mutex, &m_clusterSessionMap)
    {
        InteractiveProtoManagementHandler::QueryClusterInfoRsp rsp;
        rsp.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::QueryClusterInfoRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_clusterInfo.m_strClusterID = clusterInfo.m_strClusterID;
            rsp.m_clusterInfo.m_strClusterAddress = clusterInfo.m_strClusterAddress;
            rsp.m_clusterInfo.m_strManagementAddress = clusterInfo.m_strManagementAddress;
            rsp.m_clusterInfo.m_strAliasname = clusterInfo.m_strAliasname;
            rsp.m_clusterInfo.m_uiDeviceTotalnumber = clusterInfo.m_uiDeviceTotalnumber;
            rsp.m_clusterInfo.m_uiUserTotalnumber = clusterInfo.m_uiUserTotalnumber;
            rsp.m_clusterInfo.m_strCreatedate = clusterInfo.m_strCreatedate;
            rsp.m_clusterInfo.m_uiStatus = clusterInfo.m_uiStatus;

            m_mutex.lock();
            auto itPos = m_clusterSessionMap.find(clusterInfo.m_strClusterID);
            rsp.m_uiStatus = itPos == m_clusterSessionMap.end() ? CLUSTER_OFFLINE : itPos->second.uiStatus;
            m_mutex.unlock();
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query cluster info rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query cluster info rsp already send, dst id is " << strSrcID <<
            " and cluster id is " << clusterInfo.m_strClusterID <<
            " and address is " << clusterInfo.m_strClusterAddress <<
            " and management address is " << clusterInfo.m_strManagementAddress <<
            " and aliasname is " << clusterInfo.m_strAliasname <<
            " and accessed device total number is " << clusterInfo.m_uiDeviceTotalnumber <<
            " and accessed user total number is " << clusterInfo.m_uiUserTotalnumber <<
            " and create date is " << clusterInfo.m_strCreatedate <<
            " and cluster status is " << rsp.m_uiStatus <<
            " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query cluster info req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryClusterInfo(req.m_strClusterID, clusterInfo))
    {
        LOG_ERROR_RLD("Query cluster info failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool ManagementCenter::ShakehandClusterReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoManagementHandler::ShakehandClusterReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        InteractiveProtoManagementHandler::ShakehandClusterRsp rsp;
        rsp.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::ShakehandClusterRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Cluster shakehand rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Cluster shakehand rsp already send, dst id is " << strSrcID <<
            " and cluster id is " << req.m_strClusterID <<
            " and result is " << blResult);

    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Cluster shakehand req unserialize failed, src id is " << strSrcID);
        return false;
    }

    //TODO:集群握手
    //m_SessionMgr.Reset(req.m_strSID);

    blResult = true;

    return blResult;
}

bool ManagementCenter::QueryAllClusterReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoManagementHandler::QueryAllClusterReq req;
    std::list<InteractiveProtoManagementHandler::ClusterStatus> clusterInfoList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &clusterInfoList)
    {
        InteractiveProtoManagementHandler::QueryAllClusterRsp rsp;
        rsp.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::QueryAllClusterRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_clusterStatusList.swap(clusterInfoList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all cluster rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query all cluster rsp already send, dst id is " << strSrcID <<
            " and management address is " << req.m_strManagementAddress <<
            " and result is " << blResult);

        if (blResult)
        {
            int i = 0;
            for (auto &clusterStatus : rsp.m_clusterStatusList)
            {
                LOG_INFO_RLD("Cluster info[" << i << "]: "
                    " cluster id is " << clusterStatus.m_clusterInfo.m_strClusterID <<
                    " and address is " << clusterStatus.m_clusterInfo.m_strClusterAddress <<
                    " and management address is " << clusterStatus.m_clusterInfo.m_strManagementAddress <<
                    " and aliasname is " << clusterStatus.m_clusterInfo.m_strAliasname <<
                    " and accessed device total number is " << clusterStatus.m_clusterInfo.m_uiDeviceTotalnumber <<
                    " and accessed user total number is " << clusterStatus.m_clusterInfo.m_uiUserTotalnumber <<
                    " and create date is " << clusterStatus.m_clusterInfo.m_strCreatedate <<
                    " and cluster status is " << clusterStatus.m_uiStatus);

                ++i;
            }
        }
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query all cluster req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryAllCluster(req.m_strManagementAddress, clusterInfoList))
    {
        LOG_ERROR_RLD("Query all cluster failed, src id is " << strSrcID);
    }

    blResult = true;

    return blResult;
}

bool ManagementCenter::QueryClusterDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoManagementHandler::QueryClusterDeviceReq req;
    std::list<InteractiveProtoManagementHandler::AccessedDevice> accessedDeviceList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &accessedDeviceList)
    {
        InteractiveProtoManagementHandler::QueryClusterDeviceRsp rsp;
        rsp.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::QueryClusterDeviceRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_accessedDeviceInfoList.swap(accessedDeviceList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query cluster device rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query cluster device rsp already send, dst id is " << strSrcID <<
            " and cluster id is " << req.m_strClusterID <<
            " and result is " << blResult);

        if (blResult)
        {
            int i = 0;
            for (auto &accessedDevice : rsp.m_accessedDeviceInfoList)
            {
                LOG_INFO_RLD("Accessed device info[" << i << "]: "
                    " device id is " << accessedDevice.m_strDeviceID <<
                    " and device name is " << accessedDevice.m_strDeviceName <<
                    " and device type is " << accessedDevice.m_uiDeviceType <<
                    " and login time is " << accessedDevice.m_strLoginTime <<
                    " and logout time is " << accessedDevice.m_strLogoutTime);

                ++i;
            }
        }
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query cluster device req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryClusterDevice(req.m_strClusterID, req.m_strBegindate, req.m_strEnddate, req.m_uiRecordType, accessedDeviceList))
    {
        LOG_ERROR_RLD("Query cluster device failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool ManagementCenter::QueryClusterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoManagementHandler::QueryClusterUserReq req;
    std::list<InteractiveProtoManagementHandler::AccessedUser> accessedUserList;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req, &accessedUserList)
    {
        InteractiveProtoManagementHandler::QueryClusterUserRsp rsp;
        rsp.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::QueryClusterUserRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;

        if (blResult)
        {
            rsp.m_accessedUserInfoList.swap(accessedUserList);
        }

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query cluster user rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Query cluster user rsp already send, dst id is " << strSrcID <<
            " and cluster id is " << req.m_strClusterID <<
            " and result is " << blResult);

        if (blResult)
        {
            int i = 0;
            for (auto &accessedUser : rsp.m_accessedUserInfoList)
            {
                LOG_INFO_RLD("Accessed user info[" << i << "]: "
                    " user id is " << accessedUser.m_strUserID <<
                    " and user name is " << accessedUser.m_strUserName <<
                    " and user client type is " << accessedUser.m_uiClientType <<
                    " and login time is " << accessedUser.m_strLoginTime <<
                    " and logout time is " << accessedUser.m_strLogoutTime);

                ++i;
            }
        }
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Query cluster user req unserialize failed, src id is " << strSrcID);
        return false;
    }

    if (!QueryClusterUser(req.m_strClusterID, req.m_strBegindate, req.m_strEnddate, req.m_uiRecordType, accessedUserList))
    {
        LOG_ERROR_RLD("Query cluster user failed, src id is " << strSrcID);
        return false;
    }

    blResult = true;

    return blResult;
}

bool ManagementCenter::PushClusterDeviceReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoManagementHandler::PushClusterDeviceReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        InteractiveProtoManagementHandler::PushClusterDeviceRsp rsp;
        rsp.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::PushClusterDeviceRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Push cluster device rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Push cluster device rsp already send, dst id is " << strSrcID <<
            " and cluster id is " << req.m_strClusterID <<
            " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Push cluster device req unserialize failed, src id is " << strSrcID);
        return false;
    }

    m_DBRuner.Post(boost::bind(&ManagementCenter::PushClusterDevice, this, req.m_deviceAccessRecordList));

    blResult = true;

    return blResult;
}

bool ManagementCenter::PushClusterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    bool blResult = false;

    InteractiveProtoManagementHandler::PushClusterUserReq req;

    BOOST_SCOPE_EXIT(&blResult, this_, &strSrcID, &writer, &req)
    {
        InteractiveProtoManagementHandler::PushClusterUserRsp rsp;
        rsp.m_MngMsgType = InteractiveProtoManagementHandler::ManagementMsgType::PushClusterUserRsp_T;
        rsp.m_uiMsgSeq = ++this_->m_uiMsgSeq;
        rsp.m_strSID = req.m_strSID;
        rsp.m_iRetcode = blResult ? ReturnInfo::SUCCESS_CODE : ReturnInfo::FAILED_CODE;
        rsp.m_strRetMsg = blResult ? ReturnInfo::SUCCESS_INFO : ReturnInfo::FAILED_INFO;
        rsp.m_strValue = "value";

        std::string strSerializeOutPut;
        if (!this_->m_pProtoHandler->SerializeReq(rsp, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Push cluster user rsp serialize failed");
            return;
        }

        writer(strSrcID, strSerializeOutPut);
        LOG_INFO_RLD("Push cluster user rsp already send, dst id is " << strSrcID <<
            " and cluster id is " << req.m_strClusterID <<
            " and result is " << blResult);
    }
    BOOST_SCOPE_EXIT_END

    if (!m_pProtoHandler->UnSerializeReq(strMsg, req))
    {
        LOG_ERROR_RLD("Push cluster user req unserialize failed, src id is " << strSrcID);
        return false;
    }

    m_DBRuner.Post(boost::bind(&ManagementCenter::PushClusterUser, this, req.m_userAccessRecordList));

    blResult = true;

    return blResult;
}

bool ManagementCenter::IsValidCluster(const std::string &strClusterAddress)
{
    char sql[256] = { 0 };
    const char *sqlfmt = "select count(id) from t_cluster_info where clusteraddress = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strClusterAddress.c_str());

    unsigned int uiCount = 1;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)
    {
        uiCount = boost::lexical_cast<unsigned int>(strColumn);
        LOG_INFO_RLD("IsValidCluster sql count(id) is " << uiCount);
    };

    if (!m_pMysql->QueryExec(std::string(sql), SqlFunc))
    {
        LOG_ERROR_RLD("IsValidCluster exec sql failed, sql is " << sql);
        return false;
    }

    if (uiCount > 0)
    {
        LOG_ERROR_RLD("IsValidCluster failed, the cluster address is used, address is " << strClusterAddress);
        return false;
    }

    LOG_INFO_RLD("IsValidCluster successful, the cluster address is " << strClusterAddress);
    return true;
}

void ManagementCenter::AddClusterAgent(const std::string &strUrl, const std::string &strClusterID, const std::string &strManagementAddress)
{
    if (AddClusterPost(strUrl, strClusterID, strManagementAddress))
    {
        LOG_INFO_RLD("AddClusterAgent successful, cluster address is " << strUrl << " and cluster id is " << strClusterID <<
            " and management address is: " << strManagementAddress);

        RefreshClusterSession(strClusterID, strUrl, CLUSTER_ONLINE, true);
    }
    else
    {
        LOG_ERROR_RLD("AddClusterAgent failed, cluster address is " << strUrl << " and cluster id is " << strClusterID <<
            " and management address is: " << strManagementAddress);

        RefreshClusterSession(strClusterID, strUrl, CLUSTER_OFFLINE, true);
    }
}

bool ManagementCenter::AddClusterPost(const std::string &strUrl, const std::string &strClusterID, const std::string &strManagementAddress)
{
    std::map<std::string, std::string> reqFormMap;
    reqFormMap.insert(std::make_pair("clusterid", strClusterID));
    reqFormMap.insert(std::make_pair("management_address", strManagementAddress));

    std::string postUrl = "http://" + strUrl + "/access.cgi?action=add_cluster_agent";
    std::string strRsp;
    HttpClient httpClient;
    if (CURLE_OK != httpClient.PostForm(postUrl, reqFormMap, strRsp))
    {
        LOG_ERROR_RLD("AddClusterPost send http post failed, url is " << postUrl << " and cluster id is " << strClusterID);
        return false;
    }

    LOG_INFO_RLD("AddClusterPost successful, url is " << postUrl << " and cluster id is " << strClusterID <<
        " and response data is: " << strRsp);

    return true;
}

bool ManagementCenter::InitClusterSession()
{
    char sql[256] = { 0 };
    const char *sqlfmt = "select clusterid, clusteraddress from t_cluster_info where status = 0";
    snprintf(sql, sizeof(sql), sqlfmt);
    std::string strSql(sql);

    struct ClusterFeature
    {
        std::string strClusterID;
        std::string strClusterAddress;
    };
    ClusterFeature clusterFeature;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            clusterFeature.strClusterID = strColumn;
            break;
        case 1:
            clusterFeature.strClusterAddress = strColumn;
            result = clusterFeature;
            break;

        default:
            LOG_ERROR_RLD("InitClusterSession sql callback error, row num is " <<
                uiRowNum << " and column num is " << uiColumnNum << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("InitClusterSession exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("InitClusterSession sql result is empty, sql is " << sql);
        return true;
    }

    for (auto &result : ResultList)
    {
        auto clusterFeature = boost::any_cast<ClusterFeature>(result);
        RefreshClusterSession(clusterFeature.strClusterID, clusterFeature.strClusterAddress, CLUSTER_OFFLINE, true);
    }

    return true;
}

void ManagementCenter::ShakehandCluster()
{
    std::map<std::string, std::string> strFormMap;

    HttpClient httpClient;

    m_mutex.lock();
    std::map<std::string, ClusterSession> snapClusterSession = m_clusterSessionMap;
    m_mutex.unlock();

    for (auto &cluster : snapClusterSession)
    {
        auto &strClusterID = cluster.first;
        auto &clusterSession = cluster.second;

        strFormMap.clear();
        strFormMap.insert(std::make_pair("clusterid", strClusterID));
        std::string strUrl = "http://" + clusterSession.strClusterAddress + "/access.cgi?action=cluster_shakehand";

        AddClusterPost(clusterSession.strClusterAddress, strClusterID, m_ParamInfo.m_strManagementAddress);

        std::string strRsp;
        if (CURLE_OK != httpClient.PostForm(strUrl, strFormMap, strRsp))
        {
            LOG_ERROR_RLD("ShakehandCluster send http post failed, url is " << strUrl << " and cluster id is " << strClusterID);
            RefreshClusterSession(strClusterID, clusterSession.strClusterAddress, CLUSTER_OFFLINE, false);
            continue;
        }

        Json::Reader reader;
        Json::Value value;
        if (!reader.parse(strRsp, value))
        {
            LOG_ERROR_RLD("ShakehandCluster failed, parse http post response data error, raw data is: " << strRsp);
            RefreshClusterSession(strClusterID, clusterSession.strClusterAddress, CLUSTER_OFFLINE, false);
            continue;
        }

        std::string postRtnCode = value["retcode"].isNull() ? "" : value["retcode"].asString();
        if (postRtnCode != "0")
        {
            LOG_ERROR_RLD("ShakehandCluster failed, http post return error, return code is " << postRtnCode);
            RefreshClusterSession(strClusterID, clusterSession.strClusterAddress, CLUSTER_OFFLINE, false);
            continue;
        }

        RefreshClusterSession(strClusterID, clusterSession.strClusterAddress, CLUSTER_ONLINE, false);
        LOG_INFO_RLD("ShakehandCluster successful, cluster id is " << strClusterID);
    }
}

/*刷新集群会话状态信息时，需要考虑互斥操作*/
void ManagementCenter::RefreshClusterSession(const std::string &strClusterID, const std::string &strClusterAddress,
    const unsigned int uiStatus, const bool blAdd)
{
    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));

    m_mutex.lock();
    auto itPos = m_clusterSessionMap.find(strClusterID);
    if (itPos == m_clusterSessionMap.end())
    {
        if (blAdd)
        {
            ClusterSession clusterSession;
            clusterSession.uiStatus = uiStatus;
            clusterSession.strClusterAddress = strClusterAddress;
            m_clusterSessionMap.insert(make_pair(strClusterID, clusterSession));
        }
    }
    else
    {
        ClusterSession &clusterSession = itPos->second;
        clusterSession.uiStatus = uiStatus;
    }
    m_mutex.unlock();
}

void ManagementCenter::AddCluster(const InteractiveProtoManagementHandler::Cluster &clusterInfo)
{
    char sql[512] = { 0 };
    const char *sqlfmt = "insert into t_cluster_info (id, clusterid, clusteraddress, managementaddress, aliasname, createdate, status)"
        " values (uuid(), '%s', '%s', '%s', '%s', '%s', %d)";
    snprintf(sql, sizeof(sql), sqlfmt, clusterInfo.m_strClusterID.c_str(), clusterInfo.m_strClusterAddress.c_str(),
        clusterInfo.m_strManagementAddress.c_str(), clusterInfo.m_strAliasname.c_str(),
        clusterInfo.m_strCreatedate.c_str(), clusterInfo.m_uiStatus);

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("AddCluster exec sql failed, sql is " << sql);
    }
}

void ManagementCenter::DeleteCluster(const std::string &strClusterID)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "update t_cluster_info set status = %d where clusterid = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, DELETE_STATUS, strClusterID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("DeleteCluster exec sql failed, sql is " << sql);
    }
}

void ManagementCenter::ModifyCluster(const InteractiveProtoManagementHandler::Cluster &clusterInfo)
{
    bool blModified = false;

    char sql[512] = { 0 };
    int size = sizeof(sql);
    int len;
    snprintf(sql, size, "update t_cluster_info set ");

    //if (!clusterInfo.m_strClusterAddress.empty())
    //{
    //    len = strlen(sql);
    //    snprintf(sql + len, size - len, ", clusteraddress = '%s'", clusterInfo.m_strClusterAddress.c_str());

    //    blModified = true;
    //}

    //if (!clusterInfo.m_strManagementAddress.empty())
    //{
    //    len = strlen(sql);
    //    snprintf(sql + len, size - len, ", managementaddress = '%s'", clusterInfo.m_strManagementAddress.c_str());

    //    blModified = true;
    //}

    if (!clusterInfo.m_strAliasname.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, "aliasname = '%s'", clusterInfo.m_strAliasname.c_str());

        blModified = true;
    }

    if (!blModified)
    {
        LOG_INFO_RLD("ModifyCluster completed, there is no change");
        return;
    }

    len = strlen(sql);
    snprintf(sql + len, size - len, " where clusterid = '%s' and status = 0", clusterInfo.m_strClusterID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("ModifyCluster exec sql failed, sql is " << sql);
    }
}

bool ManagementCenter::QueryClusterInfo(const std::string &strClusterID, InteractiveProtoManagementHandler::Cluster &clusterInfo)
{
    char sql[256] = { 0 };
    const char *sqlfmt = "select clusterid, clusteraddress, managementaddress, aliasname, createdate, status"
        " from t_cluster_info where clusterid = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strClusterID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            clusterInfo.m_strClusterID = strColumn;
            break;
        case 1:
            clusterInfo.m_strClusterAddress = strColumn;
            break;
        case 2:
            clusterInfo.m_strManagementAddress = strColumn;
            break;
        case 3:
            clusterInfo.m_strAliasname = strColumn;
            break;
        case 4:
            clusterInfo.m_strCreatedate = strColumn;
            break;
        case 5:
            clusterInfo.m_uiStatus = boost::lexical_cast<unsigned int>(strColumn);
            result = clusterInfo;
            break;

        default:
            LOG_ERROR_RLD("QueryClusterInfo sql callback error, row num is " <<
                uiRowNum << " and column num is " << uiColumnNum << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryClusterInfo exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_ERROR_RLD("QueryClusterInfo sql result is empty, sql is " << sql);
        return false;
    }

    auto rstCluster = boost::any_cast<InteractiveProtoManagementHandler::Cluster>(ResultList.front());

    unsigned int uiDeviceTotal = 0;
    unsigned int uiUserTotal = 0;
    QueryClusterDeviceTotal(rstCluster.m_strClusterID, uiDeviceTotal);
    QueryClusterUserTotal(rstCluster.m_strClusterID, uiUserTotal);

    clusterInfo.m_strClusterID = rstCluster.m_strClusterID;
    clusterInfo.m_strClusterAddress = rstCluster.m_strClusterAddress;
    clusterInfo.m_strManagementAddress = rstCluster.m_strManagementAddress;
    clusterInfo.m_strAliasname = rstCluster.m_strAliasname;
    clusterInfo.m_uiDeviceTotalnumber = uiDeviceTotal;
    clusterInfo.m_uiUserTotalnumber = uiUserTotal;
    clusterInfo.m_strCreatedate = rstCluster.m_strCreatedate;
    clusterInfo.m_uiStatus = rstCluster.m_uiStatus;

    return true;
}

bool ManagementCenter::QueryAllCluster(const std::string &strManagementAddress, std::list<InteractiveProtoManagementHandler::ClusterStatus> &clusterStatusList)
{
    char sql[256] = { 0 };
    const char *sqlfmt = "select clusterid, clusteraddress, managementaddress, aliasname, createdate, status"
        " from t_cluster_info where status = 0";
    snprintf(sql, sizeof(sql), sqlfmt);

    InteractiveProtoManagementHandler::Cluster clusterInfo;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            clusterInfo.m_strClusterID = strColumn;
            break;
        case 1:
            clusterInfo.m_strClusterAddress = strColumn;
            break;
        case 2:
            clusterInfo.m_strManagementAddress = strColumn;
            break;
        case 3:
            clusterInfo.m_strAliasname = strColumn;
            break;
        case 4:
            clusterInfo.m_strCreatedate = strColumn;
            break;
        case 5:
            clusterInfo.m_uiStatus = boost::lexical_cast<unsigned int>(strColumn);
            result = clusterInfo;
            break;

        default:
            LOG_ERROR_RLD("QueryAllCluster sql callback error, row num is " <<
                uiRowNum << " and column num is " << uiColumnNum << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryAllCluster exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("QueryAllCluster sql result is empty, sql is " << sql);
        return true;
    }

    for (auto &result : ResultList)
    {
        auto clusterInfo = boost::any_cast<InteractiveProtoManagementHandler::Cluster>(result);

        unsigned int uiDeviceTotal = 0;
        unsigned int uiUserTotal = 0;
        QueryClusterDeviceTotal(clusterInfo.m_strClusterID, uiDeviceTotal);
        QueryClusterUserTotal(clusterInfo.m_strClusterID, uiUserTotal);

        InteractiveProtoManagementHandler::ClusterStatus clusterStatus;
        clusterStatus.m_clusterInfo.m_strClusterID = clusterInfo.m_strClusterID;
        clusterStatus.m_clusterInfo.m_strClusterAddress = clusterInfo.m_strClusterAddress;
        clusterStatus.m_clusterInfo.m_strManagementAddress = clusterInfo.m_strManagementAddress;
        clusterStatus.m_clusterInfo.m_strAliasname = clusterInfo.m_strAliasname;
        clusterStatus.m_clusterInfo.m_uiDeviceTotalnumber = uiDeviceTotal;
        clusterStatus.m_clusterInfo.m_uiUserTotalnumber = uiUserTotal;
        clusterStatus.m_clusterInfo.m_strCreatedate = clusterInfo.m_strCreatedate;
        clusterStatus.m_clusterInfo.m_uiStatus = clusterInfo.m_uiStatus;

        m_mutex.lock();
        auto itPos = m_clusterSessionMap.find(clusterInfo.m_strClusterID);
        clusterStatus.m_uiStatus = itPos == m_clusterSessionMap.end() ? CLUSTER_OFFLINE : itPos->second.uiStatus;
        m_mutex.unlock();

        clusterStatusList.push_back(std::move(clusterStatus));
    }

    return true;
}

bool ManagementCenter::QueryClusterDevice(const std::string &strClusterID, const std::string &strBegindate, const std::string &strEnddate,
    const unsigned int uiRecordType, std::list<InteractiveProtoManagementHandler::AccessedDevice> &accessedDeviceList,
    const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 10*/)
{
    char sql[512] = { 0 };
    int size = sizeof(sql);
    int len;

    const char *sqlfmt = "select deviceid, devicename, logintime, logouttime, devicetype"
        " from t_cluster_accessed_device_info where clusterid = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strClusterID.c_str());

    if (!strBegindate.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, " and createdate >= '%s'", strBegindate.c_str());
    }

    if (!strEnddate.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, " and createdate <= '%s'", strEnddate.c_str());
    }

    if (0 == uiRecordType)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, " and logintime is not null and logouttime is not null");
    }
    else if (1 == uiRecordType)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, " and logintime is not null");
    }
    else if (2 == uiRecordType)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, " and logouttime is not null");
    }

    len = strlen(sql);
    snprintf(sql + len, size - len, " limit %d, %d", uiBeginIndex, uiPageSize);

    InteractiveProtoManagementHandler::AccessedDevice accessedDevice;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            accessedDevice.m_strDeviceID = strColumn;
            break;
        case 1:
            accessedDevice.m_strDeviceName = strColumn;
            break;
        case 2:
            accessedDevice.m_strLoginTime = strColumn;
            break;
        case 3:
            accessedDevice.m_strLogoutTime = strColumn;
            break;
        case 4:
            accessedDevice.m_uiDeviceType = boost::lexical_cast<unsigned int>(strColumn);
            result = accessedDevice;
            break;

        default:
            LOG_ERROR_RLD("QueryClusterDevice sql callback error, row num is " <<
                uiRowNum << " and column num is " << uiColumnNum << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryClusterDevice exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("QueryClusterDevice sql result is empty, sql is " << sql);
        return true;
    }

    for (auto &result : ResultList)
    {
        accessedDeviceList.push_back(std::move(boost::any_cast<InteractiveProtoManagementHandler::AccessedDevice>(result)));
    }

    return true;
}

bool ManagementCenter::QueryClusterUser(const std::string &strClusterID, const std::string &strBegindate, const std::string &strEnddate,
    const unsigned int uiRecordType, std::list<InteractiveProtoManagementHandler::AccessedUser> &accessedUserList,
    const unsigned int uiBeginIndex /*= 0*/, const unsigned int uiPageSize /*= 10*/)
{
    char sql[512] = { 0 };
    int size = sizeof(sql);
    int len;

    const char *sqlfmt = "select userid, username, useraliasname, logintime, logouttime, clienttype"
        " from t_cluster_accessed_user_info where clusterid = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strClusterID.c_str());

    if (!strBegindate.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, " and createdate >= '%s'", strBegindate.c_str());
    }

    if (!strEnddate.empty())
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, " and createdate <= '%s'", strEnddate.c_str());
    }

    if (0 == uiRecordType)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, " and logintime is not null and logouttime is not null");
    }
    else if (1 == uiRecordType)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, " and logintime is not null");
    }
    else if (2 == uiRecordType)
    {
        len = strlen(sql);
        snprintf(sql + len, size - len, " and logouttime is not null");
    }

    len = strlen(sql);
    snprintf(sql + len, size - len, " limit %d, %d", uiBeginIndex, uiPageSize);

    InteractiveProtoManagementHandler::AccessedUser accessedUser;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            accessedUser.m_strUserID = strColumn;
            break;
        case 1:
            accessedUser.m_strUserName = strColumn;
            break;
        case 2:
            accessedUser.m_strUserAliasname = strColumn;
            break;
        case 3:
            accessedUser.m_strLoginTime = strColumn;
            break;
        case 4:
            accessedUser.m_strLogoutTime = strColumn;
            break;
        case 5:
            accessedUser.m_uiClientType = boost::lexical_cast<unsigned int>(strColumn);
            result = accessedUser;

        default:
            LOG_ERROR_RLD("QueryClusterUser sql callback error, row num is " <<
                uiRowNum << " and column num is " << uiColumnNum << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryClusterUser exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("QueryClusterUser sql result is empty, sql is " << sql);
        return true;
    }

    for (auto &result : ResultList)
    {
        accessedUserList.push_back(std::move(boost::any_cast<InteractiveProtoManagementHandler::AccessedUser>(result)));
    }

    return true;
}

bool ManagementCenter::QueryClusterDeviceTotal(const std::string &strClusterID, unsigned int &uiDeviceTotal)
{
    char sql[256] = { 0 };
    const char *sqlfmt = "select count(id) from t_cluster_accessed_device_info where clusterid = '%s' and logouttime is null and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strClusterID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        uiDeviceTotal = boost::lexical_cast<unsigned int>(strColumn);
        result = uiDeviceTotal;
        LOG_INFO_RLD("QueryClusterDeviceTotal sql count(id) is " << uiDeviceTotal);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryClusterDeviceTotal exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("QueryClusterDeviceTotal sql result is empty, sql is " << sql);
        return false;
    }

    uiDeviceTotal = boost::any_cast<unsigned int>(ResultList.front());

    return true;
}

bool ManagementCenter::QueryClusterUserTotal(const std::string &strClusterID, unsigned int &uiUserTotal)
{
    char sql[256] = { 0 };
    const char *sqlfmt = "select count(id) from t_cluster_accessed_user_info where clusterid = '%s' and logouttime is null and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strClusterID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        uiUserTotal = boost::lexical_cast<unsigned int>(strColumn);
        result = uiUserTotal;
        LOG_INFO_RLD("QueryClusterUserTotal sql count(id) is " << uiUserTotal);
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("QueryClusterUserTotal exec sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("QueryClusterUserTotal sql result is empty, sql is " << sql);
        return false;
    }

    uiUserTotal = boost::any_cast<unsigned int>(ResultList.front());

    return true;
}

void ManagementCenter::PushClusterDevice(const std::list<InteractiveProtoManagementHandler::DeviceAccessRecord> &deviceAccessRecordList)
{
    if (deviceAccessRecordList.empty())
    {
        LOG_INFO_RLD("PushClusterDevice completed, input deviceAccessRecordList is empty");

        return;
    }

    boost::posix_time::ptime nowTime = boost::posix_time::second_clock::local_time();
    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(nowTime);
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));

    for (auto &accessRecord : deviceAccessRecordList)
    {
        InteractiveProtoManagementHandler::DeviceAccessRecord deviceAccessRecord;
        deviceAccessRecord.m_strAccessID = accessRecord.m_strAccessID;
        deviceAccessRecord.m_strClusterID = accessRecord.m_strClusterID;
        deviceAccessRecord.m_strCreateDate = strCurrentTime;
        deviceAccessRecord.m_uiStatus = NORMAL_STATUS;

        InteractiveProtoManagementHandler::AccessedDevice accessedDevice;
        deviceAccessRecord.m_accessedDevice.m_strDeviceID = accessRecord.m_accessedDevice.m_strDeviceID;
        deviceAccessRecord.m_accessedDevice.m_strDeviceName = accessRecord.m_accessedDevice.m_strDeviceName;
        deviceAccessRecord.m_accessedDevice.m_uiDeviceType = accessRecord.m_accessedDevice.m_uiDeviceType;
        deviceAccessRecord.m_accessedDevice.m_strLoginTime = accessRecord.m_accessedDevice.m_strLoginTime;
        deviceAccessRecord.m_accessedDevice.m_strLogoutTime = accessRecord.m_accessedDevice.m_strLogoutTime;
        deviceAccessRecord.m_accessedDevice.m_uiOnlineDuration = accessRecord.m_accessedDevice.m_uiOnlineDuration;

        InsertAccessedDevice(deviceAccessRecord);
    }
}

bool ManagementCenter::InsertAccessedDevice(const InteractiveProtoManagementHandler::DeviceAccessRecord &deviceAccessRecord)
{
    char sql[512] = { 0 };
    const char *sqlfmt = "insert into t_cluster_accessed_device_info"
        " (id, accessid, deviceid, devicename, clusterid, logintime, logouttime, onlineduration, devicetype, createdate, status)"
        " values(uuid(), '%s', '%s', '%s', '%s', '%s', '%s', %d, %d, '%s', %d)"
        " on duplicate key update set accessid = accessid, logouttime = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, deviceAccessRecord.m_strAccessID.c_str(), deviceAccessRecord.m_accessedDevice.m_strDeviceID.c_str(), deviceAccessRecord.m_accessedDevice.m_strDeviceName.c_str(),
        deviceAccessRecord.m_strClusterID.c_str(), deviceAccessRecord.m_accessedDevice.m_strLoginTime.c_str(), deviceAccessRecord.m_accessedDevice.m_strLogoutTime.c_str(), deviceAccessRecord.m_accessedDevice.m_uiOnlineDuration,
        deviceAccessRecord.m_accessedDevice.m_uiDeviceType, deviceAccessRecord.m_strCreateDate.c_str(), deviceAccessRecord.m_uiStatus, deviceAccessRecord.m_accessedDevice.m_strLogoutTime.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("InsertAccessedDevice exec sql failed, sql is " << sql);
        return false;
    }

    return true;
}

void ManagementCenter::PushClusterUser(const std::list<InteractiveProtoManagementHandler::UserAccessRecord> &userAccessRecordList)
{
    if (userAccessRecordList.empty())
    {
        LOG_INFO_RLD("PushClusterUser completed, input userAccessRecordList is empty");

        return;
    }

    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pos = strCurrentTime.find('T');
    strCurrentTime.replace(pos, 1, std::string(" "));

    for (auto &accessRecord : userAccessRecordList)
    {
        InteractiveProtoManagementHandler::UserAccessRecord userAccessRecord;
        userAccessRecord.m_strAccessID = accessRecord.m_strAccessID;
        userAccessRecord.m_strClusterID = accessRecord.m_strClusterID;
        userAccessRecord.m_strCreateDate = strCurrentTime;
        userAccessRecord.m_uiStatus = NORMAL_STATUS;

        InteractiveProtoManagementHandler::AccessedUser accessedUser;
        userAccessRecord.m_accessedUser.m_strUserID = accessRecord.m_accessedUser.m_strUserID;
        userAccessRecord.m_accessedUser.m_strUserName = accessRecord.m_accessedUser.m_strUserName;
        userAccessRecord.m_accessedUser.m_strUserAliasname = accessRecord.m_accessedUser.m_strUserAliasname;
        userAccessRecord.m_accessedUser.m_uiClientType = accessRecord.m_accessedUser.m_uiClientType;
        userAccessRecord.m_accessedUser.m_strLoginTime = accessRecord.m_accessedUser.m_strLoginTime;
        userAccessRecord.m_accessedUser.m_strLogoutTime = accessRecord.m_accessedUser.m_strLogoutTime;
        userAccessRecord.m_accessedUser.m_uiOnlineDuration = accessRecord.m_accessedUser.m_uiOnlineDuration;

        InsertAccessedUser(userAccessRecord);
    }
}

bool ManagementCenter::InsertAccessedUser(const InteractiveProtoManagementHandler::UserAccessRecord &userAccessRecord)
{
    char sql[512] = { 0 };
    const char *sqlfmt = "insert into t_cluster_accessed_user_info"
        " (id, accessid, userid, username, useraliasname, clusterid, logintime, logouttime, onlineduration, clienttype, createdate, status)"
        " values(uuid(), '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d, %d, '%s', %d)"
        " on duplicate key update set accessid = accessid, logouttime = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, userAccessRecord.m_strAccessID.c_str(), userAccessRecord.m_accessedUser.m_strUserID.c_str(), userAccessRecord.m_accessedUser.m_strUserName.c_str(),
        userAccessRecord.m_accessedUser.m_strUserAliasname.c_str(), userAccessRecord.m_strClusterID.c_str(), userAccessRecord.m_accessedUser.m_strLoginTime.c_str(),
        userAccessRecord.m_accessedUser.m_strLogoutTime.c_str(), userAccessRecord.m_accessedUser.m_uiOnlineDuration, userAccessRecord.m_accessedUser.m_uiClientType,
        userAccessRecord.m_strCreateDate.c_str(), userAccessRecord.m_uiStatus, userAccessRecord.m_accessedUser.m_strLogoutTime.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("InsertAccessedUser exec sql failed, sql is " << sql);
        return false;
    }

    return true;
}

