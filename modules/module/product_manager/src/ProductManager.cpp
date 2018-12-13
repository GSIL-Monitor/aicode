#include <map>
#include "ProductManager.h"
#include <boost/scope_exit.hpp>
#include <boost/thread/thread.hpp>
#include <boost/algorithm/string.hpp>
#include "CommonUtility.h"
#include "ReturnCode.h"
#include "mysql_impl.h"
#include "boost/lexical_cast.hpp"
#include "json/json.h"
#include "HttpClient.h"
#include <vector>

#include "Product_constants.h"
#include "Product_types.h"

std::string ProductManager::ALLOW_ACCESS = "allow";
std::string ProductManager::DISALLOW_ACCESS = "disallow";

ProductManager::ProductManager(const ParamInfo &pinfo) :
    m_ParamInfo(pinfo),
    m_DBRuner(1),
    m_pProtoHandler(new PassengerFlowProtoHandler),
    m_pMysql(new MysqlImpl),
    m_DBCache(m_pMysql),
    m_uiMsgSeq(0),
    m_DBTimer(NULL, 600),
    m_pushGetuiIOS(pinfo.m_strPushServerUrl, pinfo.m_strIOSAppID, pinfo.m_strIOSAppKey, pinfo.m_strIOSMasterSecret, pinfo.m_iAuthExpire),
    m_pushGetuiAndroid(pinfo.m_strPushServerUrl, pinfo.m_strAndroidAppID, pinfo.m_strAndroidAppKey, pinfo.m_strAndroidMasterSecret, pinfo.m_iAuthExpire)
{
}

ProductManager::~ProductManager()
{
    m_DBTimer.Stop();

    m_SessionMgr.Stop();

    m_DBRuner.Stop();

    delete m_pMysql;
    m_pMysql = NULL;
}

bool ProductManager::Init()
{
    if (!m_pMysql->Init(m_ParamInfo.m_strDBHost.c_str(), m_ParamInfo.m_strDBUser.c_str(), m_ParamInfo.m_strDBPassword.c_str(), m_ParamInfo.m_strDBName.c_str()))
    {
        LOG_ERROR_RLD("Init db failed, db host is " << m_ParamInfo.m_strDBHost
            << " db user is " << m_ParamInfo.m_strDBUser
            << " db pwd is " << m_ParamInfo.m_strDBPassword
            << " db name is " << m_ParamInfo.m_strDBName);
        return false;
    }

    m_SessionMgr.SetMemCacheAddRess(m_ParamInfo.m_strMemAddress, m_ParamInfo.m_strMemPort);
    m_SessionMgr.SetGlobalMemCacheAddRess(m_ParamInfo.m_strMemAddressGlobal, m_ParamInfo.m_strMemPortGlobal);
    //m_SessionMgr.SetSessionTimeoutCB(boost::bind(&ClusterAccessCollector::AddAccessTimeoutRecord, m_pClusterAccessCollector, _1, _2));

    if (!m_SessionMgr.Init())
    {
        LOG_ERROR_RLD("Session mgr init failed.");
        return false;
    }

    m_SessionMgr.SetUserLoginMutex(boost::lexical_cast<bool>(m_ParamInfo.m_strUserLoginMutex));
    m_SessionMgr.SetUerLoginKickout(boost::lexical_cast<bool>(m_ParamInfo.m_strUserKickoutType));

    auto TmFunc = [&](const boost::system::error_code &ec) ->void
    {
        if (!m_pMysql->QueryExec(std::string("SET NAMES utf8")))
        {
            LOG_ERROR_RLD("Exec sql charset to utf8 failed, sql is SET NAMES utf8");
        }
        else
        {
            LOG_INFO_RLD("Set db to utf8 success.");
        }
    };

    m_DBTimer.SetTimeOutCallBack(TmFunc);

    m_DBTimer.Run(true);

    m_DBRuner.Run();

    m_SessionMgr.Run();

    m_pushGetuiIOS.Init();

    m_pushGetuiAndroid.Init();

    LOG_INFO_RLD("PassengerFlowManager init success");

    return true;
}

