#include "HttpMsgHandler.h"
#include <boost/algorithm/string.hpp>  
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scope_exit.hpp>
#include "json/json.h"
#include "util.h"
#include "mime_types.h"
#include "LogRLD.h"
#include "InteractiveProtoHandler.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include "boost/regex.hpp"
#include "ReturnCode.h"
#include "boost/date_time/gregorian/gregorian.hpp"

const std::string HttpMsgHandler::SUCCESS_CODE = "0";
const std::string HttpMsgHandler::SUCCESS_MSG = "Ok";
const std::string HttpMsgHandler::FAILED_CODE = "-1";
const std::string HttpMsgHandler::FAILED_MSG = "Inner failed";

const std::string HttpMsgHandler::REGISTER_USER_ACTION("register_user");

const std::string HttpMsgHandler::UNREGISTER_USER_ACTION("unregister_user");

const std::string HttpMsgHandler::QUERY_USER_INFO_ACTION("query_userinfo");

const std::string HttpMsgHandler::MODIFY_USER_INFO_ACTION("modify_userinfo");

const std::string HttpMsgHandler::USER_LOGIN_ACTION("user_login");

const std::string HttpMsgHandler::USER_LOGOUT_ACTION("user_logout");

const std::string HttpMsgHandler::USER_SHAKEHAND_ACTION("user_shakehand");

const std::string HttpMsgHandler::ADD_DEVICE_ACTION("add_device");

const std::string HttpMsgHandler::DELETE_DEVICE_ACTION("delete_device");

const std::string HttpMsgHandler::MODIFY_DEVICE_ACTION("modify_device");

const std::string HttpMsgHandler::QUERY_DEVICE_INFO_ACTION("query_deviceinfo");

const std::string HttpMsgHandler::QUERY_DEVICE_OF_USER_ACTION("query_device_of_user");

const std::string HttpMsgHandler::QUERY_USER_OF_DEVICE_ACTION("query_user_of_device");

const std::string HttpMsgHandler::SHARING_DEVICE_ACTION("sharing_device");

const std::string HttpMsgHandler::CANCELSHARED_DEVICE_ACTION("cancelshared_device");

const std::string HttpMsgHandler::ADD_FRIEND_ACTION("add_friend");

const std::string HttpMsgHandler::DELETE_FRIEND_ACTION("delete_friend");

const std::string HttpMsgHandler::QUERY_FRIEND_ACTION("query_friend");

const std::string HttpMsgHandler::P2P_INFO_ACTION("query_p2pserver");

const std::string HttpMsgHandler::DEVICE_LOGIN_ACTION("device_login");

const std::string HttpMsgHandler::DEVICE_P2P_INFO_ACTION("device_query_p2pserver");

const std::string HttpMsgHandler::DEVICE_SHAKEHAND_ACTION("device_shakehand");

const std::string HttpMsgHandler::DEVICE_LOGOUT_ACTION("device_logout");

const std::string HttpMsgHandler::DEVICE_SET_PROPERTY_ACTION("device_set_property");

const std::string HttpMsgHandler::QUERY_USER_FILE_ACTION("query_file");

const std::string HttpMsgHandler::DOWNLOAD_USER_FILE_ACTION("download_file");

const std::string HttpMsgHandler::DELETE_USER_FILE_ACTION("delete_file");

const std::string HttpMsgHandler::ADD_FILE_ACTION("add_file");

const std::string HttpMsgHandler::RETRIEVE_PWD_ACTION("retrieve_pwd");

const std::string HttpMsgHandler::DEVICE_QUERY_TIMEZONE_ACTION("device_query_timezone");

const std::string HttpMsgHandler::USER_QUERY_ACCESS_DOMAIN_ACTION("user_query_access_domain");

const std::string HttpMsgHandler::DEVICE_QUERY_ACCESS_DOMAIN_ACTION("device_query_access_domain");

const std::string HttpMsgHandler::DEVICE_QUERY_UPDATE_SERVICE_ACTION("device_query_update_service");

const std::string HttpMsgHandler::QUERY_UPLOAD_URL_ACTION("query_upload_url");

const std::string HttpMsgHandler::ADD_CONFIG_ACTION("add_configuration");

const std::string HttpMsgHandler::DELETE_CONFIG_ACTION("delete_configuration");

const std::string HttpMsgHandler::MOD_CONFIG_ACTION("modify_configuration");

const std::string HttpMsgHandler::QUERY_CONFIG_ACTION("query_all_configuration");

const std::string HttpMsgHandler::QUERY_APP_UPGRADE_ACTION("query_app_upgrade");

const std::string HttpMsgHandler::QUERY_DEV_UPGRADE_ACTION("query_firmware_upgrade");

const std::string HttpMsgHandler::QUERY_DEVICE_PARAM_ACTION("query_device_parameter");

const std::string HttpMsgHandler::CHECK_DEVICE_P2PID_ACTION("check_device_p2pid");

const std::string HttpMsgHandler::QUERY_PUSH_STATUS_ACTION("device_query_push_status");

const std::string HttpMsgHandler::DEVICE_EVENT_REPORT_ACTION("device_event_report");

const std::string HttpMsgHandler::QUERY_DEVICE_EVENT_ACTION("device_event_query");

const std::string HttpMsgHandler::DELETE_DEVICE_EVENT_ACTION("device_event_delete");

const std::string HttpMsgHandler::MODIFY_DEVICE_EVENT_ACTION("modify_device_event");

const std::string HttpMsgHandler::ADD_USER_SPACE_ACTION("add_user_space");

const std::string HttpMsgHandler::DELETE_USER_SPACE_ACTION("delete_user_space");

const std::string HttpMsgHandler::MODIFY_USER_SPACE_ACTION("modify_user_space");

const std::string HttpMsgHandler::QUERY_USER_SPACE_ACTION("query_user_space");

const std::string HttpMsgHandler::QUERY_STORAGE_SPACE_ACTION("query_storage_info");

HttpMsgHandler::HttpMsgHandler(const ParamInfo &parminfo):
m_ParamInfo(parminfo),
m_pInteractiveProtoHandler(new InteractiveProtoHandler)
{

}

HttpMsgHandler::~HttpMsgHandler()
{

}

int HttpMsgHandler::RspFuncCommonAction(CommMsgHandler::Packet &pt, int *piRetCode, RspFuncCommon rspfunc)
{
    const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);
    int iPreCode = 0;
    if (!PreCommonHandler(strMsgReceived, iPreCode))
    {
        ReturnInfo::RetCode(iPreCode);
        return CommMsgHandler::FAILED;
    }

    if (NULL == rspfunc)
    {
        LOG_ERROR_RLD("Rsp function is null");
        return CommMsgHandler::FAILED;
    }

    int iRspfuncRet = rspfunc(pt);

    ReturnInfo::RetCode(*piRetCode);

    return iRspfuncRet;
}

bool HttpMsgHandler::RegisterUserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));                   
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END
        
    auto itFind = pMsgInfoMap->find("username");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User name not found.");
        return blResult;
    }
    const std::string strUserName = itFind->second;


    itFind = pMsgInfoMap->find("userpwd");
    if (pMsgInfoMap->end() == itFind)
    {        
        LOG_ERROR_RLD("User password not found.");
        return blResult;
    }
    const std::string strUserPwd = itFind->second;
    
    itFind = pMsgInfoMap->find("type");    
    if (pMsgInfoMap->end() == itFind)
    {        
        LOG_ERROR_RLD("User type not found.");
        return blResult;
    }
    const std::string strType = itFind->second;

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    std::string strAliasName;
    itFind = pMsgInfoMap->find("aliasname");
    if (pMsgInfoMap->end() != itFind)
    {
        strAliasName = itFind->second;
    }

    itFind = pMsgInfoMap->find("email");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User email not found.");
        return blResult;
    }
    const std::string strEmail = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Register user info received and user name is " << strUserName << " and user pwd is " << strUserPwd << " and user type is " << strType
         << " and extend is [" << strExtend << "]" << " and alias name is " << strAliasName << " and email is " << strEmail);

    std::string strUserID;
    if (!RegisterUser(strUserName, strUserPwd, strType, strExtend, strAliasName, strEmail, strUserID))
    {
        LOG_ERROR_RLD("Register user handle failed");
        return blResult;
    }
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("userid", strUserID));
    
    blResult = true;

    return blResult;
}

bool HttpMsgHandler::UnRegisterUserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("username");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User name not found.");
        return blResult;
    }
    const std::string strUserName = itFind->second;


    itFind = pMsgInfoMap->find("userpwd");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User password not found.");
        return blResult;
    }
    const std::string strUserPwd = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserid = itFind->second;

    std::string strValue;
    itFind = pMsgInfoMap->find("value");
    if (pMsgInfoMap->end() != itFind)
    {
        strValue = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Unregister user info received and user name is " << strUserName << " and user pwd is " << strUserPwd << " and user id is " << strUserid
        << " and strValue is [" << strValue << "]" << " and session id is " << strSid);

    if (!UnRegisterUser(strSid, strUserid, strUserName, strUserPwd))
    {
        LOG_ERROR_RLD("Unregister user handle failed and user id is " << strUserid << " and user name is " << strUserName << " and user pwd is " << strUserPwd
             << " and sid is " << strSid);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("userid", strUserid));

    blResult = true;
    
    return blResult;
}

bool HttpMsgHandler::QueryUserInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserid = itFind->second;

    std::string strValue;
    itFind = pMsgInfoMap->find("value");
    if (pMsgInfoMap->end() != itFind)
    {
        strValue = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query user info received and  user id is " << strUserid << " and strValue is [" << strValue << "]"
        << " and session id is " << strSid);

    InteractiveProtoHandler::User userinfo;
    if (!QueryUserInfo<InteractiveProtoHandler::User>(strSid, strUserid, userinfo))
    {
        LOG_ERROR_RLD("Query user info handle failed and user id is " << strUserid << " and sid is " << strSid);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("userid", userinfo.m_strUserID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("username", userinfo.m_strUserName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("type", boost::lexical_cast<std::string>(userinfo.m_uiTypeInfo)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("createdate", userinfo.m_strCreatedate));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("extend", userinfo.m_strExtend));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("aliasname", userinfo.m_strAliasName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("email", userinfo.m_strEmail));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::ModifyUserInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserid = itFind->second;

    std::string strAliasName;
    itFind = pMsgInfoMap->find("aliasname");
    if (pMsgInfoMap->end() != itFind)
    {
        strAliasName = itFind->second;
    }

    std::string strNewUserPwd;
    itFind = pMsgInfoMap->find("newuserpwd");
    if (pMsgInfoMap->end() != itFind)
    {
        strNewUserPwd = itFind->second;
    }

    std::string strOldUserPwd;
    itFind = pMsgInfoMap->find("olduserpwd");
    if (pMsgInfoMap->end() != itFind)
    {
        strOldUserPwd = itFind->second;
    }

    if ((strNewUserPwd.empty() && !strOldUserPwd.empty()) || (!strNewUserPwd.empty() && strOldUserPwd.empty()))
    {
        LOG_ERROR_RLD("User new pwd and old pwd were both needed, new pwd is " << strNewUserPwd << " and old pwd is " << strOldUserPwd);
        return blResult;
    }


    unsigned int uiType = 0xFFFFFFFF;
    itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiType = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Modify user info type is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Modify user info type is invalid and input is " << itFind->second);
            return blResult;
        }
    }

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }
    
    std::string strEmail;
    itFind = pMsgInfoMap->find("email");
    if (pMsgInfoMap->end() != itFind)
    {
        strEmail = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify user info received and  user id is " << strUserid << " and user name is " << strAliasName 
        << " and user new pwd is " << strNewUserPwd << " and user old pwd is " << strOldUserPwd << " and user type is " << uiType 
        << " and extend is " << strExtend
        << " and alias name is " << strAliasName << " and email is " << strEmail
        << " and session id is " << strSid);

    if (!ModifyUserInfo(strSid, strUserid, strAliasName, strNewUserPwd, strOldUserPwd, uiType, strExtend, strAliasName, strEmail))
    {
        LOG_ERROR_RLD("Modify user info handle failed and user id is " << strUserid << " and sid is " << strSid);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::RetrievePwdHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("username");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User name not found.");
        return blResult;
    }
    const std::string strUserName = itFind->second;

    itFind = pMsgInfoMap->find("email");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Email not found.");
        return blResult;
    }
    const std::string strEmail = itFind->second;

    //////////////
    itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Type not found.");
        return blResult;
    }
    const std::string strType = itFind->second;

    unsigned int uiType = 0;
    try
    {
        uiType = boost::lexical_cast<unsigned int>(strType);
    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("Type info is invalid and error msg is " << e.what() << " and input index is " << itFind->second);
        return blResult;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Type info is invalid and input index is " << itFind->second);
        return blResult;
    }
    /////////////

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Retrieve pwd info received and  user name is " << strUserName << " and email is " << strEmail);

    if (!RetrievePwd(strUserName, strEmail, uiType))
    {
        LOG_ERROR_RLD("Retrieve pwd handle failed and user name is " << strUserName << " and email is " << strEmail);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;

}

bool HttpMsgHandler::UserLoginHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsRelationList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsRelationList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["data"] = jsRelationList;
                
            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }
        
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("username");
    auto itFind2 = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind && pMsgInfoMap->end() == itFind2)
    {
        LOG_ERROR_RLD("User name and user id not found.");
        return blResult;
    }
    const std::string strUsername = pMsgInfoMap->end() == itFind ? "" : itFind->second;
    
    itFind = pMsgInfoMap->find("userpwd");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User pwd not found.");
        return blResult;
    }
    const std::string strUserpwd = itFind->second;

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    std::string strValue;
    itFind = pMsgInfoMap->find("value");
    if (pMsgInfoMap->end() != itFind)
    {
        strValue = itFind->second;
    }

    //////////////
    itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Type not found.");
        return blResult;
    }
    const std::string strType = itFind->second;

    unsigned int uiType = 0;
    try
    {
        uiType = boost::lexical_cast<unsigned int>(strType);
    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("Type info is invalid and error msg is " << e.what() << " and input index is " << itFind->second);
        return blResult;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Type info is invalid and input index is " << itFind->second);
        return blResult;
    }
    /////////////

    itFind = pMsgInfoMap->find("terminaltype");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Terminal type not found.");
        return blResult;
    }
    const std::string strTerminalType = itFind->second;

    unsigned int uiTerminalType = 0;
    try
    {
        uiTerminalType = boost::lexical_cast<unsigned int>(strTerminalType);
    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("Terminal Type info is invalid and error msg is " << e.what() << " and input index is " << itFind->second);
        return blResult;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Terminal Type info is invalid and input index is " << itFind->second);
        return blResult;
    }
    
    std::string strUserID = pMsgInfoMap->end() == itFind2 ? "" : itFind2->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Login user info received and  user name is " << strUsername <<
        " and user id is " << strUserID << 
        " and user pwd is " << strUserpwd << " and terminal type is " << uiTerminalType <<
        " and strExtend is [" << strExtend << "]" << " and strValue is [" << strValue << "]");
        
    std::string strSid;
    std::list<InteractiveProtoHandler::Relation> relist;
    std::list<std::string> strDevNameList;
    if (!UserLogin<InteractiveProtoHandler::Relation>(strUsername, strUserpwd, uiType, uiTerminalType, relist, strUserID, strSid, strDevNameList))
    {
        LOG_ERROR_RLD("Login user handle failed and user name is " << strUsername << " and user pwd is " << strUserpwd);
        return blResult;
    }

    if (relist.size() != strDevNameList.size())
    {
        LOG_ERROR_RLD("Login user rsp info handle failed and relation list size is " << relist.size() << " and device name list size is " << strDevNameList.size());
        ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));
        return blResult;
    }

    auto itBegin2 = strDevNameList.begin();
    auto itEnd2 = strDevNameList.end();
    auto itBegin = relist.begin();
    auto itEnd = relist.end();
    while (itBegin != itEnd && itBegin2 != itEnd2)
    {
        Json::Value jsRelation;
        jsRelation["userid"] = itBegin->m_strUserID;
        jsRelation["devid"] = itBegin->m_strDevID;
        jsRelation["relation"] = itBegin->m_uiRelation;
        jsRelation["begindate"] = itBegin->m_strBeginDate;
        jsRelation["enddate"] = itBegin->m_strEndDate;
        jsRelation["extend"] = itBegin->m_strValue;
        jsRelation["devname"] = *itBegin2;

        jsRelationList.append(jsRelation);

        ++itBegin;
        ++itBegin2;
    }
        
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("sid", strSid));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("userid", strUserID));


    blResult = true;

    return blResult;
}

bool HttpMsgHandler::UserLogoutHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;
        
    std::string strValue;
    itFind = pMsgInfoMap->find("value");
    if (pMsgInfoMap->end() != itFind)
    {
        strValue = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Logout user info received and  user id is " << strUserID << " and strValue is [" << strValue << "]" 
        << " and session id is " << strSid);

    if (!UserLogout(strSid, strUserID))
    {
        LOG_ERROR_RLD("Logout user handle failed and user id is " << strUserID << " and sid is " << strSid);
        return blResult;
    }
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::ConfigInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    bool blResult = false;


    blResult = true;
    return blResult;
}

bool HttpMsgHandler::ShakehandHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    
    const std::string strUserID = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Shakehand info received and  user id is " << strUserID << " and session id is " << strSid);

    if (!Shakehand(strSid, strUserID))
    {
        LOG_ERROR_RLD("Shakehand handle failed and user id is " << strUserID << " and sid is " << strSid);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::AddDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    itFind = pMsgInfoMap->find("devname");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device name not found.");
        return blResult;
    }
    const std::string strDevName = itFind->second;

    std::string strDevPwd;
    itFind = pMsgInfoMap->find("devpwd");
    if (pMsgInfoMap->end() != itFind)
    {
        strDevPwd = itFind->second;
    }
    
    itFind = pMsgInfoMap->find("devtype");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device type not found.");
        return blResult;
    }
    const std::string strDevType = itFind->second;

    std::string strDevExtend;
    itFind = pMsgInfoMap->find("devextend");
    if (pMsgInfoMap->end() != itFind)
    {
        strDevExtend = itFind->second;
    }
    
    std::string strDevInnerInfo;
    itFind = pMsgInfoMap->find("devinnerinfo");
    if (pMsgInfoMap->end() != itFind)
    {
        strDevInnerInfo = itFind->second;
    }

    std::string strP2pid;
    itFind = pMsgInfoMap->find("p2pid");
    if (pMsgInfoMap->end() != itFind)
    {
        strP2pid = itFind->second;
    }

    std::string strDomainname;
    itFind = pMsgInfoMap->find("domainname");
    if (pMsgInfoMap->end() != itFind)
    {
        strDomainname = itFind->second;
    }

    std::string strIpaddress;
    itFind = pMsgInfoMap->find("ipaddress");
    if (pMsgInfoMap->end() != itFind)
    {
        strIpaddress = itFind->second;
    }

    DeviceIf devif;
    devif.m_strDevExtend = strDevExtend;
    devif.m_strDevID = strDevID;
    devif.m_strDevInnerInfo = strDevInnerInfo;
    devif.m_strDevName = strDevName;
    devif.m_strDevPwd = strDevPwd;
    devif.m_strDevType = strDevType;
    devif.m_strDomainname = strDomainname;
    devif.m_strIpaddress = strIpaddress;
    devif.m_strP2pid = strP2pid;
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Add device info received and  user id is " << strUserID << " and devcie id is " << strDevID << " and device name is " << strDevName
        << " and device pwd is [" << strDevPwd << "]" << " and device type is " << strDevType << " and device extend is [" << strDevExtend << "]"
        << " and device inner info is [" << strDevInnerInfo << "]" << " and p2pid is " << strP2pid << " and domain name is " << strDomainname
        << " and ip address is " << strIpaddress
        << " and session id is " << strSid);

    std::string strDevIDOut;
    if (!AddDevice(strSid, strUserID, devif, strDevIDOut))
    {
        LOG_ERROR_RLD("Add device handle failed and user id is " << strUserID << " and sid is " << strSid << " and device id is " << strDevID);
        return blResult;
    }
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("devid", strDevIDOut));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::DeleteDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));
    
    LOG_INFO_RLD("Delete device info received and  user id is " << strUserID << " and devcie id is " << strDevID 
        << " and session id is " << strSid);

    if (!DeleteDevice(strSid, strUserID, strDevID))
    {
        LOG_ERROR_RLD("Delete device handle failed and user id is " << strUserID << " and sid is " << strSid << " and device id is " << strDevID);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::ModifyDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    itFind = pMsgInfoMap->find("devname");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device name not found.");
        return blResult;
    }
    const std::string strDevName = itFind->second;

    std::string strDevPwd;
    itFind = pMsgInfoMap->find("devpwd");
    if (pMsgInfoMap->end() != itFind)
    {
        strDevPwd = itFind->second;
    }

    std::string strDevType;
    itFind = pMsgInfoMap->find("devtype");
    if (pMsgInfoMap->end() != itFind)
    {
        strDevType = itFind->second;
    }
    
    std::string strDevExtend;
    itFind = pMsgInfoMap->find("devextend");
    if (pMsgInfoMap->end() != itFind)
    {
        strDevExtend = itFind->second;
    }

    std::string strDevInnerInfo;
    itFind = pMsgInfoMap->find("devinnerinfo");
    if (pMsgInfoMap->end() != itFind)
    {
        strDevInnerInfo = itFind->second;
    }

    std::string strP2pid;
    itFind = pMsgInfoMap->find("p2pid");
    if (pMsgInfoMap->end() != itFind)
    {
        strP2pid = itFind->second;
    }

    std::string strDomainname;
    itFind = pMsgInfoMap->find("domainname");
    if (pMsgInfoMap->end() != itFind)
    {
        strDomainname = itFind->second;
    }

    std::string strIpaddress;
    itFind = pMsgInfoMap->find("ipaddress");
    if (pMsgInfoMap->end() != itFind)
    {
        strIpaddress = itFind->second;
    }

    unsigned int uiDevShared = 0; //是否是被分享的设备，0，否；1，是，默认值为0
    itFind = pMsgInfoMap->find("dev_shared");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiDevShared = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Modify device info of device shared is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Modify device info of device shared is invalid and input is " << itFind->second);
            return blResult;
        }
    }
    
    DeviceIf devif;
    devif.m_strDevExtend = strDevExtend;
    devif.m_strDevID = strDevID;
    devif.m_strDevInnerInfo = strDevInnerInfo;
    devif.m_strDevName = strDevName;
    devif.m_strDevPwd = strDevPwd;
    devif.m_strDevType = strDevType;
    devif.m_strDomainname = strDomainname;
    devif.m_strIpaddress = strIpaddress;
    devif.m_strP2pid = strP2pid;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    if (!ModifyDevice(strSid, strUserID, devif, uiDevShared))
    {
        LOG_ERROR_RLD("Modify device handle failed and user id is " << strUserID << " and sid is " << strSid << " and device id is " << strDevID);
        return blResult;
    }

    LOG_INFO_RLD("Modify device info received and  user id is " << strUserID << " and devcie id is " << strDevID << " and device name is " << strDevName
        << " and device pwd is [" << strDevPwd << "]" << " and device type is " << strDevType << " and device extend is [" << strDevExtend << "]"
        << " and device inner info is [" << strDevInnerInfo << "]" << " and device domain name is " << strDomainname << " and p2p id is " << strP2pid
        << " and session id is " << strSid << " and device shared is " << uiDevShared);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::QueryDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    std::string strUserID;
    unsigned int uiDevShared = 0; //是否是被分享的设备，0，否；1，是，默认值为0
    itFind = pMsgInfoMap->find("dev_shared");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiDevShared = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Query device info of device shared is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Query device info of device shared is invalid and input is " << itFind->second);
            return blResult;
        }

        if (uiDevShared)
        {
            itFind = pMsgInfoMap->find("userid");
            if (pMsgInfoMap->end() == itFind)
            {
                LOG_ERROR_RLD("User id not found.");
                return blResult;
            }
            strUserID = itFind->second;
        }
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query device info received and  devcie id is " << strDevID << " and device shared is " << uiDevShared << " and user id is "
        << strUserID << " and session id is " << strSid);

    std::string strUpdateDate;
    std::string strVersion;
    std::string strOnline;
    InteractiveProtoHandler::Device devinfo;
    if (!QueryDeviceInfo<InteractiveProtoHandler::Device>(strSid, strDevID, devinfo, strUpdateDate, strVersion, strOnline, uiDevShared, strUserID))
    {
        LOG_ERROR_RLD("Query device info handle failed and device id is " << strDevID << " and sid is " << strSid);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("devid", devinfo.m_strDevID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("devname", devinfo.m_strDevName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("devpwd", devinfo.m_strDevPassword));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("devtype", boost::lexical_cast<std::string>(devinfo.m_uiTypeInfo)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("devextend", devinfo.m_strExtend));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("devinnerinfo", devinfo.m_strInnerinfo));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("createdate", devinfo.m_strCreatedate));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("status", boost::lexical_cast<std::string>(devinfo.m_uiStatus)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("domainname", devinfo.m_strDomainName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("p2pid", devinfo.m_strP2pID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("updatedate", strUpdateDate));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("version", strVersion));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("online", strOnline));
        
    blResult = true;

    return blResult;
}

