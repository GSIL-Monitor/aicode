#include "InteractiveProtoManagementHandler.h"

using namespace Interactive::Message;

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

    ManagementInteractiveMessage Msg;
    Msg.Clear();

    pReq->Serializer(Msg);

    Msg.SerializeToString(&strOutput);

    return true;
};

template<typename T, typename TT> bool UnSerializerT(const ManagementInteractiveMessage &MngInteractiveMsg, TT &req)
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

    pReq->UnSerializer(MngInteractiveMsg);

    return true;
}

void SerializeClusterList(const std::list<InteractiveProtoManagementHandler::Cluster> &clusterInfoList,
    ::google::protobuf::RepeatedPtrField<::Management::Interactive::Message::Cluster> *pDstClusterInfoList)
{
    int i = 0;
    auto itBegin = clusterInfoList.begin();
    auto itEnd = clusterInfoList.end();
    while (itBegin != itEnd)
    {
        pDstClusterInfoList->Add();
        auto pDstClusterInfo = pDstClusterInfoList->Mutable(i);
        pDstClusterInfo->set_strclusterid(itBegin->m_strClusterID);
        pDstClusterInfo->set_strclusteraddress(itBegin->m_strClusterAddress);
        pDstClusterInfo->set_strmanagementaddress(itBegin->m_strManagementAddress);
        pDstClusterInfo->set_straliasname(itBegin->m_strAliasname);
        pDstClusterInfo->set_strcreatedate(itBegin->m_strCreatedate);
        pDstClusterInfo->set_uistatus(itBegin->m_uiStatus);

        ++i;
        ++itBegin;
    }
}

void UnSerializeClusterList(std::list<InteractiveProtoManagementHandler::Cluster> &clusterInfoList,
    const ::google::protobuf::RepeatedPtrField<::Management::Interactive::Message::Cluster> &srcClusterInfoList)
{
    clusterInfoList.clear();
    int iCount = srcClusterInfoList.size();
    for (int i = 0; i < iCount; ++i)
    {
        auto srcClusterInfo = srcClusterInfoList.Get(i);
        InteractiveProtoManagementHandler::Cluster clusterInfo;
        clusterInfo.m_strClusterID = srcClusterInfo.strclusterid();
        clusterInfo.m_strClusterAddress = srcClusterInfo.strclusteraddress();
        clusterInfo.m_strManagementAddress = srcClusterInfo.strmanagementaddress();
        clusterInfo.m_strAliasname = srcClusterInfo.straliasname();
        clusterInfo.m_strCreatedate = srcClusterInfo.strcreatedate();
        clusterInfo.m_uiStatus = srcClusterInfo.uistatus();

        clusterInfoList.push_back(std::move(clusterInfo));
    }
}

void SerializeAccessedDeviceList(const std::list<InteractiveProtoManagementHandler::AccessedDevice> &accessedDeviceList,
    ::google::protobuf::RepeatedPtrField<::Management::Interactive::Message::AccessedDevice>* pDstAccessedDeviceList)
{
    int i = 0;
    auto itBegin = accessedDeviceList.begin();
    auto itEnd = accessedDeviceList.end();
    while (itBegin != itEnd)
    {
        pDstAccessedDeviceList->Add();
        auto pDstAccessedDevice = pDstAccessedDeviceList->Mutable(i);
        pDstAccessedDevice->set_strdeviceid(itBegin->m_strDeviceID);
        pDstAccessedDevice->set_strdevicename(itBegin->m_strDeviceName);
        pDstAccessedDevice->set_uidevicetype(itBegin->m_uiDeviceType);
        pDstAccessedDevice->set_strlogintime(itBegin->m_strLoginTime);
        pDstAccessedDevice->set_strlogouttime(itBegin->m_strLogoutTime);

        ++i;
        ++itBegin;
    }
}

