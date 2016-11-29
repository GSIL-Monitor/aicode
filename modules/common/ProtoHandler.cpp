#include "ProtoHandler.h"
#include "TDFS.pb.h"

//using namespace TDFS::MESSAGE;

template<typename T, typename TT> bool SerializerT(const TT &req, std::string &strOutput)
{
    const T *pReq = NULL;
    try
    {
        const T &reqtmp = dynamic_cast<const T&>(req);
        pReq = &reqtmp;

    }
    catch (std::bad_cast &bad)
    {
        return false;
    }

    SyncMessage Msg;
    Msg.Clear();

    pReq->Serializer(Msg);

    Msg.SerializeToString(&strOutput);

    return true;
};

template<typename T, typename TT> bool UnSerializerT(const SyncMessage &SyncMsg, TT &req)
{
    T *pReq = NULL;
    try
    {
        T &reqtmp = dynamic_cast<T&>(req);
        pReq = &reqtmp;

    }
    catch (std::bad_cast &bad)
    {
        return false;
    }

    pReq->UnSerializer(SyncMsg);

    return true;
}


ProtoHandler::ProtoHandler()
{
    //////////////////////////////////////////////////////
    
    SerializeHandler handler;
    handler.Szr = boost::bind(&ProtoHandler::LoginReqSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::LoginReqUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::LoginReq_T, handler));
    
    //

    handler.Szr = boost::bind(&ProtoHandler::LoginRspSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::LoginRspUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::LoginRsp_T, handler));

    ////////////////////////////////////////////////////////

    handler.Szr = boost::bind(&ProtoHandler::LogoutReqSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::LogoutReqUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::LoginoutReq_T, handler));

    //

    handler.Szr = boost::bind(&ProtoHandler::LogoutRspSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::LogoutRspUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::LoginoutRsp_T, handler));

    ///////////////////////////////////////////////////////


    handler.Szr = boost::bind(&ProtoHandler::ConfigInfoReqSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::ConfigInfoReqUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::ConfigInfoReq_T, handler));

    //

    handler.Szr = boost::bind(&ProtoHandler::ConfigInfoRspSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::ConfigInfoRspUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::ConfigInfoRsp_T, handler));

    //////////////////////////////////////////////////////

    handler.Szr = boost::bind(&ProtoHandler::ShakehandReqSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::ShakehandReqUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::ShakehandReq_T, handler));

    //

    handler.Szr = boost::bind(&ProtoHandler::ShakehandRspSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::ShakehandRspUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::ShakehandRsp_T, handler));

    //////////////////////////////////////////////////////

    handler.Szr = boost::bind(&ProtoHandler::GetSyncAddressReqSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::GetSyncAddressReqUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::GetSyncAddressReq_T, handler));

    //

    handler.Szr = boost::bind(&ProtoHandler::GetSyncAddressRspSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::GetSyncAddressRspUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::GetSyncAddressRsp_T, handler));

    //////////////////////////////////////////////////////

    handler.Szr = boost::bind(&ProtoHandler::SyncFileListPendingReqSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::SyncFileListPendingReqUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::SyncFileListPendingReq_T, handler));

    //

    handler.Szr = boost::bind(&ProtoHandler::SyncFileListPendingRspSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::SyncFileListPendingRspUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::SyncFileListPendingRsp_T, handler));

    //////////////////////////////////////////////////////

    handler.Szr = boost::bind(&ProtoHandler::GetFileInfoReqSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::GetFileInfoReqUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::GetFileInfoReq_T, handler));

    //

    handler.Szr = boost::bind(&ProtoHandler::GetFileInfoRspSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::GetFileInfoRspUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::GetFileInfoRsp_T, handler));

    //////////////////////////////////////////////////////

    handler.Szr = boost::bind(&ProtoHandler::ControlCMDReqSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::ControlCMDReqUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::ControlCMDReq_T, handler));

    //

    handler.Szr = boost::bind(&ProtoHandler::ControlCMDRspSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::ControlCMDRspUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::ControlCMDRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&ProtoHandler::FullSyncReqSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::FullSyncReqUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::FullSyncReq_T, handler));

    //

    handler.Szr = boost::bind(&ProtoHandler::FullSyncRspSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::FullSyncRspUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::FullSyncRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&ProtoHandler::FullSyncConformSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::FullSyncConformUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::FullSyncConform_T, handler));

    //
    
    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&ProtoHandler::ActiveReportingReqSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::ActiveReportingReqUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::ActiveReportingReq_T, handler));

    //

    handler.Szr = boost::bind(&ProtoHandler::ActiveReportingRspSerializer, this, _1, _2);
    handler.UnSzr = boost::bind(&ProtoHandler::ActiveReportingRspUnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(TDFS::MESSAGE::MsgType::ActiveReportingRsp_T, handler));

    /////////////////////////////////////////////////////

}

ProtoHandler::~ProtoHandler()
{

}

bool ProtoHandler::GetMsgType(const std::string &strData, MsgType &msgtype)
{
    SyncMessage Msg;
    Msg.Clear();
    if (!Msg.ParseFromString(strData))
    {
        return false;
    }

    msgtype = (MsgType)Msg.type();

    return true;
}

