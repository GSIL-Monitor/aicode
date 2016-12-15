#include "InteractiveProtoHandler.h"
#include "InteractiveProtocol.pb.h"

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

    InteractiveMessage Msg;
    Msg.Clear();

    pReq->Serializer(Msg);

    Msg.SerializeToString(&strOutput);

    return true;
};

template<typename T, typename TT> bool UnSerializerT(const InteractiveMessage &InteractiveMsg, TT &req)
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

    pReq->UnSerializer(InteractiveMsg);

    return true;
}

template<typename T> void UnSerializeDevList(std::list<T> &devInfoList, 
    const ::google::protobuf::RepeatedPtrField< ::Interactive::Message::Device > &srcDevInfoList)
{
    devInfoList.clear();
    int iCount = srcDevInfoList.size();
    for (int i = 0; i < iCount; ++i)
    {
        T devInfo;
        devInfo.m_strDevID = srcDevInfoList.Get(i).strdevid();
        devInfo.m_strDevName = srcDevInfoList.Get(i).strdevname();
        devInfo.m_strDevPassword = srcDevInfoList.Get(i).strdevpassword();
        devInfo.m_uiTypeInfo = srcDevInfoList.Get(i).uitypeinfo();
        devInfo.m_strCreatedate = srcDevInfoList.Get(i).strcreatedate();
        devInfo.m_uiStatus = srcDevInfoList.Get(i).uistatus();
        devInfo.m_strExtend = srcDevInfoList.Get(i).strextend();
        devInfo.m_strInnerinfo = srcDevInfoList.Get(i).strinnerinfo();
        devInfo.m_strOwnerUserID = srcDevInfoList.Get(i).strowneruserid();

        devInfo.m_sharingUserIDList.clear();
        int iCountTmp = srcDevInfoList.Get(i).strsharinguserid_size();
        for (int k = 0; k < iCountTmp; ++k)
        {
            devInfo.m_sharingUserIDList.push_back(srcDevInfoList.Get(i).strsharinguserid(k));
        }

        devInfo.m_sharedUserIDList.clear();
        iCountTmp = srcDevInfoList.Get(i).strshareduserid_size();
        for (int k = 0; k < iCountTmp; ++k)
        {
            devInfo.m_sharedUserIDList.push_back(srcDevInfoList.Get(i).strshareduserid(k));
        }

        devInfo.m_strItemsList.clear();
        iCountTmp = srcDevInfoList.Get(i).stritems_size();
        for (int k = 0; k < iCountTmp; ++k)
        {
            devInfo.m_strItemsList.push_back(srcDevInfoList.Get(i).stritems(k));
        }

        devInfoList.push_back(std::move(devInfo));
    }
}

template<typename T> void SerializeDevList(const std::list<T> &devInfoList, 
    ::google::protobuf::RepeatedPtrField< ::Interactive::Message::Device >* pDstDevInfoList)
{
    for (unsigned int i = 0; i < devInfoList.size(); ++i)
    {
        pDstDevInfoList->Add();
    }

    auto itBegin = devInfoList.begin();
    auto itEnd = devInfoList.end();
    int i = 0;
    while (itBegin != itEnd)
    {
        pDstDevInfoList->Mutable(i)->set_strdevid(itBegin->m_strDevID);
        pDstDevInfoList->Mutable(i)->set_strdevname(itBegin->m_strDevName);
        pDstDevInfoList->Mutable(i)->set_strdevpassword(itBegin->m_strDevPassword);
        pDstDevInfoList->Mutable(i)->set_uitypeinfo(itBegin->m_uiTypeInfo);
        pDstDevInfoList->Mutable(i)->set_strcreatedate(itBegin->m_strCreatedate);
        pDstDevInfoList->Mutable(i)->set_uistatus(itBegin->m_uiStatus);
        pDstDevInfoList->Mutable(i)->set_strextend(itBegin->m_strExtend);
        pDstDevInfoList->Mutable(i)->set_strinnerinfo(itBegin->m_strInnerinfo);
        pDstDevInfoList->Mutable(i)->set_strowneruserid(itBegin->m_strOwnerUserID);


        for (unsigned int k = 0; k < itBegin->m_sharingUserIDList.size(); ++k)
        {
            pDstDevInfoList->Mutable(i)->add_strsharinguserid();
        }
        auto itSharingBegin = itBegin->m_sharingUserIDList.begin();
        auto itSharingEnd = itBegin->m_sharingUserIDList.end();
        int iSharing = 0;
        while (itSharingBegin != itSharingEnd)
        {
            pDstDevInfoList->Mutable(i)->set_strsharinguserid(iSharing, *itSharingBegin);

            ++iSharing;
            ++itSharingBegin;
        }

        for (unsigned int k = 0; k < itBegin->m_sharedUserIDList.size(); ++k)
        {
            pDstDevInfoList->Mutable(i)->add_strshareduserid();
        }
        auto itSharedBegin = itBegin->m_sharedUserIDList.begin();
        auto itSharedEnd = itBegin->m_sharedUserIDList.end();
        int iShared = 0;
        while (itSharedBegin != itSharedEnd)
        {
            pDstDevInfoList->Mutable(i)->set_strshareduserid(iShared, *itSharedBegin);

            ++iShared;
            ++itSharedBegin;
        }

        for (unsigned int k = 0; k < itBegin->m_strItemsList.size(); ++k)
        {
            pDstDevInfoList->Mutable(i)->add_stritems();
        }
        auto itItemsBegin = itBegin->m_strItemsList.begin();
        auto itItemsEnd = itBegin->m_strItemsList.end();
        int iItems = 0;
        while (itItemsBegin != itItemsEnd)
        {
            pDstDevInfoList->Mutable(i)->set_stritems(iItems, *itItemsBegin);

            ++iItems;
            ++itItemsBegin;
        }
        
        ++i;
        ++itBegin;
    }
}




InteractiveProtoHandler::InteractiveProtoHandler()
{
    //////////////////////////////////////////////////////
    
    SerializeHandler handler;
    handler.Szr = boost::bind(&InteractiveProtoHandler::RegisterUserReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::RegisterUserReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::RegisterUserReq_USR_T, handler));
    
    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::RegisterUserRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::RegisterUserRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::RegisterUserRsp_USR_T, handler));

    ////////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::UnRegisterUserReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::UnRegisterUserReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::UnRegisterUserReq_USR_T, handler));

    ////

    handler.Szr = boost::bind(&InteractiveProtoHandler::UnRegisterUserRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::UnRegisterUserRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::UnRegisterUserRsp_USR_T, handler));

    /////////////////////////////////////////////////////////
    
    handler.Szr = boost::bind(&InteractiveProtoHandler::LoginReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::LoginReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::LoginReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::LoginRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::LoginRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::LoginRsp_USR_T, handler));

    ////////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::LogoutReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::LogoutReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::LogoutReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::LogoutRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::LogoutRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::LogoutRsp_USR_T, handler));

    ////////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::ShakehandReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::ShakehandReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::ShakehandReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::ShakehandRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::ShakehandRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::ShakehandRsp_USR_T, handler));

    ////////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::ConfigInfoReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::ConfigInfoReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::ConfigInfoReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::ConfigInfoRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::ConfigInfoRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::ConfigInfoRsp_USR_T, handler));

    ////////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::AddDevReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::AddDevReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::AddDevReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::AddDevRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::AddDevRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::AddDevRsp_USR_T, handler));

    ////////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::DelDevReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::DelDevReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::DelDevReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::DelDevRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::DelDevRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::DelDevRsp_USR_T, handler));

    ///////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::ModifyDevReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::ModifyDevReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::ModifyDevReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::ModifyDevRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::ModifyDevRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::ModifyDevRsp_USR_T, handler));

    ///////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryDevReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryDevReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryDevReq_USR_T, handler));

    //
    
    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryDevRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryDevRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryDevRsp_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::SharingDevReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::SharingDevReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::SharingDevReq_USR_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::SharingDevRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::SharingDevRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::SharingDevRsp_USR_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoHandler::CancelSharedDevReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::CancelSharedDevReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::CancelSharedDevReq_USR_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::CancelSharedDevRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::CancelSharedDevRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::CancelSharedDevRsp_USR_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoHandler::AddFriendsReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::AddFriendsReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::AddFriendsReq_USR_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::AddFriendsRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::AddFriendsRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::AddFriendsRsp_USR_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoHandler::DelFriendsReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::DelFriendsReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::DelFriendsReq_USR_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::DelFriendsRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::DelFriendsRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::DelFriendsRsp_USR_T, handler));
    
    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryFriendsReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryFriendsReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryFriendsReq_USR_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryFriendsRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryFriendsRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryFriendsRsp_USR_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoHandler::GetOnlineDevInfoReq_INNER_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::GetOnlineDevInfoReq_INNER_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::GetOnlineDevInfoReq_INNER_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::GetOnlineDevInfoRsp_INNER_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::GetOnlineDevInfoRsp_INNER_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::GetOnlineDevInfoRsp_INNER_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoHandler::BroadcastOnlineDevInfo_INNER_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::BroadcastOnlineDevInfo_INNER_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::BroadcastOnlineDevInfo_INNER_T, handler));

    ///////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::GetOnlineUserInfoReq_INNER_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::GetOnlineUserInfoReq_INNER_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::GetOnlineUserInfoReq_INNER_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoHandler::GetOnlineUserInfoRsp_INNER_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::GetOnlineUserInfoRsp_INNER_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::GetOnlineUserInfoRsp_INNER_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoHandler::BroadcastOnlineUserInfo_INNER_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::BroadcastOnlineUserInfo_INNER_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::BroadcastOnlineUserInfo_INNER_T, handler));
    

}

