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
                        
        ++i;
        ++itBegin;
    }
}

void UnSerializeRelationList(std::list<InteractiveProtoHandler::Relation> &usrInfoList,
    const ::google::protobuf::RepeatedPtrField< ::Interactive::Message::Relation > &srcReInfoList)
{
    usrInfoList.clear();
    int iCount = srcReInfoList.size();
    for (int i = 0; i < iCount; ++i)
    {
        InteractiveProtoHandler::Relation reInfo;
        reInfo.m_strUserID = srcReInfoList.Get(i).struserid();
        reInfo.m_strDevID = srcReInfoList.Get(i).strdevid();
        reInfo.m_uiRelation = srcReInfoList.Get(i).uirelation();
        reInfo.m_strBeginDate = srcReInfoList.Get(i).strbegindate();
        reInfo.m_strEndDate = srcReInfoList.Get(i).strenddate();
        reInfo.m_strValue = srcReInfoList.Get(i).strvalue();
        
        usrInfoList.push_back(std::move(reInfo));
    }
}

void SerializeRelationList(const std::list<InteractiveProtoHandler::Relation> &reInfoList,
     ::google::protobuf::RepeatedPtrField< ::Interactive::Message::Relation >* pDstReInfoList)
 {
     for (unsigned int i = 0; i < reInfoList.size(); ++i)
     {
         pDstReInfoList->Add();
     }

     auto itBegin = reInfoList.begin();
     auto itEnd = reInfoList.end();
     int i = 0;
     while (itBegin != itEnd)
     {
         pDstReInfoList->Mutable(i)->set_struserid(itBegin->m_strUserID);
         pDstReInfoList->Mutable(i)->set_strdevid(itBegin->m_strDevID);
         pDstReInfoList->Mutable(i)->set_uirelation(itBegin->m_uiRelation);
         pDstReInfoList->Mutable(i)->set_strbegindate(itBegin->m_strBeginDate);
         pDstReInfoList->Mutable(i)->set_strenddate(itBegin->m_strEndDate);
         pDstReInfoList->Mutable(i)->set_strvalue(itBegin->m_strValue);

         ++i;
         ++itBegin;
     }
 }

void UnSerializeFileList(std::list<InteractiveProtoHandler::File> &fileInfoList,
    const ::google::protobuf::RepeatedPtrField< ::Interactive::Message::File > &srcFileInfoList)
{
    fileInfoList.clear();

    unsigned int iCount = srcFileInfoList.size();
    for (unsigned int i = 0; i < iCount; ++i)
    {
        auto srcFileInfo = srcFileInfoList.Get(i);
        InteractiveProtoHandler::File fileInfo;
        fileInfo.m_strFileID = srcFileInfo.strfileid();
        fileInfo.m_strUserID = srcFileInfo.struserid();
        fileInfo.m_strDevID = srcFileInfo.strdevid();
        fileInfo.m_strRemoteFileID = srcFileInfo.strremotefileid();
        fileInfo.m_strDownloadUrl = srcFileInfo.strdownloadurl();
        fileInfo.m_strFileName = srcFileInfo.strfilename();
        fileInfo.m_strSuffixName = srcFileInfo.strsuffixname();
        fileInfo.m_uiFileSize = srcFileInfo.uifilesize();
        fileInfo.m_strFileCreatedate = srcFileInfo.strfilecreatedate();
        fileInfo.m_strCreatedate = srcFileInfo.strcreatedate();
        fileInfo.m_uiStatus = srcFileInfo.uistatus();
        fileInfo.m_strExtend = srcFileInfo.strextend();

        fileInfoList.push_back(std::move(fileInfo));
    }
}

void SerializeFileList(const std::list<InteractiveProtoHandler::File> &fileInfoList,
    ::google::protobuf::RepeatedPtrField< ::Interactive::Message::File >* pDstFileInfoList)
{
    unsigned int iCount = fileInfoList.size();
    for (unsigned int i = 0; i < iCount; ++i)
    {
        pDstFileInfoList->Add();
    }

    auto itBegin = fileInfoList.begin();
    auto itEnd = fileInfoList.end();
    int i = 0;
    while (itBegin != itEnd)
    {
        auto pDstFileInfo = pDstFileInfoList->Mutable(i);
        pDstFileInfo->set_strfileid(itBegin->m_strFileID);
        pDstFileInfo->set_struserid(itBegin->m_strUserID);
        pDstFileInfo->set_strdevid(itBegin->m_strDevID);
        pDstFileInfo->set_strremotefileid(itBegin->m_strRemoteFileID);
        pDstFileInfo->set_strdownloadurl(itBegin->m_strDownloadUrl);
        pDstFileInfo->set_strfilename(itBegin->m_strFileName);
        pDstFileInfo->set_strsuffixname(itBegin->m_strSuffixName);
        pDstFileInfo->set_uifilesize(itBegin->m_uiFileSize);
        pDstFileInfo->set_strfilecreatedate(itBegin->m_strFileCreatedate);
        pDstFileInfo->set_strcreatedate(itBegin->m_strCreatedate);
        pDstFileInfo->set_uistatus(itBegin->m_uiStatus);
        pDstFileInfo->set_strextend(itBegin->m_strExtend);

        ++i;
        ++itBegin;
    }
}

