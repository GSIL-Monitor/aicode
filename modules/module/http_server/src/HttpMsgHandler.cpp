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
#include "CommMsgHandler.h"
#include <boost/date_time/posix_time/posix_time.hpp>


const std::string HttpMsgHandler::SUCCESS_CODE = "0";
const std::string HttpMsgHandler::SUCCESS_MSG = "Ok";
const std::string HttpMsgHandler::FAILED_CODE = "-1";
const std::string HttpMsgHandler::FAILED_MSG = "Inner failed";

const std::string HttpMsgHandler::REGISTER_USER_ACTION("register_user");

const std::string HttpMsgHandler::UNREGISTER_USER_ACTION("unregister_user");

const std::string HttpMsgHandler::QUERY_USER_INFO_ACTION("query_userinfo");

const std::string HttpMsgHandler::USER_LOGIN_ACTION("user_login");

const std::string HttpMsgHandler::USER_LOGOUT_ACTION("user_logout");

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

HttpMsgHandler::HttpMsgHandler(const ParamInfo &parminfo):
m_ParamInfo(parminfo),
m_pInteractiveProtoHandler(new InteractiveProtoHandler)
{

}

HttpMsgHandler::~HttpMsgHandler()
{

}


bool HttpMsgHandler::RegisterUserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    LOG_INFO_RLD("Register user info received and user name is " << strUserName << " and user pwd is " << strUserPwd << " and user type is " << strType
         << " and extend is [" << strExtend << "]");

    std::string strUserID;
    if (!RegisterUser(strUserName, strUserPwd, strType, strExtend, strUserID))
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

    LOG_INFO_RLD("Query user info received and  user id is " << strUserid << " and strValue is [" << strValue << "]"
        << " and session id is " << strSid);

    InteractiveProtoHandler::User userinfo;
    if (!QueryUserInfo<InteractiveProtoHandler::User>(strSid, strUserid, userinfo))
    {
        LOG_ERROR_RLD("Query user info handle failed and user id is " << strUserid << " and user name is " << " and sid is " << strSid);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("userid", userinfo.m_strUserID));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("username", userinfo.m_strUserName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("type", boost::lexical_cast<std::string>(userinfo.m_uiTypeInfo)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("createdate", userinfo.m_strCreatedate));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("extend", userinfo.m_strExtend));
    //ResultInfoMap.insert(std::map<std::string, std::string>::value_type("value", "xx"));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::UserLoginHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsRelationList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsRelationList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", FAILED_CODE));
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

    std::string strUserID = pMsgInfoMap->end() == itFind2 ? "" : itFind2->second;

    LOG_INFO_RLD("Login user info received and  user name is " << strUsername <<
        " and user id is " << strUserID << 
        " and user pwd is " << strUserpwd <<
        " and strExtend is [" << strExtend << "]" << " and strValue is [" << strValue << "]");
        
    std::string strSid;
    std::list<InteractiveProtoHandler::Relation> relist;
    if (!UserLogin<InteractiveProtoHandler::Relation>(strUsername, strUserpwd, relist, strUserID, strSid))
    {
        LOG_ERROR_RLD("Login user handle failed and user name is " << strUsername << " and user pwd is " << strUserpwd);
        return blResult;
    }

    auto itBegin = relist.begin();
    auto itEnd = relist.end();
    while (itBegin != itEnd)
    {
        Json::Value jsRelation;
        jsRelation["userid"] = itBegin->m_strUserID;
        jsRelation["devid"] = itBegin->m_strDevID;
        jsRelation["relation"] = itBegin->m_uiRelation;
        jsRelation["begindate"] = itBegin->m_strBeginDate;
        jsRelation["enddate"] = itBegin->m_strEndDate;
        jsRelation["extend"] = itBegin->m_strValue;

        jsRelationList.append(jsRelation);

        ++itBegin;
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

bool HttpMsgHandler::AddDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    LOG_INFO_RLD("Add device info received and  user id is " << strUserID << " and devcie id is " << strDevID << " and device name is " << strDevName
        << " and device pwd is [" << strDevPwd << "]" << " and device type is " << strDevType << " and device extend is [" << strDevExtend << "]"
        << " and device inner info is [" << strDevInnerInfo << "]"
        << " and session id is " << strSid);

    if (!AddDevice(strSid, strUserID, strDevID, strDevName, strDevPwd, strDevType, strDevExtend, strDevInnerInfo))
    {
        LOG_ERROR_RLD("Add device handle failed and user id is " << strUserID << " and sid is " << strSid << " and device id is " << strDevID);
        return blResult;
    }
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::DeleteDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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
    
    LOG_INFO_RLD("Delete device info received and  user id is " << strUserID << " and devcie id is " << strDevID 
        << " and session id is " << strSid);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::ModifyDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    LOG_INFO_RLD("Modify device info received and  user id is " << strUserID << " and devcie id is " << strDevID << " and device name is " << strDevName
        << " and device pwd is [" << strDevPwd << "]" << " and device type is " << strDevType << " and device extend is [" << strDevExtend << "]"
        << " and device inner info is [" << strDevInnerInfo << "]"
        << " and session id is " << strSid);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::QueryDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    LOG_INFO_RLD("Query device info received and  devcie id is " << strDevID
        << " and session id is " << strSid);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("devid", "jkdjkajk"));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("devname", "yuieuiw"));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("devpwd", "pwd"));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("devtype", "0"));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("devextend", "extend"));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("devinnerinfo", "jkdi893kdsjksjk"));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::QueryDevicesOfUserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsRelationList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsRelationList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", FAILED_CODE));
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

    itFind = pMsgInfoMap->find("beginindex");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Begin index not found.");
        return blResult;
    }
    const std::string strBeginIndex = itFind->second;
        
    LOG_INFO_RLD("Query device of user info received and  user id is " << strUserID
        << " and begin index is " << strBeginIndex
        << " and session id is " << strSid);


    Json::Value jsRelation;
    jsRelation["userid"] = "dlklkalk";
    jsRelation["devid"] = "wuiesd89";
    jsRelation["relation"] = "0";
    jsRelation["begindate"] = "2010-08-08";
    jsRelation["enddate"] = "2013-08-11";
    jsRelation["value"] = "testvalue";

    Json::Value jsRelation2;
    jsRelation2["userid"] = "drtertrty";
    jsRelation2["devid"] = "546546redf";
    jsRelation2["relation"] = "1";
    jsRelation2["begindate"] = "2010-08-08";
    jsRelation2["enddate"] = "2013-08-11";
    jsRelation2["value"] = "testvalue";



    jsRelationList.append(jsRelation);
    jsRelationList.append(jsRelation2);
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::QueryUsersOfDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsRelationList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsRelationList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", FAILED_CODE));
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

    itFind = pMsgInfoMap->find("beginindex");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Begin index not found.");
        return blResult;
    }
    const std::string strBeginIndex = itFind->second;

    LOG_INFO_RLD("Query user of device info received and  device id is " << strDevID
        << " and begin index is " << strBeginIndex
        << " and session id is " << strSid);


    Json::Value jsRelation;
    jsRelation["userid"] = "dlklkalk";
    jsRelation["devid"] = "wuiesd89";
    jsRelation["relation"] = "0";
    jsRelation["begindate"] = "2010-08-08";
    jsRelation["enddate"] = "2013-08-11";
    jsRelation["value"] = "testvalue";

    Json::Value jsRelation2;
    jsRelation2["userid"] = "drtertrty";
    jsRelation2["devid"] = "546546redf";
    jsRelation2["relation"] = "1";
    jsRelation2["begindate"] = "2010-08-08";
    jsRelation2["enddate"] = "2013-08-11";
    jsRelation2["value"] = "testvalue";



    jsRelationList.append(jsRelation);
    jsRelationList.append(jsRelation2);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));


    blResult = true;

    return blResult;
}