InteractiveProtoHandler::~InteractiveProtoHandler()
{

}

bool InteractiveProtoHandler::GetMsgType(const std::string &strData, MsgType &msgtype)
{
    InteractiveMessage Msg;
    Msg.Clear();
    if (!Msg.ParseFromString(strData))
    {
        return false;
    }

    msgtype = (MsgType)Msg.type();

    return true;
}

bool InteractiveProtoHandler::SerializeReq(const Req &req, std::string &strOutput)
{
    const int iType = req.m_MsgType;

    auto itFind = m_ReqAndRspHandlerMap.find(iType);
    if (m_ReqAndRspHandlerMap.end() == itFind)
    {
        return false;
    }

    return itFind->second.Szr(req, strOutput);
}

bool InteractiveProtoHandler::UnSerializeReq(const std::string &strData, Req &req)
{   
    InteractiveMessage Msg;
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

bool InteractiveProtoHandler::SerializeRsp(const Rsp &rsp, std::string &strOutput)
{
    return SerializeReq(rsp, strOutput);
}

bool InteractiveProtoHandler::UnSerializeRsp(const std::string &strData, Rsp &rsp)
{
    return UnSerializeReq(strData, rsp);
}

bool InteractiveProtoHandler::RegisterUserReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<RegisterUserReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::RegisterUserReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<RegisterUserReq_USR, Req>(InteractiveMsg, req);
}

//////
bool InteractiveProtoHandler::RegisterUserRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<RegisterUserRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::RegisterUserRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<RegisterUserRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::UnRegisterUserReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<UnRegisterUserReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::UnRegisterUserReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<UnRegisterUserReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::UnRegisterUserRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<UnRegisterUserRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::UnRegisterUserRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<UnRegisterUserRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::LoginReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<LoginReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::LoginReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<LoginReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::LoginRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<LoginRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::LoginRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<LoginRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::LogoutReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<LogoutReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::LogoutReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<LogoutReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::LogoutRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<LogoutRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::LogoutRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<LogoutRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::ShakehandReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<ShakehandReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::ShakehandReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<ShakehandReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::ShakehandRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<ShakehandRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::ShakehandRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<ShakehandRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::ConfigInfoReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<ConfigInfoReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::ConfigInfoReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<ConfigInfoReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::ConfigInfoRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<ConfigInfoRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::ConfigInfoRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<ConfigInfoRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::AddDevReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<AddDevReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::AddDevReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<AddDevReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::AddDevRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<AddDevRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::AddDevRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<AddDevRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::DelDevReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<DelDevReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::DelDevReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<DelDevReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::DelDevRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<DelDevRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::DelDevRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<DelDevRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::ModifyDevReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<ModifyDevReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::ModifyDevReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<ModifyDevReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::ModifyDevRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<ModifyDevRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::ModifyDevRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<ModifyDevRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryDevReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryDevReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryDevReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryDevReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryDevRsp_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryDevRsp_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryDevRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryDevRsp_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::SharingDevReq_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<SharingDevReq_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::SharingDevReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<SharingDevReq_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::SharingDevRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<SharingDevRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::SharingDevRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<SharingDevRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::CancelSharedDevReq_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<CancelSharedDevReq_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::CancelSharedDevReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<CancelSharedDevReq_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::CancelSharedDevRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<CancelSharedDevRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::CancelSharedDevRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<CancelSharedDevRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::AddFriendsReq_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<AddFriendsReq_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::AddFriendsReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<AddFriendsReq_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::AddFriendsRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<AddFriendsRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::AddFriendsRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<AddFriendsRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::DelFriendsReq_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<DelFriendsReq_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::DelFriendsReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<DelFriendsReq_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::DelFriendsRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<DelFriendsRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::DelFriendsRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<DelFriendsRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryFriendsReq_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryFriendsReq_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryFriendsReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryFriendsReq_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryFriendsRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryFriendsRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryFriendsRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryFriendsRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::GetOnlineDevInfoReq_INNER_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<GetOnlineDevInfoReq_INNER, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::GetOnlineDevInfoReq_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<GetOnlineDevInfoReq_INNER, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::GetOnlineDevInfoRsp_INNER_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<GetOnlineDevInfoRsp_INNER, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::GetOnlineDevInfoRsp_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<GetOnlineDevInfoRsp_INNER, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::BroadcastOnlineDevInfo_INNER_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<BroadcastOnlineDevInfo_INNER, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::BroadcastOnlineDevInfo_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<BroadcastOnlineDevInfo_INNER, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::GetOnlineUserInfoReq_INNER_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<GetOnlineUserInfoReq_INNER, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::GetOnlineUserInfoReq_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<GetOnlineUserInfoReq_INNER, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::GetOnlineUserInfoRsp_INNER_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<GetOnlineUserInfoRsp_INNER, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::GetOnlineUserInfoRsp_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<GetOnlineUserInfoRsp_INNER, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::BroadcastOnlineUserInfo_INNER_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<BroadcastOnlineUserInfo_INNER, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::BroadcastOnlineUserInfo_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<BroadcastOnlineUserInfo_INNER, Req>(InteractiveMsg, rsp);
}



void InteractiveProtoHandler::Req::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    m_MsgType = (MsgType)InteractiveMsg.type();
    m_uiMsgSeq = InteractiveMsg.uimsgseq();
    m_strSID = InteractiveMsg.strsid();
}

void InteractiveProtoHandler::Req::Serializer(InteractiveMessage &InteractiveMsg) const
{
    InteractiveMsg.set_uimsgseq(m_uiMsgSeq);
    InteractiveMsg.set_strsid(m_strSID);
}

void InteractiveProtoHandler::Rsp::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_iRetcode = InteractiveMsg.rspvalue().iretcode();
    m_strRetMsg = InteractiveMsg.rspvalue().strretmsg();
    
}

void InteractiveProtoHandler::Rsp::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.mutable_rspvalue()->set_iretcode(m_iRetcode);
    InteractiveMsg.mutable_rspvalue()->set_strretmsg(m_strRetMsg);
}

void InteractiveProtoHandler::RegisterUserReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_userInfo.m_strUserID = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().struserid();
    m_userInfo.m_strUserName = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().strusername();
    m_userInfo.m_strUserPassword = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().struserpassword();
    m_userInfo.m_uiTypeInfo = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().uitypeinfo();
    m_userInfo.m_strCreatedate = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().strcreatedate();
    m_userInfo.m_uiStatus = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().uistatus();
    m_userInfo.m_strExtend = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().strextend();

    UnSerializeDevList<Device>(m_userInfo.m_ownerDevInfoList, InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().ownerdevinfo());
    UnSerializeDevList<Device>(m_userInfo.m_sharingDevInfoList, InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().sharingdevinfo());
    UnSerializeDevList<Device>(m_userInfo.m_sharedDevInfoList, InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().shareddevinfo());
    
    m_userInfo.m_strItemsList.clear();
    int iCount = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().stritems_size();
    for (int i = 0; i < iCount; ++i)
    {
        m_userInfo.m_strItemsList.push_back(InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().stritems(i));
    }

    m_strValue = InteractiveMsg.reqvalue().registeruserreq_usr_value().strvalue();
    

    ////ÁÐ±íÊôÐÔ
    //m_userInfo.m_ownerDevInfoList.clear();
    //int iCount = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().ownerdevinfo_size();
    //for (int i = 0; i < iCount; ++i)
    //{
    //    Device devInfo;
    //    devInfo.m_strDevID = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().ownerdevinfo(i).strdevid();
    //    devInfo.m_strDevName = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().ownerdevinfo(i).strdevname();
    //    devInfo.m_strDevPassword = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().ownerdevinfo(i).strdevpassword();
    //    devInfo.m_uiTypeInfo = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().ownerdevinfo(i).uitypeinfo();
    //    devInfo.m_strCreatedate = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().ownerdevinfo(i).strcreatedate();
    //    devInfo.m_strInnerinfo = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().ownerdevinfo(i).strinnerinfo();
    //    devInfo.m_strOwnerUserID = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().ownerdevinfo(i).strowneruserid();

    //    devInfo.m_sharingUserIDList.clear();
    //    int iCountTmp = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().ownerdevinfo(i).strsharinguserid_size();
    //    for (int k = 0; k < iCountTmp; ++k)
    //    {
    //        devInfo.m_sharingUserIDList.push_back(InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().ownerdevinfo(i).strsharinguserid(k));
    //    }

    //    devInfo.m_sharedUserIDList.clear();
    //    int iCountTmp = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().ownerdevinfo(i).strshareduserid_size();
    //    for (int k = 0; k < iCountTmp; ++k)
    //    {
    //        devInfo.m_sharedUserIDList.push_back(InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().ownerdevinfo(i).strshareduserid(k));
    //    }

    //    devInfo.m_strItemsList.clear();
    //    int iCountTmp = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().ownerdevinfo(i).stritems_size();
    //    for (int k = 0; k < iCountTmp; ++k)
    //    {
    //        devInfo.m_strItemsList.push_back(InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().ownerdevinfo(i).stritems(k));
    //    }
    //    
    //    m_userInfo.m_ownerDevInfoList.push_back(devInfo);
    //}

}