bool HttpMsgHandler::QueryDevicesOfUserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsRelationList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsRelationList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["data"] = jsRelationList;

            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }        
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    unsigned int uiBeginIndex = 0;
    itFind = pMsgInfoMap->find("beginindex");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiBeginIndex = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Query devices of user info is invalid and error msg is " << e.what() << " and input index is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Query devices of user info is invalid and input index is " << itFind->second);
            return blResult;
        }
    }
                  
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query devices of user info received and  user id is " << strUserID
        << " and begin index is " << uiBeginIndex
        << " and session id is " << strSid);

    std::list<InteractiveProtoHandler::Relation> relist;
    std::list<std::string> strDevNameList;
    if (!QueryDevicesOfUser<InteractiveProtoHandler::Relation>(strSid, strUserID, uiBeginIndex, relist, strDevNameList))
    {
        LOG_ERROR_RLD("Query devices of user handle failed and user id is " << strUserID << " and session id is " << strSid);
        return blResult;
    }
    

    if (relist.size() != strDevNameList.size())
    {
        LOG_ERROR_RLD("Query devices of user handle failed and relation list size is " << relist.size() << " and device name list size is " << strDevNameList.size());
        return blResult;
    }

    auto itBegin2 = strDevNameList.begin();
    auto itEnd2 = strDevNameList.end();
    auto itBegin = relist.begin();
    auto itEnd = relist.end();
    while (itBegin != itEnd && itBegin2 != itEnd2)
    {
        Json::Value jsRelation;
        jsRelation["userid"] = itBegin->m_strUserID;
        jsRelation["devid"] = itBegin->m_strDevID;
        jsRelation["relation"] = itBegin->m_uiRelation;
        jsRelation["begindate"] = itBegin->m_strBeginDate;
        jsRelation["enddate"] = itBegin->m_strEndDate;
        jsRelation["extend"] = itBegin->m_strValue;
        jsRelation["devname"] = *itBegin2;

        jsRelationList.append(jsRelation);

        ++itBegin;
        ++itBegin2;
    }
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    
    blResult = true;

    return blResult;
}

bool HttpMsgHandler::QueryUsersOfDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsRelationList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsRelationList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["data"] = jsRelationList;

            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    unsigned int uiBeginIndex = 0;
    itFind = pMsgInfoMap->find("beginindex");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiBeginIndex = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Query users of device info is invalid and error msg is " << e.what() << " and input index is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Query users of device info is invalid and input index is " << itFind->second);
            return blResult;
        }
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));
       
    LOG_INFO_RLD("Query users of device info received and  user id is " << strDevID
        << " and begin index is " << uiBeginIndex
        << " and session id is " << strSid);

    std::list<InteractiveProtoHandler::Relation> relist;
    std::list<std::string> strUsrNameList;
    if (!QueryUsersOfDevice<InteractiveProtoHandler::Relation>(strSid, strDevID, uiBeginIndex, relist, strUsrNameList))
    {
        LOG_ERROR_RLD("Query users of device handle failed and device id is " << strDevID << " and session id is " << strSid);
        return blResult;
    }

    if (relist.size() != strUsrNameList.size())
    {
        LOG_ERROR_RLD("Query users of device handle failed and relation list size is " << relist.size() << " and user name list size is " << strUsrNameList.size());
        return blResult;
    }

    auto itBegin2 = strUsrNameList.begin();
    auto itEnd2 = strUsrNameList.end();
    auto itBegin = relist.begin();
    auto itEnd = relist.end();
    while (itBegin != itEnd && itBegin2 != itEnd2)
    {
        Json::Value jsRelation;
        jsRelation["userid"] = itBegin->m_strUserID;
        jsRelation["devid"] = itBegin->m_strDevID;
        jsRelation["relation"] = itBegin->m_uiRelation;
        jsRelation["begindate"] = itBegin->m_strBeginDate;
        jsRelation["enddate"] = itBegin->m_strEndDate;
        jsRelation["extend"] = itBegin->m_strValue;
        jsRelation["username"] = *itBegin2;

        jsRelationList.append(jsRelation);

        ++itBegin;
        ++itBegin2;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    
    blResult = true;

    return blResult;
}

bool HttpMsgHandler::SharingDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("userid_shared");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id of shared not found.");
        return blResult;
    }
    const std::string strUserIDShared = itFind->second;

    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    ////
    //itFind = pMsgInfoMap->find("relation");
    //if (pMsgInfoMap->end() == itFind)
    //{
    //    LOG_ERROR_RLD("Relation not found.");
    //    return blResult;
    //}
    const std::string strRelation("1");

    itFind = pMsgInfoMap->find("begindate");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Begin date not found.");
        return blResult;
    }
    const std::string strBeginDate = itFind->second;

    itFind = pMsgInfoMap->find("enddate");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("End date not found.");
        return blResult;
    }
    const std::string strEndDate = itFind->second;
    
    std::string strValue;
    itFind = pMsgInfoMap->find("value");
    if (pMsgInfoMap->end() != itFind)
    {
        strValue = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Sharing device info received and  user id is " << strUserID << " and user id of shared is " << strUserIDShared 
        << " and devcie id is " << strDevID << " and relation is " << strRelation
        << " and begin date is [" << strBeginDate << "]" << " and end date is " << strEndDate << " and value is [" << strValue << "]"
        << " and session id is " << strSid);

    if (!SharingDevice(strSid, strUserIDShared, strDevID, strRelation, strBeginDate, strEndDate))
    {
        LOG_ERROR_RLD("Sharing device handle failed and device id is " << strDevID << " and user id of shared is " << strUserIDShared << " and session id is " << strSid);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::CancelSharedDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("userid_shared");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id of shared not found.");
        return blResult;
    }
    const std::string strUserIDShared = itFind->second;

    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    const std::string strRelation("1");

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Cancel shared device info received and  user id is " << strUserID << " and user id of shared is " << strUserIDShared <<
        " and devcie id is " << strDevID << " and session id is " << strSid);

    if (!CancelSharedDevice(strSid, strUserIDShared, strDevID, strRelation))
    {
        LOG_ERROR_RLD("Canecl shared device handle failed and device id is " << strDevID << " and user id of shared is " << strUserIDShared << " and session id is " << strSid);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::AddFriendsHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("friendid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Friend id not found.");
        return blResult;
    }
    const std::string strFriendID = itFind->second;
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Add friend info received and  user id is " << strUserID << " and friend id is " << strFriendID 
        << " and session id is " << strSid);

    if (!AddFriends(strSid, strUserID, strFriendID))
    {
        LOG_ERROR_RLD("Add friend handle failed and user id is " << strUserID << " and friend is " << strFriendID << " and session id is " << strSid);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::DeleteFriendsHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("friendid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Friend id not found.");
        return blResult;
    }
    const std::string strFriendID = itFind->second;
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete friend info received and  user id is " << strUserID << " and friend id is " << strFriendID
        << " and session id is " << strSid);

    if (!DeleteFriends(strSid, strUserID, strFriendID))
    {
        LOG_ERROR_RLD("Delete friend handle failed and user id is " << strUserID << " and friend is " << strFriendID << " and session id is " << strSid);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::QueryFriendHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsFriendIDList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsFriendIDList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["data"] = jsFriendIDList;

            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    unsigned int uiBeginIndex = 0;
    itFind = pMsgInfoMap->find("beginindex");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiBeginIndex = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Query friend info is invalid and error msg is " << e.what() << " and input index is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Query friend info is invalid and input index is " << itFind->second);
            return blResult;
        }
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query user friend info received and  user id is " << strUserID
        << " and begin index is " << uiBeginIndex
        << " and session id is " << strSid);

    std::list<std::string> FriendsList;
    if (!QueryFriends(strSid, strUserID, uiBeginIndex, FriendsList))
    {
        LOG_ERROR_RLD("Delete friend handle failed and user id is " << strUserID << " and begin index is " << uiBeginIndex << " and session id is " << strSid);
        return blResult;
    }
    
    auto itBegin = FriendsList.begin();
    auto itEnd = FriendsList.end();
    while (itBegin != itEnd)
    {
        Json::Value jsRelation;
        jsRelation["userid"] = *itBegin;
        
        jsFriendIDList.append(jsRelation);

        ++itBegin;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::P2pInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    itFind = pMsgInfoMap->find(FCGIManager::REMOTE_ADDR);
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User remote ip not found.");
        return blResult;
    }
    const std::string strRemoteIP = itFind->second;

    itFind = pMsgInfoMap->find("p2p_type");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User p2p type not found.");
        return blResult;
    }
    const std::string strP2pType = itFind->second;

    unsigned int uiP2pType = 0;

    try
    {
        uiP2pType = boost::lexical_cast<unsigned int>(strP2pType);
    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("Query p2p type of user is invalid and error msg is " << e.what() << " and input is " << itFind->second);
        return blResult;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Query p2p type of user is invalid and input is " << itFind->second);
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));
        
    LOG_INFO_RLD("User p2p info received and  session id is " << strSid << " and device id is " << strDevID
        << " and user id is " << strUserID << " and p2p type is " << uiP2pType <<
        " and user remote ip is " << strRemoteIP);

    std::string strP2pServer;
    std::string strP2pID;
    unsigned int uiLease = 0;
    std::string strLicenseKey;
    std::string strPushID;
    if (!P2pInfo(strSid, strUserID, strDevID, strRemoteIP, uiP2pType, strP2pServer, strP2pID, uiLease, strLicenseKey, strPushID))
    {
        LOG_ERROR_RLD("User p2p info handle failed and device id is " << strDevID << " and sid is " << strSid);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("p2pserver", strP2pServer));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("p2pid", strP2pID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("lease", boost::lexical_cast<std::string>(uiLease)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("license_key", strLicenseKey));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("push_id", strPushID));
    
    blResult = true;

    return blResult;

}

bool HttpMsgHandler::CheckDeviceP2pidHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("p2pid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("P2p id not found.");
        return blResult;
    }
    const std::string strP2pid = itFind->second;

    std::string strP2pType;
    unsigned int uiP2pType = 0xFFFFFFFF;
    itFind = pMsgInfoMap->find("p2p_type");
    if (pMsgInfoMap->end() != itFind)
    {
        strP2pType = itFind->second;
        try
        {
            uiP2pType = boost::lexical_cast<unsigned int>(strP2pType);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Check p2p id type of user is invalid and error msg is " << e.what() << " and input is " << strP2pType);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Check p2p id type of user is invalid and input is " << strP2pType);
            return blResult;
        }
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));
    
    LOG_INFO_RLD("Check p2p id info received and  p2p id is " << strP2pid << " and p2p type is " << uiP2pType);

    if (!CheckDeviceP2pid(strP2pid, uiP2pType))
    {
        LOG_ERROR_RLD("Check p2p id info handle failed and p2p id is " << strP2pid << " and p2p type is " << uiP2pType);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;


}

bool HttpMsgHandler::DeviceLoginHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsTimeZone;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsTimeZone)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["timezoneinfo"] = jsTimeZone;

            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    itFind = pMsgInfoMap->find("devpwd");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device pwd not found.");
        return blResult;
    }

    const std::string strDevPwd = itFind->second;

    itFind = pMsgInfoMap->find(FCGIManager::REMOTE_ADDR);
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device remote ip not found.");
        return blResult;
    }

    const std::string strRemoteIP = itFind->second;

    itFind = pMsgInfoMap->find("devtype");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device type not found.");
        return blResult;
    }
    const std::string strDevType = itFind->second;

    unsigned int uiDevType = 0;
    try
    {
        uiDevType = boost::lexical_cast<unsigned int>(strDevType);
    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("Type info is invalid and error msg is " << e.what() << " and input is " << itFind->second);
        return blResult;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Type info is invalid and input is " << itFind->second);
        return blResult;
    }

    unsigned int uiP2pType = 0xFFFFFFFF;
    itFind = pMsgInfoMap->find("p2ptype");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiP2pType = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("P2p type info is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("P2p type info is invalid and input is " << itFind->second);
            return blResult;
        }
    }

    std::string strP2pserver;
    itFind = pMsgInfoMap->find("p2pserver");
    if (pMsgInfoMap->end() != itFind)
    {
        strP2pserver = itFind->second;
    }

    std::string strP2pID;
    itFind = pMsgInfoMap->find("p2pid");
    if (pMsgInfoMap->end() != itFind)
    {
        strP2pID = itFind->second;
    }

    unsigned int uiP2pidBuildin = 0xFFFFFFFF;
    itFind = pMsgInfoMap->find("p2pid_buildin");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiP2pidBuildin = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("P2p id build in info is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("P2p id build in info is invalid and input is " << itFind->second);
            return blResult;
        }
    }

    std::string strUserName;
    itFind = pMsgInfoMap->find("user_name");
    if (pMsgInfoMap->end() != itFind)
    {
        strUserName = itFind->second;
    }

    std::string strUserPwd;
    itFind = pMsgInfoMap->find("user_password");
    if (pMsgInfoMap->end() != itFind)
    {
        strUserPwd = itFind->second;
    }

    std::string strDistributor;
    itFind = pMsgInfoMap->find("distributor");
    if (pMsgInfoMap->end() != itFind)
    {
        strDistributor = itFind->second;
    }

    std::string strOtherProperty;
    itFind = pMsgInfoMap->find("other_property");
    if (pMsgInfoMap->end() != itFind)
    {
        strOtherProperty = itFind->second;
    }

    std::string strDomainName;
    itFind = pMsgInfoMap->find("domain_name");
    if (pMsgInfoMap->end() != itFind)
    {
        strDomainName = itFind->second;
    }
        
    LOG_INFO_RLD("Device login info received and  device id is " << strDevID << " and device pwd is " << strDevPwd << 
        " and device ip is " << strRemoteIP << " and device type is " << uiDevType << " and device p2p type is " << uiP2pType <<
        " and device p2p server is " << strP2pserver << " and device p2p id is " << strP2pID << " and device p2p id build in is " << uiP2pidBuildin <<
        " and device user name is " << strUserName << " and device user pwd is " << strUserPwd << " and distributor is " << strDistributor << 
        " and device other property is " << strOtherProperty << " and device domain name is " << strDomainName);

    DeviceLoginInfo devlogininfo;
    devlogininfo.m_strDevID = strDevID;
    devlogininfo.m_strDevIpAddress = strRemoteIP;
    devlogininfo.m_strDevPwd = strDevPwd;
    devlogininfo.m_strDistributor = strDistributor;
    devlogininfo.m_strOtherProperty = strOtherProperty;
    devlogininfo.m_strP2pID = strP2pID;
    devlogininfo.m_strP2pserver = strP2pserver;
    devlogininfo.m_strUserName = strUserName;
    devlogininfo.m_strUserPwd = strUserPwd;
    devlogininfo.m_uiDevType = uiDevType;
    devlogininfo.m_uiP2pidBuildin = uiP2pidBuildin;
    devlogininfo.m_uiP2pType = uiP2pType;
    devlogininfo.m_strDomainName = strDomainName;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    std::string strValue;
    std::string strSid;
    if (!DeviceLogin(devlogininfo, strSid, strValue))
    {
        LOG_ERROR_RLD("Device login handle failed and device id is " << strDevID << " and sid is " << strSid);
        return blResult;
    }

    //Json::Value root;
    Json::Reader reader;
    if (!reader.parse(strValue, jsTimeZone, false))
    {
        LOG_ERROR_RLD("Reader Parse failed, strValue:" << strValue);
        return false;
    }

    if (!jsTimeZone.isObject())
    {
        LOG_ERROR_RLD("Reader Parse failed, strValue:" << strValue);
        return false;
    }

    Json::Value jsCountrycode = jsTimeZone["countrycode"];
    if (jsCountrycode.isNull())
    {
        LOG_ERROR_RLD("Country code json value is null");
        return false;
    }

    Json::Value jsCountrynameEn = jsTimeZone["countryname_en"];
    if (jsCountrynameEn.isNull())
    {
        LOG_ERROR_RLD("Country name_en json value is null");
        return false;
    }

    Json::Value jsCountrynameZh = jsTimeZone["countryname_zh"];
    if (jsCountrynameZh.isNull())
    {
        LOG_ERROR_RLD("Country name_zh json value is null");
        return false;
    }

    Json::Value jsTimezone = jsTimeZone["timezone"];
    if (jsTimezone.isNull())
    {
        LOG_ERROR_RLD("Time zone json value is null");
        return false;
    }
        
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("sid", strSid));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::DeviceP2pInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    std::string strSid;
    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() != itFind)
    {
        strSid = itFind->second;
    }
    
    unsigned int uiCount = 0;
    std::string strDevID;
    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_INFO_RLD("Device id not found.");
        //return blResult;
    }
    else
    {
        ++uiCount;
        strDevID = itFind->second;
    }

    itFind = pMsgInfoMap->find(FCGIManager::REMOTE_ADDR);
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device remote ip not found.");
        return blResult;
    }
    const std::string strRemoteIP = itFind->second;

    std::string strP2pType;
    unsigned int uiP2pType = 0xFFFFFFFF;
    itFind = pMsgInfoMap->find("p2p_type");
    if (pMsgInfoMap->end() != itFind)
    {
        strP2pType = itFind->second;
        try
        {
            uiP2pType = boost::lexical_cast<unsigned int>(strP2pType);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Query p2p type of device is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Query p2p type of device is invalid and input is " << itFind->second);
            return blResult;
        }
    }

    std::string strDomainName;
    itFind = pMsgInfoMap->find("domainname");
    if (pMsgInfoMap->end() == itFind)
    {
        if (0 == uiCount)
        {
            LOG_ERROR_RLD("Device id and domainname both not found.");
            return blResult;
        }
    }
    else
    {
        strDomainName = itFind->second;
    }

    LOG_INFO_RLD("Device p2p info received and  session id is " << strSid << " and device id is " << strDevID << " and device remote ip is " << strRemoteIP
        << " and p2p type is " << uiP2pType << " and domainname is " << strDomainName);

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    std::string strP2pServer;
    std::string strP2pID;
    unsigned int uiLease = 0;
    std::string strLicenseKey;
    std::string strPushID;
    if (!DeviceP2pInfo(strSid, strDevID, strRemoteIP, uiP2pType, strDomainName, strP2pServer, strP2pID, uiLease, strLicenseKey, strPushID))
    {
        LOG_ERROR_RLD("Device p2p info handle failed and device id is " << strDevID << " and sid is " << strSid);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("p2pserver", strP2pServer));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("p2pid", strP2pID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("lease", boost::lexical_cast<std::string>(uiLease)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("license_key", strLicenseKey));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("push_id", strPushID));
        
    blResult = true;

    return blResult;
}

bool HttpMsgHandler::DeviceShakehandHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }

    const std::string strDevID = itFind->second;

    LOG_INFO_RLD("Shakehand info received and  device id is " << strDevID << " and session id is " << strSid);

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    if (!DeviceShakehand(strSid, strDevID))
    {
        LOG_ERROR_RLD("Shakehand handle failed and device id is " << strDevID << " and sid is " << strSid);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;

}

