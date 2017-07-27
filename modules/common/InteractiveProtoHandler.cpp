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
        devInfo.m_strP2pID = srcDevInfoList.Get(i).strp2pid();
        devInfo.m_strDomainName = srcDevInfoList.Get(i).strdomainname();
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
        pDstDevInfoList->Mutable(i)->set_strp2pid(itBegin->m_strP2pID);
        pDstDevInfoList->Mutable(i)->set_strdomainname(itBegin->m_strDomainName);
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
        fileInfo.m_ulFileSize = srcFileInfo.uifilesize();
        fileInfo.m_uiBusinessType = srcFileInfo.uibusinesstype();
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
        pDstFileInfo->set_uifilesize(itBegin->m_ulFileSize);
        pDstFileInfo->set_uibusinesstype(itBegin->m_uiBusinessType);
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

void UnSerializeDeviceAccessRecordList(std::list<InteractiveProtoHandler::DeviceAccessRecord> &deviceAccessRecordList,
    const ::google::protobuf::RepeatedPtrField< ::Interactive::Message::DeviceAccessRecord > &srcDeviceAccessRecordList)
{
    deviceAccessRecordList.clear();

    unsigned int iCount = srcDeviceAccessRecordList.size();
    for (unsigned int i = 0; i < iCount; ++i)
    {
        auto srcDeviceAccessRecord = srcDeviceAccessRecordList.Get(i);
        InteractiveProtoHandler::DeviceAccessRecord deviceAccessRecord;
        deviceAccessRecord.m_strAccessID = srcDeviceAccessRecord.straccessid();
        deviceAccessRecord.m_strClusterID = srcDeviceAccessRecord.strclusterid();
        deviceAccessRecord.m_strDeviceID = srcDeviceAccessRecord.strdeviceid();
        deviceAccessRecord.m_strDeviceName = srcDeviceAccessRecord.strdevicename();
        deviceAccessRecord.m_uiDeviceType = srcDeviceAccessRecord.uidevicetype();
        deviceAccessRecord.m_strLoginTime = srcDeviceAccessRecord.strlogintime();
        deviceAccessRecord.m_strLogoutTime = srcDeviceAccessRecord.strlogouttime();
        deviceAccessRecord.m_uiOnlineDuration = srcDeviceAccessRecord.uionlineduration();
        deviceAccessRecord.m_strCreateDate = srcDeviceAccessRecord.strcreatedate();
        deviceAccessRecord.m_uiStatus = srcDeviceAccessRecord.uistatus();

        deviceAccessRecordList.push_back(std::move(deviceAccessRecord));
    }
}

void SerializeDeviceAccessRecordList(const std::list<InteractiveProtoHandler::DeviceAccessRecord> &deviceAccessRecordList,
    ::google::protobuf::RepeatedPtrField< ::Interactive::Message::DeviceAccessRecord >* pDstDeviceAccessRecordList)
{
    unsigned int iCount = deviceAccessRecordList.size();
    for (unsigned int i = 0; i < iCount; ++i)
    {
        pDstDeviceAccessRecordList->Add();
    }

    auto itBegin = deviceAccessRecordList.begin();
    auto itEnd = deviceAccessRecordList.end();
    int i = 0;
    while (itBegin != itEnd)
    {
        auto pDstDeviceAccessRecord = pDstDeviceAccessRecordList->Mutable(i);
        pDstDeviceAccessRecord->set_straccessid(itBegin->m_strAccessID);
        pDstDeviceAccessRecord->set_strclusterid(itBegin->m_strClusterID);
        pDstDeviceAccessRecord->set_strdeviceid(itBegin->m_strDeviceID);
        pDstDeviceAccessRecord->set_strdevicename(itBegin->m_strDeviceName);
        pDstDeviceAccessRecord->set_uidevicetype(itBegin->m_uiDeviceType);
        pDstDeviceAccessRecord->set_strlogintime(itBegin->m_strLoginTime);
        pDstDeviceAccessRecord->set_strlogouttime(itBegin->m_strLogoutTime);
        pDstDeviceAccessRecord->set_uionlineduration(itBegin->m_uiOnlineDuration);
        pDstDeviceAccessRecord->set_strcreatedate(itBegin->m_strCreateDate);
        pDstDeviceAccessRecord->set_uistatus(itBegin->m_uiStatus);

        ++i;
        ++itBegin;
    }
}

void UnSerializeUserAccessRecordList(std::list<InteractiveProtoHandler::UserAccessRecord> &userAccessRecordList,
    const ::google::protobuf::RepeatedPtrField< ::Interactive::Message::UserAccessRecord > &srcUserAccessRecordList)
{
    userAccessRecordList.clear();

    unsigned int iCount = srcUserAccessRecordList.size();
    for (unsigned int i = 0; i < iCount; ++i)
    {
        auto srcUserAccessRecord = srcUserAccessRecordList.Get(i);
        InteractiveProtoHandler::UserAccessRecord userAccessRecord;
        userAccessRecord.m_strAccessID = srcUserAccessRecord.straccessid();
        userAccessRecord.m_strClusterID = srcUserAccessRecord.strclusterid();
        userAccessRecord.m_strUserID = srcUserAccessRecord.struserid();
        userAccessRecord.m_strUserName = srcUserAccessRecord.strusername();
        userAccessRecord.m_strUserAliasname = srcUserAccessRecord.struseraliasname();
        userAccessRecord.m_uiClientType = srcUserAccessRecord.uiclienttype();
        userAccessRecord.m_strLoginTime = srcUserAccessRecord.strlogintime();
        userAccessRecord.m_strLogoutTime = srcUserAccessRecord.strlogouttime();
        userAccessRecord.m_uiOnlineDuration = srcUserAccessRecord.uionlineduration();
        userAccessRecord.m_strCreateDate = srcUserAccessRecord.strcreatedate();
        userAccessRecord.m_uiStatus = srcUserAccessRecord.uistatus();

        userAccessRecordList.push_back(std::move(userAccessRecord));
    }
}

void SerializeUserAccessRecordList(const std::list<InteractiveProtoHandler::UserAccessRecord> &userAccessRecordList,
    ::google::protobuf::RepeatedPtrField< ::Interactive::Message::UserAccessRecord >* pDstUserAccessRecordList)
{
    unsigned int iCount = userAccessRecordList.size();
    for (unsigned int i = 0; i < iCount; ++i)
    {
        pDstUserAccessRecordList->Add();
    }

    auto itBegin = userAccessRecordList.begin();
    auto itEnd = userAccessRecordList.end();
    int i = 0;
    while (itBegin != itEnd)
    {
        auto pDstUserAccessRecord = pDstUserAccessRecordList->Mutable(i);
        pDstUserAccessRecord->set_straccessid(itBegin->m_strAccessID);
        pDstUserAccessRecord->set_strclusterid(itBegin->m_strClusterID);
        pDstUserAccessRecord->set_struserid(itBegin->m_strUserID);
        pDstUserAccessRecord->set_strusername(itBegin->m_strUserName);
        pDstUserAccessRecord->set_struseraliasname(itBegin->m_strUserAliasname);
        pDstUserAccessRecord->set_uiclienttype(itBegin->m_uiClientType);
        pDstUserAccessRecord->set_strlogintime(itBegin->m_strLoginTime);
        pDstUserAccessRecord->set_strlogouttime(itBegin->m_strLogoutTime);
        pDstUserAccessRecord->set_uionlineduration(itBegin->m_uiOnlineDuration);
        pDstUserAccessRecord->set_strcreatedate(itBegin->m_strCreateDate);
        pDstUserAccessRecord->set_uistatus(itBegin->m_uiStatus);

        ++i;
        ++itBegin;
    }
}

void UnSerializeConfigurationList(std::list<InteractiveProtoHandler::Configuration> &configurationList,
    const ::google::protobuf::RepeatedPtrField< ::Interactive::Message::Configuration > &srcConfigurationList)
{
    configurationList.clear();

    unsigned int iCount = srcConfigurationList.size();
    for (unsigned int i = 0; i < iCount; ++i)
    {
        auto srcConfiguration = srcConfigurationList.Get(i);
        InteractiveProtoHandler::Configuration configuration;
        configuration.m_strCategory = srcConfiguration.strcategory();
        configuration.m_strSubCategory = srcConfiguration.strsubcategory();
        configuration.m_strLatestVersion = srcConfiguration.strlatestversion();
        configuration.m_strDescription = srcConfiguration.strdescription();
        configuration.m_strForceVersion = srcConfiguration.strforceversion();
        configuration.m_strServerAddress = srcConfiguration.strserveraddress();
        configuration.m_strFileName = srcConfiguration.strfilename();
        configuration.m_strFileID = srcConfiguration.strfileid();
        configuration.m_uiFileSize = srcConfiguration.uifilesize();
        configuration.m_strFilePath = srcConfiguration.strfilepath();
        configuration.m_uiLeaseDuration = srcConfiguration.uileaseduration();
        configuration.m_strUpdateDate = srcConfiguration.strupdatedate();
        configuration.m_uiStatus = srcConfiguration.uistatus();
        configuration.m_strExtend = srcConfiguration.strextend();

        configurationList.push_back(std::move(configuration));
    }
}

void SerializeConfigurationList(const std::list<InteractiveProtoHandler::Configuration> &configurationList,
    ::google::protobuf::RepeatedPtrField< ::Interactive::Message::Configuration >* pDstConfigurationList)
{
    unsigned int iCount = configurationList.size();
    for (unsigned int i = 0; i < iCount; ++i)
    {
        pDstConfigurationList->Add();
    }

    auto itBegin = configurationList.begin();
    auto itEnd = configurationList.end();
    int i = 0;
    while (itBegin != itEnd)
    {
        auto pDstConfiguration = pDstConfigurationList->Mutable(i);
        pDstConfiguration->set_strcategory(itBegin->m_strCategory);
        pDstConfiguration->set_strsubcategory(itBegin->m_strSubCategory);
        pDstConfiguration->set_strlatestversion(itBegin->m_strLatestVersion);
        pDstConfiguration->set_strdescription(itBegin->m_strDescription);
        pDstConfiguration->set_strforceversion(itBegin->m_strForceVersion);
        pDstConfiguration->set_strserveraddress(itBegin->m_strServerAddress);
        pDstConfiguration->set_strfilename(itBegin->m_strFileName);
        pDstConfiguration->set_strfileid(itBegin->m_strFileID);
        pDstConfiguration->set_uifilesize(itBegin->m_uiFileSize);
        pDstConfiguration->set_strfilepath(itBegin->m_strFilePath);
        pDstConfiguration->set_uileaseduration(itBegin->m_uiLeaseDuration);
        pDstConfiguration->set_strupdatedate(itBegin->m_strUpdateDate);
        pDstConfiguration->set_uistatus(itBegin->m_uiStatus);
        pDstConfiguration->set_strextend(itBegin->m_strExtend);

        ++i;
        ++itBegin;
    }
}

void UnSerializeDeviceEventList(std::list<InteractiveProtoHandler::DeviceEvent> &deviceEventList,
    const ::google::protobuf::RepeatedPtrField< ::Interactive::Message::DeviceEvent > &srcDeviceEventList)
{
    deviceEventList.clear();

    unsigned int iCount = srcDeviceEventList.size();
    for (unsigned int i = 0; i < iCount; ++i)
    {
        auto srcDeviceEvent = srcDeviceEventList.Get(i);
        InteractiveProtoHandler::DeviceEvent deviceEvent;
        deviceEvent.m_strDeviceID = srcDeviceEvent.strdeviceid();
        deviceEvent.m_uiDeviceType = srcDeviceEvent.uidevicetype();
        deviceEvent.m_strEventID = srcDeviceEvent.streventid();
        deviceEvent.m_uiEventType = srcDeviceEvent.uieventtype();
        deviceEvent.m_uiEventState = srcDeviceEvent.uieventstate();
        deviceEvent.m_strFileUrl = srcDeviceEvent.strfileurl();
        deviceEvent.m_strEventTime = srcDeviceEvent.streventtime();
        deviceEvent.m_uiReadState = srcDeviceEvent.uireadstate();

        deviceEventList.push_back(std::move(deviceEvent));
    }
}