void UnSerializeFileUrlList(std::list<InteractiveProtoHandler::FileUrl> &fileUrlList,
    const ::google::protobuf::RepeatedPtrField< ::Interactive::Message::FileUrl > &srcFileUrlList)
{
    fileUrlList.clear();

    unsigned int iCount = srcFileUrlList.size();
    for (unsigned int i = 0; i < iCount; ++i)
    {
        auto srcFileUrl = srcFileUrlList.Get(i);
        InteractiveProtoHandler::FileUrl fileUrl;
        fileUrl.m_strFileID = srcFileUrl.strfileid();
        fileUrl.m_strDownloadUrl = srcFileUrl.strdownloadurl();

        fileUrlList.push_back(std::move(fileUrl));
    }
}

void SerializeFileUrlList(const std::list<InteractiveProtoHandler::FileUrl> &fileUrlList,
    ::google::protobuf::RepeatedPtrField< ::Interactive::Message::FileUrl >* pDstFileUrlList)
{
    unsigned int iCount = fileUrlList.size();
    for (unsigned int i = 0; i < iCount; ++i)
    {
        pDstFileUrlList->Add();
    }

    auto itBegin = fileUrlList.begin();
    auto itEnd = fileUrlList.end();
    int i = 0;
    while (itBegin != itEnd)
    {
        auto pDstFileUrl = pDstFileUrlList->Mutable(i);
        pDstFileUrl->set_strfileid(itBegin->m_strFileID);
        pDstFileUrl->set_strdownloadurl(itBegin->m_strDownloadUrl);

        ++i;
        ++itBegin;
    }
}


InteractiveProtoHandler::InteractiveProtoHandler()
{
    //////////////////////////////////////////////////////
    
    SerializeHandler handler;

    handler.Szr = boost::bind(&InteractiveProtoHandler::MsgPreHandlerReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::MsgPreHandlerReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::MsgPreHandlerReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::MsgPreHandlerRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::MsgPreHandlerRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::MsgPreHandlerRsp_USR_T, handler));

    /////////////////////////////////////////////////////

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

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryUsrInfoReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryUsrInfoReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryUsrInfoReq_USR_T, handler));

    ////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryUsrInfoRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryUsrInfoRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryUsrInfoRsp_USR_T, handler));


    /////////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::ModifyUserInfoReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::ModifyUserInfoReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::ModifyUserInfoReq_USR_T, handler));

    ////

    handler.Szr = boost::bind(&InteractiveProtoHandler::ModifyUserInfoRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::ModifyUserInfoRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::ModifyUserInfoRsp_USR_T, handler));

    ////////////////////////////////////////////////////////
    
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

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryDevInfoReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryDevInfoReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryDevInfoReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryDevInfoRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryDevInfoRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryDevInfoRsp_USR_T, handler));

    ///////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryDevReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryDevReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryDevReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryDevRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryDevRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryDevRsp_USR_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryUserReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryUserReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryUserReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryUserRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryUserRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryUserRsp_USR_T, handler));

    /////////////////////////////////////////////////////

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

    handler.Szr = boost::bind(&InteractiveProtoHandler::DeleteFileReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::DeleteFileReq_USR_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::DeleteFileReq_USR_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoHandler::DeleteFileRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::DeleteFileRsp_USR_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::DeleteFileRsp_USR_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::DownloadFileReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::DownloadFileReq_USR_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::DownloadFileReq_USR_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoHandler::DownloadFileRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::DownloadFileRsp_USR_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::DownloadFileRsp_USR_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryFileReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryFileReq_USR_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryFileReq_USR_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryFileRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryFileRsp_USR_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryFileRsp_USR_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::GetOnlineUserInfoReq_INNER_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::GetOnlineUserInfoReq_INNER_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::GetOnlineUserInfoReq_INNER_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoHandler::GetOnlineUserInfoRsp_INNER_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::GetOnlineUserInfoRsp_INNER_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::GetOnlineUserInfoRsp_INNER_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoHandler::BroadcastOnlineUserInfo_INNER_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::BroadcastOnlineUserInfo_INNER_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::BroadcastOnlineUserInfo_INNER_T, handler));
    
    //////////////////////////////////////////////////
    
    handler.Szr = boost::bind(&InteractiveProtoHandler::LoginReq_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::LoginReq_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::LoginReq_DEV_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::LoginRsp_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::LoginRsp_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::LoginRsp_DEV_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::P2pInfoReq_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::P2pInfoReq_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::P2pInfoReq_DEV_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::P2pInfoRsp_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::P2pInfoRsp_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::P2pInfoRsp_DEV_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::ShakehandReq_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::ShakehandReq_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::ShakehandReq_DEV_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::ShakehandRsp_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::ShakehandRsp_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::ShakehandRsp_DEV_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::LogoutReq_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::LogoutReq_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::LogoutReq_DEV_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::LogoutRsp_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::LogoutRsp_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::LogoutRsp_DEV_T, handler));
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

bool InteractiveProtoHandler::UnSerializeReqBase(const std::string &strData, Req &req)
{
    InteractiveMessage Msg;
    Msg.Clear();
    if (!Msg.ParseFromString(strData))
    {
        return false;
    }

    req.UnSerializer(Msg);

    return true;
}

