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
    std::map<std::string, std::string> ResultInfoMap;

    auto itFind = pMsgInfoMap->find("username");
    if (pMsgInfoMap->end() == itFind)
    {
        ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", FAILED_CODE));
        ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

        LOG_ERROR_RLD("User name not found.");
        return;
    }
    const std::string strUserName = itFind->second;


    itFind = pMsgInfoMap->find("userpwd");
    if (pMsgInfoMap->end() == itFind)
    {
        ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", FAILED_CODE));
        ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

        LOG_ERROR_RLD("User password not found.");
        return;
    }
    const std::string strUserPwd = itFind->second;


    itFind = pMsgInfoMap->find("type");    
    if (pMsgInfoMap->end() == itFind)
    {
        ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", FAILED_CODE));
        ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

        LOG_ERROR_RLD("User type not found.");
        return;
    }
    const int iType = boost::lexical_cast<int>(itFind->second);

    std::string strValue;
    itFind = pMsgInfoMap->find("value");
    if (pMsgInfoMap->end() != itFind)
    {
        strValue = itFind->second;
    }

    LOG_INFO_RLD("Register user info received and user name is " << strUserName << " and user pwd is " << strUserPwd << " and user type is " << iType);
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("userid", "123456"));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("value", "xx"));
    
    WriteMsg(ResultInfoMap, writer);


}

void HttpMsgHandler::WriteMsg(const std::map<std::string, std::string> &MsgMap, MsgWriter writer, const bool IsError)
{
    Json::Value jsBody;
    auto itBegin = MsgMap.begin();
    auto itEnd = MsgMap.end();
    while (itBegin != itEnd)
    {
        jsBody[itBegin->first] = itBegin->second;
        
        ++itBegin;
    }

    Json::FastWriter fastwriter;
    const std::string &strBody = fastwriter.write(jsBody);//jsBody.toStyledString();

    //writer(strBody.c_str(), strBody.size(), MsgWriterModel::PRINT_MODEL);

    std::string strOutputMsg;
    if (IsError)
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