void UnSerializeAccessedDeviceList(std::list<InteractiveProtoManagementHandler::AccessedDevice> &accessedDeviceList,
    const ::google::protobuf::RepeatedPtrField<::Management::Interactive::Message::AccessedDevice> &srcAccessedDeviceList)
{
    accessedDeviceList.clear();
    int iCount = srcAccessedDeviceList.size();
    for (int i = 0; i < iCount; ++i)
    {
        auto srcClusterInfo = srcAccessedDeviceList.Get(i);
        InteractiveProtoManagementHandler::AccessedDevice accessedDevice;
        accessedDevice.m_strDeviceID = srcClusterInfo.strdeviceid();
        accessedDevice.m_strDeviceName = srcClusterInfo.strdevicename();
        accessedDevice.m_uiDeviceType = srcClusterInfo.uidevicetype();
        accessedDevice.m_strLoginTime = srcClusterInfo.strlogintime();
        accessedDevice.m_strLogoutTime = srcClusterInfo.strlogouttime();

        accessedDeviceList.push_back(std::move(accessedDevice));
    }
}

void SerializeAccessedUserList(const std::list<InteractiveProtoManagementHandler::AccessedUser> &accessedUserList,
    ::google::protobuf::RepeatedPtrField<::Management::Interactive::Message::AccessedUser>* pDstAccessedUserList)
{
    int i = 0;
    auto itBegin = accessedUserList.begin();
    auto itEnd = accessedUserList.end();
    while (itBegin != itEnd)
    {
        pDstAccessedUserList->Add();
        auto pDstAccessedUser = pDstAccessedUserList->Mutable(i);
        pDstAccessedUser->set_struserid(itBegin->m_strUserID);
        pDstAccessedUser->set_strusername(itBegin->m_strUserName);
        pDstAccessedUser->set_struseraliasname(itBegin->m_strUserAliasname);
        pDstAccessedUser->set_uiclienttype(itBegin->m_uiClientType);
        pDstAccessedUser->set_strlogintime(itBegin->m_strLoginTime);
        pDstAccessedUser->set_strlogouttime(itBegin->m_strLogoutTime);

        ++i;
        ++itBegin;
    }
}

void UnSerializeAccessedUserList(std::list<InteractiveProtoManagementHandler::AccessedUser> &accessedUserList,
    const ::google::protobuf::RepeatedPtrField<::Management::Interactive::Message::AccessedUser> &srcAccessedUserList)
{
    accessedUserList.clear();
    int iCount = srcAccessedUserList.size();
    for (int i = 0; i < iCount; ++i)
    {
        auto srcClusterInfo = srcAccessedUserList.Get(i);
        InteractiveProtoManagementHandler::AccessedUser accessedUser;
        accessedUser.m_strUserID = srcClusterInfo.struserid();
        accessedUser.m_strUserName = srcClusterInfo.strusername();
        accessedUser.m_strUserAliasname = srcClusterInfo.struseraliasname();
        accessedUser.m_uiClientType = srcClusterInfo.uiclienttype();
        accessedUser.m_strLoginTime = srcClusterInfo.strlogintime();
        accessedUser.m_strLogoutTime = srcClusterInfo.strlogouttime();

        accessedUserList.push_back(std::move(accessedUser));
    }
}


InteractiveProtoManagementHandler::InteractiveProtoManagementHandler()
{
    SerializeHandler handler;

    handler.Szr = boost::bind(&InteractiveProtoManagementHandler::AddClusterReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoManagementHandler::AddClusterReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Management::Interactive::Message::ManagementMsgType::AddClusterReq_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoManagementHandler::AddClusterRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoManagementHandler::AddClusterRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Management::Interactive::Message::ManagementMsgType::AddClusterRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoManagementHandler::DeleteClusterReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoManagementHandler::DeleteClusterReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Management::Interactive::Message::ManagementMsgType::DeleteClusterReq_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoManagementHandler::DeleteClusterRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoManagementHandler::DeleteClusterRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Management::Interactive::Message::ManagementMsgType::DeleteClusterRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoManagementHandler::ModifyClusterReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoManagementHandler::ModifyClusterReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Management::Interactive::Message::ManagementMsgType::ModifyClusterReq_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoManagementHandler::ModifyClusterRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoManagementHandler::ModifyClusterRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Management::Interactive::Message::ManagementMsgType::ModifyClusterRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoManagementHandler::QueryClusterInfoReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoManagementHandler::QueryClusterInfoReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Management::Interactive::Message::ManagementMsgType::QueryClusterInfoReq_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoManagementHandler::QueryClusterInfoRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoManagementHandler::QueryClusterInfoRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Management::Interactive::Message::ManagementMsgType::QueryClusterInfoRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoManagementHandler::ShakehandClusterReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoManagementHandler::ShakehandClusterReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Management::Interactive::Message::ManagementMsgType::ShakehandClusterReq_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoManagementHandler::ShakehandClusterRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoManagementHandler::ShakehandClusterRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Management::Interactive::Message::ManagementMsgType::ShakehandClusterRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoManagementHandler::QueryAllClusterReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoManagementHandler::QueryAllClusterReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Management::Interactive::Message::ManagementMsgType::QueryAllClusterReq_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoManagementHandler::QueryAllClusterRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoManagementHandler::QueryAllClusterRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Management::Interactive::Message::ManagementMsgType::QueryAllClusterRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoManagementHandler::QueryClusterDeviceReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoManagementHandler::QueryClusterDeviceReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Management::Interactive::Message::ManagementMsgType::QueryClusterDeviceReq_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoManagementHandler::QueryClusterDeviceRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoManagementHandler::QueryClusterDeviceRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Management::Interactive::Message::ManagementMsgType::QueryClusterDeviceRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoManagementHandler::QueryClusterUserReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoManagementHandler::QueryClusterUserReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Management::Interactive::Message::ManagementMsgType::QueryClusterUserReq_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoManagementHandler::QueryClusterUserRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoManagementHandler::QueryClusterUserRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Management::Interactive::Message::ManagementMsgType::QueryClusterUserRsp_T, handler));

    //////////////////////////////////////////////////
}