bool InteractiveProtoHandler::MsgPreHandlerReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<MsgPreHandlerReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::MsgPreHandlerReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<MsgPreHandlerReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::MsgPreHandlerRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<MsgPreHandlerRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::MsgPreHandlerRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<MsgPreHandlerRsp_USR, Req>(InteractiveMsg, rsp);
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

bool InteractiveProtoHandler::QueryUsrInfoReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryUsrInfoReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryUsrInfoReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryUsrInfoReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryUsrInfoRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryUsrInfoRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryUsrInfoRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryUsrInfoRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::ModifyUserInfoReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<ModifyUserInfoReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::ModifyUserInfoReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<ModifyUserInfoReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::ModifyUserInfoRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<ModifyUserInfoRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::ModifyUserInfoRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<ModifyUserInfoRsp_USR, Req>(InteractiveMsg, rsp);
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

bool InteractiveProtoHandler::QueryDevInfoReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryDevInfoReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryDevInfoReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryDevInfoReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryDevInfoRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryDevInfoRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryDevInfoRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryDevInfoRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryDevReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryDevReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryDevReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryDevReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryDevRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryDevRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryDevRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryDevRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryUserReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryUserReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryUserReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryUserReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryUserRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryUserRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryUserRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryUserRsp_USR, Req>(InteractiveMsg, rsp);
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

bool InteractiveProtoHandler::DeleteFileReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<DeleteFileReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::DeleteFileReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<DeleteFileReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::DeleteFileRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<DeleteFileRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::DeleteFileRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<DeleteFileRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::DownloadFileReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<DownloadFileReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::DownloadFileReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<DownloadFileReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::DownloadFileRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<DownloadFileRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::DownloadFileRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<DownloadFileRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryFileReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryFileReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryFileReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryFileReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryFileRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryFileRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryFileRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryFileRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::AddFileReq_DEV_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<AddFileReq_DEV, Req>(req, strOutput);
}

bool InteractiveProtoHandler::AddFileReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<AddFileReq_DEV, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::AddFileRsp_DEV_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<AddFileRsp_DEV, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::AddFileRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<AddFileRsp_DEV, Req>(InteractiveMsg, rsp);
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

bool InteractiveProtoHandler::LoginReq_DEV_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<LoginReq_DEV, Req>(req, strOutput);
}

bool InteractiveProtoHandler::LoginReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<LoginReq_DEV, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::LoginRsp_DEV_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<LoginRsp_DEV, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::LoginRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<LoginRsp_DEV, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::P2pInfoReq_DEV_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<P2pInfoReq_DEV, Req>(req, strOutput);
}

bool InteractiveProtoHandler::P2pInfoReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<P2pInfoReq_DEV, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::P2pInfoRsp_DEV_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<P2pInfoRsp_DEV, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::P2pInfoRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<P2pInfoRsp_DEV, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::ShakehandReq_DEV_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<ShakehandReq_DEV, Req>(req, strOutput);
}

bool InteractiveProtoHandler::ShakehandReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<ShakehandReq_DEV, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::ShakehandRsp_DEV_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<ShakehandRsp_DEV, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::ShakehandRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<ShakehandRsp_DEV, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::LogoutReq_DEV_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<LogoutReq_DEV, Req>(req, strOutput);
}

bool InteractiveProtoHandler::LogoutReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<LogoutReq_DEV, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::LogoutRsp_DEV_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<LogoutRsp_DEV, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::LogoutRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<LogoutRsp_DEV, Req>(InteractiveMsg, rsp);
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

void InteractiveProtoHandler::MsgPreHandlerReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.reqvalue().msgprehandlerreq_usr_value().strvalue();
}

void InteractiveProtoHandler::MsgPreHandlerReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::MsgPreHandlerReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_msgprehandlerreq_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::MsgPreHandlerRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().msgprehandlerrsp_usr_value().strvalue();
}

void InteractiveProtoHandler::MsgPreHandlerRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::MsgPreHandlerRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_msgprehandlerrsp_usr_value()->set_strvalue(m_strValue);
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

    m_strValue = InteractiveMsg.reqvalue().registeruserreq_usr_value().strvalue();
    

    ////¡–±Ì Ù–‘
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

void InteractiveProtoHandler::QueryUsrInfoReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().queryusrinforeq_usr_value().struserid();
    m_strValue = InteractiveMsg.reqvalue().queryusrinforeq_usr_value().strvalue();

}

void InteractiveProtoHandler::QueryUsrInfoReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryUsrInfoReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_queryusrinforeq_usr_value()->set_struserid(m_strUserID);
    InteractiveMsg.mutable_reqvalue()->mutable_queryusrinforeq_usr_value()->set_strvalue(m_strValue);

}

void InteractiveProtoHandler::QueryUsrInfoRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);

    m_userInfo.m_strCreatedate = InteractiveMsg.rspvalue().queryusrinforsp_usr_value().userinfo().strcreatedate();
    m_userInfo.m_strExtend = InteractiveMsg.rspvalue().queryusrinforsp_usr_value().userinfo().strextend();
    m_userInfo.m_strUserID = InteractiveMsg.rspvalue().queryusrinforsp_usr_value().userinfo().struserid();
    m_userInfo.m_strUserName = InteractiveMsg.rspvalue().queryusrinforsp_usr_value().userinfo().strusername();
    m_userInfo.m_strUserPassword = InteractiveMsg.rspvalue().queryusrinforsp_usr_value().userinfo().struserpassword();
    m_userInfo.m_uiStatus = InteractiveMsg.rspvalue().queryusrinforsp_usr_value().userinfo().uistatus();
    m_userInfo.m_uiTypeInfo = InteractiveMsg.rspvalue().queryusrinforsp_usr_value().userinfo().uitypeinfo();

    m_strValue = InteractiveMsg.rspvalue().queryusrinforsp_usr_value().strvalue();

}