void SerializeDeviceEventList(const std::list<InteractiveProtoHandler::DeviceEvent> &deviceEventList,
    ::google::protobuf::RepeatedPtrField< ::Interactive::Message::DeviceEvent >* pDstDeviceEventList)
{
    unsigned int iCount = deviceEventList.size();
    for (unsigned int i = 0; i < iCount; ++i)
    {
        pDstDeviceEventList->Add();
    }

    auto itBegin = deviceEventList.begin();
    auto itEnd = deviceEventList.end();
    int i = 0;
    while (itBegin != itEnd)
    {
        auto pDstDeviceEvent = pDstDeviceEventList->Mutable(i);
        pDstDeviceEvent->set_strdeviceid(itBegin->m_strDeviceID);
        pDstDeviceEvent->set_uidevicetype(itBegin->m_uiDeviceType);
        pDstDeviceEvent->set_streventid(itBegin->m_strEventID);
        pDstDeviceEvent->set_uieventtype(itBegin->m_uiEventType);
        pDstDeviceEvent->set_uieventstate(itBegin->m_uiEventState);
        pDstDeviceEvent->set_strfileurl(itBegin->m_strFileUrl);
        pDstDeviceEvent->set_streventtime(itBegin->m_strEventTime);
        pDstDeviceEvent->set_uireadstate(itBegin->m_uiReadState);

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

    handler.Szr = boost::bind(&InteractiveProtoHandler::P2pInfoReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::P2pInfoReq_USR_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::P2pInfoReq_USR_T, handler));
    
    handler.Szr = boost::bind(&InteractiveProtoHandler::P2pInfoRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::P2pInfoRsp_USR_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::P2pInfoRsp_USR_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryAccessDomainNameReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryAccessDomainNameReq_USR_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryAccessDomainNameReq_USR_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryAccessDomainNameRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryAccessDomainNameRsp_USR_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryAccessDomainNameRsp_USR_T, handler));

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

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryAccessDomainNameReq_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryAccessDomainNameReq_DEV_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryAccessDomainNameReq_DEV_T, handler));

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryAccessDomainNameRsp_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryAccessDomainNameRsp_DEV_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryAccessDomainNameRsp_DEV_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::AddFileReq_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::AddFileReq_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::AddFileReq_DEV_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::AddFileRsp_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::AddFileRsp_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::AddFileRsp_DEV_T, handler));

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

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::RetrievePwdReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::RetrievePwdReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::RetrievePwdReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::RetrievePwdRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::RetrievePwdRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::RetrievePwdRsp_USR_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryTimeZoneReq_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryTimeZoneReq_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryTimeZoneReq_DEV_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryTimeZoneRsp_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryTimeZoneRsp_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryTimeZoneRsp_DEV_T, handler));
    
    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryUpgradeSiteReq_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryUpgradeSiteReq_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryUpgradeSiteReq_DEV_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryUpgradeSiteRsp_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryUpgradeSiteRsp_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryUpgradeSiteRsp_DEV_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::GetDeviceAccessRecordReq_INNER_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::GetDeviceAccessRecordReq_INNER_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::GetDeviceAccessRecordReq_INNER_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::GetDeviceAccessRecordRsp_INNER_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::GetDeviceAccessRecordRsp_INNER_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::GetDeviceAccessRecordRsp_INNER_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::GetUserAccessRecordReq_INNER_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::GetUserAccessRecordReq_INNER_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::GetUserAccessRecordReq_INNER_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::GetUserAccessRecordRsp_INNER_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::GetUserAccessRecordRsp_INNER_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::GetUserAccessRecordRsp_INNER_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryAppUpgradeReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryAppUpgradeReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryAppUpgradeReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryAppUpgradeRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryAppUpgradeRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryAppUpgradeRsp_USR_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryFirmwareUpgradeReq_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryFirmwareUpgradeReq_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryFirmwareUpgradeReq_DEV_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryFirmwareUpgradeRsp_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryFirmwareUpgradeRsp_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryFirmwareUpgradeRsp_DEV_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryUploadURLReq_MGR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryUploadURLReq_MGR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryUploadURLReq_MGR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryUploadURLRsp_MGR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryUploadURLRsp_MGR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryUploadURLRsp_MGR_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::AddConfigurationReq_MGR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::AddConfigurationReq_MGR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::AddConfigurationReq_MGR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::AddConfigurationRsp_MGR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::AddConfigurationRsp_MGR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::AddConfigurationRsp_MGR_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::DeleteConfigurationReq_MGR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::DeleteConfigurationReq_MGR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::DeleteConfigurationReq_MGR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::DeleteConfigurationRsp_MGR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::DeleteConfigurationRsp_MGR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::DeleteConfigurationRsp_MGR_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::ModifyConfigurationReq_MGR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::ModifyConfigurationReq_MGR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::ModifyConfigurationReq_MGR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::ModifyConfigurationRsp_MGR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::ModifyConfigurationRsp_MGR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::ModifyConfigurationRsp_MGR_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryAllConfigurationReq_MGR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryAllConfigurationReq_MGR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryAllConfigurationReq_MGR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryAllConfigurationRsp_MGR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryAllConfigurationRsp_MGR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryAllConfigurationRsp_MGR_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::ModifyDevicePropertyReq_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::ModifyDevicePropertyReq_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::ModifyDevicePropertyReq_DEV_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::ModifyDevicePropertyRsp_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::ModifyDevicePropertyRsp_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::ModifyDevicePropertyRsp_DEV_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryDeviceParameterReq_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryDeviceParameterReq_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryDeviceParameterReq_DEV_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryDeviceParameterRsp_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryDeviceParameterRsp_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryDeviceParameterRsp_DEV_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryIfP2pIDValidReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryIfP2pIDValidReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryIfP2pIDValidReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryIfP2pIDValidRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryIfP2pIDValidRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryIfP2pIDValidRsp_USR_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryPlatformPushStatusReq_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryPlatformPushStatusReq_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryPlatformPushStatusReq_DEV_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryPlatformPushStatusRsp_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryPlatformPushStatusRsp_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryPlatformPushStatusRsp_DEV_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::DeviceEventReportReq_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::DeviceEventReportReq_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::DeviceEventReportReq_DEV_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::DeviceEventReportRsp_DEV_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::DeviceEventReportRsp_DEV_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::DeviceEventReportRsp_DEV_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryAllDeviceEventReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryAllDeviceEventReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryAllDeviceEventReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryAllDeviceEventRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryAllDeviceEventRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryAllDeviceEventRsp_USR_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::DeleteDeviceEventReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::DeleteDeviceEventReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::DeleteDeviceEventReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::DeleteDeviceEventRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::DeleteDeviceEventRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::DeleteDeviceEventRsp_USR_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::ModifyDeviceEventReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::ModifyDeviceEventReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::ModifyDeviceEventReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::ModifyDeviceEventRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::ModifyDeviceEventRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::ModifyDeviceEventRsp_USR_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::AddStorageDetailReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::AddStorageDetailReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::AddStorageDetailReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::AddStorageDetailRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::AddStorageDetailRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::AddStorageDetailRsp_USR_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::DeleteStorageDetailReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::DeleteStorageDetailReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::DeleteStorageDetailReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::DeleteStorageDetailRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::DeleteStorageDetailRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::DeleteStorageDetailRsp_USR_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::ModifyStorageDetailReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::ModifyStorageDetailReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::ModifyStorageDetailReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::ModifyStorageDetailRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::ModifyStorageDetailRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::ModifyStorageDetailRsp_USR_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryStorageDetailReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryStorageDetailReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryStorageDetailReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryStorageDetailRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryStorageDetailRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryStorageDetailRsp_USR_T, handler));

    //////////////////////////////////////////////////

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryRegionStorageInfoReq_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryRegionStorageInfoReq_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryRegionStorageInfoReq_USR_T, handler));

    //

    handler.Szr = boost::bind(&InteractiveProtoHandler::QueryRegionStorageInfoRsp_USR_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&InteractiveProtoHandler::QueryRegionStorageInfoRsp_USR_UnSerializer, this, _1, _2);

    m_ReqAndRspHandlerMap.insert(std::make_pair(Interactive::Message::MsgType::QueryRegionStorageInfoRsp_USR_T, handler));
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

bool InteractiveProtoHandler::P2pInfoReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<P2pInfoReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::P2pInfoReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<P2pInfoReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::P2pInfoRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<P2pInfoRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::P2pInfoRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<P2pInfoRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryAccessDomainNameReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryAccessDomainNameReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryAccessDomainNameReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryAccessDomainNameReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryAccessDomainNameRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryAccessDomainNameRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryAccessDomainNameRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryAccessDomainNameRsp_USR, Req>(InteractiveMsg, rsp);
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

bool InteractiveProtoHandler::QueryAccessDomainNameReq_DEV_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryAccessDomainNameReq_DEV, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryAccessDomainNameReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryAccessDomainNameReq_DEV, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryAccessDomainNameRsp_DEV_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryAccessDomainNameRsp_DEV, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryAccessDomainNameRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryAccessDomainNameRsp_DEV, Req>(InteractiveMsg, rsp);
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

bool InteractiveProtoHandler::RetrievePwdReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<RetrievePwdReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::RetrievePwdReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<RetrievePwdReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::RetrievePwdRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<RetrievePwdRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::RetrievePwdRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<RetrievePwdRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryTimeZoneReq_DEV_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryTimeZoneReq_DEV, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryTimeZoneReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryTimeZoneReq_DEV, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryTimeZoneRsp_DEV_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryTimeZoneRsp_DEV, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryTimeZoneRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryTimeZoneRsp_DEV, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryUpgradeSiteReq_DEV_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryUpgradeSiteReq_DEV, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryUpgradeSiteReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryUpgradeSiteReq_DEV, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryUpgradeSiteRsp_DEV_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryUpgradeSiteRsp_DEV, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryUpgradeSiteRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryUpgradeSiteRsp_DEV, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::GetDeviceAccessRecordReq_INNER_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<GetDeviceAccessRecordReq_INNER, Req>(req, strOutput);
}

bool InteractiveProtoHandler::GetDeviceAccessRecordReq_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<GetDeviceAccessRecordReq_INNER, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::GetDeviceAccessRecordRsp_INNER_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<GetDeviceAccessRecordRsp_INNER, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::GetDeviceAccessRecordRsp_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<GetDeviceAccessRecordRsp_INNER, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::GetUserAccessRecordReq_INNER_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<GetUserAccessRecordReq_INNER, Req>(req, strOutput);
}

bool InteractiveProtoHandler::GetUserAccessRecordReq_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<GetUserAccessRecordReq_INNER, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::GetUserAccessRecordRsp_INNER_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<GetUserAccessRecordRsp_INNER, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::GetUserAccessRecordRsp_INNER_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<GetUserAccessRecordRsp_INNER, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryAppUpgradeReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryAppUpgradeReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryAppUpgradeReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryAppUpgradeReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryAppUpgradeRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryAppUpgradeRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryAppUpgradeRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryAppUpgradeRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryFirmwareUpgradeReq_DEV_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryFirmwareUpgradeReq_DEV, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryFirmwareUpgradeReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryFirmwareUpgradeReq_DEV, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryFirmwareUpgradeRsp_DEV_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryFirmwareUpgradeRsp_DEV, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryFirmwareUpgradeRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryFirmwareUpgradeRsp_DEV, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryUploadURLReq_MGR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryUploadURLReq_MGR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryUploadURLReq_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryUploadURLReq_MGR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryUploadURLRsp_MGR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryUploadURLRsp_MGR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryUploadURLRsp_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryUploadURLRsp_MGR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::AddConfigurationReq_MGR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<AddConfigurationReq_MGR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::AddConfigurationReq_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<AddConfigurationReq_MGR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::AddConfigurationRsp_MGR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<AddConfigurationRsp_MGR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::AddConfigurationRsp_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<AddConfigurationRsp_MGR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::DeleteConfigurationReq_MGR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<DeleteConfigurationReq_MGR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::DeleteConfigurationReq_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<DeleteConfigurationReq_MGR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::DeleteConfigurationRsp_MGR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<DeleteConfigurationRsp_MGR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::DeleteConfigurationRsp_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<DeleteConfigurationRsp_MGR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::ModifyConfigurationReq_MGR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<ModifyConfigurationReq_MGR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::ModifyConfigurationReq_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<ModifyConfigurationReq_MGR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::ModifyConfigurationRsp_MGR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<ModifyConfigurationRsp_MGR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::ModifyConfigurationRsp_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<ModifyConfigurationRsp_MGR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryAllConfigurationReq_MGR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryAllConfigurationReq_MGR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryAllConfigurationReq_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryAllConfigurationReq_MGR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryAllConfigurationRsp_MGR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryAllConfigurationRsp_MGR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryAllConfigurationRsp_MGR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryAllConfigurationRsp_MGR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::ModifyDevicePropertyReq_DEV_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<ModifyDevicePropertyReq_DEV, Req>(req, strOutput);
}

bool InteractiveProtoHandler::ModifyDevicePropertyReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<ModifyDevicePropertyReq_DEV, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::ModifyDevicePropertyRsp_DEV_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<ModifyDevicePropertyRsp_DEV, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::ModifyDevicePropertyRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<ModifyDevicePropertyRsp_DEV, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryDeviceParameterReq_DEV_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryDeviceParameterReq_DEV, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryDeviceParameterReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryDeviceParameterReq_DEV, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryDeviceParameterRsp_DEV_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryDeviceParameterRsp_DEV, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryDeviceParameterRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryDeviceParameterRsp_DEV, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryIfP2pIDValidReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryIfP2pIDValidReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryIfP2pIDValidReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryIfP2pIDValidReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryIfP2pIDValidRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryIfP2pIDValidRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryIfP2pIDValidRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryIfP2pIDValidRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryPlatformPushStatusReq_DEV_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryPlatformPushStatusReq_DEV, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryPlatformPushStatusReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryPlatformPushStatusReq_DEV, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryPlatformPushStatusRsp_DEV_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryPlatformPushStatusRsp_DEV, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryPlatformPushStatusRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryPlatformPushStatusRsp_DEV, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::DeviceEventReportReq_DEV_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<DeviceEventReportReq_DEV, Req>(req, strOutput);
}

bool InteractiveProtoHandler::DeviceEventReportReq_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<DeviceEventReportReq_DEV, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::DeviceEventReportRsp_DEV_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<DeviceEventReportRsp_DEV, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::DeviceEventReportRsp_DEV_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<DeviceEventReportRsp_DEV, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryAllDeviceEventReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryAllDeviceEventReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryAllDeviceEventReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryAllDeviceEventReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryAllDeviceEventRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryAllDeviceEventRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryAllDeviceEventRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryAllDeviceEventRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::DeleteDeviceEventReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<DeleteDeviceEventReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::DeleteDeviceEventReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<DeleteDeviceEventReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::DeleteDeviceEventRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<DeleteDeviceEventRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::DeleteDeviceEventRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<DeleteDeviceEventRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::ModifyDeviceEventReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<ModifyDeviceEventReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::ModifyDeviceEventReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<ModifyDeviceEventReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::ModifyDeviceEventRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<ModifyDeviceEventRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::ModifyDeviceEventRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<ModifyDeviceEventRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::AddStorageDetailReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<AddStorageDetailReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::AddStorageDetailReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<AddStorageDetailReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::AddStorageDetailRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<AddStorageDetailRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::AddStorageDetailRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<AddStorageDetailRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::DeleteStorageDetailReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<DeleteStorageDetailReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::DeleteStorageDetailReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<DeleteStorageDetailReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::DeleteStorageDetailRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<DeleteStorageDetailRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::DeleteStorageDetailRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<DeleteStorageDetailRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::ModifyStorageDetailReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<ModifyStorageDetailReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::ModifyStorageDetailReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<ModifyStorageDetailReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::ModifyStorageDetailRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<ModifyStorageDetailRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::ModifyStorageDetailRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<ModifyStorageDetailRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryStorageDetailReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryStorageDetailReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryStorageDetailReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryStorageDetailReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryStorageDetailRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryStorageDetailRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryStorageDetailRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryStorageDetailRsp_USR, Req>(InteractiveMsg, rsp);
}

bool InteractiveProtoHandler::QueryRegionStorageInfoReq_USR_Serializer(const Req &req, std::string &strOutput)
{
    return SerializerT<QueryRegionStorageInfoReq_USR, Req>(req, strOutput);
}

bool InteractiveProtoHandler::QueryRegionStorageInfoReq_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &req)
{
    return UnSerializerT<QueryRegionStorageInfoReq_USR, Req>(InteractiveMsg, req);
}

bool InteractiveProtoHandler::QueryRegionStorageInfoRsp_USR_Serializer(const Req &rsp, std::string &strOutput)
{
    return SerializerT<QueryRegionStorageInfoRsp_USR, Req>(rsp, strOutput);
}

bool InteractiveProtoHandler::QueryRegionStorageInfoRsp_USR_UnSerializer(const InteractiveMessage &InteractiveMsg, Req &rsp)
{
    return UnSerializerT<QueryRegionStorageInfoRsp_USR, Req>(InteractiveMsg, rsp);
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
    m_userInfo.m_strAliasName = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().straliasname();
    m_userInfo.m_strEmail = InteractiveMsg.reqvalue().registeruserreq_usr_value().userinfo().stremail();
    
    m_strValue = InteractiveMsg.reqvalue().registeruserreq_usr_value().strvalue();
    
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
    uinfo->set_straliasname(m_userInfo.m_strAliasName);
    uinfo->set_stremail(m_userInfo.m_strEmail);
    
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
    m_userInfo.m_strAliasName = InteractiveMsg.reqvalue().unregisteruserreq_usr_value().userinfo().straliasname();
    m_userInfo.m_strEmail = InteractiveMsg.reqvalue().unregisteruserreq_usr_value().userinfo().stremail();

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
    uinfo->set_straliasname(m_userInfo.m_strAliasName);
    uinfo->set_stremail(m_userInfo.m_strEmail);

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
    m_userInfo.m_strAliasName = InteractiveMsg.rspvalue().queryusrinforsp_usr_value().userinfo().straliasname();
    m_userInfo.m_strEmail = InteractiveMsg.rspvalue().queryusrinforsp_usr_value().userinfo().stremail();


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
    InteractiveMsg.mutable_rspvalue()->mutable_queryusrinforsp_usr_value()->mutable_userinfo()->set_straliasname(m_userInfo.m_strAliasName);
    InteractiveMsg.mutable_rspvalue()->mutable_queryusrinforsp_usr_value()->mutable_userinfo()->set_stremail(m_userInfo.m_strEmail);
    
    InteractiveMsg.mutable_rspvalue()->mutable_queryusrinforsp_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::ModifyUserInfoReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strOldPwd = InteractiveMsg.reqvalue().modifyuserinforeq_usr_value().stroldpwd();

    m_userInfo.m_strUserID = InteractiveMsg.reqvalue().modifyuserinforeq_usr_value().userinfo().struserid();
    m_userInfo.m_strUserName = InteractiveMsg.reqvalue().modifyuserinforeq_usr_value().userinfo().strusername();
    m_userInfo.m_strUserPassword = InteractiveMsg.reqvalue().modifyuserinforeq_usr_value().userinfo().struserpassword();
    m_userInfo.m_uiTypeInfo = InteractiveMsg.reqvalue().modifyuserinforeq_usr_value().userinfo().uitypeinfo();
    m_userInfo.m_strCreatedate = InteractiveMsg.reqvalue().modifyuserinforeq_usr_value().userinfo().strcreatedate();
    m_userInfo.m_uiStatus = InteractiveMsg.reqvalue().modifyuserinforeq_usr_value().userinfo().uistatus();
    m_userInfo.m_strExtend = InteractiveMsg.reqvalue().modifyuserinforeq_usr_value().userinfo().strextend();
    m_userInfo.m_strAliasName = InteractiveMsg.reqvalue().modifyuserinforeq_usr_value().userinfo().straliasname();
    m_userInfo.m_strEmail = InteractiveMsg.reqvalue().modifyuserinforeq_usr_value().userinfo().stremail();

}