void ProductManager::AddProduct(AddProductRT& _return, const std::string& strSid, const std::string& strUserID, const ProductInfo& pdt)
{
    std::string strPdtID = CreateUUID();
    bool blResult = false;
    BOOST_SCOPE_EXIT(&blResult, this_, &_return, &g_Product_constants, &strPdtID)
    {
        ProductRTInfo rtd;
        rtd.__set_iRtCode(blResult ? g_Product_constants.PDT_SUCCESS_CODE : g_Product_constants.PDT_FAILED_CODE);
        _return.__set_rtcode(rtd);

        if (blResult)
        {
            _return.__set_strPdtID(strPdtID);
        }
    }
    BOOST_SCOPE_EXIT_END

    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_product_info (id, pdtid, pdtname, typeinfo, aliasname, pdtprice, pic, extend)"
        " values (uuid(), '%s', '%s', %d, '%s', %f, '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strPdtID.c_str(), pdt.strName.c_str(), pdt.iType, pdt.strAliasName.c_str(), pdt.dlPrice, pdt.strPic.c_str(), pdt.strExtend.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Add product exec sql failed, sql is " << sql);
        return;
    }

    blResult = true;
}

void ProductManager::RemoveProduct(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID)
{
    bool blResult = false;
    BOOST_SCOPE_EXIT(&blResult, this_, &_return, &g_Product_constants)
    {
        _return.__set_iRtCode(blResult ? g_Product_constants.PDT_SUCCESS_CODE : g_Product_constants.PDT_FAILED_CODE);
    }
    BOOST_SCOPE_EXIT_END

    char sql[1024] = { 0 };
    const char *sqlfmt = "delete from t_product_info where pdtid = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, strPdtID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Remove product exec sql failed, sql is " << sql);
        return;
    }

    blResult = true;
}

void ProductManager::ModifyProduct(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID, const ProductInfo& pdt)
{
    bool blResult = false;
    BOOST_SCOPE_EXIT(&blResult, this_, &_return, &g_Product_constants)
    {
        _return.__set_iRtCode(blResult ? g_Product_constants.PDT_SUCCESS_CODE : g_Product_constants.PDT_FAILED_CODE);
    }
    BOOST_SCOPE_EXIT_END

    bool blModified = false;

    char sql[2048] = { 0 };
    int size = sizeof(sql);
    int len = snprintf(sql, size, "update t_product_info set id = id");

    if (0xFFFFFFFF != pdt.dlPrice)
    {
        len += snprintf(sql + len, size - len, ", pdtprice = %f", pdt.dlPrice);
        blModified = true;
    }

    if (!pdt.strName.empty())
    {
        len += snprintf(sql + len, size - len, ", pdtname = '%s'", pdt.strName.c_str());
        blModified = true;
    }

    if (!pdt.strAliasName.empty())
    {
        len += snprintf(sql + len, size - len, ", aliasname = '%s'", pdt.strAliasName.c_str());
        blModified = true;
    }

    if (-1 != pdt.iType)
    {
        len += snprintf(sql + len, size - len, ", typeinfo = %d", pdt.iType);
        blModified = true;
    }

    if (!pdt.strPic.empty())
    {
        len += snprintf(sql + len, size - len, ", pic = '%s'", pdt.strPic.c_str());
        blModified = true;
    }

    if (!pdt.strExtend.empty())
    {
        len += snprintf(sql + len, size - len, ", extend = '%s'", pdt.strExtend.c_str());
        blModified = true;
    }

    if (!blModified)
    {
        LOG_INFO_RLD("Modify product completed, product info is not changed");
        return;
    }

    snprintf(sql + len, size - len, " where pdtid = '%s'", strPdtID.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Modify product exec sql failed, sql is " << sql);
        return;
    }

    blResult = true;
}