void InteractiveProtoHandler::RegisterUserReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::RegisterUserReq_USR_T);

    InteractiveMsg.mutable_reqvalue()->mutable_registeruserreq_usr_value()->set_strvalue(m_strValue);
    auto uinfo = InteractiveMsg.mutable_reqvalue()->mutable_registeruserreq_usr_value()->mutable_userinfo();
    
    uinfo->set_struserid(m_userInfo.m_strUserID);
    uinfo->set_strusername(m_userInfo.m_strUserName);
    uinfo->set_struserpassword(m_userInfo.m_strUserPassword);
    uinfo->set_uitypeinfo(m_userInfo.m_uiTypeInfo);
    uinfo->set_strcreatedate(m_userInfo.m_strCreatedate);
    uinfo->set_uistatus(m_userInfo.m_uiStatus);
    uinfo->set_strextend(m_userInfo.m_strExtend);
    
    SerializeDevList<Device>(m_userInfo.m_ownerDevInfoList, uinfo->mutable_ownerdevinfo());
    SerializeDevList<Device>(m_userInfo.m_sharingDevInfoList, uinfo->mutable_sharingdevinfo());
    SerializeDevList<Device>(m_userInfo.m_sharedDevInfoList, uinfo->mutable_shareddevinfo());

    for (unsigned int i = 0; i < m_userInfo.m_strItemsList.size(); ++i)
    {
        uinfo->add_stritems();
    }
    auto itBegin = m_userInfo.m_strItemsList.begin();
    auto itEnd = m_userInfo.m_strItemsList.end();
    int i = 0;
    while (itBegin != itEnd)
    {
        
        uinfo->set_stritems(i, *itBegin);

        ++i;
        ++itBegin;
    }
}


void InteractiveProtoHandler::RegisterUserRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.rspvalue().registeruserrsp_usr_value().struserid();
    m_strValue = InteractiveMsg.rspvalue().registeruserrsp_usr_value().strvalue();

}

void InteractiveProtoHandler::RegisterUserRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::RegisterUserRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_registeruserrsp_usr_value()->set_struserid(m_strUserID);
    InteractiveMsg.mutable_rspvalue()->mutable_registeruserrsp_usr_value()->set_strvalue(m_strValue);

}

void InteractiveProtoHandler::UnRegisterUserReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_userInfo.m_strUserID = InteractiveMsg.reqvalue().unregisteruserreq_usr_value().userinfo().struserid();
    m_userInfo.m_strUserName = InteractiveMsg.reqvalue().unregisteruserreq_usr_value().userinfo().strusername();
    m_userInfo.m_strUserPassword = InteractiveMsg.reqvalue().unregisteruserreq_usr_value().userinfo().struserpassword();
    m_userInfo.m_uiTypeInfo = InteractiveMsg.reqvalue().unregisteruserreq_usr_value().userinfo().uitypeinfo();
    m_userInfo.m_strCreatedate = InteractiveMsg.reqvalue().unregisteruserreq_usr_value().userinfo().strcreatedate();
    m_userInfo.m_uiStatus = InteractiveMsg.reqvalue().unregisteruserreq_usr_value().userinfo().uistatus();
    m_userInfo.m_strExtend = InteractiveMsg.reqvalue().unregisteruserreq_usr_value().userinfo().strextend();

    UnSerializeDevList<Device>(m_userInfo.m_ownerDevInfoList, InteractiveMsg.reqvalue().unregisteruserreq_usr_value().userinfo().ownerdevinfo());
    UnSerializeDevList<Device>(m_userInfo.m_sharingDevInfoList, InteractiveMsg.reqvalue().unregisteruserreq_usr_value().userinfo().sharingdevinfo());
    UnSerializeDevList<Device>(m_userInfo.m_sharedDevInfoList, InteractiveMsg.reqvalue().unregisteruserreq_usr_value().userinfo().shareddevinfo());

    m_userInfo.m_strItemsList.clear();
    int iCount = InteractiveMsg.reqvalue().unregisteruserreq_usr_value().userinfo().stritems_size();
    for (int i = 0; i < iCount; ++i)
    {
        m_userInfo.m_strItemsList.push_back(InteractiveMsg.reqvalue().unregisteruserreq_usr_value().userinfo().stritems(i));
    }

    m_strValue = InteractiveMsg.reqvalue().unregisteruserreq_usr_value().strvalue();
    
}

void InteractiveProtoHandler::UnRegisterUserReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::UnRegisterUserReq_USR_T);

    InteractiveMsg.mutable_reqvalue()->mutable_unregisteruserreq_usr_value()->set_strvalue(m_strValue);
    auto uinfo = InteractiveMsg.mutable_reqvalue()->mutable_unregisteruserreq_usr_value()->mutable_userinfo();

    uinfo->set_struserid(m_userInfo.m_strUserID);
    uinfo->set_strusername(m_userInfo.m_strUserName);
    uinfo->set_struserpassword(m_userInfo.m_strUserPassword);
    uinfo->set_uitypeinfo(m_userInfo.m_uiTypeInfo);
    uinfo->set_strcreatedate(m_userInfo.m_strCreatedate);
    uinfo->set_uistatus(m_userInfo.m_uiStatus);
    uinfo->set_strextend(m_userInfo.m_strExtend);

    SerializeDevList<Device>(m_userInfo.m_ownerDevInfoList, uinfo->mutable_ownerdevinfo());
    SerializeDevList<Device>(m_userInfo.m_sharingDevInfoList, uinfo->mutable_sharingdevinfo());
    SerializeDevList<Device>(m_userInfo.m_sharedDevInfoList, uinfo->mutable_shareddevinfo());

    for (unsigned int i = 0; i < m_userInfo.m_strItemsList.size(); ++i)
    {
        uinfo->add_stritems();
    }
    auto itBegin = m_userInfo.m_strItemsList.begin();
    auto itEnd = m_userInfo.m_strItemsList.end();
    int i = 0;
    while (itBegin != itEnd)
    {

        uinfo->set_stritems(i, *itBegin);

        ++i;
        ++itBegin;
    }
}

void InteractiveProtoHandler::UnRegisterUserRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.rspvalue().unregisteruserrsp_usr_value().struserid();
    m_strValue = InteractiveMsg.rspvalue().unregisteruserrsp_usr_value().strvalue();

}

void InteractiveProtoHandler::UnRegisterUserRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::UnRegisterUserRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_unregisteruserrsp_usr_value()->set_struserid(m_strUserID);
    InteractiveMsg.mutable_rspvalue()->mutable_unregisteruserrsp_usr_value()->set_strvalue(m_strValue);

}