void InteractiveProtoHandler::QueryUsrInfoRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryUsrInfoRsp_USR_T);

    InteractiveMsg.mutable_rspvalue()->mutable_queryusrinforsp_usr_value()->mutable_userinfo()->set_strcreatedate(m_userInfo.m_strCreatedate);
    InteractiveMsg.mutable_rspvalue()->mutable_queryusrinforsp_usr_value()->mutable_userinfo()->set_strextend(m_userInfo.m_strExtend);
    InteractiveMsg.mutable_rspvalue()->mutable_queryusrinforsp_usr_value()->mutable_userinfo()->set_struserid(m_userInfo.m_strUserID);
    InteractiveMsg.mutable_rspvalue()->mutable_queryusrinforsp_usr_value()->mutable_userinfo()->set_strusername(m_userInfo.m_strUserName);
    InteractiveMsg.mutable_rspvalue()->mutable_queryusrinforsp_usr_value()->mutable_userinfo()->set_struserpassword(m_userInfo.m_strUserPassword);
    InteractiveMsg.mutable_rspvalue()->mutable_queryusrinforsp_usr_value()->mutable_userinfo()->set_uistatus(m_userInfo.m_uiStatus);
    InteractiveMsg.mutable_rspvalue()->mutable_queryusrinforsp_usr_value()->mutable_userinfo()->set_uitypeinfo(m_userInfo.m_uiTypeInfo);

    InteractiveMsg.mutable_rspvalue()->mutable_queryusrinforsp_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::ModifyUserInfoReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_userInfo.m_strUserID = InteractiveMsg.reqvalue().modifyuserinforeq_usr_value().userinfo().struserid();
    m_userInfo.m_strUserName = InteractiveMsg.reqvalue().modifyuserinforeq_usr_value().userinfo().strusername();
    m_userInfo.m_strUserPassword = InteractiveMsg.reqvalue().modifyuserinforeq_usr_value().userinfo().struserpassword();
    m_userInfo.m_uiTypeInfo = InteractiveMsg.reqvalue().modifyuserinforeq_usr_value().userinfo().uitypeinfo();
    m_userInfo.m_strCreatedate = InteractiveMsg.reqvalue().modifyuserinforeq_usr_value().userinfo().strcreatedate();
    m_userInfo.m_uiStatus = InteractiveMsg.reqvalue().modifyuserinforeq_usr_value().userinfo().uistatus();
    m_userInfo.m_strExtend = InteractiveMsg.reqvalue().modifyuserinforeq_usr_value().userinfo().strextend();
}

void InteractiveProtoHandler::ModifyUserInfoReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ModifyUserInfoReq_USR_T);

    auto uinfo = InteractiveMsg.mutable_reqvalue()->mutable_modifyuserinforeq_usr_value()->mutable_userinfo();
    uinfo->set_struserid(m_userInfo.m_strUserID);
    uinfo->set_strusername(m_userInfo.m_strUserName);
    uinfo->set_struserpassword(m_userInfo.m_strUserPassword);
    uinfo->set_uitypeinfo(m_userInfo.m_uiTypeInfo);
    uinfo->set_strcreatedate(m_userInfo.m_strCreatedate);
    uinfo->set_uistatus(m_userInfo.m_uiStatus);
    uinfo->set_strextend(m_userInfo.m_strExtend);
}

void InteractiveProtoHandler::ModifyUserInfoRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().modifyuserinforsp_usr_value().strvalue();
}

void InteractiveProtoHandler::ModifyUserInfoRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ModifyUserInfoRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_modifyuserinforsp_usr_value()->set_strvalue(m_strValue);
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

}

void InteractiveProtoHandler::LoginRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.rspvalue().loginrsp_usr_value().struserid();
    UnSerializeRelationList(m_reInfoList, InteractiveMsg.rspvalue().loginrsp_usr_value().relationinfo());
    m_strValue = InteractiveMsg.rspvalue().loginrsp_usr_value().strvalue();

}

void InteractiveProtoHandler::LoginRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::LoginRsp_USR_T);

    InteractiveMsg.mutable_rspvalue()->mutable_loginrsp_usr_value()->set_strvalue(m_strValue);
    InteractiveMsg.mutable_rspvalue()->mutable_loginrsp_usr_value()->set_struserid(m_strUserID);

    auto uinfo = InteractiveMsg.mutable_rspvalue()->mutable_loginrsp_usr_value()->mutable_relationinfo();
    SerializeRelationList(m_reInfoList, uinfo);

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

void InteractiveProtoHandler::QueryDevInfoReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);

    m_strDevID = InteractiveMsg.reqvalue().querydevinforeq_usr_value().strdevid();
    m_strValue = InteractiveMsg.reqvalue().querydevinforeq_usr_value().strvalue();

}