InteractiveProtoManagementHandler::~InteractiveProtoManagementHandler()
{

}

bool InteractiveProtoManagementHandler::GetManagementMsgType(const std::string &strData, ManagementMsgType &msgtype)
{
    ManagementInteractiveMessage Msg;
    Msg.Clear();
    if (!Msg.ParseFromString(strData))
    {
        return false;
    }

    msgtype = (ManagementMsgType)Msg.type();

    return true;
}

bool InteractiveProtoManagementHandler::SerializeReq(const Request &req, std::string &strOutput)
{
    const int iType = req.m_MngMsgType;

    auto itFind = m_ReqAndRspHandlerMap.find(iType);
    if (m_ReqAndRspHandlerMap.end() == itFind)
    {
        return false;
    }

    return itFind->second.Szr(req, strOutput);
}

bool InteractiveProtoManagementHandler::UnSerializeReq(const std::string &strData, Request &req)
{
    ManagementInteractiveMessage Msg;
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

bool InteractiveProtoManagementHandler::SerializeRsp(const Response &rsp, std::string &strOutput)
{
    return SerializeReq(rsp, strOutput);
}

bool InteractiveProtoManagementHandler::UnSerializeRsp(const std::string &strData, Response &rsp)
{
    return UnSerializeReq(strData, rsp);
}

bool InteractiveProtoManagementHandler::UnSerializeReqBase(const std::string &strData, Request &req)
{
    ManagementInteractiveMessage Msg;
    Msg.Clear();
    if (!Msg.ParseFromString(strData))
    {
        return false;
    }

    req.UnSerializer(Msg);

    return true;
}

bool InteractiveProtoManagementHandler::AddClusterReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<AddClusterReq, Request>(req, strOutput);
}

bool InteractiveProtoManagementHandler::AddClusterReq_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req)
{
    return UnSerializerT<AddClusterReq, Request>(MngInteractiveMsg, req);
}

bool InteractiveProtoManagementHandler::AddClusterRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<AddClusterRsp, Request>(rsp, strOutput);
}

bool InteractiveProtoManagementHandler::AddClusterRsp_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &rsp)
{
    return UnSerializerT<AddClusterRsp, Request>(MngInteractiveMsg, rsp);
}

bool InteractiveProtoManagementHandler::DeleteClusterReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<DeleteClusterReq, Request>(req, strOutput);
}

bool InteractiveProtoManagementHandler::DeleteClusterReq_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req)
{
    return UnSerializerT<DeleteClusterReq, Request>(MngInteractiveMsg, req);
}

bool InteractiveProtoManagementHandler::DeleteClusterRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<DeleteClusterRsp, Request>(rsp, strOutput);
}

bool InteractiveProtoManagementHandler::DeleteClusterRsp_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &rsp)
{
    return UnSerializerT<DeleteClusterRsp, Request>(MngInteractiveMsg, rsp);
}

