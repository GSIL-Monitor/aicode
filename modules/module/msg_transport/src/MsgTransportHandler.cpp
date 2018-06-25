#include "MsgTransportHandler.h"
#include <boost/algorithm/string.hpp>  
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scope_exit.hpp>
#include "json/json.h"
#include "LogRLD.h"
#include "InteractiveProtoHandler.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include "boost/regex.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"
#include "CacheMgr.h"

//#define TIXML_USE_STL
//#include "tinyxml/tinyxml.h"
//#include "tinyxml/tinystr.h"

const std::string MsgTransportHandler::SUCCESS_CODE = "0";
const std::string MsgTransportHandler::SUCCESS_MSG = "Ok";
const std::string MsgTransportHandler::FAILED_CODE = "-1";
const std::string MsgTransportHandler::FAILED_MSG = "Inner failed";

const std::string MsgTransportHandler::REGISTER_USER_ACTION("register_user");

MsgTransportHandler::MsgTransportHandler(const ParamInfo &parminfo, CacheMgr &sgr) :
m_ParamInfo(parminfo),
m_pInteractiveProtoHandler(new InteractiveProtoHandler),
m_MsgReceiveRunner(1),
m_SessionMgr(sgr),
m_SidTimer(NULL, 5)
{
    auto TmFunc = [&](const boost::system::error_code &ec) ->void
    {
        if (ec)
        {
            LOG_ERROR_RLD("Sid timer error: " << ec.message());
            return;
        }

        boost::unique_lock<boost::mutex> lock(m_ReceiveSidMutex);
        Json::Value jsSidInfo;
        jsSidInfo["receive_sid"] = m_strReceiveSid;
        Json::FastWriter fastwriter;
        const std::string &strBody = fastwriter.write(jsSidInfo); //jsBody.toStyledString();

        if (!m_SessionMgr.MemCacheCreate(m_strID, strBody, 60))
        {
            LOG_ERROR_RLD("Set receive sid value to cache failed and value is " << strBody);
        }

    };
    m_SidTimer.SetTimeOutCallBack(TmFunc);

    //
    m_MsgReceiveRunner.Run();

    auto Receiver = [&]() -> void
    {
        while (true)
        {
            bool blRet = false;

            //开始建立连接，获取proxy对应返回的sid值。            
            boost::shared_ptr<CommMsgHandler> pCommMsgHdr;
            do 
            {
                LOG_INFO_RLD("Begin connect proxy....");
                pCommMsgHdr.reset(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));

                if (CommMsgHandler::SUCCEED != pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress,
                    m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval))
                {
                    LOG_ERROR_RLD("Connect channel failed and remote address is " << m_ParamInfo.m_strRemoteAddress <<
                        "and port is " << m_ParamInfo.m_strRemotePort);
                    blRet = false;
                }
                else
                {
                    blRet = true;
                }

                if (!blRet)
                {
                    boost::this_thread::sleep(boost::posix_time::microseconds(1000));
                }
            } while (!blRet);
            LOG_INFO_RLD("End connect proxy....");


            //设置获取到的sid值（表示接受消息的sid）到cache中去
            LOG_INFO_RLD("Begin set sid to cache and id is " << m_strID);
            {
                const std::string &strValue = pCommMsgHdr->GetSessionID();
                boost::unique_lock<boost::mutex> lock(m_ReceiveSidMutex);
                m_strReceiveSid = strValue;
            }

            m_SidTimer.Run(true);
            LOG_INFO_RLD("End set sid to cache and id is " << m_strID << " and sid is " << pCommMsgHdr->GetSessionID());

            //开始被动接收消息
            int iRet = CommMsgHandler::SUCCEED;
            std::string strSrcID;
            auto RspInfinite = [&](CommMsgHandler::Packet &pt) -> int
            {
                strSrcID = pt.strSrcID;
                const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

                InteractiveProtoHandler::ControlChannelReq_DEV CtrlReq;
                if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, CtrlReq))
                {
                    LOG_ERROR_RLD("Control req unserialize failed.");
                    return iRet = CommMsgHandler::FAILED;
                }
            

                LOG_INFO_RLD("Receive control req and  device id is  " << CtrlReq.m_strDeviceID << " and device type is " << CtrlReq.m_uiDeviceType <<
                    " and extend is " << CtrlReq.m_strExtend);

                boost::this_thread::sleep(boost::posix_time::milliseconds(3000));

                return CommMsgHandler::SUCCEED;
            };

            auto ReqInfinite = [&](CommMsgHandler::SendWriter writer) -> int
            {
                InteractiveProtoHandler::ControlChannelRsp_DEV CtrlRsp;
                CtrlRsp.m_MsgType = InteractiveProtoHandler::MsgType::ControlChannelRsp_DEV_T;
                CtrlRsp.m_uiMsgSeq = 1;
                CtrlRsp.m_strSID = "";
                CtrlRsp.m_strValue = "Rsp value";
                CtrlRsp.m_iRetcode = 0;

                std::string strSerializeOutPut;
                if (!m_pInteractiveProtoHandler->SerializeReq(CtrlRsp, strSerializeOutPut))
                {
                    LOG_ERROR_RLD("Control rsp serialize failed.");
                    return CommMsgHandler::FAILED;
                }

                LOG_INFO_RLD("Send control rsp");

                return writer(strSrcID, "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
            };

            pCommMsgHdr->SetReqAndRspInfinite(ReqInfinite, RspInfinite);

            iRet = pCommMsgHdr->StartReceiveInfinite(); //不会返回的，一旦返回就是异常，需要整个重新建立连接重来
            LOG_ERROR_RLD("Receive inifinite has returned and code is " << iRet);
        }
    };

    m_MsgReceiveRunner.Post(Receiver);

}