void InteractiveProtoHandler::LoginReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_userInfo.m_strUserID = InteractiveMsg.reqvalue().loginreq_usr_value().userinfo().struserid();
    m_userInfo.m_strUserName = InteractiveMsg.reqvalue().loginreq_usr_value().userinfo().strusername();
    m_userInfo.m_strUserPassword = InteractiveMsg.reqvalue().loginreq_usr_value().userinfo().struserpassword();
    m_userInfo.m_uiTypeInfo = InteractiveMsg.reqvalue().loginreq_usr_value().userinfo().uitypeinfo();
    m_userInfo.m_strCreatedate = InteractiveMsg.reqvalue().loginreq_usr_value().userinfo().strcreatedate();
    m_userInfo.m_uiStatus = InteractiveMsg.reqvalue().loginreq_usr_value().userinfo().uistatus();
    m_userInfo.m_strExtend = InteractiveMsg.reqvalue().loginreq_usr_value().userinfo().strextend();

    UnSerializeDevList<Device>(m_userInfo.m_ownerDevInfoList, InteractiveMsg.reqvalue().loginreq_usr_value().userinfo().ownerdevinfo());
    UnSerializeDevList<Device>(m_userInfo.m_sharingDevInfoList, InteractiveMsg.reqvalue().loginreq_usr_value().userinfo().sharingdevinfo());
    UnSerializeDevList<Device>(m_userInfo.m_sharedDevInfoList, InteractiveMsg.reqvalue().loginreq_usr_value().userinfo().shareddevinfo());

    m_userInfo.m_strItemsList.clear();
    int iCount = InteractiveMsg.reqvalue().loginreq_usr_value().userinfo().stritems_size();
    for (int i = 0; i < iCount; ++i)
    {
        m_userInfo.m_strItemsList.push_back(InteractiveMsg.reqvalue().loginreq_usr_value().userinfo().stritems(i));
    }

    m_strValue = InteractiveMsg.reqvalue().loginreq_usr_value().strvalue();
}

void InteractiveProtoHandler::LoginReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::LoginReq_USR_T);

    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_usr_value()->set_strvalue(m_strValue);
    auto uinfo = InteractiveMsg.mutable_reqvalue()->mutable_loginreq_usr_value()->mutable_userinfo();

    uinfo->set_struserid(m_userInfo.m_strUserID);
    uinfo->set_strusername(m_userInfo.m_strUserName);
    uinfo->set_struserpassword(m_userInfo.m_strUserPassword);
    uinfo->set_uitypeinfo(m_userInfo.m_uiTypeInfo);
    uinfo->set_strcreatedate(m_userInfo.m_strCreatedate);
    uinfo->set_uistatus(m_userInfo.m_uiStatus);
    uinfo->set_strextend(m_userInfo.m_strExtend);

    SerializeDevList<Device>(m_userInfo.m_ownerDevInfoList, uinfo->mutable_ownerdevinfo());
    SerializeDevList<Device>(m_userInfo.m_sharingDevInfoList, uinfo->mutable_sharingdevinfo());
    SerializeDevList<Device>(m_userInfo.m_sharedDevInfoList, uinfo->mutable_shareddevinfo());

    for (unsigned int i = 0; i < m_userInfo.m_strItemsList.size(); ++i)
    {
        uinfo->add_stritems();
    }
    auto itBegin = m_userInfo.m_strItemsList.begin();
    auto itEnd = m_userInfo.m_strItemsList.end();
    int i = 0;
    while (itBegin != itEnd)
    {

        uinfo->set_stritems(i, *itBegin);

        ++i;
        ++itBegin;
    }
}

void InteractiveProtoHandler::LoginRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    UnSerializeDevList<Device>(m_devInfoList, InteractiveMsg.rspvalue().loginrsp_usr_value().devinfo());
    m_strValue = InteractiveMsg.rspvalue().loginrsp_usr_value().strvalue();

}

void InteractiveProtoHandler::LoginRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::LoginRsp_USR_T);

    InteractiveMsg.mutable_rspvalue()->mutable_loginrsp_usr_value()->set_strvalue(m_strValue);

    auto uinfo = InteractiveMsg.mutable_rspvalue()->mutable_loginrsp_usr_value()->mutable_devinfo();
    SerializeDevList<Device>(m_devInfoList, uinfo);

}

void InteractiveProtoHandler::LogoutReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_userInfo.m_strUserID = InteractiveMsg.reqvalue().logoutreq_usr_value().userinfo().struserid();
    m_userInfo.m_strUserName = InteractiveMsg.reqvalue().logoutreq_usr_value().userinfo().strusername();
    m_userInfo.m_strUserPassword = InteractiveMsg.reqvalue().logoutreq_usr_value().userinfo().struserpassword();
    m_userInfo.m_uiTypeInfo = InteractiveMsg.reqvalue().logoutreq_usr_value().userinfo().uitypeinfo();
    m_userInfo.m_strCreatedate = InteractiveMsg.reqvalue().logoutreq_usr_value().userinfo().strcreatedate();
    m_userInfo.m_uiStatus = InteractiveMsg.reqvalue().logoutreq_usr_value().userinfo().uistatus();
    m_userInfo.m_strExtend = InteractiveMsg.reqvalue().logoutreq_usr_value().userinfo().strextend();

    UnSerializeDevList<Device>(m_userInfo.m_ownerDevInfoList, InteractiveMsg.reqvalue().logoutreq_usr_value().userinfo().ownerdevinfo());
    UnSerializeDevList<Device>(m_userInfo.m_sharingDevInfoList, InteractiveMsg.reqvalue().logoutreq_usr_value().userinfo().sharingdevinfo());
    UnSerializeDevList<Device>(m_userInfo.m_sharedDevInfoList, InteractiveMsg.reqvalue().logoutreq_usr_value().userinfo().shareddevinfo());

    m_userInfo.m_strItemsList.clear();
    int iCount = InteractiveMsg.reqvalue().logoutreq_usr_value().userinfo().stritems_size();
    for (int i = 0; i < iCount; ++i)
    {
        m_userInfo.m_strItemsList.push_back(InteractiveMsg.reqvalue().logoutreq_usr_value().userinfo().stritems(i));
    }

    m_strValue = InteractiveMsg.reqvalue().logoutreq_usr_value().strvalue();
}

void InteractiveProtoHandler::LogoutReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::LogoutReq_USR_T);

    InteractiveMsg.mutable_reqvalue()->mutable_logoutreq_usr_value()->set_strvalue(m_strValue);
    auto uinfo = InteractiveMsg.mutable_reqvalue()->mutable_logoutreq_usr_value()->mutable_userinfo();

    uinfo->set_struserid(m_userInfo.m_strUserID);
    uinfo->set_strusername(m_userInfo.m_strUserName);
    uinfo->set_struserpassword(m_userInfo.m_strUserPassword);
    uinfo->set_uitypeinfo(m_userInfo.m_uiTypeInfo);
    uinfo->set_strcreatedate(m_userInfo.m_strCreatedate);
    uinfo->set_uistatus(m_userInfo.m_uiStatus);
    uinfo->set_strextend(m_userInfo.m_strExtend);

    SerializeDevList<Device>(m_userInfo.m_ownerDevInfoList, uinfo->mutable_ownerdevinfo());
    SerializeDevList<Device>(m_userInfo.m_sharingDevInfoList, uinfo->mutable_sharingdevinfo());
    SerializeDevList<Device>(m_userInfo.m_sharedDevInfoList, uinfo->mutable_shareddevinfo());

    for (unsigned int i = 0; i < m_userInfo.m_strItemsList.size(); ++i)
    {
        uinfo->add_stritems();
    }
    auto itBegin = m_userInfo.m_strItemsList.begin();
    auto itEnd = m_userInfo.m_strItemsList.end();
    int i = 0;
    while (itBegin != itEnd)
    {

        uinfo->set_stritems(i, *itBegin);

        ++i;
        ++itBegin;
    }
}

void InteractiveProtoHandler::LogoutRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().logoutrsp_usr_value().strvalue();
}

void InteractiveProtoHandler::LogoutRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::LogoutRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_logoutrsp_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::ShakehandReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().shakehandreq_usr_value().struserid();
    m_strValue = InteractiveMsg.reqvalue().shakehandreq_usr_value().strvalue();

}

void InteractiveProtoHandler::ShakehandReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ShakehandReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_shakehandreq_usr_value()->set_struserid(m_strUserID);
    InteractiveMsg.mutable_reqvalue()->mutable_shakehandreq_usr_value()->set_strvalue(m_strValue);

}

void InteractiveProtoHandler::ShakehandRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().shakehandrsp_usr_value().strvalue();
}

void InteractiveProtoHandler::ShakehandRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ShakehandRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_shakehandrsp_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::ConfigInfoReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().configinforeq_usr_value().struserid();
    m_strValue = InteractiveMsg.reqvalue().configinforeq_usr_value().strvalue();

}

void InteractiveProtoHandler::ConfigInfoReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ConfigInfoReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_configinforeq_usr_value()->set_struserid(m_strUserID);
    InteractiveMsg.mutable_reqvalue()->mutable_configinforeq_usr_value()->set_strvalue(m_strValue);

}

void InteractiveProtoHandler::ConfigInfoRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().configinforsp_usr_value().strvalue();

    m_strItemsList.clear();
    int iCount = InteractiveMsg.rspvalue().configinforsp_usr_value().stritems_size();
    for (int i = 0; i < iCount; ++i)
    {
        m_strItemsList.push_back(InteractiveMsg.rspvalue().configinforsp_usr_value().stritems(i));
    }

}