void ProductManager::QueryProduct(QueryProductRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID)
{
    bool blResult = false;
    BOOST_SCOPE_EXIT(&blResult, this_, &_return, &g_Product_constants)
    {
        ProductRTInfo rtd;
        rtd.__set_iRtCode(blResult ? g_Product_constants.PDT_SUCCESS_CODE : g_Product_constants.PDT_FAILED_CODE);
        _return.__set_rtcode(rtd);

    }
    BOOST_SCOPE_EXIT_END

    char sql[1024] = { 0 };
    const char *sqlfmt = "select pdtid, pdtname, typeinfo, aliasname, pdtprice, pic, extend from"
        " t_product_info where pdtid = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strPdtID.c_str());
    
    ProductInfo pdt;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            pdt.__set_strID(strColumn);
            break;
        case 1:
            pdt.__set_strName(strColumn);            
            break;
        case 2:
            pdt.__set_iType(boost::lexical_cast<int>(strColumn));            
            break;
        case 3:
            pdt.__set_strAliasName(strColumn);            
            break;
        case 4:            
            pdt.__set_dlPrice(boost::lexical_cast<double>(strColumn));            
            break;
        case 5:
            pdt.__set_strPic(strColumn);            
            break;
        case 6:
            pdt.__set_strExtend(strColumn);            
            break;

        default:
            LOG_ERROR_RLD("Query product sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("Query product exec sql failed, sql is " << sql);
        return;
    }

    std::vector<ProductProperty> pptList;
    if (!QueryProductProperty(strPdtID, pptList))
    {
        LOG_ERROR_RLD("Query product property exec sql failed, sql is " << sql);
        return;
    }
    
    pdt.__set_pptList(pptList);
    _return.__set_pdt(pdt);

    blResult = true;
}

void ProductManager::QueryAllProduct(QueryAllProductRT& _return, const std::string& strSid, const std::string& strUserID)
{
    bool blResult = false;
    BOOST_SCOPE_EXIT(&blResult, this_, &_return, &g_Product_constants)
    {
        ProductRTInfo rtd;
        rtd.__set_iRtCode(blResult ? g_Product_constants.PDT_SUCCESS_CODE : g_Product_constants.PDT_FAILED_CODE);
        _return.__set_rtcode(rtd);

    }
    BOOST_SCOPE_EXIT_END

    char sql[1024] = { 0 };
    const char *sqlfmt = "select pdtid, pdtname, typeinfo, aliasname, pdtprice, pic, extend from"
        " t_product_info where status = 0";
    snprintf(sql, sizeof(sql), sqlfmt);

    ProductInfo pdt;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            pdt.strID = strColumn;
            break;
        case 1:
            pdt.strName = strColumn;
            break;
        case 2:
            pdt.iType = boost::lexical_cast<int>(strColumn);
            break;
        case 3:
            pdt.strAliasName = strColumn;
            break;
        case 4:
            pdt.dlPrice = boost::lexical_cast<double>(strColumn);
            break;
        case 5:
            pdt.strPic = strColumn;
            break;
        case 6:
            pdt.strExtend = strColumn;
            _return.pdtlist.push_back(pdt);

        default:
            LOG_ERROR_RLD("Query product sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("Query product exec sql failed, sql is " << sql);
        return;
    }

    for (auto itBegin = _return.pdtlist.begin(), itEnd = _return.pdtlist.end(); itBegin != itEnd; ++itBegin)
    {
        if (!QueryProductProperty(itBegin->strID, itBegin->pptList))
        {
            LOG_ERROR_RLD("Query product property exec sql failed, sql is " << sql);
            return;
        }
    }

    blResult = true;
}