bool HttpMsgHandler::DeviceLogoutHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Logout device info received and  device id is " << strDevID << " and session id is " << strSid);

    if (!DeviceLogout(strSid, strDevID))
    {
        LOG_ERROR_RLD("Logout device handle failed and device id is " << strDevID << " and sid is " << strSid);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::DeviceSetPropertyHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    std::string strDomainName;
    itFind = pMsgInfoMap->find("domainname");
    if (pMsgInfoMap->end() != itFind)
    {
        strDomainName = itFind->second;
    }
    
    std::string strDevType = "0"; //默认值为门铃
    itFind = pMsgInfoMap->find("devtype");
    if (pMsgInfoMap->end() != itFind)
    {
        strDevType = itFind->second;
    }

    unsigned int uiDevType = 0; //默认为门铃类型。

    try
    {
        uiDevType = boost::lexical_cast<unsigned int>(strDevType);
    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("Device type is invalid and error msg is " << e.what() << " and input is " << itFind->second);
        return blResult;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Device type is is invalid and input is " << itFind->second);
        return blResult;
    }
        
    std::string strCorpid;
    itFind = pMsgInfoMap->find("corpid");
    if (pMsgInfoMap->end() != itFind)
    {
        strCorpid = itFind->second;
    }

    std::string strDvsname;
    itFind = pMsgInfoMap->find("dvsname");
    if (pMsgInfoMap->end() != itFind)
    {
        strDvsname = itFind->second;
    }

    std::string strDvsIp;
    itFind = pMsgInfoMap->find("dvsip");
    if (pMsgInfoMap->end() != itFind)
    {
        strDvsIp = itFind->second;
    }

    std::string strWebport;
    itFind = pMsgInfoMap->find("webport");
    if (pMsgInfoMap->end() != itFind)
    {
        strWebport = itFind->second;
    }

    std::string strCtrlport;
    itFind = pMsgInfoMap->find("ctrlport");
    if (pMsgInfoMap->end() != itFind)
    {
        strCtrlport = itFind->second;
    }

    std::string strProtocol;
    itFind = pMsgInfoMap->find("protocol");
    if (pMsgInfoMap->end() != itFind)
    {
        strProtocol = itFind->second;
    }

    std::string strUserid;
    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() != itFind)
    {
        strUserid = itFind->second;
    }

    std::string strPassword;
    itFind = pMsgInfoMap->find("password");
    if (pMsgInfoMap->end() != itFind)
    {
        strPassword = itFind->second;
    }

    std::string strModel;
    itFind = pMsgInfoMap->find("model");
    if (pMsgInfoMap->end() != itFind)
    {
        strModel = itFind->second;
    }

    std::string strPostfrequency;
    itFind = pMsgInfoMap->find("postfrequency");
    if (pMsgInfoMap->end() != itFind)
    {
        strPostfrequency = itFind->second;
    }

    std::string strVersion;
    itFind = pMsgInfoMap->find("version");
    if (pMsgInfoMap->end() != itFind)
    {
        strVersion = itFind->second;
    }

    std::string strStatus;
    itFind = pMsgInfoMap->find("status");
    if (pMsgInfoMap->end() != itFind)
    {
        strStatus = itFind->second;
    }

    std::string strServerip;
    itFind = pMsgInfoMap->find("serverip");
    if (pMsgInfoMap->end() != itFind)
    {
        strServerip = itFind->second;
    }

    std::string strServerport;
    itFind = pMsgInfoMap->find("serverport");
    if (pMsgInfoMap->end() != itFind)
    {
        strServerport = itFind->second;
    }

    std::string strTransfer;
    itFind = pMsgInfoMap->find("transfer");
    if (pMsgInfoMap->end() != itFind)
    {
        strTransfer = itFind->second;
    }

    std::string strMobileport;
    itFind = pMsgInfoMap->find("mobileport");
    if (pMsgInfoMap->end() != itFind)
    {
        strMobileport = itFind->second;
    }

    std::string strChannelcount;
    itFind = pMsgInfoMap->find("channelcount");
    if (pMsgInfoMap->end() != itFind)
    {
        strChannelcount = itFind->second;
    }

    std::string strP2pid;
    itFind = pMsgInfoMap->find("p2pid");
    if (pMsgInfoMap->end() != itFind)
    {
        strP2pid = itFind->second;
    }

    std::string strDvsip2;
    itFind = pMsgInfoMap->find("dvsip2");
    if (pMsgInfoMap->end() != itFind)
    {
        strDvsip2 = itFind->second;
    }

    std::string strDoorbellName;
    itFind = pMsgInfoMap->find("doorbell_name");
    if (pMsgInfoMap->end() != itFind)
    {
        strDoorbellName = itFind->second;
    }

    std::string strSerialNum;
    itFind = pMsgInfoMap->find("serial_number");
    if (pMsgInfoMap->end() != itFind)
    {
        strSerialNum = itFind->second;
    }

    std::string strDoorbellP2pid;
    itFind = pMsgInfoMap->find("doorbell_p2pid");
    if (pMsgInfoMap->end() != itFind)
    {
        strDoorbellP2pid = itFind->second;
    }

    std::string strBatteryCap;
    itFind = pMsgInfoMap->find("battery_capacity");
    if (pMsgInfoMap->end() != itFind)
    {
        strBatteryCap = itFind->second;
    }

    std::string strChargeState;
    itFind = pMsgInfoMap->find("charging_state");
    if (pMsgInfoMap->end() != itFind)
    {
        strChargeState = itFind->second;
    }

    std::string strWifiSig;
    itFind = pMsgInfoMap->find("wifi_signal");
    if (pMsgInfoMap->end() != itFind)
    {
        strWifiSig = itFind->second;
    }

    std::string strVolumeLevel;
    itFind = pMsgInfoMap->find("volume_level");
    if (pMsgInfoMap->end() != itFind)
    {
        strVolumeLevel = itFind->second;
    }

    std::string strSubCategory;
    std::string strVersionNum;
    bool blFlag = false;
    itFind = pMsgInfoMap->find("version_number");
    if (pMsgInfoMap->end() != itFind)
    {
        blFlag = true;

        strVersionNum = itFind->second;

        itFind = pMsgInfoMap->find("sub_category");
        if (pMsgInfoMap->end() == itFind)
        {
            LOG_ERROR_RLD("Device set property error because sub category not found when version number was already set and device id is " << strDevID
                << " and version number is " << strVersionNum);
            return blResult;
        }

        strSubCategory = itFind->second;
    }

    if (!blFlag)
    {
        itFind = pMsgInfoMap->find("sub_category");
        if (pMsgInfoMap->end() != itFind)
        {
            strSubCategory = itFind->second;
        }
    }
    
    std::string strChannelNum;
    itFind = pMsgInfoMap->find("channel_number");
    if (pMsgInfoMap->end() != itFind)
    {
        strChannelNum = itFind->second;
    }

    std::string strCodeType;
    itFind = pMsgInfoMap->find("coding_type");
    if (pMsgInfoMap->end() != itFind)
    {
        strCodeType = itFind->second;
    }

    std::string strPirAlarmSwitch;
    itFind = pMsgInfoMap->find("pir_alarm_swtich");
    if (pMsgInfoMap->end() != itFind)
    {
        strPirAlarmSwitch = itFind->second;
    }

    std::string strDoorbellSwitch;
    itFind = pMsgInfoMap->find("doorbell_switch");
    if (pMsgInfoMap->end() != itFind)
    {
        strDoorbellSwitch = itFind->second;
    }

    std::string strPirAlarmLevel;
    itFind = pMsgInfoMap->find("pir_alarm_level");
    if (pMsgInfoMap->end() != itFind)
    {
        strPirAlarmLevel = itFind->second;
    }

    std::string strPirInEffectiveTime;
    itFind = pMsgInfoMap->find("pir_ineffective_time");
    if (pMsgInfoMap->end() != itFind)
    {
        strPirInEffectiveTime = itFind->second;
    }

    std::string strCurrentWifi;
    itFind = pMsgInfoMap->find("current_wifi");
    if (pMsgInfoMap->end() != itFind)
    {
        strCurrentWifi = itFind->second;
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Set device property info received and  device id is " << strDevID << " and device type is " << uiDevType << 
        " and domain name id is " << strDomainName <<
        " and corpid is " << strCorpid << " and dvsname is " << strDvsname << " and dvsip is " << strDvsIp << " and webport is " << strWebport <<
        " and ctrlport is " << strCtrlport << " and protocol is " << strProtocol << " and userid is " << strUserid << " and password is " << strPassword <<
        " and model is " << strModel << " and postfrequency is " << strPostfrequency << " and version " << strVersion << " and status is " << strStatus <<
        " and serverip is " << strServerip << " and serverport is " << strServerport << " and transfer" << strTransfer << " and mobileport" << strMobileport <<
        " and channelcount is " << strChannelcount << " and p2pid is " << strP2pid << " and dvsip2 is " << strDvsip2 <<
        " and doorbell name is " << strDoorbellName << " and doorbell serial num is " << strSerialNum << " and doorbell p2pid is " << strDoorbellP2pid <<
        " and battery cap is " << strBatteryCap << " and charge state is " << strChargeState << " and wifi signal is " << strWifiSig <<
        " and volume level is " << strVolumeLevel << " and version number is " << strVersionNum << " and channel num is " << strChannelNum <<
        " and coding type is " << strCodeType << " and pir alarm switch is " << strPirAlarmSwitch << " and doorbell switch is " << strDoorbellSwitch <<
        " and pir alarm level is " << strPirAlarmLevel << " and pir ineffective time is " << strPirInEffectiveTime << " and currenct wifi is " << strCurrentWifi <<
        " and sub category is " << strSubCategory);

    DeviceProperty devpt;
    devpt.m_strChannelCount = strChannelcount;
    devpt.m_strCorpid = strCorpid;
    devpt.m_strCtrlport = strCtrlport;
    devpt.m_strDevid = strDevID;
    devpt.m_uiDevType = uiDevType;
    devpt.m_strDomainName = strDomainName;
    devpt.m_strDvsip = strDvsIp;
    devpt.m_strDvsname = strDvsname;
    devpt.m_strMobilePort = strMobileport;
    devpt.m_strModel = strModel;
    devpt.m_strPassword = strPassword;
    devpt.m_strPostfrequency = strPostfrequency;
    devpt.m_strProtocol = strProtocol;
    devpt.m_strServerIp = strServerip;
    devpt.m_strServerPort = strServerport;
    devpt.m_strStatus = strStatus;
    devpt.m_strTransfer = strTransfer;
    devpt.m_strUserid = strUserid;
    devpt.m_strVersion = strVersion;
    devpt.m_strWebport = strWebport;
    devpt.m_strP2pid = strP2pid;
    devpt.m_strDvsip2 = strDvsip2;
    devpt.m_strDoorbellName = strDoorbellName;
    devpt.m_strSerialNum = strSerialNum;
    devpt.m_strDoorbellP2pid = strDoorbellP2pid;
    devpt.m_strBatteryCap = strBatteryCap;
    devpt.m_strChargingState = strChargeState;
    devpt.m_strWifiSig = strWifiSig;
    devpt.m_strVolumeLevel = strVolumeLevel;
    devpt.m_strVersionNum = strVersionNum;
    devpt.m_strChannelNum = strChannelNum;
    devpt.m_strCodingType = strCodeType;
    devpt.m_strPirAlarmSwitch = strPirAlarmSwitch;
    devpt.m_strDoorbellSwitch = strDoorbellSwitch;
    devpt.m_strPirAlarmLevel = strPirAlarmLevel;
    devpt.m_strPirIneffectiveTime = strPirInEffectiveTime;
    devpt.m_strCurrentWifi = strCurrentWifi;
    devpt.m_strSubCategory = strSubCategory;

    if (!DeviceSetProperty(strSid, devpt))
    {
        LOG_ERROR_RLD("Set device property handle failed and device id is " << strDevID << " and sid is " << strSid);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::QueryUserFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsFileList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsFileList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["data"] = jsFileList;

            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }

    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");   
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    std::string strDevID;
    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() != itFind)
    {
        strDevID = itFind->second;
    }

    unsigned int uiBeginIndex = 0;    
    itFind = pMsgInfoMap->find("beginindex");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiBeginIndex = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Query file begin index info of user is invalid and error msg is " << e.what() << " and input index is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Query file begin index info of user is invalid and input index is " << itFind->second);
            return blResult;
        }        
    }

    std::string strBeginDate;
    itFind = pMsgInfoMap->find("begindate");
    if (pMsgInfoMap->end() != itFind)
    {
        strBeginDate = itFind->second;

        if (!ValidDatetime(strBeginDate))
        {
            LOG_ERROR_RLD("File begin date is invalid and input date is " << strBeginDate);
            return blResult;
        }
    }

    std::string strEndDate;
    itFind = pMsgInfoMap->find("enddate");
    if (pMsgInfoMap->end() != itFind)
    {
        strEndDate = itFind->second;

        if (!ValidDatetime(strEndDate))
        {
            LOG_ERROR_RLD("File end date is invalid and input date is " << strEndDate);
            return blResult;
        }
    }

    unsigned int uiBussinessType = 0xFFFFFFFF;
    itFind = pMsgInfoMap->find("businesstype");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiBussinessType = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Query file business type info of user is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Query file business type info of user is invalid and input is " << itFind->second);
            return blResult;
        }
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query user file info received and  session id is " << strSid << " and user id is " << strUserID << " and device id is " << strDevID
        << " and begin index is " << uiBeginIndex << " and begin date is " << strBeginDate 
        << " and end date is " << strEndDate  << " and business type is " << uiBussinessType);
    
    std::list<InteractiveProtoHandler::File> filelist;
    if (!QueryUserFile<InteractiveProtoHandler::File>(strSid, strUserID, strDevID, uiBeginIndex, strBeginDate, strEndDate, uiBussinessType, filelist))
    {
        LOG_ERROR_RLD("Query user file failed and user id is " << strUserID << " and device id is " << strDevID);
        return blResult;
    }

    auto itBegin = filelist.begin();
    auto itEnd = filelist.end();
    while (itBegin != itEnd)
    {
        Json::Value jsFile;
        jsFile["fileid"] = itBegin->m_strFileID;
        jsFile["name"] = itBegin->m_strFileName;
        jsFile["size"] = boost::lexical_cast<std::string>(itBegin->m_ulFileSize);
        jsFile["type"] = itBegin->m_strSuffixName;
        jsFile["businesstype"] = itBegin->m_uiBusinessType;
        jsFile["createdate"] = itBegin->m_strFileCreatedate;
        jsFile["url"] = itBegin->m_strDownloadUrl;

        jsFileList.append(jsFile);

        ++itBegin;
    }
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::DownloadUserFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("fileid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("File id not found.");
        return blResult;
    }
    const std::string strFileID = itFind->second;
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Download file info received and  user id is " << strUserID << " and session id is " << strSid <<
        " and file id is " << strFileID);

    std::string strFileUrl;
    if (!DownloadUserFile(strSid, strUserID, strFileID, strFileUrl))
    {
        LOG_ERROR_RLD("Download file info failed and user id is " << strUserID << " and sid is " << strSid <<
            " and file id is " << strFileID);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("address", strFileUrl));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;

}

bool HttpMsgHandler::DeleteUserFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("fileid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("File id not found.");
        return blResult;
    }
    const std::string strFileID = itFind->second;

    LOG_INFO_RLD("Delete file info received and  user id is " << strUserID << " and session id is " << strSid <<
        " and file id is " << strFileID);

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    if (!DeleteUserFile(strSid, strUserID, strFileID))
    {
        LOG_ERROR_RLD("Delete file info failed and user id is " << strUserID << " and sid is " << strSid <<
            " and file id is " << strFileID);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::ParseMsgOfCompact(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter wr)
{
    auto itFind = pMsgInfoMap->find("compact_msg");
    if (pMsgInfoMap->end() != itFind)
    {
        const std::string &strValue = itFind->second;

        LOG_INFO_RLD("Compact msg is " << strValue);
        
        Json::Reader reader;
        Json::Value root;
        if (!reader.parse(strValue, root, false))
        {
            LOG_ERROR_RLD("Compact msg parse failed beacuse value parsed failed and value is " << strValue);
            return false;
        }
        
        if (!root.isObject())
        {
            LOG_ERROR_RLD("Compact msg parse failed beacuse json root parsed failed and value is " << strValue);
            return false;
        }

        Json::Value::Members members = root.getMemberNames();
        auto itBegin = members.begin();
        auto itEnd = members.end();
        while (itBegin != itEnd)
        {
            if (root[*itBegin].type() == Json::stringValue) //目前只支持string类型json字段
            {
                pMsgInfoMap->insert(MsgInfoMap::value_type(*itBegin, root[*itBegin].asString()));
            }

            ++itBegin;
        }

        pMsgInfoMap->erase(itFind);         

    }

    return true;
}

bool HttpMsgHandler::AddDeviceFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("id");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    itFind = pMsgInfoMap->find("fileid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("File id not found.");
        return blResult;
    }
    const std::string strFileID = itFind->second;

    itFind = pMsgInfoMap->find("url");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("File url not found.");
        return blResult;
    }
    const std::string strFileUrl = itFind->second;

    itFind = pMsgInfoMap->find("name");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("File name not found.");
        return blResult;
    }
    const std::string strFileName = itFind->second;

    std::string strSuffixName;
    itFind = pMsgInfoMap->find("suffixname");
    if (pMsgInfoMap->end() != itFind)
    {
        strSuffixName = itFind->second;
    }

    itFind = pMsgInfoMap->find("size");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("File size not found.");
        return blResult;
    }
    const std::string strFileSize = itFind->second;

    unsigned long int ulFileSize = 0;

    try
    {
        ulFileSize = boost::lexical_cast<unsigned long int>(strFileSize);
    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("Add device file size info is invalid and error msg is " << e.what() << " and input size is " << strFileSize);
        return blResult;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Add device file size info is invalid" << " and input size is " << strFileSize);
        return blResult;
    }

    itFind = pMsgInfoMap->find("createdate");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("File create date not found.");
        return blResult;
    }
    const std::string strCreateDate = itFind->second;

    boost::regex reg0("([0-9]{4}[0-9]{2}[0-9]{2}[0-9]{2}[0-9]{2}[0-9]{2})"); //yyyyMMddHHmmss
    boost::regex reg1("([0-9]{4}[0-9]{2}[0-9]{2})"); //yyyyMMdd
    boost::regex reg2("([0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2})"); //yyyy-MM-dd HH:mm:ss
    boost::regex reg3("([0-9]{4}-[0-9]{2}-[0-9]{2})"); ////yyyy-MM-dd
    boost::regex reg4("([0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2})"); //yyyy-MM-dd  HH:mm

    if (!boost::regex_match(strCreateDate, reg0) && !boost::regex_match(strCreateDate, reg1) && !boost::regex_match(strCreateDate, reg2) &&
        !boost::regex_match(strCreateDate, reg3) && !boost::regex_match(strCreateDate, reg4))
    {
        LOG_ERROR_RLD("File create date is invalid and input date is " << strCreateDate);
        return blResult;
    }

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    itFind = pMsgInfoMap->find("businesstype");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("File businesstype not found.");
        return blResult;
    }

    unsigned int uiBussinessType = 0xFFFFFFFF;

    try
    {
        uiBussinessType = boost::lexical_cast<unsigned int>(itFind->second);
    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("Add device file business type info of user is invalid and error msg is " << e.what() << " and input is " << itFind->second);
        return blResult;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Add device file business type info of user is invalid and input is " << itFind->second);
        return blResult;
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Add device file info received and  device id is " << strDevID << " and file id is " << strFileID <<
        " and url is " << strFileUrl << " and file name is " << strFileName << " and suffix name is " << strSuffixName <<
        " and file size is " << strFileSize << " and create date is " << strCreateDate <<
        " and extend is " << strExtend << " and bussiness type is " << uiBussinessType);
    
    if (!AddDeviceFile(strDevID, strFileID, strFileUrl, strFileName, strSuffixName, ulFileSize, strCreateDate, strExtend, uiBussinessType))
    {
        LOG_ERROR_RLD("Add device file info failed and device id is " << strDevID << " and file id is " << strFileID);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::AddUserFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

        //ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    //目前未实现，暂时返回为真
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return true;
}

bool HttpMsgHandler::AddFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("type");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Type not found.");
        return blResult;
    }
    const std::string strType = itFind->second;

    unsigned int uiTypeInfo = 0; //0：用户，1：设备

    try
    {
        uiTypeInfo = boost::lexical_cast<unsigned int>(strType);
    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("Add file type info is invalid and error msg is " << e.what() << " and input type is " << strType);
        return blResult;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Add file type info is invalid" << " and input type is " << strType);
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    switch (uiTypeInfo)
    {
    case 0: //用户
        blResult = true;
        return AddUserFileHandler(pMsgInfoMap, writer);
    case 1: //设备
        blResult = true;
        return AddDeviceFileHandler(pMsgInfoMap, writer);
    default:
        LOG_ERROR_RLD("Unknown file type and type is " << uiTypeInfo);
        break;
    }

    return blResult;    
}

bool HttpMsgHandler::DeviceQueryTimeZoneHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    itFind = pMsgInfoMap->find(FCGIManager::REMOTE_ADDR);
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device remote ip not found.");
        return blResult;
    }
    const std::string strRemoteIP = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Device query timezone info received and  session id is " << strSid << " and device id is " << strDevID << " and device remote ip is " << strRemoteIP);

    std::string strCountrycode;
    std::string strCountryNameEn;
    std::string strCountryNameZh;
    std::string strTimeZone;
    if (!DeviceQueryTimeZone(strSid, strDevID, strRemoteIP, strCountrycode, strCountryNameEn, strCountryNameZh, strTimeZone))
    {
        LOG_ERROR_RLD("Device p2p info handle failed and device id is " << strDevID << " and sid is " << strSid);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("countrycode", strCountrycode));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("ountryname_en", strCountryNameEn));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("ountryname_zh", strCountryNameZh));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("timezone", strTimeZone));


    blResult = true;

    return blResult;


}

bool HttpMsgHandler::UserQueryAccessDomainNameHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    std::string strUserName;
    auto itFind = pMsgInfoMap->find("username");
    if (pMsgInfoMap->end() != itFind)
    {
        strUserName = itFind->second;
    }

    itFind = pMsgInfoMap->find(FCGIManager::REMOTE_ADDR);
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User remote ip not found.");
        return blResult;
    }
    const std::string strRemoteIP = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("User query access domain name info received and user name is " << strUserName 
        << " and remote ip is " << strRemoteIP);

    std::string strAccessDomainName;
    std::string strLease;
    if (!UserQueryAccessDomainName(strRemoteIP, strUserName, strAccessDomainName, strLease))
    {
        LOG_ERROR_RLD("User query access domain name info handle failed and user name is " << strUserName << " and remote ip is " << strRemoteIP);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("domainname", strAccessDomainName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("lease", strLease));


    blResult = true;

    return blResult;
}

bool HttpMsgHandler::DeviceQueryAccessDomainNameHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;        
    }
    const std::string strDevID = itFind->second;

    itFind = pMsgInfoMap->find(FCGIManager::REMOTE_ADDR);
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device remote ip not found.");
        return blResult;
    }
    const std::string strRemoteIP = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Device query access domain name info received and device id is " << strDevID
        << " and remote ip is " << strRemoteIP);

    std::string strAccessDomainName;
    std::string strLease;
    if (!DeviceQueryAccessDomainName(strRemoteIP, strDevID, strAccessDomainName, strLease))
    {
        LOG_ERROR_RLD("Device query access domain name info handle failed and device id is " << strDevID << " and remote ip is " << strRemoteIP);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("domainname", strAccessDomainName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("lease", strLease));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::DeviceQueryUpdateServiceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    itFind = pMsgInfoMap->find(FCGIManager::REMOTE_ADDR);
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device remote ip not found.");
        return blResult;
    }
    const std::string strRemoteIP = itFind->second;

    itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Device query update service address info received and device id is " << strDevID
        << " and remote ip is " << strRemoteIP << " and sid is " << strSid);

    std::string strUpdateServiceAddressName;
    std::string strLease;
    if (!DeviceQueryUpdateService(strSid, strRemoteIP, strDevID, strUpdateServiceAddressName, strLease))
    {
        LOG_ERROR_RLD("Device query update service address info handle failed and device id is " << strDevID << " and remote ip is " << strRemoteIP);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("update_address", strUpdateServiceAddressName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("lease", strLease));

    blResult = true;

    return blResult;


}

bool HttpMsgHandler::QueryUploadURLHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    LOG_INFO_RLD("Query upload url received.");

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    std::string strURL;
    if (!QueryUploadURL(strURL))
    {
        LOG_ERROR_RLD("Query upload url handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("upload_url", strURL));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::AddConfigurationHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("category");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Category not found.");
        return blResult;
    }
    const std::string strCategory = itFind->second;

    itFind = pMsgInfoMap->find("sub_category");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sub category not found.");
        return blResult;
    }
    const std::string strSubcategory = itFind->second;

    std::string strContent;
    itFind = pMsgInfoMap->find("content");
    if (pMsgInfoMap->end() != itFind)
    {
        strContent = itFind->second;
    }

    std::string strLatestVersion;
    itFind = pMsgInfoMap->find("latest_version");
    if (pMsgInfoMap->end() != itFind)
    {
        strLatestVersion = itFind->second;
    }

    std::string strDesc;
    itFind = pMsgInfoMap->find("description");
    if (pMsgInfoMap->end() != itFind)
    {
        strDesc = itFind->second;
    }

    std::string strForceVersion;
    itFind = pMsgInfoMap->find("force_version");
    if (pMsgInfoMap->end() != itFind)
    {
        strForceVersion = itFind->second;
    }

    std::string strServerAddress;
    itFind = pMsgInfoMap->find("server_address");
    if (pMsgInfoMap->end() != itFind)
    {
        strServerAddress = itFind->second;
    }

    std::string strFilename;
    itFind = pMsgInfoMap->find("file_name");
    if (pMsgInfoMap->end() != itFind)
    {
        strFilename = itFind->second;
    }

    std::string strFileID;
    itFind = pMsgInfoMap->find("file_id");
    if (pMsgInfoMap->end() != itFind)
    {
        strFileID = itFind->second;
    }

    unsigned int uiFileSize = 0;
    std::string strFileSize;
    itFind = pMsgInfoMap->find("file_size");
    if (pMsgInfoMap->end() != itFind)
    {
        strFileSize = itFind->second;
        
        try
        {
            uiFileSize = boost::lexical_cast<unsigned int>(strFileSize);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Value invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Value invalid and input is " << itFind->second);
            return blResult;
        }
    }

    unsigned int uiLease = 0;
    std::string strLease;
    itFind = pMsgInfoMap->find("lease");
    if (pMsgInfoMap->end() != itFind)
    {
        strLease = itFind->second;

        try
        {
            uiLease = boost::lexical_cast<unsigned int>(strLease);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Value invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Value invalid and input is " << itFind->second);
            return blResult;
        }
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Add configuration info received and category is " << strCategory << " and subcategory is " << strSubcategory
        << " and latest version is " << strLatestVersion << " and description is " << strDesc << " and force version " << strForceVersion
        << " and server address is " << strServerAddress << " and file name is " << strFilename << " and file id is " << strFileID 
        << " and file size is " << strFileSize << " and lease is " << strLease);

    if (!AddConfiguration(strCategory, strSubcategory, strLatestVersion, strDesc, strForceVersion, strServerAddress, strFilename, strFileID, uiFileSize, uiLease))
    {
        LOG_ERROR_RLD("Add configuration handle failed and category is " << strCategory << " and subgategory is " << strSubcategory);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::DeleteConfigurationHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("category");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Category not found.");
        return blResult;
    }
    const std::string strCategory = itFind->second;

    itFind = pMsgInfoMap->find("sub_category");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sub category not found.");
        return blResult;
    }
    const std::string strSubcategory = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete configuration info received and category is " << strCategory << " and subcategory is " << strSubcategory);

    if (!DeleteConfiguration(strCategory, strSubcategory))
    {
        LOG_ERROR_RLD("Delete configuration handle failed and category is " << strCategory << " and subgategory is " << strSubcategory);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::ModifyConfigurationHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("category");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Category not found.");
        return blResult;
    }
    const std::string strCategory = itFind->second;

    itFind = pMsgInfoMap->find("sub_category");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sub category not found.");
        return blResult;
    }
    const std::string strSubcategory = itFind->second;

    std::string strContent;
    itFind = pMsgInfoMap->find("content");
    if (pMsgInfoMap->end() != itFind)
    {
        strContent = itFind->second;
    }

    std::string strLatestVersion;
    itFind = pMsgInfoMap->find("latest_version");
    if (pMsgInfoMap->end() != itFind)
    {
        strLatestVersion = itFind->second;
    }

    std::string strDesc;
    itFind = pMsgInfoMap->find("description");
    if (pMsgInfoMap->end() != itFind)
    {
        strDesc = itFind->second;
    }

    std::string strForceVersion;
    itFind = pMsgInfoMap->find("force_version");
    if (pMsgInfoMap->end() != itFind)
    {
        strForceVersion = itFind->second;
    }

    std::string strServerAddress;
    itFind = pMsgInfoMap->find("server_address");
    if (pMsgInfoMap->end() != itFind)
    {
        strServerAddress = itFind->second;
    }

    std::string strFilename;
    itFind = pMsgInfoMap->find("file_name");
    if (pMsgInfoMap->end() != itFind)
    {
        strFilename = itFind->second;
    }

    std::string strFileID;
    itFind = pMsgInfoMap->find("file_id");
    if (pMsgInfoMap->end() != itFind)
    {
        strFileID = itFind->second;
    }

    unsigned int uiFileSize = 0;
    std::string strFileSize;
    itFind = pMsgInfoMap->find("file_size");
    if (pMsgInfoMap->end() != itFind)
    {
        strFileSize = itFind->second;

        try
        {
            uiFileSize = boost::lexical_cast<unsigned int>(strFileSize);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Value invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Value invalid and input is " << itFind->second);
            return blResult;
        }
    }

    unsigned int uiLease = 0;
    std::string strLease;
    itFind = pMsgInfoMap->find("lease");
    if (pMsgInfoMap->end() != itFind)
    {
        strLease = itFind->second;

        try
        {
            uiLease = boost::lexical_cast<unsigned int>(strLease);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Value invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Value invalid and input is " << itFind->second);
            return blResult;
        }
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify configuration info received and category is " << strCategory << " and subcategory is " << strSubcategory
        << " and latest version is " << strLatestVersion << " and description is " << strDesc << " and force version " << strForceVersion
        << " and server address is " << strServerAddress << " and file name is " << strFilename << " and file id is " << strFileID
        << " and file size is " << strFileSize << " and lease is " << strLease);

    if (!ModifyConfiguration(strCategory, strSubcategory, strLatestVersion, strDesc, strForceVersion, strServerAddress, strFilename, strFileID, uiFileSize, uiLease))
    {
        LOG_ERROR_RLD("Modify configuration handle failed and category is " << strCategory << " and subgategory is " << strSubcategory);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;

}