bool ProtoHandler::SerializeReq(const Req &req, std::string &strOutput)
{
    const int iType = req.m_MsgType;

    auto itFind = m_ReqAndRspHandlerMap.find(iType);
    if (m_ReqAndRspHandlerMap.end() == itFind)
    {
        return false;
    }

    return itFind->second.Szr(req, strOutput);
}

bool ProtoHandler::UnSerializeReq(const std::string &strData, Req &req)
{   
    SyncMessage Msg;
    Msg.Clear();
    if (!Msg.ParseFromString(strData))
    {
        return false;
    }
    
    const int iType = Msg.type();

    auto itFind = m_ReqAndRspHandlerMap.find(iType);
    if (m_ReqAndRspHandlerMap.end() == itFind)
    {
        return false;
    }
        
    return itFind->second.UnSzr(Msg, req);

}

bool ProtoHandler::SerializeRsp(const Rsp &rsp, std::string &strOutput)
{
    return SerializeReq(rsp, strOutput);
}

bool ProtoHandler::UnSerializeRsp(const std::string &strData, Rsp &rsp)
{
    return UnSerializeReq(strData, rsp);
}

bool ProtoHandler::LoginReqSerializer(const Req &req, std::string &strOutput)
{
    return SerializerT<LoginReq, Req>(req, strOutput);
}

bool ProtoHandler::LoginReqUnSerializer(const SyncMessage &SyncMsg, Req &req)
{
    return UnSerializerT<LoginReq, Req>(SyncMsg, req);
}

bool ProtoHandler::LoginRspSerializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<LoginRsp, Req>(rsp, strOutput);
}

bool ProtoHandler::LoginRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp)
{
    return UnSerializerT<LoginRsp, Req>(SyncMsg, rsp);
}

bool ProtoHandler::LogoutReqSerializer(const Req &req, std::string &strOutput)
{
    return SerializerT<LogoutReq, Req>(req, strOutput);
}

bool ProtoHandler::LogoutReqUnSerializer(const SyncMessage &SyncMsg, Req &req)
{
    return UnSerializerT<LogoutReq, Req>(SyncMsg, req);
}

bool ProtoHandler::LogoutRspSerializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<LogoutRsp, Req>(rsp, strOutput);
}

bool ProtoHandler::LogoutRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp)
{
    return UnSerializerT<LogoutRsp, Req>(SyncMsg, rsp);
}

bool ProtoHandler::ConfigInfoReqSerializer(const Req &req, std::string &strOutput)
{
    return SerializerT<ConfigInfoReq, Req>(req, strOutput);
}

bool ProtoHandler::ConfigInfoReqUnSerializer(const SyncMessage &SyncMsg, Req &req)
{
    return UnSerializerT<ConfigInfoReq, Req>(SyncMsg, req);
}

bool ProtoHandler::ConfigInfoRspSerializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<ConfigInfoRsp, Req>(rsp, strOutput);
}

bool ProtoHandler::ConfigInfoRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp)
{
    return UnSerializerT<ConfigInfoRsp, Req>(SyncMsg, rsp);    
}

bool ProtoHandler::ShakehandReqSerializer(const Req &req, std::string &strOutput)
{
    return SerializerT<ShakehandReq, Req>(req, strOutput);
}

bool ProtoHandler::ShakehandReqUnSerializer(const SyncMessage &SyncMsg, Req &req)
{
    return UnSerializerT<ShakehandReq, Req>(SyncMsg, req);
}

bool ProtoHandler::ShakehandRspSerializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<ShakehandRsp, Req>(rsp, strOutput);
}

bool ProtoHandler::ShakehandRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp)
{
    return UnSerializerT<ShakehandRsp, Req>(SyncMsg, rsp);
}

bool ProtoHandler::GetSyncAddressReqSerializer(const Req &req, std::string &strOutput)
{
    return SerializerT<GetSyncAddressReq, Req>(req, strOutput);
}

bool ProtoHandler::GetSyncAddressReqUnSerializer(const SyncMessage &SyncMsg, Req &req)
{
    return UnSerializerT<GetSyncAddressReq, Req>(SyncMsg, req);
}

bool ProtoHandler::GetSyncAddressRspSerializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<GetSyncAddressRsp, Req>(rsp, strOutput);
}

bool ProtoHandler::GetSyncAddressRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp)
{
    return UnSerializerT<GetSyncAddressRsp, Req>(SyncMsg, rsp);
}

bool ProtoHandler::SyncFileListPendingReqSerializer(const Req &req, std::string &strOutput)
{
    return SerializerT<SyncFileListPendingReq, Req>(req, strOutput);
}

bool ProtoHandler::SyncFileListPendingReqUnSerializer(const SyncMessage &SyncMsg, Req &req)
{
    return UnSerializerT<SyncFileListPendingReq, Req>(SyncMsg, req);
}

bool ProtoHandler::SyncFileListPendingRspSerializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<SyncFileListPendingRsp, Req>(rsp, strOutput);
}

bool ProtoHandler::SyncFileListPendingRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp)
{
    return UnSerializerT<SyncFileListPendingRsp, Req>(SyncMsg, rsp);
}

bool ProtoHandler::GetFileInfoReqSerializer(const Req &req, std::string &strOutput)
{
    return SerializerT<GetFileInfoReq, Req>(req, strOutput);
}