bool InteractiveProtoManagementHandler::ModifyClusterReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<ModifyClusterReq, Request>(req, strOutput);
}

bool InteractiveProtoManagementHandler::ModifyClusterReq_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req)
{
    return UnSerializerT<ModifyClusterReq, Request>(MngInteractiveMsg, req);
}

bool InteractiveProtoManagementHandler::ModifyClusterRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<ModifyClusterRsp, Request>(rsp, strOutput);
}

bool InteractiveProtoManagementHandler::ModifyClusterRsp_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &rsp)
{
    return UnSerializerT<ModifyClusterRsp, Request>(MngInteractiveMsg, rsp);
}

bool InteractiveProtoManagementHandler::QueryClusterInfoReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryClusterInfoReq, Request>(req, strOutput);
}

bool InteractiveProtoManagementHandler::QueryClusterInfoReq_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req)
{
    return UnSerializerT<QueryClusterInfoReq, Request>(MngInteractiveMsg, req);
}

bool InteractiveProtoManagementHandler::QueryClusterInfoRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryClusterInfoRsp, Request>(rsp, strOutput);
}

bool InteractiveProtoManagementHandler::QueryClusterInfoRsp_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &rsp)
{
    return UnSerializerT<QueryClusterInfoRsp, Request>(MngInteractiveMsg, rsp);
}

bool InteractiveProtoManagementHandler::ShakehandClusterReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<ShakehandClusterReq, Request>(req, strOutput);
}

bool InteractiveProtoManagementHandler::ShakehandClusterReq_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req)
{
    return UnSerializerT<ShakehandClusterReq, Request>(MngInteractiveMsg, req);
}

bool InteractiveProtoManagementHandler::ShakehandClusterRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<ShakehandClusterRsp, Request>(rsp, strOutput);
}

bool InteractiveProtoManagementHandler::ShakehandClusterRsp_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &rsp)
{
    return UnSerializerT<ShakehandClusterRsp, Request>(MngInteractiveMsg, rsp);
}

bool InteractiveProtoManagementHandler::QueryAllClusterReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryAllClusterReq, Request>(req, strOutput);
}

bool InteractiveProtoManagementHandler::QueryAllClusterReq_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req)
{
    return UnSerializerT<QueryAllClusterReq, Request>(MngInteractiveMsg, req);
}

bool InteractiveProtoManagementHandler::QueryAllClusterRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryAllClusterRsp, Request>(rsp, strOutput);
}

bool InteractiveProtoManagementHandler::QueryAllClusterRsp_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &rsp)
{
    return UnSerializerT<QueryAllClusterRsp, Request>(MngInteractiveMsg, rsp);
}

bool InteractiveProtoManagementHandler::QueryClusterDeviceReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryClusterDeviceReq, Request>(req, strOutput);
}

bool InteractiveProtoManagementHandler::QueryClusterDeviceReq_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req)
{
    return UnSerializerT<QueryClusterDeviceReq, Request>(MngInteractiveMsg, req);
}

bool InteractiveProtoManagementHandler::QueryClusterDeviceRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryClusterDeviceRsp, Request>(rsp, strOutput);
}

bool InteractiveProtoManagementHandler::QueryClusterDeviceRsp_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &rsp)
{
    return UnSerializerT<QueryClusterDeviceRsp, Request>(MngInteractiveMsg, rsp);
}

bool InteractiveProtoManagementHandler::QueryClusterUserReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryClusterUserReq, Request>(req, strOutput);
}

bool InteractiveProtoManagementHandler::QueryClusterUserReq_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &req)
{
    return UnSerializerT<QueryClusterUserReq, Request>(MngInteractiveMsg, req);
}

bool InteractiveProtoManagementHandler::QueryClusterUserRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryClusterUserRsp, Request>(rsp, strOutput);
}

bool InteractiveProtoManagementHandler::QueryClusterUserRsp_UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg, Request &rsp)
{
    return UnSerializerT<QueryClusterUserRsp, Request>(MngInteractiveMsg, rsp);
}


void InteractiveProtoManagementHandler::Request::Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const
{
    MngInteractiveMsg.set_uimsgseq(m_uiMsgSeq);
}

void InteractiveProtoManagementHandler::Request::UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg)
{
    m_MngMsgType = (ManagementMsgType)MngInteractiveMsg.type();
    m_uiMsgSeq = MngInteractiveMsg.uimsgseq();
}

