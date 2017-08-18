#include "PassengerFlowMsgHandler.h"
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

const std::string PassengerFlowMsgHandler::SUCCESS_CODE = "0";
const std::string PassengerFlowMsgHandler::SUCCESS_MSG = "Ok";
const std::string PassengerFlowMsgHandler::FAILED_CODE = "-1";
const std::string PassengerFlowMsgHandler::FAILED_MSG = "Inner failed";

const std::string PassengerFlowMsgHandler::REGISTER_USER_ACTION("register_user");


PassengerFlowMsgHandler::PassengerFlowMsgHandler(const ParamInfo &parminfo):
m_ParamInfo(parminfo),
m_pInteractiveProtoHandler(new InteractiveProtoHandler)
{

}

PassengerFlowMsgHandler::~PassengerFlowMsgHandler()
{

}

int PassengerFlowMsgHandler::RspFuncCommonAction(CommMsgHandler::Packet &pt, int *piRetCode, RspFuncCommon rspfunc)
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

bool PassengerFlowMsgHandler::PreCommonHandler(const std::string &strMsgReceived, int &iRetCode)
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


bool PassengerFlowMsgHandler::RegisterUserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
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


bool PassengerFlowMsgHandler::RegisterUser(const std::string &strUserName, const std::string &strUserPwd, const std::string &strType, const std::string &strExtend, 
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
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc, boost::bind(&PassengerFlowMsgHandler::RspFuncCommonAction, this, _1, &iRet, RspFunc));

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress, 
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool PassengerFlowMsgHandler::ParseMsgOfCompact(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter wr)
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

void PassengerFlowMsgHandler::WriteMsg(const std::map<std::string, std::string> &MsgMap, MsgWriter writer, const bool blResult, boost::function<void(void*)> PostFunc)
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


bool PassengerFlowMsgHandler::ValidDatetime(const std::string &strDatetime)
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

