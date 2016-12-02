#include "UserManager.h"
#include <boost/scope_exit.hpp>
#include "intf.h"
#include "CommonUtility.h"
#include "ReturnCode.h"

UserManager::UserManager(const ParamInfo &pinfo) : m_ParamInfo(pinfo), m_DBRuner(1), m_pProtoHandler(new InteractiveProtoHandler)
{
    
}


UserManager::~UserManager()
{
    m_DBRuner.Stop();
}

bool UserManager::Init()
{
    

    m_DBRuner.Run();

    return true;
}

bool UserManager::RegisterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{
    InteractiveProtoHandler::RegisterUserReq_USR RegUsrReq;
    if (!m_pProtoHandler->UnSerializeReq(strMsg, RegUsrReq))
    {
        LOG_ERROR_RLD("Register user req unserialize failed, src id is " << strSrcID);
        return false;
    }

    //注册用户是无需到数据库中校验的，用户名是可以重复的。
    //这里是异步执行sql，防止阻塞，后续可以使用其他方式比如MQ来消除数据库瓶颈
    m_DBRuner.Post(boost::bind(&UserManager::InsertUserToDB, this, CreateUUID(), RegUsrReq.m_userInfo.m_strUserName,
        RegUsrReq.m_userInfo.m_strUserPassword, RegUsrReq.m_userInfo.m_uiTypeInfo, RegUsrReq.m_userInfo.m_strCreatedate,
        RegUsrReq.m_userInfo.m_uiStatus, RegUsrReq.m_userInfo.m_strExtend));

    InteractiveProtoHandler::RegisterUserRsp_USR RegUsrRsp;
    RegUsrRsp.m_MsgType = InteractiveProtoHandler::MsgType::RegisterUserRsp_USR_T;
    RegUsrRsp.m_uiMsgSeq = 1;
    RegUsrRsp.m_strSID = CreateUUID();
    RegUsrRsp.m_iRetcode = ReturnInfo::SUCCESS_CODE;
    RegUsrRsp.m_strRetMsg = ReturnInfo::SUCCESS_INFO;
    RegUsrRsp.m_strUserID = CreateUUID();
    RegUsrRsp.m_strValue = "value";

    std::string strSerializeOutPut;
    //InteractiveProtoHandler iphander;
    if (!m_pProtoHandler->SerializeReq(RegUsrRsp, strSerializeOutPut))
    {
        LOG_ERROR_RLD("Register user rsp serialize failed.");
        return false;
    }

    writer(strSrcID, strSerializeOutPut);
    LOG_INFO_RLD("Register user rsp already send, dst id is " << strSrcID);
    return true;

}

bool UserManager::UnRegisterUserReq(const std::string &strMsg, const std::string &strSrcID, MsgWriter writer)
{

    return true;
}

void UserManager::InsertUserToDB(const std::string &strUserID, const std::string &strUserName, const std::string &strUserPwd, 
    const int iTypeInfo, const std::string &strCreateDate, const int iStatus, const std::string &strExtend)
{
    char sql[1024] = { 0 };
    const char* sqlfmt = "insert into t_user_info("
        "id,userid,username, userpassword, typeinfo, createdate, status, extend) values(uuid(),"        
        "'%s',%s,'%s','%d','%s', %d,'%s')";
    snprintf(sql, sizeof(sql), sqlfmt, strUserID.c_str(), strUserName.c_str(), strUserPwd.c_str(), iTypeInfo, strCreateDate.c_str(), iStatus, strExtend.c_str());

    if (!DataAccessInstance::instance().QuerySql(std::string(sql)))
    {
        LOG_ERROR_RLD("Insert t_user_info sql exec failed, sql is " << sql);        
    }
}