bool HttpMsgHandler::SharingDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("relation");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Relation not found.");
        return blResult;
    }
    const std::string strRelation = itFind->second;

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

    LOG_INFO_RLD("Sharing device info received and  user id is " << strUserID << " and devcie id is " << strDevID << " and relation is " << strRelation
        << " and begin date is [" << strBeginDate << "]" << " and end date is " << strEndDate << " and value is [" << strValue << "]"
        << " and session id is " << strSid);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::CancelSharedDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    itFind = pMsgInfoMap->find("relation");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Relation not found.");
        return blResult;
    }
    const std::string strRelation = itFind->second;

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

    LOG_INFO_RLD("Cancel shared device info received and  user id is " << strUserID << " and devcie id is " << strDevID << " and relation is " << strRelation
        << " and begin date is [" << strBeginDate << "]" << " and end date is " << strEndDate << " and value is [" << strValue << "]"
        << " and session id is " << strSid);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::AddFriendsHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    
    LOG_INFO_RLD("Add friend info received and  user id is " << strUserID << " and friend id is " << strFriendID 
        << " and session id is " << strSid);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::DeleteFriendsHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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


    LOG_INFO_RLD("Delete friend info received and  user id is " << strUserID << " and friend id is " << strFriendID
        << " and session id is " << strSid);

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::QueryFriendHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;
    Json::Value jsFriendIDList;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult, &jsFriendIDList)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", FAILED_CODE));
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

    itFind = pMsgInfoMap->find("beginindex");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Begin index not found.");
        return blResult;
    }
    const std::string strBeginIndex = itFind->second;

    LOG_INFO_RLD("Query user friend info received and  user id is " << strUserID
        << " and begin index is " << strBeginIndex
        << " and session id is " << strSid);


    Json::Value jsRelation;
    jsRelation["userid"] = "dlklkalk";
    
    Json::Value jsRelation2;
    jsRelation2["userid"] = "8989uijklkd";
    
    jsFriendIDList.append(jsRelation);
    jsFriendIDList.append(jsRelation2);

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