void InteractiveProtoHandler::ConfigInfoRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ConfigInfoRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_configinforsp_usr_value()->set_strvalue(m_strValue);

    for (unsigned int i = 0; i < m_strItemsList.size(); ++i)
    {
        InteractiveMsg.mutable_rspvalue()->mutable_configinforsp_usr_value()->add_stritems();
    }

    auto itBegin = m_strItemsList.begin();
    auto itEnd = m_strItemsList.end();
    int i = 0;
    while (itBegin != itEnd)
    {

        InteractiveMsg.mutable_rspvalue()->mutable_configinforsp_usr_value()->set_stritems(i, *itBegin);

        ++i;
        ++itBegin;
    }

}

void InteractiveProtoHandler::AddDevReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().adddevreq_usr_value().struserid();
    
    m_devInfo.m_strDevID = InteractiveMsg.reqvalue().adddevreq_usr_value().devinfo().strdevid();
    m_devInfo.m_strDevName = InteractiveMsg.reqvalue().adddevreq_usr_value().devinfo().strdevname();
    m_devInfo.m_strDevPassword = InteractiveMsg.reqvalue().adddevreq_usr_value().devinfo().strdevpassword();
    m_devInfo.m_uiTypeInfo = InteractiveMsg.reqvalue().adddevreq_usr_value().devinfo().uitypeinfo();
    m_devInfo.m_strCreatedate = InteractiveMsg.reqvalue().adddevreq_usr_value().devinfo().strcreatedate();
    m_devInfo.m_uiStatus = InteractiveMsg.reqvalue().adddevreq_usr_value().devinfo().uistatus();
    m_devInfo.m_strExtend = InteractiveMsg.reqvalue().adddevreq_usr_value().devinfo().strextend();
    m_devInfo.m_strInnerinfo = InteractiveMsg.reqvalue().adddevreq_usr_value().devinfo().strinnerinfo();
    m_devInfo.m_strOwnerUserID = InteractiveMsg.reqvalue().adddevreq_usr_value().devinfo().strowneruserid();

    m_devInfo.m_sharingUserIDList.clear();
    int iCountTmp = InteractiveMsg.reqvalue().adddevreq_usr_value().devinfo().strsharinguserid_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        m_devInfo.m_sharingUserIDList.push_back(InteractiveMsg.reqvalue().adddevreq_usr_value().devinfo().strsharinguserid(i));
    }

    m_devInfo.m_sharedUserIDList.clear();
    iCountTmp = InteractiveMsg.reqvalue().adddevreq_usr_value().devinfo().strshareduserid_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        m_devInfo.m_sharedUserIDList.push_back(InteractiveMsg.reqvalue().adddevreq_usr_value().devinfo().strshareduserid(i));
    }

    m_devInfo.m_strItemsList.clear();
    iCountTmp = InteractiveMsg.reqvalue().adddevreq_usr_value().devinfo().stritems_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        m_devInfo.m_strItemsList.push_back(InteractiveMsg.reqvalue().adddevreq_usr_value().devinfo().stritems(i));
    }

}

void InteractiveProtoHandler::AddDevReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::AddDevReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->set_struserid(m_strUserID);
    
    InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->set_strdevid(m_devInfo.m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->set_strdevname(m_devInfo.m_strDevName);
    InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->set_strdevpassword(m_devInfo.m_strDevPassword);
    InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->set_uitypeinfo(m_devInfo.m_uiTypeInfo);
    InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->set_strcreatedate(m_devInfo.m_strCreatedate);
    InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->set_uistatus(m_devInfo.m_uiStatus);
    InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->set_strextend(m_devInfo.m_strExtend);
    InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->set_strinnerinfo(m_devInfo.m_strInnerinfo);
    InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->set_strowneruserid(m_devInfo.m_strOwnerUserID);

    for (unsigned int i = 0; i < m_devInfo.m_sharingUserIDList.size(); ++i)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->add_strsharinguserid();
    }
    auto itSharingBegin = m_devInfo.m_sharingUserIDList.begin();
    auto itSharingEnd = m_devInfo.m_sharingUserIDList.end();
    int iSharing = 0;
    while (itSharingBegin != itSharingEnd)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->set_strsharinguserid(iSharing, *itSharingBegin);

        ++iSharing;
        ++itSharingBegin;
    }

    for (unsigned int i = 0; i < m_devInfo.m_sharedUserIDList.size(); ++i)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->add_strshareduserid();
    }
    auto itSharedBegin = m_devInfo.m_sharedUserIDList.begin();
    auto itSharedEnd = m_devInfo.m_sharedUserIDList.end();
    int iShared = 0;
    while (itSharedBegin != itSharedEnd)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->set_strshareduserid(iShared, *itSharedBegin);

        ++iShared;
        ++itSharedBegin;
    }

    for (unsigned int i = 0; i < m_devInfo.m_strItemsList.size(); ++i)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->add_stritems();
    }
    auto itItemBegin = m_devInfo.m_strItemsList.begin();
    auto itItemEnd = m_devInfo.m_strItemsList.end();
    int iItem = 0;
    while (itItemBegin != itItemEnd)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->set_strshareduserid(iItem, *itItemBegin);

        ++iItem;
        ++itItemBegin;
    }

}

void InteractiveProtoHandler::AddDevRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().adddevrsp_usr_value().strvalue();
}

void InteractiveProtoHandler::AddDevRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::AddDevRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_adddevrsp_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::DelDevReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().deldevreq_usr_value().struserid();
        
    m_strDevIDList.clear();
    int iCountTmp = InteractiveMsg.reqvalue().deldevreq_usr_value().strdevid_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        m_strDevIDList.push_back(InteractiveMsg.reqvalue().deldevreq_usr_value().strdevid(i));
    }
    
}

void InteractiveProtoHandler::DelDevReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::DelDevReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_deldevreq_usr_value()->set_struserid(m_strUserID);
    
    for (unsigned int i = 0; i < m_strDevIDList.size(); ++i)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_deldevreq_usr_value()->add_strdevid();
    }
    auto itBegin = m_strDevIDList.begin();
    auto itEnd = m_strDevIDList.end();
    int iDev = 0;
    while (itBegin != itEnd)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_deldevreq_usr_value()->set_strdevid(iDev, *itBegin);

        ++iDev;
        ++itBegin;
    }
}


void InteractiveProtoHandler::DelDevRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().deldevrsp_usr_value().strvalue();

    m_strDevIDFailedList.clear();
    int iCountTmp = InteractiveMsg.rspvalue().deldevrsp_usr_value().strdevidfailed_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        m_strDevIDFailedList.push_back(InteractiveMsg.rspvalue().deldevrsp_usr_value().strdevidfailed(i));
    }

}

void InteractiveProtoHandler::DelDevRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::DelDevRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_deldevrsp_usr_value()->set_strvalue(m_strValue);

    for (unsigned int i = 0; i < m_strDevIDFailedList.size(); ++i)
    {
        InteractiveMsg.mutable_rspvalue()->mutable_deldevrsp_usr_value()->add_strdevidfailed();
    }
    auto itBegin = m_strDevIDFailedList.begin();
    auto itEnd = m_strDevIDFailedList.end();
    int iDev = 0;
    while (itBegin != itEnd)
    {
        InteractiveMsg.mutable_rspvalue()->mutable_deldevrsp_usr_value()->set_strdevidfailed(iDev, *itBegin);

        ++iDev;
        ++itBegin;
    }
}

void InteractiveProtoHandler::ModifyDevReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().modifydevreq_usr_value().struserid();

    m_devInfo.m_strDevID = InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().strdevid();
    m_devInfo.m_strDevName = InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().strdevname();
    m_devInfo.m_strDevPassword = InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().strdevpassword();
    m_devInfo.m_uiTypeInfo = InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().uitypeinfo();
    m_devInfo.m_strCreatedate = InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().strcreatedate();
    m_devInfo.m_uiStatus = InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().uistatus();
    m_devInfo.m_strExtend = InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().strextend();
    m_devInfo.m_strInnerinfo = InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().strinnerinfo();
    m_devInfo.m_strOwnerUserID = InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().strowneruserid();

    m_devInfo.m_sharingUserIDList.clear();
    int iCountTmp = InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().strsharinguserid_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        m_devInfo.m_sharingUserIDList.push_back(InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().strsharinguserid(i));
    }

    m_devInfo.m_sharedUserIDList.clear();
    iCountTmp = InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().strshareduserid_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        m_devInfo.m_sharedUserIDList.push_back(InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().strshareduserid(i));
    }

    m_devInfo.m_strItemsList.clear();
    iCountTmp = InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().stritems_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        m_devInfo.m_strItemsList.push_back(InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().stritems(i));
    }

}