void InteractiveProtoHandler::ModifyUserInfoReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ModifyUserInfoReq_USR_T);

    InteractiveMsg.mutable_reqvalue()->mutable_modifyuserinforeq_usr_value()->set_stroldpwd(m_strOldPwd);

    auto uinfo = InteractiveMsg.mutable_reqvalue()->mutable_modifyuserinforeq_usr_value()->mutable_userinfo();
    uinfo->set_struserid(m_userInfo.m_strUserID);
    uinfo->set_strusername(m_userInfo.m_strUserName);
    uinfo->set_struserpassword(m_userInfo.m_strUserPassword);
    uinfo->set_uitypeinfo(m_userInfo.m_uiTypeInfo);
    uinfo->set_strcreatedate(m_userInfo.m_strCreatedate);
    uinfo->set_uistatus(m_userInfo.m_uiStatus);
    uinfo->set_strextend(m_userInfo.m_strExtend);
    uinfo->set_straliasname(m_userInfo.m_strAliasName);
    uinfo->set_stremail(m_userInfo.m_strEmail);

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
    m_userInfo.m_strAliasName = InteractiveMsg.reqvalue().loginreq_usr_value().userinfo().straliasname();
    m_userInfo.m_strEmail = InteractiveMsg.reqvalue().loginreq_usr_value().userinfo().stremail();

    m_uiTerminalType = InteractiveMsg.reqvalue().loginreq_usr_value().uiterminaltype();
    m_uiType = InteractiveMsg.reqvalue().loginreq_usr_value().uitype();
    m_strValue = InteractiveMsg.reqvalue().loginreq_usr_value().strvalue();
}

void InteractiveProtoHandler::LoginReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::LoginReq_USR_T);

    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_usr_value()->set_strvalue(m_strValue);
    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_usr_value()->set_uiterminaltype(m_uiTerminalType);
    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_usr_value()->set_uitype(m_uiType);
    auto uinfo = InteractiveMsg.mutable_reqvalue()->mutable_loginreq_usr_value()->mutable_userinfo();

    uinfo->set_struserid(m_userInfo.m_strUserID);
    uinfo->set_strusername(m_userInfo.m_strUserName);
    uinfo->set_struserpassword(m_userInfo.m_strUserPassword);
    uinfo->set_uitypeinfo(m_userInfo.m_uiTypeInfo);
    uinfo->set_strcreatedate(m_userInfo.m_strCreatedate);
    uinfo->set_uistatus(m_userInfo.m_uiStatus);
    uinfo->set_strextend(m_userInfo.m_strExtend);
    uinfo->set_straliasname(m_userInfo.m_strAliasName);
    uinfo->set_stremail(m_userInfo.m_strEmail);

}

void InteractiveProtoHandler::LoginRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.rspvalue().loginrsp_usr_value().struserid();
    UnSerializeRelationList(m_reInfoList, InteractiveMsg.rspvalue().loginrsp_usr_value().relationinfo());
    m_strValue = InteractiveMsg.rspvalue().loginrsp_usr_value().strvalue();

    m_strDevNameList.clear();
    int iCount = InteractiveMsg.rspvalue().loginrsp_usr_value().strdevname_size();
    for (int i = 0; i < iCount; ++i)
    {
        m_strDevNameList.push_back(InteractiveMsg.rspvalue().loginrsp_usr_value().strdevname(i));
    }
}

void InteractiveProtoHandler::LoginRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::LoginRsp_USR_T);

    InteractiveMsg.mutable_rspvalue()->mutable_loginrsp_usr_value()->set_strvalue(m_strValue);
    InteractiveMsg.mutable_rspvalue()->mutable_loginrsp_usr_value()->set_struserid(m_strUserID);

    auto uinfo = InteractiveMsg.mutable_rspvalue()->mutable_loginrsp_usr_value()->mutable_relationinfo();
    SerializeRelationList(m_reInfoList, uinfo);

    int i = 0;
    auto itBegin = m_strDevNameList.begin();
    auto itEnd = m_strDevNameList.end();
    while (itBegin != itEnd)
    {
        InteractiveMsg.mutable_rspvalue()->mutable_loginrsp_usr_value()->add_strdevname();
        InteractiveMsg.mutable_rspvalue()->mutable_loginrsp_usr_value()->set_strdevname(i, *itBegin);

        ++i;
        ++itBegin;
    }
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
    m_userInfo.m_strAliasName = InteractiveMsg.reqvalue().logoutreq_usr_value().userinfo().straliasname();
    m_userInfo.m_strEmail = InteractiveMsg.reqvalue().logoutreq_usr_value().userinfo().stremail();
    
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
    uinfo->set_straliasname(m_userInfo.m_strAliasName);
    uinfo->set_stremail(m_userInfo.m_strEmail);

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
    m_devInfo.m_strP2pID = InteractiveMsg.reqvalue().adddevreq_usr_value().devinfo().strp2pid();
    m_devInfo.m_strDomainName = InteractiveMsg.reqvalue().adddevreq_usr_value().devinfo().strdomainname();
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
    InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->set_strp2pid(m_devInfo.m_strP2pID);
    InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->set_strdomainname(m_devInfo.m_strDomainName);
    InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->set_strcreatedate(m_devInfo.m_strCreatedate);
    InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->set_uistatus(m_devInfo.m_uiStatus);
    InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->set_strextend(m_devInfo.m_strExtend);
    InteractiveMsg.mutable_reqvalue()->mutable_adddevreq_usr_value()->mutable_devinfo()->set_strinnerinfo(m_devInfo.m_strInnerinfo);
}

void InteractiveProtoHandler::AddDevRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strDeviceID = InteractiveMsg.rspvalue().adddevrsp_usr_value().strdeviceid();
    m_strValue = InteractiveMsg.rspvalue().adddevrsp_usr_value().strvalue();
}

void InteractiveProtoHandler::AddDevRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::AddDevRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_adddevrsp_usr_value()->set_strdeviceid(m_strDeviceID);
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
    m_uiDeviceShared = InteractiveMsg.reqvalue().modifydevreq_usr_value().uideviceshared();

    m_devInfo.m_strDevID = InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().strdevid();
    m_devInfo.m_strDevName = InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().strdevname();
    m_devInfo.m_strDevPassword = InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().strdevpassword();
    m_devInfo.m_uiTypeInfo = InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().uitypeinfo();
    m_devInfo.m_strP2pID = InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().strp2pid();
    m_devInfo.m_strDomainName = InteractiveMsg.reqvalue().modifydevreq_usr_value().devinfo().strdomainname();
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
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->set_uideviceshared(m_uiDeviceShared);

    InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->set_strdevid(m_devInfo.m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->set_strdevname(m_devInfo.m_strDevName);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->set_strdevpassword(m_devInfo.m_strDevPassword);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->set_uitypeinfo(m_devInfo.m_uiTypeInfo);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->set_strp2pid(m_devInfo.m_strP2pID);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevreq_usr_value()->mutable_devinfo()->set_strdomainname(m_devInfo.m_strDomainName);
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

    m_strUserID = InteractiveMsg.reqvalue().querydevinforeq_usr_value().struserid();
    m_strDevID = InteractiveMsg.reqvalue().querydevinforeq_usr_value().strdevid();
    m_uiDeviceShared = InteractiveMsg.reqvalue().querydevinforeq_usr_value().uideviceshared();
    m_strValue = InteractiveMsg.reqvalue().querydevinforeq_usr_value().strvalue();
}

void InteractiveProtoHandler::QueryDevInfoReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryDevInfoReq_USR_T);

    InteractiveMsg.mutable_reqvalue()->mutable_querydevinforeq_usr_value()->set_struserid(m_strUserID);
    InteractiveMsg.mutable_reqvalue()->mutable_querydevinforeq_usr_value()->set_strdevid(m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_querydevinforeq_usr_value()->set_uideviceshared(m_uiDeviceShared);
    InteractiveMsg.mutable_reqvalue()->mutable_querydevinforeq_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::QueryDevInfoRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);

    m_devInfo.m_strCreatedate = InteractiveMsg.rspvalue().querydevinforsp_usr_value().devinfo().strcreatedate();
    m_devInfo.m_strDevID = InteractiveMsg.rspvalue().querydevinforsp_usr_value().devinfo().strdevid();
    m_devInfo.m_strDevName = InteractiveMsg.rspvalue().querydevinforsp_usr_value().devinfo().strdevname();
    m_devInfo.m_strDevPassword = InteractiveMsg.rspvalue().querydevinforsp_usr_value().devinfo().strdevpassword();
    m_devInfo.m_strP2pID = InteractiveMsg.rspvalue().querydevinforsp_usr_value().devinfo().strp2pid();
    m_devInfo.m_strDomainName = InteractiveMsg.rspvalue().querydevinforsp_usr_value().devinfo().strdomainname();
    m_devInfo.m_strExtend = InteractiveMsg.rspvalue().querydevinforsp_usr_value().devinfo().strextend();
    m_devInfo.m_strInnerinfo = InteractiveMsg.rspvalue().querydevinforsp_usr_value().devinfo().strinnerinfo();
    m_devInfo.m_uiStatus = InteractiveMsg.rspvalue().querydevinforsp_usr_value().devinfo().uistatus();
    m_devInfo.m_uiTypeInfo = InteractiveMsg.rspvalue().querydevinforsp_usr_value().devinfo().uitypeinfo();

    m_strVersion = InteractiveMsg.rspvalue().querydevinforsp_usr_value().strversion();
    m_strOnlineStatus = InteractiveMsg.rspvalue().querydevinforsp_usr_value().stronlinestatus();
    m_strUpdateDate = InteractiveMsg.rspvalue().querydevinforsp_usr_value().strupdatedate();
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
    InteractiveMsg.mutable_rspvalue()->mutable_querydevinforsp_usr_value()->mutable_devinfo()->set_strp2pid(m_devInfo.m_strP2pID);
    InteractiveMsg.mutable_rspvalue()->mutable_querydevinforsp_usr_value()->mutable_devinfo()->set_strdomainname(m_devInfo.m_strDomainName);
    InteractiveMsg.mutable_rspvalue()->mutable_querydevinforsp_usr_value()->mutable_devinfo()->set_strextend(m_devInfo.m_strExtend);
    InteractiveMsg.mutable_rspvalue()->mutable_querydevinforsp_usr_value()->mutable_devinfo()->set_strinnerinfo(m_devInfo.m_strInnerinfo);
    InteractiveMsg.mutable_rspvalue()->mutable_querydevinforsp_usr_value()->mutable_devinfo()->set_uistatus(m_devInfo.m_uiStatus);
    InteractiveMsg.mutable_rspvalue()->mutable_querydevinforsp_usr_value()->mutable_devinfo()->set_uitypeinfo(m_devInfo.m_uiTypeInfo);

    InteractiveMsg.mutable_rspvalue()->mutable_querydevinforsp_usr_value()->set_strversion(m_strVersion);
    InteractiveMsg.mutable_rspvalue()->mutable_querydevinforsp_usr_value()->set_stronlinestatus(m_strOnlineStatus);
    InteractiveMsg.mutable_rspvalue()->mutable_querydevinforsp_usr_value()->set_strupdatedate(m_strUpdateDate);
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
    
    m_strDevNameList.clear();
    int iCount = InteractiveMsg.rspvalue().querydevrsp_usr_value().strdevname_size();
    for (int i = 0; i < iCount; ++i)
    {
        m_strDevNameList.push_back(InteractiveMsg.rspvalue().querydevrsp_usr_value().strdevname(i));
    }
}

void InteractiveProtoHandler::QueryDevRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryDevRsp_USR_T);

    auto uinfo = InteractiveMsg.mutable_rspvalue()->mutable_querydevrsp_usr_value()->mutable_allrelationinfo();
    SerializeRelationList(m_allRelationInfoList, uinfo);

    int i = 0;
    auto itBegin = m_strDevNameList.begin();
    auto itEnd = m_strDevNameList.end();
    while (itBegin != itEnd)
    {
        InteractiveMsg.mutable_rspvalue()->mutable_querydevrsp_usr_value()->add_strdevname();
        InteractiveMsg.mutable_rspvalue()->mutable_querydevrsp_usr_value()->set_strdevname(i, *itBegin);

        ++i;
        ++itBegin;
    }
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

    m_strUserNameList.clear();
    int iCount = InteractiveMsg.rspvalue().queryuserrsp_usr_value().strusername_size();
    for (int i = 0; i < iCount; ++i)
    {
        m_strUserNameList.push_back(InteractiveMsg.rspvalue().queryuserrsp_usr_value().strusername(i));
    }
}

void InteractiveProtoHandler::QueryUserRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryUserRsp_USR_T);

    auto uinfo = InteractiveMsg.mutable_rspvalue()->mutable_queryuserrsp_usr_value()->mutable_allrelationinfo();
    SerializeRelationList(m_allRelationInfoList, uinfo);

    int i = 0;
    auto itBegin = m_strUserNameList.begin();
    auto itEnd = m_strUserNameList.end();
    while (itBegin != itEnd)
    {
        InteractiveMsg.mutable_rspvalue()->mutable_queryuserrsp_usr_value()->add_strusername();
        InteractiveMsg.mutable_rspvalue()->mutable_queryuserrsp_usr_value()->set_strusername(i, *itBegin);

        ++i;
        ++itBegin;
    }
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
    m_strBeginDate = InteractiveMsg.reqvalue().queryfilereq_usr_value().strbegindate();
    m_strEndDate = InteractiveMsg.reqvalue().queryfilereq_usr_value().strenddate();
    m_uiBusinessType = InteractiveMsg.reqvalue().queryfilereq_usr_value().uibusinesstype();
    m_strValue = InteractiveMsg.reqvalue().queryfilereq_usr_value().strvalue();
}

void InteractiveProtoHandler::QueryFileReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryFileReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfilereq_usr_value()->set_struserid(m_strUserID);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfilereq_usr_value()->set_strdevid(m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfilereq_usr_value()->set_uibeginindex(m_uiBeginIndex);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfilereq_usr_value()->set_strbegindate(m_strBeginDate);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfilereq_usr_value()->set_strenddate(m_strEndDate);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfilereq_usr_value()->set_uibusinesstype(m_uiBusinessType);
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

void InteractiveProtoHandler::P2pInfoReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().p2pinforeq_usr_value().struserid();
    m_strUserIpAddress = InteractiveMsg.reqvalue().p2pinforeq_usr_value().struseripaddress();
    m_strDevID = InteractiveMsg.reqvalue().p2pinforeq_usr_value().strdevid();
    m_uiP2pSupplier = InteractiveMsg.reqvalue().p2pinforeq_usr_value().uip2psupplier();
}