bool HttpMsgHandler::PreCommonHandler(const std::string &strMsgReceived)
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

        if (CommMsgHandler::SUCCEED != rsp.m_iRetcode)
        {
            LOG_ERROR_RLD("Msg prehandler rsp return failed.");
            return false;
        }
    }

    return true;
}

bool HttpMsgHandler::RegisterUser(const std::string &strUserName, const std::string &strUserPwd, const std::string &strType, const std::string &strExtend, std::string &strUserID)
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
            return false;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Register user type info is invalid" << " and input type is " << strType);
            return false;
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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

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
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

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
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
    
}

template<typename T>
bool HttpMsgHandler::UserLogin(const std::string &strUserName, const std::string &strUserPwd, std::list<T> &RelationList,
    std::string &strUserID, std::string &strSid)
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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

        InteractiveProtoHandler::LoginRsp_USR UsrLoginRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, UsrLoginRsp))
        {
            LOG_ERROR_RLD("Login user rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        //strUserID = UnRegUsrRsp.m_strUserID;

        RelationList.clear();
        auto itBegin = UsrLoginRsp.m_reInfoList.begin();
        auto itEnd = UsrLoginRsp.m_reInfoList.end();
        while (itBegin != itEnd)
        {
            //boost::shared_ptr<InteractiveProtoHandler::Relation> pRelTmp(new InteractiveProtoHandler::Relation);
            //(*pRelTmp) = *itBegin;
            //RelationList.push_back(pRelTmp);
            // 

            RelationList.push_back(*itBegin);

            ++itBegin;
        }

        strUserID = UsrLoginRsp.m_strUserID;
        strSid = UsrLoginRsp.m_strSID;
        iRet = UsrLoginRsp.m_iRetcode;

        LOG_INFO_RLD("Login user id is " << UsrLoginRsp.m_strUserID << " and session id is " << UsrLoginRsp.m_strSID << 
            " and return code is " << UsrLoginRsp.m_iRetcode <<
            " and return msg is " << UsrLoginRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress, 
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

template<typename T>
bool HttpMsgHandler::QueryUserInfo(const std::string &strSid, const std::string &strUserID, T &UserInfo)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
        std::string::size_type pos = strCurrentTime.find('T');
        strCurrentTime.replace(pos, 1, std::string(" "));

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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

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
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

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
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::AddDevice(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, 
    const std::string &strDevName, const std::string &strDevPwd, const std::string &strDevType, 
    const std::string &strDevExtend, const std::string &strDevInnerInfo)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        unsigned int uiTypeInfo = 0;
        try
        {
            uiTypeInfo = boost::lexical_cast<unsigned int>(strDevType);
        }
        catch (boost::bad_lexical_cast & e)
        {
            LOG_ERROR_RLD("Add device type info is invalid and error msg is " << e.what() << " and input type is " << strDevType);
            return false;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Add device type info is invalid" << " and input type is " << strDevType);
            return false;
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
        AddDevReq.m_devInfo.m_strDevID = strDevID;
        AddDevReq.m_devInfo.m_strDevName = strDevName;
        AddDevReq.m_devInfo.m_strDevPassword = strDevPwd;
        AddDevReq.m_devInfo.m_strExtend = strDevExtend;
        AddDevReq.m_devInfo.m_strInnerinfo = strDevInnerInfo;
        AddDevReq.m_devInfo.m_uiStatus = 0;
        AddDevReq.m_devInfo.m_uiTypeInfo = uiTypeInfo;

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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

        InteractiveProtoHandler::AddDevRsp_USR AddDevRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, AddDevRsp))
        {
            LOG_ERROR_RLD("Add device rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        iRet = AddDevRsp.m_iRetcode;

        LOG_INFO_RLD("Add device is " << strUserID << " and session id is " << strSid <<
            " and return code is " << AddDevRsp.m_iRetcode <<
            " and return msg is " << AddDevRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