void InteractiveProtoHandler::ModifyDevReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ModifyDevReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->set_struserid(m_strUserID);

    InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->set_strdevid(m_devInfo.m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->set_strdevname(m_devInfo.m_strDevName);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->set_strdevpassword(m_devInfo.m_strDevPassword);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->set_uitypeinfo(m_devInfo.m_uiTypeInfo);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->set_strcreatedate(m_devInfo.m_strCreatedate);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->set_uistatus(m_devInfo.m_uiStatus);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->set_strextend(m_devInfo.m_strExtend);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->set_strinnerinfo(m_devInfo.m_strInnerinfo);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->set_strowneruserid(m_devInfo.m_strOwnerUserID);

    for (unsigned int i = 0; i < m_devInfo.m_sharingUserIDList.size(); ++i)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->add_strsharinguserid();
    }
    auto itSharingBegin = m_devInfo.m_sharingUserIDList.begin();
    auto itSharingEnd = m_devInfo.m_sharingUserIDList.end();
    int iSharing = 0;
    while (itSharingBegin != itSharingEnd)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->set_strsharinguserid(iSharing, *itSharingBegin);

        ++iSharing;
        ++itSharingBegin;
    }

    for (unsigned int i = 0; i < m_devInfo.m_sharedUserIDList.size(); ++i)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->add_strshareduserid();
    }
    auto itSharedBegin = m_devInfo.m_sharedUserIDList.begin();
    auto itSharedEnd = m_devInfo.m_sharedUserIDList.end();
    int iShared = 0;
    while (itSharedBegin != itSharedEnd)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->set_strshareduserid(iShared, *itSharedBegin);

        ++iShared;
        ++itSharedBegin;
    }

    for (unsigned int i = 0; i < m_devInfo.m_strItemsList.size(); ++i)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->add_stritems();
    }
    auto itItemBegin = m_devInfo.m_strItemsList.begin();
    auto itItemEnd = m_devInfo.m_strItemsList.end();
    int iItem = 0;
    while (itItemBegin != itItemEnd)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->set_strshareduserid(iItem, *itItemBegin);

        ++iItem;
        ++itItemBegin;
    }
}

void InteractiveProtoHandler::ModifyDevRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().modifydevrsp_usr_value().strvalue();
}

void InteractiveProtoHandler::ModifyDevRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ModifyDevRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_modifydevrsp_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::QueryDevReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().querydevreq_usr_value().struserid();
    m_uiBeginIndex = InteractiveMsg.reqvalue().querydevreq_usr_value().uibeginindex();
    m_strValue = InteractiveMsg.reqvalue().querydevreq_usr_value().strvalue();

}

void InteractiveProtoHandler::QueryDevReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryDevReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_querydevreq_usr_value()->set_struserid(m_strUserID);
    InteractiveMsg.mutable_reqvalue()->mutable_querydevreq_usr_value()->set_uibeginindex(m_uiBeginIndex);
    InteractiveMsg.mutable_reqvalue()->mutable_querydevreq_usr_value()->set_strvalue(m_strValue);

}

void InteractiveProtoHandler::QueryDevRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    UnSerializeDevList<Device>(m_allDevInfoList, InteractiveMsg.rspvalue().querydevrsp_usr_value().alldevinfo());

}

void InteractiveProtoHandler::QueryDevRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryDevRsp_USR_T);

    auto uinfo = InteractiveMsg.mutable_rspvalue()->mutable_loginrsp_usr_value()->mutable_devinfo();
    SerializeDevList<Device>(m_allDevInfoList, uinfo);

}

void InteractiveProtoHandler::SharingDevReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().sharingdevreq_usr_value().struserid();
    m_strToUserID = InteractiveMsg.reqvalue().sharingdevreq_usr_value().strtouserid();
    
    m_devInfo.m_strDevID = InteractiveMsg.reqvalue().sharingdevreq_usr_value().devinfo().strdevid();
    m_devInfo.m_strDevName = InteractiveMsg.reqvalue().sharingdevreq_usr_value().devinfo().strdevname();
    m_devInfo.m_strDevPassword = InteractiveMsg.reqvalue().sharingdevreq_usr_value().devinfo().strdevpassword();
    m_devInfo.m_uiTypeInfo = InteractiveMsg.reqvalue().sharingdevreq_usr_value().devinfo().uitypeinfo();
    m_devInfo.m_strCreatedate = InteractiveMsg.reqvalue().sharingdevreq_usr_value().devinfo().strcreatedate();
    m_devInfo.m_uiStatus = InteractiveMsg.reqvalue().sharingdevreq_usr_value().devinfo().uistatus();
    m_devInfo.m_strExtend = InteractiveMsg.reqvalue().sharingdevreq_usr_value().devinfo().strextend();
    m_devInfo.m_strInnerinfo = InteractiveMsg.reqvalue().sharingdevreq_usr_value().devinfo().strinnerinfo();
    m_devInfo.m_strOwnerUserID = InteractiveMsg.reqvalue().sharingdevreq_usr_value().devinfo().strowneruserid();

    m_devInfo.m_sharingUserIDList.clear();
    int iCountTmp = InteractiveMsg.reqvalue().sharingdevreq_usr_value().devinfo().strsharinguserid_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        m_devInfo.m_sharingUserIDList.push_back(InteractiveMsg.reqvalue().sharingdevreq_usr_value().devinfo().strsharinguserid(i));
    }

    m_devInfo.m_sharedUserIDList.clear();
    iCountTmp = InteractiveMsg.reqvalue().sharingdevreq_usr_value().devinfo().strshareduserid_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        m_devInfo.m_sharedUserIDList.push_back(InteractiveMsg.reqvalue().sharingdevreq_usr_value().devinfo().strshareduserid(i));
    }

    m_devInfo.m_strItemsList.clear();
    iCountTmp = InteractiveMsg.reqvalue().sharingdevreq_usr_value().devinfo().stritems_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        m_devInfo.m_strItemsList.push_back(InteractiveMsg.reqvalue().sharingdevreq_usr_value().devinfo().stritems(i));
    }

    m_uiRelation = InteractiveMsg.reqvalue().sharingdevreq_usr_value().uirelation();
    m_strBeginDate = InteractiveMsg.reqvalue().sharingdevreq_usr_value().strbegindate();
    m_strEndDate = InteractiveMsg.reqvalue().sharingdevreq_usr_value().strenddate();
    m_strCreateDate = InteractiveMsg.reqvalue().sharingdevreq_usr_value().strcreatedate();
    m_strValue = InteractiveMsg.reqvalue().sharingdevreq_usr_value().strvalue();


}

void InteractiveProtoHandler::SharingDevReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::SharingDevReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->set_struserid(m_strUserID);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->set_strtouserid(m_strToUserID);

    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_devinfo()->set_strdevid(m_devInfo.m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_devinfo()->set_strdevname(m_devInfo.m_strDevName);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_devinfo()->set_strdevpassword(m_devInfo.m_strDevPassword);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_devinfo()->set_uitypeinfo(m_devInfo.m_uiTypeInfo);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_devinfo()->set_strcreatedate(m_devInfo.m_strCreatedate);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_devinfo()->set_uistatus(m_devInfo.m_uiStatus);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_devinfo()->set_strextend(m_devInfo.m_strExtend);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_devinfo()->set_strinnerinfo(m_devInfo.m_strInnerinfo);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_devinfo()->set_strowneruserid(m_devInfo.m_strOwnerUserID);

    for (unsigned int i = 0; i < m_devInfo.m_sharingUserIDList.size(); ++i)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_devinfo()->add_strsharinguserid();
    }
    auto itSharingBegin = m_devInfo.m_sharingUserIDList.begin();
    auto itSharingEnd = m_devInfo.m_sharingUserIDList.end();
    int iSharing = 0;
    while (itSharingBegin != itSharingEnd)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_devinfo()->set_strsharinguserid(iSharing, *itSharingBegin);

        ++iSharing;
        ++itSharingBegin;
    }

    for (unsigned int i = 0; i < m_devInfo.m_sharedUserIDList.size(); ++i)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_devinfo()->add_strshareduserid();
    }
    auto itSharedBegin = m_devInfo.m_sharedUserIDList.begin();
    auto itSharedEnd = m_devInfo.m_sharedUserIDList.end();
    int iShared = 0;
    while (itSharedBegin != itSharedEnd)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_devinfo()->set_strshareduserid(iShared, *itSharedBegin);

        ++iShared;
        ++itSharedBegin;
    }

    for (unsigned int i = 0; i < m_devInfo.m_strItemsList.size(); ++i)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_devinfo()->add_stritems();
    }
    auto itItemBegin = m_devInfo.m_strItemsList.begin();
    auto itItemEnd = m_devInfo.m_strItemsList.end();
    int iItem = 0;
    while (itItemBegin != itItemEnd)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_devinfo()->set_strshareduserid(iItem, *itItemBegin);

        ++iItem;
        ++itItemBegin;
    }

    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->set_uirelation(m_uiRelation);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->set_strbegindate(m_strBeginDate);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->set_strenddate(m_strEndDate);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->set_strcreatedate(m_strCreateDate);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->set_strvalue(m_strValue);

}