void InteractiveProtoHandler::QueryDevInfoReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryDevInfoReq_USR_T);

    InteractiveMsg.mutable_reqvalue()->mutable_querydevinforeq_usr_value()->set_strdevid(m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_querydevinforeq_usr_value()->set_strvalue(m_strValue);

}

void InteractiveProtoHandler::QueryDevInfoRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);

    m_devInfo.m_strCreatedate = InteractiveMsg.rspvalue().querydevinforsp_usr_value().devinfo().strcreatedate();
    m_devInfo.m_strDevID = InteractiveMsg.rspvalue().querydevinforsp_usr_value().devinfo().strdevid();
    m_devInfo.m_strDevName = InteractiveMsg.rspvalue().querydevinforsp_usr_value().devinfo().strdevname();
    m_devInfo.m_strDevPassword = InteractiveMsg.rspvalue().querydevinforsp_usr_value().devinfo().strdevpassword();
    m_devInfo.m_strExtend = InteractiveMsg.rspvalue().querydevinforsp_usr_value().devinfo().strextend();
    m_devInfo.m_strInnerinfo = InteractiveMsg.rspvalue().querydevinforsp_usr_value().devinfo().strinnerinfo();
    m_devInfo.m_uiStatus = InteractiveMsg.rspvalue().querydevinforsp_usr_value().devinfo().uistatus();
    m_devInfo.m_uiTypeInfo = InteractiveMsg.rspvalue().querydevinforsp_usr_value().devinfo().uitypeinfo();

    m_strValue = InteractiveMsg.rspvalue().querydevinforsp_usr_value().strvalue();

}

void InteractiveProtoHandler::QueryDevInfoRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryDevInfoRsp_USR_T);

    InteractiveMsg.mutable_rspvalue()->mutable_querydevinforsp_usr_value()->mutable_devinfo()->set_strcreatedate(m_devInfo.m_strCreatedate);
    InteractiveMsg.mutable_rspvalue()->mutable_querydevinforsp_usr_value()->mutable_devinfo()->set_strdevid(m_devInfo.m_strDevID);
    InteractiveMsg.mutable_rspvalue()->mutable_querydevinforsp_usr_value()->mutable_devinfo()->set_strdevname(m_devInfo.m_strDevName);
    InteractiveMsg.mutable_rspvalue()->mutable_querydevinforsp_usr_value()->mutable_devinfo()->set_strdevpassword(m_devInfo.m_strDevPassword);
    InteractiveMsg.mutable_rspvalue()->mutable_querydevinforsp_usr_value()->mutable_devinfo()->set_strextend(m_devInfo.m_strExtend);
    InteractiveMsg.mutable_rspvalue()->mutable_querydevinforsp_usr_value()->mutable_devinfo()->set_strinnerinfo(m_devInfo.m_strInnerinfo);
    InteractiveMsg.mutable_rspvalue()->mutable_querydevinforsp_usr_value()->mutable_devinfo()->set_uistatus(m_devInfo.m_uiStatus);
    InteractiveMsg.mutable_rspvalue()->mutable_querydevinforsp_usr_value()->mutable_devinfo()->set_uitypeinfo(m_devInfo.m_uiTypeInfo);

    InteractiveMsg.mutable_rspvalue()->mutable_querydevinforsp_usr_value()->set_strvalue(m_strValue);


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
    UnSerializeRelationList(m_allRelationInfoList, InteractiveMsg.rspvalue().querydevrsp_usr_value().allrelationinfo());
    
}

void InteractiveProtoHandler::QueryDevRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryDevRsp_USR_T);

    auto uinfo = InteractiveMsg.mutable_rspvalue()->mutable_querydevrsp_usr_value()->mutable_allrelationinfo();
    SerializeRelationList(m_allRelationInfoList, uinfo);

}

void InteractiveProtoHandler::QueryUserReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strDevID = InteractiveMsg.reqvalue().queryuserreq_usr_value().strdevid();
    m_uiBeginIndex = InteractiveMsg.reqvalue().queryuserreq_usr_value().uibeginindex();
    m_strValue = InteractiveMsg.reqvalue().queryuserreq_usr_value().strvalue();
}

void InteractiveProtoHandler::QueryUserReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryUserReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_queryuserreq_usr_value()->set_strdevid(m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_queryuserreq_usr_value()->set_uibeginindex(m_uiBeginIndex);
    InteractiveMsg.mutable_reqvalue()->mutable_queryuserreq_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::QueryUserRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    UnSerializeRelationList(m_allRelationInfoList, InteractiveMsg.rspvalue().queryuserrsp_usr_value().allrelationinfo());
}

void InteractiveProtoHandler::QueryUserRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryUserRsp_USR_T);

    auto uinfo = InteractiveMsg.mutable_rspvalue()->mutable_queryuserrsp_usr_value()->mutable_allrelationinfo();
    SerializeRelationList(m_allRelationInfoList, uinfo);
}

void InteractiveProtoHandler::SharingDevReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    
    m_relationInfo.m_strBeginDate = InteractiveMsg.reqvalue().sharingdevreq_usr_value().relationinfo().strbegindate();
    m_relationInfo.m_strDevID = InteractiveMsg.reqvalue().sharingdevreq_usr_value().relationinfo().strdevid();
    m_relationInfo.m_strEndDate = InteractiveMsg.reqvalue().sharingdevreq_usr_value().relationinfo().strenddate();
    m_relationInfo.m_strUserID = InteractiveMsg.reqvalue().sharingdevreq_usr_value().relationinfo().struserid();
    m_relationInfo.m_strValue = InteractiveMsg.reqvalue().sharingdevreq_usr_value().relationinfo().strvalue();
    m_relationInfo.m_uiRelation = InteractiveMsg.reqvalue().sharingdevreq_usr_value().relationinfo().uirelation();

    m_strValue = InteractiveMsg.reqvalue().sharingdevreq_usr_value().strvalue();


}