bool HttpMsgHandler::QueryConfigurationHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsCfgList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsCfgList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["data"] = jsCfgList;

            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }
    }
    BOOST_SCOPE_EXIT_END

    unsigned int uiBeginIndex = 0;
    auto itFind = pMsgInfoMap->find("beginindex");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiBeginIndex = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Value invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Value invalid and input is " << itFind->second);
            return blResult;
        }
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query configuration info received and begin index is " << uiBeginIndex);

    std::list<InteractiveProtoHandler::Configuration> CfgList;
    if (!QueryConfiguration<InteractiveProtoHandler::Configuration>(uiBeginIndex, CfgList))
    {
        LOG_ERROR_RLD("Query configuration handle failed and begin index is " << uiBeginIndex);
        return blResult;
    }

    auto itBegin = CfgList.begin();
    auto itEnd = CfgList.end();
    while (itBegin != itEnd)
    {
        Json::Value jsCfg;
        jsCfg["category"] = itBegin->m_strCategory;
        jsCfg["sub_category"] = itBegin->m_strSubCategory;
        jsCfg["latest_version"] = itBegin->m_strLatestVersion;
        jsCfg["description"] = itBegin->m_strDescription;
        jsCfg["force_version"] = itBegin->m_strForceVersion;
        jsCfg["file_name"] = itBegin->m_strFileName;
        jsCfg["file_size"] = boost::lexical_cast<std::string>(itBegin->m_uiFileSize);
        jsCfg["file_path"] = itBegin->m_strFilePath;
        jsCfg["lease"] = boost::lexical_cast<std::string>(itBegin->m_uiLeaseDuration);
        jsCfg["update_date"] = itBegin->m_strUpdateDate;

        jsCfgList.append(jsCfg);

        ++itBegin;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::QueryAppUpgradeHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("category");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Category not found.");
        return blResult;
    }
    const std::string strCategory = itFind->second;

    itFind = pMsgInfoMap->find("sub_category");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sub category not found.");
        return blResult;
    }
    const std::string strSubcategory = itFind->second;

    itFind = pMsgInfoMap->find("current_version");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Current version not found.");
        return blResult;
    }
    const std::string strCurrentVersion = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query app upgrade info received and category is " << strCategory << " and subcategory is " << strSubcategory
        << " and current version is " << strCurrentVersion);

    std::string strNewVersionValid;
    std::string strAppName;
    std::string strAppPath;
    unsigned int uiAppSize = 0;
    std::string strNewVersion;
    std::string strDesc;
    std::string strForceUpgrade;
    std::string strUpdateDate;
    if (!QueryAppUpgrade(strCategory, strSubcategory, strCurrentVersion, strNewVersionValid, strAppName, strAppPath, uiAppSize, 
        strNewVersion, strDesc, strForceUpgrade, strUpdateDate))
    {
        LOG_ERROR_RLD("Query app upgrade handle failed and category is " << strCategory << " and subgategory is " << strSubcategory << 
            " and current version is " << strCurrentVersion);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("new_version_valid", strNewVersionValid));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("app_name", strAppName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("app_path", strAppPath));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("app_size", boost::lexical_cast<std::string>(uiAppSize)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("version", strNewVersion));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("description", strDesc));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("force_upgrade", strForceUpgrade));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("update_date", strUpdateDate));

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::QueryDevUpgradeHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("category");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Category not found.");
        return blResult;
    }
    const std::string strCategory = itFind->second;

    std::string strSubcategory;
    std::string strCurrentVersion;
    std::string strDevID;
    if (strCategory == "IPC")
    {
        itFind = pMsgInfoMap->find("sub_category");
        if (pMsgInfoMap->end() == itFind)
        {
            LOG_ERROR_RLD("Sub category not found.");
            return blResult;
        }
        strSubcategory = itFind->second;

        itFind = pMsgInfoMap->find("current_version");
        if (pMsgInfoMap->end() == itFind)
        {
            LOG_ERROR_RLD("Current version not found.");
            return blResult;
        }
        strCurrentVersion = itFind->second;

    }
    else if (strCategory == "Doorbell")
    {
        itFind = pMsgInfoMap->find("devid");
        if (pMsgInfoMap->end() == itFind)
        {
            LOG_ERROR_RLD("Device id not found.");
            return blResult;
        }
        strDevID = itFind->second;
    }
    else
    {
        LOG_ERROR_RLD("Category is invalid and value is " << strCategory);
        return blResult;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query device firmware upgrade info received and category is " << strCategory << " and subcategory is " << strSubcategory
        << " and current version is " << strCurrentVersion << " and device id is " << strDevID);

    std::string strNewVersionValid;
    std::string strFirmwareName;
    std::string strFirmwarePath;
    unsigned int uiFirmwareSize = 0;
    std::string strNewVersion;
    std::string strDesc;
    std::string strForceUpgrade;
    std::string strUpdateDate;
    if (!QueryDevUpgrade(strCategory, strSubcategory, strCurrentVersion, strDevID, strNewVersionValid, strFirmwareName, strFirmwarePath, uiFirmwareSize,
        strNewVersion, strDesc, strForceUpgrade, strUpdateDate))
    {
        LOG_ERROR_RLD("Query device firmware upgrade handle failed and category is " << strCategory << " and subgategory is " << strSubcategory <<
            " and current version is " << strCurrentVersion);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("new_version_valid", strNewVersionValid));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("firmware_name", strFirmwareName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("firmware_path", strFirmwarePath));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("firmware_size", boost::lexical_cast<std::string>(uiFirmwareSize)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("version", strNewVersion));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("description", strDesc));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("force_upgrade", strForceUpgrade));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("update_date", strUpdateDate));

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::QueryDevParamHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    itFind = pMsgInfoMap->find("devtype");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device type not found.");
        return blResult;
    }
    const std::string strDevType = itFind->second;

    unsigned int uiDevType = 0;
    try
    {
        uiDevType = boost::lexical_cast<unsigned int>(strDevType);
    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("Type info is invalid and error msg is " << e.what() << " and input is " << itFind->second);
        return blResult;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Type info is invalid and input is " << itFind->second);
        return blResult;
    }
    
    std::string strQueryType("all");
    itFind = pMsgInfoMap->find("query_type");
    if (pMsgInfoMap->end() != itFind)
    {
        strQueryType = itFind->second;
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query device param info received and device id is " << strDevID << " and device type is " << strDevType
        << " and query type is " << strQueryType << " and session id is " << strSid);

    DeviceProperty devpt;
    if (!QueryDevParam(strSid, strDevID, uiDevType, strQueryType, devpt))
    {
        LOG_ERROR_RLD("Query device param handle failed and device id is " << strDevID << " and device type is " << uiDevType
            << " and query type is " << strQueryType << " and session id is " << strSid);
        return blResult;
    }
    
    if (0 == uiDevType) //门铃设备
    {
        ResultInfoMap.insert(std::map<std::string, std::string>::value_type("doorbell_name", devpt.m_strDoorbellName));
        ResultInfoMap.insert(std::map<std::string, std::string>::value_type("volume_level", devpt.m_strVolumeLevel));
        ResultInfoMap.insert(std::map<std::string, std::string>::value_type("pir_alarm_switch", devpt.m_strPirAlarmSwitch));
        ResultInfoMap.insert(std::map<std::string, std::string>::value_type("doorbell_switch", devpt.m_strDoorbellSwitch));
        ResultInfoMap.insert(std::map<std::string, std::string>::value_type("pir_alarm_level", devpt.m_strPirAlarmLevel));
        ResultInfoMap.insert(std::map<std::string, std::string>::value_type("pir_ineffective_time", devpt.m_strPirIneffectiveTime));


        if (strQueryType == "all")
        {
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("serial_number", devpt.m_strSerialNum));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("doorbell_p2pid", devpt.m_strDoorbellP2pid));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("battery_capacity", devpt.m_strBatteryCap));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("charging_state", devpt.m_strChargingState));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("wifi_signal", devpt.m_strWifiSig));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("version_number", devpt.m_strVersionNum));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("channel_number", devpt.m_strChannelNum));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("coding_type", devpt.m_strCodingType));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("current_wifi", devpt.m_strCurrentWifi));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("sub_category", devpt.m_strSubCategory));

        }
    }

    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;

    return true;
}

bool HttpMsgHandler::QueryPushStatusHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Devid not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query push status info received and device id is " << strDevID);

    std::string strPushStatus;
    if (!QueryPushStatus(strDevID, strPushStatus))
    {
        LOG_ERROR_RLD("Query push status handle failed and device id is " << strDevID);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("push_status", strPushStatus));

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::DeviceEventReportHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    itFind = pMsgInfoMap->find("devtype");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device type not found.");
        return blResult;
    }

    unsigned int uiDevType = 0;
    try
    {
        uiDevType = boost::lexical_cast<unsigned int>(itFind->second);
    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("Device event report info of device type is invalid and error msg is " << e.what() << " and input is " << itFind->second);
        return blResult;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Device event report info of device type is invalid and input is " << itFind->second);
        return blResult;
    }

    itFind = pMsgInfoMap->find("event_type");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Event type not found.");
        return blResult;
    }

    unsigned int uiEventType = 0;
    try
    {
        uiEventType = boost::lexical_cast<unsigned int>(itFind->second);
    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("Device event report info of event type is invalid and error msg is " << e.what() << " and input is " << itFind->second);
        return blResult;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Device event report info of event type is invalid and input is " << itFind->second);
        return blResult;
    }

    itFind = pMsgInfoMap->find("event_status");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Event status not found.");
        return blResult;
    }

    unsigned int uiEventStatus = 0;
    try
    {
        uiEventStatus = boost::lexical_cast<unsigned int>(itFind->second);
    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("Device event report info of event status is invalid and error msg is " << e.what() << " and input is " << itFind->second);
        return blResult;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Device event report info of event status is invalid and input is " << itFind->second);
        return blResult;
    }

    std::string strFileid;
    itFind = pMsgInfoMap->find("fileid");
    if (pMsgInfoMap->end() != itFind)
    {
        strFileid = itFind->second;
    }
    
    itFind = pMsgInfoMap->find("event_time");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Event time not found.");
        return blResult;
    }
    const std::string strEventTime = itFind->second;

    if (!ValidDatetime(strEventTime))
    {
        LOG_ERROR_RLD("File begin date is invalid and input date is " << strEventTime);
        return blResult;
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Device event report info received and device id is " << strDevID << " and device type is " << uiDevType
        << " and event type is " << uiEventType << " and event status is " << uiEventStatus << " and file id is " << strFileid 
        << " and sid is " << strSid << " and event time is " << strEventTime);

    Event ev;
    ev.m_strDevID = strDevID;
    ev.m_uiDevType = uiDevType;
    ev.m_uiEventStatus = uiEventStatus;
    ev.m_uiEventType = uiEventType;
    ev.m_strFileID = strFileid;
    ev.m_strEventTime = strEventTime;
    if (!DeviceEventReport(strSid, ev))
    {
        LOG_ERROR_RLD("Device event report handle failed and device id is " << strDevID);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("event_id", ev.m_strEventID));

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;

}