void InteractiveProtoHandler::P2pInfoReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::P2pInfoReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_p2pinforeq_usr_value()->set_struserid(m_strUserID);
    InteractiveMsg.mutable_reqvalue()->mutable_p2pinforeq_usr_value()->set_struseripaddress(m_strUserIpAddress);
    InteractiveMsg.mutable_reqvalue()->mutable_p2pinforeq_usr_value()->set_strdevid(m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_p2pinforeq_usr_value()->set_uip2psupplier(m_uiP2pSupplier);
}

void InteractiveProtoHandler::P2pInfoRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strP2pID = InteractiveMsg.rspvalue().p2pinforsp_usr_value().strp2pid();
    m_strP2pServer = InteractiveMsg.rspvalue().p2pinforsp_usr_value().strp2pserver();
    m_uiLease = InteractiveMsg.rspvalue().p2pinforsp_usr_value().uilease();
    m_strLicenseKey = InteractiveMsg.rspvalue().p2pinforsp_usr_value().strlicensekey();
    m_strPushID = InteractiveMsg.rspvalue().p2pinforsp_usr_value().strpushid();
}

void InteractiveProtoHandler::P2pInfoRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::P2pInfoRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_p2pinforsp_usr_value()->set_strp2pid(m_strP2pID);
    InteractiveMsg.mutable_rspvalue()->mutable_p2pinforsp_usr_value()->set_strp2pserver(m_strP2pServer);
    InteractiveMsg.mutable_rspvalue()->mutable_p2pinforsp_usr_value()->set_uilease(m_uiLease);
    InteractiveMsg.mutable_rspvalue()->mutable_p2pinforsp_usr_value()->set_strlicensekey(m_strLicenseKey);
    InteractiveMsg.mutable_rspvalue()->mutable_p2pinforsp_usr_value()->set_strpushid(m_strPushID);
}

void InteractiveProtoHandler::QueryAccessDomainNameReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserIpAddress = InteractiveMsg.reqvalue().queryaccessdomainnamereq_usr_value().struseripaddress();
    m_strValue = InteractiveMsg.reqvalue().queryaccessdomainnamereq_usr_value().strvalue();
}

void InteractiveProtoHandler::QueryAccessDomainNameReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryAccessDomainNameReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_queryaccessdomainnamereq_usr_value()->set_struseripaddress(m_strUserIpAddress);
    InteractiveMsg.mutable_reqvalue()->mutable_queryaccessdomainnamereq_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::QueryAccessDomainNameRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strDomainName = InteractiveMsg.rspvalue().queryaccessdomainnamersp_usr_value().strdomainname();
    m_uiLease = InteractiveMsg.rspvalue().queryaccessdomainnamersp_usr_value().uilease();
    m_strValue = InteractiveMsg.rspvalue().queryaccessdomainnamersp_usr_value().strvalue();
}

void InteractiveProtoHandler::QueryAccessDomainNameRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryAccessDomainNameRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_queryaccessdomainnamersp_usr_value()->set_strdomainname(m_strDomainName);
    InteractiveMsg.mutable_rspvalue()->mutable_queryaccessdomainnamersp_usr_value()->set_uilease(m_uiLease);
    InteractiveMsg.mutable_rspvalue()->mutable_queryaccessdomainnamersp_usr_value()->set_strvalue(m_strValue);
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
        usr.m_strAliasName = ItUser.straliasname();
        usr.m_strEmail = ItUser.stremail();

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
        uinfo->set_straliasname(itBegin->m_strAliasName);
        uinfo->set_stremail(itBegin->m_strEmail);
        
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
        usr.m_strAliasName = ItUser.straliasname();
        usr.m_strEmail = ItUser.stremail();

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
        uinfo->set_straliasname(itBegin->m_strAliasName);
        uinfo->set_stremail(itBegin->m_strEmail);
        
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
    m_uiDeviceType = InteractiveMsg.reqvalue().loginreq_dev_value().uidevicetype();
    m_uiP2pSupplier = InteractiveMsg.reqvalue().loginreq_dev_value().uip2psupplier();
    m_strP2pID = InteractiveMsg.reqvalue().loginreq_dev_value().strp2pid();
    m_strP2pServr = InteractiveMsg.reqvalue().loginreq_dev_value().strp2pserver();
    m_uiP2pBuildin = InteractiveMsg.reqvalue().loginreq_dev_value().uip2pbuildin();
    m_strUserName = InteractiveMsg.reqvalue().loginreq_dev_value().strusername();
    m_strUserPassword = InteractiveMsg.reqvalue().loginreq_dev_value().struserpassword();
    m_strDistributor = InteractiveMsg.reqvalue().loginreq_dev_value().strdistributor();
    m_strDomainName = InteractiveMsg.reqvalue().loginreq_dev_value().strdomainname();
    m_strOtherProperty = InteractiveMsg.reqvalue().loginreq_dev_value().strotherproperty();
}

void InteractiveProtoHandler::LoginReq_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::LoginReq_DEV_T);
    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_dev_value()->set_strvalue(m_strValue);
    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_dev_value()->set_strdevid(m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_dev_value()->set_strpassword(m_strPassword);
    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_dev_value()->set_uidevicetype(m_uiDeviceType);
    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_dev_value()->set_uip2psupplier(m_uiP2pSupplier);
    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_dev_value()->set_strp2pid(m_strP2pID);
    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_dev_value()->set_strp2pserver(m_strP2pServr);
    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_dev_value()->set_uip2pbuildin(m_uiP2pBuildin);
    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_dev_value()->set_strusername(m_strUserName);
    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_dev_value()->set_struserpassword(m_strUserPassword);
    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_dev_value()->set_strdistributor(m_strDistributor);
    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_dev_value()->set_strdomainname(m_strDomainName);
    InteractiveMsg.mutable_reqvalue()->mutable_loginreq_dev_value()->set_strotherproperty(m_strOtherProperty);
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
    m_uiP2pSupplier = InteractiveMsg.reqvalue().p2pinforeq_dev_value().uip2psupplier();
    m_strDomainName = InteractiveMsg.reqvalue().p2pinforeq_dev_value().strdomainname();
}

void InteractiveProtoHandler::P2pInfoReq_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::P2pInfoReq_DEV_T);
    InteractiveMsg.mutable_reqvalue()->mutable_p2pinforeq_dev_value()->set_strdevid(m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_p2pinforeq_dev_value()->set_strdevipaddress(m_strDevIpAddress);
    InteractiveMsg.mutable_reqvalue()->mutable_p2pinforeq_dev_value()->set_uip2psupplier(m_uiP2pSupplier);
    InteractiveMsg.mutable_reqvalue()->mutable_p2pinforeq_dev_value()->set_strdomainname(m_strDomainName);
}

void InteractiveProtoHandler::P2pInfoRsp_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strP2pID = InteractiveMsg.rspvalue().p2pinforsp_dev_value().strp2pid();
    m_strP2pServer = InteractiveMsg.rspvalue().p2pinforsp_dev_value().strp2pserver();
    m_uiLease = InteractiveMsg.rspvalue().p2pinforsp_dev_value().uilease();
    m_strLicenseKey = InteractiveMsg.rspvalue().p2pinforsp_dev_value().strlicensekey();
    m_strPushID = InteractiveMsg.rspvalue().p2pinforsp_dev_value().strpushid();
}

void InteractiveProtoHandler::P2pInfoRsp_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::P2pInfoRsp_DEV_T);
    InteractiveMsg.mutable_rspvalue()->mutable_p2pinforsp_dev_value()->set_strp2pid(m_strP2pID);
    InteractiveMsg.mutable_rspvalue()->mutable_p2pinforsp_dev_value()->set_strp2pserver(m_strP2pServer);
    InteractiveMsg.mutable_rspvalue()->mutable_p2pinforsp_dev_value()->set_uilease(m_uiLease);
    InteractiveMsg.mutable_rspvalue()->mutable_p2pinforsp_dev_value()->set_strlicensekey(m_strLicenseKey);
    InteractiveMsg.mutable_rspvalue()->mutable_p2pinforsp_dev_value()->set_strpushid(m_strPushID);
}

void InteractiveProtoHandler::QueryAccessDomainNameReq_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strDevIpAddress = InteractiveMsg.reqvalue().queryaccessdomainnamereq_dev_value().strdevipaddress();
    m_strValue = InteractiveMsg.reqvalue().queryaccessdomainnamereq_dev_value().strvalue();
}

void InteractiveProtoHandler::QueryAccessDomainNameReq_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryAccessDomainNameReq_DEV_T);
    InteractiveMsg.mutable_reqvalue()->mutable_queryaccessdomainnamereq_dev_value()->set_strdevipaddress(m_strDevIpAddress);
    InteractiveMsg.mutable_reqvalue()->mutable_queryaccessdomainnamereq_dev_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::QueryAccessDomainNameRsp_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strDomainName = InteractiveMsg.rspvalue().queryaccessdomainnamersp_dev_value().strdomainname();
    m_uiLease = InteractiveMsg.rspvalue().queryaccessdomainnamersp_dev_value().uilease();
    m_strValue = InteractiveMsg.rspvalue().queryaccessdomainnamersp_dev_value().strvalue();
}

void InteractiveProtoHandler::QueryAccessDomainNameRsp_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryAccessDomainNameRsp_DEV_T);
    InteractiveMsg.mutable_rspvalue()->mutable_queryaccessdomainnamersp_dev_value()->set_strdomainname(m_strDomainName);
    InteractiveMsg.mutable_rspvalue()->mutable_queryaccessdomainnamersp_dev_value()->set_uilease(m_uiLease);
    InteractiveMsg.mutable_rspvalue()->mutable_queryaccessdomainnamersp_dev_value()->set_strvalue(m_strValue);
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

void InteractiveProtoHandler::RetrievePwdReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserName = InteractiveMsg.reqvalue().retrievepwdreq_usr_value().strusername();
    m_strEmail = InteractiveMsg.reqvalue().retrievepwdreq_usr_value().stremail();
    m_uiAppType = InteractiveMsg.reqvalue().retrievepwdreq_usr_value().uitype();
}

void InteractiveProtoHandler::RetrievePwdReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::RetrievePwdReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_retrievepwdreq_usr_value()->set_strusername(m_strUserName);
    InteractiveMsg.mutable_reqvalue()->mutable_retrievepwdreq_usr_value()->set_stremail(m_strEmail);
    InteractiveMsg.mutable_reqvalue()->mutable_retrievepwdreq_usr_value()->set_uitype(m_uiAppType);
}

void InteractiveProtoHandler::RetrievePwdRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().retrievepwdrsp_usr_value().strvalue();
}

void InteractiveProtoHandler::RetrievePwdRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::RetrievePwdRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_retrievepwdrsp_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::QueryTimeZoneReq_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strDevID = InteractiveMsg.reqvalue().querytimezonereq_dev_value().strdevid();
    m_strDevIpAddress = InteractiveMsg.reqvalue().querytimezonereq_dev_value().strdevipaddress();
}

void InteractiveProtoHandler::QueryTimeZoneReq_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryTimeZoneReq_DEV_T);
    InteractiveMsg.mutable_reqvalue()->mutable_querytimezonereq_dev_value()->set_strdevid(m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_querytimezonereq_dev_value()->set_strdevipaddress(m_strDevIpAddress);
}

void InteractiveProtoHandler::QueryTimeZoneRsp_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strCountryCode = InteractiveMsg.rspvalue().querytimezonersp_dev_value().strcountrycode();
    m_strCountryNameEn = InteractiveMsg.rspvalue().querytimezonersp_dev_value().strcountrynameen();
    m_strCountryNameZh = InteractiveMsg.rspvalue().querytimezonersp_dev_value().strcountrynamezh();
    m_strTimeZone = InteractiveMsg.rspvalue().querytimezonersp_dev_value().strtimezone();

}

void InteractiveProtoHandler::QueryTimeZoneRsp_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryTimeZoneRsp_DEV_T);
    InteractiveMsg.mutable_rspvalue()->mutable_querytimezonersp_dev_value()->set_strcountrycode(m_strCountryCode);
    InteractiveMsg.mutable_rspvalue()->mutable_querytimezonersp_dev_value()->set_strcountrynameen(m_strCountryNameEn);
    InteractiveMsg.mutable_rspvalue()->mutable_querytimezonersp_dev_value()->set_strcountrynamezh(m_strCountryNameZh);
    InteractiveMsg.mutable_rspvalue()->mutable_querytimezonersp_dev_value()->set_strtimezone(m_strTimeZone);

}

void InteractiveProtoHandler::QueryUpgradeSiteReq_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strDevID = InteractiveMsg.reqvalue().queryupgradesitereq_dev_value().strdevid();
    m_strDevIpAddress = InteractiveMsg.reqvalue().queryupgradesitereq_dev_value().strdevipaddress();
}

void InteractiveProtoHandler::QueryUpgradeSiteReq_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryUpgradeSiteReq_DEV_T);
    InteractiveMsg.mutable_reqvalue()->mutable_queryupgradesitereq_dev_value()->set_strdevid(m_strDevID);
    InteractiveMsg.mutable_reqvalue()->mutable_queryupgradesitereq_dev_value()->set_strdevipaddress(m_strDevIpAddress);
}

void InteractiveProtoHandler::QueryUpgradeSiteRsp_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strUpgradeSiteUrl = InteractiveMsg.rspvalue().queryupgradesitersp_dev_value().strupgradesiteurl();
    m_uiLease = InteractiveMsg.rspvalue().queryupgradesitersp_dev_value().uilease();
}

void InteractiveProtoHandler::QueryUpgradeSiteRsp_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryUpgradeSiteRsp_DEV_T);
    InteractiveMsg.mutable_rspvalue()->mutable_queryupgradesitersp_dev_value()->set_strupgradesiteurl(m_strUpgradeSiteUrl);
    InteractiveMsg.mutable_rspvalue()->mutable_queryupgradesitersp_dev_value()->set_uilease(m_uiLease);
}

void InteractiveProtoHandler::GetDeviceAccessRecordReq_INNER::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_uiBeginIndex = InteractiveMsg.reqvalue().getdeviceaccessrecordreq_inner_value().uibeginindex();
}

void InteractiveProtoHandler::GetDeviceAccessRecordReq_INNER::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::GetDeviceAccessRecordReq_INNER_T);
    InteractiveMsg.mutable_reqvalue()->mutable_getdeviceaccessrecordreq_inner_value()->set_uibeginindex(m_uiBeginIndex);
}

void InteractiveProtoHandler::GetDeviceAccessRecordRsp_INNER::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_uiRecordTotal = InteractiveMsg.rspvalue().getdeviceaccessrecordrsp_inner_value().uirecordtotal();

    UnSerializeDeviceAccessRecordList(m_deviceAccessRecordList, InteractiveMsg.rspvalue().getdeviceaccessrecordrsp_inner_value().deviceaccessrecord());
}

void InteractiveProtoHandler::GetDeviceAccessRecordRsp_INNER::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::GetDeviceAccessRecordRsp_INNER_T);
    InteractiveMsg.mutable_rspvalue()->mutable_getdeviceaccessrecordrsp_inner_value()->set_uirecordtotal(m_uiRecordTotal);

    auto devRecord = InteractiveMsg.mutable_rspvalue()->mutable_getdeviceaccessrecordrsp_inner_value()->mutable_deviceaccessrecord();
    SerializeDeviceAccessRecordList(m_deviceAccessRecordList, devRecord);
}

void InteractiveProtoHandler::GetUserAccessRecordReq_INNER::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_uiBeginIndex = InteractiveMsg.reqvalue().getuseraccessrecordreq_inner_value().uibeginindex();
}