void InteractiveProtoHandler::SharingDevReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);

    InteractiveMsg.set_type(Interactive::Message::MsgType::SharingDevReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_relationinfo()->set_strbegindate(m_relationInfo.m_strBeginDate);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_relationinfo()->set_strdevid(m_relationInfo.m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_relationinfo()->set_strenddate(m_relationInfo.m_strEndDate);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_relationinfo()->set_struserid(m_relationInfo.m_strUserID);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_relationinfo()->set_strvalue(m_relationInfo.m_strValue);
    InteractiveMsg.mutable_reqvalue()->mutable_sharingdevreq_usr_value()->mutable_relationinfo()->set_uirelation(m_relationInfo.m_uiRelation);
    
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
    
    m_relationInfo.m_strBeginDate = InteractiveMsg.reqvalue().cancelshareddevreq_usr_value().relationinfo().strbegindate();
    m_relationInfo.m_strDevID = InteractiveMsg.reqvalue().cancelshareddevreq_usr_value().relationinfo().strdevid();
    m_relationInfo.m_strEndDate = InteractiveMsg.reqvalue().cancelshareddevreq_usr_value().relationinfo().strenddate();
    m_relationInfo.m_strUserID = InteractiveMsg.reqvalue().cancelshareddevreq_usr_value().relationinfo().struserid();
    m_relationInfo.m_strValue = InteractiveMsg.reqvalue().cancelshareddevreq_usr_value().relationinfo().strvalue();
    m_relationInfo.m_uiRelation = InteractiveMsg.reqvalue().cancelshareddevreq_usr_value().relationinfo().uirelation();

    m_strValue = InteractiveMsg.reqvalue().cancelshareddevreq_usr_value().strvalue();
}

void InteractiveProtoHandler::CancelSharedDevReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);

    InteractiveMsg.set_type(Interactive::Message::MsgType::CancelSharedDevReq_USR_T);
    
    InteractiveMsg.mutable_reqvalue()->mutable_cancelshareddevreq_usr_value()->mutable_relationinfo()->set_strbegindate(m_relationInfo.m_strBeginDate);
    InteractiveMsg.mutable_reqvalue()->mutable_cancelshareddevreq_usr_value()->mutable_relationinfo()->set_strdevid(m_relationInfo.m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_cancelshareddevreq_usr_value()->mutable_relationinfo()->set_strenddate(m_relationInfo.m_strEndDate);
    InteractiveMsg.mutable_reqvalue()->mutable_cancelshareddevreq_usr_value()->mutable_relationinfo()->set_struserid(m_relationInfo.m_strUserID);
    InteractiveMsg.mutable_reqvalue()->mutable_cancelshareddevreq_usr_value()->mutable_relationinfo()->set_strvalue(m_relationInfo.m_strValue);
    InteractiveMsg.mutable_reqvalue()->mutable_cancelshareddevreq_usr_value()->mutable_relationinfo()->set_uirelation(m_relationInfo.m_uiRelation);

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
    m_uiBeginIndex = InteractiveMsg.reqvalue().queryfriendsreq_usr_value().uibeginindex();
    m_strValue = InteractiveMsg.reqvalue().queryfriendsreq_usr_value().strvalue();

}

void InteractiveProtoHandler::QueryFriendsReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryFriendsReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfriendsreq_usr_value()->set_struserid(m_strUserID);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfriendsreq_usr_value()->set_uibeginindex(m_uiBeginIndex);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfriendsreq_usr_value()->set_strvalue(m_strValue);

}

void InteractiveProtoHandler::QueryFriendsRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);

    m_allFriendUserIDList.clear();
    int iCountTmp = InteractiveMsg.rspvalue().queryfriendsrsp_usr_value().strallfrienduserid_size();
    for (int i = 0; i < iCountTmp; ++i)
    {                
        m_allFriendUserIDList.push_back(InteractiveMsg.rspvalue().queryfriendsrsp_usr_value().strallfrienduserid(i));
    }

}

void InteractiveProtoHandler::QueryFriendsRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryFriendsRsp_USR_T);

    for (unsigned int i = 0; i < m_allFriendUserIDList.size(); ++i)
    {
        InteractiveMsg.mutable_rspvalue()->mutable_queryfriendsrsp_usr_value()->add_strallfrienduserid();
    }
    auto itBegin = m_allFriendUserIDList.begin();
    auto itEnd = m_allFriendUserIDList.end();
    int iUser = 0;
    while (itBegin != itEnd)
    {
        InteractiveMsg.mutable_rspvalue()->mutable_queryfriendsrsp_usr_value()->set_strallfrienduserid(iUser, *itBegin);
        
        ++iUser;
        ++itBegin;
    }
}

void InteractiveProtoHandler::DeleteFileReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().deletefilereq_usr_value().struserid();

    m_strFileIDList.clear();
    int iCountTmp = InteractiveMsg.reqvalue().deletefilereq_usr_value().strfileid_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        m_strFileIDList.push_back(InteractiveMsg.reqvalue().deletefilereq_usr_value().strfileid(i));
    }
}