bool ProtoHandler::GetFileInfoReqUnSerializer(const SyncMessage &SyncMsg, Req &req)
{
    return UnSerializerT<GetFileInfoReq, Req>(SyncMsg, req);
}

bool ProtoHandler::GetFileInfoRspSerializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<GetFileInfoRsp, Req>(rsp, strOutput);
}

bool ProtoHandler::GetFileInfoRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp)
{
    return UnSerializerT<GetFileInfoRsp, Req>(SyncMsg, rsp);
}

bool ProtoHandler::ControlCMDReqSerializer(const Req &req, std::string &strOutput)
{
    return SerializerT<ControlCMDReq, Req>(req, strOutput);
}

bool ProtoHandler::ControlCMDReqUnSerializer(const SyncMessage &SyncMsg, Req &req)
{
    return UnSerializerT<ControlCMDReq, Req>(SyncMsg, req);
}

bool ProtoHandler::ControlCMDRspSerializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<ControlCMDRsp, Req>(rsp, strOutput);
}

bool ProtoHandler::ControlCMDRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp)
{
    return UnSerializerT<ControlCMDRsp, Req>(SyncMsg, rsp);
}

bool ProtoHandler::FullSyncReqSerializer(const Req &req, std::string &strOutput)
{
    return SerializerT<FullSyncReq, Req>(req, strOutput);
}

bool ProtoHandler::FullSyncReqUnSerializer(const SyncMessage &SyncMsg, Req &req)
{
    return UnSerializerT<FullSyncReq, Req>(SyncMsg, req);
}

bool ProtoHandler::FullSyncRspSerializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<FullSyncRsp, Req>(rsp, strOutput);
}

bool ProtoHandler::FullSyncRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp)
{
    return UnSerializerT<FullSyncRsp, Req>(SyncMsg, rsp);
}

bool ProtoHandler::FullSyncConformSerializer(const Req &req, std::string &strOutput)
{
    return SerializerT<FullSyncConform, Req>(req, strOutput);
}

bool ProtoHandler::FullSyncConformUnSerializer(const SyncMessage &SyncMsg, Req &req)
{
    return UnSerializerT<FullSyncConform, Req>(SyncMsg, req);
}

bool ProtoHandler::ActiveReportingReqSerializer(const Req &req, std::string &strOutput)
{
    return SerializerT<ActiveReportingReq, Req>(req, strOutput);
}

bool ProtoHandler::ActiveReportingReqUnSerializer(const SyncMessage &SyncMsg, Req &req)
{
    return UnSerializerT<ActiveReportingReq, Req>(SyncMsg, req);
}

bool ProtoHandler::ActiveReportingRspSerializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<ActiveReportingRsp, Req>(rsp, strOutput);
}

bool ProtoHandler::ActiveReportingRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp)
{
    return UnSerializerT<ActiveReportingRsp, Req>(SyncMsg, rsp);
}

void ProtoHandler::Req::UnSerializer(const SyncMessage &SyncMsg)
{
    m_MsgType = (MsgType)SyncMsg.type();
    m_uiMsgSeq = SyncMsg.uimsgseq();
    m_strSID = SyncMsg.strsid();
}

void ProtoHandler::Req::Serializer(SyncMessage &SyncMsg) const
{
    SyncMsg.set_uimsgseq(m_uiMsgSeq);
    SyncMsg.set_strsid(m_strSID);
}

void ProtoHandler::Rsp::UnSerializer(const SyncMessage &SyncMsg)
{
    Req::UnSerializer(SyncMsg);
    m_iRetcode = SyncMsg.rspvalue().iretcode();
    m_strRetMsg = SyncMsg.rspvalue().strretmsg();
    
}

void ProtoHandler::Rsp::Serializer(SyncMessage &SyncMsg) const
{
    Req::Serializer(SyncMsg);
    SyncMsg.mutable_rspvalue()->set_iretcode(m_iRetcode);
    SyncMsg.mutable_rspvalue()->set_strretmsg(m_strRetMsg);
}

void ProtoHandler::LoginReq::UnSerializer(const SyncMessage &SyncMsg)
{
    Req::UnSerializer(SyncMsg);
    m_strSyncServiceName = SyncMsg.reqvalue().loginreqvalue().strsyncservicename();
    m_strPassword = SyncMsg.reqvalue().loginreqvalue().strpassword();
    m_strStorageIP = SyncMsg.reqvalue().loginreqvalue().strstorageip();
    m_strStoragePort = SyncMsg.reqvalue().loginreqvalue().strstorageport();

}

void ProtoHandler::LoginReq::Serializer(SyncMessage &SyncMsg) const
{
    Req::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::LoginReq_T);
    SyncMsg.mutable_reqvalue()->mutable_loginreqvalue()->set_strsyncservicename(m_strSyncServiceName);
    SyncMsg.mutable_reqvalue()->mutable_loginreqvalue()->set_strpassword(m_strPassword);
    SyncMsg.mutable_reqvalue()->mutable_loginreqvalue()->set_strstorageip(m_strStorageIP);
    SyncMsg.mutable_reqvalue()->mutable_loginreqvalue()->set_strstorageport(m_strStoragePort);
    
}