void InteractiveProtoManagementHandler::Response::Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const
{
    Request::Serializer(MngInteractiveMsg);
    MngInteractiveMsg.mutable_rspvalue()->set_iretcode(m_iRetcode);
    MngInteractiveMsg.mutable_rspvalue()->set_strretmsg(m_strRetMsg);
}
void InteractiveProtoManagementHandler::Response::UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg)
{
    Request::UnSerializer(MngInteractiveMsg);
    m_iRetcode = MngInteractiveMsg.rspvalue().iretcode();
    m_strRetMsg = MngInteractiveMsg.rspvalue().strretmsg();
}


void InteractiveProtoManagementHandler::AddClusterReq::Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const
{
    Request::Serializer(MngInteractiveMsg);
    MngInteractiveMsg.set_type(Management::Interactive::Message::ManagementMsgType::AddClusterReq_T);

    auto clusterInfo = MngInteractiveMsg.mutable_reqvalue()->mutable_addclusterreq_value()->mutable_clusterinfo();

    clusterInfo->set_strclusterid(m_clusterInfo.m_strClusterID);
    clusterInfo->set_strclusteraddress(m_clusterInfo.m_strClusterAddress);
    clusterInfo->set_strmanagementaddress(m_clusterInfo.m_strManagementAddress);
    clusterInfo->set_straliasname(m_clusterInfo.m_strAliasname);
    clusterInfo->set_strcreatedate(m_clusterInfo.m_strCreatedate);
    clusterInfo->set_uistatus(m_clusterInfo.m_uiStatus);
}

void InteractiveProtoManagementHandler::AddClusterReq::UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg)
{
    Request::UnSerializer(MngInteractiveMsg);
    m_clusterInfo.m_strClusterID = MngInteractiveMsg.reqvalue().addclusterreq_value().clusterinfo().strclusterid();
    m_clusterInfo.m_strClusterAddress = MngInteractiveMsg.reqvalue().addclusterreq_value().clusterinfo().strclusteraddress();
    m_clusterInfo.m_strManagementAddress = MngInteractiveMsg.reqvalue().addclusterreq_value().clusterinfo().strmanagementaddress();
    m_clusterInfo.m_strAliasname = MngInteractiveMsg.reqvalue().addclusterreq_value().clusterinfo().straliasname();
    m_clusterInfo.m_strCreatedate = MngInteractiveMsg.reqvalue().addclusterreq_value().clusterinfo().strcreatedate();
    m_clusterInfo.m_uiStatus = MngInteractiveMsg.reqvalue().addclusterreq_value().clusterinfo().uistatus();
}

void InteractiveProtoManagementHandler::AddClusterRsp::Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const
{
    Response::Serializer(MngInteractiveMsg);
    MngInteractiveMsg.set_type(Management::Interactive::Message::ManagementMsgType::AddClusterRsp_T);
    MngInteractiveMsg.mutable_rspvalue()->mutable_addclusterrsp_value()->set_strclusterid(m_strClusterID);
}

void InteractiveProtoManagementHandler::AddClusterRsp::UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg)
{
    Response::UnSerializer(MngInteractiveMsg);
    m_strClusterID = MngInteractiveMsg.rspvalue().addclusterrsp_value().strclusterid();
}

void InteractiveProtoManagementHandler::DeleteClusterReq::Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const
{
    Request::Serializer(MngInteractiveMsg);
    MngInteractiveMsg.set_type(Management::Interactive::Message::ManagementMsgType::DeleteClusterReq_T);

    MngInteractiveMsg.mutable_reqvalue()->mutable_deleteclusterreq_value()->set_strclusterid(m_strClusterID);
}

void InteractiveProtoManagementHandler::DeleteClusterReq::UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg)
{
    Request::UnSerializer(MngInteractiveMsg);
    m_strClusterID = MngInteractiveMsg.reqvalue().deleteclusterreq_value().strclusterid();
}

void InteractiveProtoManagementHandler::DeleteClusterRsp::Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const
{
    Response::Serializer(MngInteractiveMsg);
    MngInteractiveMsg.set_type(Management::Interactive::Message::ManagementMsgType::DeleteClusterRsp_T);

    MngInteractiveMsg.mutable_rspvalue()->mutable_deleteclusterrsp_value()->set_strvalue(m_strValue);
}