void InteractiveProtoHandler::DeleteFileReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::DeleteFileReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_deletefilereq_usr_value()->set_struserid(m_strUserID);

    for (unsigned int i = 0; i < m_strFileIDList.size(); ++i)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_deletefilereq_usr_value()->add_strfileid();
    }
    auto itBegin = m_strFileIDList.begin();
    auto itEnd = m_strFileIDList.end();
    int iUser = 0;
    while (itBegin != itEnd)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_deletefilereq_usr_value()->set_strfileid(iUser, *itBegin);

        ++iUser;
        ++itBegin;
    }
}

void InteractiveProtoHandler::DeleteFileRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().deletefilersp_usr_value().strvalue();

    m_strFileIDFailedList.clear();
    int iCountTmp = InteractiveMsg.rspvalue().deletefilersp_usr_value().strfileidfailed_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        m_strFileIDFailedList.push_back(InteractiveMsg.rspvalue().deletefilersp_usr_value().strfileidfailed(i));
    }
}

void InteractiveProtoHandler::DeleteFileRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::DeleteFileRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_deletefilersp_usr_value()->set_strvalue(m_strValue);

    for (unsigned int i = 0; i < m_strFileIDFailedList.size(); ++i)
    {
        InteractiveMsg.mutable_rspvalue()->mutable_deletefilersp_usr_value()->add_strfileidfailed();
    }
    auto itBegin = m_strFileIDFailedList.begin();
    auto itEnd = m_strFileIDFailedList.end();
    int iUser = 0;
    while (itBegin != itEnd)
    {
        InteractiveMsg.mutable_rspvalue()->mutable_deletefilersp_usr_value()->set_strfileidfailed(iUser, *itBegin);

        ++iUser;
        ++itBegin;
    }
}

void InteractiveProtoHandler::DownloadFileReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().downloadfilereq_usr_value().struserid();

    m_strFileIDList.clear();
    int iCountTmp = InteractiveMsg.reqvalue().downloadfilereq_usr_value().strfileid_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        m_strFileIDList.push_back(InteractiveMsg.reqvalue().downloadfilereq_usr_value().strfileid(i));
    }
}

void InteractiveProtoHandler::DownloadFileReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::DownloadFileReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_downloadfilereq_usr_value()->set_struserid(m_strUserID);

    for (unsigned int i = 0; i < m_strFileIDList.size(); ++i)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_downloadfilereq_usr_value()->add_strfileid();
    }
    auto itBegin = m_strFileIDList.begin();
    auto itEnd = m_strFileIDList.end();
    int iUser = 0;
    while (itBegin != itEnd)
    {
        InteractiveMsg.mutable_reqvalue()->mutable_downloadfilereq_usr_value()->set_strfileid(iUser, *itBegin);

        ++iUser;
        ++itBegin;
    }
}

void InteractiveProtoHandler::DownloadFileRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().downloadfilersp_usr_value().strvalue();

    UnSerializeFileUrlList(m_fileUrlList, InteractiveMsg.rspvalue().downloadfilersp_usr_value().fileurl());
}

void InteractiveProtoHandler::DownloadFileRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::DownloadFileRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_downloadfilersp_usr_value()->set_strvalue(m_strValue);

    auto uurl = InteractiveMsg.mutable_rspvalue()->mutable_downloadfilersp_usr_value()->mutable_fileurl();
    SerializeFileUrlList(m_fileUrlList, uurl);
}

void InteractiveProtoHandler::QueryFileReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().queryfilereq_usr_value().struserid();
    m_strDevID = InteractiveMsg.reqvalue().queryfilereq_usr_value().strdevid();
    m_uiBeginIndex = InteractiveMsg.reqvalue().queryfilereq_usr_value().uibeginindex();
    m_strValue = InteractiveMsg.reqvalue().queryfilereq_usr_value().strvalue();
}

void InteractiveProtoHandler::QueryFileReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryFileReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfilereq_usr_value()->set_struserid(m_strUserID);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfilereq_usr_value()->set_strdevid(m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfilereq_usr_value()->set_uibeginindex(m_uiBeginIndex);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfilereq_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::QueryFileRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().queryfilersp_usr_value().strvalue();

    UnSerializeFileList(m_fileList, InteractiveMsg.rspvalue().queryfilersp_usr_value().fileinfo());
}

void InteractiveProtoHandler::QueryFileRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryFileRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_queryfilersp_usr_value()->set_strvalue(m_strValue);

    auto uinfo = InteractiveMsg.mutable_rspvalue()->mutable_queryfilersp_usr_value()->mutable_fileinfo();
    SerializeFileList(m_fileList, uinfo);
}

void InteractiveProtoHandler::AddFileReq_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strDevID = InteractiveMsg.reqvalue().addfilereq_dev_value().strdevid();
    m_strValue = InteractiveMsg.reqvalue().addfilereq_dev_value().strvalue();

    UnSerializeFileList(m_fileList, InteractiveMsg.reqvalue().addfilereq_dev_value().fileinfo());
}

void InteractiveProtoHandler::AddFileReq_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::AddFileReq_DEV_T);
    InteractiveMsg.mutable_reqvalue()->mutable_addfilereq_dev_value()->set_strdevid(m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_addfilereq_dev_value()->set_strvalue(m_strValue);

    auto uinfo = InteractiveMsg.mutable_reqvalue()->mutable_addfilereq_dev_value()->mutable_fileinfo();
    SerializeFileList(m_fileList, uinfo);
}