void InteractiveProtoHandler::GetUserAccessRecordReq_INNER::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::GetUserAccessRecordReq_INNER_T);
    InteractiveMsg.mutable_reqvalue()->mutable_getuseraccessrecordreq_inner_value()->set_uibeginindex(m_uiBeginIndex);
}

void InteractiveProtoHandler::GetUserAccessRecordRsp_INNER::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_uiRecordTotal = InteractiveMsg.rspvalue().getuseraccessrecordrsp_inner_value().uirecordtotal();

    UnSerializeUserAccessRecordList(m_userAccessRecordList, InteractiveMsg.rspvalue().getuseraccessrecordrsp_inner_value().useraccessrecord());
}

void InteractiveProtoHandler::GetUserAccessRecordRsp_INNER::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::GetUserAccessRecordRsp_INNER_T);
    InteractiveMsg.mutable_rspvalue()->mutable_getuseraccessrecordrsp_inner_value()->set_uirecordtotal(m_uiRecordTotal);

    auto userRecord = InteractiveMsg.mutable_rspvalue()->mutable_getuseraccessrecordrsp_inner_value()->mutable_useraccessrecord();
    SerializeUserAccessRecordList(m_userAccessRecordList, userRecord);
}

void InteractiveProtoHandler::QueryAppUpgradeReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strCategory = InteractiveMsg.reqvalue().queryappupgradereq_usr_value().strcategory();
    m_strSubCategory = InteractiveMsg.reqvalue().queryappupgradereq_usr_value().strsubcategory();
    m_strCurrentVersion = InteractiveMsg.reqvalue().queryappupgradereq_usr_value().strcurrentversion();
}

void InteractiveProtoHandler::QueryAppUpgradeReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryAppUpgradeReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_queryappupgradereq_usr_value()->set_strcategory(m_strCategory);
    InteractiveMsg.mutable_reqvalue()->mutable_queryappupgradereq_usr_value()->set_strsubcategory(m_strSubCategory);
    InteractiveMsg.mutable_reqvalue()->mutable_queryappupgradereq_usr_value()->set_strcurrentversion(m_strCurrentVersion);
}

void InteractiveProtoHandler::QueryAppUpgradeRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_appUpgrade.m_uiNewVersionValid = InteractiveMsg.rspvalue().queryappupgradersp_usr_value().uinewversionvalid();
    m_appUpgrade.m_strAppName = InteractiveMsg.rspvalue().queryappupgradersp_usr_value().strappname();
    m_appUpgrade.m_strAppPath = InteractiveMsg.rspvalue().queryappupgradersp_usr_value().strapppath();
    m_appUpgrade.m_uiAppSize = InteractiveMsg.rspvalue().queryappupgradersp_usr_value().uiappsize();
    m_appUpgrade.m_strVersion = InteractiveMsg.rspvalue().queryappupgradersp_usr_value().strversion();
    m_appUpgrade.m_strDescription = InteractiveMsg.rspvalue().queryappupgradersp_usr_value().strdescription();
    m_appUpgrade.m_uiForceUpgrade = InteractiveMsg.rspvalue().queryappupgradersp_usr_value().uiforceupgrade();
    m_appUpgrade.m_strUpdateDate = InteractiveMsg.rspvalue().queryappupgradersp_usr_value().strupdatedate();
}

void InteractiveProtoHandler::QueryAppUpgradeRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryAppUpgradeRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_queryappupgradersp_usr_value()->set_uinewversionvalid(m_appUpgrade.m_uiNewVersionValid);
    InteractiveMsg.mutable_rspvalue()->mutable_queryappupgradersp_usr_value()->set_strappname(m_appUpgrade.m_strAppName);
    InteractiveMsg.mutable_rspvalue()->mutable_queryappupgradersp_usr_value()->set_strapppath(m_appUpgrade.m_strAppPath);
    InteractiveMsg.mutable_rspvalue()->mutable_queryappupgradersp_usr_value()->set_uiappsize(m_appUpgrade.m_uiAppSize);
    InteractiveMsg.mutable_rspvalue()->mutable_queryappupgradersp_usr_value()->set_strversion(m_appUpgrade.m_strVersion);
    InteractiveMsg.mutable_rspvalue()->mutable_queryappupgradersp_usr_value()->set_strdescription(m_appUpgrade.m_strDescription);
    InteractiveMsg.mutable_rspvalue()->mutable_queryappupgradersp_usr_value()->set_uiforceupgrade(m_appUpgrade.m_uiForceUpgrade);
    InteractiveMsg.mutable_rspvalue()->mutable_queryappupgradersp_usr_value()->set_strupdatedate(m_appUpgrade.m_strUpdateDate);
}

void InteractiveProtoHandler::QueryFirmwareUpgradeReq_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strCategory = InteractiveMsg.reqvalue().queryfirmwareupgradereq_dev_value().strcategory();
    m_strSubCategory = InteractiveMsg.reqvalue().queryfirmwareupgradereq_dev_value().strsubcategory();
    m_strCurrentVersion = InteractiveMsg.reqvalue().queryfirmwareupgradereq_dev_value().strcurrentversion();
    m_strDeviceID = InteractiveMsg.reqvalue().queryfirmwareupgradereq_dev_value().strdeviceid();
}

void InteractiveProtoHandler::QueryFirmwareUpgradeReq_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryFirmwareUpgradeReq_DEV_T);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfirmwareupgradereq_dev_value()->set_strcategory(m_strCategory);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfirmwareupgradereq_dev_value()->set_strsubcategory(m_strSubCategory);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfirmwareupgradereq_dev_value()->set_strcurrentversion(m_strCurrentVersion);
    InteractiveMsg.mutable_reqvalue()->mutable_queryfirmwareupgradereq_dev_value()->set_strdeviceid(m_strDeviceID);
}

void InteractiveProtoHandler::QueryFirmwareUpgradeRsp_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_firmwareUpgrade.m_uiNewVersionValid = InteractiveMsg.rspvalue().queryfirmwareupgradersp_dev_value().uinewversionvalid();
    m_firmwareUpgrade.m_strFirmwareName = InteractiveMsg.rspvalue().queryfirmwareupgradersp_dev_value().strfirmwarename();
    m_firmwareUpgrade.m_strFirmwarePath = InteractiveMsg.rspvalue().queryfirmwareupgradersp_dev_value().strfirmwarepath();
    m_firmwareUpgrade.m_uiFirmwareSize = InteractiveMsg.rspvalue().queryfirmwareupgradersp_dev_value().uifirmwaresize();
    m_firmwareUpgrade.m_strVersion = InteractiveMsg.rspvalue().queryfirmwareupgradersp_dev_value().strversion();
    m_firmwareUpgrade.m_strDescription = InteractiveMsg.rspvalue().queryfirmwareupgradersp_dev_value().strdescription();
    m_firmwareUpgrade.m_uiForceUpgrade = InteractiveMsg.rspvalue().queryfirmwareupgradersp_dev_value().uiforceupgrade();
    m_firmwareUpgrade.m_strUpdateDate = InteractiveMsg.rspvalue().queryfirmwareupgradersp_dev_value().strupdatedate();
}

void InteractiveProtoHandler::QueryFirmwareUpgradeRsp_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryFirmwareUpgradeRsp_DEV_T);
    InteractiveMsg.mutable_rspvalue()->mutable_queryfirmwareupgradersp_dev_value()->set_uinewversionvalid(m_firmwareUpgrade.m_uiNewVersionValid);
    InteractiveMsg.mutable_rspvalue()->mutable_queryfirmwareupgradersp_dev_value()->set_strfirmwarename(m_firmwareUpgrade.m_strFirmwareName);
    InteractiveMsg.mutable_rspvalue()->mutable_queryfirmwareupgradersp_dev_value()->set_strfirmwarepath(m_firmwareUpgrade.m_strFirmwarePath);
    InteractiveMsg.mutable_rspvalue()->mutable_queryfirmwareupgradersp_dev_value()->set_uifirmwaresize(m_firmwareUpgrade.m_uiFirmwareSize);
    InteractiveMsg.mutable_rspvalue()->mutable_queryfirmwareupgradersp_dev_value()->set_strversion(m_firmwareUpgrade.m_strVersion);
    InteractiveMsg.mutable_rspvalue()->mutable_queryfirmwareupgradersp_dev_value()->set_strdescription(m_firmwareUpgrade.m_strDescription);
    InteractiveMsg.mutable_rspvalue()->mutable_queryfirmwareupgradersp_dev_value()->set_uiforceupgrade(m_firmwareUpgrade.m_uiForceUpgrade);
    InteractiveMsg.mutable_rspvalue()->mutable_queryfirmwareupgradersp_dev_value()->set_strupdatedate(m_firmwareUpgrade.m_strUpdateDate);
}

void InteractiveProtoHandler::QueryUploadURLReq_MGR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.reqvalue().queryuploadurlreq_mgr_value().strvalue();
}

void InteractiveProtoHandler::QueryUploadURLReq_MGR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryUploadURLReq_MGR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_queryuploadurlreq_mgr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::QueryUploadURLRsp_MGR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strUploadURL = InteractiveMsg.rspvalue().queryuploadurlrsp_mgr_value().struploadurl();
}

void InteractiveProtoHandler::QueryUploadURLRsp_MGR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryUploadURLRsp_MGR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_queryuploadurlrsp_mgr_value()->set_struploadurl(m_strUploadURL);
}

void InteractiveProtoHandler::AddConfigurationReq_MGR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_configuration.m_strCategory = InteractiveMsg.reqvalue().addconfigurationreq_mgr_value().configurationinfo().strcategory();
    m_configuration.m_strSubCategory = InteractiveMsg.reqvalue().addconfigurationreq_mgr_value().configurationinfo().strsubcategory();
    m_configuration.m_strLatestVersion = InteractiveMsg.reqvalue().addconfigurationreq_mgr_value().configurationinfo().strlatestversion();
    m_configuration.m_strDescription = InteractiveMsg.reqvalue().addconfigurationreq_mgr_value().configurationinfo().strdescription();
    m_configuration.m_strForceVersion = InteractiveMsg.reqvalue().addconfigurationreq_mgr_value().configurationinfo().strforceversion();
    m_configuration.m_strServerAddress = InteractiveMsg.reqvalue().addconfigurationreq_mgr_value().configurationinfo().strserveraddress();
    m_configuration.m_strFileName = InteractiveMsg.reqvalue().addconfigurationreq_mgr_value().configurationinfo().strfilename();
    m_configuration.m_strFileID = InteractiveMsg.reqvalue().addconfigurationreq_mgr_value().configurationinfo().strfileid();
    m_configuration.m_uiFileSize = InteractiveMsg.reqvalue().addconfigurationreq_mgr_value().configurationinfo().uifilesize();
    m_configuration.m_strFilePath = InteractiveMsg.reqvalue().addconfigurationreq_mgr_value().configurationinfo().strfilepath();
    m_configuration.m_uiLeaseDuration = InteractiveMsg.reqvalue().addconfigurationreq_mgr_value().configurationinfo().uileaseduration();
    m_configuration.m_strUpdateDate = InteractiveMsg.reqvalue().addconfigurationreq_mgr_value().configurationinfo().strupdatedate();
    m_configuration.m_uiStatus = InteractiveMsg.reqvalue().addconfigurationreq_mgr_value().configurationinfo().uistatus();
    m_configuration.m_strExtend = InteractiveMsg.reqvalue().addconfigurationreq_mgr_value().configurationinfo().strextend();
}

void InteractiveProtoHandler::AddConfigurationReq_MGR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::AddConfigurationReq_MGR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_addconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strcategory(m_configuration.m_strCategory);
    InteractiveMsg.mutable_reqvalue()->mutable_addconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strsubcategory(m_configuration.m_strSubCategory);
    InteractiveMsg.mutable_reqvalue()->mutable_addconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strlatestversion(m_configuration.m_strLatestVersion);
    InteractiveMsg.mutable_reqvalue()->mutable_addconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strdescription(m_configuration.m_strDescription);
    InteractiveMsg.mutable_reqvalue()->mutable_addconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strforceversion(m_configuration.m_strForceVersion);
    InteractiveMsg.mutable_reqvalue()->mutable_addconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strserveraddress(m_configuration.m_strServerAddress);
    InteractiveMsg.mutable_reqvalue()->mutable_addconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strfilename(m_configuration.m_strFileName);
    InteractiveMsg.mutable_reqvalue()->mutable_addconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strfileid(m_configuration.m_strFileID);
    InteractiveMsg.mutable_reqvalue()->mutable_addconfigurationreq_mgr_value()->mutable_configurationinfo()->set_uifilesize(m_configuration.m_uiFileSize);
    InteractiveMsg.mutable_reqvalue()->mutable_addconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strfilepath(m_configuration.m_strFilePath);
    InteractiveMsg.mutable_reqvalue()->mutable_addconfigurationreq_mgr_value()->mutable_configurationinfo()->set_uileaseduration(m_configuration.m_uiLeaseDuration);
    InteractiveMsg.mutable_reqvalue()->mutable_addconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strupdatedate(m_configuration.m_strUpdateDate);
    InteractiveMsg.mutable_reqvalue()->mutable_addconfigurationreq_mgr_value()->mutable_configurationinfo()->set_uistatus(m_configuration.m_uiStatus);
    InteractiveMsg.mutable_reqvalue()->mutable_addconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strextend(m_configuration.m_strExtend);
}

void InteractiveProtoHandler::AddConfigurationRsp_MGR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().addconfigurationrsp_mgr_value().strvalue();
}

void InteractiveProtoHandler::AddConfigurationRsp_MGR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::AddConfigurationRsp_MGR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_addconfigurationrsp_mgr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::DeleteConfigurationReq_MGR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strCategory = InteractiveMsg.reqvalue().deleteconfigurationreq_mgr_value().strcategory();
    m_strSubCategory = InteractiveMsg.reqvalue().deleteconfigurationreq_mgr_value().strsubcategory();
}

void InteractiveProtoHandler::DeleteConfigurationReq_MGR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::DeleteConfigurationReq_MGR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_deleteconfigurationreq_mgr_value()->set_strcategory(m_strCategory);
    InteractiveMsg.mutable_reqvalue()->mutable_deleteconfigurationreq_mgr_value()->set_strsubcategory(m_strSubCategory);
}

void InteractiveProtoHandler::DeleteConfigurationRsp_MGR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().deleteconfigurationrsp_mgr_value().strvalue();
}

void InteractiveProtoHandler::DeleteConfigurationRsp_MGR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::DeleteConfigurationRsp_MGR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_deleteconfigurationrsp_mgr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::ModifyConfigurationReq_MGR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_configuration.m_strCategory = InteractiveMsg.reqvalue().modifyconfigurationreq_mgr_value().configurationinfo().strcategory();
    m_configuration.m_strSubCategory = InteractiveMsg.reqvalue().modifyconfigurationreq_mgr_value().configurationinfo().strsubcategory();
    m_configuration.m_strLatestVersion = InteractiveMsg.reqvalue().modifyconfigurationreq_mgr_value().configurationinfo().strlatestversion();
    m_configuration.m_strDescription = InteractiveMsg.reqvalue().modifyconfigurationreq_mgr_value().configurationinfo().strdescription();
    m_configuration.m_strForceVersion = InteractiveMsg.reqvalue().modifyconfigurationreq_mgr_value().configurationinfo().strforceversion();
    m_configuration.m_strServerAddress = InteractiveMsg.reqvalue().modifyconfigurationreq_mgr_value().configurationinfo().strserveraddress();
    m_configuration.m_strFileName = InteractiveMsg.reqvalue().modifyconfigurationreq_mgr_value().configurationinfo().strfilename();
    m_configuration.m_strFileID = InteractiveMsg.reqvalue().modifyconfigurationreq_mgr_value().configurationinfo().strfileid();
    m_configuration.m_uiFileSize = InteractiveMsg.reqvalue().modifyconfigurationreq_mgr_value().configurationinfo().uifilesize();
    m_configuration.m_strFilePath = InteractiveMsg.reqvalue().modifyconfigurationreq_mgr_value().configurationinfo().strfilepath();
    m_configuration.m_uiLeaseDuration = InteractiveMsg.reqvalue().modifyconfigurationreq_mgr_value().configurationinfo().uileaseduration();
    m_configuration.m_strUpdateDate = InteractiveMsg.reqvalue().modifyconfigurationreq_mgr_value().configurationinfo().strupdatedate();
    m_configuration.m_uiStatus = InteractiveMsg.reqvalue().modifyconfigurationreq_mgr_value().configurationinfo().uistatus();
    m_configuration.m_strExtend = InteractiveMsg.reqvalue().modifyconfigurationreq_mgr_value().configurationinfo().strextend();
}