MsgTransportHandler::~MsgTransportHandler()
{
    m_SidTimer.Stop();
}

bool MsgTransportHandler::CtrlMsgHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap)
{
    //ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        //if (!blResult)
        //{
        //    ResultInfoMap.clear();
        //    //ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
        //    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));                   
        //}

        //this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END
    
    //
    auto itFind = pMsgInfoMap->find("dst_id");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Dst id not found.");
        return blResult;
    }
    const std::string strDstId = itFind->second;


    //itFind = pMsgInfoMap->find("userpwd");
    //if (pMsgInfoMap->end() == itFind)
    //{        
    //    LOG_ERROR_RLD("User password not found.");
    //    return blResult;
    //}
    //const std::string strUserPwd = itFind->second;
    
    std::string strValue;
    if (!m_SessionMgr.MemCacheGet(strDstId, strValue))
    {
        LOG_ERROR_RLD("Dst id not found in the cache and id is " << strDstId);
        return blResult;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(strValue, root, false))
    {
        LOG_ERROR_RLD("Parsed failed and value is " << strValue);
        return false;
    }

    if (!root.isObject())
    {
        LOG_ERROR_RLD("Value is invalid and value is " << strValue);
        return false;
    }

    Json::Value jsReceiveSid = root["receive_sid"];

    if (jsReceiveSid.isNull())
    {
        LOG_ERROR_RLD("Value is null and value is " << strValue);
        return false;
    }
    
    const std::string &strReceiveSid = jsReceiveSid.asString();

    //ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Control msg and dst id is " << strDstId << " and dst sid is " << strReceiveSid);
    
    if (!CtrlMsg(strReceiveSid))
    {
        LOG_ERROR_RLD("Ctrl msg handle failed");
        return blResult;
    }
    
    //ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    //ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    
    blResult = true;

    return blResult;
}

bool MsgTransportHandler::CtrlMsg(const std::string &strDstSid)
{
    auto ReqFunc = [&](CommMsgHandler::SendWriter writer) -> int
    {
        
        InteractiveProtoHandler::ControlChannelReq_DEV CtrlReq;
        CtrlReq.m_MsgType = InteractiveProtoHandler::MsgType::ControlChannelReq_DEV_T;
        CtrlReq.m_uiMsgSeq = 1;
        CtrlReq.m_strSID = "";
        CtrlReq.m_strDeviceID = m_strID; //"Sender device id 0";
        CtrlReq.m_uiDeviceType = 9;
        CtrlReq.m_strExtend = "Sender extend";

        std::string strSerializeOutPut;
        if (!m_pInteractiveProtoHandler->SerializeReq(CtrlReq, strSerializeOutPut))
        {
            LOG_ERROR_RLD("Control req serialize failed.");
            return CommMsgHandler::FAILED;
        }

        LOG_INFO_RLD("Send control req and  device id is  " << CtrlReq.m_strDeviceID << " and device type is " << CtrlReq.m_uiDeviceType <<
            " and extend is " << CtrlReq.m_strExtend);

        return writer(strDstSid, "1", strSerializeOutPut.c_str(), strSerializeOutPut.length());
    };

    int iRet = CommMsgHandler::SUCCEED;
    auto RspFunc = [&](CommMsgHandler::Packet &pt) -> int
    {
        const std::string &strMsgReceived = std::string(pt.pBuffer.get(), pt.buflen);

        InteractiveProtoHandler::ControlChannelRsp_DEV CtrlRsp;
        if (!m_pInteractiveProtoHandler->UnSerializeReq(strMsgReceived, CtrlRsp))
        {
            LOG_ERROR_RLD("Contrl rsp unserialize failed.");
            return iRet = CommMsgHandler::FAILED;
        }
        

        LOG_INFO_RLD("Receive control  rsp and value is " << CtrlRsp.m_strValue << " and return code is " << CtrlRsp.m_iRetcode <<
            " and return msg is " << CtrlRsp.m_strRetMsg);

        return CommMsgHandler::SUCCEED;
    };
    
    boost::shared_ptr<CommMsgHandler> pCommMsgHdr(new CommMsgHandler(m_ParamInfo.m_strSelfID, m_ParamInfo.m_uiCallFuncTimeout));
    pCommMsgHdr->SetReqAndRspHandler(ReqFunc,  RspFunc);

    return CommMsgHandler::SUCCEED == pCommMsgHdr->Start(m_ParamInfo.m_strRemoteAddress, 
        m_ParamInfo.m_strRemotePort, 0, m_ParamInfo.m_uiShakehandOfChannelInterval) &&
        CommMsgHandler::SUCCEED == iRet;
}

bool MsgTransportHandler::ValidDatetime(const std::string &strDatetime)
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

bool MsgTransportHandler::GetValueList(const std::string &strValue, std::list<std::string> &strValueList)
{
    bool blResult = false;

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(strValue, root))
    {
        LOG_ERROR_RLD("Value info parse failed, raw data is : " << strValue);
        return blResult;
    }

    if (!root.isArray())
    {
        LOG_ERROR_RLD("Value info parse failed, raw data is : " << strValue);
        return blResult;
    }

    LOG_INFO_RLD("Value list size is " << root.size());

    for (unsigned int i = 0; i < root.size(); ++i)
    {
        auto jsValueItem = root[i];
        if (jsValueItem.isNull() || !jsValueItem.isString())
        {
            LOG_ERROR_RLD("Value info type is error, raw data is: " << strValue);
            return blResult;
        }

        strValueList.emplace_back(jsValueItem.asString());

        LOG_INFO_RLD("Value item is " << jsValueItem.asString());
    }

    blResult = true;

    return blResult;
}