bool HttpMsgHandler::QueryDeviceEventHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsEventList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsEventList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }
        else
        {
            auto FuncTmp = [&](void *pValue)
            {
                Json::Value *pJsBody = (Json::Value*)pValue;
                (*pJsBody)["data"] = jsEventList;

            };

            this_->WriteMsg(ResultInfoMap, writer, blResult, FuncTmp);
        }

    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;
    
    unsigned int uiDevShared = 0;
    itFind = pMsgInfoMap->find("dev_shared");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiDevShared = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Device event query info of device shared is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Device event query info of device shared is invalid and input is " << itFind->second);
            return blResult;
        }
    }

    unsigned int uiEventType = 2;
    itFind = pMsgInfoMap->find("event_type");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiEventType = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Device event query info of event type is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Device event query info of event type is invalid and input is " << itFind->second);
            return blResult;
        }
    }

    unsigned int uiView = 1;
    itFind = pMsgInfoMap->find("view_already");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiView = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Device event query info of view is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Device event query info of view is invalid and input is " << itFind->second);
            return blResult;
        }
    }

    unsigned int uiBeginIndex = 0;
    itFind = pMsgInfoMap->find("beginindex");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiBeginIndex = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Device event query info of begin index is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Device event query info of begin index is invalid and input is " << itFind->second);
            return blResult;
        }
    }

    std::string strBeginDate;
    itFind = pMsgInfoMap->find("begindate");
    if (pMsgInfoMap->end() != itFind)
    {
        strBeginDate = itFind->second;

        if (!ValidDatetime(strBeginDate))
        {
            LOG_ERROR_RLD("Begin date is invalid and input date is " << strBeginDate);
            return blResult;
        }
    }

    std::string strEndDate;
    itFind = pMsgInfoMap->find("enddate");
    if (pMsgInfoMap->end() != itFind)
    {
        strEndDate = itFind->second;

        if (!ValidDatetime(strEndDate))
        {
            LOG_ERROR_RLD("End date is invalid and input date is " << strEndDate);
            return blResult;
        }
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query device event info received and  user id is " << strUserID <<
        " and device id is " << strDevID << " and device shared is " << uiDevShared << " and event type is " << uiEventType <<
        " and view is " << uiView << " and begin index is " << uiBeginIndex << " and begin date is " << strBeginDate <<
        " and end date is " << strEndDate);

    std::list<Event> evlist;
    if (!QueryDeviceEvent(strSid, strUserID, strDevID, uiDevShared, uiEventType, uiView, uiBeginIndex, strBeginDate, strEndDate, evlist))
    {
        LOG_ERROR_RLD("Query device event handle failed");
        return blResult;
    }

    auto itBegin = evlist.begin();
    auto itEnd = evlist.end();
    while (itBegin != itEnd)
    {
        Json::Value jsEvent;
        jsEvent["devid"] = itBegin->m_strDevID;
        jsEvent["event_id"] = itBegin->m_strEventID;
        jsEvent["fileurl"] = itBegin->m_strFileURL;
        jsEvent["devtype"] = itBegin->m_uiDevType;
        jsEvent["event_status"] = itBegin->m_uiEventStatus;
        jsEvent["event_type"] = itBegin->m_uiEventType;
        jsEvent["event_time"] = itBegin->m_strEventTime;

        jsEventList.append(jsEvent);

        ++itBegin;
    }
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::DeleteDeviceEventHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    itFind = pMsgInfoMap->find("event_id");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Event id not found.");
        return blResult;
    }
    const std::string strEventID = itFind->second;
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete device event info received and sid is " << strSid << " and user id is " << strUserID << " and device id is " << strDevID
        << " and event id is " << strEventID);

    if (!DeleteDeviceEvent(strSid, strUserID, strDevID, strEventID))
    {
        LOG_ERROR_RLD("Delete device event handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::ModifyDeviceEventHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    std::string strUserID;
    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() != itFind)
    {
        strUserID = itFind->second;
    }
    
    itFind = pMsgInfoMap->find("devid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }
    const std::string strDevID = itFind->second;

    itFind = pMsgInfoMap->find("event_id");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Event id not found.");
        return blResult;
    }
    const std::string strEventID = itFind->second;
    
    unsigned int uiEventType = 0xFFFFFFFF;
    itFind = pMsgInfoMap->find("event_type");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiEventType = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Device event report info of event type is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Device event report info of event type is invalid and input is " << itFind->second);
            return blResult;
        }
    }
           
    unsigned int uiEventStatus = 0xFFFFFFFF;
    itFind = pMsgInfoMap->find("event_status");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiEventStatus = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Device event report info of event status is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Device event report info of event status is invalid and input is " << itFind->second);
            return blResult;
        }
    }

    std::string strEventTime;
    itFind = pMsgInfoMap->find("event_time");
    if (pMsgInfoMap->end() != itFind)
    {
        strEventTime = itFind->second;
    }
    
    if (!ValidDatetime(strEventTime))
    {
        LOG_ERROR_RLD("File begin date is invalid and input date is " << strEventTime);
        return blResult;
    }

    std::string strFileID;
    itFind = pMsgInfoMap->find("fileid");
    if (pMsgInfoMap->end() != itFind)
    {
        strFileID = itFind->second;
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify device event info received and sid is " << strSid << " and user id is " << strUserID << " and device id is " << strDevID
        << " and event id is " << strEventID << " and event type is " << uiEventType << " and event status is " << uiEventStatus << 
        " and event time is " << strEventTime << " and file id is " << strFileID);

    Event ev;
    ev.m_strDevID = strDevID;
    ev.m_strEventID = strEventID;
    //ev.m_uiDevType = uiDevType;
    ev.m_uiEventStatus = uiEventStatus;
    ev.m_uiEventType = uiEventType;
    ev.m_strFileID = strFileID;
    ev.m_strEventTime = strEventTime;
    if (!ModifyDeviceEvent(strSid, ev))
    {
        LOG_ERROR_RLD("Modify device event report handle failed and device id is " << strDevID);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::AddUserSpaceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    unsigned int uiDomainID = 0;
    itFind = pMsgInfoMap->find("domain_id");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiDomainID = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Domain id is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Domain id is invalid and input is " << itFind->second);
            return blResult;
        }
    }

    std::string strStName;
    itFind = pMsgInfoMap->find("storage_name");
    if (pMsgInfoMap->end() != itFind)
    {
        strStName = itFind->second;
    }
    
    itFind = pMsgInfoMap->find("overlap_type");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Device id not found.");
        return blResult;
    }

    unsigned int uiOverlapType = 0;
    try
    {
        uiOverlapType = boost::lexical_cast<unsigned int>(itFind->second);
    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("Overlap type is invalid and error msg is " << e.what() << " and input is " << itFind->second);
        return blResult;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Overlap type is invalid and input is " << itFind->second);
        return blResult;
    }

    if (0 != uiOverlapType && 1 != uiOverlapType)
    {
        LOG_ERROR_RLD("Overlap type is invalid and value is " << uiOverlapType);
        return blResult;
    }

    itFind = pMsgInfoMap->find("storagetime_up_limit");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Storage uplimit not found.");
        return blResult;
    }

    unsigned int uiStUpLimit = 0;
    try
    {
        uiStUpLimit = boost::lexical_cast<unsigned int>(itFind->second);
    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("Storage uplimit is invalid and error msg is " << e.what() << " and input is " << itFind->second);
        return blResult;
    }
    catch (...)
    {
        LOG_ERROR_RLD("Storage uplimit is invalid and input is " << itFind->second);
        return blResult;
    }

    unsigned int uiStDownLimit = 0;
    itFind = pMsgInfoMap->find("storagetime_down_limit");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiStDownLimit = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Storage downlimit is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Storage downlimit is invalid and input is " << itFind->second);
            return blResult;
        }
    }

    if (uiStDownLimit >= uiStUpLimit)
    {
        LOG_ERROR_RLD("Storage downlimit is larger than uplimit and uplimit is " << uiStUpLimit << " and downlimit is " << uiStDownLimit);
        return blResult;
    }

    std::string strBeginDate;
    itFind = pMsgInfoMap->find("begindate");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidDatetime(itFind->second))
        {
            LOG_ERROR_RLD("Begin date is invalid and value is " << itFind->second);
            return blResult;
        }

        strBeginDate = itFind->second;
    }
    else
    {
        std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
        std::string::size_type pos = strCurrentTime.find('T');
        strCurrentTime.replace(pos, 1, std::string(" "));

        strBeginDate = strCurrentTime;
    }
    
    std::string strEndDate("2199-01-01");
    itFind = pMsgInfoMap->find("enddate");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidDatetime(itFind->second))
        {
            LOG_ERROR_RLD("End date is invalid and value is " << itFind->second);
            return blResult;
        }

        strEndDate = itFind->second;
    }
   
    boost::gregorian::date begindate = boost::gregorian::from_string(strBeginDate);
    boost::gregorian::date enddate = boost::gregorian::from_string(strEndDate);

    if (begindate >= enddate)
    {
        LOG_ERROR_RLD("Begin date is larger than end date and begin date is " << strBeginDate << " and end date is " << strEndDate);
        return blResult;
    }
    
    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Add storage space info received and sid is " << strSid << " and user id is " << strUserID << " and domain id is " << uiDomainID
        << " and storage name is " << strStName << " and overlap type is " << uiOverlapType << " and storage uplimit is " << uiStUpLimit
        << " and storage downlimit is " << uiStDownLimit << " and begin date is " << strBeginDate << " and end date is " << strEndDate
        << " and extend is " << strExtend);

    SpaceInfo stif;
    stif.m_strBeginDate = strBeginDate;
    stif.m_strEndDate = strEndDate;
    stif.m_strExtend = strExtend;
    stif.m_strStorageName = strStName;
    stif.m_uiDomainID = uiDomainID;
    stif.m_uiOverlapType = uiOverlapType;
    stif.m_uiStorageTimeDownLimit = uiStDownLimit;
    stif.m_uiStorageTimeUpLimit = uiStUpLimit;

    if (!AddUserSpace(strSid, strUserID, stif))
    {
        LOG_ERROR_RLD("Add storage space handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::DeleteUserSpaceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;
    
    unsigned int uiDomainID = 0;
    itFind = pMsgInfoMap->find("domain_id");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiDomainID = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Domain id is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Domain id is invalid and input is " << itFind->second);
            return blResult;
        }
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete user space info received and sid is " << strSid << " and user id is " << strUserID << " and domain id is " << uiDomainID);

    if (!DeleteUserSpace(strSid, strUserID, uiDomainID))
    {
        LOG_ERROR_RLD("Delete user space handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::ModifyUserSpaceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;
    
    std::string strStName;
    itFind = pMsgInfoMap->find("storage_name");
    if (pMsgInfoMap->end() != itFind)
    {
        strStName = itFind->second;
    }

    unsigned int uiOverlapType = 0;
    itFind = pMsgInfoMap->find("overlap_type");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiOverlapType = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Overlap type is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Overlap type is invalid and input is " << itFind->second);
            return blResult;
        }

        if (0 != uiOverlapType && 1 == uiOverlapType)
        {
            LOG_ERROR_RLD("Overlap type is invalid and value is " << uiOverlapType);
            return blResult;
        }
    }

    unsigned int uiStUpLimit = 0xFFFFFFFF;
    itFind = pMsgInfoMap->find("storagetime_up_limit");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiStUpLimit = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Storage uplimit is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Storage uplimit is invalid and input is " << itFind->second);
            return blResult;
        }
    }

    unsigned int uiStDownLimit = 0xFFFFFFFF;
    itFind = pMsgInfoMap->find("storagetime_down_limit");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiStDownLimit = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Storage downlimit is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Storage downlimit is invalid and input is " << itFind->second);
            return blResult;
        }
    }
        
    if (0xFFFFFFFF != uiStUpLimit && 0xFFFFFFFF != uiStDownLimit && uiStDownLimit >= uiStUpLimit)
    {
        LOG_ERROR_RLD("Storage downlimit is larger than uplimit and uplimit is " << uiStUpLimit << " and downlimit is " << uiStDownLimit);
        return blResult;
    }

    std::string strBeginDate;
    itFind = pMsgInfoMap->find("begindate");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidDatetime(itFind->second))
        {
            LOG_ERROR_RLD("Begin date is invalid and value is " << itFind->second);
            return blResult;
        }

        strBeginDate = itFind->second;
    }
    else
    {
        std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
        std::string::size_type pos = strCurrentTime.find('T');
        strCurrentTime.replace(pos, 1, std::string(" "));

        strBeginDate = strCurrentTime;
    }

    std::string strEndDate("2199-01-01");
    itFind = pMsgInfoMap->find("enddate");
    if (pMsgInfoMap->end() != itFind)
    {
        if (!ValidDatetime(itFind->second))
        {
            LOG_ERROR_RLD("End date is invalid and value is " << itFind->second);
            return blResult;
        }

        strEndDate = itFind->second;
    }

    boost::gregorian::date begindate = boost::gregorian::from_string(strBeginDate);
    boost::gregorian::date enddate = boost::gregorian::from_string(strEndDate);

    if (begindate >= enddate)
    {
        LOG_ERROR_RLD("Begin date is larger than end date and begin date is " << strBeginDate << " and end date is " << strEndDate);
        return blResult;
    }

    std::string strExtend;
    itFind = pMsgInfoMap->find("extend");
    if (pMsgInfoMap->end() != itFind)
    {
        strExtend = itFind->second;
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Modify storage space info received and sid is " << strSid << " and user id is " << strUserID
        << " and storage name is " << strStName << " and overlap type is " << uiOverlapType << " and storage uplimit is " << uiStUpLimit
        << " and storage downlimit is " << uiStDownLimit << " and begin date is " << strBeginDate << " and end date is " << strEndDate
        << " and extend is " << strExtend);

    SpaceInfo stif;
    stif.m_strBeginDate = strBeginDate;
    stif.m_strEndDate = strEndDate;
    stif.m_strExtend = strExtend;
    stif.m_strStorageName = strStName;
    stif.m_uiDomainID = 0;
    stif.m_uiOverlapType = uiOverlapType;
    stif.m_uiStorageTimeDownLimit = uiStDownLimit;
    stif.m_uiStorageTimeUpLimit = uiStUpLimit;

    if (!ModifyUserSpace(strSid, strUserID, stif))
    {
        LOG_ERROR_RLD("Modify storage space handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;

}

bool HttpMsgHandler::QueryUserSpaceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;

    unsigned int uiDomainID = 0;
    itFind = pMsgInfoMap->find("domain_id");
    if (pMsgInfoMap->end() != itFind)
    {
        try
        {
            uiDomainID = boost::lexical_cast<unsigned int>(itFind->second);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Domain id is invalid and error msg is " << e.what() << " and input is " << itFind->second);
            return blResult;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Domain id is invalid and input is " << itFind->second);
            return blResult;
        }
    }

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query user space info received and sid is " << strSid << " and user id is " << strUserID << " and domain id is " << uiDomainID);

    SpaceInfo stif;
    if (!QueryUserSpace(strSid, strUserID, uiDomainID, stif))
    {
        LOG_ERROR_RLD("Query user space handle failed");
        return blResult;
    }
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("domain_id", boost::lexical_cast<std::string>(stif.m_uiDomainID)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("storage_name", stif.m_strStorageName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("overlap_type", boost::lexical_cast<std::string>(stif.m_uiOverlapType)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("storagetime_up_limit", boost::lexical_cast<std::string>(stif.m_uiStorageTimeUpLimit)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("storagetime_down_limit", boost::lexical_cast<std::string>(stif.m_uiStorageTimeDownLimit)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("begindate", stif.m_strBeginDate));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("enddate", stif.m_strEndDate));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("extend", stif.m_strExtend));

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::QueryStorageSpaceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("sid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Sid not found.");
        return blResult;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return blResult;
    }
    const std::string strUserID = itFind->second;
        
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query storage space info received and sid is " << strSid << " and user id is " << strUserID);

    StorageInfo stif;
    if (!QueryStorageSpace(strSid, strUserID, stif))
    {
        LOG_ERROR_RLD("Query storage space handle failed");
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("domain_id", boost::lexical_cast<std::string>(stif.m_uiDomainID)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("sizeof_space", boost::lexical_cast<std::string>(stif.m_uiSpaceSize)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("sizeof_spaceused", boost::lexical_cast<std::string>(stif.m_uiSpaceSizeUsed)));
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

void HttpMsgHandler::WriteMsg(const std::map<std::string, std::string> &MsgMap, MsgWriter writer, const bool blResult, boost::function<void(void*)> PostFunc)
{
    Json::Value jsBody;
    auto itBegin = MsgMap.begin();
    auto itEnd = MsgMap.end();
    while (itBegin != itEnd)
    {
        jsBody[itBegin->first] = itBegin->second;
        
        ++itBegin;
    }

    if (NULL != PostFunc)
    {
        PostFunc((void *)&jsBody);
    }    

    //Json::FastWriter fastwriter;
    Json::StyledWriter stylewriter;
    const std::string &strBody = stylewriter.write(jsBody); //fastwriter.write(jsBody);//jsBody.toStyledString();

    //writer(strBody.c_str(), strBody.size(), MsgWriterModel::PRINT_MODEL);

    std::string strOutputMsg;
    if (!blResult)
    {
        strOutputMsg = "Status: 500  Error\r\nContent-Type: text/html\r\n\r\n";
    }
    else
    {
        strOutputMsg = "Status: 200 OK\r\nContent-Type: text/html\r\n\r\n";
    }

    strOutputMsg += strBody;
    strOutputMsg += "\r\n";

    writer(strOutputMsg.c_str(), strOutputMsg.size(), MsgWriterModel::PRINT_MODEL);
    
    //writer("Content-type: text/*\r\n\r\n", 0, MsgWriterModel::PRINT_MODEL);
    //writer("<title>FastCGI Hello! (C, fcgi_stdio library)</title>\n", 0, MsgWriterModel::PRINT_MODEL);

}

bool HttpMsgHandler::PreCommonHandler(const std::string &strMsgReceived, int &iRetCode)
{
    InteractiveProtoHandler::MsgType mtype;
    if (!m_pInteractiveProtoHandler->GetMsgType(strMsgReceived, mtype))
    {
        LOG_ERROR_RLD("Get msg type failed.");
        return false;
    }

    if (InteractiveProtoHandler::MsgType::MsgPreHandlerRsp_USR_T == mtype)
    {
        InteractiveProtoHandler::MsgPreHandlerRsp_USR rsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, rsp))
        {
            LOG_ERROR_RLD("Msg prehandler rsp unserialize failed.");
            return false;
        }

        LOG_INFO_RLD("Msg prehandler rsp return code is " << rsp.m_iRetcode << " return msg is " << rsp.m_strRetMsg <<
            " and user session id is " << rsp.m_strSID);

        if (CommMsgHandler::SUCCEED != (iRetCode = rsp.m_iRetcode))
        {
            LOG_ERROR_RLD("Msg prehandler rsp return failed.");
            return false;
        }
    }

    return true;
}

bool HttpMsgHandler::RegisterUser(const std::string &strUserName, const std::string &strUserPwd, const std::string &strType, const std::string &strExtend, 
    const std::string &strAliasName, const std::string &strEmail, std::string &strUserID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        unsigned int uiTypeInfo = 0;

        try
        {
            uiTypeInfo = boost::lexical_cast<unsigned int>(strType);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Register user type info is invalid and error msg is " << e.what() << " and input type is " << strType);
            return CommMsgHandler::FAILED;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Register user type info is invalid" << " and input type is " << strType);
            return CommMsgHandler::FAILED;
        }

        std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
        std::string::size_type pos = strCurrentTime.find('T');
        strCurrentTime.replace(pos, 1, std::string(" "));

        InteractiveProtoHandler::RegisterUserReq_USR RegUsrReq;
        RegUsrReq.m_MsgType = InteractiveProtoHandler::MsgType::RegisterUserReq_USR_T;
        RegUsrReq.m_uiMsgSeq = 1;
        RegUsrReq.m_strSID = "";
        RegUsrReq.m_strValue = "";
        RegUsrReq.m_userInfo.m_uiStatus = 0;
        RegUsrReq.m_userInfo.m_strUserID = "";
        RegUsrReq.m_userInfo.m_strUserName = strUserName;
        RegUsrReq.m_userInfo.m_strUserPassword = strUserPwd;
        RegUsrReq.m_userInfo.m_uiTypeInfo = uiTypeInfo;
        RegUsrReq.m_userInfo.m_strCreatedate = strCurrentTime;
        RegUsrReq.m_userInfo.m_strExtend = strExtend;
        RegUsrReq.m_userInfo.m_strAliasName = strAliasName;
        RegUsrReq.m_userInfo.m_strEmail = strEmail;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(RegUsrReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Register user req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::RegisterUserRsp_USR RegUsrRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, RegUsrRsp))
        {
            LOG_ERROR_RLD("Register user rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }
        
        strUserID = RegUsrRsp.m_strUserID;
        iRet = RegUsrRsp.m_iRetcode;

        

        LOG_INFO_RLD("Register user id is " << strUserID << " and return code is " << RegUsrRsp.m_iRetcode <<
            " and return msg is " << RegUsrRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };
    
    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress, 
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::UnRegisterUser(const std::string &strSid, const std::string &strUserID, const std::string &strUserName, const std::string &strUserPwd)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        
        std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
        std::string::size_type pos = strCurrentTime.find('T');
        strCurrentTime.replace(pos, 1, std::string(" "));

        InteractiveProtoHandler::UnRegisterUserReq_USR UnRegUsrReq;
        UnRegUsrReq.m_MsgType = InteractiveProtoHandler::MsgType::UnRegisterUserReq_USR_T;
        UnRegUsrReq.m_uiMsgSeq = 1;
        UnRegUsrReq.m_strSID = strSid;
        UnRegUsrReq.m_strValue = "";
        UnRegUsrReq.m_userInfo.m_uiStatus = 0;
        UnRegUsrReq.m_userInfo.m_strUserID = strUserID;
        UnRegUsrReq.m_userInfo.m_strUserName = strUserName;
        UnRegUsrReq.m_userInfo.m_strUserPassword = strUserPwd;
        UnRegUsrReq.m_userInfo.m_uiTypeInfo = 0;
        UnRegUsrReq.m_userInfo.m_strCreatedate = strCurrentTime;
        UnRegUsrReq.m_userInfo.m_strExtend = "";
        UnRegUsrReq.m_userInfo.m_strAliasName = "";
        UnRegUsrReq.m_userInfo.m_strEmail = "";

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(UnRegUsrReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("UnRegister user req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);
        
        InteractiveProtoHandler::UnRegisterUserRsp_USR UnRegUsrRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, UnRegUsrRsp))
        {
            LOG_ERROR_RLD("UnRegister user rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }
        
        iRet = UnRegUsrRsp.m_iRetcode;

        

        LOG_INFO_RLD("Unregister user id is " << UnRegUsrRsp.m_strUserID << " and return code is " << UnRegUsrRsp.m_iRetcode <<
            " and return msg is " << UnRegUsrRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    
    
    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;    
}

template<typename T>
bool HttpMsgHandler::UserLogin(const std::string &strUserName, const std::string &strUserPwd, const unsigned int uiType, 
    const unsigned int uiTerminalType, std::list<T> &RelationList,
    std::string &strUserID, std::string &strSid, std::list<std::string> &strDevNameList)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {

        std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
        std::string::size_type pos = strCurrentTime.find('T');
        strCurrentTime.replace(pos, 1, std::string(" "));

        InteractiveProtoHandler::LoginReq_USR UsrLoginReq;
        UsrLoginReq.m_MsgType = InteractiveProtoHandler::MsgType::LoginReq_USR_T;
        UsrLoginReq.m_uiMsgSeq = 1;
        UsrLoginReq.m_strSID = "";
        UsrLoginReq.m_strValue = "";
        UsrLoginReq.m_userInfo.m_uiStatus = 0;
        UsrLoginReq.m_userInfo.m_strUserID = strUserID;
        UsrLoginReq.m_userInfo.m_strUserName = strUserName;
        UsrLoginReq.m_userInfo.m_strUserPassword = strUserPwd;
        UsrLoginReq.m_userInfo.m_uiTypeInfo = 0;
        UsrLoginReq.m_userInfo.m_strCreatedate = strCurrentTime;
        UsrLoginReq.m_userInfo.m_strExtend = "";
        UsrLoginReq.m_userInfo.m_strAliasName = "";
        UsrLoginReq.m_userInfo.m_strEmail = "";
        UsrLoginReq.m_uiTerminalType = uiTerminalType;
        UsrLoginReq.m_uiType = uiType;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(UsrLoginReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Login user req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::LoginRsp_USR UsrLoginRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, UsrLoginRsp))
        {
            LOG_ERROR_RLD("Login user rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        RelationList.clear();
        RelationList.swap(UsrLoginRsp.m_reInfoList);
        strDevNameList.clear();
        strDevNameList.swap(UsrLoginRsp.m_strDevNameList);
        

        strUserID = UsrLoginRsp.m_strUserID;
        strSid = UsrLoginRsp.m_strSID;
        iRet = UsrLoginRsp.m_iRetcode;

        

        LOG_INFO_RLD("Login user id is " << UsrLoginRsp.m_strUserID << " and session id is " << UsrLoginRsp.m_strSID << 
            " and return code is " << UsrLoginRsp.m_iRetcode <<
            " and return msg is " << UsrLoginRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress, 
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

template<typename T>
bool HttpMsgHandler::QueryUserInfo(const std::string &strSid, const std::string &strUserID, T &UserInfo)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryUsrInfoReq_USR QueryUsrInfoReq;
        QueryUsrInfoReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryUsrInfoReq_USR_T;
        QueryUsrInfoReq.m_uiMsgSeq = 1;
        QueryUsrInfoReq.m_strSID = strSid;
        QueryUsrInfoReq.m_strValue = "";
        QueryUsrInfoReq.m_strUserID = strUserID;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryUsrInfoReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query user info req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::QueryUsrInfoRsp_USR QueryUsrInfoRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryUsrInfoRsp))
        {
            LOG_ERROR_RLD("Query user info rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        UserInfo = QueryUsrInfoRsp.m_userInfo;

        iRet = QueryUsrInfoRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query user info  return code is " << QueryUsrInfoRsp.m_iRetcode <<
            " and return msg is " << QueryUsrInfoRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::ModifyUserInfo(const std::string &strSid, const std::string &strUserID, const std::string &strUserName, 
    const std::string &strNewUserPwd, const std::string &strOldUserPwd, const unsigned int uiType, const std::string &strExtend,
    const std::string &strAliasName, const std::string &strEmail)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::ModifyUserInfoReq_USR ModifyUsrInfoReq;
        ModifyUsrInfoReq.m_MsgType = InteractiveProtoHandler::MsgType::ModifyUserInfoReq_USR_T;
        ModifyUsrInfoReq.m_uiMsgSeq = 1;
        ModifyUsrInfoReq.m_strSID = strSid;
        ModifyUsrInfoReq.m_userInfo.m_strCreatedate = "";
        ModifyUsrInfoReq.m_userInfo.m_strExtend = strExtend;
        ModifyUsrInfoReq.m_userInfo.m_strUserID = strUserID;
        ModifyUsrInfoReq.m_userInfo.m_strUserName = strUserName;
        ModifyUsrInfoReq.m_userInfo.m_strUserPassword = strNewUserPwd;
        ModifyUsrInfoReq.m_userInfo.m_uiStatus = 0;
        ModifyUsrInfoReq.m_userInfo.m_uiTypeInfo = uiType;
        ModifyUsrInfoReq.m_userInfo.m_strAliasName = strAliasName;
        ModifyUsrInfoReq.m_userInfo.m_strEmail = strEmail;

        ModifyUsrInfoReq.m_strOldPwd = strOldUserPwd;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModifyUsrInfoReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify user info req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::ModifyUserInfoRsp_USR ModifyUsrInfoRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModifyUsrInfoRsp))
        {
            LOG_ERROR_RLD("Modify user info rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModifyUsrInfoRsp.m_iRetcode;

        

        LOG_INFO_RLD("Modify user info  return code is " << ModifyUsrInfoRsp.m_iRetcode <<
            " and return msg is " << ModifyUsrInfoRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::UserLogout(const std::string &strSid, const std::string &strUserID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {

        std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
        std::string::size_type pos = strCurrentTime.find('T');
        strCurrentTime.replace(pos, 1, std::string(" "));

        InteractiveProtoHandler::LogoutReq_USR UsrLogoutReq;
        UsrLogoutReq.m_MsgType = InteractiveProtoHandler::MsgType::LogoutReq_USR_T;
        UsrLogoutReq.m_uiMsgSeq = 1;
        UsrLogoutReq.m_strSID = strSid;
        UsrLogoutReq.m_strValue = "";
        UsrLogoutReq.m_userInfo.m_uiStatus = 0;
        UsrLogoutReq.m_userInfo.m_strUserID = strUserID;
        UsrLogoutReq.m_userInfo.m_strUserName = "";
        UsrLogoutReq.m_userInfo.m_strUserPassword = "";
        UsrLogoutReq.m_userInfo.m_uiTypeInfo = 0;
        UsrLogoutReq.m_userInfo.m_strCreatedate = strCurrentTime;
        UsrLogoutReq.m_userInfo.m_strExtend = "";
        UsrLogoutReq.m_userInfo.m_strAliasName = "";
        UsrLogoutReq.m_userInfo.m_strEmail = "";
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(UsrLogoutReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Logout user req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::LogoutRsp_USR UsrLogoutRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, UsrLogoutRsp))
        {
            LOG_ERROR_RLD("Logout user rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = UsrLogoutRsp.m_iRetcode;

        

        LOG_INFO_RLD("Logout user id is " << strUserID << " and session id is " << strSid <<
            " and return code is " << UsrLogoutRsp.m_iRetcode <<
            " and return msg is " << UsrLogoutRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::Shakehand(const std::string &strSid, const std::string &strUserID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::ShakehandReq_USR ShakehandReq;
        ShakehandReq.m_MsgType = InteractiveProtoHandler::MsgType::ShakehandReq_USR_T;
        ShakehandReq.m_uiMsgSeq = 1;
        ShakehandReq.m_strSID = strSid;
        ShakehandReq.m_strValue = "";
        ShakehandReq.m_strUserID = strUserID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ShakehandReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Shakehand req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::ShakehandRsp_USR ShakehandRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ShakehandRsp))
        {
            LOG_ERROR_RLD("Shakehand rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ShakehandRsp.m_iRetcode;

        

        LOG_INFO_RLD("Shakehand user id is " << strUserID << " and session id is " << strSid <<
            " and return code is " << ShakehandRsp.m_iRetcode <<
            " and return msg is " << ShakehandRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::AddDevice(const std::string &strSid, const std::string &strUserID, const DeviceIf &devif, std::string &strDevID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        unsigned int uiTypeInfo = 0;
        try
        {
            uiTypeInfo = boost::lexical_cast<unsigned int>(devif.m_strDevType);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Add device type info is invalid and error msg is " << e.what() << " and input type is " << devif.m_strDevType);
            return CommMsgHandler::FAILED;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Add device type info is invalid" << " and input type is " << devif.m_strDevType);
            return CommMsgHandler::FAILED;
        }

        std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
        std::string::size_type pos = strCurrentTime.find('T');
        strCurrentTime.replace(pos, 1, std::string(" "));

        InteractiveProtoHandler::AddDevReq_USR AddDevReq;
        AddDevReq.m_MsgType = InteractiveProtoHandler::MsgType::AddDevReq_USR_T;
        AddDevReq.m_uiMsgSeq = 1;
        AddDevReq.m_strSID = strSid;
        AddDevReq.m_strUserID = strUserID;
        AddDevReq.m_devInfo.m_strCreatedate = strCurrentTime;
        AddDevReq.m_devInfo.m_strDevID = devif.m_strDevID;
        AddDevReq.m_devInfo.m_strDevName = devif.m_strDevName;
        AddDevReq.m_devInfo.m_strDevPassword = devif.m_strDevPwd;
        AddDevReq.m_devInfo.m_strExtend = devif.m_strDevExtend;
        AddDevReq.m_devInfo.m_strInnerinfo = devif.m_strDevInnerInfo;
        AddDevReq.m_devInfo.m_uiStatus = 0;
        AddDevReq.m_devInfo.m_uiTypeInfo = uiTypeInfo;
        AddDevReq.m_devInfo.m_strDomainName = devif.m_strDomainname;
        AddDevReq.m_devInfo.m_strP2pID = devif.m_strP2pid;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddDevReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add device req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::AddDevRsp_USR AddDevRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddDevRsp))
        {
            LOG_ERROR_RLD("Add device rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = AddDevRsp.m_iRetcode;

        strDevID = AddDevRsp.m_strDeviceID;

        

        LOG_INFO_RLD("Add device is " << strUserID << " and session id is " << strSid <<
            " and return code is " << AddDevRsp.m_iRetcode <<
            " and return msg is " << AddDevRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::DeleteDevice(const std::string &strSid, const std::string &strUserID, const std::string &strDevID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::DelDevReq_USR DelDevReq;
        DelDevReq.m_MsgType = InteractiveProtoHandler::MsgType::DelDevReq_USR_T;
        DelDevReq.m_uiMsgSeq = 1;
        DelDevReq.m_strSID = strSid;
        DelDevReq.m_strDevIDList.push_back(strDevID);
        DelDevReq.m_strUserID = strUserID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelDevReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete device req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::DelDevRsp_USR DelDevRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelDevRsp))
        {
            LOG_ERROR_RLD("Delete device rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DelDevRsp.m_iRetcode;

        

        LOG_INFO_RLD("Delete device info user id is " << strUserID << " and session id is " << strSid <<
            " and return code is " << DelDevRsp.m_iRetcode <<
            " and return msg is " << DelDevRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::ModifyDevice(const std::string &strSid, const std::string &strUserID, const DeviceIf &devif, const unsigned int uiDevShared)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        unsigned int uiTypeInfo = 0xFFFFFFFF;
        if (!devif.m_strDevType.empty())
        {
            try
            {
                uiTypeInfo = boost::lexical_cast<unsigned int>(devif.m_strDevType);
            }
            catch (boost::bad_lexical_cast & e)
            {
                LOG_ERROR_RLD("Modify device type info is invalid and error msg is " << e.what() << " and input type is " << devif.m_strDevType);
                return CommMsgHandler::FAILED;
            }
            catch (...)
            {
                LOG_ERROR_RLD("Modify device type info is invalid" << " and input type is " << devif.m_strDevType);
                return CommMsgHandler::FAILED;
            }
        }

        InteractiveProtoHandler::ModifyDevReq_USR ModifyDevReq;
        ModifyDevReq.m_MsgType = InteractiveProtoHandler::MsgType::ModifyDevReq_USR_T;
        ModifyDevReq.m_uiMsgSeq = 1;
        ModifyDevReq.m_strSID = strSid;
        ModifyDevReq.m_strUserID = strUserID;
        ModifyDevReq.m_devInfo.m_strCreatedate = "";
        ModifyDevReq.m_devInfo.m_strDevID = devif.m_strDevID;
        ModifyDevReq.m_devInfo.m_strDevName = devif.m_strDevName;
        ModifyDevReq.m_devInfo.m_strDevPassword = devif.m_strDevPwd;
        ModifyDevReq.m_devInfo.m_strExtend = devif.m_strDevExtend;
        ModifyDevReq.m_devInfo.m_strInnerinfo = devif.m_strDevInnerInfo;
        ModifyDevReq.m_devInfo.m_uiStatus = 0xFFFFFFFF;
        ModifyDevReq.m_devInfo.m_uiTypeInfo = uiTypeInfo;
        ModifyDevReq.m_devInfo.m_strDomainName = devif.m_strDomainname;
        ModifyDevReq.m_devInfo.m_strP2pID = devif.m_strP2pid;
        ModifyDevReq.m_uiDeviceShared = uiDevShared;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModifyDevReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify device req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::ModifyDevRsp_USR ModifyDevRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModifyDevRsp))
        {
            LOG_ERROR_RLD("Modify device rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModifyDevRsp.m_iRetcode;

        

        LOG_INFO_RLD("Modify device is " << strUserID << " and session id is " << strSid <<
            " and return code is " << ModifyDevRsp.m_iRetcode <<
            " and return msg is " << ModifyDevRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;

}

template<typename T>
bool HttpMsgHandler::QueryDeviceInfo(const std::string &strSid, const std::string &strDevID, T &DevInfo,
    std::string &strUpdateDate, std::string &strVersion, std::string &strOnline, const unsigned int uiDevShared, const std::string &strUserID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryDevInfoReq_USR QueryDevInfoReq;
        QueryDevInfoReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryDevInfoReq_USR_T;
        QueryDevInfoReq.m_uiMsgSeq = 1;
        QueryDevInfoReq.m_strSID = strSid;
        QueryDevInfoReq.m_strValue = "";
        QueryDevInfoReq.m_strDevID = strDevID;
        QueryDevInfoReq.m_uiDeviceShared = uiDevShared;
        QueryDevInfoReq.m_strUserID = strUserID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryDevInfoReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query device info req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::QueryDevInfoRsp_USR QueryDevInfoRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryDevInfoRsp))
        {
            LOG_ERROR_RLD("Query device info rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        DevInfo = QueryDevInfoRsp.m_devInfo;
        strUpdateDate = QueryDevInfoRsp.m_strUpdateDate;
        strVersion = QueryDevInfoRsp.m_strVersion;
        strOnline = QueryDevInfoRsp.m_strOnlineStatus;

        iRet = QueryDevInfoRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query device info  return code is " << QueryDevInfoRsp.m_iRetcode <<
            " and return msg is " << QueryDevInfoRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;

}

template<typename T>
bool HttpMsgHandler::QueryDevicesOfUser(const std::string &strSid, const std::string &strUserID, const unsigned int uiBeginIndex, std::list<T> &RelationList,
    std::list<std::string> &strDevNameList)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {        
        InteractiveProtoHandler::QueryDevReq_USR QueryDevReq;
        QueryDevReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryDevReq_USR_T;
        QueryDevReq.m_uiMsgSeq = 1;
        QueryDevReq.m_strSID = strSid;
        QueryDevReq.m_strValue = "";
        QueryDevReq.m_strUserID = strUserID;
        QueryDevReq.m_uiBeginIndex = uiBeginIndex;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryDevReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query devices of user req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::QueryDevRsp_USR QueryDevRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryDevRsp))
        {
            LOG_ERROR_RLD("Query devices of user rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        RelationList.clear();
        RelationList.swap(QueryDevRsp.m_allRelationInfoList);
        strDevNameList.clear();
        strDevNameList.swap(QueryDevRsp.m_strDevNameList);

        iRet = QueryDevRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query devices of user id is " << strUserID << " and session id is " << strSid <<
            " and return code is " << QueryDevRsp.m_iRetcode <<
            " and return msg is " << QueryDevRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}


template<typename T>
bool HttpMsgHandler::QueryUsersOfDevice(const std::string &strSid, const std::string &strDevID, const unsigned int uiBeginIndex, std::list<T> &RelationList,
    std::list<std::string> &strUsrNameList)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryUserReq_USR QueryUserReq;
        QueryUserReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryUserReq_USR_T;
        QueryUserReq.m_uiMsgSeq = 1;
        QueryUserReq.m_strSID = strSid;
        QueryUserReq.m_strValue = "";
        QueryUserReq.m_strDevID = strDevID;
        QueryUserReq.m_uiBeginIndex = uiBeginIndex;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryUserReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query users of device req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);
        
        InteractiveProtoHandler::QueryUserRsp_USR QueryUserRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryUserRsp))
        {
            LOG_ERROR_RLD("Query users of device rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        RelationList.clear();
        RelationList.swap(QueryUserRsp.m_allRelationInfoList);
        strUsrNameList.clear();
        strUsrNameList.swap(QueryUserRsp.m_strUserNameList);

        iRet = QueryUserRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query users of device id is " << strDevID << " and session id is " << strSid <<
            " and return code is " << QueryUserRsp.m_iRetcode <<
            " and return msg is " << QueryUserRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
    
}

bool HttpMsgHandler::SharingDevice(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, 
    const std::string &strRelation, const std::string &strBeginDate, const std::string &strEndDate)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        unsigned int uiRelation = 0;
        if (!strRelation.empty())
        {
            try
            {
                uiRelation = boost::lexical_cast<unsigned int>(strRelation);
            }
            catch (boost::bad_lexical_cast & e)
            {
                LOG_ERROR_RLD("Sharing device relation info is invalid and error msg is " << e.what() << " and input relation is " << strRelation);
                return CommMsgHandler::FAILED;
            }
            catch (...)
            {
                LOG_ERROR_RLD("Sharing device relation info is invalid" << " and input relation is " << strRelation);
                return CommMsgHandler::FAILED;
            }
        }
        
        boost::regex reg0("([0-9]{4}[0-9]{2}[0-9]{2}[0-9]{2}[0-9]{2}[0-9]{2})"); //yyyyMMddHHmmss
        boost::regex reg1("([0-9]{4}[0-9]{2}[0-9]{2})"); //yyyyMMdd
        boost::regex reg2("([0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2})"); //yyyy-MM-dd HH:mm:ss
        boost::regex reg3("([0-9]{4}-[0-9]{2}-[0-9]{2})"); ////yyyy-MM-dd
        boost::regex reg4("([0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2})"); //yyyy-MM-dd  HH:mm
        
        if (!boost::regex_match(strBeginDate, reg0) && !boost::regex_match(strBeginDate, reg1) && !boost::regex_match(strBeginDate, reg2) &&
            !boost::regex_match(strBeginDate, reg3) && !boost::regex_match(strBeginDate, reg4))
        {
            LOG_ERROR_RLD("Sharing device begin date is invalid and input begin date is " << strBeginDate);
            return CommMsgHandler::FAILED;
        }

        if (!boost::regex_match(strEndDate, reg0) && !boost::regex_match(strEndDate, reg1) && !boost::regex_match(strEndDate, reg2) &&
            !boost::regex_match(strEndDate, reg3) && !boost::regex_match(strEndDate, reg4))
        {
            LOG_ERROR_RLD("Sharing device end date is invalid and input end date is " << strEndDate);
            return CommMsgHandler::FAILED;
        }
        
        InteractiveProtoHandler::SharingDevReq_USR SharingDevReq;
        SharingDevReq.m_MsgType = InteractiveProtoHandler::MsgType::SharingDevReq_USR_T;
        SharingDevReq.m_uiMsgSeq = 1;
        SharingDevReq.m_strSID = strSid;
        SharingDevReq.m_strValue = "";
        SharingDevReq.m_relationInfo.m_strBeginDate = strBeginDate;
        SharingDevReq.m_relationInfo.m_strEndDate = strEndDate;
        SharingDevReq.m_relationInfo.m_strDevID = strDevID;
        SharingDevReq.m_relationInfo.m_strUserID = strUserID;
        SharingDevReq.m_relationInfo.m_strValue = "";
        SharingDevReq.m_relationInfo.m_uiRelation = uiRelation;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(SharingDevReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Sharing device req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::SharingDevRsp_USR SharingDevRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, SharingDevRsp))
        {
            LOG_ERROR_RLD("Sharing device rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = SharingDevRsp.m_iRetcode;

        

        LOG_INFO_RLD("Sharing device info user id is " << strUserID << " and device id is " << strDevID << " and session id is " << strSid <<
            " and return code is " << SharingDevRsp.m_iRetcode <<
            " and return msg is " << SharingDevRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;

}

bool HttpMsgHandler::CancelSharedDevice(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const std::string &strRelation)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        unsigned int uiRelation = 0;
        if (!strRelation.empty())
        {
            try
            {
                uiRelation = boost::lexical_cast<unsigned int>(strRelation);
            }
            catch (boost::bad_lexical_cast & e)
            {
                LOG_ERROR_RLD("Cancel shared device relation info is invalid and error msg is " << e.what() << " and input relation is " << strRelation);
                return CommMsgHandler::FAILED;
            }
            catch (...)
            {
                LOG_ERROR_RLD("Cancel shared device relation info is invalid" << " and input relation is " << strRelation);
                return CommMsgHandler::FAILED;
            }
        }
        
        InteractiveProtoHandler::CancelSharedDevReq_USR CancelShardDevReq;
        CancelShardDevReq.m_MsgType = InteractiveProtoHandler::MsgType::CancelSharedDevReq_USR_T;
        CancelShardDevReq.m_uiMsgSeq = 1;
        CancelShardDevReq.m_strSID = strSid;
        CancelShardDevReq.m_strValue = "";
        CancelShardDevReq.m_relationInfo.m_strBeginDate = "";
        CancelShardDevReq.m_relationInfo.m_strEndDate = "";
        CancelShardDevReq.m_relationInfo.m_strDevID = strDevID;
        CancelShardDevReq.m_relationInfo.m_strUserID = strUserID;
        CancelShardDevReq.m_relationInfo.m_strValue = "";
        CancelShardDevReq.m_relationInfo.m_uiRelation = uiRelation;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(CancelShardDevReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Cancel shared device req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::CancelSharedDevRsp_USR CancelSharedDevRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, CancelSharedDevRsp))
        {
            LOG_ERROR_RLD("Cancel shared device rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = CancelSharedDevRsp.m_iRetcode;

        

        LOG_INFO_RLD("Cancel shared device info user id is " << strUserID << " and device id is " << strDevID << " and session id is " << strSid <<
            " and return code is " << CancelSharedDevRsp.m_iRetcode <<
            " and return msg is " << CancelSharedDevRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
    
}

bool HttpMsgHandler::AddFriends(const std::string &strSid, const std::string &strUserID, const std::string &strFriendID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {        
        InteractiveProtoHandler::AddFriendsReq_USR AddFriendsReq;
        AddFriendsReq.m_MsgType = InteractiveProtoHandler::MsgType::AddFriendsReq_USR_T;
        AddFriendsReq.m_uiMsgSeq = 1;
        AddFriendsReq.m_strSID = strSid;
        AddFriendsReq.m_strUserID = strUserID;
        AddFriendsReq.m_strFriendUserID = strFriendID;        

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddFriendsReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add friend req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::AddFriendsRsp_USR AddFriendsRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddFriendsRsp))
        {
            LOG_ERROR_RLD("Add friend rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = AddFriendsRsp.m_iRetcode;

        

        LOG_INFO_RLD("Add friend info user id is " << strUserID << " and friend id is " << strFriendID << " and session id is " << strSid <<
            " and return code is " << AddFriendsRsp.m_iRetcode <<
            " and return msg is " << AddFriendsRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;

}

bool HttpMsgHandler::DeleteFriends(const std::string &strSid, const std::string &strUserID, const std::string &strFriendID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::DelFriendsReq_USR DelFriendsReq;
        DelFriendsReq.m_MsgType = InteractiveProtoHandler::MsgType::DelFriendsReq_USR_T;
        DelFriendsReq.m_uiMsgSeq = 1;
        DelFriendsReq.m_strSID = strSid;
        DelFriendsReq.m_strUserID = strUserID;
        DelFriendsReq.m_strFriendUserIDList.push_back(strFriendID);

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelFriendsReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete friend req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);
        
        InteractiveProtoHandler::DelFriendsRsp_USR DelFriendsRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelFriendsRsp))
        {
            LOG_ERROR_RLD("Delete friend rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DelFriendsRsp.m_iRetcode;

        

        LOG_INFO_RLD("Delete friend info user id is " << strUserID << " and friend id is " << strFriendID << " and session id is " << strSid <<
            " and return code is " << DelFriendsRsp.m_iRetcode <<
            " and return msg is " << DelFriendsRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;

}

bool HttpMsgHandler::QueryFriends(const std::string &strSid, const std::string &strUserID, const unsigned int uiBeginIndex, std::list<std::string> &FriendList)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryFriendsReq_USR QueryFriendsReq;
        QueryFriendsReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryFriendsReq_USR_T;
        QueryFriendsReq.m_uiMsgSeq = 1;
        QueryFriendsReq.m_strSID = strSid;
        QueryFriendsReq.m_strValue = "";
        QueryFriendsReq.m_uiBeginIndex = uiBeginIndex;
        QueryFriendsReq.m_strUserID = strUserID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryFriendsReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query friend req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::QueryFriendsRsp_USR QueryFriendsRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryFriendsRsp))
        {
            LOG_ERROR_RLD("Query friend rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        FriendList.clear();
        FriendList.swap(QueryFriendsRsp.m_allFriendUserIDList);

        iRet = QueryFriendsRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query friend user id is " << strUserID << " and session id is " << strSid <<
            " and return code is " << QueryFriendsRsp.m_iRetcode <<
            " and return msg is " << QueryFriendsRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;

}

bool HttpMsgHandler::P2pInfo(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const std::string &strUserIpAddress, const unsigned int uiP2pType,
    std::string &strP2pServer, std::string &strP2pID, unsigned int &uiLease, std::string &strLicenseKey, std::string &strPushID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::P2pInfoReq_USR P2pInfoReq;
        P2pInfoReq.m_MsgType = InteractiveProtoHandler::MsgType::P2pInfoReq_USR_T;
        P2pInfoReq.m_uiMsgSeq = 1;
        P2pInfoReq.m_strSID = strSid;
        P2pInfoReq.m_strUserID = strUserID;
        P2pInfoReq.m_strDevID = strDevID;
        P2pInfoReq.m_strUserIpAddress = strUserIpAddress;
        P2pInfoReq.m_uiP2pSupplier = uiP2pType;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(P2pInfoReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("P2p info of user req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::P2pInfoRsp_USR P2pInfoRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, P2pInfoRsp))
        {
            LOG_ERROR_RLD("P2p info of user rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        strP2pServer = P2pInfoRsp.m_strP2pServer;
        strP2pID = P2pInfoRsp.m_strP2pID;
        uiLease = P2pInfoRsp.m_uiLease;
        strLicenseKey = P2pInfoRsp.m_strLicenseKey;
        strPushID = P2pInfoRsp.m_strPushID;

        iRet = P2pInfoRsp.m_iRetcode;

        

        LOG_INFO_RLD("P2p info of user id is " << strUserID << " and device id is " << strDevID << " and session id is " << P2pInfoRsp.m_strSID <<
            " and p2p server is " << strP2pServer << " and p2p id is " << strP2pID << " and lease is " << uiLease << " and license key is " << strLicenseKey <<
            " and push id is " << strPushID <<
            " and return code is " << P2pInfoRsp.m_iRetcode <<
            " and return msg is " << P2pInfoRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;

}

bool HttpMsgHandler::DeviceLogin(const DeviceLoginInfo &DevLogInfo, std::string &strSid, std::string &strValue)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::LoginReq_DEV DevLoginReq;
        DevLoginReq.m_MsgType = InteractiveProtoHandler::MsgType::LoginReq_DEV_T;
        DevLoginReq.m_uiMsgSeq = 1;
        DevLoginReq.m_strSID = "";
        DevLoginReq.m_strValue = DevLogInfo.m_strDevIpAddress; //这里Value保留值填写的内容是设备的接口ip，没有再独立定义字段
        DevLoginReq.m_strDevID = DevLogInfo.m_strDevID;
        DevLoginReq.m_strPassword = DevLogInfo.m_strDevPwd;
        DevLoginReq.m_uiDeviceType = DevLogInfo.m_uiDevType;
        DevLoginReq.m_uiP2pSupplier = DevLogInfo.m_uiP2pType;
        DevLoginReq.m_strP2pServr = DevLogInfo.m_strP2pserver;
        DevLoginReq.m_strP2pID = DevLogInfo.m_strP2pID;
        DevLoginReq.m_uiP2pBuildin = DevLogInfo.m_uiP2pidBuildin;
        DevLoginReq.m_strUserName = DevLogInfo.m_strUserName;
        DevLoginReq.m_strUserPassword = DevLogInfo.m_strUserPwd;
        DevLoginReq.m_strDistributor = DevLogInfo.m_strDistributor;
        DevLoginReq.m_strOtherProperty = DevLogInfo.m_strOtherProperty;
        DevLoginReq.m_strDomainName = DevLogInfo.m_strDomainName;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DevLoginReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Login device req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);
        
        InteractiveProtoHandler::LoginRsp_DEV DevLoginRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DevLoginRsp))
        {
            LOG_ERROR_RLD("Login device rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        strSid = DevLoginRsp.m_strSID;
        strValue = DevLoginRsp.m_strValue;
        iRet = DevLoginRsp.m_iRetcode;

        

        LOG_INFO_RLD("Login device id is " << DevLogInfo.m_strDevID << " and session id is " << DevLoginRsp.m_strSID <<
            " and value is " << strValue <<
            " and return code is " << DevLoginRsp.m_iRetcode <<
            " and return msg is " << DevLoginRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::DeviceP2pInfo(const std::string &strSid, const std::string &strDevID, const std::string &strDevIpAddress, const unsigned int uiP2pType,
    const std::string &strDomainName, std::string &strP2pServer, std::string &strP2pID, unsigned int &uiLease, std::string &strLicenseKey, std::string &strPushID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::P2pInfoReq_DEV DevP2pInfoReq;
        DevP2pInfoReq.m_MsgType = InteractiveProtoHandler::MsgType::P2pInfoReq_DEV_T;
        DevP2pInfoReq.m_uiMsgSeq = 1;
        DevP2pInfoReq.m_strSID = strSid;
        DevP2pInfoReq.m_strDevID = strDevID;
        DevP2pInfoReq.m_strDevIpAddress = strDevIpAddress;
        DevP2pInfoReq.m_uiP2pSupplier = uiP2pType;
        DevP2pInfoReq.m_strDomainName = strDomainName;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DevP2pInfoReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("P2p info of device req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);
        
        InteractiveProtoHandler::P2pInfoRsp_DEV DevP2pInfoRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DevP2pInfoRsp))
        {
            LOG_ERROR_RLD("P2p info of device rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        strP2pServer = DevP2pInfoRsp.m_strP2pServer;
        strP2pID = DevP2pInfoRsp.m_strP2pID;
        uiLease = DevP2pInfoRsp.m_uiLease;
        strLicenseKey = DevP2pInfoRsp.m_strLicenseKey;
        strPushID = DevP2pInfoRsp.m_strPushID;

        iRet = DevP2pInfoRsp.m_iRetcode;

        

        LOG_INFO_RLD("P2p info of device id is " << strDevID << " and session id is " << DevP2pInfoRsp.m_strSID <<
            " and p2p server is " << strP2pServer << " and p2p id is " << strP2pID << " and lease is " << uiLease << " and license key is " << strLicenseKey <<
            " and push id is " << strPushID <<
            " and return code is " << DevP2pInfoRsp.m_iRetcode <<
            " and return msg is " << DevP2pInfoRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::DeviceShakehand(const std::string &strSid, const std::string &strDevID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::ShakehandReq_DEV ShakehandReqDev;
        ShakehandReqDev.m_MsgType = InteractiveProtoHandler::MsgType::ShakehandReq_DEV_T;
        ShakehandReqDev.m_uiMsgSeq = 1;
        ShakehandReqDev.m_strSID = strSid;
        ShakehandReqDev.m_strValue = "";
        ShakehandReqDev.m_strDevID = strDevID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ShakehandReqDev, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Shakehand device req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::ShakehandRsp_DEV ShakehandRspDev;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ShakehandRspDev))
        {
            LOG_ERROR_RLD("Shakehand device rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ShakehandRspDev.m_iRetcode;

        

        LOG_INFO_RLD("Shakehand device id is " << strDevID << " and session id is " << strSid <<
            " and return code is " << ShakehandRspDev.m_iRetcode <<
            " and return msg is " << ShakehandRspDev.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::DeviceLogout(const std::string &strSid, const std::string &strDevID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::LogoutReq_DEV LogoutReqDev;
        LogoutReqDev.m_MsgType = InteractiveProtoHandler::MsgType::LogoutReq_DEV_T;
        LogoutReqDev.m_uiMsgSeq = 1;
        LogoutReqDev.m_strSID = strSid;
        LogoutReqDev.m_strValue = "";
        LogoutReqDev.m_strDevID = strDevID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(LogoutReqDev, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Logout device req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::LogoutRsp_DEV LogoutRspDev;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, LogoutRspDev))
        {
            LOG_ERROR_RLD("Logout device rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = LogoutRspDev.m_iRetcode;

        

        LOG_INFO_RLD("Logout device id is " << strDevID << " and session id is " << strSid <<
            " and return code is " << LogoutRspDev.m_iRetcode <<
            " and return msg is " << LogoutRspDev.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::DeviceSetProperty(const std::string &strSid, const DeviceProperty &devpt)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::ModifyDevicePropertyReq_DEV ModDevPtReq;
        ModDevPtReq.m_MsgType = InteractiveProtoHandler::MsgType::ModifyDevicePropertyReq_DEV_T;
        ModDevPtReq.m_uiMsgSeq = 1;
        ModDevPtReq.m_strSID = strSid;
        ModDevPtReq.m_uiDeviceType = devpt.m_uiDevType;
        ModDevPtReq.m_strChannelCount = devpt.m_strChannelCount;
        ModDevPtReq.m_strCorpID = devpt.m_strCorpid;
        ModDevPtReq.m_strCtrlPort = devpt.m_strCtrlport;
        ModDevPtReq.m_strDeviceID = devpt.m_strDevid;
        ModDevPtReq.m_strDeviceIP = devpt.m_strDvsip;
        ModDevPtReq.m_strDeviceName = devpt.m_strDvsname;
        ModDevPtReq.m_strDeviceStatus = devpt.m_strStatus;
        ModDevPtReq.m_strDomainName = devpt.m_strDomainName;
        ModDevPtReq.m_strMobilePort = devpt.m_strMobilePort;
        ModDevPtReq.m_strModel = devpt.m_strModel;
        ModDevPtReq.m_strPostFrequency = devpt.m_strPostfrequency;
        ModDevPtReq.m_strProtocol = devpt.m_strProtocol;
        ModDevPtReq.m_strServerIP = devpt.m_strServerIp;
        ModDevPtReq.m_strServerPort = devpt.m_strServerPort;
        ModDevPtReq.m_strTransfer = devpt.m_strTransfer;
        ModDevPtReq.m_strVersion = devpt.m_strVersion;
        ModDevPtReq.m_strWebPort = devpt.m_strWebport;
        ModDevPtReq.m_strP2pID = devpt.m_strP2pid;
        ModDevPtReq.m_strDeviceIP2 = devpt.m_strDvsip2;
        ModDevPtReq.m_doorbellParameter.m_strDoorbellName = devpt.m_strDoorbellName;
        ModDevPtReq.m_doorbellParameter.m_strSerialNumber = devpt.m_strSerialNum;
        ModDevPtReq.m_doorbellParameter.m_strDoorbellP2Pid = devpt.m_strDoorbellP2pid;
        ModDevPtReq.m_doorbellParameter.m_strBatteryCapacity = devpt.m_strBatteryCap;
        ModDevPtReq.m_doorbellParameter.m_strChargingState = devpt.m_strChargingState;
        ModDevPtReq.m_doorbellParameter.m_strWifiSignal = devpt.m_strWifiSig;
        ModDevPtReq.m_doorbellParameter.m_strVolumeLevel = devpt.m_strVolumeLevel;
        ModDevPtReq.m_doorbellParameter.m_strVersionNumber = devpt.m_strVersionNum;
        ModDevPtReq.m_doorbellParameter.m_strChannelNumber = devpt.m_strChannelNum;
        ModDevPtReq.m_doorbellParameter.m_strCodingType = devpt.m_strCodingType;
        ModDevPtReq.m_doorbellParameter.m_strPIRAlarmSwtich = devpt.m_strPirAlarmSwitch;
        ModDevPtReq.m_doorbellParameter.m_strDoorbellSwitch = devpt.m_strDoorbellSwitch;
        ModDevPtReq.m_doorbellParameter.m_strPIRAlarmLevel = devpt.m_strPirAlarmLevel;
        ModDevPtReq.m_doorbellParameter.m_strPIRIneffectiveTime = devpt.m_strPirIneffectiveTime;
        ModDevPtReq.m_doorbellParameter.m_strCurrentWifi = devpt.m_strCurrentWifi;
        ModDevPtReq.m_doorbellParameter.m_strSubCategory = devpt.m_strSubCategory;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModDevPtReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Set device property req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);
        
        InteractiveProtoHandler::ModifyDevicePropertyRsp_DEV ModDevPtRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModDevPtRsp))
        {
            LOG_ERROR_RLD("Set device property rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModDevPtRsp.m_iRetcode;

        

        LOG_INFO_RLD("Set device property and  device id is " << devpt.m_strDevid << " and session id is " << strSid <<
            " and return code is " << ModDevPtRsp.m_iRetcode <<
            " and return msg is " << ModDevPtRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

template<typename T>
bool HttpMsgHandler::QueryUserFile(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const unsigned int uiBeginIndex, 
    const std::string &strBeginDate, const std::string &strEndDate, const unsigned int uiBussinessType, std::list<T> &FileList)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryFileReq_USR QueryUserFileReq;
        QueryUserFileReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryFileReq_USR_T;
        QueryUserFileReq.m_uiMsgSeq = 1;
        QueryUserFileReq.m_strSID = strSid;
        QueryUserFileReq.m_strValue = "";
        QueryUserFileReq.m_strDevID = strDevID;
        QueryUserFileReq.m_uiBeginIndex = uiBeginIndex;
        QueryUserFileReq.m_strUserID = strUserID;
        QueryUserFileReq.m_strBeginDate = strBeginDate;
        QueryUserFileReq.m_strEndDate = strEndDate;
        QueryUserFileReq.m_uiBusinessType = uiBussinessType;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryUserFileReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query files of user req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::QueryFileRsp_USR QueryUserFileRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryUserFileRsp))
        {
            LOG_ERROR_RLD("Query files of user rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        FileList.clear();
        FileList.swap(QueryUserFileRsp.m_fileList);

        iRet = QueryUserFileRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query files of user and user id is " << strUserID << " and device id is " << strDevID << 
            " and begin index is " << uiBeginIndex <<
            " and session id is " << strSid <<
            " and return code is " << QueryUserFileRsp.m_iRetcode <<
            " and return msg is " << QueryUserFileRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
    
}

bool HttpMsgHandler::DownloadUserFile(const std::string &strSid, const std::string &strUserID, const std::string &strFileID, std::string &strFileUrl)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::DownloadFileReq_USR DownloadUserFileReq;
        DownloadUserFileReq.m_MsgType = InteractiveProtoHandler::MsgType::DownloadFileReq_USR_T;
        DownloadUserFileReq.m_uiMsgSeq = 1;
        DownloadUserFileReq.m_strSID = strSid;
        DownloadUserFileReq.m_strUserID = strUserID;
        DownloadUserFileReq.m_strFileIDList.push_back(strFileID);

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DownloadUserFileReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Download files of user req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::DownloadFileRsp_USR DownloadUserFileRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DownloadUserFileRsp))
        {
            LOG_ERROR_RLD("Download files of user rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        if (!DownloadUserFileRsp.m_fileUrlList.empty())
        {
            auto FileUrl = DownloadUserFileRsp.m_fileUrlList.front();
            strFileUrl = FileUrl.m_strDownloadUrl;
        }
        
        iRet = DownloadUserFileRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query files of user and user id is " << strUserID <<
            " and file url is " << strFileUrl <<
            " and session id is " << strSid <<
            " and return code is " << DownloadUserFileRsp.m_iRetcode <<
            " and return msg is " << DownloadUserFileRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::DeleteUserFile(const std::string &strSid, const std::string &strUserID, const std::string &strFileID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::DeleteFileReq_USR DeleteUserFileReq;
        DeleteUserFileReq.m_MsgType = InteractiveProtoHandler::MsgType::DeleteFileReq_USR_T;
        DeleteUserFileReq.m_uiMsgSeq = 1;
        DeleteUserFileReq.m_strSID = strSid;
        DeleteUserFileReq.m_strUserID = strUserID;
        DeleteUserFileReq.m_strFileIDList.push_back(strFileID);

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DeleteUserFileReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete files of user req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::DeleteFileRsp_USR DeleteUserFileRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DeleteUserFileRsp))
        {
            LOG_ERROR_RLD("Delete files of user rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DeleteUserFileRsp.m_iRetcode;

        

        LOG_INFO_RLD("Delete files of user and user id is " << strUserID <<
            " and file id is " << strFileID <<
            " and session id is " << strSid <<
            " and return code is " << DeleteUserFileRsp.m_iRetcode <<
            " and return msg is " << DeleteUserFileRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;

}

bool HttpMsgHandler::AddDeviceFile(const std::string &strDevID, const std::string &strRemoteFileID, const std::string &strDownloadUrl, 
    const std::string &strFileName, const std::string &strSuffixName, const unsigned long int uiFileSize, 
    const std::string &strFileCreatedate, const std::string &strExtend, const unsigned int uiBussinessType)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::AddFileReq_DEV AddDevFileReq;
        AddDevFileReq.m_MsgType = InteractiveProtoHandler::MsgType::AddFileReq_DEV_T;
        AddDevFileReq.m_uiMsgSeq = 1;
        AddDevFileReq.m_strSID = "";
        AddDevFileReq.m_strDevID = strDevID;
        AddDevFileReq.m_strValue = "";

        InteractiveProtoHandler::File FileInfo;
        FileInfo.m_strFileCreatedate = strFileCreatedate;
        FileInfo.m_strDevID = strDevID;
        FileInfo.m_strDownloadUrl = strDownloadUrl;
        FileInfo.m_strExtend = strExtend;
        FileInfo.m_strFileName = strFileName;
        FileInfo.m_strRemoteFileID = strRemoteFileID;
        FileInfo.m_strSuffixName = strSuffixName;
        FileInfo.m_ulFileSize = uiFileSize;
        FileInfo.m_uiBusinessType = uiBussinessType;
        
        AddDevFileReq.m_fileList.push_back(FileInfo);

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddDevFileReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add device file req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);
        
        InteractiveProtoHandler::AddFileRsp_DEV AddDevFileRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddDevFileRsp))
        {
            LOG_ERROR_RLD("Add device file rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = AddDevFileRsp.m_iRetcode;

        

        LOG_INFO_RLD("Add device file device id is " << strDevID <<
            " and file remote id is " << strRemoteFileID <<
            " and return code is " << AddDevFileRsp.m_iRetcode <<
            " and return msg is " << AddDevFileRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::RetrievePwd(const std::string &strUserName, const std::string &strEmail, const unsigned int uiType)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::RetrievePwdReq_USR RetrievePwdReq;
        RetrievePwdReq.m_MsgType = InteractiveProtoHandler::MsgType::RetrievePwdReq_USR_T;
        RetrievePwdReq.m_uiMsgSeq = 1;
        RetrievePwdReq.m_strSID = "";
        RetrievePwdReq.m_strEmail = strEmail;
        RetrievePwdReq.m_strUserName = strUserName;
        RetrievePwdReq.m_uiAppType = uiType;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(RetrievePwdReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Retrieve pwd req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);
        
        InteractiveProtoHandler::RetrievePwdRsp_USR RetrievePwdRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, RetrievePwdRsp))
        {
            LOG_ERROR_RLD("Retrieve pwd rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = RetrievePwdRsp.m_iRetcode;

        

        LOG_INFO_RLD("Retrieve pwd info user name is " << strUserName << " and email is " << strEmail <<
            " and return code is " << RetrievePwdRsp.m_iRetcode <<
            " and return msg is " << RetrievePwdRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;

}

bool HttpMsgHandler::DeviceQueryTimeZone(const std::string &strSid, const std::string &strDevID, const std::string &strDevIpAddress, std::string &strCountrycode,
    std::string &strCountryNameEn, std::string &strCountryNameZh, std::string &strTimeZone)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryTimeZoneReq_DEV QueryTimeZoneInfoReq;
        QueryTimeZoneInfoReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryTimeZoneReq_DEV_T;
        QueryTimeZoneInfoReq.m_uiMsgSeq = 1;
        QueryTimeZoneInfoReq.m_strSID = strSid;
        QueryTimeZoneInfoReq.m_strDevID = strDevID;
        QueryTimeZoneInfoReq.m_strDevIpAddress = strDevIpAddress;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryTimeZoneInfoReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query timezone info of device req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);
        
        InteractiveProtoHandler::QueryTimeZoneRsp_DEV QueryTimeZoneInfoInfoRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryTimeZoneInfoInfoRsp))
        {
            LOG_ERROR_RLD("Query timezone info of device rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        strCountrycode = QueryTimeZoneInfoInfoRsp.m_strCountryCode;
        strCountryNameEn = QueryTimeZoneInfoInfoRsp.m_strCountryNameEn;
        strCountryNameZh = QueryTimeZoneInfoInfoRsp.m_strCountryNameZh;
        strTimeZone = QueryTimeZoneInfoInfoRsp.m_strTimeZone;
        
        iRet = QueryTimeZoneInfoInfoRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query timezone info of device id is " << strDevID << " and device ip is " << strDevIpAddress << 
            " and country code is " << strCountrycode << " and country name en is " << QueryTimeZoneInfoInfoRsp.m_strCountryNameEn << 
            " and country name zh is " << QueryTimeZoneInfoInfoRsp.m_strCountryNameZh <<
            " and return code is " << QueryTimeZoneInfoInfoRsp.m_iRetcode <<
            " and return msg is " << QueryTimeZoneInfoInfoRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::UserQueryAccessDomainName(const std::string &strIpAddress, const std::string &strUserName, std::string &strAccessDomainName, std::string &strLease)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryAccessDomainNameReq_USR QueryDomainReq;
        QueryDomainReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryAccessDomainNameReq_USR_T;
        QueryDomainReq.m_uiMsgSeq = 1;
        QueryDomainReq.m_strSID = "";
        QueryDomainReq.m_strValue = "";
        QueryDomainReq.m_strUserIpAddress = strIpAddress;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryDomainReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query user access domain req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::QueryAccessDomainNameRsp_USR QueryDomainRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryDomainRsp))
        {
            LOG_ERROR_RLD("Query user access domain rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        strAccessDomainName = QueryDomainRsp.m_strDomainName;
        strLease = boost::lexical_cast<std::string>(QueryDomainRsp.m_uiLease);
        iRet = QueryDomainRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query user access domain and user name is " << strUserName << " and user ip is " << strIpAddress <<
            " and domain name is " << strAccessDomainName <<
            " and return code is " << QueryDomainRsp.m_iRetcode <<
            " and return msg is " << QueryDomainRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::DeviceQueryAccessDomainName(const std::string &strIpAddress, const std::string &strDevID, std::string &strAccessDomainName, std::string &strLease)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryAccessDomainNameReq_DEV QueryDomainReq;
        QueryDomainReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryAccessDomainNameReq_DEV_T;
        QueryDomainReq.m_uiMsgSeq = 1;
        QueryDomainReq.m_strSID = "";
        QueryDomainReq.m_strValue = "";
        QueryDomainReq.m_strDevIpAddress = strIpAddress;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryDomainReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query device access domain req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);
        
        InteractiveProtoHandler::QueryAccessDomainNameRsp_DEV QueryDomainRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryDomainRsp))
        {
            LOG_ERROR_RLD("Query device access domain rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        strAccessDomainName = QueryDomainRsp.m_strDomainName;
        strLease = boost::lexical_cast<std::string>(QueryDomainRsp.m_uiLease);
        iRet = QueryDomainRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query device access domain and device id is " << strDevID << " and device ip is " << strIpAddress <<
            " and domain name is " << strAccessDomainName <<
            " and return code is " << QueryDomainRsp.m_iRetcode <<
            " and return msg is " << QueryDomainRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::DeviceQueryUpdateService(const std::string &strSid, const std::string &strIpAddress, const std::string &strDevID, std::string &strUpdateAddress, std::string &strLease)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryUpgradeSiteReq_DEV QueryUpdateServiceReq;
        QueryUpdateServiceReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryUpgradeSiteReq_DEV_T;
        QueryUpdateServiceReq.m_uiMsgSeq = 1;
        QueryUpdateServiceReq.m_strSID = strSid;
        QueryUpdateServiceReq.m_strDevID = strDevID;
        QueryUpdateServiceReq.m_strDevIpAddress = strIpAddress;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryUpdateServiceReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query device update address req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);
        
        InteractiveProtoHandler::QueryUpgradeSiteRsp_DEV QueryUpdateServiceRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryUpdateServiceRsp))
        {
            LOG_ERROR_RLD("Query device update service rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        strUpdateAddress = QueryUpdateServiceRsp.m_strUpgradeSiteUrl;
        strLease = boost::lexical_cast<std::string>(QueryUpdateServiceRsp.m_uiLease);
        iRet = QueryUpdateServiceRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query device update address and device id is " << strDevID << " and device ip is " << strIpAddress <<
            " and update address is " << strUpdateAddress <<
            " and return code is " << QueryUpdateServiceRsp.m_iRetcode <<
            " and return msg is " << QueryUpdateServiceRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::AddConfiguration(const std::string &strCategory, const std::string &strSubcategory, const std::string &strLatestVersion, 
    const std::string &strDesc, const std::string &strForceVersion, const std::string &strServerAddress, const std::string &strFilename, const std::string &strFileID,
    const unsigned int uiFileSize, const unsigned int uiLease)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
        std::string::size_type pos = strCurrentTime.find('T');
        strCurrentTime.replace(pos, 1, std::string(" "));

        InteractiveProtoHandler::AddConfigurationReq_MGR AddCfgReq;
        AddCfgReq.m_MsgType = InteractiveProtoHandler::MsgType::AddConfigurationReq_MGR_T;
        AddCfgReq.m_uiMsgSeq = 1;
        AddCfgReq.m_strSID = "";
        AddCfgReq.m_configuration.m_strCategory = strCategory;
        AddCfgReq.m_configuration.m_strDescription = strDesc;
        AddCfgReq.m_configuration.m_strFileID = strFileID;
        AddCfgReq.m_configuration.m_strFileName = strFilename;
        AddCfgReq.m_configuration.m_strForceVersion = strForceVersion;
        AddCfgReq.m_configuration.m_strLatestVersion = strLatestVersion;
        AddCfgReq.m_configuration.m_strSubCategory = strSubcategory;
        AddCfgReq.m_configuration.m_strUpdateDate = strCurrentTime;
        AddCfgReq.m_configuration.m_uiFileSize = uiFileSize;
        AddCfgReq.m_configuration.m_uiLeaseDuration = uiLease;
        AddCfgReq.m_configuration.m_uiStatus = 0;
        AddCfgReq.m_configuration.m_strServerAddress = strServerAddress;

        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddCfgReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add configuration req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::AddConfigurationRsp_MGR AddCfgRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddCfgRsp))
        {
            LOG_ERROR_RLD("Add configuration rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = AddCfgRsp.m_iRetcode;
        
        

        LOG_INFO_RLD("Add configuration and category is " << strCategory << " and subcategory is " << strSubcategory <<
            " and return code is " << AddCfgRsp.m_iRetcode <<
            " and return msg is " << AddCfgRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::DeleteConfiguration(const std::string &strCategory, const std::string &strSubcategory)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::DeleteConfigurationReq_MGR DelCfgReq;
        DelCfgReq.m_MsgType = InteractiveProtoHandler::MsgType::DeleteConfigurationReq_MGR_T;
        DelCfgReq.m_uiMsgSeq = 1;
        DelCfgReq.m_strSID = "";
        DelCfgReq.m_strCategory = strCategory;
        DelCfgReq.m_strSubCategory = strSubcategory;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelCfgReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete configuration req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);
        
        InteractiveProtoHandler::DeleteConfigurationRsp_MGR DelCfgRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelCfgRsp))
        {
            LOG_ERROR_RLD("Delete configuration rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DelCfgRsp.m_iRetcode;

        

        LOG_INFO_RLD("Delete configuration and category is " << strCategory << " and subcategory is " << strSubcategory <<
            " and return code is " << DelCfgRsp.m_iRetcode <<
            " and return msg is " << DelCfgRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::ModifyConfiguration(const std::string &strCategory, const std::string &strSubcategory, const std::string &strLatestVersion, const std::string &strDesc, const std::string &strForceVersion, const std::string &strServerAddress, const std::string &strFilename, const std::string &strFileID, const unsigned int uiFileSize, const unsigned int uiLease)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
        std::string::size_type pos = strCurrentTime.find('T');
        strCurrentTime.replace(pos, 1, std::string(" "));

        InteractiveProtoHandler::ModifyConfigurationReq_MGR ModCfgReq;
        ModCfgReq.m_MsgType = InteractiveProtoHandler::MsgType::ModifyConfigurationReq_MGR_T;
        ModCfgReq.m_uiMsgSeq = 1;
        ModCfgReq.m_strSID = "";
        ModCfgReq.m_configuration.m_strCategory = strCategory;
        ModCfgReq.m_configuration.m_strDescription = strDesc;
        ModCfgReq.m_configuration.m_strFileID = strFileID;
        ModCfgReq.m_configuration.m_strFileName = strFilename;
        ModCfgReq.m_configuration.m_strForceVersion = strForceVersion;
        ModCfgReq.m_configuration.m_strLatestVersion = strLatestVersion;
        ModCfgReq.m_configuration.m_strSubCategory = strSubcategory;
        ModCfgReq.m_configuration.m_strUpdateDate = strCurrentTime;
        ModCfgReq.m_configuration.m_uiFileSize = uiFileSize;
        ModCfgReq.m_configuration.m_uiLeaseDuration = uiLease;
        ModCfgReq.m_configuration.m_uiStatus = 0;
        ModCfgReq.m_configuration.m_strServerAddress = strServerAddress;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModCfgReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify configuration req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::ModifyConfigurationRsp_MGR ModCfgRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModCfgRsp))
        {
            LOG_ERROR_RLD("Modify configuration rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModCfgRsp.m_iRetcode;

        

        LOG_INFO_RLD("Modify configuration and category is " << strCategory << " and subcategory is " << strSubcategory <<
            " and return code is " << ModCfgRsp.m_iRetcode <<
            " and return msg is " << ModCfgRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

template<typename T>
bool HttpMsgHandler::QueryConfiguration(const unsigned int uiBeginIndex, std::list<T> &CfgList)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryAllConfigurationReq_MGR QueryCfgReq;
        QueryCfgReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryAllConfigurationReq_MGR_T;
        QueryCfgReq.m_uiMsgSeq = 1;
        QueryCfgReq.m_strSID = "";
        QueryCfgReq.m_uiBeginIndex = uiBeginIndex;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryCfgReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query all config req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::QueryAllConfigurationRsp_MGR QueryCfgRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryCfgRsp))
        {
            LOG_ERROR_RLD("Query all config rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        CfgList.clear();
        CfgList.swap(QueryCfgRsp.m_configurationList);

        iRet = QueryCfgRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query all config and begin index is " << uiBeginIndex <<
            " and return code is " << QueryCfgRsp.m_iRetcode <<
            " and return msg is " << QueryCfgRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;

}

bool HttpMsgHandler::QueryUploadURL(std::string &strURL)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryUploadURLReq_MGR QueryUploadURLReq;
        QueryUploadURLReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryUploadURLReq_MGR_T;
        QueryUploadURLReq.m_uiMsgSeq = 1;
        QueryUploadURLReq.m_strSID = "";
        QueryUploadURLReq.m_strValue = "";
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryUploadURLReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query upload url req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::QueryUploadURLRsp_MGR QueryUploadURLRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryUploadURLRsp))
        {
            LOG_ERROR_RLD("Query upload url rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        strURL = QueryUploadURLRsp.m_strUploadURL;

        iRet = QueryUploadURLRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query upload url is " << strURL <<
            " and return code is " << QueryUploadURLRsp.m_iRetcode <<
            " and return msg is " << QueryUploadURLRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::QueryAppUpgrade(const std::string &strCategory, const std::string &strSubcategory, const std::string &strCurrentVersion, 
    std::string &strNewVersionValid, std::string &strAppName, std::string &strAppPath, unsigned int &uiAppSize, 
    std::string &strNewVersion, std::string &strDesc, std::string &strForceUpgrade, std::string &strUpdateDate)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryAppUpgradeReq_USR QueryAppVerReq;
        QueryAppVerReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryAppUpgradeReq_USR_T;
        QueryAppVerReq.m_uiMsgSeq = 1;
        QueryAppVerReq.m_strSID = "";
        QueryAppVerReq.m_strCategory = strCategory;
        QueryAppVerReq.m_strSubCategory = strSubcategory;
        QueryAppVerReq.m_strCurrentVersion = strCurrentVersion;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryAppVerReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query app version req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::QueryAppUpgradeRsp_USR QueryAppVerRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryAppVerRsp))
        {
            LOG_ERROR_RLD("Query app version rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        strNewVersionValid = boost::lexical_cast<std::string>(QueryAppVerRsp.m_appUpgrade.m_uiNewVersionValid);
        strAppName = QueryAppVerRsp.m_appUpgrade.m_strAppName;
        strAppPath = QueryAppVerRsp.m_appUpgrade.m_strAppPath;
        uiAppSize = QueryAppVerRsp.m_appUpgrade.m_uiAppSize;
        strNewVersion = QueryAppVerRsp.m_appUpgrade.m_strVersion;
        strDesc = QueryAppVerRsp.m_appUpgrade.m_strDescription;
        strForceUpgrade = boost::lexical_cast<std::string>(QueryAppVerRsp.m_appUpgrade.m_uiForceUpgrade);
        strUpdateDate = QueryAppVerRsp.m_appUpgrade.m_strUpdateDate;

        iRet = QueryAppVerRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query app version and new version vaild is  " << strNewVersionValid << " and app name is " << strAppName <<
            " and app path " << strAppPath << " and app size is " << uiAppSize << " and new version is " << strNewVersion <<
            " and description is " << strDesc << " and force upgrade is " << strForceUpgrade << " and update date is " << strUpdateDate <<
            " and return code is " << QueryAppVerRsp.m_iRetcode <<
            " and return msg is " << QueryAppVerRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::QueryDevUpgrade(const std::string &strCategory, const std::string &strSubcategory, const std::string &strCurrentVersion, 
    const std::string &strDevID,
    std::string &strNewVersionValid, std::string &strFirmwareName, std::string &strFirmwarePath, unsigned int &uiFirmwareSize, 
    std::string &strNewVersion, std::string &strDesc, std::string &strForceUpgrade, std::string &strUpdateDate)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryFirmwareUpgradeReq_DEV QueryDevVerReq;
        QueryDevVerReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryFirmwareUpgradeReq_DEV_T;
        QueryDevVerReq.m_uiMsgSeq = 1;
        QueryDevVerReq.m_strSID = "";
        QueryDevVerReq.m_strCategory = strCategory;
        QueryDevVerReq.m_strSubCategory = strSubcategory;
        QueryDevVerReq.m_strCurrentVersion = strCurrentVersion;
        QueryDevVerReq.m_strDeviceID = strDevID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryDevVerReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query device firmware version req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::QueryFirmwareUpgradeRsp_DEV QueryDevVerRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryDevVerRsp))
        {
            LOG_ERROR_RLD("Query device firmware version rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        strNewVersionValid = boost::lexical_cast<std::string>(QueryDevVerRsp.m_firmwareUpgrade.m_uiNewVersionValid);
        strFirmwareName = QueryDevVerRsp.m_firmwareUpgrade.m_strFirmwareName;
        strFirmwarePath = QueryDevVerRsp.m_firmwareUpgrade.m_strFirmwarePath;
        uiFirmwareSize = QueryDevVerRsp.m_firmwareUpgrade.m_uiFirmwareSize;
        strNewVersion = QueryDevVerRsp.m_firmwareUpgrade.m_strVersion;
        strDesc = QueryDevVerRsp.m_firmwareUpgrade.m_strDescription;
        strForceUpgrade = boost::lexical_cast<std::string>(QueryDevVerRsp.m_firmwareUpgrade.m_uiForceUpgrade);
        strUpdateDate = QueryDevVerRsp.m_firmwareUpgrade.m_strUpdateDate;

        iRet = QueryDevVerRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query device firmware version and new version vaild is  " << strNewVersionValid << " and firmware name is " << strFirmwareName <<
            " and firmware path " << strFirmwarePath << " and firmware size is " << uiFirmwareSize << " and new version is " << strNewVersion <<
            " and description is " << strDesc << " and force upgrade is " << strForceUpgrade << " and update date is " << strUpdateDate <<
            " and return code is " << QueryDevVerRsp.m_iRetcode <<
            " and return msg is " << QueryDevVerRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::QueryDevParam(const std::string &strSid, const std::string &strDevID, const unsigned int uiDevType, const std::string &strQueryType, DeviceProperty &devpt)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryDeviceParameterReq_DEV QueryDevParamReq;
        QueryDevParamReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryDeviceParameterReq_DEV_T;
        QueryDevParamReq.m_uiMsgSeq = 1;
        QueryDevParamReq.m_strSID = strSid;
        QueryDevParamReq.m_strDeviceID = strDevID;
        QueryDevParamReq.m_strQueryType = strQueryType;
        QueryDevParamReq.m_uiDeviceType = uiDevType;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryDevParamReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query device param req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);
        
        InteractiveProtoHandler::QueryDeviceParameterRsp_DEV QueryDevParamRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryDevParamRsp))
        {
            LOG_ERROR_RLD("Query device param rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        if (0 == uiDevType) //门铃设备
        {
            devpt.m_strDoorbellName = QueryDevParamRsp.m_doorbellParameter.m_strDoorbellName;
            devpt.m_strVolumeLevel = QueryDevParamRsp.m_doorbellParameter.m_strVolumeLevel;
            devpt.m_strPirAlarmSwitch = QueryDevParamRsp.m_doorbellParameter.m_strPIRAlarmSwtich;
            devpt.m_strDoorbellSwitch = QueryDevParamRsp.m_doorbellParameter.m_strDoorbellSwitch;
            devpt.m_strPirAlarmLevel = QueryDevParamRsp.m_doorbellParameter.m_strPIRAlarmLevel;
            devpt.m_strPirIneffectiveTime = QueryDevParamRsp.m_doorbellParameter.m_strPIRIneffectiveTime;

            if (strQueryType == "all")
            {
                devpt.m_strSerialNum = QueryDevParamRsp.m_doorbellParameter.m_strSerialNumber;
                devpt.m_strDoorbellP2pid = QueryDevParamRsp.m_doorbellParameter.m_strDoorbellP2Pid;
                devpt.m_strBatteryCap = QueryDevParamRsp.m_doorbellParameter.m_strBatteryCapacity;
                devpt.m_strChargingState = QueryDevParamRsp.m_doorbellParameter.m_strChargingState;
                devpt.m_strWifiSig = QueryDevParamRsp.m_doorbellParameter.m_strWifiSignal;
                devpt.m_strVersionNum = QueryDevParamRsp.m_doorbellParameter.m_strVersionNumber;
                devpt.m_strChannelNum = QueryDevParamRsp.m_doorbellParameter.m_strChannelNumber;
                devpt.m_strCodingType = QueryDevParamRsp.m_doorbellParameter.m_strCodingType;
                devpt.m_strCurrentWifi = QueryDevParamRsp.m_doorbellParameter.m_strCurrentWifi;
                devpt.m_strSubCategory = QueryDevParamRsp.m_doorbellParameter.m_strSubCategory;
            }

            LOG_INFO_RLD("Query device param info and doorbell name is " << devpt.m_strDoorbellName << " and volume level is " << devpt.m_strVolumeLevel <<
                " and pir alarm switch is " << devpt.m_strPirAlarmSwitch << " and doorbell switch is " << devpt.m_strDoorbellSwitch <<
                " and pir alarm level is " << devpt.m_strPirAlarmLevel << " and pir ineffective time is " << devpt.m_strPirIneffectiveTime <<
                " and return code is " << QueryDevParamRsp.m_iRetcode <<
                " and return msg is " << QueryDevParamRsp.m_strRetMsg);
        }
        
        iRet = QueryDevParamRsp.m_iRetcode;

        

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::CheckDeviceP2pid(const std::string &strP2pid, const unsigned int uiP2pType)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryIfP2pIDValidReq_USR QueryP2pidValidReq;
        QueryP2pidValidReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryIfP2pIDValidReq_USR_T;
        QueryP2pidValidReq.m_uiMsgSeq = 1;
        QueryP2pidValidReq.m_strSID = "";
        QueryP2pidValidReq.m_strP2pID = strP2pid;
        QueryP2pidValidReq.m_uiSuppliser = uiP2pType;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryP2pidValidReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Check device p2p id req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::QueryIfP2pIDValidRsp_USR QueryP2pidValidRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryP2pidValidRsp))
        {
            LOG_ERROR_RLD("Check device p2p id rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }
        
        iRet = QueryP2pidValidRsp.m_iRetcode;

        

        LOG_INFO_RLD("Check device p2p id info" << " and return code is " << QueryP2pidValidRsp.m_iRetcode <<
            " and return msg is " << QueryP2pidValidRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::QueryPushStatus(const std::string &strDevID, std::string &strPushStatus)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryPlatformPushStatusReq_DEV QueryPushStatusReq;
        QueryPushStatusReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryPlatformPushStatusReq_DEV_T;
        QueryPushStatusReq.m_uiMsgSeq = 1;
        QueryPushStatusReq.m_strSID = "";
        QueryPushStatusReq.m_strDeviceID = strDevID;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryPushStatusReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query push status req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::QueryPlatformPushStatusRsp_DEV QueryPushStatusRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryPushStatusRsp))
        {
            LOG_ERROR_RLD("Query push status rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        strPushStatus = QueryPushStatusRsp.m_strStatus;

        iRet = QueryPushStatusRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query push status info and device id is " << strDevID << " and push status is " << strPushStatus <<
            " and return code is " << QueryPushStatusRsp.m_iRetcode <<
            " and return msg is " << QueryPushStatusRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::DeviceEventReport(const std::string &strSid, Event &ev)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::DeviceEventReportReq_DEV DevEventReportReq;
        DevEventReportReq.m_MsgType = InteractiveProtoHandler::MsgType::DeviceEventReportReq_DEV_T;
        DevEventReportReq.m_uiMsgSeq = 1;
        DevEventReportReq.m_strSID = strSid;
        DevEventReportReq.m_strDeviceID = ev.m_strDevID;
        DevEventReportReq.m_strFileID = ev.m_strFileID;
        DevEventReportReq.m_uiDeviceType = ev.m_uiDevType;
        DevEventReportReq.m_uiEventState = ev.m_uiEventStatus;
        DevEventReportReq.m_uiEventType = ev.m_uiEventType;
        DevEventReportReq.m_strEventTime = ev.m_strEventTime;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DevEventReportReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Device event report req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::DeviceEventReportRsp_DEV DevEventReportRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DevEventReportRsp))
        {
            LOG_ERROR_RLD("Device event report rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        ev.m_strEventID = DevEventReportRsp.m_strEventID;

        iRet = DevEventReportRsp.m_iRetcode;

        

        LOG_INFO_RLD("Device event report info and device id is " << ev.m_strDevID << " and event id is " << ev.m_strDevID <<
            " and return code is " << DevEventReportRsp.m_iRetcode <<
            " and return msg is " << DevEventReportRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::QueryDeviceEvent(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const unsigned int uiDevShared, 
    const unsigned int uiEventType, const unsigned int uiView, const unsigned int uiBeginIndex, const std::string &strBeginDate, const std::string &strEndDate, 
    std::list<Event> &evlist)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryAllDeviceEventReq_USR QueryDevEventReportReq;
        QueryDevEventReportReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryAllDeviceEventReq_USR_T;
        QueryDevEventReportReq.m_uiMsgSeq = 1;
        QueryDevEventReportReq.m_strSID = strSid;
        QueryDevEventReportReq.m_strDeviceID = strDevID;
        QueryDevEventReportReq.m_strUserID = strUserID;
        QueryDevEventReportReq.m_uiBeginIndex = uiBeginIndex;
        QueryDevEventReportReq.m_uiDeviceShared = uiDevShared;
        QueryDevEventReportReq.m_uiEventType = uiEventType;
        QueryDevEventReportReq.m_uiReadState = uiView;
        QueryDevEventReportReq.m_strBeginDate = strBeginDate;
        QueryDevEventReportReq.m_strEndDate = strEndDate;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QueryDevEventReportReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query device event report req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::QueryAllDeviceEventRsp_USR QueryDevEventReportRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryDevEventReportRsp))
        {
            LOG_ERROR_RLD("Query device event report rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        evlist.clear();               
        for (auto it : QueryDevEventReportRsp.m_deviceEventList)
        {
            Event ev;
            ev.m_strDevID = it.m_strDeviceID;
            ev.m_strEventID = it.m_strEventID;
            ev.m_strFileURL = it.m_strFileUrl;
            ev.m_uiDevType = it.m_uiDeviceType;
            ev.m_uiEventStatus = it.m_uiEventState;
            ev.m_uiEventType = it.m_uiEventType;
            ev.m_strEventTime = it.m_strEventTime;
            evlist.push_back(std::move(ev));
        }

        iRet = QueryDevEventReportRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query device event report info and sid id is " << strSid << " and user id is " << strUserID <<
            " and device id is " << strDevID << " and device shared is " << uiDevShared << " event type is " << uiEventType <<
            " and view is " << uiView << " and begin index is " << uiBeginIndex << " and begin date is " << strBeginDate <<
            " and end date is " << strEndDate <<
            " and return code is " << QueryDevEventReportRsp.m_iRetcode <<
            " and return msg is " << QueryDevEventReportRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::DeleteDeviceEvent(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, const std::string &strEventID)
{
    //
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::DeleteDeviceEventReq_USR DelDevEventReportReq;
        DelDevEventReportReq.m_MsgType = InteractiveProtoHandler::MsgType::DeleteDeviceEventReq_USR_T;
        DelDevEventReportReq.m_uiMsgSeq = 1;
        DelDevEventReportReq.m_strSID = strSid;
        DelDevEventReportReq.m_strDeviceID = strDevID;
        DelDevEventReportReq.m_strUserID = strUserID;
        DelDevEventReportReq.m_strEventID = strEventID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelDevEventReportReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete device event req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::DeleteDeviceEventRsp_USR DelDevEventReportRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelDevEventReportRsp))
        {
            LOG_ERROR_RLD("Delete device event rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }
        
        iRet = DelDevEventReportRsp.m_iRetcode;

        

        LOG_INFO_RLD("Delete device event info and sid id is " << strSid << " and user id is " << strUserID <<
            " and device id is " << strDevID << " and event id is " << strEventID <<
            " and return code is " << DelDevEventReportRsp.m_iRetcode <<
            " and return msg is " << DelDevEventReportRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::ModifyDeviceEvent(const std::string &strSid, const Event &ev)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::ModifyDeviceEventReq_USR ModDevEventReportReq;
        ModDevEventReportReq.m_MsgType = InteractiveProtoHandler::MsgType::ModifyDeviceEventReq_USR_T;
        ModDevEventReportReq.m_uiMsgSeq = 1;
        ModDevEventReportReq.m_strSID = strSid;
        ModDevEventReportReq.m_strDeviceID = ev.m_strDevID;
        ModDevEventReportReq.m_strEventID = ev.m_strEventID;
        ModDevEventReportReq.m_uiEventState = ev.m_uiEventStatus;
        ModDevEventReportReq.m_uiEventType = ev.m_uiEventType;
        ModDevEventReportReq.m_strUpdateTime = ev.m_strEventTime;
        ModDevEventReportReq.m_strFileID = ev.m_strFileID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModDevEventReportReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify device event report req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::ModifyDeviceEventRsp_USR ModDevEventReportRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModDevEventReportRsp))
        {
            LOG_ERROR_RLD("Modify device event report rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModDevEventReportRsp.m_iRetcode;

        LOG_INFO_RLD("Modify device event report info and device id is " << ev.m_strDevID << " and event id is " << ev.m_strDevID <<
            " and return code is " << ModDevEventReportRsp.m_iRetcode <<
            " and return msg is " << ModDevEventReportRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;

}

bool HttpMsgHandler::AddUserSpace(const std::string &strSid, const std::string &strUserID, const SpaceInfo &stif)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::AddStorageDetailReq_USR AddSTSpaceReq;
        AddSTSpaceReq.m_MsgType = InteractiveProtoHandler::MsgType::AddStorageDetailReq_USR_T;
        AddSTSpaceReq.m_uiMsgSeq = 1;
        AddSTSpaceReq.m_strSID = strSid;
        AddSTSpaceReq.m_storageDetail.m_strBeginDate = stif.m_strBeginDate;
        AddSTSpaceReq.m_storageDetail.m_strEndDate = stif.m_strEndDate;
        AddSTSpaceReq.m_storageDetail.m_strExtend = stif.m_strExtend;
        AddSTSpaceReq.m_storageDetail.m_strObjectID = strUserID;
        AddSTSpaceReq.m_storageDetail.m_uiObjectType = 0;
        AddSTSpaceReq.m_storageDetail.m_strStorageName = stif.m_strStorageName;
        AddSTSpaceReq.m_storageDetail.m_uiDomainID = stif.m_uiDomainID;
        AddSTSpaceReq.m_storageDetail.m_uiOverlapType = stif.m_uiOverlapType;
        AddSTSpaceReq.m_storageDetail.m_uiSizeOfSpaceUsed = 0;
        AddSTSpaceReq.m_storageDetail.m_uiStorageTimeDownLimit = stif.m_uiStorageTimeDownLimit;
        AddSTSpaceReq.m_storageDetail.m_uiStorageTimeUpLimit = stif.m_uiStorageTimeUpLimit;
        AddSTSpaceReq.m_storageDetail.m_uiStorageType = 0;
        AddSTSpaceReq.m_storageDetail.m_uiStorageUnitType = 0;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(AddSTSpaceReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Add user space req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::AddStorageDetailRsp_USR AddStSpaceRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddStSpaceRsp))
        {
            LOG_ERROR_RLD("Add user space rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = AddStSpaceRsp.m_iRetcode;

        

        LOG_INFO_RLD("Add user space info and sid id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << AddStSpaceRsp.m_iRetcode <<
            " and return msg is " << AddStSpaceRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::DeleteUserSpace(const std::string &strSid, const std::string &strUserID, const unsigned int &uiDomainID)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::DeleteStorageDetailReq_USR DelSTSpaceReq;
        DelSTSpaceReq.m_MsgType = InteractiveProtoHandler::MsgType::DeleteStorageDetailReq_USR_T;
        DelSTSpaceReq.m_uiMsgSeq = 1;
        DelSTSpaceReq.m_strSID = strSid;
        DelSTSpaceReq.m_strObjectID = strUserID;
        DelSTSpaceReq.m_uiDomainID = uiDomainID;
        
        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(DelSTSpaceReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Delete user space req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::DeleteStorageDetailRsp_USR DelStSpaceRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DelStSpaceRsp))
        {
            LOG_ERROR_RLD("Delete user space rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = DelStSpaceRsp.m_iRetcode;

        

        LOG_INFO_RLD("Delete user space info and sid id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << DelStSpaceRsp.m_iRetcode <<
            " and return msg is " << DelStSpaceRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::ModifyUserSpace(const std::string &strSid, const std::string &strUserID, const SpaceInfo &stif)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::ModifyStorageDetailReq_USR ModSTSpaceReq;
        ModSTSpaceReq.m_MsgType = InteractiveProtoHandler::MsgType::ModifyStorageDetailReq_USR_T;
        ModSTSpaceReq.m_uiMsgSeq = 1;
        ModSTSpaceReq.m_strSID = strSid;
        ModSTSpaceReq.m_storageDetail.m_strBeginDate = stif.m_strBeginDate;
        ModSTSpaceReq.m_storageDetail.m_strEndDate = stif.m_strEndDate;
        ModSTSpaceReq.m_storageDetail.m_strExtend = stif.m_strExtend;
        ModSTSpaceReq.m_storageDetail.m_strObjectID = strUserID;
        ModSTSpaceReq.m_storageDetail.m_uiObjectType = 0;
        ModSTSpaceReq.m_storageDetail.m_strStorageName = stif.m_strStorageName;
        ModSTSpaceReq.m_storageDetail.m_uiDomainID = stif.m_uiDomainID;
        ModSTSpaceReq.m_storageDetail.m_uiOverlapType = stif.m_uiOverlapType;
        ModSTSpaceReq.m_storageDetail.m_uiSizeOfSpaceUsed = 0;
        ModSTSpaceReq.m_storageDetail.m_uiStorageTimeDownLimit = stif.m_uiStorageTimeDownLimit;
        ModSTSpaceReq.m_storageDetail.m_uiStorageTimeUpLimit = stif.m_uiStorageTimeUpLimit;
        ModSTSpaceReq.m_storageDetail.m_uiStorageType = 0;
        ModSTSpaceReq.m_storageDetail.m_uiStorageUnitType = 0;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(ModSTSpaceReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Modify user space req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::ModifyStorageDetailRsp_USR ModStSpaceRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, ModStSpaceRsp))
        {
            LOG_ERROR_RLD("Modify user space rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = ModStSpaceRsp.m_iRetcode;

        

        LOG_INFO_RLD("Modify user space info and sid id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << ModStSpaceRsp.m_iRetcode <<
            " and return msg is " << ModStSpaceRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::QueryUserSpace(const std::string &strSid, const std::string &strUserID, const unsigned int &uiDomainID, SpaceInfo &stif)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryStorageDetailReq_USR QuerySTSpaceReq;
        QuerySTSpaceReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryStorageDetailReq_USR_T;
        QuerySTSpaceReq.m_uiMsgSeq = 1;
        QuerySTSpaceReq.m_strSID = strSid;
        QuerySTSpaceReq.m_strObjectID = strUserID;
        QuerySTSpaceReq.m_uiDomainID = uiDomainID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QuerySTSpaceReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query user space req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::QueryStorageDetailRsp_USR QueryStSpaceRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryStSpaceRsp))
        {
            LOG_ERROR_RLD("Query user space rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        stif.m_strBeginDate = QueryStSpaceRsp.m_storageDetail.m_strBeginDate;
        stif.m_strEndDate = QueryStSpaceRsp.m_storageDetail.m_strEndDate;
        stif.m_strExtend = QueryStSpaceRsp.m_storageDetail.m_strExtend;
        stif.m_strStorageName = QueryStSpaceRsp.m_storageDetail.m_strStorageName;
        stif.m_uiDomainID = QueryStSpaceRsp.m_storageDetail.m_uiDomainID;
        stif.m_uiOverlapType = QueryStSpaceRsp.m_storageDetail.m_uiOverlapType;
        stif.m_uiStorageTimeDownLimit = QueryStSpaceRsp.m_storageDetail.m_uiStorageTimeDownLimit;
        stif.m_uiStorageTimeUpLimit = QueryStSpaceRsp.m_storageDetail.m_uiStorageTimeUpLimit;

        iRet = QueryStSpaceRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query user space info and sid id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << QueryStSpaceRsp.m_iRetcode <<
            " and return msg is " << QueryStSpaceRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::QueryStorageSpace(const std::string &strSid, const std::string &strUserID, StorageInfo &stif)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryRegionStorageInfoReq_USR QuerySTSpaceReq;
        QuerySTSpaceReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryRegionStorageInfoReq_USR_T;
        QuerySTSpaceReq.m_uiMsgSeq = 1;
        QuerySTSpaceReq.m_strSID = strSid;
        QuerySTSpaceReq.m_strUserID = strUserID;

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(QuerySTSpaceReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Query storage space req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        return writer("0", "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::QueryRegionStorageInfoRsp_USR QueryStSpaceRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryStSpaceRsp))
        {
            LOG_ERROR_RLD("Query storage space rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        stif.m_uiDomainID = QueryStSpaceRsp.m_uiDomainID;
        stif.m_uiSpaceSize = QueryStSpaceRsp.m_uiSizeOfSpace;
        stif.m_uiSpaceSizeUsed = QueryStSpaceRsp.m_uiSizeOfSpaceUsed;
        
        iRet = QueryStSpaceRsp.m_iRetcode;

        

        LOG_INFO_RLD("Query storage space info and sid id is " << strSid << " and user id is " << strUserID <<
            " and return code is " << QueryStSpaceRsp.m_iRetcode <<
            " and return msg is " << QueryStSpaceRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&HttpMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::ValidDatetime(const std::string &strDatetime)
{
    if (strDatetime.empty())
    {
        return false;
    }

    boost::regex reg0("([0-9]{4}[0-9]{2}[0-9]{2}[0-9]{2}[0-9]{2}[0-9]{2})"); //yyyyMMddHHmmss
    boost::regex reg1("([0-9]{4}[0-9]{2}[0-9]{2})"); //yyyyMMdd
    boost::regex reg2("([0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2})"); //yyyy-MM-dd HH:mm:ss
    boost::regex reg3("([0-9]{4}-[0-9]{2}-[0-9]{2})"); ////yyyy-MM-dd
    boost::regex reg4("([0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2})"); //yyyy-MM-dd  HH:mm

    if (!boost::regex_match(strDatetime, reg0) && !boost::regex_match(strDatetime, reg1) && !boost::regex_match(strDatetime, reg2) &&
        !boost::regex_match(strDatetime, reg3) && !boost::regex_match(strDatetime, reg4))
    {
        LOG_ERROR_RLD("Date time is invalid and input date is " << strDatetime);
        return false;
    }

    return true;
}