void ProtoHandler::LoginRsp::UnSerializer(const SyncMessage &SyncMsg)
{
    Rsp::UnSerializer(SyncMsg);
    m_strLoginSID = SyncMsg.rspvalue().loginrspvalue().strsid();
}

void ProtoHandler::LoginRsp::Serializer(SyncMessage &SyncMsg) const
{
    Rsp::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::LoginRsp_T);
    SyncMsg.mutable_rspvalue()->mutable_loginrspvalue()->set_strsid(m_strLoginSID);
}

void ProtoHandler::LogoutReq::UnSerializer(const SyncMessage &SyncMsg)
{
    Req::UnSerializer(SyncMsg);
    m_strValue = SyncMsg.reqvalue().loginoutreqvalue().strvalue();
}

void ProtoHandler::LogoutReq::Serializer(SyncMessage &SyncMsg) const
{
    Req::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::LoginoutReq_T);
    SyncMsg.mutable_reqvalue()->mutable_loginoutreqvalue()->set_strvalue(m_strValue);
}

void ProtoHandler::LogoutRsp::UnSerializer(const SyncMessage &SyncMsg)
{
    Rsp::UnSerializer(SyncMsg);
    m_strValue = SyncMsg.rspvalue().loginoutrspvalue().strvalue();
}

void ProtoHandler::LogoutRsp::Serializer(SyncMessage &SyncMsg) const
{
    Rsp::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::LoginoutRsp_T);
    SyncMsg.mutable_rspvalue()->mutable_loginoutrspvalue()->set_strvalue(m_strValue);
}

void ProtoHandler::ConfigInfoReq::UnSerializer(const SyncMessage &SyncMsg)
{
    Req::UnSerializer(SyncMsg);
    m_strSyncServiceName = SyncMsg.reqvalue().configinforeqvalue().strsyncservicename();
}

void ProtoHandler::ConfigInfoReq::Serializer(SyncMessage &SyncMsg) const
{
    Req::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::ConfigInfoReq_T);
    SyncMsg.mutable_reqvalue()->mutable_configinforeqvalue()->set_strsyncservicename(m_strSyncServiceName);
}

void ProtoHandler::ConfigInfoRsp::UnSerializer(const SyncMessage &SyncMsg)
{
    Rsp::UnSerializer(SyncMsg);
    m_strConfigJson = SyncMsg.rspvalue().configinforspvalue().strconfigjson();
}

void ProtoHandler::ConfigInfoRsp::Serializer(SyncMessage &SyncMsg) const
{
    Rsp::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::ConfigInfoRsp_T);
    SyncMsg.mutable_rspvalue()->mutable_configinforspvalue()->set_strconfigjson(m_strConfigJson);
}

void ProtoHandler::ShakehandReq::UnSerializer(const SyncMessage &SyncMsg)
{
    Req::UnSerializer(SyncMsg);
    m_strValue = SyncMsg.reqvalue().shakehandreqvalue().strvalue();
    m_status = (SyncServiceStatus)SyncMsg.reqvalue().shakehandreqvalue().status(); //////
}

void ProtoHandler::ShakehandReq::Serializer(SyncMessage &SyncMsg) const
{
    Req::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::ShakehandReq_T);
    SyncMsg.mutable_reqvalue()->mutable_shakehandreqvalue()->set_strvalue(m_strValue);
    SyncMsg.mutable_reqvalue()->mutable_shakehandreqvalue()->set_status((TDFS::MESSAGE::SyncServiceStatus)m_status); //////
}

void ProtoHandler::ShakehandRsp::UnSerializer(const SyncMessage &SyncMsg)
{
    Rsp::UnSerializer(SyncMsg);
    m_strValue = SyncMsg.rspvalue().shakehandrspvalue().strvalue();
}

void ProtoHandler::ShakehandRsp::Serializer(SyncMessage &SyncMsg) const
{
    Rsp::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::ShakehandRsp_T);
    SyncMsg.mutable_rspvalue()->mutable_shakehandrspvalue()->set_strvalue(m_strValue);
}

void ProtoHandler::GetSyncAddressReq::UnSerializer(const SyncMessage &SyncMsg)
{
    Req::UnSerializer(SyncMsg);
    m_strValue = SyncMsg.reqvalue().getsyncaddressreqvalue().strvalue();
}

void ProtoHandler::GetSyncAddressReq::Serializer(SyncMessage &SyncMsg) const
{
    Req::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::GetSyncAddressReq_T);
    SyncMsg.mutable_reqvalue()->mutable_getsyncaddressreqvalue()->set_strvalue(m_strValue);
}

void ProtoHandler::GetSyncAddressRsp::UnSerializer(const SyncMessage &SyncMsg)
{
    m_AddressList.clear();

    Rsp::UnSerializer(SyncMsg);
    int iCount = SyncMsg.rspvalue().getsyncaddressrspvalue().addressofsync_size();
    for (int i = 0; i < iCount; ++i)
    {
        Address adrs;
        adrs.strAddress = SyncMsg.rspvalue().getsyncaddressrspvalue().addressofsync(i).straddress();
        adrs.type = (Address::NodeType)SyncMsg.rspvalue().getsyncaddressrspvalue().addressofsync(i).type();
        adrs.AreaID = SyncMsg.rspvalue().getsyncaddressrspvalue().addressofsync(i).areaid();
        adrs.m_strStorageIP = SyncMsg.rspvalue().getsyncaddressrspvalue().addressofsync(i).strstorageip();
        adrs.m_strStoragePort = SyncMsg.rspvalue().getsyncaddressrspvalue().addressofsync(i).strstorageport();
        adrs.status = (SyncServiceStatus)SyncMsg.rspvalue().getsyncaddressrspvalue().addressofsync(i).status();
        m_AddressList.push_back(adrs);
    }

}