void InteractiveProtoManagementHandler::DeleteClusterRsp::UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg)
{
    Response::UnSerializer(MngInteractiveMsg);

    m_strValue = MngInteractiveMsg.rspvalue().deleteclusterrsp_value().strvalue();
}

void InteractiveProtoManagementHandler::ModifyClusterReq::Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const
{
    Request::Serializer(MngInteractiveMsg);
    MngInteractiveMsg.set_type(Management::Interactive::Message::ManagementMsgType::ModifyClusterReq_T);

    auto clusterInfo = MngInteractiveMsg.mutable_reqvalue()->mutable_modifyclusterreq_value()->mutable_clusterinfo();

    clusterInfo->set_strclusterid(m_clusterInfo.m_strClusterID);
    clusterInfo->set_strclusteraddress(m_clusterInfo.m_strClusterAddress);
    clusterInfo->set_strmanagementaddress(m_clusterInfo.m_strManagementAddress);
    clusterInfo->set_straliasname(m_clusterInfo.m_strAliasname);
}

void InteractiveProtoManagementHandler::ModifyClusterReq::UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg)
{
    Request::UnSerializer(MngInteractiveMsg);
    m_clusterInfo.m_strClusterID = MngInteractiveMsg.reqvalue().modifyclusterreq_value().clusterinfo().strclusterid();
    m_clusterInfo.m_strClusterAddress = MngInteractiveMsg.reqvalue().modifyclusterreq_value().clusterinfo().strclusteraddress();
    m_clusterInfo.m_strManagementAddress = MngInteractiveMsg.reqvalue().modifyclusterreq_value().clusterinfo().strmanagementaddress();
    m_clusterInfo.m_strAliasname = MngInteractiveMsg.reqvalue().modifyclusterreq_value().clusterinfo().straliasname();
}

void InteractiveProtoManagementHandler::ModifyClusterRsp::Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const
{
    Response::Serializer(MngInteractiveMsg);
    MngInteractiveMsg.set_type(Management::Interactive::Message::ManagementMsgType::ModifyClusterRsp_T);

    MngInteractiveMsg.mutable_rspvalue()->mutable_modifyclusterrsp_value()->set_strvalue(m_strValue);
}

void InteractiveProtoManagementHandler::ModifyClusterRsp::UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg)
{
    Response::UnSerializer(MngInteractiveMsg);

    m_strValue = MngInteractiveMsg.rspvalue().modifyclusterrsp_value().strvalue();
}

void InteractiveProtoManagementHandler::QueryClusterInfoReq::Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const
{
    Request::Serializer(MngInteractiveMsg);
    MngInteractiveMsg.set_type(Management::Interactive::Message::ManagementMsgType::QueryClusterInfoReq_T);
    MngInteractiveMsg.mutable_reqvalue()->mutable_queryclusterinforeq_value()->set_strclusterid(m_strClusterID);
}

void InteractiveProtoManagementHandler::QueryClusterInfoReq::UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg)
{
    Request::UnSerializer(MngInteractiveMsg);
    m_strClusterID = MngInteractiveMsg.reqvalue().queryclusterinforeq_value().strclusterid();
}

void InteractiveProtoManagementHandler::QueryClusterInfoRsp::Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const
{
    Response::Serializer(MngInteractiveMsg);
    MngInteractiveMsg.set_type(Management::Interactive::Message::ManagementMsgType::QueryClusterInfoRsp_T);

    auto clusterInfo = MngInteractiveMsg.mutable_rspvalue()->mutable_queryclusterinforsp_value()->mutable_clusterinfo();

    clusterInfo->set_strclusterid(m_clusterInfo.m_strClusterID);
    clusterInfo->set_strclusteraddress(m_clusterInfo.m_strClusterAddress);
    clusterInfo->set_strmanagementaddress(m_clusterInfo.m_strManagementAddress);
    clusterInfo->set_straliasname(m_clusterInfo.m_strAliasname);
}

