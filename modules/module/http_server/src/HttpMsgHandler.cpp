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


const std::string HttpMsgHandler::SUCCESS_CODE = "0";
const std::string HttpMsgHandler::SUCCESS_MSG = "Ok";
const std::string HttpMsgHandler::FAILED_CODE = "-1";
const std::string HttpMsgHandler::FAILED_MSG = "Inner failed";

HttpMsgHandler::HttpMsgHandler(const ParamInfo &parminfo) : m_ParamInfo(parminfo)
{

}

HttpMsgHandler::~HttpMsgHandler()
{

}

void HttpMsgHandler::RegisterUserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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
        return;
    }
    const std::string strUserName = itFind->second;


    itFind = pMsgInfoMap->find("userpwd");
    if (pMsgInfoMap->end() == itFind)
    {        
        LOG_ERROR_RLD("User password not found.");
        return;
    }
    const std::string strUserPwd = itFind->second;
    
    itFind = pMsgInfoMap->find("type");    
    if (pMsgInfoMap->end() == itFind)
    {        
        LOG_ERROR_RLD("User type not found.");
        return;
    }
    const int iType = boost::lexical_cast<int>(itFind->second);

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

    LOG_INFO_RLD("Register user info received and user name is " << strUserName << " and user pwd is " << strUserPwd << " and user type is " << iType
         << " and extend is [" << strExtend << "] and strValue is [" <<   strValue << "]");
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("userid", "123456"));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("value", "xx"));
    
    blResult = true;

}

void HttpMsgHandler::UnRegisterUserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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
        return;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("username");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User name not found.");
        return;
    }
    const std::string strUserName = itFind->second;


    itFind = pMsgInfoMap->find("userpwd");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User password not found.");
        return;
    }
    const std::string strUserPwd = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return;
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

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("userid", "123456"));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("value", "xx"));

    blResult = true;
    
}

void HttpMsgHandler::QueryUserInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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
        return;
    }
    const std::string strSid = itFind->second;

    itFind = pMsgInfoMap->find("userid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User id not found.");
        return;
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



    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("userid", "123456"));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("username", "xxxx"));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("type", "x"));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("createdate", "2010-02-03"));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("extend", "xxxx"));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("value", "xx"));

    blResult = true;
}

void HttpMsgHandler::UserLoginHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User name not found.");
        return;
    }
    const std::string strUsername = itFind->second;

    itFind = pMsgInfoMap->find("userpwd");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("User pwd not found.");
        return;
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

    LOG_INFO_RLD("Login user info received and  user name is " << strUsername << " and user pwd is " << strUserpwd
        << " and strExtend is [" << strExtend << "]" << " and strValue is [" << strValue << "]");
    
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
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("value", "xx"));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("sid", "jkjkdjk89892s"));

    blResult = true;


}

void HttpMsgHandler::UserLogoutHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{

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