void ProtoHandler::GetSyncAddressRsp::Serializer(SyncMessage &SyncMsg) const
{
    Rsp::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::GetSyncAddressRsp_T);

    for (unsigned int i = 0; i < m_AddressList.size(); ++i)
    {
        SyncMsg.mutable_rspvalue()->mutable_getsyncaddressrspvalue()->add_addressofsync();
    }

    auto itBegin = m_AddressList.begin();
    auto itEnd = m_AddressList.end();
    int i = 0;
    while (itBegin != itEnd)
    {        
        SyncMsg.mutable_rspvalue()->mutable_getsyncaddressrspvalue()->mutable_addressofsync(i)->set_straddress(itBegin->strAddress);
        SyncMsg.mutable_rspvalue()->mutable_getsyncaddressrspvalue()->mutable_addressofsync(i)->set_type((TDFS::MESSAGE::Address_NodeType)itBegin->type);
        SyncMsg.mutable_rspvalue()->mutable_getsyncaddressrspvalue()->mutable_addressofsync(i)->set_areaid(itBegin->AreaID);
        SyncMsg.mutable_rspvalue()->mutable_getsyncaddressrspvalue()->mutable_addressofsync(i)->set_strstorageip(itBegin->m_strStorageIP);
        SyncMsg.mutable_rspvalue()->mutable_getsyncaddressrspvalue()->mutable_addressofsync(i)->set_strstorageport(itBegin->m_strStoragePort);
        SyncMsg.mutable_rspvalue()->mutable_getsyncaddressrspvalue()->mutable_addressofsync(i)->set_status((TDFS::MESSAGE::SyncServiceStatus)itBegin->status);

        ++i;
        ++itBegin;
    }
}

void ProtoHandler::SyncFileListPendingReq::UnSerializer(const SyncMessage &SyncMsg)
{
    m_FileInfoList.clear();

    Req::UnSerializer(SyncMsg);
    int iCount = SyncMsg.reqvalue().syncfilelistpendingreqvalue().fileinfolist_size();
    for (int i = 0; i < iCount; ++i)
    {
        FileInfo fi;
        fi.strCreatedate = SyncMsg.reqvalue().syncfilelistpendingreqvalue().fileinfolist(i).strcreatedate();
        fi.strFileID = SyncMsg.reqvalue().syncfilelistpendingreqvalue().fileinfolist(i).strfileid();
        fi.strFileMD5 = SyncMsg.reqvalue().syncfilelistpendingreqvalue().fileinfolist(i).strfilemd5();
        fi.strFileName = SyncMsg.reqvalue().syncfilelistpendingreqvalue().fileinfolist(i).strfilename();
        fi.uiFileSize = SyncMsg.reqvalue().syncfilelistpendingreqvalue().fileinfolist(i).uifilesize();

        m_FileInfoList.push_back(fi);
    }
}

void ProtoHandler::SyncFileListPendingReq::Serializer(SyncMessage &SyncMsg) const
{
    Req::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::SyncFileListPendingReq_T);

    for (unsigned int i = 0; i < m_FileInfoList.size(); ++i)
    {
        SyncMsg.mutable_reqvalue()->mutable_syncfilelistpendingreqvalue()->add_fileinfolist();
    }

    auto itBegin = m_FileInfoList.begin();
    auto itEnd = m_FileInfoList.end();
    int i = 0;
    while (itBegin != itEnd)
    {
        SyncMsg.mutable_reqvalue()->mutable_syncfilelistpendingreqvalue()->mutable_fileinfolist(i)->set_strcreatedate(itBegin->strCreatedate);
        SyncMsg.mutable_reqvalue()->mutable_syncfilelistpendingreqvalue()->mutable_fileinfolist(i)->set_strfileid(itBegin->strFileID);
        SyncMsg.mutable_reqvalue()->mutable_syncfilelistpendingreqvalue()->mutable_fileinfolist(i)->set_strfilemd5(itBegin->strFileMD5);
        SyncMsg.mutable_reqvalue()->mutable_syncfilelistpendingreqvalue()->mutable_fileinfolist(i)->set_strfilename(itBegin->strFileName);
        SyncMsg.mutable_reqvalue()->mutable_syncfilelistpendingreqvalue()->mutable_fileinfolist(i)->set_uifilesize(itBegin->uiFileSize);

        ++i;
        ++itBegin;
    }
}