void InteractiveProtoHandler::ModifyConfigurationReq_MGR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ModifyConfigurationReq_MGR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_modifyconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strcategory(m_configuration.m_strCategory);
    InteractiveMsg.mutable_reqvalue()->mutable_modifyconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strsubcategory(m_configuration.m_strSubCategory);
    InteractiveMsg.mutable_reqvalue()->mutable_modifyconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strlatestversion(m_configuration.m_strLatestVersion);
    InteractiveMsg.mutable_reqvalue()->mutable_modifyconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strdescription(m_configuration.m_strDescription);
    InteractiveMsg.mutable_reqvalue()->mutable_modifyconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strforceversion(m_configuration.m_strForceVersion);
    InteractiveMsg.mutable_reqvalue()->mutable_modifyconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strserveraddress(m_configuration.m_strServerAddress);
    InteractiveMsg.mutable_reqvalue()->mutable_modifyconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strfilename(m_configuration.m_strFileName);
    InteractiveMsg.mutable_reqvalue()->mutable_modifyconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strfileid(m_configuration.m_strFileID);
    InteractiveMsg.mutable_reqvalue()->mutable_modifyconfigurationreq_mgr_value()->mutable_configurationinfo()->set_uifilesize(m_configuration.m_uiFileSize);
    InteractiveMsg.mutable_reqvalue()->mutable_modifyconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strfilepath(m_configuration.m_strFilePath);
    InteractiveMsg.mutable_reqvalue()->mutable_modifyconfigurationreq_mgr_value()->mutable_configurationinfo()->set_uileaseduration(m_configuration.m_uiLeaseDuration);
    InteractiveMsg.mutable_reqvalue()->mutable_modifyconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strupdatedate(m_configuration.m_strUpdateDate);
    InteractiveMsg.mutable_reqvalue()->mutable_modifyconfigurationreq_mgr_value()->mutable_configurationinfo()->set_uistatus(m_configuration.m_uiStatus);
    InteractiveMsg.mutable_reqvalue()->mutable_modifyconfigurationreq_mgr_value()->mutable_configurationinfo()->set_strextend(m_configuration.m_strExtend);
}

void InteractiveProtoHandler::ModifyConfigurationRsp_MGR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().modifyconfigurationrsp_mgr_value().strvalue();
}

void InteractiveProtoHandler::ModifyConfigurationRsp_MGR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ModifyConfigurationRsp_MGR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_modifyconfigurationrsp_mgr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::QueryAllConfigurationReq_MGR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_uiBeginIndex = InteractiveMsg.reqvalue().queryallconfigurationreq_mgr_value().uibeginindex();
}

void InteractiveProtoHandler::QueryAllConfigurationReq_MGR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryAllConfigurationReq_MGR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_queryallconfigurationreq_mgr_value()->set_uibeginindex(m_uiBeginIndex);
}

void InteractiveProtoHandler::QueryAllConfigurationRsp_MGR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);

    UnSerializeConfigurationList(m_configurationList, InteractiveMsg.rspvalue().queryallconfigurationrsp_mgr_value().configurationinfo());
}

void InteractiveProtoHandler::QueryAllConfigurationRsp_MGR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryAllConfigurationRsp_MGR_T);

    auto cfgInfo = InteractiveMsg.mutable_rspvalue()->mutable_queryallconfigurationrsp_mgr_value()->mutable_configurationinfo();
    SerializeConfigurationList(m_configurationList, cfgInfo);
}

void InteractiveProtoHandler::ModifyDevicePropertyReq_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strDeviceID = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strdeviceid();
    m_strDomainName = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strdomainname();
    m_strP2pID = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strp2pid();
    m_strCorpID = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strcorpid();
    m_strDeviceName = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strdevicename();
    m_strDeviceIP = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strdeviceip();
    m_strDeviceIP2 = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strdeviceip2();
    m_strWebPort = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strwebport();
    m_strCtrlPort = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strctrlport();
    m_strProtocol = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strprotocol();
    m_strModel = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strmodel();
    m_strPostFrequency = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strpostfrequency();
    m_strVersion = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strversion();
    m_strDeviceStatus = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strdevicestatus();
    m_strServerIP = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strserverip();
    m_strServerPort = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strserverport();
    m_strTransfer = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strtransfer();
    m_strMobilePort = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strmobileport();
    m_strChannelCount = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strchannelcount();
    m_uiDeviceType = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().uidevicetype();
    m_strRequestSource = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().strrequestsource();
    m_doorbellParameter.m_strDoorbellName = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().doorbellparameter().strdoorbellname();
    m_doorbellParameter.m_strSerialNumber = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().doorbellparameter().strserialnumber();
    m_doorbellParameter.m_strDoorbellP2Pid = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().doorbellparameter().strdoorbellp2pid();
    m_doorbellParameter.m_strBatteryCapacity = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().doorbellparameter().strbatterycapacity();
    m_doorbellParameter.m_strChargingState = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().doorbellparameter().strchargingstate();
    m_doorbellParameter.m_strWifiSignal = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().doorbellparameter().strwifisignal();
    m_doorbellParameter.m_strVolumeLevel = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().doorbellparameter().strvolumelevel();
    m_doorbellParameter.m_strVersionNumber = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().doorbellparameter().strversionnumber();
    m_doorbellParameter.m_strChannelNumber = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().doorbellparameter().strchannelnumber();
    m_doorbellParameter.m_strCodingType = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().doorbellparameter().strcodingtype();
    m_doorbellParameter.m_strPIRAlarmSwtich = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().doorbellparameter().strpiralarmswtich();
    m_doorbellParameter.m_strDoorbellSwitch = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().doorbellparameter().strdoorbellswitch();
    m_doorbellParameter.m_strPIRAlarmLevel = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().doorbellparameter().strpiralarmlevel();
    m_doorbellParameter.m_strPIRIneffectiveTime = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().doorbellparameter().strpirineffectivetime();
    m_doorbellParameter.m_strCurrentWifi = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().doorbellparameter().strcurrentwifi();
    m_doorbellParameter.m_strSubCategory = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().doorbellparameter().strsubcategory();
    m_doorbellParameter.m_strDisturbMode = InteractiveMsg.reqvalue().modifydevicepropertyreq_dev_value().doorbellparameter().strdisturbmode();
}

void InteractiveProtoHandler::ModifyDevicePropertyReq_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ModifyDevicePropertyReq_DEV_T);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strdeviceid(m_strDeviceID);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strdomainname(m_strDomainName);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strp2pid(m_strP2pID);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strcorpid(m_strCorpID);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strdevicename(m_strDeviceName);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strdeviceip(m_strDeviceIP);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strdeviceip2(m_strDeviceIP2);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strwebport(m_strWebPort);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strctrlport(m_strCtrlPort);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strprotocol(m_strProtocol);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strmodel(m_strModel);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strpostfrequency(m_strPostFrequency);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strversion(m_strVersion);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strdevicestatus(m_strDeviceStatus);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strserverip(m_strServerIP);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strserverport(m_strServerPort);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strtransfer(m_strTransfer);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strmobileport(m_strMobilePort);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strchannelcount(m_strChannelCount);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_uidevicetype(m_uiDeviceType);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->set_strrequestsource(m_strRequestSource);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->mutable_doorbellparameter()->set_strdoorbellname(m_doorbellParameter.m_strDoorbellName);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->mutable_doorbellparameter()->set_strserialnumber(m_doorbellParameter.m_strSerialNumber);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->mutable_doorbellparameter()->set_strdoorbellp2pid(m_doorbellParameter.m_strDoorbellP2Pid);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->mutable_doorbellparameter()->set_strbatterycapacity(m_doorbellParameter.m_strBatteryCapacity);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->mutable_doorbellparameter()->set_strchargingstate(m_doorbellParameter.m_strChargingState);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->mutable_doorbellparameter()->set_strwifisignal(m_doorbellParameter.m_strWifiSignal);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->mutable_doorbellparameter()->set_strvolumelevel(m_doorbellParameter.m_strVolumeLevel);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->mutable_doorbellparameter()->set_strversionnumber(m_doorbellParameter.m_strVersionNumber);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->mutable_doorbellparameter()->set_strchannelnumber(m_doorbellParameter.m_strChannelNumber);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->mutable_doorbellparameter()->set_strcodingtype(m_doorbellParameter.m_strCodingType);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->mutable_doorbellparameter()->set_strpiralarmswtich(m_doorbellParameter.m_strPIRAlarmSwtich);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->mutable_doorbellparameter()->set_strdoorbellswitch(m_doorbellParameter.m_strDoorbellSwitch);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->mutable_doorbellparameter()->set_strpiralarmlevel(m_doorbellParameter.m_strPIRAlarmLevel);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->mutable_doorbellparameter()->set_strpirineffectivetime(m_doorbellParameter.m_strPIRIneffectiveTime);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->mutable_doorbellparameter()->set_strcurrentwifi(m_doorbellParameter.m_strCurrentWifi);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->mutable_doorbellparameter()->set_strsubcategory(m_doorbellParameter.m_strSubCategory);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydevicepropertyreq_dev_value()->mutable_doorbellparameter()->set_strdisturbmode(m_doorbellParameter.m_strDisturbMode);
}

void InteractiveProtoHandler::ModifyDevicePropertyRsp_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().modifydevicepropertyrsp_dev_value().strvalue();
}

void InteractiveProtoHandler::ModifyDevicePropertyRsp_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ModifyDevicePropertyRsp_DEV_T);
    InteractiveMsg.mutable_rspvalue()->mutable_modifydevicepropertyrsp_dev_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::QueryDeviceParameterReq_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strDeviceID = InteractiveMsg.reqvalue().querydeviceparameterreq_dev_value().strdeviceid();
    m_uiDeviceType = InteractiveMsg.reqvalue().querydeviceparameterreq_dev_value().uidevicetype();
    m_strQueryType = InteractiveMsg.reqvalue().querydeviceparameterreq_dev_value().strquerytype();
}

void InteractiveProtoHandler::QueryDeviceParameterReq_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryDeviceParameterReq_DEV_T);
    InteractiveMsg.mutable_reqvalue()->mutable_querydeviceparameterreq_dev_value()->set_strdeviceid(m_strDeviceID);
    InteractiveMsg.mutable_reqvalue()->mutable_querydeviceparameterreq_dev_value()->set_uidevicetype(m_uiDeviceType);
    InteractiveMsg.mutable_reqvalue()->mutable_querydeviceparameterreq_dev_value()->set_strquerytype(m_strQueryType);
}

void InteractiveProtoHandler::QueryDeviceParameterRsp_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_doorbellParameter.m_strDoorbellName = InteractiveMsg.rspvalue().querydeviceparameterrsp_dev_value().doorbellparameter().strdoorbellname();
    m_doorbellParameter.m_strSerialNumber = InteractiveMsg.rspvalue().querydeviceparameterrsp_dev_value().doorbellparameter().strserialnumber();
    m_doorbellParameter.m_strDoorbellP2Pid = InteractiveMsg.rspvalue().querydeviceparameterrsp_dev_value().doorbellparameter().strdoorbellp2pid();
    m_doorbellParameter.m_strBatteryCapacity = InteractiveMsg.rspvalue().querydeviceparameterrsp_dev_value().doorbellparameter().strbatterycapacity();
    m_doorbellParameter.m_strChargingState = InteractiveMsg.rspvalue().querydeviceparameterrsp_dev_value().doorbellparameter().strchargingstate();
    m_doorbellParameter.m_strWifiSignal = InteractiveMsg.rspvalue().querydeviceparameterrsp_dev_value().doorbellparameter().strwifisignal();
    m_doorbellParameter.m_strVolumeLevel = InteractiveMsg.rspvalue().querydeviceparameterrsp_dev_value().doorbellparameter().strvolumelevel();
    m_doorbellParameter.m_strVersionNumber = InteractiveMsg.rspvalue().querydeviceparameterrsp_dev_value().doorbellparameter().strversionnumber();
    m_doorbellParameter.m_strChannelNumber = InteractiveMsg.rspvalue().querydeviceparameterrsp_dev_value().doorbellparameter().strchannelnumber();
    m_doorbellParameter.m_strCodingType = InteractiveMsg.rspvalue().querydeviceparameterrsp_dev_value().doorbellparameter().strcodingtype();
    m_doorbellParameter.m_strPIRAlarmSwtich = InteractiveMsg.rspvalue().querydeviceparameterrsp_dev_value().doorbellparameter().strpiralarmswtich();
    m_doorbellParameter.m_strDoorbellSwitch = InteractiveMsg.rspvalue().querydeviceparameterrsp_dev_value().doorbellparameter().strdoorbellswitch();
    m_doorbellParameter.m_strPIRAlarmLevel = InteractiveMsg.rspvalue().querydeviceparameterrsp_dev_value().doorbellparameter().strpiralarmlevel();
    m_doorbellParameter.m_strPIRIneffectiveTime = InteractiveMsg.rspvalue().querydeviceparameterrsp_dev_value().doorbellparameter().strpirineffectivetime();
    m_doorbellParameter.m_strCurrentWifi = InteractiveMsg.rspvalue().querydeviceparameterrsp_dev_value().doorbellparameter().strcurrentwifi();
    m_doorbellParameter.m_strSubCategory = InteractiveMsg.rspvalue().querydeviceparameterrsp_dev_value().doorbellparameter().strsubcategory();
    m_doorbellParameter.m_strDisturbMode = InteractiveMsg.rspvalue().querydeviceparameterrsp_dev_value().doorbellparameter().strdisturbmode();
}