void InteractiveProtoHandler::SharingDevRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().sharingdevrsp_usr_value().strvalue();
}

void InteractiveProtoHandler::SharingDevRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::SharingDevRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_sharingdevrsp_usr_value()->set_strvalue(m_strValue);
}


void InteractiveProtoHandler::CancelSharedDevReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().cancelshareddevreq_usr_value().struserid();
    m_strDevID = InteractiveMsg.reqvalue().cancelshareddevreq_usr_value().strdevid();
    m_strValue = InteractiveMsg.reqvalue().cancelshareddevreq_usr_value().strvalue();
}

void InteractiveProtoHandler::CancelSharedDevReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::CancelSharedDevReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_cancelshareddevreq_usr_value()->set_struserid(m_strUserID);
    InteractiveMsg.mutable_reqvalue()->mutable_cancelshareddevreq_usr_value()->set_strdevid(m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_cancelshareddevreq_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::CancelSharedDevRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().cancelshareddevrsp_usr_value().strvalue();
}

void InteractiveProtoHandler::CancelSharedDevRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::CancelSharedDevRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_cancelshareddevrsp_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::AddFriendsReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().addfriendsreq_usr_value().struserid();
    m_strFriendUserID = InteractiveMsg.reqvalue().addfriendsreq_usr_value().strfrienduserid();
    
}

void InteractiveProtoHandler::AddFriendsReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::AddFriendsReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_addfriendsreq_usr_value()->set_struserid(m_strUserID);
    InteractiveMsg.mutable_reqvalue()->mutable_addfriendsreq_usr_value()->set_strfrienduserid(m_strFriendUserID);
    
}

void InteractiveProtoHandler::AddFriendsRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().addfriendsrsp_usr_value().strvalue();
}

void InteractiveProtoHandler::AddFriendsRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::AddFriendsRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_addfriendsrsp_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::DelFriendsReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().delfriendsreq_usr_value().struserid();

    m_strFriendUserIDList.clear();
    int iCountTmp = InteractiveMsg.reqvalue().delfriendsreq_usr_value().strfrienduserid_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        m_strFriendUserIDList.push_back(InteractiveMsg.reqvalue().delfriendsreq_usr_value().strfrienduserid(i));
    }

}

void InteractiveProtoHandler::DelFriendsReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::DelFriendsReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_delfriendsreq_usr_value()->set_struserid(m_strUserID);

    for (unsigned int i = 0; i < m_strFriendUserIDList.size(); ++i)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_delfriendsreq_usr_value()->add_strfrienduserid();
    }
    auto itBegin = m_strFriendUserIDList.begin();
    auto itEnd = m_strFriendUserIDList.end();
    int iUser = 0;
    while (itBegin != itEnd)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_delfriendsreq_usr_value()->set_strfrienduserid(iUser, *itBegin);

        ++iUser;
        ++itBegin;
    }
}


void InteractiveProtoHandler::DelFriendsRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().delfriendsrsp_usr_value().strvalue();

    m_strFriendUserIDFailedList.clear();
    int iCountTmp = InteractiveMsg.rspvalue().delfriendsrsp_usr_value().strfrienduseridfailed_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        m_strFriendUserIDFailedList.push_back(InteractiveMsg.rspvalue().delfriendsrsp_usr_value().strfrienduseridfailed(i));
    }

}

void InteractiveProtoHandler::DelFriendsRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::DelFriendsRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_delfriendsrsp_usr_value()->set_strvalue(m_strValue);

    for (unsigned int i = 0; i < m_strFriendUserIDFailedList.size(); ++i)
    {
        InteractiveMsg.mutable_rspvalue()->mutable_delfriendsrsp_usr_value()->add_strfrienduseridfailed();
    }
    auto itBegin = m_strFriendUserIDFailedList.begin();
    auto itEnd = m_strFriendUserIDFailedList.end();
    int iUser = 0;
    while (itBegin != itEnd)
    {
        InteractiveMsg.mutable_rspvalue()->mutable_delfriendsrsp_usr_value()->set_strfrienduseridfailed(iUser, *itBegin);

        ++iUser;
        ++itBegin;
    }
}

void InteractiveProtoHandler::QueryFriendsReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().queryfriendsreq_usr_value().struserid();
    m_strValue = InteractiveMsg.reqvalue().queryfriendsreq_usr_value().strvalue();

}

void InteractiveProtoHandler::QueryFriendsReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryFriendsReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfriendsreq_usr_value()->set_struserid(m_strUserID);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfriendsreq_usr_value()->set_strvalue(m_strValue);

}

void InteractiveProtoHandler::QueryFriendsRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().queryfriendsrsp_usr_value().strvalue();

    m_allFriendUserInfoList.clear();
    int iCountTmp = InteractiveMsg.rspvalue().queryfriendsrsp_usr_value().allfrienduserinfo_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        auto ItUser = InteractiveMsg.rspvalue().queryfriendsrsp_usr_value().allfrienduserinfo(i);

        User usr;
        usr.m_strUserID = ItUser.struserid();
        usr.m_strUserName = ItUser.strusername();
        usr.m_strUserPassword = ItUser.struserpassword();
        usr.m_uiTypeInfo = ItUser.uitypeinfo();
        usr.m_strCreatedate = ItUser.strcreatedate();
        usr.m_uiStatus = ItUser.uistatus();
        usr.m_strExtend = ItUser.strextend();

        UnSerializeDevList<Device>(usr.m_ownerDevInfoList, ItUser.ownerdevinfo());
        UnSerializeDevList<Device>(usr.m_sharingDevInfoList, ItUser.sharingdevinfo());
        UnSerializeDevList<Device>(usr.m_sharedDevInfoList, ItUser.shareddevinfo());

        usr.m_strItemsList.clear();
        int iCount = ItUser.stritems_size();
        for (int k = 0; k < iCount; ++k)
        {
            usr.m_strItemsList.push_back(ItUser.stritems(k));
        }

        m_allFriendUserInfoList.push_back(usr);
    }

}

void InteractiveProtoHandler::QueryFriendsRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryFriendsRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_queryfriendsrsp_usr_value()->set_strvalue(m_strValue);

    for (unsigned int i = 0; i < m_allFriendUserInfoList.size(); ++i)
    {
        InteractiveMsg.mutable_rspvalue()->mutable_queryfriendsrsp_usr_value()->add_allfrienduserinfo();
    }
    auto itBegin = m_allFriendUserInfoList.begin();
    auto itEnd = m_allFriendUserInfoList.end();
    int iUser = 0;
    while (itBegin != itEnd)
    {
        auto uinfo = InteractiveMsg.mutable_rspvalue()->mutable_queryfriendsrsp_usr_value()->mutable_allfrienduserinfo(iUser);

        uinfo->set_struserid(itBegin->m_strUserID);
        uinfo->set_strusername(itBegin->m_strUserName);
        uinfo->set_struserpassword(itBegin->m_strUserPassword);
        uinfo->set_uitypeinfo(itBegin->m_uiTypeInfo);
        uinfo->set_strcreatedate(itBegin->m_strCreatedate);
        uinfo->set_uistatus(itBegin->m_uiStatus);
        uinfo->set_strextend(itBegin->m_strExtend);

        SerializeDevList<Device>(itBegin->m_ownerDevInfoList, uinfo->mutable_ownerdevinfo());
        SerializeDevList<Device>(itBegin->m_sharingDevInfoList, uinfo->mutable_sharingdevinfo());
        SerializeDevList<Device>(itBegin->m_sharedDevInfoList, uinfo->mutable_shareddevinfo());

        for (unsigned int i = 0; i < itBegin->m_strItemsList.size(); ++i)
        {
            uinfo->add_stritems();
        }
        auto itBeginItem = itBegin->m_strItemsList.begin();
        auto itEnditem = itBegin->m_strItemsList.end();
        int i = 0;
        while (itBeginItem != itEnditem)
        {

            uinfo->set_stritems(i, *itBeginItem);

            ++i;
            ++itBeginItem;
        }
        
        ++iUser;
        ++itBegin;
    }
}

void InteractiveProtoHandler::GetOnlineDevInfoReq_INNER::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.reqvalue().getonlinedevinforeq_inner_value().strvalue();

}

void InteractiveProtoHandler::GetOnlineDevInfoReq_INNER::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::GetOnlineDevInfoReq_INNER_T);
    InteractiveMsg.mutable_reqvalue()->mutable_getonlinedevinforeq_inner_value()->set_strvalue(m_strValue);

}

void InteractiveProtoHandler::GetOnlineDevInfoRsp_INNER::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    UnSerializeDevList<Device>(m_devInfoList, InteractiveMsg.rspvalue().getonlinedevinforsp_inner_value().devinfo());
    m_strValue = InteractiveMsg.rspvalue().getonlinedevinforsp_inner_value().strvalue();

}