void ProductManager::AddProductProperty(AddProductPropertyRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID, 
    const ProductProperty& pdtppt)
{
    std::string strPdtpptID = CreateUUID();

    bool blResult = false;
    BOOST_SCOPE_EXIT(&blResult, this_, &_return, &g_Product_constants, &strPdtpptID)
    {
        ProductRTInfo rtd;
        rtd.__set_iRtCode(blResult ? g_Product_constants.PDT_SUCCESS_CODE : g_Product_constants.PDT_FAILED_CODE);
        _return.__set_rtcode(rtd);

        if (blResult)
        {
            _return.__set_strPdtpptID(strPdtpptID);
        }

    }
    BOOST_SCOPE_EXIT_END

    char sql[1024] = { 0 };
    const char *sqlfmt = "insert into t_product_property (id, pdtpptid, pdtid, property_type, property_name, property_value, extend)"
        " values (uuid(), '%s', '%s', %d, '%s', '%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strPdtpptID.c_str(), strPdtID.c_str(), pdtppt.iType, pdtppt.strName.c_str(), pdtppt.strValue.c_str(), pdtppt.strExtend.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Add product property exec sql failed, sql is " << sql);
        return;
    }

    blResult = true;
}

void ProductManager::RemoveProductProperty(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID, 
    const std::string& strPdtpptID)
{
    bool blResult = false;
    BOOST_SCOPE_EXIT(&blResult, this_, &_return, &g_Product_constants)
    {
        _return.__set_iRtCode(blResult ? g_Product_constants.PDT_SUCCESS_CODE : g_Product_constants.PDT_FAILED_CODE);
    }
    BOOST_SCOPE_EXIT_END

    char sql[1024] = { 0 };
    int size = sizeof(sql);
    const char *sqlfmt = "delete from t_product_property where pdtid = '%s'";
    int len = snprintf(sql, sizeof(sql), sqlfmt, strPdtID.c_str());

    if (!strPdtpptID.empty())
    {
        len += snprintf(sql + len, size - len, " and pdtpptid = '%s'", strPdtpptID.c_str());        
    }

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Remove product property exec sql failed, sql is " << sql);
        return;
    }

    blResult = true;
}


void ProductManager::AddOrd(AddOrdRT& _return, const std::string& strSid, const std::string& strUserID, const OrderInfo& ord)
{

}

void ProductManager::RemoveOrd(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID)
{

}

void ProductManager::ModifyOrd(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID, const OrderInfo& ord)
{

}

void ProductManager::QueryOrd(QueryOrdRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID)
{

}

void ProductManager::QueryAllOrd(QueryAllOrdRT& _return, const std::string& strSid, const std::string& strUserID, const QueryAllOrdParam& qryparam)
{

}

bool ProductManager::PreCommonHandler(int iCmd, const std::string &strSid, int *piRet)
{
    //
    //ReturnInfo::RetCode(ReturnInfo::SESSION_TIMEOUT);

    bool blResult = false;

    //if (PassengerFlowProtoHandler::CustomerFlowMsgType::LoginReq_USR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::RegisterUserReq_USR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::RegisterUserRsp_USR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::LoginReq_DEV_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::AddFileReq_DEV_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::RetrievePwdReq_USR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAccessDomainNameReq_DEV_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAccessDomainNameReq_USR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAppUpgradeReq_USR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::QueryFirmwareUpgradeReq_DEV_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::AddConfigurationReq_MGR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::DeleteConfigurationReq_MGR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::ModifyConfigurationReq_MGR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::QueryAllConfigurationReq_MGR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::QueryUploadURLReq_MGR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::P2pInfoReq_DEV_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::QueryIfP2pIDValidReq_USR_T == req.m_MsgType ||
    //    PassengerFlowProtoHandler::CustomerFlowMsgType::QueryPlatformPushStateReq_DEV_T == req.m_MsgType)
    /*{
        LOG_INFO_RLD("PreCommonHandler return true because no need to check and msg type is " << req.m_MsgType);
        blResult = true;
        return blResult;
    }*/

    //通用预处理操作
    if (!m_SessionMgr.Exist(strSid))
    {
        LOG_ERROR_RLD("PreCommonHandler return false because session is invalid and sid is " << strSid);
        
        if (piRet)
        {
            *piRet = ReturnInfo::SESSION_TIMEOUT;
        }

        blResult = false;
        return blResult;
    }
    
    blResult = true;
    return blResult;
}