void InteractiveProtoHandler::QueryDeviceParameterRsp_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryDeviceParameterRsp_DEV_T);
    InteractiveMsg.mutable_rspvalue()->mutable_querydeviceparameterrsp_dev_value()->mutable_doorbellparameter()->set_strdoorbellname(m_doorbellParameter.m_strDoorbellName);
    InteractiveMsg.mutable_rspvalue()->mutable_querydeviceparameterrsp_dev_value()->mutable_doorbellparameter()->set_strserialnumber(m_doorbellParameter.m_strSerialNumber);
    InteractiveMsg.mutable_rspvalue()->mutable_querydeviceparameterrsp_dev_value()->mutable_doorbellparameter()->set_strdoorbellp2pid(m_doorbellParameter.m_strDoorbellP2Pid);
    InteractiveMsg.mutable_rspvalue()->mutable_querydeviceparameterrsp_dev_value()->mutable_doorbellparameter()->set_strbatterycapacity(m_doorbellParameter.m_strBatteryCapacity);
    InteractiveMsg.mutable_rspvalue()->mutable_querydeviceparameterrsp_dev_value()->mutable_doorbellparameter()->set_strchargingstate(m_doorbellParameter.m_strChargingState);
    InteractiveMsg.mutable_rspvalue()->mutable_querydeviceparameterrsp_dev_value()->mutable_doorbellparameter()->set_strwifisignal(m_doorbellParameter.m_strWifiSignal);
    InteractiveMsg.mutable_rspvalue()->mutable_querydeviceparameterrsp_dev_value()->mutable_doorbellparameter()->set_strvolumelevel(m_doorbellParameter.m_strVolumeLevel);
    InteractiveMsg.mutable_rspvalue()->mutable_querydeviceparameterrsp_dev_value()->mutable_doorbellparameter()->set_strversionnumber(m_doorbellParameter.m_strVersionNumber);
    InteractiveMsg.mutable_rspvalue()->mutable_querydeviceparameterrsp_dev_value()->mutable_doorbellparameter()->set_strchannelnumber(m_doorbellParameter.m_strChannelNumber);
    InteractiveMsg.mutable_rspvalue()->mutable_querydeviceparameterrsp_dev_value()->mutable_doorbellparameter()->set_strcodingtype(m_doorbellParameter.m_strCodingType);
    InteractiveMsg.mutable_rspvalue()->mutable_querydeviceparameterrsp_dev_value()->mutable_doorbellparameter()->set_strpiralarmswtich(m_doorbellParameter.m_strPIRAlarmSwtich);
    InteractiveMsg.mutable_rspvalue()->mutable_querydeviceparameterrsp_dev_value()->mutable_doorbellparameter()->set_strdoorbellswitch(m_doorbellParameter.m_strDoorbellSwitch);
    InteractiveMsg.mutable_rspvalue()->mutable_querydeviceparameterrsp_dev_value()->mutable_doorbellparameter()->set_strpiralarmlevel(m_doorbellParameter.m_strPIRAlarmLevel);
    InteractiveMsg.mutable_rspvalue()->mutable_querydeviceparameterrsp_dev_value()->mutable_doorbellparameter()->set_strpirineffectivetime(m_doorbellParameter.m_strPIRIneffectiveTime);
    InteractiveMsg.mutable_rspvalue()->mutable_querydeviceparameterrsp_dev_value()->mutable_doorbellparameter()->set_strcurrentwifi(m_doorbellParameter.m_strCurrentWifi);
    InteractiveMsg.mutable_rspvalue()->mutable_querydeviceparameterrsp_dev_value()->mutable_doorbellparameter()->set_strsubcategory(m_doorbellParameter.m_strSubCategory);
    InteractiveMsg.mutable_rspvalue()->mutable_querydeviceparameterrsp_dev_value()->mutable_doorbellparameter()->set_strdisturbmode(m_doorbellParameter.m_strDisturbMode);
}

void InteractiveProtoHandler::QueryIfP2pIDValidReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strP2pID = InteractiveMsg.reqvalue().queryifp2pidvalidreq_usr_value().strp2pid();
    m_uiSuppliser = InteractiveMsg.reqvalue().queryifp2pidvalidreq_usr_value().uip2ptype();
}

void InteractiveProtoHandler::QueryIfP2pIDValidReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryIfP2pIDValidReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_queryifp2pidvalidreq_usr_value()->set_strp2pid(m_strP2pID);
    InteractiveMsg.mutable_reqvalue()->mutable_queryifp2pidvalidreq_usr_value()->set_uip2ptype(m_uiSuppliser);
}

void InteractiveProtoHandler::QueryIfP2pIDValidRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().queryifp2pidvalidrsp_usr_value().strvalue();
}

void InteractiveProtoHandler::QueryIfP2pIDValidRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryIfP2pIDValidRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_queryifp2pidvalidrsp_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::QueryPlatformPushStatusReq_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strDeviceID = InteractiveMsg.reqvalue().queryplatformpushstatusreq_dev_value().strdeviceid();
}

void InteractiveProtoHandler::QueryPlatformPushStatusReq_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryPlatformPushStatusReq_DEV_T);
    InteractiveMsg.mutable_reqvalue()->mutable_queryplatformpushstatusreq_dev_value()->set_strdeviceid(m_strDeviceID);

}

void InteractiveProtoHandler::QueryPlatformPushStatusRsp_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strStatus = InteractiveMsg.rspvalue().queryplatformpushstatusrsp_dev_value().strstatus();
}

void InteractiveProtoHandler::QueryPlatformPushStatusRsp_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryPlatformPushStatusRsp_DEV_T);
    InteractiveMsg.mutable_rspvalue()->mutable_queryplatformpushstatusrsp_dev_value()->set_strstatus(m_strStatus);
}

void InteractiveProtoHandler::DeviceEventReportReq_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strDeviceID = InteractiveMsg.reqvalue().deviceeventreportreq_dev_value().strdeviceid();
    m_uiDeviceType = InteractiveMsg.reqvalue().deviceeventreportreq_dev_value().uidevicetype();
    m_uiEventType = InteractiveMsg.reqvalue().deviceeventreportreq_dev_value().uieventtype();
    m_uiEventState = InteractiveMsg.reqvalue().deviceeventreportreq_dev_value().uieventstate();
    m_strFileID = InteractiveMsg.reqvalue().deviceeventreportreq_dev_value().strfileid();
    m_strEventTime = InteractiveMsg.reqvalue().deviceeventreportreq_dev_value().streventtime();
}

void InteractiveProtoHandler::DeviceEventReportReq_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::DeviceEventReportReq_DEV_T);
    InteractiveMsg.mutable_reqvalue()->mutable_deviceeventreportreq_dev_value()->set_strdeviceid(m_strDeviceID);
    InteractiveMsg.mutable_reqvalue()->mutable_deviceeventreportreq_dev_value()->set_uidevicetype(m_uiDeviceType);
    InteractiveMsg.mutable_reqvalue()->mutable_deviceeventreportreq_dev_value()->set_uieventtype(m_uiEventType);
    InteractiveMsg.mutable_reqvalue()->mutable_deviceeventreportreq_dev_value()->set_uieventstate(m_uiEventState);
    InteractiveMsg.mutable_reqvalue()->mutable_deviceeventreportreq_dev_value()->set_strfileid(m_strFileID);
    InteractiveMsg.mutable_reqvalue()->mutable_deviceeventreportreq_dev_value()->set_streventtime(m_strEventTime);
}

void InteractiveProtoHandler::DeviceEventReportRsp_DEV::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strEventID = InteractiveMsg.rspvalue().deviceeventreportrsp_dev_value().streventid();
}

void InteractiveProtoHandler::DeviceEventReportRsp_DEV::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::DeviceEventReportRsp_DEV_T);
    InteractiveMsg.mutable_rspvalue()->mutable_deviceeventreportrsp_dev_value()->set_streventid(m_strEventID);
}

void InteractiveProtoHandler::QueryAllDeviceEventReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().queryalldeviceeventreq_usr_value().struserid();
    m_strDeviceID = InteractiveMsg.reqvalue().queryalldeviceeventreq_usr_value().strdeviceid();
    m_uiDeviceShared = InteractiveMsg.reqvalue().queryalldeviceeventreq_usr_value().uideviceshared();
    m_uiEventType = InteractiveMsg.reqvalue().queryalldeviceeventreq_usr_value().uieventtype();
    m_uiReadState = InteractiveMsg.reqvalue().queryalldeviceeventreq_usr_value().uireadstate();
    m_uiBeginIndex = InteractiveMsg.reqvalue().queryalldeviceeventreq_usr_value().uibeginindex();
    m_strBeginDate = InteractiveMsg.reqvalue().queryalldeviceeventreq_usr_value().strbegindate();
    m_strEndDate = InteractiveMsg.reqvalue().queryalldeviceeventreq_usr_value().strenddate();
}

void InteractiveProtoHandler::QueryAllDeviceEventReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryAllDeviceEventReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_queryalldeviceeventreq_usr_value()->set_struserid(m_strUserID);
    InteractiveMsg.mutable_reqvalue()->mutable_queryalldeviceeventreq_usr_value()->set_strdeviceid(m_strDeviceID);
    InteractiveMsg.mutable_reqvalue()->mutable_queryalldeviceeventreq_usr_value()->set_uideviceshared(m_uiDeviceShared);
    InteractiveMsg.mutable_reqvalue()->mutable_queryalldeviceeventreq_usr_value()->set_uieventtype(m_uiEventType);
    InteractiveMsg.mutable_reqvalue()->mutable_queryalldeviceeventreq_usr_value()->set_uireadstate(m_uiReadState);
    InteractiveMsg.mutable_reqvalue()->mutable_queryalldeviceeventreq_usr_value()->set_uibeginindex(m_uiBeginIndex);
    InteractiveMsg.mutable_reqvalue()->mutable_queryalldeviceeventreq_usr_value()->set_strbegindate(m_strBeginDate);
    InteractiveMsg.mutable_reqvalue()->mutable_queryalldeviceeventreq_usr_value()->set_strenddate(m_strEndDate);
}

void InteractiveProtoHandler::QueryAllDeviceEventRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);

    UnSerializeDeviceEventList(m_deviceEventList, InteractiveMsg.rspvalue().queryalldeviceeventrsp_usr_value().deviceevent());
}

void InteractiveProtoHandler::QueryAllDeviceEventRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryAllDeviceEventRsp_USR_T);

    auto cfgInfo = InteractiveMsg.mutable_rspvalue()->mutable_queryalldeviceeventrsp_usr_value()->mutable_deviceevent();
    SerializeDeviceEventList(m_deviceEventList, cfgInfo);
}

void InteractiveProtoHandler::DeleteDeviceEventReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().deletedeviceeventreq_usr_value().struserid();
    m_strDeviceID = InteractiveMsg.reqvalue().deletedeviceeventreq_usr_value().strdeviceid();
    m_strEventID = InteractiveMsg.reqvalue().deletedeviceeventreq_usr_value().streventid();
}

void InteractiveProtoHandler::DeleteDeviceEventReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::DeleteDeviceEventReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_deletedeviceeventreq_usr_value()->set_struserid(m_strUserID);
    InteractiveMsg.mutable_reqvalue()->mutable_deletedeviceeventreq_usr_value()->set_strdeviceid(m_strDeviceID);
    InteractiveMsg.mutable_reqvalue()->mutable_deletedeviceeventreq_usr_value()->set_streventid(m_strEventID);
}

void InteractiveProtoHandler::DeleteDeviceEventRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().deletedeviceeventrsp_usr_value().strvalue();
}

void InteractiveProtoHandler::DeleteDeviceEventRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::DeleteDeviceEventRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_deletedeviceeventrsp_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::ModifyDeviceEventReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().modifydeviceeventreq_usr_value().struserid();
    m_strDeviceID = InteractiveMsg.reqvalue().modifydeviceeventreq_usr_value().strdeviceid();
    m_strEventID = InteractiveMsg.reqvalue().modifydeviceeventreq_usr_value().streventid();
    m_uiEventType = InteractiveMsg.reqvalue().modifydeviceeventreq_usr_value().uieventtype();
    m_uiEventState = InteractiveMsg.reqvalue().modifydeviceeventreq_usr_value().uieventstate();
    m_strUpdateTime = InteractiveMsg.reqvalue().modifydeviceeventreq_usr_value().strupdatetime();
    m_strFileID = InteractiveMsg.reqvalue().modifydeviceeventreq_usr_value().strfileid();
    m_uiReadState = InteractiveMsg.reqvalue().modifydeviceeventreq_usr_value().uireadstate();
}

void InteractiveProtoHandler::ModifyDeviceEventReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ModifyDeviceEventReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydeviceeventreq_usr_value()->set_struserid(m_strUserID);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydeviceeventreq_usr_value()->set_strdeviceid(m_strDeviceID);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydeviceeventreq_usr_value()->set_streventid(m_strEventID);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydeviceeventreq_usr_value()->set_uieventtype(m_uiEventType);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydeviceeventreq_usr_value()->set_uieventstate(m_uiEventState);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydeviceeventreq_usr_value()->set_strupdatetime(m_strUpdateTime);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydeviceeventreq_usr_value()->set_strfileid(m_strFileID);
    InteractiveMsg.mutable_reqvalue()->mutable_modifydeviceeventreq_usr_value()->set_uireadstate(m_uiReadState);
}

void InteractiveProtoHandler::ModifyDeviceEventRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().modifydeviceeventrsp_usr_value().strvalue();
}

void InteractiveProtoHandler::ModifyDeviceEventRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ModifyDeviceEventRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_modifydeviceeventrsp_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::AddStorageDetailReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_storageDetail.m_uiDomainID = InteractiveMsg.reqvalue().addstoragedetailreq_usr_value().storagedetail().uidomainid();
    m_storageDetail.m_strObjectID = InteractiveMsg.reqvalue().addstoragedetailreq_usr_value().storagedetail().strobjectid();
    m_storageDetail.m_uiObjectType = InteractiveMsg.reqvalue().addstoragedetailreq_usr_value().storagedetail().uiobjecttype();
    m_storageDetail.m_strStorageName = InteractiveMsg.reqvalue().addstoragedetailreq_usr_value().storagedetail().strstoragename();
    m_storageDetail.m_uiStorageType = InteractiveMsg.reqvalue().addstoragedetailreq_usr_value().storagedetail().uistoragetype();
    m_storageDetail.m_uiOverlapType = InteractiveMsg.reqvalue().addstoragedetailreq_usr_value().storagedetail().uioverlaptype();
    m_storageDetail.m_uiStorageTimeUpLimit = InteractiveMsg.reqvalue().addstoragedetailreq_usr_value().storagedetail().uistoragetimeuplimit();
    m_storageDetail.m_uiStorageTimeDownLimit = InteractiveMsg.reqvalue().addstoragedetailreq_usr_value().storagedetail().uistoragetimedownlimit();
    m_storageDetail.m_uiSizeOfSpaceUsed = InteractiveMsg.reqvalue().addstoragedetailreq_usr_value().storagedetail().uisizeofspaceused();
    m_storageDetail.m_uiStorageUnitType = InteractiveMsg.reqvalue().addstoragedetailreq_usr_value().storagedetail().uistorageunittype();
    m_storageDetail.m_strBeginDate = InteractiveMsg.reqvalue().addstoragedetailreq_usr_value().storagedetail().strbegindate();
    m_storageDetail.m_strEndDate = InteractiveMsg.reqvalue().addstoragedetailreq_usr_value().storagedetail().strenddate();
    m_storageDetail.m_uiStatus = InteractiveMsg.reqvalue().addstoragedetailreq_usr_value().storagedetail().uistatus();
    m_storageDetail.m_strExtend = InteractiveMsg.reqvalue().addstoragedetailreq_usr_value().storagedetail().strextend();
}