void InteractiveProtoHandler::GetOnlineDevInfoRsp_INNER::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::GetOnlineDevInfoRsp_INNER_T);

    auto uinfo = InteractiveMsg.mutable_rspvalue()->mutable_getonlinedevinforsp_inner_value()->mutable_devinfo();
    SerializeDevList<Device>(m_devInfoList, uinfo);
    InteractiveMsg.mutable_rspvalue()->mutable_getonlinedevinforsp_inner_value()->set_strvalue(m_strValue);

}

void InteractiveProtoHandler::BroadcastOnlineDevInfo_INNER::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    UnSerializeDevList<Device>(m_devInfoList, InteractiveMsg.reqvalue().broadcastonlinedevinfo_inner_value().devinfo());
    m_strValue = InteractiveMsg.reqvalue().broadcastonlinedevinfo_inner_value().strvalue();

}

void InteractiveProtoHandler::BroadcastOnlineDevInfo_INNER::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::BroadcastOnlineDevInfo_INNER_T);

    auto uinfo = InteractiveMsg.mutable_reqvalue()->mutable_broadcastonlinedevinfo_inner_value()->mutable_devinfo();
    SerializeDevList<Device>(m_devInfoList, uinfo);
    InteractiveMsg.mutable_reqvalue()->mutable_broadcastonlinedevinfo_inner_value()->set_strvalue(m_strValue);

}

void InteractiveProtoHandler::GetOnlineUserInfoReq_INNER::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.reqvalue().getonlineuserinforeq_inner_value().strvalue();

}

void InteractiveProtoHandler::GetOnlineUserInfoReq_INNER::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::GetOnlineUserInfoReq_INNER_T);
    InteractiveMsg.mutable_reqvalue()->mutable_getonlineuserinforeq_inner_value()->set_strvalue(m_strValue);

}

void InteractiveProtoHandler::GetOnlineUserInfoRsp_INNER::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().getonlineuserinforsp_inner_value().strvalue();

    m_userInfoList.clear();
    int iCountTmp = InteractiveMsg.rspvalue().getonlineuserinforsp_inner_value().userinfo_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        auto ItUser = InteractiveMsg.rspvalue().getonlineuserinforsp_inner_value().userinfo(i);

        User usr;
        usr.m_strUserID = ItUser.struserid();
        usr.m_strUserName = ItUser.strusername();
        usr.m_strUserPassword = ItUser.struserpassword();
        usr.m_uiTypeInfo = ItUser.uitypeinfo();
        usr.m_strCreatedate = ItUser.strcreatedate();
        usr.m_uiStatus = ItUser.uistatus();
        usr.m_strExtend = ItUser.strextend();

        UnSerializeDevList<Device>(usr.m_ownerDevInfoList, ItUser.ownerdevinfo());
        UnSerializeDevList<Device>(usr.m_sharingDevInfoList, ItUser.sharingdevinfo());
        UnSerializeDevList<Device>(usr.m_sharedDevInfoList, ItUser.shareddevinfo());

        usr.m_strItemsList.clear();
        int iCount = ItUser.stritems_size();
        for (int k = 0; k < iCount; ++k)
        {
            usr.m_strItemsList.push_back(ItUser.stritems(k));
        }

        m_userInfoList.push_back(usr);
    }

}

void InteractiveProtoHandler::GetOnlineUserInfoRsp_INNER::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::GetOnlineUserInfoRsp_INNER_T);
    InteractiveMsg.mutable_rspvalue()->mutable_getonlineuserinforsp_inner_value()->set_strvalue(m_strValue);

    for (unsigned int i = 0; i < m_userInfoList.size(); ++i)
    {
        InteractiveMsg.mutable_rspvalue()->mutable_getonlineuserinforsp_inner_value()->add_userinfo();
    }
    auto itBegin = m_userInfoList.begin();
    auto itEnd = m_userInfoList.end();
    int iUser = 0;
    while (itBegin != itEnd)
    {
        auto uinfo = InteractiveMsg.mutable_rspvalue()->mutable_getonlineuserinforsp_inner_value()->mutable_userinfo(iUser);

        uinfo->set_struserid(itBegin->m_strUserID);
        uinfo->set_strusername(itBegin->m_strUserName);
        uinfo->set_struserpassword(itBegin->m_strUserPassword);
        uinfo->set_uitypeinfo(itBegin->m_uiTypeInfo);
        uinfo->set_strcreatedate(itBegin->m_strCreatedate);
        uinfo->set_uistatus(itBegin->m_uiStatus);
        uinfo->set_strextend(itBegin->m_strExtend);

        SerializeDevList<Device>(itBegin->m_ownerDevInfoList, uinfo->mutable_ownerdevinfo());
        SerializeDevList<Device>(itBegin->m_sharingDevInfoList, uinfo->mutable_sharingdevinfo());
        SerializeDevList<Device>(itBegin->m_sharedDevInfoList, uinfo->mutable_shareddevinfo());

        for (unsigned int i = 0; i < itBegin->m_strItemsList.size(); ++i)
        {
            uinfo->add_stritems();
        }
        auto itBeginItem = itBegin->m_strItemsList.begin();
        auto itEnditem = itBegin->m_strItemsList.end();
        int i = 0;
        while (itBeginItem != itEnditem)
        {

            uinfo->set_stritems(i, *itBeginItem);

            ++i;
            ++itBeginItem;
        }

        ++iUser;
        ++itBegin;
    }
}

void InteractiveProtoHandler::BroadcastOnlineUserInfo_INNER::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.reqvalue().broadcastonlineuserinfo_inner_value().strvalue();

    m_userInfoList.clear();
    int iCountTmp = InteractiveMsg.reqvalue().broadcastonlineuserinfo_inner_value().userinfo_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        auto ItUser = InteractiveMsg.reqvalue().broadcastonlineuserinfo_inner_value().userinfo(i);

        User usr;
        usr.m_strUserID = ItUser.struserid();
        usr.m_strUserName = ItUser.strusername();
        usr.m_strUserPassword = ItUser.struserpassword();
        usr.m_uiTypeInfo = ItUser.uitypeinfo();
        usr.m_strCreatedate = ItUser.strcreatedate();
        usr.m_uiStatus = ItUser.uistatus();
        usr.m_strExtend = ItUser.strextend();

        UnSerializeDevList<Device>(usr.m_ownerDevInfoList, ItUser.ownerdevinfo());
        UnSerializeDevList<Device>(usr.m_sharingDevInfoList, ItUser.sharingdevinfo());
        UnSerializeDevList<Device>(usr.m_sharedDevInfoList, ItUser.shareddevinfo());

        usr.m_strItemsList.clear();
        int iCount = ItUser.stritems_size();
        for (int k = 0; k < iCount; ++k)
        {
            usr.m_strItemsList.push_back(ItUser.stritems(k));
        }

        m_userInfoList.push_back(usr);
    }

}

void InteractiveProtoHandler::BroadcastOnlineUserInfo_INNER::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::BroadcastOnlineUserInfo_INNER_T);
    InteractiveMsg.mutable_reqvalue()->mutable_broadcastonlineuserinfo_inner_value()->set_strvalue(m_strValue);

    for (unsigned int i = 0; i < m_userInfoList.size(); ++i)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_broadcastonlineuserinfo_inner_value()->add_userinfo();
    }
    auto itBegin = m_userInfoList.begin();
    auto itEnd = m_userInfoList.end();
    int iUser = 0;
    while (itBegin != itEnd)
    {
        auto uinfo = InteractiveMsg.mutable_reqvalue()->mutable_broadcastonlineuserinfo_inner_value()->mutable_userinfo(iUser);

        uinfo->set_struserid(itBegin->m_strUserID);
        uinfo->set_strusername(itBegin->m_strUserName);
        uinfo->set_struserpassword(itBegin->m_strUserPassword);
        uinfo->set_uitypeinfo(itBegin->m_uiTypeInfo);
        uinfo->set_strcreatedate(itBegin->m_strCreatedate);
        uinfo->set_uistatus(itBegin->m_uiStatus);
        uinfo->set_strextend(itBegin->m_strExtend);

        SerializeDevList<Device>(itBegin->m_ownerDevInfoList, uinfo->mutable_ownerdevinfo());
        SerializeDevList<Device>(itBegin->m_sharingDevInfoList, uinfo->mutable_sharingdevinfo());
        SerializeDevList<Device>(itBegin->m_sharedDevInfoList, uinfo->mutable_shareddevinfo());

        for (unsigned int i = 0; i < itBegin->m_strItemsList.size(); ++i)
        {
            uinfo->add_stritems();
        }
        auto itBeginItem = itBegin->m_strItemsList.begin();
        auto itEnditem = itBegin->m_strItemsList.end();
        int i = 0;
        while (itBeginItem != itEnditem)
        {

            uinfo->set_stritems(i, *itBeginItem);

            ++i;
            ++itBeginItem;
        }

        ++iUser;
        ++itBegin;
    }
}