void ProtoHandler::SyncFileListPendingRsp::UnSerializer(const SyncMessage &SyncMsg)
{
    m_FileInfoList.clear();

    Rsp::UnSerializer(SyncMsg);
    int iCount = SyncMsg.rspvalue().syncfilelistpendingrspvalue().fileinfolist_size();
    for (int i = 0; i < iCount; ++i)
    {
        FileInfo fi;
        fi.strCreatedate = SyncMsg.rspvalue().syncfilelistpendingrspvalue().fileinfolist(i).strcreatedate();
        fi.strFileID = SyncMsg.rspvalue().syncfilelistpendingrspvalue().fileinfolist(i).strfileid();
        fi.strFileMD5 = SyncMsg.rspvalue().syncfilelistpendingrspvalue().fileinfolist(i).strfilemd5();
        fi.strFileName = SyncMsg.rspvalue().syncfilelistpendingrspvalue().fileinfolist(i).strfilename();
        fi.uiFileSize = SyncMsg.rspvalue().syncfilelistpendingrspvalue().fileinfolist(i).uifilesize();

        m_FileInfoList.push_back(fi);
    }
}

void ProtoHandler::SyncFileListPendingRsp::Serializer(SyncMessage &SyncMsg) const
{
    Rsp::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::SyncFileListPendingRsp_T);

    for (unsigned int i = 0; i < m_FileInfoList.size(); ++i)
    {
        SyncMsg.mutable_rspvalue()->mutable_syncfilelistpendingrspvalue()->add_fileinfolist();
    }

    auto itBegin = m_FileInfoList.begin();
    auto itEnd = m_FileInfoList.end();
    int i = 0;
    while (itBegin != itEnd)
    {
        SyncMsg.mutable_rspvalue()->mutable_syncfilelistpendingrspvalue()->mutable_fileinfolist(i)->set_strcreatedate(itBegin->strCreatedate);
        SyncMsg.mutable_rspvalue()->mutable_syncfilelistpendingrspvalue()->mutable_fileinfolist(i)->set_strfileid(itBegin->strFileID);
        SyncMsg.mutable_rspvalue()->mutable_syncfilelistpendingrspvalue()->mutable_fileinfolist(i)->set_strfilemd5(itBegin->strFileMD5);
        SyncMsg.mutable_rspvalue()->mutable_syncfilelistpendingrspvalue()->mutable_fileinfolist(i)->set_strfilename(itBegin->strFileName);
        SyncMsg.mutable_rspvalue()->mutable_syncfilelistpendingrspvalue()->mutable_fileinfolist(i)->set_uifilesize(itBegin->uiFileSize);

        ++i;
        ++itBegin;
    }
}

void ProtoHandler::GetFileInfoReq::UnSerializer(const SyncMessage &SyncMsg)
{
    Req::UnSerializer(SyncMsg);
    m_strFileID = SyncMsg.reqvalue().getfileinforeqvalue().strfileid();
}

void ProtoHandler::GetFileInfoReq::Serializer(SyncMessage &SyncMsg) const
{
    Req::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::GetFileInfoReq_T);
    SyncMsg.mutable_reqvalue()->mutable_getfileinforeqvalue()->set_strfileid(m_strFileID);
}

void ProtoHandler::GetFileInfoRsp::UnSerializer(const SyncMessage &SyncMsg)
{
    Rsp::UnSerializer(SyncMsg);
    m_fileinfo.strCreatedate = SyncMsg.rspvalue().getfileinforspvalue().fileinfo().strcreatedate();
    m_fileinfo.strFileID = SyncMsg.rspvalue().getfileinforspvalue().fileinfo().strfileid();
    m_fileinfo.strFileMD5 = SyncMsg.rspvalue().getfileinforspvalue().fileinfo().strfilemd5();
    m_fileinfo.strFileName = SyncMsg.rspvalue().getfileinforspvalue().fileinfo().strfilename();
    m_fileinfo.uiFileSize = SyncMsg.rspvalue().getfileinforspvalue().fileinfo().uifilesize();    
}

void ProtoHandler::GetFileInfoRsp::Serializer(SyncMessage &SyncMsg) const
{
    Rsp::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::GetFileInfoRsp_T);
    SyncMsg.mutable_rspvalue()->mutable_getfileinforspvalue()->mutable_fileinfo()->set_strcreatedate(m_fileinfo.strCreatedate);
    SyncMsg.mutable_rspvalue()->mutable_getfileinforspvalue()->mutable_fileinfo()->set_strfileid(m_fileinfo.strFileID);
    SyncMsg.mutable_rspvalue()->mutable_getfileinforspvalue()->mutable_fileinfo()->set_strfilemd5(m_fileinfo.strFileMD5);
    SyncMsg.mutable_rspvalue()->mutable_getfileinforspvalue()->mutable_fileinfo()->set_strfilename(m_fileinfo.strFileName);
    SyncMsg.mutable_rspvalue()->mutable_getfileinforspvalue()->mutable_fileinfo()->set_uifilesize(m_fileinfo.uiFileSize);    
}

void ProtoHandler::ControlCMDReq::UnSerializer(const SyncMessage &SyncMsg)
{
    Req::UnSerializer(SyncMsg);
    m_uiCMD = SyncMsg.reqvalue().controlcmdreqvalue().uicmd();
    m_strCMDMsg = SyncMsg.reqvalue().controlcmdreqvalue().strcmdmsg();
    m_blNeedActiveReporting = SyncMsg.reqvalue().controlcmdreqvalue().blneedactivereporting();

}