void InteractiveProtoManagementHandler::QueryClusterInfoRsp::UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg)
{
    Response::UnSerializer(MngInteractiveMsg);
    m_clusterInfo.m_strClusterID = MngInteractiveMsg.rspvalue().queryclusterinforsp_value().clusterinfo().strclusterid();
    m_clusterInfo.m_strClusterAddress = MngInteractiveMsg.rspvalue().queryclusterinforsp_value().clusterinfo().strclusteraddress();
    m_clusterInfo.m_strManagementAddress = MngInteractiveMsg.rspvalue().queryclusterinforsp_value().clusterinfo().strmanagementaddress();
    m_clusterInfo.m_strAliasname = MngInteractiveMsg.rspvalue().queryclusterinforsp_value().clusterinfo().straliasname();
}

void InteractiveProtoManagementHandler::ShakehandClusterReq::Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const
{
    Request::Serializer(MngInteractiveMsg);
    MngInteractiveMsg.set_type(Management::Interactive::Message::ManagementMsgType::ShakehandClusterReq_T);

    MngInteractiveMsg.mutable_reqvalue()->mutable_shakehandclusterreq_value()->set_strclusterid(m_strClusterID);
}

void InteractiveProtoManagementHandler::ShakehandClusterReq::UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg)
{
    Request::UnSerializer(MngInteractiveMsg);
    m_strClusterID = MngInteractiveMsg.reqvalue().shakehandclusterreq_value().strclusterid();
}

void InteractiveProtoManagementHandler::ShakehandClusterRsp::Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const
{
    Response::Serializer(MngInteractiveMsg);
    MngInteractiveMsg.set_type(Management::Interactive::Message::ManagementMsgType::ShakehandClusterRsp_T);

    MngInteractiveMsg.mutable_rspvalue()->mutable_shakehandclusterrsp_value()->set_strvalue(m_strValue);
}

void InteractiveProtoManagementHandler::ShakehandClusterRsp::UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg)
{
    Response::UnSerializer(MngInteractiveMsg);

    m_strValue = MngInteractiveMsg.rspvalue().shakehandclusterrsp_value().strvalue();
}

void InteractiveProtoManagementHandler::QueryAllClusterReq::Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const
{
    Request::Serializer(MngInteractiveMsg);
    MngInteractiveMsg.set_type(Management::Interactive::Message::ManagementMsgType::QueryAllClusterReq_T);

    MngInteractiveMsg.mutable_reqvalue()->mutable_queryallclusterreq_value()->set_strmanagementaddress(m_strManagementAddress);
}

void InteractiveProtoManagementHandler::QueryAllClusterReq::UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg)
{
    Request::UnSerializer(MngInteractiveMsg);

    m_strManagementAddress = MngInteractiveMsg.reqvalue().queryallclusterreq_value().strmanagementaddress();
}

void InteractiveProtoManagementHandler::QueryAllClusterRsp::Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const
{
    Response::Serializer(MngInteractiveMsg);
    MngInteractiveMsg.set_type(Management::Interactive::Message::ManagementMsgType::QueryAllClusterRsp_T);

    auto clusterInfo = MngInteractiveMsg.mutable_rspvalue()->mutable_queryallclusterrsp_value()->mutable_clusterinfo();
    SerializeClusterList(m_clusterInfoList, clusterInfo);
}

void InteractiveProtoManagementHandler::QueryAllClusterRsp::UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg)
{
    Response::UnSerializer(MngInteractiveMsg);
    UnSerializeClusterList(m_clusterInfoList, MngInteractiveMsg.rspvalue().queryallclusterrsp_value().clusterinfo());
}

void InteractiveProtoManagementHandler::QueryClusterDeviceReq::Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const
{
    Request::Serializer(MngInteractiveMsg);
    MngInteractiveMsg.set_type(Management::Interactive::Message::ManagementMsgType::QueryClusterDeviceReq_T);

    MngInteractiveMsg.mutable_reqvalue()->mutable_queryclusterdevicereq_value()->set_strclusterid(m_strClusterID);
    MngInteractiveMsg.mutable_reqvalue()->mutable_queryclusterdevicereq_value()->set_strbegindate(m_strBegindate);
    MngInteractiveMsg.mutable_reqvalue()->mutable_queryclusterdevicereq_value()->set_strenddate(m_strEnddate);
    MngInteractiveMsg.mutable_reqvalue()->mutable_queryclusterdevicereq_value()->set_uirecordtype(m_uiRecordType);
    MngInteractiveMsg.mutable_reqvalue()->mutable_queryclusterdevicereq_value()->set_uibeginindex(m_uiBeginIndex);
}

