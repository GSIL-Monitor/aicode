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
#include "boost/regex.hpp"


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

const std::string HttpMsgHandler::DEVICE_LOGIN_ACTION("device_login");

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
    //ResultInfoMap.insert(std::map<std::string, std::string>::value_type("value", "xx"));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::ModifyUserInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    std::string strUserName;
    itFind = pMsgInfoMap->find("username");
    if (pMsgInfoMap->end() != itFind)
    {
        strUserName = itFind->second;
    }

    std::string strUserPwd;
    itFind = pMsgInfoMap->find("userpwd");
    if (pMsgInfoMap->end() != itFind)
    {
        strUserPwd = itFind->second;
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
    
    LOG_INFO_RLD("Modify user info received and  user id is " << strUserid << " and user name is " << strUserName 
        << " and user pwd is " << strUserPwd << " and user type is " << uiType << " and extend is " << strExtend
        << " and session id is " << strSid);

    if (!ModifyUserInfo(strSid, strUserid, strUserName, strUserPwd, uiType, strExtend))
    {
        LOG_ERROR_RLD("Modify user info handle failed and user id is " << strUserid << " and sid is " << strSid);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));

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

bool HttpMsgHandler::ShakehandHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    if (!ModifyDevice(strSid, strUserID, strDevID, strDevName, strDevPwd, strDevType, strDevExtend, strDevInnerInfo))
    {
        LOG_ERROR_RLD("Modify device handle failed and user id is " << strUserID << " and sid is " << strSid << " and device id is " << strDevID);
        return blResult;
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

    InteractiveProtoHandler::Device devinfo;
    if (!QueryDeviceInfo<InteractiveProtoHandler::Device>(strSid, strDevID, devinfo))
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
    
    unsigned int uiBeginIndex = 0;

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
            
    LOG_INFO_RLD("Query devices of user info received and  user id is " << strUserID
        << " and begin index is " << uiBeginIndex
        << " and session id is " << strSid);

    std::list<InteractiveProtoHandler::Relation> relist;
    if (!QueryDevicesOfUser<InteractiveProtoHandler::Relation>(strSid, strUserID, uiBeginIndex, relist))
    {
        LOG_ERROR_RLD("Query devices of user handle failed and user id is " << strUserID << " and session id is " << strSid);
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

    unsigned int uiBeginIndex = 0;

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

    LOG_INFO_RLD("Query users of device info received and  user id is " << strDevID
        << " and begin index is " << uiBeginIndex
        << " and session id is " << strSid);

    std::list<InteractiveProtoHandler::Relation> relist;
    if (!QueryUsersOfDevice<InteractiveProtoHandler::Relation>(strSid, strDevID, uiBeginIndex, relist))
    {
        LOG_ERROR_RLD("Query users of device handle failed and device id is " << strDevID << " and session id is " << strSid);
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

    unsigned int uiBeginIndex = 0;

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

bool HttpMsgHandler::DeviceLoginHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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

    LOG_INFO_RLD("Device login info received and  device id is " << strDevID << " and device pwd is " << strDevPwd);

    std::string strSid;
    if (!DeviceLogin(strDevID, strDevPwd, strSid))
    {
        LOG_ERROR_RLD("Device login handle failed and device id is " << strDevID << " and sid is " << strSid);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("sid", strSid));

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

        RelationList.clear();
        RelationList.swap(UsrLoginRsp.m_reInfoList);

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

bool HttpMsgHandler::ModifyUserInfo(const std::string &strSid, const std::string &strUserID, const std::string &strUserName, 
    const std::string &strUserPwd, const unsigned int uiType, const std::string &strExtend)
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
        ModifyUsrInfoReq.m_userInfo.m_strUserPassword = strUserPwd;
        ModifyUsrInfoReq.m_userInfo.m_uiStatus = 0;
        ModifyUsrInfoReq.m_userInfo.m_uiTypeInfo = uiType;

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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

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
            return CommMsgHandler::FAILED;
        }
        catch (...)
        {
            LOG_ERROR_RLD("Add device type info is invalid" << " and input type is " << strDevType);
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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

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
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool HttpMsgHandler::ModifyDevice(const std::string &strSid, const std::string &strUserID, const std::string &strDevID, 
    const std::string &strDevName, const std::string &strDevPwd, const std::string &strDevType, 
    const std::string &strDevExtend, const std::string &strDevInnerInfo)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        unsigned int uiTypeInfo = 0xFFFFFFFF;
        if (!strDevType.empty())
        {
            try
            {
                uiTypeInfo = boost::lexical_cast<unsigned int>(strDevType);
            }
            catch (boost::bad_lexical_cast & e)
            {
                LOG_ERROR_RLD("Modify device type info is invalid and error msg is " << e.what() << " and input type is " << strDevType);
                return CommMsgHandler::FAILED;
            }
            catch (...)
            {
                LOG_ERROR_RLD("Modify device type info is invalid" << " and input type is " << strDevType);
                return CommMsgHandler::FAILED;
            }
        }

        InteractiveProtoHandler::ModifyDevReq_USR ModifyDevReq;
        ModifyDevReq.m_MsgType = InteractiveProtoHandler::MsgType::ModifyDevReq_USR_T;
        ModifyDevReq.m_uiMsgSeq = 1;
        ModifyDevReq.m_strSID = strSid;
        ModifyDevReq.m_strUserID = strUserID;
        ModifyDevReq.m_devInfo.m_strCreatedate = "";
        ModifyDevReq.m_devInfo.m_strDevID = strDevID;
        ModifyDevReq.m_devInfo.m_strDevName = strDevName;
        ModifyDevReq.m_devInfo.m_strDevPassword = strDevPwd;
        ModifyDevReq.m_devInfo.m_strExtend = strDevExtend;
        ModifyDevReq.m_devInfo.m_strInnerinfo = strDevInnerInfo;
        ModifyDevReq.m_devInfo.m_uiStatus = 0xFFFFFFFF;
        ModifyDevReq.m_devInfo.m_uiTypeInfo = uiTypeInfo;

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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

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
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;

}

template<typename T>
bool HttpMsgHandler::QueryDeviceInfo(const std::string &strSid, const std::string &strDevID, T &DevInfo)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        InteractiveProtoHandler::QueryDevInfoReq_USR QueryDevInfoReq;
        QueryDevInfoReq.m_MsgType = InteractiveProtoHandler::MsgType::QueryDevInfoReq_USR_T;
        QueryDevInfoReq.m_uiMsgSeq = 1;
        QueryDevInfoReq.m_strSID = strSid;
        QueryDevInfoReq.m_strValue = "";
        QueryDevInfoReq.m_strDevID = strDevID;

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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

        InteractiveProtoHandler::QueryDevInfoRsp_USR QueryDevInfoRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryDevInfoRsp))
        {
            LOG_ERROR_RLD("Query device info rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        DevInfo = QueryDevInfoRsp.m_devInfo;

        iRet = QueryDevInfoRsp.m_iRetcode;

        LOG_INFO_RLD("Query device info  return code is " << QueryDevInfoRsp.m_iRetcode <<
            " and return msg is " << QueryDevInfoRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;

}

template<typename T>
bool HttpMsgHandler::QueryDevicesOfUser(const std::string &strSid, const std::string &strUserID, const unsigned int uiBeginIndex, std::list<T> &RelationList)
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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

        InteractiveProtoHandler::QueryDevRsp_USR QueryDevRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryDevRsp))
        {
            LOG_ERROR_RLD("Query devices of user rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        RelationList.clear();
        RelationList.swap(QueryDevRsp.m_allRelationInfoList);

        iRet = QueryDevRsp.m_iRetcode;

        LOG_INFO_RLD("Query devices of user id is " << strUserID << " and session id is " << strSid <<
            " and return code is " << QueryDevRsp.m_iRetcode <<
            " and return msg is " << QueryDevRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}


template<typename T>
bool HttpMsgHandler::QueryUsersOfDevice(const std::string &strSid, const std::string &strDevID, const unsigned int uiBeginIndex, std::list<T> &RelationList)
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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

        InteractiveProtoHandler::QueryUserRsp_USR QueryUserRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, QueryUserRsp))
        {
            LOG_ERROR_RLD("Query users of device rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        RelationList.clear();
        RelationList.swap(QueryUserRsp.m_allRelationInfoList);

        iRet = QueryUserRsp.m_iRetcode;

        LOG_INFO_RLD("Query users of device id is " << strDevID << " and session id is " << strSid <<
            " and return code is " << QueryUserRsp.m_iRetcode <<
            " and return msg is " << QueryUserRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

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
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

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
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

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
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

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
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

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
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;

}

bool HttpMsgHandler::DeviceLogin(const std::string &strDevID, const std::string &strDevPwd, std::string &strSid)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {

        std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
        std::string::size_type pos = strCurrentTime.find('T');
        strCurrentTime.replace(pos, 1, std::string(" "));

        InteractiveProtoHandler::LoginReq_DEV DevLoginReq;
        DevLoginReq.m_MsgType = InteractiveProtoHandler::MsgType::LoginReq_DEV_T;
        DevLoginReq.m_uiMsgSeq = 1;
        DevLoginReq.m_strSID = "";
        DevLoginReq.m_strValue = "";
        DevLoginReq.m_strDevID = strDevID;
        DevLoginReq.m_strPassword = strDevPwd;
        
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

        if (!PreCommonHandler(strMsgReceived))
        {
            return iRet = CommMsgHandler::FAILED;
        }

        InteractiveProtoHandler::LoginRsp_DEV DevLoginRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, DevLoginRsp))
        {
            LOG_ERROR_RLD("Login device rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }

        strSid = DevLoginRsp.m_strSID;
        iRet = DevLoginRsp.m_iRetcode;

        LOG_INFO_RLD("Login device id is " << strDevID << " and session id is " << DevLoginRsp.m_strSID <<
            " and return code is " << DevLoginRsp.m_iRetcode <<
            " and return msg is " << DevLoginRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };

    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