void InteractiveProtoHandler::AddFileRsp_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().addfilersp_dev_value().strvalue();

    m_strFileIDFailedList.clear();
    int iCountTmp = InteractiveMsg.rspvalue().addfilersp_dev_value().strfileidfailed_size();
    for (int i = 0; i < iCountTmp; ++i)
    {
        m_strFileIDFailedList.push_back(InteractiveMsg.rspvalue().addfilersp_dev_value().strfileidfailed(i));
    }
}

void InteractiveProtoHandler::AddFileRsp_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::AddFileRsp_DEV_T);
    InteractiveMsg.mutable_rspvalue()->mutable_addfilersp_dev_value()->set_strvalue(m_strValue);

    for (unsigned int i = 0; i < m_strFileIDFailedList.size(); ++i)
    {
        InteractiveMsg.mutable_rspvalue()->mutable_addfilersp_dev_value()->add_strfileidfailed();
    }
    auto itBegin = m_strFileIDFailedList.begin();
    auto itEnd = m_strFileIDFailedList.end();
    int iUser = 0;
    while (itBegin != itEnd)
    {
        InteractiveMsg.mutable_rspvalue()->mutable_addfilersp_dev_value()->set_strfileidfailed(iUser, *itBegin);

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
        
        ++iUser;
        ++itBegin;
    }
}


void InteractiveProtoHandler::LoginReq_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.reqvalue().loginreq_dev_value().strvalue();
    m_strDevID = InteractiveMsg.reqvalue().loginreq_dev_value().strdevid();
    m_strPassword = InteractiveMsg.reqvalue().loginreq_dev_value().strpassword();
}

void InteractiveProtoHandler::LoginReq_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::LoginReq_DEV_T);
    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_dev_value()->set_strvalue(m_strValue);
    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_dev_value()->set_strdevid(m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_dev_value()->set_strpassword(m_strPassword);
}

void InteractiveProtoHandler::LoginRsp_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().loginrsp_dev_value().strvalue();
}

void InteractiveProtoHandler::LoginRsp_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::LoginRsp_DEV_T);
    InteractiveMsg.mutable_rspvalue()->mutable_loginrsp_dev_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::P2pInfoReq_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strDevID = InteractiveMsg.reqvalue().p2pinforeq_dev_value().strdevid();
    m_strDevIpAddress = InteractiveMsg.reqvalue().p2pinforeq_dev_value().strdevipaddress();
}

void InteractiveProtoHandler::P2pInfoReq_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::P2pInfoReq_DEV_T);
    InteractiveMsg.mutable_reqvalue()->mutable_p2pinforeq_dev_value()->set_strdevid(m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_p2pinforeq_dev_value()->set_strdevipaddress(m_strDevIpAddress);
}

void InteractiveProtoHandler::P2pInfoRsp_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strP2pID = InteractiveMsg.rspvalue().p2pinforsp_dev_value().strp2pid();
    m_strP2pServer = InteractiveMsg.rspvalue().p2pinforsp_dev_value().strp2pserver();
    m_uiLease = InteractiveMsg.rspvalue().p2pinforsp_dev_value().uilease();
}

void InteractiveProtoHandler::P2pInfoRsp_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::P2pInfoRsp_DEV_T);
    InteractiveMsg.mutable_rspvalue()->mutable_p2pinforsp_dev_value()->set_strp2pid(m_strP2pID);
    InteractiveMsg.mutable_rspvalue()->mutable_p2pinforsp_dev_value()->set_strp2pserver(m_strP2pServer);
    InteractiveMsg.mutable_rspvalue()->mutable_p2pinforsp_dev_value()->set_uilease(m_uiLease);
}

void InteractiveProtoHandler::ShakehandReq_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strDevID = InteractiveMsg.reqvalue().shakehandreq_dev_value().strdevid();
    m_strValue = InteractiveMsg.reqvalue().shakehandreq_dev_value().strvalue();
}

void InteractiveProtoHandler::ShakehandReq_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ShakehandReq_DEV_T);
    InteractiveMsg.mutable_reqvalue()->mutable_shakehandreq_dev_value()->set_strdevid(m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_shakehandreq_dev_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::ShakehandRsp_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().shakehandrsp_dev_value().strvalue();
}

void InteractiveProtoHandler::ShakehandRsp_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ShakehandRsp_DEV_T);
    InteractiveMsg.mutable_rspvalue()->mutable_shakehandrsp_dev_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::LogoutReq_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strDevID = InteractiveMsg.reqvalue().logoutreq_dev_value().strdevid();
    m_strValue = InteractiveMsg.reqvalue().logoutreq_dev_value().strvalue();
}

void InteractiveProtoHandler::LogoutReq_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::LogoutReq_DEV_T);
    InteractiveMsg.mutable_reqvalue()->mutable_logoutreq_dev_value()->set_strdevid(m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_logoutreq_dev_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::LogoutRsp_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().logoutrsp_dev_value().strvalue();
}

void InteractiveProtoHandler::LogoutRsp_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::LogoutRsp_DEV_T);
    InteractiveMsg.mutable_rspvalue()->mutable_logoutrsp_dev_value()->set_strvalue(m_strValue);
}