bool ProductManager::PushMessage(const std::string &strTitle, const std::string &strContent, const std::string &strPayload, const std::string &strUserID)
{
    MessagePush_Getui::PushMessage message;
    message.strTitle = strTitle;
    message.strNotyContent = strContent;
    message.strPayloadContent = strPayload; //strContent;
    message.bIsOffline = true;
    message.iOfflineExpireTime = 10000000;//3600;
    //message.strClientID = "9fff548fac1537a7963a49a9b191e195";
    message.strAlias = strUserID;

    //无法确定处理人登录时使用的终端，所以Android和iOS都推送
    m_pushGetuiIOS.PushSingle(message, MessagePush_Getui::PLATFORM_IOS);
    m_pushGetuiAndroid.PushSingle(message, MessagePush_Getui::PLATFORM_ANDROID);

    return true;
}

std::string ProductManager::CurrentTime()
{
    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    strCurrentTime.replace(strCurrentTime.find_first_of('T'), 1, std::string(" "));

    return strCurrentTime;
}

int ProductManager::Year(const std::string &strDate)
{
    return boost::lexical_cast<int>(strDate.substr(0, 4));
}

int ProductManager::Month(const std::string &strDate)
{
    return boost::lexical_cast<int>(strDate.substr(5, 2));
}

int ProductManager::Quarter(const std::string &strDate)
{
    return (Month(strDate) - 1) / 4 + 1;
}

int ProductManager::MonthDuration(const std::string &strBeginDate, const std::string &strEndDate)
{
    /*int beginYear = Year(strBeginDate);
    int endYear = Year(strEndDate);
    int beginMonth = Month(strBeginDate);
    int endMonth = Month(strEndDate);

    if (beginYear == endYear)
    {
        return endMonth - beginMonth + 1;
    }
    else
    {
        return (endYear - beginYear - 1) * 12 + endMonth + 12 - beginMonth + 1;
    }*/

    return (Year(strEndDate) - Year(strBeginDate)) * 12 + Month(strEndDate) - Month(strBeginDate) + 1;
}

int ProductManager::QuarterDuration(const std::string &strBeginDate, const std::string &strEndDate)
{
    return (Year(strEndDate) - Year(strBeginDate)) * 4 + Quarter(strEndDate) - Quarter(strBeginDate) + 1;
}

int ProductManager::TimePrecisionScale(const std::string &strDate, const unsigned int uiTimePrecision)
{
    boost::posix_time::ptime epoch = boost::posix_time::time_from_string("1970-01-01 00:00:00");
    boost::posix_time::ptime date = boost::posix_time::time_from_string(strDate);
    return ((date - epoch).total_seconds() / uiTimePrecision) * uiTimePrecision;
}

bool ProductManager::QueryProductProperty(const std::string &strPdtID, std::vector<ProductProperty> &pdtpptlist)
{
    char sql[1024] = { 0 };
    const char *sqlfmt = "select pdtpptid, property_type, property_name, property_value, extend from"
        " t_product_property where pdtid = '%s' and status = 0";
    snprintf(sql, sizeof(sql), sqlfmt, strPdtID.c_str());

    ProductProperty ppt;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &result)
    {
        switch (uiColumnNum)
        {
        case 0:
            ppt.__set_strID(strColumn);
            break;
        case 1:
            ppt.__set_iType(boost::lexical_cast<int>(strColumn));            
            break;
        case 2:
            ppt.__set_strName(strColumn);            
            break;
        case 3:
            ppt.__set_strValue(strColumn);            
            break;
        case 4:
            ppt.__set_strExtend(strColumn);            
            pdtpptlist.push_back(ppt);
            break;
        default:
            LOG_ERROR_RLD("Query product property sql callback error, row num is " << uiRowNum
                << " and column num is " << uiColumnNum
                << " and value is " << strColumn);
            break;
        }
    };

    std::list<boost::any> ResultList;
    if (!m_DBCache.QuerySql(std::string(sql), ResultList, SqlFunc, true))
    {
        LOG_ERROR_RLD("Query product property exec sql failed, sql is " << sql);        
        return false;
    }

    return true;
}