void ProtoHandler::ControlCMDReq::Serializer(SyncMessage &SyncMsg) const
{
    Req::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::ControlCMDReq_T);
    SyncMsg.mutable_reqvalue()->mutable_controlcmdreqvalue()->set_uicmd(m_uiCMD);
    SyncMsg.mutable_reqvalue()->mutable_controlcmdreqvalue()->set_strcmdmsg(m_strCMDMsg);
    SyncMsg.mutable_reqvalue()->mutable_controlcmdreqvalue()->set_blneedactivereporting(m_blNeedActiveReporting);
}

void ProtoHandler::ControlCMDRsp::UnSerializer(const SyncMessage &SyncMsg)
{
    Rsp::UnSerializer(SyncMsg);
    m_uiCMD = SyncMsg.rspvalue().controlcmdrspvalue().uicmd();
    m_blNeedActiveReporting = SyncMsg.rspvalue().controlcmdrspvalue().blneedactivereporting();
}

void ProtoHandler::ControlCMDRsp::Serializer(SyncMessage &SyncMsg) const
{
    Rsp::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::ControlCMDRsp_T);
    SyncMsg.mutable_rspvalue()->mutable_controlcmdrspvalue()->set_uicmd(m_uiCMD);
    SyncMsg.mutable_rspvalue()->mutable_controlcmdrspvalue()->set_blneedactivereporting(m_blNeedActiveReporting);
}

void ProtoHandler::FullSyncReq::UnSerializer(const SyncMessage &SyncMsg)
{
    Req::UnSerializer(SyncMsg);
    m_strValue = SyncMsg.reqvalue().fullsyncreqvalue().strvalue();
}

void ProtoHandler::FullSyncReq::Serializer(SyncMessage &SyncMsg) const
{
    Req::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::FullSyncReq_T);
    SyncMsg.mutable_reqvalue()->mutable_fullsyncreqvalue()->set_strvalue(m_strValue);
}

void ProtoHandler::FullSyncRsp::UnSerializer(const SyncMessage &SyncMsg)
{
    m_FileInfoList.clear();

    Rsp::UnSerializer(SyncMsg);
    int iCount = SyncMsg.rspvalue().fullsyncrspvalue().fileinfolist_size();
    for (int i = 0; i < iCount; ++i)
    {
        FileInfo fi;
        fi.strCreatedate = SyncMsg.rspvalue().fullsyncrspvalue().fileinfolist(i).strcreatedate();
        fi.strFileID = SyncMsg.rspvalue().fullsyncrspvalue().fileinfolist(i).strfileid();
        fi.strFileMD5 = SyncMsg.rspvalue().fullsyncrspvalue().fileinfolist(i).strfilemd5();
        fi.strFileName = SyncMsg.rspvalue().fullsyncrspvalue().fileinfolist(i).strfilename();
        fi.uiFileSize = SyncMsg.rspvalue().fullsyncrspvalue().fileinfolist(i).uifilesize();

        m_FileInfoList.push_back(fi);
    }

}

void ProtoHandler::FullSyncRsp::Serializer(SyncMessage &SyncMsg) const
{
    Rsp::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::FullSyncRsp_T);

    for (unsigned int i = 0; i < m_FileInfoList.size(); ++i)
    {
        SyncMsg.mutable_rspvalue()->mutable_fullsyncrspvalue()->add_fileinfolist();
    }

    auto itBegin = m_FileInfoList.begin();
    auto itEnd = m_FileInfoList.end();
    int i = 0;
    while (itBegin != itEnd)
    {
        SyncMsg.mutable_rspvalue()->mutable_fullsyncrspvalue()->mutable_fileinfolist(i)->set_strcreatedate(itBegin->strCreatedate);
        SyncMsg.mutable_rspvalue()->mutable_fullsyncrspvalue()->mutable_fileinfolist(i)->set_strfileid(itBegin->strFileID);
        SyncMsg.mutable_rspvalue()->mutable_fullsyncrspvalue()->mutable_fileinfolist(i)->set_strfilemd5(itBegin->strFileMD5);
        SyncMsg.mutable_rspvalue()->mutable_fullsyncrspvalue()->mutable_fileinfolist(i)->set_strfilename(itBegin->strFileName);
        SyncMsg.mutable_rspvalue()->mutable_fullsyncrspvalue()->mutable_fileinfolist(i)->set_uifilesize(itBegin->uiFileSize);

        ++i;
        ++itBegin;
    }
}

void ProtoHandler::FullSyncConform::UnSerializer(const SyncMessage &SyncMsg)
{
    m_FileInfoList.clear();

    Req::UnSerializer(SyncMsg);
    int iCount = SyncMsg.reqvalue().fullsyncconformvalue().fileinfolist_size();
    for (int i = 0; i < iCount; ++i)
    {
        FileInfo fi;
        fi.strCreatedate = SyncMsg.reqvalue().fullsyncconformvalue().fileinfolist(i).strcreatedate();
        fi.strFileID = SyncMsg.reqvalue().fullsyncconformvalue().fileinfolist(i).strfileid();
        fi.strFileMD5 = SyncMsg.reqvalue().fullsyncconformvalue().fileinfolist(i).strfilemd5();
        fi.strFileName = SyncMsg.reqvalue().fullsyncconformvalue().fileinfolist(i).strfilename();
        fi.uiFileSize = SyncMsg.reqvalue().fullsyncconformvalue().fileinfolist(i).uifilesize();

        m_FileInfoList.push_back(fi);
    }

    m_uiFlag = SyncMsg.reqvalue().fullsyncconformvalue().uiflag();
}