void InteractiveProtoHandler::AddStorageDetailReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::AddStorageDetailReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_addstoragedetailreq_usr_value()->mutable_storagedetail()->set_uidomainid(m_storageDetail.m_uiDomainID);
    InteractiveMsg.mutable_reqvalue()->mutable_addstoragedetailreq_usr_value()->mutable_storagedetail()->set_strobjectid(m_storageDetail.m_strObjectID);
    InteractiveMsg.mutable_reqvalue()->mutable_addstoragedetailreq_usr_value()->mutable_storagedetail()->set_uiobjecttype(m_storageDetail.m_uiObjectType);
    InteractiveMsg.mutable_reqvalue()->mutable_addstoragedetailreq_usr_value()->mutable_storagedetail()->set_strstoragename(m_storageDetail.m_strStorageName);
    InteractiveMsg.mutable_reqvalue()->mutable_addstoragedetailreq_usr_value()->mutable_storagedetail()->set_uistoragetype(m_storageDetail.m_uiStorageType);
    InteractiveMsg.mutable_reqvalue()->mutable_addstoragedetailreq_usr_value()->mutable_storagedetail()->set_uioverlaptype(m_storageDetail.m_uiOverlapType);
    InteractiveMsg.mutable_reqvalue()->mutable_addstoragedetailreq_usr_value()->mutable_storagedetail()->set_uistoragetimeuplimit(m_storageDetail.m_uiStorageTimeUpLimit);
    InteractiveMsg.mutable_reqvalue()->mutable_addstoragedetailreq_usr_value()->mutable_storagedetail()->set_uistoragetimedownlimit(m_storageDetail.m_uiStorageTimeDownLimit);
    InteractiveMsg.mutable_reqvalue()->mutable_addstoragedetailreq_usr_value()->mutable_storagedetail()->set_uisizeofspaceused(m_storageDetail.m_uiSizeOfSpaceUsed);
    InteractiveMsg.mutable_reqvalue()->mutable_addstoragedetailreq_usr_value()->mutable_storagedetail()->set_uistorageunittype(m_storageDetail.m_uiStorageUnitType);
    InteractiveMsg.mutable_reqvalue()->mutable_addstoragedetailreq_usr_value()->mutable_storagedetail()->set_strbegindate(m_storageDetail.m_strBeginDate);
    InteractiveMsg.mutable_reqvalue()->mutable_addstoragedetailreq_usr_value()->mutable_storagedetail()->set_strenddate(m_storageDetail.m_strEndDate);
    InteractiveMsg.mutable_reqvalue()->mutable_addstoragedetailreq_usr_value()->mutable_storagedetail()->set_uistatus(m_storageDetail.m_uiStatus);
    InteractiveMsg.mutable_reqvalue()->mutable_addstoragedetailreq_usr_value()->mutable_storagedetail()->set_strextend(m_storageDetail.m_strExtend);
}

void InteractiveProtoHandler::AddStorageDetailRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().addstoragedetailrsp_usr_value().strvalue();
}

void InteractiveProtoHandler::AddStorageDetailRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::AddStorageDetailRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_addstoragedetailrsp_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::DeleteStorageDetailReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strObjectID = InteractiveMsg.reqvalue().deletestoragedetailreq_usr_value().strobjectid();
    m_uiDomainID = InteractiveMsg.reqvalue().deletestoragedetailreq_usr_value().uidomainid();
}

void InteractiveProtoHandler::DeleteStorageDetailReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::DeleteStorageDetailReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_deletestoragedetailreq_usr_value()->set_uidomainid(m_uiDomainID);
    InteractiveMsg.mutable_reqvalue()->mutable_deletestoragedetailreq_usr_value()->set_strobjectid(m_strObjectID);
}

void InteractiveProtoHandler::DeleteStorageDetailRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().deletestoragedetailrsp_usr_value().strvalue();
}

void InteractiveProtoHandler::DeleteStorageDetailRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::DeleteStorageDetailRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_deletestoragedetailrsp_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::ModifyStorageDetailReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_storageDetail.m_uiDomainID = InteractiveMsg.reqvalue().modifystoragedetailreq_usr_value().storagedetail().uidomainid();
    m_storageDetail.m_strObjectID = InteractiveMsg.reqvalue().modifystoragedetailreq_usr_value().storagedetail().strobjectid();
    m_storageDetail.m_uiObjectType = InteractiveMsg.reqvalue().modifystoragedetailreq_usr_value().storagedetail().uiobjecttype();
    m_storageDetail.m_strStorageName = InteractiveMsg.reqvalue().modifystoragedetailreq_usr_value().storagedetail().strstoragename();
    m_storageDetail.m_uiStorageType = InteractiveMsg.reqvalue().modifystoragedetailreq_usr_value().storagedetail().uistoragetype();
    m_storageDetail.m_uiOverlapType = InteractiveMsg.reqvalue().modifystoragedetailreq_usr_value().storagedetail().uioverlaptype();
    m_storageDetail.m_uiStorageTimeUpLimit = InteractiveMsg.reqvalue().modifystoragedetailreq_usr_value().storagedetail().uistoragetimeuplimit();
    m_storageDetail.m_uiStorageTimeDownLimit = InteractiveMsg.reqvalue().modifystoragedetailreq_usr_value().storagedetail().uistoragetimedownlimit();
    m_storageDetail.m_uiSizeOfSpaceUsed = InteractiveMsg.reqvalue().modifystoragedetailreq_usr_value().storagedetail().uisizeofspaceused();
    m_storageDetail.m_uiStorageUnitType = InteractiveMsg.reqvalue().modifystoragedetailreq_usr_value().storagedetail().uistorageunittype();
    m_storageDetail.m_strBeginDate = InteractiveMsg.reqvalue().modifystoragedetailreq_usr_value().storagedetail().strbegindate();
    m_storageDetail.m_strEndDate = InteractiveMsg.reqvalue().modifystoragedetailreq_usr_value().storagedetail().strenddate();
    m_storageDetail.m_uiStatus = InteractiveMsg.reqvalue().modifystoragedetailreq_usr_value().storagedetail().uistatus();
    m_storageDetail.m_strExtend = InteractiveMsg.reqvalue().modifystoragedetailreq_usr_value().storagedetail().strextend();
}

void InteractiveProtoHandler::ModifyStorageDetailReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ModifyStorageDetailReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_modifystoragedetailreq_usr_value()->mutable_storagedetail()->set_uidomainid(m_storageDetail.m_uiDomainID);
    InteractiveMsg.mutable_reqvalue()->mutable_modifystoragedetailreq_usr_value()->mutable_storagedetail()->set_strobjectid(m_storageDetail.m_strObjectID);
    InteractiveMsg.mutable_reqvalue()->mutable_modifystoragedetailreq_usr_value()->mutable_storagedetail()->set_uiobjecttype(m_storageDetail.m_uiObjectType);
    InteractiveMsg.mutable_reqvalue()->mutable_modifystoragedetailreq_usr_value()->mutable_storagedetail()->set_strstoragename(m_storageDetail.m_strStorageName);
    InteractiveMsg.mutable_reqvalue()->mutable_modifystoragedetailreq_usr_value()->mutable_storagedetail()->set_uistoragetype(m_storageDetail.m_uiStorageType);
    InteractiveMsg.mutable_reqvalue()->mutable_modifystoragedetailreq_usr_value()->mutable_storagedetail()->set_uioverlaptype(m_storageDetail.m_uiOverlapType);
    InteractiveMsg.mutable_reqvalue()->mutable_modifystoragedetailreq_usr_value()->mutable_storagedetail()->set_uistoragetimeuplimit(m_storageDetail.m_uiStorageTimeUpLimit);
    InteractiveMsg.mutable_reqvalue()->mutable_modifystoragedetailreq_usr_value()->mutable_storagedetail()->set_uistoragetimedownlimit(m_storageDetail.m_uiStorageTimeDownLimit);
    InteractiveMsg.mutable_reqvalue()->mutable_modifystoragedetailreq_usr_value()->mutable_storagedetail()->set_uisizeofspaceused(m_storageDetail.m_uiSizeOfSpaceUsed);
    InteractiveMsg.mutable_reqvalue()->mutable_modifystoragedetailreq_usr_value()->mutable_storagedetail()->set_uistorageunittype(m_storageDetail.m_uiStorageUnitType);
    InteractiveMsg.mutable_reqvalue()->mutable_modifystoragedetailreq_usr_value()->mutable_storagedetail()->set_strbegindate(m_storageDetail.m_strBeginDate);
    InteractiveMsg.mutable_reqvalue()->mutable_modifystoragedetailreq_usr_value()->mutable_storagedetail()->set_strenddate(m_storageDetail.m_strEndDate);
    InteractiveMsg.mutable_reqvalue()->mutable_modifystoragedetailreq_usr_value()->mutable_storagedetail()->set_uistatus(m_storageDetail.m_uiStatus);
    InteractiveMsg.mutable_reqvalue()->mutable_modifystoragedetailreq_usr_value()->mutable_storagedetail()->set_strextend(m_storageDetail.m_strExtend);
}

void InteractiveProtoHandler::ModifyStorageDetailRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_strValue = InteractiveMsg.rspvalue().modifystoragedetailrsp_usr_value().strvalue();
}

void InteractiveProtoHandler::ModifyStorageDetailRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::ModifyStorageDetailRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_modifystoragedetailrsp_usr_value()->set_strvalue(m_strValue);
}

void InteractiveProtoHandler::QueryStorageDetailReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strObjectID = InteractiveMsg.reqvalue().querystoragedetailreq_usr_value().strobjectid();
    m_uiDomainID = InteractiveMsg.reqvalue().querystoragedetailreq_usr_value().uidomainid();
}

void InteractiveProtoHandler::QueryStorageDetailReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryStorageDetailReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_querystoragedetailreq_usr_value()->set_uidomainid(m_uiDomainID);
    InteractiveMsg.mutable_reqvalue()->mutable_querystoragedetailreq_usr_value()->set_strobjectid(m_strObjectID);
}

void InteractiveProtoHandler::QueryStorageDetailRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_storageDetail.m_uiDomainID = InteractiveMsg.rspvalue().querystoragedetailrsp_usr_value().storagedetail().uidomainid();
    m_storageDetail.m_strObjectID = InteractiveMsg.rspvalue().querystoragedetailrsp_usr_value().storagedetail().strobjectid();
    m_storageDetail.m_uiObjectType = InteractiveMsg.rspvalue().querystoragedetailrsp_usr_value().storagedetail().uiobjecttype();
    m_storageDetail.m_strStorageName = InteractiveMsg.rspvalue().querystoragedetailrsp_usr_value().storagedetail().strstoragename();
    m_storageDetail.m_uiStorageType = InteractiveMsg.rspvalue().querystoragedetailrsp_usr_value().storagedetail().uistoragetype();
    m_storageDetail.m_uiOverlapType = InteractiveMsg.rspvalue().querystoragedetailrsp_usr_value().storagedetail().uioverlaptype();
    m_storageDetail.m_uiStorageTimeUpLimit = InteractiveMsg.rspvalue().querystoragedetailrsp_usr_value().storagedetail().uistoragetimeuplimit();
    m_storageDetail.m_uiStorageTimeDownLimit = InteractiveMsg.rspvalue().querystoragedetailrsp_usr_value().storagedetail().uistoragetimedownlimit();
    m_storageDetail.m_uiSizeOfSpaceUsed = InteractiveMsg.rspvalue().querystoragedetailrsp_usr_value().storagedetail().uisizeofspaceused();
    m_storageDetail.m_uiStorageUnitType = InteractiveMsg.rspvalue().querystoragedetailrsp_usr_value().storagedetail().uistorageunittype();
    m_storageDetail.m_strBeginDate = InteractiveMsg.rspvalue().querystoragedetailrsp_usr_value().storagedetail().strbegindate();
    m_storageDetail.m_strEndDate = InteractiveMsg.rspvalue().querystoragedetailrsp_usr_value().storagedetail().strenddate();
    m_storageDetail.m_uiStatus = InteractiveMsg.rspvalue().querystoragedetailrsp_usr_value().storagedetail().uistatus();
    m_storageDetail.m_strExtend = InteractiveMsg.rspvalue().querystoragedetailrsp_usr_value().storagedetail().strextend();
}

void InteractiveProtoHandler::QueryStorageDetailRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryStorageDetailRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_querystoragedetailrsp_usr_value()->mutable_storagedetail()->set_uidomainid(m_storageDetail.m_uiDomainID);
    InteractiveMsg.mutable_rspvalue()->mutable_querystoragedetailrsp_usr_value()->mutable_storagedetail()->set_strobjectid(m_storageDetail.m_strObjectID);
    InteractiveMsg.mutable_rspvalue()->mutable_querystoragedetailrsp_usr_value()->mutable_storagedetail()->set_uiobjecttype(m_storageDetail.m_uiObjectType);
    InteractiveMsg.mutable_rspvalue()->mutable_querystoragedetailrsp_usr_value()->mutable_storagedetail()->set_strstoragename(m_storageDetail.m_strStorageName);
    InteractiveMsg.mutable_rspvalue()->mutable_querystoragedetailrsp_usr_value()->mutable_storagedetail()->set_uistoragetype(m_storageDetail.m_uiStorageType);
    InteractiveMsg.mutable_rspvalue()->mutable_querystoragedetailrsp_usr_value()->mutable_storagedetail()->set_uioverlaptype(m_storageDetail.m_uiOverlapType);
    InteractiveMsg.mutable_rspvalue()->mutable_querystoragedetailrsp_usr_value()->mutable_storagedetail()->set_uistoragetimeuplimit(m_storageDetail.m_uiStorageTimeUpLimit);
    InteractiveMsg.mutable_rspvalue()->mutable_querystoragedetailrsp_usr_value()->mutable_storagedetail()->set_uistoragetimedownlimit(m_storageDetail.m_uiStorageTimeDownLimit);
    InteractiveMsg.mutable_rspvalue()->mutable_querystoragedetailrsp_usr_value()->mutable_storagedetail()->set_uisizeofspaceused(m_storageDetail.m_uiSizeOfSpaceUsed);
    InteractiveMsg.mutable_rspvalue()->mutable_querystoragedetailrsp_usr_value()->mutable_storagedetail()->set_uistorageunittype(m_storageDetail.m_uiStorageUnitType);
    InteractiveMsg.mutable_rspvalue()->mutable_querystoragedetailrsp_usr_value()->mutable_storagedetail()->set_strbegindate(m_storageDetail.m_strBeginDate);
    InteractiveMsg.mutable_rspvalue()->mutable_querystoragedetailrsp_usr_value()->mutable_storagedetail()->set_strenddate(m_storageDetail.m_strEndDate);
    InteractiveMsg.mutable_rspvalue()->mutable_querystoragedetailrsp_usr_value()->mutable_storagedetail()->set_uistatus(m_storageDetail.m_uiStatus);
    InteractiveMsg.mutable_rspvalue()->mutable_querystoragedetailrsp_usr_value()->mutable_storagedetail()->set_strextend(m_storageDetail.m_strExtend);
}

void InteractiveProtoHandler::QueryRegionStorageInfoReq_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Req::UnSerializer(InteractiveMsg);
    m_strUserID = InteractiveMsg.reqvalue().queryregionstorageinforeq_usr_value().struserid();
}

void InteractiveProtoHandler::QueryRegionStorageInfoReq_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Req::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryRegionStorageInfoReq_USR_T);
    InteractiveMsg.mutable_reqvalue()->mutable_queryregionstorageinforeq_usr_value()->set_struserid(m_strUserID);
}

void InteractiveProtoHandler::QueryRegionStorageInfoRsp_USR::UnSerializer(const InteractiveMessage &InteractiveMsg)
{
    Rsp::UnSerializer(InteractiveMsg);
    m_uiDomainID = InteractiveMsg.rspvalue().queryregionstorageinforsp_usr_value().uidomainid();
    m_uiSizeOfSpace = InteractiveMsg.rspvalue().queryregionstorageinforsp_usr_value().uisizeofspace();
    m_uiSizeOfSpaceUsed = InteractiveMsg.rspvalue().queryregionstorageinforsp_usr_value().uisizeofspaceused();
}

void InteractiveProtoHandler::QueryRegionStorageInfoRsp_USR::Serializer(InteractiveMessage &InteractiveMsg) const
{
    Rsp::Serializer(InteractiveMsg);
    InteractiveMsg.set_type(Interactive::Message::MsgType::QueryRegionStorageInfoRsp_USR_T);
    InteractiveMsg.mutable_rspvalue()->mutable_queryregionstorageinforsp_usr_value()->set_uidomainid(m_uiDomainID);
    InteractiveMsg.mutable_rspvalue()->mutable_queryregionstorageinforsp_usr_value()->set_uisizeofspace(m_uiSizeOfSpace);
    InteractiveMsg.mutable_rspvalue()->mutable_queryregionstorageinforsp_usr_value()->set_uisizeofspaceused(m_uiSizeOfSpaceUsed);
}