void InteractiveProtoManagementHandler::QueryClusterDeviceReq::UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg)
{
    Request::UnSerializer(MngInteractiveMsg);
    m_strClusterID = MngInteractiveMsg.reqvalue().queryclusterdevicereq_value().strclusterid();
    m_strBegindate = MngInteractiveMsg.reqvalue().queryclusterdevicereq_value().strbegindate();
    m_strEnddate = MngInteractiveMsg.reqvalue().queryclusterdevicereq_value().strenddate();
    m_uiRecordType = MngInteractiveMsg.reqvalue().queryclusterdevicereq_value().uirecordtype();
    m_uiBeginIndex = MngInteractiveMsg.reqvalue().queryclusterdevicereq_value().uibeginindex();
}

void InteractiveProtoManagementHandler::QueryClusterDeviceRsp::Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const
{
    Response::Serializer(MngInteractiveMsg);
    MngInteractiveMsg.set_type(Management::Interactive::Message::ManagementMsgType::QueryClusterDeviceRsp_T);

    auto clusterInfo = MngInteractiveMsg.mutable_rspvalue()->mutable_queryclusterdevicersp_value()->mutable_accesseddeviceinfo();
    SerializeAccessedDeviceList(m_accessedDeviceInfoList, clusterInfo);
}

void InteractiveProtoManagementHandler::QueryClusterDeviceRsp::UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg)
{
    Response::UnSerializer(MngInteractiveMsg);
    UnSerializeAccessedDeviceList(m_accessedDeviceInfoList, MngInteractiveMsg.rspvalue().queryclusterdevicersp_value().accesseddeviceinfo());
}

void InteractiveProtoManagementHandler::QueryClusterUserReq::Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const
{
    Request::Serializer(MngInteractiveMsg);
    MngInteractiveMsg.set_type(Management::Interactive::Message::ManagementMsgType::QueryClusterUserReq_T);

    MngInteractiveMsg.mutable_reqvalue()->mutable_queryclusteruserreq_value()->set_strclusterid(m_strClusterID);
    MngInteractiveMsg.mutable_reqvalue()->mutable_queryclusteruserreq_value()->set_strbegindate(m_strBegindate);
    MngInteractiveMsg.mutable_reqvalue()->mutable_queryclusteruserreq_value()->set_strenddate(m_strEnddate);
    MngInteractiveMsg.mutable_reqvalue()->mutable_queryclusteruserreq_value()->set_uirecordtype(m_uiRecordType);
    MngInteractiveMsg.mutable_reqvalue()->mutable_queryclusteruserreq_value()->set_uibeginindex(m_uiBeginIndex);
}

void InteractiveProtoManagementHandler::QueryClusterUserReq::UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg)
{
    Request::UnSerializer(MngInteractiveMsg);
    m_strClusterID = MngInteractiveMsg.reqvalue().queryclusteruserreq_value().strclusterid();
    m_strBegindate = MngInteractiveMsg.reqvalue().queryclusteruserreq_value().strbegindate();
    m_strEnddate = MngInteractiveMsg.reqvalue().queryclusteruserreq_value().strenddate();
    m_uiRecordType = MngInteractiveMsg.reqvalue().queryclusteruserreq_value().uirecordtype();
    m_uiBeginIndex = MngInteractiveMsg.reqvalue().queryclusteruserreq_value().uibeginindex();
}

void InteractiveProtoManagementHandler::QueryClusterUserRsp::Serializer(ManagementInteractiveMessage &MngInteractiveMsg) const
{
    Response::Serializer(MngInteractiveMsg);
    MngInteractiveMsg.set_type(Management::Interactive::Message::ManagementMsgType::QueryClusterUserRsp_T);

    auto clusterInfo = MngInteractiveMsg.mutable_rspvalue()->mutable_queryclusteruserrsp_value()->mutable_accesseduserinfo();
    SerializeAccessedUserList(m_accessedUserInfoList, clusterInfo);
}

void InteractiveProtoManagementHandler::QueryClusterUserRsp::UnSerializer(const ManagementInteractiveMessage &MngInteractiveMsg)
{
    Response::UnSerializer(MngInteractiveMsg);
    UnSerializeAccessedUserList(m_accessedUserInfoList, MngInteractiveMsg.rspvalue().queryclusteruserrsp_value().accesseduserinfo());
}