void ProtoHandler::FullSyncConform::Serializer(SyncMessage &SyncMsg) const
{
    Req::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::FullSyncConform_T);

    for (unsigned int i = 0; i < m_FileInfoList.size(); ++i)
    {
        SyncMsg.mutable_reqvalue()->mutable_fullsyncconformvalue()->add_fileinfolist();
    }

    auto itBegin = m_FileInfoList.begin();
    auto itEnd = m_FileInfoList.end();
    int i = 0;
    while (itBegin != itEnd)
    {
        SyncMsg.mutable_reqvalue()->mutable_fullsyncconformvalue()->mutable_fileinfolist(i)->set_strcreatedate(itBegin->strCreatedate);
        SyncMsg.mutable_reqvalue()->mutable_fullsyncconformvalue()->mutable_fileinfolist(i)->set_strfileid(itBegin->strFileID);
        SyncMsg.mutable_reqvalue()->mutable_fullsyncconformvalue()->mutable_fileinfolist(i)->set_strfilemd5(itBegin->strFileMD5);
        SyncMsg.mutable_reqvalue()->mutable_fullsyncconformvalue()->mutable_fileinfolist(i)->set_strfilename(itBegin->strFileName);
        SyncMsg.mutable_reqvalue()->mutable_fullsyncconformvalue()->mutable_fileinfolist(i)->set_uifilesize(itBegin->uiFileSize);

        ++i;
        ++itBegin;
    }

    SyncMsg.mutable_reqvalue()->mutable_fullsyncconformvalue()->set_uiflag(m_uiFlag);
}

void ProtoHandler::ActiveReportingReq::UnSerializer(const SyncMessage &SyncMsg)
{
    Req::UnSerializer(SyncMsg);
    m_uiTotal = SyncMsg.reqvalue().activereportingreqvalue().uitotal();
    m_uiCurrentCompleted = SyncMsg.reqvalue().activereportingreqvalue().uicurrentcompleted();

    m_CurrentCompletedFileInfo.strCreatedate = SyncMsg.reqvalue().activereportingreqvalue().currentcompletedfileinfo().strcreatedate();
    m_CurrentCompletedFileInfo.strFileID = SyncMsg.reqvalue().activereportingreqvalue().currentcompletedfileinfo().strfileid();
    m_CurrentCompletedFileInfo.strFileMD5 = SyncMsg.reqvalue().activereportingreqvalue().currentcompletedfileinfo().strfilemd5();
    m_CurrentCompletedFileInfo.strFileName = SyncMsg.reqvalue().activereportingreqvalue().currentcompletedfileinfo().strfilename();
    m_CurrentCompletedFileInfo.uiFileSize = SyncMsg.reqvalue().activereportingreqvalue().currentcompletedfileinfo().uifilesize();

}

void ProtoHandler::ActiveReportingReq::Serializer(SyncMessage &SyncMsg) const
{
    Req::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::ActiveReportingReq_T);
    SyncMsg.mutable_reqvalue()->mutable_activereportingreqvalue()->set_uitotal(m_uiTotal);
    SyncMsg.mutable_reqvalue()->mutable_activereportingreqvalue()->set_uicurrentcompleted(m_uiCurrentCompleted);

    SyncMsg.mutable_reqvalue()->mutable_activereportingreqvalue()->mutable_currentcompletedfileinfo()->set_strcreatedate(m_CurrentCompletedFileInfo.strCreatedate);
    SyncMsg.mutable_reqvalue()->mutable_activereportingreqvalue()->mutable_currentcompletedfileinfo()->set_strfileid(m_CurrentCompletedFileInfo.strFileID);
    SyncMsg.mutable_reqvalue()->mutable_activereportingreqvalue()->mutable_currentcompletedfileinfo()->set_strfilemd5(m_CurrentCompletedFileInfo.strFileMD5);
    SyncMsg.mutable_reqvalue()->mutable_activereportingreqvalue()->mutable_currentcompletedfileinfo()->set_strfilename(m_CurrentCompletedFileInfo.strFileName);
    SyncMsg.mutable_reqvalue()->mutable_activereportingreqvalue()->mutable_currentcompletedfileinfo()->set_uifilesize(m_CurrentCompletedFileInfo.uiFileSize);

}

void ProtoHandler::ActiveReportingRsp::UnSerializer(const SyncMessage &SyncMsg)
{
    Rsp::UnSerializer(SyncMsg);
    m_strValue = SyncMsg.rspvalue().activereportingrspvalue().strvalue();

}

void ProtoHandler::ActiveReportingRsp::Serializer(SyncMessage &SyncMsg) const
{
    Rsp::Serializer(SyncMsg);
    SyncMsg.set_type(TDFS::MESSAGE::MsgType::ActiveReportingRsp_T);
    SyncMsg.mutable_rspvalue()->mutable_activereportingrspvalue()->set_strvalue(m_strValue);
}
