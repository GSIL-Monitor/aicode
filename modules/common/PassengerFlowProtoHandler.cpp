#include "PassengerFlowProtoHandler.h"
#include "InteractiveProtocolCustomerFlow.pb.h"
#include <boost/bind.hpp>

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

    CustomerFlowMessage Msg;
    Msg.Clear();

    pReq->Serializer(Msg);

    Msg.SerializeToString(&strOutput);

    return true;
};

template<typename T, typename TT> bool UnSerializerT(const CustomerFlowMessage &message, TT &req)
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

    pReq->UnSerializer(message);

    return true;
}

void SerializeGroupList(const std::list<PassengerFlowProtoHandler::Group> &groupInfoList,
    ::google::protobuf::RepeatedPtrField<::CustomerFlow::Interactive::Message::Group> *pDstGroupInfoList)
{
    for (auto itBegin = groupInfoList.begin(), itEnd = groupInfoList.end(); itBegin != itEnd; ++itBegin)
    {
        auto pDstGroupInfo = pDstGroupInfoList->Add();
        pDstGroupInfo->set_strgroupid(itBegin->m_strGroupID);
        pDstGroupInfo->set_strgroupname(itBegin->m_strGroupName);
        pDstGroupInfo->set_strcreatedate(itBegin->m_strCreateDate);
        pDstGroupInfo->set_uistate(itBegin->m_uiState);
    }
}

void UnSerializeGroupList(std::list<PassengerFlowProtoHandler::Group> &groupInfoList,
    const ::google::protobuf::RepeatedPtrField<::CustomerFlow::Interactive::Message::Group> &srcGroupInfoList)
{
    for (int i = 0, sz = srcGroupInfoList.size(); i < sz; ++i)
    {
        auto srcGroupInfo = srcGroupInfoList.Get(i);
        PassengerFlowProtoHandler::Group groupInfo;
        groupInfo.m_strGroupID = srcGroupInfo.strgroupid();
        groupInfo.m_strGroupName = srcGroupInfo.strgroupname();
        groupInfo.m_strCreateDate = srcGroupInfo.strcreatedate();
        groupInfo.m_uiState = srcGroupInfo.uistate();

        groupInfoList.push_back(std::move(groupInfo));
    }
}

void SerializeEntranceList(const std::list<PassengerFlowProtoHandler::Entrance> &entranceInfoList,
    ::google::protobuf::RepeatedPtrField<::CustomerFlow::Interactive::Message::Entrance> *pDstEntranceInfoList)
{
    for (auto itBegin = entranceInfoList.begin(), itEnd = entranceInfoList.end(); itBegin != itEnd; ++itBegin)
    {
        auto pDstEntranceInfo = pDstEntranceInfoList->Add();
        pDstEntranceInfo->set_strentranceid(itBegin->m_strEntranceID);
        pDstEntranceInfo->set_strentrancename(itBegin->m_strEntranceName);

        for (auto it = itBegin->m_strDeviceIDList.begin(), end = itBegin->m_strDeviceIDList.end(); it != end; ++it)
        {
            pDstEntranceInfo->add_strdeviceid(*it);
        }
    }
}

void UnSerializeEntranceList(std::list<PassengerFlowProtoHandler::Entrance> &entranceInfoList,
    const ::google::protobuf::RepeatedPtrField<::CustomerFlow::Interactive::Message::Entrance> &srcEntranceInfoList)
{
    for (int i = 0, sz = srcEntranceInfoList.size(); i < sz; ++i)
    {
        auto srcEntranceInfo = srcEntranceInfoList.Get(i);
        PassengerFlowProtoHandler::Entrance entranceInfo;
        entranceInfo.m_strEntranceID = srcEntranceInfo.strentranceid();
        entranceInfo.m_strEntranceName = srcEntranceInfo.strentrancename();

        for (int j = 0, sz2 = srcEntranceInfo.strdeviceid_size(); j < sz2; ++j)
        {
            entranceInfo.m_strDeviceIDList.push_back(srcEntranceInfo.strdeviceid(j));
        }

        entranceInfoList.push_back(std::move(entranceInfo));
    }
}

void SerializeStoreList(const std::list<PassengerFlowProtoHandler::Store> &storeInfoList,
    ::google::protobuf::RepeatedPtrField<::CustomerFlow::Interactive::Message::Store> *pDstStoreInfoList)
{
    for (auto itBegin = storeInfoList.begin(), itEnd = storeInfoList.end(); itBegin != itEnd; ++itBegin)
    {
        auto pDstStoreInfo = pDstStoreInfoList->Add();
        pDstStoreInfo->set_strstoreid(itBegin->m_strStoreID);
        pDstStoreInfo->set_strstorename(itBegin->m_strStoreName);
        pDstStoreInfo->set_strgoodscategory(itBegin->m_strGoodsCategory);
        pDstStoreInfo->set_straddress(itBegin->m_strAddress);
        pDstStoreInfo->set_uiopenstate(itBegin->m_uiOpenState);
        pDstStoreInfo->set_strcreatedate(itBegin->m_strCreateDate);
        pDstStoreInfo->set_strextend(itBegin->m_strExtend);
        pDstStoreInfo->set_uistate(itBegin->m_uiState);

        auto area = pDstStoreInfo->mutable_area();
        area->set_strareaid(itBegin->m_area.m_strAreaID);
        area->set_strareaname(itBegin->m_area.m_strAreaName);

        SerializeEntranceList(itBegin->m_entranceList, pDstStoreInfo->mutable_entrance());
    }
}

void UnSerializeStoreList(std::list<PassengerFlowProtoHandler::Store> &storeInfoList,
    const ::google::protobuf::RepeatedPtrField<::CustomerFlow::Interactive::Message::Store> &srcStoreInfoList)
{
    for (int i = 0, sz = srcStoreInfoList.size(); i < sz; ++i)
    {
        auto srcStoreInfo = srcStoreInfoList.Get(i);
        PassengerFlowProtoHandler::Store storeInfo;
        storeInfo.m_strStoreID = srcStoreInfo.strstoreid();
        storeInfo.m_strStoreName = srcStoreInfo.strstorename();
        storeInfo.m_strGoodsCategory = srcStoreInfo.strgoodscategory();
        storeInfo.m_strAddress = srcStoreInfo.straddress();
        storeInfo.m_uiOpenState = srcStoreInfo.uiopenstate();
        storeInfo.m_strCreateDate = srcStoreInfo.strcreatedate();
        storeInfo.m_strExtend = srcStoreInfo.strextend();
        storeInfo.m_uiState = srcStoreInfo.uistate();

        auto area = srcStoreInfo.area();
        storeInfo.m_area.m_strAreaID = area.strareaid();
        storeInfo.m_area.m_strAreaName = area.strareaname();

        UnSerializeEntranceList(storeInfo.m_entranceList, srcStoreInfo.entrance());

        storeInfoList.push_back(std::move(storeInfo));
    }
}

void SerializePatrolStoreEntranceList(const std::list<PassengerFlowProtoHandler::PatrolStoreEntrance> &storeEntranceList,
    ::google::protobuf::RepeatedPtrField<::CustomerFlow::Interactive::Message::PatrolStoreEntrance> *pDstStoreEntranceList)
{
    for (auto itBegin = storeEntranceList.begin(), itEnd = storeEntranceList.end(); itBegin != itEnd; ++itBegin)
    {
        auto pDstPatrolStoreEntrance = pDstStoreEntranceList->Add();
        pDstPatrolStoreEntrance->set_strstoreid(itBegin->m_strStoreID);
        pDstPatrolStoreEntrance->set_strstorename(itBegin->m_strStoreName);

        for (auto it = itBegin->m_entranceList.begin(), end = itBegin->m_entranceList.end(); it != end; ++it)
        {
            auto entrance = pDstPatrolStoreEntrance->add_entrance();
            entrance->set_strentranceid(it->m_strEntranceID);
            entrance->set_strentrancename(it->m_strEntranceName);
        }
    }
}

void UnSerializePatrolStoreEntranceList(std::list<PassengerFlowProtoHandler::PatrolStoreEntrance> &storeEntranceList,
    const ::google::protobuf::RepeatedPtrField<::CustomerFlow::Interactive::Message::PatrolStoreEntrance> &srcStoreEntranceList)
{
    for (int i = 0, sz = srcStoreEntranceList.size(); i < sz; ++i)
    {
        auto srcPatrolStoreEntrance = srcStoreEntranceList.Get(i);
        PassengerFlowProtoHandler::PatrolStoreEntrance storeEntrance;
        storeEntrance.m_strStoreID = srcPatrolStoreEntrance.strstoreid();
        storeEntrance.m_strStoreName = srcPatrolStoreEntrance.strstorename();

        for (int j = 0, sz2 = srcPatrolStoreEntrance.entrance_size(); j < sz2; ++j)
        {
            auto srcEntrance = srcPatrolStoreEntrance.entrance(j);
            PassengerFlowProtoHandler::Entrance entrance;
            entrance.m_strEntranceID = srcEntrance.strentranceid();
            entrance.m_strEntranceName = srcEntrance.strentrancename();
            storeEntrance.m_entranceList.push_back(entrance);
        }

        storeEntranceList.push_back(std::move(storeEntrance));
    }
}

void SerializeRawCustomerFlowList(const std::list<PassengerFlowProtoHandler::RawCustomerFlow> &rawCustomerFlowInfoList,
    ::google::protobuf::RepeatedPtrField<::CustomerFlow::Interactive::Message::RawCustomerFlow> *pDstRawCustomerFlowInfoList)
{
    for (auto itBegin = rawCustomerFlowInfoList.begin(), itEnd = rawCustomerFlowInfoList.end(); itBegin != itEnd; ++itBegin)
    {
        auto pDstRawCustomerFlowInfo = pDstRawCustomerFlowInfoList->Add();
        pDstRawCustomerFlowInfo->set_uienternumber(itBegin->m_uiEnterNumber);
        pDstRawCustomerFlowInfo->set_uileavenumber(itBegin->m_uiLeaveNumber);
        pDstRawCustomerFlowInfo->set_uistaynumber(itBegin->m_uiStayNumber);
        pDstRawCustomerFlowInfo->set_strdatatime(itBegin->m_strDataTime);
    }
}

void UnSerializeRawCustomerFlowList(std::list<PassengerFlowProtoHandler::RawCustomerFlow> &rawCustomerFlowInfoList,
    const ::google::protobuf::RepeatedPtrField<::CustomerFlow::Interactive::Message::RawCustomerFlow> &srcRawCustomerFlowInfoList)
{
    for (int i = 0, sz = srcRawCustomerFlowInfoList.size(); i < sz; ++i)
    {
        auto srcRawCustomerFlowInfo = srcRawCustomerFlowInfoList.Get(i);
        PassengerFlowProtoHandler::RawCustomerFlow rawCustomerFlowInfo;
        rawCustomerFlowInfo.m_uiEnterNumber = srcRawCustomerFlowInfo.uienternumber();
        rawCustomerFlowInfo.m_uiLeaveNumber = srcRawCustomerFlowInfo.uileavenumber();
        rawCustomerFlowInfo.m_uiStayNumber = srcRawCustomerFlowInfo.uistaynumber();
        rawCustomerFlowInfo.m_strDataTime = srcRawCustomerFlowInfo.strdatatime();

        rawCustomerFlowInfoList.push_back(std::move(rawCustomerFlowInfo));
    }
}

void SerializeVIPConsumeHistoryList(const std::list<PassengerFlowProtoHandler::VIPConsumeHistory> &consumeHistoryList,
    ::google::protobuf::RepeatedPtrField<::CustomerFlow::Interactive::Message::VIPConsumeHistory> *pDstConsumeHistoryList)
{
    for (auto itBegin = consumeHistoryList.begin(), itEnd = consumeHistoryList.end(); itBegin != itEnd; ++itBegin)
    {
        auto pDstConsumeHistory = pDstConsumeHistoryList->Add();
        pDstConsumeHistory->set_strconsumeid(itBegin->m_strConsumeID);
        pDstConsumeHistory->set_strvipid(itBegin->m_strVIPID);
        pDstConsumeHistory->set_strgoodsname(itBegin->m_strGoodsName);
        pDstConsumeHistory->set_uigoodsnumber(itBegin->m_uiGoodsNumber);
        pDstConsumeHistory->set_strsalesman(itBegin->m_strSalesman);
        pDstConsumeHistory->set_dconsumeamount(itBegin->m_dConsumeAmount);
        pDstConsumeHistory->set_strconsumedate(itBegin->m_strConsumeDate);
    }
}

void UnSerializeVIPConsumeHistoryList(std::list<PassengerFlowProtoHandler::VIPConsumeHistory> &consumeHistoryList,
    const ::google::protobuf::RepeatedPtrField<::CustomerFlow::Interactive::Message::VIPConsumeHistory> &srcConsumeHistoryList)
{
    for (int i = 0, sz = srcConsumeHistoryList.size(); i < sz; ++i)
    {
        auto srcConsumeHistory = srcConsumeHistoryList.Get(i);
        PassengerFlowProtoHandler::VIPConsumeHistory consumeHistory;
        consumeHistory.m_strConsumeID = srcConsumeHistory.strconsumeid();
        consumeHistory.m_strVIPID = srcConsumeHistory.strvipid();
        consumeHistory.m_strGoodsName = srcConsumeHistory.strgoodsname();
        consumeHistory.m_uiGoodsNumber = srcConsumeHistory.uigoodsnumber();
        consumeHistory.m_strSalesman = srcConsumeHistory.strsalesman();
        consumeHistory.m_dConsumeAmount = srcConsumeHistory.dconsumeamount();
        consumeHistory.m_strConsumeDate = srcConsumeHistory.strconsumedate();

        consumeHistoryList.push_back(consumeHistory);
    }
}

void SerializeVIPCustomerList(const std::list<PassengerFlowProtoHandler::VIPCustomer> &customerList,
    ::google::protobuf::RepeatedPtrField<::CustomerFlow::Interactive::Message::VIPCustomer> *pDstCustomerList)
{
    for (auto itBegin = customerList.begin(), itEnd = customerList.end(); itBegin != itEnd; ++itBegin)
    {
        auto pDstVIPCustomerInfo = pDstCustomerList->Add();
        pDstVIPCustomerInfo->set_strvipid(itBegin->m_strVIPID);
        pDstVIPCustomerInfo->set_strprofilepicture(itBegin->m_strProfilePicture);
        pDstVIPCustomerInfo->set_strvipname(itBegin->m_strVIPName);
        pDstVIPCustomerInfo->set_strcellphone(itBegin->m_strCellphone);
        pDstVIPCustomerInfo->set_strvisitdate(itBegin->m_strVisitDate);
        pDstVIPCustomerInfo->set_uivisittimes(itBegin->m_uiVisitTimes);
        pDstVIPCustomerInfo->set_strregisterdate(itBegin->m_strRegisterDate);
        pDstVIPCustomerInfo->set_uistate(itBegin->m_uiState);

        SerializeVIPConsumeHistoryList(itBegin->m_consumeHistoryList, pDstVIPCustomerInfo->mutable_consumehistory());
    }
}

void UnSerializeVIPCustomerList(std::list<PassengerFlowProtoHandler::VIPCustomer> &customerList,
    const ::google::protobuf::RepeatedPtrField<::CustomerFlow::Interactive::Message::VIPCustomer> &srcCustomerList)
{
    for (int i = 0, sz = srcCustomerList.size(); i < sz; ++i)
    {
        auto srcVIPCustomerInfo = srcCustomerList.Get(i);
        PassengerFlowProtoHandler::VIPCustomer storeInfo;
        storeInfo.m_strVIPID = srcVIPCustomerInfo.strvipid();
        storeInfo.m_strProfilePicture = srcVIPCustomerInfo.strprofilepicture();
        storeInfo.m_strVIPName = srcVIPCustomerInfo.strvipname();
        storeInfo.m_strCellphone = srcVIPCustomerInfo.strcellphone();
        storeInfo.m_strVisitDate = srcVIPCustomerInfo.strvisitdate();
        storeInfo.m_uiVisitTimes = srcVIPCustomerInfo.uivisittimes();
        storeInfo.m_strRegisterDate = srcVIPCustomerInfo.strregisterdate();
        storeInfo.m_uiState = srcVIPCustomerInfo.uistate();

        UnSerializeVIPConsumeHistoryList(storeInfo.m_consumeHistoryList, srcVIPCustomerInfo.consumehistory());

        customerList.push_back(std::move(storeInfo));
    }
}

void SerializeEvaluationTemplateList(const std::list<PassengerFlowProtoHandler::EvaluationItem> &itemList,
    ::google::protobuf::RepeatedPtrField<::CustomerFlow::Interactive::Message::EvaluationItem> *pDstItemList)
{
    for (auto itBegin = itemList.begin(), itEnd = itemList.end(); itBegin != itEnd; ++itBegin)
    {
        auto pDstEvaluationTemplateInfo = pDstItemList->Add();
        pDstEvaluationTemplateInfo->set_stritemid(itBegin->m_strItemID);
        pDstEvaluationTemplateInfo->set_stritemname(itBegin->m_strItemName);
        pDstEvaluationTemplateInfo->set_strdescription(itBegin->m_strDescription);
        pDstEvaluationTemplateInfo->set_dtotalpoint(itBegin->m_dTotalPoint);
    }
}

void UnSerializeEvaluationTemplateList(std::list<PassengerFlowProtoHandler::EvaluationItem> &itemList,
    const ::google::protobuf::RepeatedPtrField<::CustomerFlow::Interactive::Message::EvaluationItem> &srcItemList)
{
    for (int i = 0, sz = srcItemList.size(); i < sz; ++i)
    {
        auto srcEvaluationTemplateInfo = srcItemList.Get(i);
        PassengerFlowProtoHandler::EvaluationItem evaluationItem;
        evaluationItem.m_strItemID = srcEvaluationTemplateInfo.stritemid();
        evaluationItem.m_strItemName = srcEvaluationTemplateInfo.stritemname();
        evaluationItem.m_strDescription = srcEvaluationTemplateInfo.strdescription();
        evaluationItem.m_dTotalPoint = srcEvaluationTemplateInfo.dtotalpoint();

        itemList.push_back(std::move(evaluationItem));
    }
}


PassengerFlowProtoHandler::PassengerFlowProtoHandler()
{
    SerializeHandler handler;

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::CustomerFlowPreHandleReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::CustomerFlowPreHandleReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::CustomerFlowPreHandleReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::CustomerFlowPreHandleRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::CustomerFlowPreHandleRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::CustomerFlowPreHandleRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ShakehandReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ShakehandReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ShakehandReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ShakehandRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ShakehandRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ShakehandRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddAreaReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddAreaReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddAreaReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddAreaRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddAreaRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddAreaRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteAreaReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteAreaReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteAreaReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteAreaRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteAreaRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteAreaRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyAreaReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyAreaReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyAreaReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyAreaRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyAreaRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyAreaRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAreaInfoReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAreaInfoReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAreaInfoReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAreaInfoRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAreaInfoRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAreaInfoRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllAreaReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllAreaReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllAreaReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllAreaRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllAreaRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllAreaRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::BindPushClientIDReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::BindPushClientIDReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::BindPushClientIDReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::BindPushClientIDRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::BindPushClientIDRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::BindPushClientIDRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::UnbindPushClientIDReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::UnbindPushClientIDReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::UnbindPushClientIDReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::UnbindPushClientIDRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::UnbindPushClientIDRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::UnbindPushClientIDRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddGroupReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddGroupReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddGroupReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddGroupRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddGroupRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddGroupRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteGroupReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteGroupReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteGroupReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteGroupRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteGroupRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteGroupRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyGroupReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyGroupReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyGroupReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyGroupRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyGroupRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyGroupRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryGroupInfoReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryGroupInfoReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryGroupInfoReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryGroupInfoRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryGroupInfoRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryGroupInfoRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllGroupReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllGroupReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllGroupReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllGroupRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllGroupRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllGroupRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddStoreReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddStoreReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddStoreReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddStoreRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddStoreRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddStoreRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteStoreReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteStoreReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteStoreReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteStoreRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteStoreRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteStoreRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyStoreReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyStoreReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyStoreReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyStoreRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyStoreRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyStoreRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryStoreInfoReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryStoreInfoReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryStoreInfoReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryStoreInfoRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryStoreInfoRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryStoreInfoRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllStoreReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllStoreReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllStoreReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllStoreRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllStoreRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllStoreRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddEntranceReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddEntranceReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddEntranceReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddEntranceRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddEntranceRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddEntranceRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteEntranceReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteEntranceReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteEntranceReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteEntranceRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteEntranceRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteEntranceRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyEntranceReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyEntranceReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyEntranceReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyEntranceRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyEntranceRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyEntranceRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddEntranceDeviceReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddEntranceDeviceReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddEntranceDeviceReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddEntranceDeviceRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddEntranceDeviceRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddEntranceDeviceRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteEntranceDeviceReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteEntranceDeviceReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteEntranceDeviceReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteEntranceDeviceRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteEntranceDeviceRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteEntranceDeviceRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddEventReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddEventReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddEventReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddEventRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddEventRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddEventRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteEventReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteEventReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteEventReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteEventRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteEventRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteEventRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyEventReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyEventReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyEventReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyEventRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyEventRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyEventRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryEventInfoReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryEventInfoReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryEventInfoReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryEventInfoRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryEventInfoRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryEventInfoRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllEventReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllEventReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllEventReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllEventRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllEventRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllEventRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddSmartGuardStoreReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddSmartGuardStoreReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddSmartGuardStoreReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddSmartGuardStoreRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddSmartGuardStoreRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddSmartGuardStoreRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteSmartGuardStoreReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteSmartGuardStoreReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteSmartGuardStoreReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteSmartGuardStoreRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteSmartGuardStoreRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteSmartGuardStoreRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifySmartGuardStoreReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifySmartGuardStoreReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifySmartGuardStoreReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifySmartGuardStoreRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifySmartGuardStoreRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifySmartGuardStoreRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QuerySmartGuardStoreInfoReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QuerySmartGuardStoreInfoReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QuerySmartGuardStoreInfoReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QuerySmartGuardStoreInfoRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QuerySmartGuardStoreInfoRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QuerySmartGuardStoreInfoRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllSmartGuardStoreReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllSmartGuardStoreReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllSmartGuardStoreReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllSmartGuardStoreRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllSmartGuardStoreRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllSmartGuardStoreRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddRegularPatrolReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddRegularPatrolReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddRegularPatrolReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddRegularPatrolRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddRegularPatrolRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddRegularPatrolRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteRegularPatrolReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteRegularPatrolReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteRegularPatrolReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteRegularPatrolRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteRegularPatrolRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteRegularPatrolRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyRegularPatrolReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyRegularPatrolReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyRegularPatrolReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyRegularPatrolRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyRegularPatrolRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyRegularPatrolRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryRegularPatrolInfoReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryRegularPatrolInfoReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryRegularPatrolInfoReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryRegularPatrolInfoRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryRegularPatrolInfoRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryRegularPatrolInfoRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllRegularPatrolReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllRegularPatrolReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllRegularPatrolReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllRegularPatrolRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllRegularPatrolRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllRegularPatrolRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::UserJoinStoreReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::UserJoinStoreReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::UserJoinStoreReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::UserJoinStoreRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::UserJoinStoreRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::UserJoinStoreRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::UserQuitStoreReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::UserQuitStoreReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::UserQuitStoreReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::UserQuitStoreRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::UserQuitStoreRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::UserQuitStoreRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryStoreAllUserReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryStoreAllUserReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryStoreAllUserReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryStoreAllUserRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryStoreAllUserRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryStoreAllUserRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddVIPCustomerReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddVIPCustomerReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddVIPCustomerReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddVIPCustomerRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddVIPCustomerRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddVIPCustomerRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteVIPCustomerReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteVIPCustomerReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteVIPCustomerReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteVIPCustomerRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteVIPCustomerRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteVIPCustomerRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyVIPCustomerReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyVIPCustomerReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyVIPCustomerReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyVIPCustomerRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyVIPCustomerRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyVIPCustomerRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryVIPCustomerInfoReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryVIPCustomerInfoReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryVIPCustomerInfoReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryVIPCustomerInfoRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryVIPCustomerInfoRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryVIPCustomerInfoRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllVIPCustomerReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllVIPCustomerReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllVIPCustomerReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllVIPCustomerRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllVIPCustomerRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllVIPCustomerRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddVIPConsumeHistoryReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddVIPConsumeHistoryReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddVIPConsumeHistoryReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddVIPConsumeHistoryRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddVIPConsumeHistoryRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddVIPConsumeHistoryRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteVIPConsumeHistoryReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteVIPConsumeHistoryReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteVIPConsumeHistoryReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteVIPConsumeHistoryRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteVIPConsumeHistoryRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteVIPConsumeHistoryRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyVIPConsumeHistoryReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyVIPConsumeHistoryReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyVIPConsumeHistoryReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyVIPConsumeHistoryRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyVIPConsumeHistoryRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyVIPConsumeHistoryRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllVIPConsumeHistoryReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllVIPConsumeHistoryReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllVIPConsumeHistoryReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllVIPConsumeHistoryRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllVIPConsumeHistoryRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllVIPConsumeHistoryRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddEvaluationTemplateReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddEvaluationTemplateReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddEvaluationTemplateReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddEvaluationTemplateRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddEvaluationTemplateRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddEvaluationTemplateRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteEvaluationTemplateReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteEvaluationTemplateReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteEvaluationTemplateReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteEvaluationTemplateRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteEvaluationTemplateRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteEvaluationTemplateRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyEvaluationTemplateReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyEvaluationTemplateReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyEvaluationTemplateReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyEvaluationTemplateRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyEvaluationTemplateRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyEvaluationTemplateRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllEvaluationTemplateReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllEvaluationTemplateReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllEvaluationTemplateReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllEvaluationTemplateRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllEvaluationTemplateRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllEvaluationTemplateRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddStoreEvaluationReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddStoreEvaluationReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddStoreEvaluationReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddStoreEvaluationRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddStoreEvaluationRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddStoreEvaluationRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteStoreEvaluationReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteStoreEvaluationReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteStoreEvaluationReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteStoreEvaluationRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteStoreEvaluationRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteStoreEvaluationRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyStoreEvaluationReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyStoreEvaluationReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyStoreEvaluationReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyStoreEvaluationRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyStoreEvaluationRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyStoreEvaluationRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryStoreEvaluationInfoReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryStoreEvaluationInfoReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryStoreEvaluationInfoReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryStoreEvaluationInfoRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryStoreEvaluationInfoRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryStoreEvaluationInfoRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllStoreEvaluationReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllStoreEvaluationReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllStoreEvaluationReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllStoreEvaluationRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllStoreEvaluationRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllStoreEvaluationRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddRemotePatrolStoreReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddRemotePatrolStoreReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddRemotePatrolStoreReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::AddRemotePatrolStoreRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::AddRemotePatrolStoreRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddRemotePatrolStoreRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteRemotePatrolStoreReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteRemotePatrolStoreReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteRemotePatrolStoreReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::DeleteRemotePatrolStoreRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::DeleteRemotePatrolStoreRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteRemotePatrolStoreRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyRemotePatrolStoreReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyRemotePatrolStoreReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyRemotePatrolStoreReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ModifyRemotePatrolStoreRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ModifyRemotePatrolStoreRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyRemotePatrolStoreRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryRemotePatrolStoreInfoReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryRemotePatrolStoreInfoReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryRemotePatrolStoreInfoReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryRemotePatrolStoreInfoRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryRemotePatrolStoreInfoRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryRemotePatrolStoreInfoRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllRemotePatrolStoreReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllRemotePatrolStoreReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllRemotePatrolStoreReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryAllRemotePatrolStoreRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryAllRemotePatrolStoreRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllRemotePatrolStoreRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ImportPOSDataReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ImportPOSDataReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ImportPOSDataReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ImportPOSDataRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ImportPOSDataRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ImportPOSDataRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryCustomerFlowStatisticReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryCustomerFlowStatisticReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryCustomerFlowStatisticReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::QueryCustomerFlowStatisticRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::QueryCustomerFlowStatisticRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryCustomerFlowStatisticRsp_T, handler));

    /////////////////////////////////////////////////////

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ReportCustomerFlowDataReq_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ReportCustomerFlowDataReq_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ReportCustomerFlowDataReq_T, handler));

    handler.Szr = boost::bind(&PassengerFlowProtoHandler::ReportCustomerFlowDataRsp_Serializer, this, _1, _2);
    handler.UnSzr = boost::bind(&PassengerFlowProtoHandler::ReportCustomerFlowDataRsp_UnSerializer, this, _1, _2);
    m_ReqAndRspHandlerMap.insert(std::make_pair(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ReportCustomerFlowDataRsp_T, handler));
}

PassengerFlowProtoHandler::~PassengerFlowProtoHandler()
{

}

bool PassengerFlowProtoHandler::GetCustomerFlowMsgType(const std::string &strData, CustomerFlowMsgType &msgtype)
{
    CustomerFlowMessage Msg;
    Msg.Clear();
    if (!Msg.ParseFromString(strData))
    {
        return false;
    }

    msgtype = (CustomerFlowMsgType)Msg.type();

    return true;
}

bool PassengerFlowProtoHandler::SerializeReq(const Request &req, std::string &strOutput)
{
    const int iType = req.m_MsgType;

    auto itFind = m_ReqAndRspHandlerMap.find(iType);
    if (m_ReqAndRspHandlerMap.end() == itFind)
    {
        return false;
    }

    return itFind->second.Szr(req, strOutput);
}

bool PassengerFlowProtoHandler::UnSerializeReq(const std::string &strData, Request &req)
{
    CustomerFlowMessage Msg;
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

bool PassengerFlowProtoHandler::SerializeRsp(const Response &rsp, std::string &strOutput)
{
    return SerializeReq(rsp, strOutput);
}

bool PassengerFlowProtoHandler::UnSerializeRsp(const std::string &strData, Response &rsp)
{
    return UnSerializeReq(strData, rsp);
}

bool PassengerFlowProtoHandler::UnSerializeReqBase(const std::string &strData, Request &req)
{
    CustomerFlowMessage Msg;
    Msg.Clear();
    if (!Msg.ParseFromString(strData))
    {
        return false;
    }

    req.UnSerializer(Msg);

    return true;
}

bool PassengerFlowProtoHandler::CustomerFlowPreHandleReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<CustomerFlowPreHandleReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::CustomerFlowPreHandleReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<CustomerFlowPreHandleReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::CustomerFlowPreHandleRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<CustomerFlowPreHandleRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::CustomerFlowPreHandleRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<CustomerFlowPreHandleRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::ShakehandReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<ShakehandReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::ShakehandReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<ShakehandReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::ShakehandRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<ShakehandRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::ShakehandRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<ShakehandRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::AddAreaReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<AddAreaReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::AddAreaReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<AddAreaReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::AddAreaRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<AddAreaRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::AddAreaRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<AddAreaRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::DeleteAreaReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<DeleteAreaReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::DeleteAreaReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<DeleteAreaReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::DeleteAreaRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<DeleteAreaRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::DeleteAreaRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<DeleteAreaRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::ModifyAreaReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<ModifyAreaReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::ModifyAreaReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<ModifyAreaReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::ModifyAreaRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<ModifyAreaRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::ModifyAreaRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<ModifyAreaRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryAreaInfoReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryAreaInfoReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryAreaInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryAreaInfoReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryAreaInfoRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryAreaInfoRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryAreaInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryAreaInfoRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryAllAreaReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryAllAreaReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllAreaReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryAllAreaReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryAllAreaRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryAllAreaRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllAreaRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryAllAreaRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::BindPushClientIDReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<BindPushClientIDReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::BindPushClientIDReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<BindPushClientIDReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::BindPushClientIDRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<BindPushClientIDRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::BindPushClientIDRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<BindPushClientIDRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::UnbindPushClientIDReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<UnbindPushClientIDReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::UnbindPushClientIDReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<UnbindPushClientIDReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::UnbindPushClientIDRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<UnbindPushClientIDRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::UnbindPushClientIDRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<UnbindPushClientIDRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::AddGroupReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<AddGroupReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::AddGroupReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<AddGroupReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::AddGroupRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<AddGroupRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::AddGroupRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<AddGroupRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::DeleteGroupReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<DeleteGroupReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::DeleteGroupReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<DeleteGroupReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::DeleteGroupRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<DeleteGroupRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::DeleteGroupRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<DeleteGroupRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::ModifyGroupReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<ModifyGroupReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::ModifyGroupReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<ModifyGroupReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::ModifyGroupRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<ModifyGroupRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::ModifyGroupRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<ModifyGroupRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryGroupInfoReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryGroupInfoReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryGroupInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryGroupInfoReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryGroupInfoRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryGroupInfoRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryGroupInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryGroupInfoRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryAllGroupReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryAllGroupReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllGroupReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryAllGroupReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryAllGroupRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryAllGroupRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllGroupRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryAllGroupRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::AddStoreReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<AddStoreReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::AddStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<AddStoreReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::AddStoreRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<AddStoreRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::AddStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<AddStoreRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::DeleteStoreReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<DeleteStoreReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::DeleteStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<DeleteStoreReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::DeleteStoreRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<DeleteStoreRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::DeleteStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<DeleteStoreRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::ModifyStoreReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<ModifyStoreReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::ModifyStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<ModifyStoreReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::ModifyStoreRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<ModifyStoreRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::ModifyStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<ModifyStoreRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryStoreInfoReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryStoreInfoReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryStoreInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryStoreInfoReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryStoreInfoRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryStoreInfoRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryStoreInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryStoreInfoRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryAllStoreReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryAllStoreReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryAllStoreReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryAllStoreRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryAllStoreRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryAllStoreRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::AddEntranceReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<AddEntranceReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::AddEntranceReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<AddEntranceReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::AddEntranceRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<AddEntranceRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::AddEntranceRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<AddEntranceRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::DeleteEntranceReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<DeleteEntranceReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::DeleteEntranceReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<DeleteEntranceReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::DeleteEntranceRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<DeleteEntranceRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::DeleteEntranceRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<DeleteEntranceRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::ModifyEntranceReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<ModifyEntranceReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::ModifyEntranceReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<ModifyEntranceReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::ModifyEntranceRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<ModifyEntranceRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::ModifyEntranceRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<ModifyEntranceRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::AddEntranceDeviceReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<AddEntranceDeviceReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::AddEntranceDeviceReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<AddEntranceDeviceReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::AddEntranceDeviceRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<AddEntranceDeviceRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::AddEntranceDeviceRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<AddEntranceDeviceRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::DeleteEntranceDeviceReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<DeleteEntranceDeviceReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::DeleteEntranceDeviceReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<DeleteEntranceDeviceReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::DeleteEntranceDeviceRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<DeleteEntranceDeviceRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::DeleteEntranceDeviceRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<DeleteEntranceDeviceRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::AddEventReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<AddEventReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::AddEventReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<AddEventReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::AddEventRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<AddEventRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::AddEventRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<AddEventRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::DeleteEventReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<DeleteEventReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::DeleteEventReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<DeleteEventReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::DeleteEventRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<DeleteEventRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::DeleteEventRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<DeleteEventRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::ModifyEventReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<ModifyEventReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::ModifyEventReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<ModifyEventReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::ModifyEventRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<ModifyEventRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::ModifyEventRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<ModifyEventRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryEventInfoReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryEventInfoReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryEventInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryEventInfoReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryEventInfoRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryEventInfoRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryEventInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryEventInfoRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryAllEventReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryAllEventReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllEventReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryAllEventReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryAllEventRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryAllEventRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllEventRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryAllEventRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::AddSmartGuardStoreReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<AddSmartGuardStoreReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::AddSmartGuardStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<AddSmartGuardStoreReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::AddSmartGuardStoreRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<AddSmartGuardStoreRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::AddSmartGuardStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<AddSmartGuardStoreRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::DeleteSmartGuardStoreReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<DeleteSmartGuardStoreReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::DeleteSmartGuardStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<DeleteSmartGuardStoreReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::DeleteSmartGuardStoreRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<DeleteSmartGuardStoreRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::DeleteSmartGuardStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<DeleteSmartGuardStoreRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::ModifySmartGuardStoreReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<ModifySmartGuardStoreReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::ModifySmartGuardStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<ModifySmartGuardStoreReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::ModifySmartGuardStoreRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<ModifySmartGuardStoreRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::ModifySmartGuardStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<ModifySmartGuardStoreRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QuerySmartGuardStoreInfoReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QuerySmartGuardStoreInfoReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QuerySmartGuardStoreInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QuerySmartGuardStoreInfoReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QuerySmartGuardStoreInfoRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QuerySmartGuardStoreInfoRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QuerySmartGuardStoreInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QuerySmartGuardStoreInfoRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryAllSmartGuardStoreReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryAllSmartGuardStoreReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllSmartGuardStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryAllSmartGuardStoreReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryAllSmartGuardStoreRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryAllSmartGuardStoreRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllSmartGuardStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryAllSmartGuardStoreRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::AddRegularPatrolReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<AddRegularPatrolReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::AddRegularPatrolReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<AddRegularPatrolReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::AddRegularPatrolRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<AddRegularPatrolRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::AddRegularPatrolRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<AddRegularPatrolRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::DeleteRegularPatrolReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<DeleteRegularPatrolReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::DeleteRegularPatrolReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<DeleteRegularPatrolReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::DeleteRegularPatrolRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<DeleteRegularPatrolRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::DeleteRegularPatrolRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<DeleteRegularPatrolRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::ModifyRegularPatrolReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<ModifyRegularPatrolReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::ModifyRegularPatrolReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<ModifyRegularPatrolReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::ModifyRegularPatrolRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<ModifyRegularPatrolRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::ModifyRegularPatrolRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<ModifyRegularPatrolRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryRegularPatrolInfoReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryRegularPatrolInfoReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryRegularPatrolInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryRegularPatrolInfoReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryRegularPatrolInfoRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryRegularPatrolInfoRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryRegularPatrolInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryRegularPatrolInfoRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryAllRegularPatrolReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryAllRegularPatrolReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllRegularPatrolReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryAllRegularPatrolReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryAllRegularPatrolRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryAllRegularPatrolRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllRegularPatrolRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryAllRegularPatrolRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::UserJoinStoreReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<UserJoinStoreReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::UserJoinStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<UserJoinStoreReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::UserJoinStoreRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<UserJoinStoreRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::UserJoinStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<UserJoinStoreRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::UserQuitStoreReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<UserQuitStoreReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::UserQuitStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<UserQuitStoreReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::UserQuitStoreRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<UserQuitStoreRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::UserQuitStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<UserQuitStoreRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryStoreAllUserReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryStoreAllUserReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryStoreAllUserReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryStoreAllUserReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryStoreAllUserRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryStoreAllUserRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryStoreAllUserRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryStoreAllUserRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::AddVIPCustomerReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<AddVIPCustomerReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::AddVIPCustomerReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<AddVIPCustomerReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::AddVIPCustomerRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<AddVIPCustomerRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::AddVIPCustomerRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<AddVIPCustomerRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::DeleteVIPCustomerReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<DeleteVIPCustomerReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::DeleteVIPCustomerReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<DeleteVIPCustomerReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::DeleteVIPCustomerRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<DeleteVIPCustomerRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::DeleteVIPCustomerRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<DeleteVIPCustomerRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::ModifyVIPCustomerReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<ModifyVIPCustomerReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::ModifyVIPCustomerReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<ModifyVIPCustomerReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::ModifyVIPCustomerRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<ModifyVIPCustomerRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::ModifyVIPCustomerRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<ModifyVIPCustomerRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryVIPCustomerInfoReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryVIPCustomerInfoReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryVIPCustomerInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryVIPCustomerInfoReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryVIPCustomerInfoRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryVIPCustomerInfoRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryVIPCustomerInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryVIPCustomerInfoRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryAllVIPCustomerReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryAllVIPCustomerReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllVIPCustomerReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryAllVIPCustomerReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryAllVIPCustomerRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryAllVIPCustomerRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllVIPCustomerRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryAllVIPCustomerRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::AddVIPConsumeHistoryReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<AddVIPConsumeHistoryReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::AddVIPConsumeHistoryReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<AddVIPConsumeHistoryReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::AddVIPConsumeHistoryRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<AddVIPConsumeHistoryRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::AddVIPConsumeHistoryRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<AddVIPConsumeHistoryRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::DeleteVIPConsumeHistoryReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<DeleteVIPConsumeHistoryReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::DeleteVIPConsumeHistoryReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<DeleteVIPConsumeHistoryReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::DeleteVIPConsumeHistoryRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<DeleteVIPConsumeHistoryRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::DeleteVIPConsumeHistoryRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<DeleteVIPConsumeHistoryRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::ModifyVIPConsumeHistoryReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<ModifyVIPConsumeHistoryReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::ModifyVIPConsumeHistoryReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<ModifyVIPConsumeHistoryReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::ModifyVIPConsumeHistoryRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<ModifyVIPConsumeHistoryRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::ModifyVIPConsumeHistoryRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<ModifyVIPConsumeHistoryRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryAllVIPConsumeHistoryReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryAllVIPConsumeHistoryReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllVIPConsumeHistoryReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryAllVIPConsumeHistoryReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryAllVIPConsumeHistoryRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryAllVIPConsumeHistoryRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllVIPConsumeHistoryRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryAllVIPConsumeHistoryRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::AddEvaluationTemplateReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<AddEvaluationTemplateReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::AddEvaluationTemplateReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<AddEvaluationTemplateReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::AddEvaluationTemplateRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<AddEvaluationTemplateRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::AddEvaluationTemplateRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<AddEvaluationTemplateRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::DeleteEvaluationTemplateReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<DeleteEvaluationTemplateReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::DeleteEvaluationTemplateReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<DeleteEvaluationTemplateReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::DeleteEvaluationTemplateRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<DeleteEvaluationTemplateRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::DeleteEvaluationTemplateRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<DeleteEvaluationTemplateRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::ModifyEvaluationTemplateReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<ModifyEvaluationTemplateReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::ModifyEvaluationTemplateReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<ModifyEvaluationTemplateReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::ModifyEvaluationTemplateRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<ModifyEvaluationTemplateRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::ModifyEvaluationTemplateRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<ModifyEvaluationTemplateRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryAllEvaluationTemplateReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryAllEvaluationTemplateReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllEvaluationTemplateReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryAllEvaluationTemplateReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryAllEvaluationTemplateRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryAllEvaluationTemplateRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllEvaluationTemplateRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryAllEvaluationTemplateRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::AddStoreEvaluationReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<AddStoreEvaluationReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::AddStoreEvaluationReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<AddStoreEvaluationReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::AddStoreEvaluationRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<AddStoreEvaluationRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::AddStoreEvaluationRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<AddStoreEvaluationRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::DeleteStoreEvaluationReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<DeleteStoreEvaluationReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::DeleteStoreEvaluationReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<DeleteStoreEvaluationReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::DeleteStoreEvaluationRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<DeleteStoreEvaluationRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::DeleteStoreEvaluationRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<DeleteStoreEvaluationRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::ModifyStoreEvaluationReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<ModifyStoreEvaluationReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::ModifyStoreEvaluationReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<ModifyStoreEvaluationReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::ModifyStoreEvaluationRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<ModifyStoreEvaluationRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::ModifyStoreEvaluationRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<ModifyStoreEvaluationRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryStoreEvaluationInfoReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryStoreEvaluationInfoReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryStoreEvaluationInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryStoreEvaluationInfoReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryStoreEvaluationInfoRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryStoreEvaluationInfoRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryStoreEvaluationInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryStoreEvaluationInfoRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryAllStoreEvaluationReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryAllStoreEvaluationReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllStoreEvaluationReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryAllStoreEvaluationReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryAllStoreEvaluationRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryAllStoreEvaluationRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllStoreEvaluationRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryAllStoreEvaluationRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::AddRemotePatrolStoreReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<AddRemotePatrolStoreReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::AddRemotePatrolStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<AddRemotePatrolStoreReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::AddRemotePatrolStoreRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<AddRemotePatrolStoreRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::AddRemotePatrolStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<AddRemotePatrolStoreRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::DeleteRemotePatrolStoreReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<DeleteRemotePatrolStoreReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::DeleteRemotePatrolStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<DeleteRemotePatrolStoreReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::DeleteRemotePatrolStoreRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<DeleteRemotePatrolStoreRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::DeleteRemotePatrolStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<DeleteRemotePatrolStoreRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::ModifyRemotePatrolStoreReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<ModifyRemotePatrolStoreReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::ModifyRemotePatrolStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<ModifyRemotePatrolStoreReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::ModifyRemotePatrolStoreRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<ModifyRemotePatrolStoreRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::ModifyRemotePatrolStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<ModifyRemotePatrolStoreRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryRemotePatrolStoreInfoReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryRemotePatrolStoreInfoReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryRemotePatrolStoreInfoReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryRemotePatrolStoreInfoReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryRemotePatrolStoreInfoRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryRemotePatrolStoreInfoRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryRemotePatrolStoreInfoRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryRemotePatrolStoreInfoRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryAllRemotePatrolStoreReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryAllRemotePatrolStoreReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllRemotePatrolStoreReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryAllRemotePatrolStoreReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryAllRemotePatrolStoreRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryAllRemotePatrolStoreRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryAllRemotePatrolStoreRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryAllRemotePatrolStoreRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::ImportPOSDataReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<ImportPOSDataReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::ImportPOSDataReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<ImportPOSDataReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::ImportPOSDataRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<ImportPOSDataRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::ImportPOSDataRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<ImportPOSDataRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::QueryCustomerFlowStatisticReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<QueryCustomerFlowStatisticReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::QueryCustomerFlowStatisticReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<QueryCustomerFlowStatisticReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::QueryCustomerFlowStatisticRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<QueryCustomerFlowStatisticRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::QueryCustomerFlowStatisticRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<QueryCustomerFlowStatisticRsp, Request>(message, rsp);
}

bool PassengerFlowProtoHandler::ReportCustomerFlowDataReq_Serializer(const Request &req, std::string &strOutput)
{
    return SerializerT<ReportCustomerFlowDataReq, Request>(req, strOutput);
}

bool PassengerFlowProtoHandler::ReportCustomerFlowDataReq_UnSerializer(const CustomerFlowMessage &message, Request &req)
{
    return UnSerializerT<ReportCustomerFlowDataReq, Request>(message, req);
}

bool PassengerFlowProtoHandler::ReportCustomerFlowDataRsp_Serializer(const Request &rsp, std::string &strOutput)
{
    return SerializerT<ReportCustomerFlowDataRsp, Request>(rsp, strOutput);
}

bool PassengerFlowProtoHandler::ReportCustomerFlowDataRsp_UnSerializer(const CustomerFlowMessage &message, Request &rsp)
{
    return UnSerializerT<ReportCustomerFlowDataRsp, Request>(message, rsp);
}


void PassengerFlowProtoHandler::Request::Serializer(CustomerFlowMessage &message) const
{
    message.set_uimsgseq(m_uiMsgSeq);
}

void PassengerFlowProtoHandler::Request::UnSerializer(const CustomerFlowMessage &message)
{
    m_MsgType = (CustomerFlowMsgType)message.type();
    m_uiMsgSeq = message.uimsgseq();
}

void PassengerFlowProtoHandler::Response::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    auto rsp = message.mutable_rspvalue();
    rsp->set_iretcode(m_iRetcode);
    rsp->set_strretmsg(m_strRetMsg);
}
void PassengerFlowProtoHandler::Response::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto rsp = message.rspvalue();
    m_iRetcode = rsp.iretcode();
    m_strRetMsg = rsp.strretmsg();
}

void PassengerFlowProtoHandler::CustomerFlowPreHandleReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::CustomerFlowPreHandleReq_T);

    message.mutable_reqvalue()->mutable_customerflowprehandlereq_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::CustomerFlowPreHandleReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    m_strValue = message.reqvalue().customerflowprehandlereq_value().strvalue();
}

void PassengerFlowProtoHandler::CustomerFlowPreHandleRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::CustomerFlowPreHandleRsp_T);

    message.mutable_rspvalue()->mutable_customerflowprehandlersp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::CustomerFlowPreHandleRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    m_strValue = message.rspvalue().customerflowprehandlersp_value().strvalue();
}

void PassengerFlowProtoHandler::ShakehandReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ShakehandReq_T);

    message.mutable_reqvalue()->mutable_shakehandreq_value()->set_strid(m_strID);
}

void PassengerFlowProtoHandler::ShakehandReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    m_strID = message.reqvalue().shakehandreq_value().strid();
}

void PassengerFlowProtoHandler::ShakehandRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ShakehandRsp_T);

    message.mutable_rspvalue()->mutable_shakehandrsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::ShakehandRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().shakehandrsp_value().strvalue();
}

void PassengerFlowProtoHandler::AddAreaReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddAreaReq_T);

    auto req = message.mutable_reqvalue()->mutable_addareareq_value();
    req->set_struserid(m_strUserID);

    auto area = req->mutable_areainfo();
    area->set_strareaid(m_areaInfo.m_strAreaID);
    area->set_strareaname(m_areaInfo.m_strAreaName);
    area->set_uilevel(m_areaInfo.m_uiLevel);
    area->set_strparentareaid(m_areaInfo.m_strParentAreaID);
    area->set_strcreatedate(m_areaInfo.m_strCreateDate);
    area->set_uistate(m_areaInfo.m_uiState);
    area->set_strextend(m_areaInfo.m_strExtend);
}

void PassengerFlowProtoHandler::AddAreaReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().addareareq_value();
    m_strUserID = req.struserid();

    auto area = req.areainfo();
    m_areaInfo.m_strAreaID = area.strareaid();
    m_areaInfo.m_strAreaName = area.strareaname();
    m_areaInfo.m_uiLevel = area.uilevel();
    m_areaInfo.m_strParentAreaID = area.strparentareaid();
    m_areaInfo.m_strCreateDate = area.strcreatedate();
    m_areaInfo.m_uiState = area.uistate();
    m_areaInfo.m_strExtend = area.strextend();
}

void PassengerFlowProtoHandler::AddAreaRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddAreaRsp_T);
    message.mutable_rspvalue()->mutable_addarearsp_value()->set_strareaid(m_strAreaID);
}

void PassengerFlowProtoHandler::AddAreaRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    m_strAreaID = message.rspvalue().addarearsp_value().strareaid();
}

void PassengerFlowProtoHandler::DeleteAreaReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteAreaReq_T);

    auto req = message.mutable_reqvalue()->mutable_deleteareareq_value();
    req->set_struserid(m_strUserID);
    req->set_strareaid(m_strAreaID);
}

void PassengerFlowProtoHandler::DeleteAreaReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().deleteareareq_value();
    m_strUserID = req.struserid();
    m_strAreaID = req.strareaid();
}

void PassengerFlowProtoHandler::DeleteAreaRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteAreaRsp_T);

    message.mutable_rspvalue()->mutable_deletearearsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::DeleteAreaRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().deletearearsp_value().strvalue();
}

void PassengerFlowProtoHandler::ModifyAreaReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyAreaReq_T);

    auto req = message.mutable_reqvalue()->mutable_modifyareareq_value();
    req->set_struserid(m_strUserID);

    auto area = req->mutable_areainfo();
    area->set_strareaid(m_areaInfo.m_strAreaID);
    area->set_strareaname(m_areaInfo.m_strAreaName);
    area->set_uilevel(m_areaInfo.m_uiLevel);
    area->set_strparentareaid(m_areaInfo.m_strParentAreaID);
    area->set_strcreatedate(m_areaInfo.m_strCreateDate);
    area->set_uistate(m_areaInfo.m_uiState);
    area->set_strextend(m_areaInfo.m_strExtend);
}

void PassengerFlowProtoHandler::ModifyAreaReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().modifyareareq_value();
    m_strUserID = req.struserid();

    auto area = req.areainfo();
    m_areaInfo.m_strAreaID = area.strareaid();
    m_areaInfo.m_strAreaName = area.strareaname();
    m_areaInfo.m_uiLevel = area.uilevel();
    m_areaInfo.m_strParentAreaID = area.strparentareaid();
    m_areaInfo.m_strCreateDate = area.strcreatedate();
    m_areaInfo.m_uiState = area.uistate();
    m_areaInfo.m_strExtend = area.strextend();
}

void PassengerFlowProtoHandler::ModifyAreaRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyAreaRsp_T);

    message.mutable_rspvalue()->mutable_modifyarearsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::ModifyAreaRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().modifyarearsp_value().strvalue();
}

void PassengerFlowProtoHandler::QueryAreaInfoReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAreaInfoReq_T);

    auto req = message.mutable_reqvalue()->mutable_queryareainforeq_value();
    req->set_struserid(m_strUserID);
    req->set_strareaid(m_strAreaID);
}

void PassengerFlowProtoHandler::QueryAreaInfoReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().queryareainforeq_value();
    m_strUserID = req.struserid();
    m_strAreaID = req.strareaid();
}

void PassengerFlowProtoHandler::QueryAreaInfoRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAreaInfoRsp_T);

    auto area = message.mutable_rspvalue()->mutable_queryareainforsp_value()->mutable_areainfo();
    area->set_strareaid(m_areaInfo.m_strAreaID);
    area->set_strareaname(m_areaInfo.m_strAreaName);
    area->set_uilevel(m_areaInfo.m_uiLevel);
    area->set_strparentareaid(m_areaInfo.m_strParentAreaID);
    area->set_strcreatedate(m_areaInfo.m_strCreateDate);
    area->set_uistate(m_areaInfo.m_uiState);
    area->set_strextend(m_areaInfo.m_strExtend);
}

void PassengerFlowProtoHandler::QueryAreaInfoRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    auto area = message.rspvalue().queryareainforsp_value().areainfo();
    m_areaInfo.m_strAreaID = area.strareaid();
    m_areaInfo.m_strAreaName = area.strareaname();
    m_areaInfo.m_uiLevel = area.uilevel();
    m_areaInfo.m_strParentAreaID = area.strparentareaid();
    m_areaInfo.m_strCreateDate = area.strcreatedate();
    m_areaInfo.m_uiState = area.uistate();
    m_areaInfo.m_strExtend = area.strextend();
}

void PassengerFlowProtoHandler::QueryAllAreaReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllAreaReq_T);

    message.mutable_reqvalue()->mutable_queryallareareq_value()->set_struserid(m_strUserID);
}

void PassengerFlowProtoHandler::QueryAllAreaReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);

    m_strUserID = message.reqvalue().queryallareareq_value().struserid();
}

void PassengerFlowProtoHandler::QueryAllAreaRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllAreaRsp_T);

    auto rsp = message.mutable_rspvalue()->mutable_queryallarearsp_value();
    for (auto it = m_areaList.begin(), end = m_areaList.end(); it != end; ++it)
    {
        auto area = rsp->add_areainfo();
        area->set_strareaid(it->m_strAreaID);
        area->set_strareaname(it->m_strAreaName);
        area->set_uilevel(it->m_uiLevel);
        area->set_strparentareaid(it->m_strParentAreaID);
        area->set_strcreatedate(it->m_strCreateDate);
        area->set_uistate(it->m_uiState);
        area->set_strextend(it->m_strExtend);
    }
}

void PassengerFlowProtoHandler::QueryAllAreaRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    auto rsp = message.rspvalue().queryallarearsp_value();
    for (int i = 0, sz = rsp.areainfo_size(); i < sz; ++i)
    {
        auto rspArea = rsp.areainfo(i);
        Area area;
        area.m_strAreaID = rspArea.strareaid();
        area.m_strAreaName = rspArea.strareaname();
        area.m_uiLevel = rspArea.uilevel();
        area.m_strParentAreaID = rspArea.strparentareaid();
        area.m_strCreateDate = rspArea.strcreatedate();
        area.m_uiState = rspArea.uistate();
        area.m_strExtend = rspArea.strextend();

        m_areaList.push_back(area);
    }
}

void PassengerFlowProtoHandler::BindPushClientIDReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::BindPushClientIDReq_T);

    auto req = message.mutable_reqvalue()->mutable_bindpushclientidreq_value();
    req->set_struserid(m_strUserID);
    req->set_strclientid(m_strClientID);
}

void PassengerFlowProtoHandler::BindPushClientIDReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().bindpushclientidreq_value();
    m_strUserID = req.struserid();
    m_strClientID = req.strclientid();
}

void PassengerFlowProtoHandler::BindPushClientIDRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::BindPushClientIDRsp_T);

    message.mutable_rspvalue()->mutable_bindpushclientidrsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::BindPushClientIDRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().bindpushclientidrsp_value().strvalue();
}

void PassengerFlowProtoHandler::UnbindPushClientIDReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::UnbindPushClientIDReq_T);

    auto req = message.mutable_reqvalue()->mutable_unbindpushclientidreq_value();
    req->set_struserid(m_strUserID);
    req->set_strclientid(m_strClientID);
}

void PassengerFlowProtoHandler::UnbindPushClientIDReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().unbindpushclientidreq_value();
    m_strUserID = req.struserid();
    m_strClientID = req.strclientid();
}

void PassengerFlowProtoHandler::UnbindPushClientIDRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::UnbindPushClientIDRsp_T);

    message.mutable_rspvalue()->mutable_unbindpushclientidrsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::UnbindPushClientIDRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().unbindpushclientidrsp_value().strvalue();
}

void PassengerFlowProtoHandler::AddGroupReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddGroupReq_T);

    auto req = message.mutable_reqvalue()->mutable_addgroupreq_value();
    req->set_struserid(m_strUserID);

    auto group = req->mutable_groupinfo();
    group->set_strgroupid(m_groupInfo.m_strGroupID);
    group->set_strgroupname(m_groupInfo.m_strGroupName);
    group->set_strcreatedate(m_groupInfo.m_strCreateDate);
    group->set_uistate(m_groupInfo.m_uiState);
}

void PassengerFlowProtoHandler::AddGroupReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().addgroupreq_value();
    m_strUserID = req.struserid();

    auto group = req.groupinfo();
    m_groupInfo.m_strGroupID = group.strgroupid();
    m_groupInfo.m_strGroupName = group.strgroupname();
    m_groupInfo.m_strCreateDate = group.strcreatedate();
    m_groupInfo.m_uiState = group.uistate();
}

void PassengerFlowProtoHandler::AddGroupRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddGroupRsp_T);
    message.mutable_rspvalue()->mutable_addgrouprsp_value()->set_strgroupid(m_strGroupID);
}

void PassengerFlowProtoHandler::AddGroupRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    m_strGroupID = message.rspvalue().addgrouprsp_value().strgroupid();
}

void PassengerFlowProtoHandler::DeleteGroupReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteGroupReq_T);

    auto req = message.mutable_reqvalue()->mutable_deletegroupreq_value();
    req->set_struserid(m_strUserID);
    req->set_strgroupid(m_strGroupID);
}

void PassengerFlowProtoHandler::DeleteGroupReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().deletegroupreq_value();
    m_strUserID = req.struserid();
    m_strGroupID = req.strgroupid();
}

void PassengerFlowProtoHandler::DeleteGroupRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteGroupRsp_T);

    message.mutable_rspvalue()->mutable_deletegrouprsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::DeleteGroupRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().deletegrouprsp_value().strvalue();
}

void PassengerFlowProtoHandler::ModifyGroupReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyGroupReq_T);

    auto req = message.mutable_reqvalue()->mutable_modifygroupreq_value();
    req->set_struserid(m_strUserID);

    auto group = req->mutable_groupinfo();
    group->set_strgroupid(m_groupInfo.m_strGroupID);
    group->set_strgroupname(m_groupInfo.m_strGroupName);
    group->set_strcreatedate(m_groupInfo.m_strCreateDate);
    group->set_uistate(m_groupInfo.m_uiState);
}

void PassengerFlowProtoHandler::ModifyGroupReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().modifygroupreq_value();
    m_strUserID = req.struserid();

    auto group = req.groupinfo();
    m_groupInfo.m_strGroupID = group.strgroupid();
    m_groupInfo.m_strGroupName = group.strgroupname();
    m_groupInfo.m_strCreateDate = group.strcreatedate();
    m_groupInfo.m_uiState = group.uistate();
}

void PassengerFlowProtoHandler::ModifyGroupRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyGroupRsp_T);

    message.mutable_rspvalue()->mutable_modifygrouprsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::ModifyGroupRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().modifygrouprsp_value().strvalue();
}

void PassengerFlowProtoHandler::QueryGroupInfoReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryGroupInfoReq_T);
    auto req = message.mutable_reqvalue()->mutable_querygroupinforeq_value();
    req->set_struserid(m_strUserID);
    req->set_strgroupid(m_strGroupID);
}

void PassengerFlowProtoHandler::QueryGroupInfoReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().querygroupinforeq_value();
    m_strUserID = req.struserid();
    m_strGroupID = req.strgroupid();
}

void PassengerFlowProtoHandler::QueryGroupInfoRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryGroupInfoRsp_T);

    auto group = message.mutable_rspvalue()->mutable_querygroupinforsp_value()->mutable_groupinfo();
    group->set_strgroupid(m_groupInfo.m_strGroupID);
    group->set_strgroupname(m_groupInfo.m_strGroupName);
    group->set_strcreatedate(m_groupInfo.m_strCreateDate);
    group->set_uistate(m_groupInfo.m_uiState);
}

void PassengerFlowProtoHandler::QueryGroupInfoRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    auto group = message.rspvalue().querygroupinforsp_value().groupinfo();
    m_groupInfo.m_strGroupID = group.strgroupid();
    m_groupInfo.m_strGroupName = group.strgroupname();
    m_groupInfo.m_strCreateDate = group.strcreatedate();
    m_groupInfo.m_uiState = group.uistate();
}

void PassengerFlowProtoHandler::QueryAllGroupReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllGroupReq_T);

    message.mutable_reqvalue()->mutable_queryallgroupreq_value()->set_struserid(m_strUserID);
}

void PassengerFlowProtoHandler::QueryAllGroupReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);

    m_strUserID = message.reqvalue().queryallgroupreq_value().struserid();
}

void PassengerFlowProtoHandler::QueryAllGroupRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllGroupRsp_T);

    auto group = message.mutable_rspvalue()->mutable_queryallgrouprsp_value()->mutable_groupinfo();
    SerializeGroupList(m_groupList, group);
}

void PassengerFlowProtoHandler::QueryAllGroupRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    UnSerializeGroupList(m_groupList, message.rspvalue().queryallgrouprsp_value().groupinfo());
}

void PassengerFlowProtoHandler::AddStoreReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddStoreReq_T);

    auto req = message.mutable_reqvalue()->mutable_addstorereq_value();
    req->set_struserid(m_strUserID);

    auto store = req->mutable_storeinfo();
    store->set_strstoreid(m_storeInfo.m_strStoreID);
    store->set_strstorename(m_storeInfo.m_strStoreName);
    store->set_strgoodscategory(m_storeInfo.m_strGoodsCategory);
    store->set_straddress(m_storeInfo.m_strAddress);
    store->mutable_area()->set_strareaid(m_storeInfo.m_area.m_strAreaID);
    store->set_uiopenstate(m_storeInfo.m_uiOpenState);
    store->set_strcreatedate(m_storeInfo.m_strCreateDate);
    store->set_strextend(m_storeInfo.m_strExtend);
    store->set_uistate(m_storeInfo.m_uiState);
    SerializeEntranceList(m_storeInfo.m_entranceList, store->mutable_entrance());
}

void PassengerFlowProtoHandler::AddStoreReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().addstorereq_value();
    m_strUserID = req.struserid();

    auto store = req.storeinfo();
    m_storeInfo.m_strStoreID = store.strstoreid();
    m_storeInfo.m_strStoreName = store.strstorename();
    m_storeInfo.m_strGoodsCategory = store.strgoodscategory();
    m_storeInfo.m_strAddress = store.straddress();
    m_storeInfo.m_area.m_strAreaID = store.area().strareaid();
    m_storeInfo.m_uiOpenState = store.uiopenstate();
    m_storeInfo.m_strCreateDate = store.strcreatedate();
    m_storeInfo.m_strExtend = store.strextend();
    m_storeInfo.m_uiState = store.uistate();
    UnSerializeEntranceList(m_storeInfo.m_entranceList, store.entrance());
}

void PassengerFlowProtoHandler::AddStoreRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddStoreRsp_T);
    message.mutable_rspvalue()->mutable_addstorersp_value()->set_strstoreid(m_strStoreID);
}

void PassengerFlowProtoHandler::AddStoreRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    m_strStoreID = message.rspvalue().addstorersp_value().strstoreid();
}

void PassengerFlowProtoHandler::DeleteStoreReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteStoreReq_T);

    auto req = message.mutable_reqvalue()->mutable_deletestorereq_value();
    req->set_struserid(m_strUserID);
    req->set_strstoreid(m_strStoreID);
}

void PassengerFlowProtoHandler::DeleteStoreReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().deletestorereq_value();
    m_strUserID = req.struserid();
    m_strStoreID = req.strstoreid();
}

void PassengerFlowProtoHandler::DeleteStoreRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteStoreRsp_T);

    message.mutable_rspvalue()->mutable_deletestorersp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::DeleteStoreRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().deletestorersp_value().strvalue();
}

void PassengerFlowProtoHandler::ModifyStoreReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyStoreReq_T);

    auto req = message.mutable_reqvalue()->mutable_modifystorereq_value();
    req->set_struserid(m_strUserID);

    auto store = req->mutable_storeinfo();
    store->set_strstoreid(m_storeInfo.m_strStoreID);
    store->set_strstorename(m_storeInfo.m_strStoreName);
    store->set_strgoodscategory(m_storeInfo.m_strGoodsCategory);
    store->set_straddress(m_storeInfo.m_strAddress);
    store->mutable_area()->set_strareaid(m_storeInfo.m_area.m_strAreaID);
    store->set_uiopenstate(m_storeInfo.m_uiOpenState);
    store->set_strcreatedate(m_storeInfo.m_strCreateDate);
    store->set_strextend(m_storeInfo.m_strExtend);
    store->set_uistate(m_storeInfo.m_uiState);
    SerializeEntranceList(m_storeInfo.m_entranceList, store->mutable_entrance());
}

void PassengerFlowProtoHandler::ModifyStoreReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().modifystorereq_value();
    m_strUserID = req.struserid();

    auto store = req.storeinfo();
    m_storeInfo.m_strStoreID = store.strstoreid();
    m_storeInfo.m_strStoreName = store.strstorename();
    m_storeInfo.m_strGoodsCategory = store.strgoodscategory();
    m_storeInfo.m_strAddress = store.straddress();
    m_storeInfo.m_area.m_strAreaID = store.area().strareaid();
    m_storeInfo.m_uiOpenState = store.uiopenstate();
    m_storeInfo.m_strCreateDate = store.strcreatedate();
    m_storeInfo.m_strExtend = store.strextend();
    m_storeInfo.m_uiState = store.uistate();
    UnSerializeEntranceList(m_storeInfo.m_entranceList, store.entrance());
}

void PassengerFlowProtoHandler::ModifyStoreRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyStoreRsp_T);

    message.mutable_rspvalue()->mutable_modifystorersp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::ModifyStoreRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().modifystorersp_value().strvalue();
}

void PassengerFlowProtoHandler::QueryStoreInfoReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryStoreInfoReq_T);
    auto req = message.mutable_reqvalue()->mutable_querystoreinforeq_value();
    req->set_struserid(m_strUserID);
    req->set_strstoreid(m_strStoreID);
}

void PassengerFlowProtoHandler::QueryStoreInfoReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().querystoreinforeq_value();
    m_strUserID = req.struserid();
    m_strStoreID = req.strstoreid();
}

void PassengerFlowProtoHandler::QueryStoreInfoRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryStoreInfoRsp_T);

    auto rsp = message.mutable_rspvalue()->mutable_querystoreinforsp_value();
    auto store = rsp->mutable_storeinfo();
    store->set_strstoreid(m_storeInfo.m_strStoreID);
    store->set_strstorename(m_storeInfo.m_strStoreName);
    store->set_strgoodscategory(m_storeInfo.m_strGoodsCategory);
    store->set_straddress(m_storeInfo.m_strAddress);
    store->set_uiopenstate(m_storeInfo.m_uiOpenState);
    store->set_strcreatedate(m_storeInfo.m_strCreateDate);
    store->set_strextend(m_storeInfo.m_strExtend);
    store->set_uistate(m_storeInfo.m_uiState);

    auto storeArea = store->mutable_area();
    storeArea->set_strareaid(m_storeInfo.m_area.m_strAreaID);
    storeArea->set_strareaname(m_storeInfo.m_area.m_strAreaName);
    SerializeEntranceList(m_storeInfo.m_entranceList, store->mutable_entrance());

    for (auto it = m_areaList.begin(), end = m_areaList.end(); it != end; ++it)
    {
        auto area = rsp->add_area();
        area->set_strareaid(it->m_strAreaID);
        area->set_strareaname(it->m_strAreaName);
    }
}

void PassengerFlowProtoHandler::QueryStoreInfoRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    auto rsp = message.rspvalue().querystoreinforsp_value();
    auto store = rsp.storeinfo();
    m_storeInfo.m_strStoreID = store.strstoreid();
    m_storeInfo.m_strStoreName = store.strstorename();
    m_storeInfo.m_strGoodsCategory = store.strgoodscategory();
    m_storeInfo.m_strAddress = store.straddress();
    m_storeInfo.m_uiOpenState = store.uiopenstate();
    m_storeInfo.m_strCreateDate = store.strcreatedate();
    m_storeInfo.m_strExtend = store.strextend();
    m_storeInfo.m_uiState = store.uistate();

    auto storeArea = store.area();
    m_storeInfo.m_area.m_strAreaID = storeArea.strareaid();
    m_storeInfo.m_area.m_strAreaName = storeArea.strareaname();
    UnSerializeEntranceList(m_storeInfo.m_entranceList, store.entrance());

    for (int i = 0, sz = rsp.area_size(); i < sz; ++i)
    {
        auto rspArea = rsp.area(i);
        Area area;
        area.m_strAreaID = rspArea.strareaid();
        area.m_strAreaName = rspArea.strareaname();

        m_areaList.push_back(area);
    }
}

void PassengerFlowProtoHandler::QueryAllStoreReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllStoreReq_T);

    auto req = message.mutable_reqvalue()->mutable_queryallstorereq_value();
    req->set_struserid(m_strUserID);
    req->set_uibeginindex(m_uiBeginIndex);
}

void PassengerFlowProtoHandler::QueryAllStoreReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);

    auto req = message.reqvalue().queryallstorereq_value();
    m_strUserID = req.struserid();
    m_uiBeginIndex = req.uibeginindex();
}

void PassengerFlowProtoHandler::QueryAllStoreRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllStoreRsp_T);

    SerializeStoreList(m_storeList, message.mutable_rspvalue()->mutable_queryallstorersp_value()->mutable_storeinfo());
}

void PassengerFlowProtoHandler::QueryAllStoreRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    UnSerializeStoreList(m_storeList, message.rspvalue().queryallstorersp_value().storeinfo());
}

void PassengerFlowProtoHandler::AddEntranceReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddEntranceReq_T);

    auto req = message.mutable_reqvalue()->mutable_addentrancereq_value();
    req->set_struserid(m_strUserID);
    req->set_strstoreid(m_strStoreID);

    auto entrance = req->mutable_entranceinfo();
    entrance->set_strentranceid(m_entranceInfo.m_strEntranceID);
    entrance->set_strentrancename(m_entranceInfo.m_strEntranceName);

    for (auto it = m_entranceInfo.m_strDeviceIDList.begin(), end = m_entranceInfo.m_strDeviceIDList.end(); it != end; ++it)
    {
        entrance->add_strdeviceid(*it);
    }
}

void PassengerFlowProtoHandler::AddEntranceReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().addentrancereq_value();
    m_strUserID = req.struserid();
    m_strStoreID = req.strstoreid();

    auto entrance = req.entranceinfo();
    m_entranceInfo.m_strEntranceID = entrance.strentranceid();
    m_entranceInfo.m_strEntranceName = entrance.strentrancename();

    auto deviceList = entrance.strdeviceid();
    for (int i = 0, sz = deviceList.size(); i < sz; ++i)
    {
        m_entranceInfo.m_strDeviceIDList.push_back(deviceList.Get(i));
    }
}

void PassengerFlowProtoHandler::AddEntranceRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddEntranceRsp_T);
    message.mutable_rspvalue()->mutable_addentrancersp_value()->set_strentranceid(m_strEntranceID);
}

void PassengerFlowProtoHandler::AddEntranceRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    m_strEntranceID = message.rspvalue().addentrancersp_value().strentranceid();
}

void PassengerFlowProtoHandler::DeleteEntranceReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteEntranceReq_T);

    auto req = message.mutable_reqvalue()->mutable_deleteentrancereq_value();
    req->set_struserid(m_strUserID);
    req->set_strstoreid(m_strStoreID);
    req->set_strentranceid(m_strEntranceID);
}

void PassengerFlowProtoHandler::DeleteEntranceReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().deleteentrancereq_value();
    m_strUserID = req.struserid();
    m_strStoreID = req.strstoreid();
    m_strEntranceID = req.strentranceid();
}

void PassengerFlowProtoHandler::DeleteEntranceRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteEntranceRsp_T);

    message.mutable_rspvalue()->mutable_deleteentrancersp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::DeleteEntranceRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().deleteentrancersp_value().strvalue();
}

void PassengerFlowProtoHandler::ModifyEntranceReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyEntranceReq_T);

    auto req = message.mutable_reqvalue()->mutable_modifyentrancereq_value();
    req->set_struserid(m_strUserID);
    req->set_strstoreid(m_strStoreID);

    auto entrance = req->mutable_entranceinfo();
    entrance->set_strentranceid(m_entranceInfo.m_strEntranceID);
    entrance->set_strentrancename(m_entranceInfo.m_strEntranceName);

    for (auto it = m_entranceInfo.m_strDeviceIDList.begin(), end = m_entranceInfo.m_strDeviceIDList.end(); it != end; ++it)
    {
        entrance->add_strdeviceid(*it);
    }

    for (auto it = m_strAddedDeviceIDList.begin(), end = m_strAddedDeviceIDList.end(); it != end; ++it)
    {
        req->add_straddeddeviceid(*it);
    }

    for (auto it = m_strDeletedDeviceIDList.begin(), end = m_strDeletedDeviceIDList.end(); it != end; ++it)
    {
        req->add_strdeleteddeviceid(*it);
    }
}

void PassengerFlowProtoHandler::ModifyEntranceReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().modifyentrancereq_value();
    m_strUserID = req.struserid();
    m_strStoreID = req.strstoreid();

    auto entrance = req.entranceinfo();
    m_entranceInfo.m_strEntranceID = entrance.strentranceid();
    m_entranceInfo.m_strEntranceName = entrance.strentrancename();

    auto deviceList = entrance.strdeviceid();
    for (int i = 0, sz = deviceList.size(); i < sz; ++i)
    {
        m_entranceInfo.m_strDeviceIDList.push_back(deviceList.Get(i));
    }

    auto addedList = req.straddeddeviceid();
    for (int i = 0, sz = addedList.size(); i < sz; ++i)
    {
        m_strAddedDeviceIDList.push_back(addedList.Get(i));
    }

    auto deletedList = req.strdeleteddeviceid();
    for (int i = 0, sz = deletedList.size(); i < sz; ++i)
    {
        m_strDeletedDeviceIDList.push_back(deletedList.Get(i));
    }
}

void PassengerFlowProtoHandler::ModifyEntranceRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyEntranceRsp_T);

    message.mutable_rspvalue()->mutable_modifyentrancersp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::ModifyEntranceRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().modifyentrancersp_value().strvalue();
}

void PassengerFlowProtoHandler::AddEntranceDeviceReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddEntranceDeviceReq_T);

    auto req = message.mutable_reqvalue()->mutable_addentrancedevicereq_value();
    req->set_struserid(m_strUserID);
    req->set_strstoreid(m_strStoreID);
    req->set_strentranceid(m_strEntranceID);
    req->set_strdeviceid(m_strDeviceID);
}

void PassengerFlowProtoHandler::AddEntranceDeviceReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().addentrancedevicereq_value();
    m_strUserID = req.struserid();
    m_strStoreID = req.strstoreid();
    m_strEntranceID = req.strentranceid();
    m_strDeviceID = req.strdeviceid();
}

void PassengerFlowProtoHandler::AddEntranceDeviceRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddEntranceDeviceRsp_T);

    message.mutable_rspvalue()->mutable_addentrancedevicersp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::AddEntranceDeviceRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().addentrancedevicersp_value().strvalue();
}

void PassengerFlowProtoHandler::DeleteEntranceDeviceReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteEntranceDeviceReq_T);

    auto req = message.mutable_reqvalue()->mutable_deleteentrancedevicereq_value();
    req->set_struserid(m_strUserID);
    req->set_strstoreid(m_strStoreID);
    req->set_strentranceid(m_strEntranceID);
    req->set_strdeviceid(m_strDeviceID);
}

void PassengerFlowProtoHandler::DeleteEntranceDeviceReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().deleteentrancedevicereq_value();
    m_strUserID = req.struserid();
    m_strStoreID = req.strstoreid();
    m_strEntranceID = req.strentranceid();
    m_strDeviceID = req.strdeviceid();
}

void PassengerFlowProtoHandler::DeleteEntranceDeviceRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteEntranceDeviceRsp_T);

    message.mutable_rspvalue()->mutable_deleteentrancedevicersp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::DeleteEntranceDeviceRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().deleteentrancedevicersp_value().strvalue();
}

void PassengerFlowProtoHandler::AddEventReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddEventReq_T);

    auto event = message.mutable_reqvalue()->mutable_addeventreq_value()->mutable_eventinfo();
    event->set_streventid(m_eventInfo.m_strEventID);
    event->set_strsource(m_eventInfo.m_strSource);
    event->set_strsubmitdate(m_eventInfo.m_strSubmitDate);
    event->set_strexpiredate(m_eventInfo.m_strExpireDate);
    event->set_struserid(m_eventInfo.m_strUserID);
    event->set_strdeviceid(m_eventInfo.m_strDeviceID);
    event->set_strprocessstate(m_eventInfo.m_strProcessState);
    event->set_strremark(m_eventInfo.m_strRemark);
    event->set_uiviewstate(m_eventInfo.m_uiViewState);
    event->set_strcreatedate(m_eventInfo.m_strCreateDate);
    event->set_strextend(m_eventInfo.m_strExtend);
    event->set_uistate(m_eventInfo.m_uiState);

    for (auto it = m_eventInfo.m_uiTypeList.begin(), end = m_eventInfo.m_uiTypeList.end(); it != end; ++it)
    {
        event->add_uitype(*it);
    }

    for (auto it = m_eventInfo.m_strHandlerList.begin(), end = m_eventInfo.m_strHandlerList.end(); it != end; ++it)
    {
        event->add_strhandler(*it);
    }
}

void PassengerFlowProtoHandler::AddEventReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);

    auto event = message.reqvalue().addeventreq_value().eventinfo();
    m_eventInfo.m_strEventID = event.streventid();
    m_eventInfo.m_strSource = event.strsource();
    m_eventInfo.m_strSubmitDate = event.strsubmitdate();
    m_eventInfo.m_strExpireDate = event.strexpiredate();
    m_eventInfo.m_strUserID = event.struserid();
    m_eventInfo.m_strDeviceID = event.strdeviceid();
    m_eventInfo.m_strProcessState = event.strprocessstate();
    m_eventInfo.m_strRemark = event.strremark();
    m_eventInfo.m_uiViewState = event.uiviewstate();
    m_eventInfo.m_strCreateDate = event.strcreatedate();
    m_eventInfo.m_strExtend = event.strextend();
    m_eventInfo.m_uiState = event.uistate();

    auto typeList = event.uitype();
    for (int i = 0, sz = typeList.size(); i < sz; ++i)
    {
        m_eventInfo.m_uiTypeList.push_back(typeList.Get(i));
    }

    auto respList = event.strhandler();
    for (int i = 0, sz = respList.size(); i < sz; ++i)
    {
        m_eventInfo.m_strHandlerList.push_back(respList.Get(i));
    }
}

void PassengerFlowProtoHandler::AddEventRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddEventRsp_T);
    message.mutable_rspvalue()->mutable_addeventrsp_value()->set_streventid(m_strEventID);
}

void PassengerFlowProtoHandler::AddEventRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    m_strEventID = message.rspvalue().addeventrsp_value().streventid();
}

void PassengerFlowProtoHandler::DeleteEventReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteEventReq_T);

    auto req = message.mutable_reqvalue()->mutable_deleteeventreq_value();
    req->set_struserid(m_strUserID);
    req->set_streventid(m_strEventID);
}

void PassengerFlowProtoHandler::DeleteEventReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().deleteeventreq_value();
    m_strUserID = req.struserid();
    m_strEventID = req.streventid();
}

void PassengerFlowProtoHandler::DeleteEventRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteEventRsp_T);

    message.mutable_rspvalue()->mutable_deleteeventrsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::DeleteEventRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().deleteeventrsp_value().strvalue();
}

void PassengerFlowProtoHandler::ModifyEventReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyEventReq_T);

    auto req = message.mutable_reqvalue()->mutable_modifyeventreq_value();
    req->set_struserid(m_strUserID);

    auto event = req->mutable_eventinfo();
    event->set_streventid(m_eventInfo.m_strEventID);
    event->set_strsource(m_eventInfo.m_strSource);
    event->set_strsubmitdate(m_eventInfo.m_strSubmitDate);
    event->set_strexpiredate(m_eventInfo.m_strExpireDate);
    event->set_struserid(m_eventInfo.m_strUserID);
    event->set_strdeviceid(m_eventInfo.m_strDeviceID);
    event->set_strprocessstate(m_eventInfo.m_strProcessState);
    event->set_strremark(m_eventInfo.m_strRemark);
    event->set_uiviewstate(m_eventInfo.m_uiViewState);
    event->set_strcreatedate(m_eventInfo.m_strCreateDate);
    event->set_strextend(m_eventInfo.m_strExtend);
    event->set_uistate(m_eventInfo.m_uiState);

    for (auto it = m_eventInfo.m_uiTypeList.begin(), end = m_eventInfo.m_uiTypeList.end(); it != end; ++it)
    {
        event->add_uitype(*it);
    }

    for (auto it = m_eventInfo.m_strHandlerList.begin(), end = m_eventInfo.m_strHandlerList.end(); it != end; ++it)
    {
        event->add_strhandler(*it);
    }
}

void PassengerFlowProtoHandler::ModifyEventReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().modifyeventreq_value();
    m_strUserID = req.struserid();

    auto event = req.eventinfo();
    m_eventInfo.m_strEventID = event.streventid();
    m_eventInfo.m_strSource = event.strsource();
    m_eventInfo.m_strSubmitDate = event.strsubmitdate();
    m_eventInfo.m_strExpireDate = event.strexpiredate();
    m_eventInfo.m_strUserID = event.struserid();
    m_eventInfo.m_strDeviceID = event.strdeviceid();
    m_eventInfo.m_strProcessState = event.strprocessstate();
    m_eventInfo.m_strRemark = event.strremark();
    m_eventInfo.m_uiViewState = event.uiviewstate();
    m_eventInfo.m_strCreateDate = event.strcreatedate();
    m_eventInfo.m_strExtend = event.strextend();
    m_eventInfo.m_uiState = event.uistate();

    auto typeList = event.uitype();
    for (int i = 0, sz = typeList.size(); i < sz; ++i)
    {
        m_eventInfo.m_uiTypeList.push_back(typeList.Get(i));
    }

    auto respList = event.strhandler();
    for (int i = 0, sz = respList.size(); i < sz; ++i)
    {
        m_eventInfo.m_strHandlerList.push_back(respList.Get(i));
    }
}

void PassengerFlowProtoHandler::ModifyEventRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyEventRsp_T);

    message.mutable_rspvalue()->mutable_modifyeventrsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::ModifyEventRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().modifyeventrsp_value().strvalue();
}

void PassengerFlowProtoHandler::QueryEventInfoReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryEventInfoReq_T);
    auto req = message.mutable_reqvalue()->mutable_queryeventinforeq_value();
    req->set_struserid(m_strUserID);
    req->set_streventid(m_strEventID);
}

void PassengerFlowProtoHandler::QueryEventInfoReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().queryeventinforeq_value();
    m_strUserID = req.struserid();
    m_strEventID = req.streventid();
}

void PassengerFlowProtoHandler::QueryEventInfoRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryEventInfoRsp_T);

    auto event = message.mutable_rspvalue()->mutable_queryeventinforsp_value()->mutable_eventinfo();
    event->set_streventid(m_eventInfo.m_strEventID);
    event->set_strsource(m_eventInfo.m_strSource);
    event->set_strsubmitdate(m_eventInfo.m_strSubmitDate);
    event->set_strexpiredate(m_eventInfo.m_strExpireDate);
    event->set_struserid(m_eventInfo.m_strUserID);
    event->set_strdeviceid(m_eventInfo.m_strDeviceID);
    event->set_strprocessstate(m_eventInfo.m_strProcessState);
    event->set_strremark(m_eventInfo.m_strRemark);
    event->set_uiviewstate(m_eventInfo.m_uiViewState);
    event->set_strcreatedate(m_eventInfo.m_strCreateDate);
    event->set_strextend(m_eventInfo.m_strExtend);
    event->set_uistate(m_eventInfo.m_uiState);

    for (auto it = m_eventInfo.m_uiTypeList.begin(), end = m_eventInfo.m_uiTypeList.end(); it != end; ++it)
    {
        event->add_uitype(*it);
    }

    for (auto it = m_eventInfo.m_strHandlerList.begin(), end = m_eventInfo.m_strHandlerList.end(); it != end; ++it)
    {
        event->add_strhandler(*it);
    }
}

void PassengerFlowProtoHandler::QueryEventInfoRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    auto event = message.rspvalue().queryeventinforsp_value().eventinfo();
    m_eventInfo.m_strEventID = event.streventid();
    m_eventInfo.m_strSource = event.strsource();
    m_eventInfo.m_strSubmitDate = event.strsubmitdate();
    m_eventInfo.m_strExpireDate = event.strexpiredate();
    m_eventInfo.m_strUserID = event.struserid();
    m_eventInfo.m_strDeviceID = event.strdeviceid();
    m_eventInfo.m_strProcessState = event.strprocessstate();
    m_eventInfo.m_strRemark = event.strremark();
    m_eventInfo.m_uiViewState = event.uiviewstate();
    m_eventInfo.m_strCreateDate = event.strcreatedate();
    m_eventInfo.m_strExtend = event.strextend();
    m_eventInfo.m_uiState = event.uistate();

    auto typeList = event.uitype();
    for (int i = 0, sz = typeList.size(); i < sz; ++i)
    {
        m_eventInfo.m_uiTypeList.push_back(typeList.Get(i));
    }

    auto respList = event.strhandler();
    for (int i = 0, sz = respList.size(); i < sz; ++i)
    {
        m_eventInfo.m_strHandlerList.push_back(respList.Get(i));
    }
}

void PassengerFlowProtoHandler::QueryAllEventReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllEventReq_T);

    auto req = message.mutable_reqvalue()->mutable_queryalleventreq_value();
    req->set_struserid(m_strUserID);
    req->set_uiprocessstate(m_uiProcessState);
    req->set_strbegindate(m_strBeginDate);
    req->set_strenddate(m_strEndDate);
    req->set_uibeginindex(m_uiBeginIndex);
}

void PassengerFlowProtoHandler::QueryAllEventReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);

    auto req = message.reqvalue().queryalleventreq_value();
    m_strUserID = req.struserid();
    m_uiProcessState = req.uiprocessstate();
    m_strBeginDate = req.strbegindate();
    m_strEndDate = req.strenddate();
    m_uiBeginIndex = req.uibeginindex();
}

void PassengerFlowProtoHandler::QueryAllEventRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllEventRsp_T);

    auto rsp = message.mutable_rspvalue()->mutable_queryalleventrsp_value();
    for (auto it = m_eventList.begin(), end = m_eventList.end(); it != end; ++it)
    {
        auto event = rsp->add_eventinfo();
        event->set_streventid(it->m_strEventID);
        event->set_strsource(it->m_strSource);
        event->set_strsubmitdate(it->m_strSubmitDate);
        event->set_strexpiredate(it->m_strExpireDate);
        event->set_struserid(it->m_strUserID);
        event->set_strdeviceid(it->m_strDeviceID);
        event->set_strprocessstate(it->m_strProcessState);
        event->set_strremark(it->m_strRemark);
        event->set_uiviewstate(it->m_uiViewState);
        event->set_strcreatedate(it->m_strCreateDate);
        event->set_strextend(it->m_strExtend);
        event->set_uistate(it->m_uiState);

        for (auto type = it->m_uiTypeList.begin(), end = it->m_uiTypeList.end(); type != end; ++type)
        {
            event->add_uitype(*type);
        }

        for (auto resp = it->m_strHandlerList.begin(), end = it->m_strHandlerList.end(); resp != end; ++resp)
        {
            event->add_strhandler(*resp);
        }
    }
}

void PassengerFlowProtoHandler::QueryAllEventRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    auto rsp = message.rspvalue().queryalleventrsp_value();
    for (int i = 0, sz = rsp.eventinfo_size(); i < sz; ++i)
    {
        auto rspEvent = rsp.eventinfo(i);
        PassengerFlowProtoHandler::Event event;
        event.m_strEventID = rspEvent.streventid();
        event.m_strSource = rspEvent.strsource();
        event.m_strSubmitDate = rspEvent.strsubmitdate();
        event.m_strExpireDate = rspEvent.strexpiredate();
        event.m_strUserID = rspEvent.struserid();
        event.m_strDeviceID = rspEvent.strdeviceid();
        event.m_strProcessState = rspEvent.strprocessstate();
        event.m_strRemark = rspEvent.strremark();
        event.m_uiViewState = rspEvent.uiviewstate();
        event.m_strCreateDate = rspEvent.strcreatedate();
        event.m_strExtend = rspEvent.strextend();
        event.m_uiState = rspEvent.uistate();

        auto typeList = rspEvent.uitype();
        for (int i = 0, sz = typeList.size(); i < sz; ++i)
        {
            event.m_uiTypeList.push_back(typeList.Get(i));
        }

        auto respList = rspEvent.strhandler();
        for (int i = 0, sz = respList.size(); i < sz; ++i)
        {
            event.m_strHandlerList.push_back(respList.Get(i));
        }

        m_eventList.push_back(event);
    }
}

void PassengerFlowProtoHandler::AddSmartGuardStoreReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddSmartGuardStoreReq_T);

    auto req = message.mutable_reqvalue()->mutable_addsmartguardstorereq_value();
    req->set_struserid(m_strUserID);

    auto smartGuardStore = req->mutable_smartguardstore();
    smartGuardStore->set_strplanid(m_smartGuardStore.m_strPlanID);
    smartGuardStore->set_strstoreid(m_smartGuardStore.m_strStoreID);
    smartGuardStore->set_strstorename(m_smartGuardStore.m_strStoreName);
    smartGuardStore->set_strplanname(m_smartGuardStore.m_strPlanName);
    smartGuardStore->set_strenable(m_smartGuardStore.m_strEnable);
    smartGuardStore->set_strbegintime(m_smartGuardStore.m_strBeginTime);
    smartGuardStore->set_strendtime(m_smartGuardStore.m_strEndTime);
    smartGuardStore->set_strbegintime2(m_smartGuardStore.m_strBeginTime2);
    smartGuardStore->set_strendtime2(m_smartGuardStore.m_strEndTime2);
    smartGuardStore->set_strcreatedate(m_smartGuardStore.m_strCreateDate);
    smartGuardStore->set_uistate(m_smartGuardStore.m_uiState);

    for (auto it = m_smartGuardStore.m_strEntranceIDList.begin(), end = m_smartGuardStore.m_strEntranceIDList.end(); it != end; ++it)
    {
        smartGuardStore->add_strentranceid(*it);
    }
}

void PassengerFlowProtoHandler::AddSmartGuardStoreReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().addsmartguardstorereq_value();
    m_strUserID = req.struserid();

    auto smartGuardStore = req.smartguardstore();
    m_smartGuardStore.m_strPlanID = smartGuardStore.strplanid();
    m_smartGuardStore.m_strStoreID = smartGuardStore.strstoreid();
    m_smartGuardStore.m_strStoreName = smartGuardStore.strstorename();
    m_smartGuardStore.m_strPlanName = smartGuardStore.strplanname();
    m_smartGuardStore.m_strEnable = smartGuardStore.strenable();
    m_smartGuardStore.m_strBeginTime = smartGuardStore.strbegintime();
    m_smartGuardStore.m_strEndTime = smartGuardStore.strendtime();
    m_smartGuardStore.m_strBeginTime2 = smartGuardStore.strbegintime2();
    m_smartGuardStore.m_strEndTime2 = smartGuardStore.strendtime2();
    m_smartGuardStore.m_strCreateDate = smartGuardStore.strcreatedate();
    m_smartGuardStore.m_uiState = smartGuardStore.uistate();

    for (int i = 0, sz = smartGuardStore.strentranceid_size(); i < sz; ++i)
    {
        m_smartGuardStore.m_strEntranceIDList.push_back(smartGuardStore.strentranceid(i));
    }
}

void PassengerFlowProtoHandler::AddSmartGuardStoreRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddSmartGuardStoreRsp_T);
    message.mutable_rspvalue()->mutable_addsmartguardstorersp_value()->set_strplanid(m_strPlanID);
}

void PassengerFlowProtoHandler::AddSmartGuardStoreRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    m_strPlanID = message.rspvalue().addsmartguardstorersp_value().strplanid();
}

void PassengerFlowProtoHandler::DeleteSmartGuardStoreReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteSmartGuardStoreReq_T);

    auto req = message.mutable_reqvalue()->mutable_deletesmartguardstorereq_value();
    req->set_struserid(m_strUserID);
    req->set_strplanid(m_strPlanID);
}

void PassengerFlowProtoHandler::DeleteSmartGuardStoreReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().deletesmartguardstorereq_value();
    m_strUserID = req.struserid();
    m_strPlanID = req.strplanid();
}

void PassengerFlowProtoHandler::DeleteSmartGuardStoreRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteSmartGuardStoreRsp_T);

    message.mutable_rspvalue()->mutable_deletesmartguardstorersp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::DeleteSmartGuardStoreRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().deletesmartguardstorersp_value().strvalue();
}

void PassengerFlowProtoHandler::ModifySmartGuardStoreReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifySmartGuardStoreReq_T);

    auto req = message.mutable_reqvalue()->mutable_modifysmartguardstorereq_value();
    req->set_struserid(m_strUserID);

    auto smartGuardStore = req->mutable_smartguardstore();
    smartGuardStore->set_strplanid(m_smartGuardStore.m_strPlanID);
    smartGuardStore->set_strstoreid(m_smartGuardStore.m_strStoreID);
    smartGuardStore->set_strstorename(m_smartGuardStore.m_strStoreName);
    smartGuardStore->set_strplanname(m_smartGuardStore.m_strPlanName);
    smartGuardStore->set_strenable(m_smartGuardStore.m_strEnable);
    smartGuardStore->set_strbegintime(m_smartGuardStore.m_strBeginTime);
    smartGuardStore->set_strendtime(m_smartGuardStore.m_strEndTime);
    smartGuardStore->set_strbegintime2(m_smartGuardStore.m_strBeginTime2);
    smartGuardStore->set_strendtime2(m_smartGuardStore.m_strEndTime2);
    smartGuardStore->set_strcreatedate(m_smartGuardStore.m_strCreateDate);
    smartGuardStore->set_uistate(m_smartGuardStore.m_uiState);

    for (auto it = m_smartGuardStore.m_strEntranceIDList.begin(), end = m_smartGuardStore.m_strEntranceIDList.end(); it != end; ++it)
    {
        smartGuardStore->add_strentranceid(*it);
    }
}

void PassengerFlowProtoHandler::ModifySmartGuardStoreReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().modifysmartguardstorereq_value();
    m_strUserID = req.struserid();

    auto smartGuardStore = req.smartguardstore();
    m_smartGuardStore.m_strPlanID = smartGuardStore.strplanid();
    m_smartGuardStore.m_strStoreID = smartGuardStore.strstoreid();
    m_smartGuardStore.m_strStoreName = smartGuardStore.strstorename();
    m_smartGuardStore.m_strPlanName = smartGuardStore.strplanname();
    m_smartGuardStore.m_strEnable = smartGuardStore.strenable();
    m_smartGuardStore.m_strBeginTime = smartGuardStore.strbegintime();
    m_smartGuardStore.m_strEndTime = smartGuardStore.strendtime();
    m_smartGuardStore.m_strBeginTime2 = smartGuardStore.strbegintime2();
    m_smartGuardStore.m_strEndTime2 = smartGuardStore.strendtime2();
    m_smartGuardStore.m_strCreateDate = smartGuardStore.strcreatedate();
    m_smartGuardStore.m_uiState = smartGuardStore.uistate();

    for (int i = 0, sz = smartGuardStore.strentranceid_size(); i < sz; ++i)
    {
        m_smartGuardStore.m_strEntranceIDList.push_back(smartGuardStore.strentranceid(i));
    }
}

void PassengerFlowProtoHandler::ModifySmartGuardStoreRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifySmartGuardStoreRsp_T);

    message.mutable_rspvalue()->mutable_modifysmartguardstorersp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::ModifySmartGuardStoreRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().modifysmartguardstorersp_value().strvalue();
}

void PassengerFlowProtoHandler::QuerySmartGuardStoreInfoReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QuerySmartGuardStoreInfoReq_T);
    auto req = message.mutable_reqvalue()->mutable_querysmartguardstoreinforeq_value();
    req->set_struserid(m_strUserID);
    req->set_strplanid(m_strPlanID);
}

void PassengerFlowProtoHandler::QuerySmartGuardStoreInfoReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().querysmartguardstoreinforeq_value();
    m_strUserID = req.struserid();
    m_strPlanID = req.strplanid();
}

void PassengerFlowProtoHandler::QuerySmartGuardStoreInfoRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QuerySmartGuardStoreInfoRsp_T);

    auto smartGuardStore = message.mutable_rspvalue()->mutable_querysmartguardstoreinforsp_value()->mutable_smartguardstore();
    smartGuardStore->set_strplanid(m_smartGuardStore.m_strPlanID);
    smartGuardStore->set_strstoreid(m_smartGuardStore.m_strStoreID);
    smartGuardStore->set_strstorename(m_smartGuardStore.m_strStoreName);
    smartGuardStore->set_strplanname(m_smartGuardStore.m_strPlanName);
    smartGuardStore->set_strenable(m_smartGuardStore.m_strEnable);
    smartGuardStore->set_strbegintime(m_smartGuardStore.m_strBeginTime);
    smartGuardStore->set_strendtime(m_smartGuardStore.m_strEndTime);
    smartGuardStore->set_strbegintime2(m_smartGuardStore.m_strBeginTime2);
    smartGuardStore->set_strendtime2(m_smartGuardStore.m_strEndTime2);
    smartGuardStore->set_strcreatedate(m_smartGuardStore.m_strCreateDate);
    smartGuardStore->set_uistate(m_smartGuardStore.m_uiState);

    for (auto it = m_smartGuardStore.m_strEntranceIDList.begin(), end = m_smartGuardStore.m_strEntranceIDList.end(); it != end; ++it)
    {
        smartGuardStore->add_strentranceid(*it);
    }
}

void PassengerFlowProtoHandler::QuerySmartGuardStoreInfoRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    auto smartGuardStore = message.rspvalue().querysmartguardstoreinforsp_value().smartguardstore();
    m_smartGuardStore.m_strPlanID = smartGuardStore.strplanid();
    m_smartGuardStore.m_strStoreID = smartGuardStore.strstoreid();
    m_smartGuardStore.m_strStoreName = smartGuardStore.strstorename();
    m_smartGuardStore.m_strPlanName = smartGuardStore.strplanname();
    m_smartGuardStore.m_strEnable = smartGuardStore.strenable();
    m_smartGuardStore.m_strBeginTime = smartGuardStore.strbegintime();
    m_smartGuardStore.m_strEndTime = smartGuardStore.strendtime();
    m_smartGuardStore.m_strBeginTime2 = smartGuardStore.strbegintime2();
    m_smartGuardStore.m_strEndTime2 = smartGuardStore.strendtime2();
    m_smartGuardStore.m_strCreateDate = smartGuardStore.strcreatedate();
    m_smartGuardStore.m_uiState = smartGuardStore.uistate();

    for (int i = 0, sz = smartGuardStore.strentranceid_size(); i < sz; ++i)
    {
        m_smartGuardStore.m_strEntranceIDList.push_back(smartGuardStore.strentranceid(i));
    }
}

void PassengerFlowProtoHandler::QueryAllSmartGuardStoreReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllSmartGuardStoreReq_T);

    auto req = message.mutable_reqvalue()->mutable_queryallsmartguardstorereq_value();
    req->set_struserid(m_strUserID);
    req->set_strdeviceid(m_strDeviceID);
    req->set_uibeginindex(m_uiBeginIndex);
}

void PassengerFlowProtoHandler::QueryAllSmartGuardStoreReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);

    auto req = message.reqvalue().queryallsmartguardstorereq_value();
    m_strUserID = req.struserid();
    m_strDeviceID = req.strdeviceid();
    m_uiBeginIndex = req.uibeginindex();
}

void PassengerFlowProtoHandler::QueryAllSmartGuardStoreRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllSmartGuardStoreRsp_T);

    auto rsp = message.mutable_rspvalue()->mutable_queryallsmartguardstorersp_value();
    for (auto it = m_planList.begin(), end = m_planList.end(); it != end; ++it)
    {
        auto smartGuardStore = rsp->add_smartguardstore();
        smartGuardStore->set_strplanid(it->m_strPlanID);
        smartGuardStore->set_strstoreid(it->m_strStoreID);
        smartGuardStore->set_strstorename(it->m_strStoreName);
        smartGuardStore->set_strplanname(it->m_strPlanName);
        smartGuardStore->set_strenable(it->m_strEnable);
        smartGuardStore->set_strbegintime(it->m_strBeginTime);
        smartGuardStore->set_strendtime(it->m_strEndTime);
        smartGuardStore->set_strbegintime2(it->m_strBeginTime2);
        smartGuardStore->set_strendtime2(it->m_strEndTime2);
        smartGuardStore->set_strcreatedate(it->m_strCreateDate);
        smartGuardStore->set_uistate(it->m_uiState);

        for (auto entrance = it->m_strEntranceIDList.begin(), end = it->m_strEntranceIDList.end(); entrance != end; ++entrance)
        {
            smartGuardStore->add_strentranceid(*entrance);
        }
    }
}

void PassengerFlowProtoHandler::QueryAllSmartGuardStoreRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    auto rsp = message.rspvalue().queryallsmartguardstorersp_value();
    for (int i = 0, sz = rsp.smartguardstore_size(); i < sz; ++i)
    {
        auto rspSmartGuardStore = rsp.smartguardstore(i);
        PassengerFlowProtoHandler::SmartGuardStore smartGuardStore;
        smartGuardStore.m_strPlanID = rspSmartGuardStore.strplanid();
        smartGuardStore.m_strStoreID = rspSmartGuardStore.strstoreid();
        smartGuardStore.m_strStoreName = rspSmartGuardStore.strstorename();
        smartGuardStore.m_strPlanName = rspSmartGuardStore.strplanname();
        smartGuardStore.m_strEnable = rspSmartGuardStore.strenable();
        smartGuardStore.m_strBeginTime = rspSmartGuardStore.strbegintime();
        smartGuardStore.m_strEndTime = rspSmartGuardStore.strendtime();
        smartGuardStore.m_strBeginTime2 = rspSmartGuardStore.strbegintime2();
        smartGuardStore.m_strEndTime2 = rspSmartGuardStore.strendtime2();
        smartGuardStore.m_strCreateDate = rspSmartGuardStore.strcreatedate();
        smartGuardStore.m_uiState = rspSmartGuardStore.uistate();

        for (int i = 0, sz = rspSmartGuardStore.strentranceid_size(); i < sz; ++i)
        {
            smartGuardStore.m_strEntranceIDList.push_back(rspSmartGuardStore.strentranceid(i));
        }

        m_planList.push_back(smartGuardStore);
    }
}

void PassengerFlowProtoHandler::AddRegularPatrolReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddRegularPatrolReq_T);

    auto req = message.mutable_reqvalue()->mutable_addregularpatrolreq_value();
    req->set_struserid(m_strUserID);

    auto regularPatrol = req->mutable_regularpatrol();
    regularPatrol->set_strplanid(m_regularPatrol.m_strPlanID);
    regularPatrol->set_strplanname(m_regularPatrol.m_strPlanName);
    regularPatrol->set_strenable(m_regularPatrol.m_strEnable);
    regularPatrol->set_strcreatedate(m_regularPatrol.m_strCreateDate);
    regularPatrol->set_uistate(m_regularPatrol.m_uiState);
    regularPatrol->set_strextend(m_regularPatrol.m_strExtend);

    for (auto it = m_regularPatrol.m_strPatrolTimeList.begin(), end = m_regularPatrol.m_strPatrolTimeList.end(); it != end; ++it)
    {
        regularPatrol->add_strpatroltime(*it);
    }

    for (auto it = m_regularPatrol.m_strHandlerList.begin(), end = m_regularPatrol.m_strHandlerList.end(); it != end; ++it)
    {
        regularPatrol->add_strhandler(*it);
    }

    SerializePatrolStoreEntranceList(m_regularPatrol.m_storeEntranceList, regularPatrol->mutable_storeentrance());
}

void PassengerFlowProtoHandler::AddRegularPatrolReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().addregularpatrolreq_value();
    m_strUserID = req.struserid();

    auto regularPatrol = req.regularpatrol();
    m_regularPatrol.m_strPlanID = regularPatrol.strplanid();
    m_regularPatrol.m_strPlanName = regularPatrol.strplanname();
    m_regularPatrol.m_strEnable = regularPatrol.strenable();
    m_regularPatrol.m_strCreateDate = regularPatrol.strcreatedate();
    m_regularPatrol.m_uiState = regularPatrol.uistate();
    m_regularPatrol.m_strExtend = regularPatrol.strextend();

    for (int i = 0, sz = regularPatrol.strpatroltime_size(); i < sz; ++i)
    {
        m_regularPatrol.m_strPatrolTimeList.push_back(regularPatrol.strpatroltime(i));
    }

    for (int i = 0, sz = regularPatrol.strhandler_size(); i < sz; ++i)
    {
        m_regularPatrol.m_strHandlerList.push_back(regularPatrol.strhandler(i));
    }

    UnSerializePatrolStoreEntranceList(m_regularPatrol.m_storeEntranceList, regularPatrol.storeentrance());
}

void PassengerFlowProtoHandler::AddRegularPatrolRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddRegularPatrolRsp_T);
    message.mutable_rspvalue()->mutable_addregularpatrolrsp_value()->set_strplanid(m_strPlanID);
}

void PassengerFlowProtoHandler::AddRegularPatrolRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    m_strPlanID = message.rspvalue().addregularpatrolrsp_value().strplanid();
}

void PassengerFlowProtoHandler::DeleteRegularPatrolReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteRegularPatrolReq_T);

    auto req = message.mutable_reqvalue()->mutable_deleteregularpatrolreq_value();
    req->set_struserid(m_strUserID);
    req->set_strplanid(m_strPlanID);
}

void PassengerFlowProtoHandler::DeleteRegularPatrolReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().deleteregularpatrolreq_value();
    m_strUserID = req.struserid();
    m_strPlanID = req.strplanid();
}

void PassengerFlowProtoHandler::DeleteRegularPatrolRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteRegularPatrolRsp_T);

    message.mutable_rspvalue()->mutable_deleteregularpatrolrsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::DeleteRegularPatrolRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().deleteregularpatrolrsp_value().strvalue();
}

void PassengerFlowProtoHandler::ModifyRegularPatrolReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyRegularPatrolReq_T);

    auto req = message.mutable_reqvalue()->mutable_modifyregularpatrolreq_value();
    req->set_struserid(m_strUserID);

    auto regularPatrol = req->mutable_regularpatrol();
    regularPatrol->set_strplanid(m_regularPatrol.m_strPlanID);
    regularPatrol->set_strplanname(m_regularPatrol.m_strPlanName);
    regularPatrol->set_strenable(m_regularPatrol.m_strEnable);
    regularPatrol->set_strcreatedate(m_regularPatrol.m_strCreateDate);
    regularPatrol->set_uistate(m_regularPatrol.m_uiState);
    regularPatrol->set_strextend(m_regularPatrol.m_strExtend);

    for (auto it = m_regularPatrol.m_strPatrolTimeList.begin(), end = m_regularPatrol.m_strPatrolTimeList.end(); it != end; ++it)
    {
        regularPatrol->add_strpatroltime(*it);
    }

    for (auto it = m_regularPatrol.m_strHandlerList.begin(), end = m_regularPatrol.m_strHandlerList.end(); it != end; ++it)
    {
        regularPatrol->add_strhandler(*it);
    }

    SerializePatrolStoreEntranceList(m_regularPatrol.m_storeEntranceList, regularPatrol->mutable_storeentrance());
}

void PassengerFlowProtoHandler::ModifyRegularPatrolReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().modifyregularpatrolreq_value();
    m_strUserID = req.struserid();

    auto regularPatrol = req.regularpatrol();
    m_regularPatrol.m_strPlanID = regularPatrol.strplanid();
    m_regularPatrol.m_strPlanName = regularPatrol.strplanname();
    m_regularPatrol.m_strEnable = regularPatrol.strenable();
    m_regularPatrol.m_strCreateDate = regularPatrol.strcreatedate();
    m_regularPatrol.m_uiState = regularPatrol.uistate();
    m_regularPatrol.m_strExtend = regularPatrol.strextend();

    for (int i = 0, sz = regularPatrol.strpatroltime_size(); i < sz; ++i)
    {
        m_regularPatrol.m_strPatrolTimeList.push_back(regularPatrol.strpatroltime(i));
    }

    for (int i = 0, sz = regularPatrol.strhandler_size(); i < sz; ++i)
    {
        m_regularPatrol.m_strHandlerList.push_back(regularPatrol.strhandler(i));
    }

    UnSerializePatrolStoreEntranceList(m_regularPatrol.m_storeEntranceList, regularPatrol.storeentrance());
}

void PassengerFlowProtoHandler::ModifyRegularPatrolRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyRegularPatrolRsp_T);

    message.mutable_rspvalue()->mutable_modifyregularpatrolrsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::ModifyRegularPatrolRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().modifyregularpatrolrsp_value().strvalue();
}

void PassengerFlowProtoHandler::QueryRegularPatrolInfoReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryRegularPatrolInfoReq_T);
    auto req = message.mutable_reqvalue()->mutable_queryregularpatrolinforeq_value();
    req->set_struserid(m_strUserID);
    req->set_strplanid(m_strPlanID);
}

void PassengerFlowProtoHandler::QueryRegularPatrolInfoReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().queryregularpatrolinforeq_value();
    m_strUserID = req.struserid();
    m_strPlanID = req.strplanid();
}

void PassengerFlowProtoHandler::QueryRegularPatrolInfoRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryRegularPatrolInfoRsp_T);

    auto regularPatrol = message.mutable_rspvalue()->mutable_queryregularpatrolinforsp_value()->mutable_regularpatrol();
    regularPatrol->set_strplanid(m_regularPatrol.m_strPlanID);
    regularPatrol->set_strplanname(m_regularPatrol.m_strPlanName);
    regularPatrol->set_strenable(m_regularPatrol.m_strEnable);
    regularPatrol->set_strcreatedate(m_regularPatrol.m_strCreateDate);
    regularPatrol->set_uistate(m_regularPatrol.m_uiState);
    regularPatrol->set_strextend(m_regularPatrol.m_strExtend);

    for (auto it = m_regularPatrol.m_strPatrolTimeList.begin(), end = m_regularPatrol.m_strPatrolTimeList.end(); it != end; ++it)
    {
        regularPatrol->add_strpatroltime(*it);
    }

    for (auto it = m_regularPatrol.m_strHandlerList.begin(), end = m_regularPatrol.m_strHandlerList.end(); it != end; ++it)
    {
        regularPatrol->add_strhandler(*it);
    }

    SerializePatrolStoreEntranceList(m_regularPatrol.m_storeEntranceList, regularPatrol->mutable_storeentrance());
}

void PassengerFlowProtoHandler::QueryRegularPatrolInfoRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    auto regularPatrol = message.rspvalue().queryregularpatrolinforsp_value().regularpatrol();
    m_regularPatrol.m_strPlanID = regularPatrol.strplanid();
    m_regularPatrol.m_strPlanName = regularPatrol.strplanname();
    m_regularPatrol.m_strEnable = regularPatrol.strenable();
    m_regularPatrol.m_strCreateDate = regularPatrol.strcreatedate();
    m_regularPatrol.m_uiState = regularPatrol.uistate();
    m_regularPatrol.m_strExtend = regularPatrol.strextend();

    for (int i = 0, sz = regularPatrol.strpatroltime_size(); i < sz; ++i)
    {
        m_regularPatrol.m_strPatrolTimeList.push_back(regularPatrol.strpatroltime(i));
    }

    for (int i = 0, sz = regularPatrol.strhandler_size(); i < sz; ++i)
    {
        m_regularPatrol.m_strHandlerList.push_back(regularPatrol.strhandler(i));
    }

    UnSerializePatrolStoreEntranceList(m_regularPatrol.m_storeEntranceList, regularPatrol.storeentrance());
}

void PassengerFlowProtoHandler::QueryAllRegularPatrolReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllRegularPatrolReq_T);

    auto req = message.mutable_reqvalue()->mutable_queryallregularpatrolreq_value();
    req->set_struserid(m_strUserID);
    req->set_strdeviceid(m_strDeviceID);
    req->set_uibeginindex(m_uiBeginIndex);
}

void PassengerFlowProtoHandler::QueryAllRegularPatrolReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);

    auto req = message.reqvalue().queryallregularpatrolreq_value();
    m_strUserID = req.struserid();
    m_strDeviceID = req.strdeviceid();
    m_uiBeginIndex = req.uibeginindex();
}

void PassengerFlowProtoHandler::QueryAllRegularPatrolRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllRegularPatrolRsp_T);

    auto rsp = message.mutable_rspvalue()->mutable_queryallregularpatrolrsp_value();
    for (auto it = m_planList.begin(), end = m_planList.end(); it != end; ++it)
    {
        auto regularPatrol = rsp->add_regularpatrol();
        regularPatrol->set_strplanid(it->m_strPlanID);
        regularPatrol->set_strplanname(it->m_strPlanName);
        regularPatrol->set_strenable(it->m_strEnable);
        regularPatrol->set_strcreatedate(it->m_strCreateDate);
        regularPatrol->set_uistate(it->m_uiState);
        regularPatrol->set_strextend(it->m_strExtend);

        for (auto time = it->m_strPatrolTimeList.begin(), end = it->m_strPatrolTimeList.end(); time != end; ++time)
        {
            regularPatrol->add_strpatroltime(*time);
        }

        for (auto handler = it->m_strHandlerList.begin(), end = it->m_strHandlerList.end(); handler != end; ++handler)
        {
            regularPatrol->add_strhandler(*handler);
        }

        SerializePatrolStoreEntranceList(it->m_storeEntranceList, regularPatrol->mutable_storeentrance());
    }
}

void PassengerFlowProtoHandler::QueryAllRegularPatrolRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    auto rsp = message.rspvalue().queryallregularpatrolrsp_value();
    for (int i = 0, sz = rsp.regularpatrol_size(); i < sz; ++i)
    {
        auto rspRegularPatrol = rsp.regularpatrol(i);
        PassengerFlowProtoHandler::RegularPatrol regularPatrol;
        regularPatrol.m_strPlanID = rspRegularPatrol.strplanid();
        regularPatrol.m_strPlanName = rspRegularPatrol.strplanname();
        regularPatrol.m_strEnable = rspRegularPatrol.strenable();
        regularPatrol.m_strCreateDate = rspRegularPatrol.strcreatedate();
        regularPatrol.m_uiState = rspRegularPatrol.uistate();
        regularPatrol.m_strExtend = rspRegularPatrol.strextend();

        for (int i = 0, sz = rspRegularPatrol.strpatroltime_size(); i < sz; ++i)
        {
            regularPatrol.m_strPatrolTimeList.push_back(rspRegularPatrol.strpatroltime(i));
        }

        for (int i = 0, sz = rspRegularPatrol.strhandler_size(); i < sz; ++i)
        {
            regularPatrol.m_strHandlerList.push_back(rspRegularPatrol.strhandler(i));
        }

        UnSerializePatrolStoreEntranceList(regularPatrol.m_storeEntranceList, rspRegularPatrol.storeentrance());

        m_planList.push_back(regularPatrol);
    }
}

void PassengerFlowProtoHandler::UserJoinStoreReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::UserJoinStoreReq_T);

    auto req = message.mutable_reqvalue()->mutable_userjoinstorereq_value();
    req->set_struserid(m_strUserID);
    req->set_strstoreid(m_strStoreID);
    req->set_strrole(m_strRole);
}

void PassengerFlowProtoHandler::UserJoinStoreReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().userjoinstorereq_value();
    m_strUserID = req.struserid();
    m_strStoreID = req.strstoreid();
    m_strRole = req.strrole();
}

void PassengerFlowProtoHandler::UserJoinStoreRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::UserJoinStoreRsp_T);

    message.mutable_rspvalue()->mutable_userjoinstorersp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::UserJoinStoreRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().userjoinstorersp_value().strvalue();
}

void PassengerFlowProtoHandler::UserQuitStoreReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::UserQuitStoreReq_T);

    auto req = message.mutable_reqvalue()->mutable_userquitstorereq_value();
    req->set_stradministratorid(m_strAdministratorID);
    req->set_struserid(m_strUserID);
    req->set_strstoreid(m_strStoreID);
}

void PassengerFlowProtoHandler::UserQuitStoreReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().userquitstorereq_value();
    m_strAdministratorID = req.stradministratorid();
    m_strUserID = req.struserid();
    m_strStoreID = req.strstoreid();
}

void PassengerFlowProtoHandler::UserQuitStoreRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::UserQuitStoreRsp_T);

    message.mutable_rspvalue()->mutable_userquitstorersp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::UserQuitStoreRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().userquitstorersp_value().strvalue();
}

void PassengerFlowProtoHandler::QueryStoreAllUserReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryStoreAllUserReq_T);

    auto req = message.mutable_reqvalue()->mutable_querystorealluserreq_value();
    req->set_struserid(m_strUserID);
    req->set_strstoreid(m_strStoreID);
}

void PassengerFlowProtoHandler::QueryStoreAllUserReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().querystorealluserreq_value();
    m_strUserID = req.struserid();
    m_strStoreID = req.strstoreid();
}

void PassengerFlowProtoHandler::QueryStoreAllUserRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryStoreAllUserRsp_T);

    auto rsp = message.mutable_rspvalue()->mutable_querystorealluserrsp_value();
    for (auto it = m_userList.begin(), end = m_userList.end(); it != end; ++it)
    {
        auto user = rsp->add_user();
        user->set_struserid(it->m_strUserID);
        user->set_strusername(it->m_strUserName);
        user->set_strrole(it->m_strRole);
    }
}

void PassengerFlowProtoHandler::QueryStoreAllUserRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    auto rsp = message.rspvalue().querystorealluserrsp_value();
    for (int i = 0, sz = rsp.user_size(); i < sz; ++i)
    {
        auto rspUser = rsp.user(i);
        PassengerFlowProtoHandler::UserBrief user;
        user.m_strUserID = rspUser.struserid();
        user.m_strUserName = rspUser.strusername();
        user.m_strRole = rspUser.strrole();

        m_userList.push_back(user);
    }
}

void PassengerFlowProtoHandler::AddVIPCustomerReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddVIPCustomerReq_T);

    auto req = message.mutable_reqvalue()->mutable_addvipcustomerreq_value();
    req->set_struserid(m_strUserID);

    auto customerInfo = req->mutable_customerinfo();
    customerInfo->set_strvipid(m_customerInfo.m_strVisitDate);
    customerInfo->set_strprofilepicture(m_customerInfo.m_strProfilePicture);
    customerInfo->set_strvipname(m_customerInfo.m_strVIPName);
    customerInfo->set_strcellphone(m_customerInfo.m_strCellphone);
    customerInfo->set_strvisitdate(m_customerInfo.m_strVisitDate);
    customerInfo->set_uivisittimes(m_customerInfo.m_uiVisitTimes);
    customerInfo->set_strregisterdate(m_customerInfo.m_strRegisterDate);
    customerInfo->set_uistate(m_customerInfo.m_uiState);
}

void PassengerFlowProtoHandler::AddVIPCustomerReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().addvipcustomerreq_value();
    m_strUserID = req.struserid();

    auto customerInfo = req.customerinfo();
    m_customerInfo.m_strVIPID = customerInfo.strvipid();
    m_customerInfo.m_strProfilePicture = customerInfo.strprofilepicture();
    m_customerInfo.m_strVIPName = customerInfo.strvipname();
    m_customerInfo.m_strCellphone = customerInfo.strcellphone();
    m_customerInfo.m_strVisitDate = customerInfo.strvisitdate();
    m_customerInfo.m_uiVisitTimes = customerInfo.uivisittimes();
    m_customerInfo.m_strRegisterDate = customerInfo.strregisterdate();
    m_customerInfo.m_uiState = customerInfo.uistate();
}

void PassengerFlowProtoHandler::AddVIPCustomerRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddVIPCustomerRsp_T);
    message.mutable_rspvalue()->mutable_addvipcustomerrsp_value()->set_strvipid(m_strVIPID);
}

void PassengerFlowProtoHandler::AddVIPCustomerRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    m_strVIPID = message.rspvalue().addvipcustomerrsp_value().strvipid();
}

void PassengerFlowProtoHandler::DeleteVIPCustomerReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteVIPCustomerReq_T);

    auto req = message.mutable_reqvalue()->mutable_deletevipcustomerreq_value();
    req->set_struserid(m_strUserID);
    req->set_strvipid(m_strVIPID);
}

void PassengerFlowProtoHandler::DeleteVIPCustomerReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().deletevipcustomerreq_value();
    m_strUserID = req.struserid();
    m_strVIPID = req.strvipid();
}

void PassengerFlowProtoHandler::DeleteVIPCustomerRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteVIPCustomerRsp_T);

    message.mutable_rspvalue()->mutable_deletevipcustomerrsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::DeleteVIPCustomerRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().deletevipcustomerrsp_value().strvalue();
}

void PassengerFlowProtoHandler::ModifyVIPCustomerReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyVIPCustomerReq_T);

    auto req = message.mutable_reqvalue()->mutable_modifyvipcustomerreq_value();
    req->set_struserid(m_strUserID);

    auto customerInfo = req->mutable_customerinfo();
    customerInfo->set_strvipid(m_customerInfo.m_strVIPID);
    customerInfo->set_strprofilepicture(m_customerInfo.m_strProfilePicture);
    customerInfo->set_strvipname(m_customerInfo.m_strVIPName);
    customerInfo->set_strcellphone(m_customerInfo.m_strCellphone);
    customerInfo->set_strvisitdate(m_customerInfo.m_strVisitDate);
    customerInfo->set_uivisittimes(m_customerInfo.m_uiVisitTimes);
    customerInfo->set_strregisterdate(m_customerInfo.m_strRegisterDate);
    customerInfo->set_uistate(m_customerInfo.m_uiState);
}

void PassengerFlowProtoHandler::ModifyVIPCustomerReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().modifyvipcustomerreq_value();
    m_strUserID = req.struserid();

    auto customerInfo = req.customerinfo();
    m_customerInfo.m_strVIPID = customerInfo.strvipid();
    m_customerInfo.m_strProfilePicture = customerInfo.strprofilepicture();
    m_customerInfo.m_strVIPName = customerInfo.strvipname();
    m_customerInfo.m_strCellphone = customerInfo.strcellphone();
    m_customerInfo.m_strVisitDate = customerInfo.strvisitdate();
    m_customerInfo.m_uiVisitTimes = customerInfo.uivisittimes();
    m_customerInfo.m_strRegisterDate = customerInfo.strregisterdate();
    m_customerInfo.m_uiState = customerInfo.uistate();
}

void PassengerFlowProtoHandler::ModifyVIPCustomerRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyVIPCustomerRsp_T);

    message.mutable_rspvalue()->mutable_modifyvipcustomerrsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::ModifyVIPCustomerRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().modifyvipcustomerrsp_value().strvalue();
}

void PassengerFlowProtoHandler::QueryVIPCustomerInfoReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryVIPCustomerInfoReq_T);
    auto req = message.mutable_reqvalue()->mutable_queryvipcustomerinforeq_value();
    req->set_struserid(m_strUserID);
    req->set_strvipid(m_strVIPID);
}

void PassengerFlowProtoHandler::QueryVIPCustomerInfoReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().queryvipcustomerinforeq_value();
    m_strUserID = req.struserid();
    m_strVIPID = req.strvipid();
}

void PassengerFlowProtoHandler::QueryVIPCustomerInfoRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryVIPCustomerInfoRsp_T);

    auto customerInfo = message.mutable_rspvalue()->mutable_queryvipcustomerinforsp_value()->mutable_customerinfo();
    customerInfo->set_strvipid(m_customerInfo.m_strVisitDate);
    customerInfo->set_strprofilepicture(m_customerInfo.m_strProfilePicture);
    customerInfo->set_strvipname(m_customerInfo.m_strVIPName);
    customerInfo->set_strcellphone(m_customerInfo.m_strCellphone);
    customerInfo->set_strvisitdate(m_customerInfo.m_strVisitDate);
    customerInfo->set_uivisittimes(m_customerInfo.m_uiVisitTimes);
    customerInfo->set_strregisterdate(m_customerInfo.m_strRegisterDate);
    customerInfo->set_uistate(m_customerInfo.m_uiState);

    SerializeVIPConsumeHistoryList(m_customerInfo.m_consumeHistoryList, customerInfo->mutable_consumehistory());
}

void PassengerFlowProtoHandler::QueryVIPCustomerInfoRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    auto customerInfo = message.rspvalue().queryvipcustomerinforsp_value().customerinfo();
    m_customerInfo.m_strVIPID = customerInfo.strvipid();
    m_customerInfo.m_strProfilePicture = customerInfo.strprofilepicture();
    m_customerInfo.m_strVIPName = customerInfo.strvipname();
    m_customerInfo.m_strCellphone = customerInfo.strcellphone();
    m_customerInfo.m_strVisitDate = customerInfo.strvisitdate();
    m_customerInfo.m_uiVisitTimes = customerInfo.uivisittimes();
    m_customerInfo.m_strRegisterDate = customerInfo.strregisterdate();
    m_customerInfo.m_uiState = customerInfo.uistate();

    UnSerializeVIPConsumeHistoryList(m_customerInfo.m_consumeHistoryList, customerInfo.consumehistory());
}

void PassengerFlowProtoHandler::QueryAllVIPCustomerReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllVIPCustomerReq_T);

    auto req = message.mutable_reqvalue()->mutable_queryallvipcustomerreq_value();
    req->set_struserid(m_strUserID);
    req->set_uibeginindex(m_uiBeginIndex);
}

void PassengerFlowProtoHandler::QueryAllVIPCustomerReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);

    auto req = message.reqvalue().queryallvipcustomerreq_value();
    m_strUserID = req.struserid();
    m_uiBeginIndex = req.uibeginindex();
}

void PassengerFlowProtoHandler::QueryAllVIPCustomerRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllVIPCustomerRsp_T);

    SerializeVIPCustomerList(m_customerList, message.mutable_rspvalue()->mutable_queryallvipcustomerrsp_value()->mutable_customer());
}

void PassengerFlowProtoHandler::QueryAllVIPCustomerRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    UnSerializeVIPCustomerList(m_customerList, message.rspvalue().queryallvipcustomerrsp_value().customer());
}

void PassengerFlowProtoHandler::AddVIPConsumeHistoryReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddVIPConsumeHistoryReq_T);

    auto req = message.mutable_reqvalue()->mutable_addvipconsumehistoryreq_value();
    req->set_struserid(m_strUserID);

    auto consumeHistory = req->mutable_consumehistory();
    consumeHistory->set_strconsumeid(m_consumeHistory.m_strConsumeID);
    consumeHistory->set_strvipid(m_consumeHistory.m_strVIPID);
    consumeHistory->set_strgoodsname(m_consumeHistory.m_strGoodsName);
    consumeHistory->set_uigoodsnumber(m_consumeHistory.m_uiGoodsNumber);
    consumeHistory->set_strsalesman(m_consumeHistory.m_strSalesman);
    consumeHistory->set_dconsumeamount(m_consumeHistory.m_dConsumeAmount);
    consumeHistory->set_strconsumedate(m_consumeHistory.m_strConsumeDate);
}

void PassengerFlowProtoHandler::AddVIPConsumeHistoryReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().addvipconsumehistoryreq_value();
    m_strUserID = req.struserid();

    auto consumeHistory = req.consumehistory();
    m_consumeHistory.m_strConsumeID = consumeHistory.strconsumeid();
    m_consumeHistory.m_strVIPID = consumeHistory.strvipid();
    m_consumeHistory.m_strGoodsName = consumeHistory.strgoodsname();
    m_consumeHistory.m_uiGoodsNumber = consumeHistory.uigoodsnumber();
    m_consumeHistory.m_strSalesman = consumeHistory.strsalesman();
    m_consumeHistory.m_dConsumeAmount = consumeHistory.dconsumeamount();
    m_consumeHistory.m_strConsumeDate = consumeHistory.strconsumedate();
}

void PassengerFlowProtoHandler::AddVIPConsumeHistoryRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddVIPConsumeHistoryRsp_T);
    message.mutable_rspvalue()->mutable_addvipconsumehistoryrsp_value()->set_strconsumeid(m_strConsumeID);
}

void PassengerFlowProtoHandler::AddVIPConsumeHistoryRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    m_strConsumeID = message.rspvalue().addvipconsumehistoryrsp_value().strconsumeid();
}

void PassengerFlowProtoHandler::DeleteVIPConsumeHistoryReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteVIPConsumeHistoryReq_T);

    auto req = message.mutable_reqvalue()->mutable_deletevipconsumehistoryreq_value();
    req->set_struserid(m_strUserID);
    req->set_strconsumeid(m_strConsumeID);
}

void PassengerFlowProtoHandler::DeleteVIPConsumeHistoryReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().deletevipconsumehistoryreq_value();
    m_strUserID = req.struserid();
    m_strConsumeID = req.strconsumeid();
}

void PassengerFlowProtoHandler::DeleteVIPConsumeHistoryRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteVIPConsumeHistoryRsp_T);

    message.mutable_rspvalue()->mutable_deletevipconsumehistoryrsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::DeleteVIPConsumeHistoryRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().deletevipconsumehistoryrsp_value().strvalue();
}

void PassengerFlowProtoHandler::ModifyVIPConsumeHistoryReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyVIPConsumeHistoryReq_T);

    auto req = message.mutable_reqvalue()->mutable_modifyvipconsumehistoryreq_value();
    req->set_struserid(m_strUserID);

    auto consumeHistory = req->mutable_consumehistory();
    consumeHistory->set_strvipid(m_consumeHistory.m_strVIPID);
    consumeHistory->set_strconsumeid(m_consumeHistory.m_strConsumeID);
    consumeHistory->set_strvipid(m_consumeHistory.m_strVIPID);
    consumeHistory->set_strgoodsname(m_consumeHistory.m_strGoodsName);
    consumeHistory->set_uigoodsnumber(m_consumeHistory.m_uiGoodsNumber);
    consumeHistory->set_strsalesman(m_consumeHistory.m_strSalesman);
    consumeHistory->set_dconsumeamount(m_consumeHistory.m_dConsumeAmount);
    consumeHistory->set_strconsumedate(m_consumeHistory.m_strConsumeDate);
}

void PassengerFlowProtoHandler::ModifyVIPConsumeHistoryReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().modifyvipconsumehistoryreq_value();
    m_strUserID = req.struserid();

    auto consumeHistory = req.consumehistory();
    m_consumeHistory.m_strVIPID = consumeHistory.strvipid();
    m_consumeHistory.m_strConsumeID = consumeHistory.strconsumeid();
    m_consumeHistory.m_strVIPID = consumeHistory.strvipid();
    m_consumeHistory.m_strGoodsName = consumeHistory.strgoodsname();
    m_consumeHistory.m_uiGoodsNumber = consumeHistory.uigoodsnumber();
    m_consumeHistory.m_strSalesman = consumeHistory.strsalesman();
    m_consumeHistory.m_dConsumeAmount = consumeHistory.dconsumeamount();
    m_consumeHistory.m_strConsumeDate = consumeHistory.strconsumedate();
}

void PassengerFlowProtoHandler::ModifyVIPConsumeHistoryRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyVIPConsumeHistoryRsp_T);

    message.mutable_rspvalue()->mutable_modifyvipconsumehistoryrsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::ModifyVIPConsumeHistoryRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().modifyvipconsumehistoryrsp_value().strvalue();
}

void PassengerFlowProtoHandler::QueryAllVIPConsumeHistoryReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllVIPConsumeHistoryReq_T);

    auto req = message.mutable_reqvalue()->mutable_queryallvipconsumehistoryreq_value();
    req->set_struserid(m_strUserID);
    req->set_strvipid(m_strVIPID);
    req->set_uibeginindex(m_uiBeginIndex);
}

void PassengerFlowProtoHandler::QueryAllVIPConsumeHistoryReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);

    auto req = message.reqvalue().queryallvipconsumehistoryreq_value();
    m_strUserID = req.struserid();
    m_strVIPID = req.strvipid();
    m_uiBeginIndex = req.uibeginindex();
}

void PassengerFlowProtoHandler::QueryAllVIPConsumeHistoryRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllVIPConsumeHistoryRsp_T);

    SerializeVIPConsumeHistoryList(m_consumeHistoryList, message.mutable_rspvalue()->mutable_queryallvipconsumehistoryrsp_value()->mutable_consumehistory());
}

void PassengerFlowProtoHandler::QueryAllVIPConsumeHistoryRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    UnSerializeVIPConsumeHistoryList(m_consumeHistoryList, message.rspvalue().queryallvipconsumehistoryrsp_value().consumehistory());
}

void PassengerFlowProtoHandler::AddEvaluationTemplateReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddEvaluationTemplateReq_T);

    auto req = message.mutable_reqvalue()->mutable_addevaluationtemplatereq_value();
    req->set_struserid(m_strUserID);

    auto item = req->mutable_evaluationitem();
    item->set_stritemid(m_evaluationItem.m_strItemID);
    item->set_stritemname(m_evaluationItem.m_strItemName);
    item->set_strdescription(m_evaluationItem.m_strDescription);
    item->set_dtotalpoint(m_evaluationItem.m_dTotalPoint);
}

void PassengerFlowProtoHandler::AddEvaluationTemplateReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().addevaluationtemplatereq_value();
    m_strUserID = req.struserid();

    auto item = req.evaluationitem();
    m_evaluationItem.m_strItemID = item.stritemid();
    m_evaluationItem.m_strItemName = item.stritemname();
    m_evaluationItem.m_strDescription = item.strdescription();
    m_evaluationItem.m_dTotalPoint = item.dtotalpoint();
}

void PassengerFlowProtoHandler::AddEvaluationTemplateRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddEvaluationTemplateRsp_T);
    message.mutable_rspvalue()->mutable_addevaluationtemplatersp_value()->set_strevaluationid(m_strEvaluationID);
}

void PassengerFlowProtoHandler::AddEvaluationTemplateRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    m_strEvaluationID = message.rspvalue().addevaluationtemplatersp_value().strevaluationid();
}

void PassengerFlowProtoHandler::DeleteEvaluationTemplateReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteEvaluationTemplateReq_T);

    auto req = message.mutable_reqvalue()->mutable_deleteevaluationtemplatereq_value();
    req->set_struserid(m_strUserID);
    req->set_strevaluationid(m_strEvaluationID);
}

void PassengerFlowProtoHandler::DeleteEvaluationTemplateReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().deleteevaluationtemplatereq_value();
    m_strUserID = req.struserid();
    m_strEvaluationID = req.strevaluationid();
}

void PassengerFlowProtoHandler::DeleteEvaluationTemplateRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteEvaluationTemplateRsp_T);

    message.mutable_rspvalue()->mutable_deleteevaluationtemplatersp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::DeleteEvaluationTemplateRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().deleteevaluationtemplatersp_value().strvalue();
}

void PassengerFlowProtoHandler::ModifyEvaluationTemplateReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyEvaluationTemplateReq_T);

    auto req = message.mutable_reqvalue()->mutable_modifyevaluationtemplatereq_value();
    req->set_struserid(m_strUserID);

    auto item = req->mutable_evaluationitem();
    item->set_stritemid(m_evaluationItem.m_strItemID);
    item->set_stritemname(m_evaluationItem.m_strItemName);
    item->set_strdescription(m_evaluationItem.m_strDescription);
    item->set_dtotalpoint(m_evaluationItem.m_dTotalPoint);
}

void PassengerFlowProtoHandler::ModifyEvaluationTemplateReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().modifyevaluationtemplatereq_value();
    m_strUserID = req.struserid();

    auto item = req.evaluationitem();
    m_evaluationItem.m_strItemID = item.stritemid();
    m_evaluationItem.m_strItemName = item.stritemname();
    m_evaluationItem.m_strDescription = item.strdescription();
    m_evaluationItem.m_dTotalPoint = item.dtotalpoint();
}

void PassengerFlowProtoHandler::ModifyEvaluationTemplateRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyEvaluationTemplateRsp_T);

    message.mutable_rspvalue()->mutable_modifyevaluationtemplatersp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::ModifyEvaluationTemplateRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().modifyevaluationtemplatersp_value().strvalue();
}

void PassengerFlowProtoHandler::QueryAllEvaluationTemplateReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllEvaluationTemplateReq_T);

    auto req = message.mutable_reqvalue()->mutable_queryallevaluationtemplatereq_value();
    req->set_struserid(m_strUserID);
}

void PassengerFlowProtoHandler::QueryAllEvaluationTemplateReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);

    auto req = message.reqvalue().queryallevaluationtemplatereq_value();
    m_strUserID = req.struserid();
}

void PassengerFlowProtoHandler::QueryAllEvaluationTemplateRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllEvaluationTemplateRsp_T);

    SerializeEvaluationTemplateList(m_evaluationItemList, message.mutable_rspvalue()->mutable_queryallevaluationtemplatersp_value()->mutable_evaluationitem());
}

void PassengerFlowProtoHandler::QueryAllEvaluationTemplateRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    UnSerializeEvaluationTemplateList(m_evaluationItemList, message.rspvalue().queryallevaluationtemplatersp_value().evaluationitem());
}

void PassengerFlowProtoHandler::AddStoreEvaluationReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddStoreEvaluationReq_T);

    auto evaluation = message.mutable_reqvalue()->mutable_addstoreevaluationreq_value()->mutable_storeevaluation();
    evaluation->set_strevaluationid(m_storeEvaluation.m_strEvaluationID);
    evaluation->set_strstoreid(m_storeEvaluation.m_strStoreID);
    evaluation->set_struseridcreate(m_storeEvaluation.m_strUserIDCreate);
    evaluation->set_struseridcheck(m_storeEvaluation.m_strUserIDCheck);
    evaluation->set_dtotalscore(m_storeEvaluation.m_dTotalScore);
    evaluation->set_uicheckstatus(m_storeEvaluation.m_uiCheckStatus);
    evaluation->set_strcreatedate(m_storeEvaluation.m_strCreateDate);

    for (auto it = m_storeEvaluation.m_itemScoreList.begin(), end = m_storeEvaluation.m_itemScoreList.end(); it != end; ++it)
    {
        auto score = evaluation->add_itemscore();
        score->set_dscore(it->m_dScore);
        score->set_strdescription(it->m_strDescription);

        auto item = score->mutable_evaluationitem();
        item->set_stritemid(it->m_evaluationItem.m_strItemID);
        item->set_stritemname(it->m_evaluationItem.m_strItemName);
        item->set_dtotalpoint(it->m_evaluationItem.m_dTotalPoint);
        item->set_strdescription(it->m_evaluationItem.m_strDescription);
    }
}

void PassengerFlowProtoHandler::AddStoreEvaluationReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);

    auto evaluation = message.reqvalue().addstoreevaluationreq_value().storeevaluation();
    m_storeEvaluation.m_strEvaluationID = evaluation.strevaluationid();
    m_storeEvaluation.m_strStoreID = evaluation.strstoreid();
    m_storeEvaluation.m_strUserIDCreate = evaluation.struseridcreate();
    m_storeEvaluation.m_strUserIDCheck = evaluation.struseridcheck();
    m_storeEvaluation.m_dTotalScore = evaluation.dtotalscore();
    m_storeEvaluation.m_uiCheckStatus = evaluation.uicheckstatus();
    m_storeEvaluation.m_strCreateDate = evaluation.strcreatedate();

    for (int i = 0, sz = evaluation.itemscore_size(); i < sz; ++i)
    {
        auto rspScore = evaluation.itemscore(i);
        auto rspItem = rspScore.evaluationitem();

        PassengerFlowProtoHandler::EvaluationItemScore score;
        score.m_dScore = rspScore.dscore();
        score.m_strDescription = rspScore.strdescription();
        score.m_evaluationItem.m_strItemID = rspItem.stritemid();
        score.m_evaluationItem.m_strItemName = rspItem.stritemname();
        score.m_evaluationItem.m_dTotalPoint = rspItem.dtotalpoint();
        score.m_evaluationItem.m_strDescription = rspItem.strdescription();

        m_storeEvaluation.m_itemScoreList.push_back(score);
    }
}

void PassengerFlowProtoHandler::AddStoreEvaluationRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddStoreEvaluationRsp_T);
    message.mutable_rspvalue()->mutable_addstoreevaluationrsp_value()->set_strevaluationid(m_strEvaluationID);
}

void PassengerFlowProtoHandler::AddStoreEvaluationRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    m_strEvaluationID = message.rspvalue().addstoreevaluationrsp_value().strevaluationid();
}

void PassengerFlowProtoHandler::DeleteStoreEvaluationReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteStoreEvaluationReq_T);

    auto req = message.mutable_reqvalue()->mutable_deletestoreevaluationreq_value();
    req->set_struserid(m_strUserID);
    req->set_strevaluationid(m_strEvaluationID);
}

void PassengerFlowProtoHandler::DeleteStoreEvaluationReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().deletestoreevaluationreq_value();
    m_strUserID = req.struserid();
    m_strEvaluationID = req.strevaluationid();
}

void PassengerFlowProtoHandler::DeleteStoreEvaluationRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteStoreEvaluationRsp_T);

    message.mutable_rspvalue()->mutable_deletestoreevaluationrsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::DeleteStoreEvaluationRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().deletestoreevaluationrsp_value().strvalue();
}

void PassengerFlowProtoHandler::ModifyStoreEvaluationReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyStoreEvaluationReq_T);

    auto req = message.mutable_reqvalue()->mutable_modifystoreevaluationreq_value();
    req->set_struserid(m_strUserID);

    auto evaluation = req->mutable_storeevaluation();
    evaluation->set_strevaluationid(m_storeEvaluation.m_strEvaluationID);
    evaluation->set_strstoreid(m_storeEvaluation.m_strStoreID);
    evaluation->set_struseridcreate(m_storeEvaluation.m_strUserIDCreate);
    evaluation->set_struseridcheck(m_storeEvaluation.m_strUserIDCheck);
    evaluation->set_dtotalscore(m_storeEvaluation.m_dTotalScore);
    evaluation->set_uicheckstatus(m_storeEvaluation.m_uiCheckStatus);
    evaluation->set_strcreatedate(m_storeEvaluation.m_strCreateDate);

    for (auto it = m_storeEvaluation.m_itemScoreList.begin(), end = m_storeEvaluation.m_itemScoreList.end(); it != end; ++it)
    {
        auto score = evaluation->add_itemscore();
        score->set_dscore(it->m_dScore);
        score->set_strdescription(it->m_strDescription);

        auto item = score->mutable_evaluationitem();
        item->set_stritemid(it->m_evaluationItem.m_strItemID);
        item->set_stritemname(it->m_evaluationItem.m_strItemName);
        item->set_dtotalpoint(it->m_evaluationItem.m_dTotalPoint);
        item->set_strdescription(it->m_evaluationItem.m_strDescription);
    }
}

void PassengerFlowProtoHandler::ModifyStoreEvaluationReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().modifystoreevaluationreq_value();
    m_strUserID = req.struserid();

    auto evaluation = req.storeevaluation();
    m_storeEvaluation.m_strEvaluationID = evaluation.strevaluationid();
    m_storeEvaluation.m_strStoreID = evaluation.strstoreid();
    m_storeEvaluation.m_strUserIDCreate = evaluation.struseridcreate();
    m_storeEvaluation.m_strUserIDCheck = evaluation.struseridcheck();
    m_storeEvaluation.m_dTotalScore = evaluation.dtotalscore();
    m_storeEvaluation.m_uiCheckStatus = evaluation.uicheckstatus();
    m_storeEvaluation.m_strCreateDate = evaluation.strcreatedate();

    for (int i = 0, sz = evaluation.itemscore_size(); i < sz; ++i)
    {
        auto rspScore = evaluation.itemscore(i);
        auto rspItem = rspScore.evaluationitem();

        PassengerFlowProtoHandler::EvaluationItemScore score;
        score.m_dScore = rspScore.dscore();
        score.m_strDescription = rspScore.strdescription();
        score.m_evaluationItem.m_strItemID = rspItem.stritemid();
        score.m_evaluationItem.m_strItemName = rspItem.stritemname();
        score.m_evaluationItem.m_dTotalPoint = rspItem.dtotalpoint();
        score.m_evaluationItem.m_strDescription = rspItem.strdescription();

        m_storeEvaluation.m_itemScoreList.push_back(score);
    }
}

void PassengerFlowProtoHandler::ModifyStoreEvaluationRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyStoreEvaluationRsp_T);

    message.mutable_rspvalue()->mutable_modifystoreevaluationrsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::ModifyStoreEvaluationRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().modifystoreevaluationrsp_value().strvalue();
}

void PassengerFlowProtoHandler::QueryStoreEvaluationInfoReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryStoreEvaluationInfoReq_T);

    auto req = message.mutable_reqvalue()->mutable_querystoreevaluationinforeq_value();
    req->set_struserid(m_strUserID);
    req->set_strstoreid(m_strStoreID);
    req->set_strevaluationid(m_strEvaluationID);
}

void PassengerFlowProtoHandler::QueryStoreEvaluationInfoReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);

    auto req = message.reqvalue().querystoreevaluationinforeq_value();
    m_strUserID = req.struserid();
    m_strStoreID = req.strstoreid();
    m_strEvaluationID = req.strevaluationid();
}

void PassengerFlowProtoHandler::QueryStoreEvaluationInfoRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryStoreEvaluationInfoRsp_T);

    auto evaluation = message.mutable_rspvalue()->mutable_querystoreevaluationinforsp_value()->mutable_storeevaluation();
    evaluation->set_strevaluationid(m_storeEvaluation.m_strEvaluationID);
    evaluation->set_strstoreid(m_storeEvaluation.m_strStoreID);
    evaluation->set_struseridcreate(m_storeEvaluation.m_strUserIDCreate);
    evaluation->set_struseridcheck(m_storeEvaluation.m_strUserIDCheck);
    evaluation->set_dtotalscore(m_storeEvaluation.m_dTotalScore);
    evaluation->set_uicheckstatus(m_storeEvaluation.m_uiCheckStatus);
    evaluation->set_strcreatedate(m_storeEvaluation.m_strCreateDate);

    for (auto it = m_storeEvaluation.m_itemScoreList.begin(), end = m_storeEvaluation.m_itemScoreList.end(); it != end; ++it)
    {
        auto score = evaluation->add_itemscore();
        score->set_dscore(it->m_dScore);
        score->set_strdescription(it->m_strDescription);

        auto item = score->mutable_evaluationitem();
        item->set_stritemid(it->m_evaluationItem.m_strItemID);
        item->set_stritemname(it->m_evaluationItem.m_strItemName);
        item->set_dtotalpoint(it->m_evaluationItem.m_dTotalPoint);
        item->set_strdescription(it->m_evaluationItem.m_strDescription);
    }
}

void PassengerFlowProtoHandler::QueryStoreEvaluationInfoRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    auto evaluation = message.rspvalue().querystoreevaluationinforsp_value().storeevaluation();
    m_storeEvaluation.m_strEvaluationID = evaluation.strevaluationid();
    m_storeEvaluation.m_strStoreID = evaluation.strstoreid();
    m_storeEvaluation.m_strUserIDCreate = evaluation.struseridcreate();
    m_storeEvaluation.m_strUserIDCheck = evaluation.struseridcheck();
    m_storeEvaluation.m_dTotalScore = evaluation.dtotalscore();
    m_storeEvaluation.m_uiCheckStatus = evaluation.uicheckstatus();
    m_storeEvaluation.m_strCreateDate = evaluation.strcreatedate();

    for (int i = 0, sz = evaluation.itemscore_size(); i < sz; ++i)
    {
        auto rspScore = evaluation.itemscore(i);
        auto rspItem = rspScore.evaluationitem();

        PassengerFlowProtoHandler::EvaluationItemScore score;
        score.m_dScore = rspScore.dscore();
        score.m_strDescription = rspScore.strdescription();
        score.m_evaluationItem.m_strItemID = rspItem.stritemid();
        score.m_evaluationItem.m_strItemName = rspItem.stritemname();
        score.m_evaluationItem.m_dTotalPoint = rspItem.dtotalpoint();
        score.m_evaluationItem.m_strDescription = rspItem.strdescription();

        m_storeEvaluation.m_itemScoreList.push_back(score);
    }
}

void PassengerFlowProtoHandler::QueryAllStoreEvaluationReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllStoreEvaluationReq_T);

    auto req = message.mutable_reqvalue()->mutable_queryallstoreevaluationreq_value();
    req->set_struserid(m_strUserID);
    req->set_strstoreid(m_strStoreID);
    req->set_strbegindate(m_strBeginDate);
    req->set_strenddate(m_strEndDate);
    req->set_uibeginindex(m_uiBeginIndex);
}

void PassengerFlowProtoHandler::QueryAllStoreEvaluationReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);

    auto req = message.reqvalue().queryallstoreevaluationreq_value();
    m_strUserID = req.struserid();
    m_strStoreID = req.strstoreid();
    m_strBeginDate = req.strbegindate();
    m_strEndDate = req.strenddate();
    m_uiBeginIndex = req.uibeginindex();
}

void PassengerFlowProtoHandler::QueryAllStoreEvaluationRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllStoreEvaluationRsp_T);

    auto rsp = message.mutable_rspvalue()->mutable_queryallstoreevaluationrsp_value();
    for (auto it = m_storeEvaluationList.begin(), end = m_storeEvaluationList.end(); it != end; ++it)
    {
        auto evaluation = rsp->add_storeevaluation();
        evaluation->set_strevaluationid(it->m_strEvaluationID);
        evaluation->set_struseridcreate(it->m_strUserIDCreate);
        evaluation->set_struseridcheck(it->m_strUserIDCheck);
        evaluation->set_dtotalscore(it->m_dTotalScore);
        evaluation->set_uicheckstatus(it->m_uiCheckStatus);
        evaluation->set_strcreatedate(it->m_strCreateDate);
    }
}

void PassengerFlowProtoHandler::QueryAllStoreEvaluationRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    auto rsp = message.rspvalue().queryallstoreevaluationrsp_value();
    for (int i = 0, sz = rsp.storeevaluation_size(); i < sz; ++i)
    {
        auto rspEvaluation = rsp.storeevaluation(i);
        PassengerFlowProtoHandler::StoreEvaluation evaluation;
        evaluation.m_strEvaluationID = rspEvaluation.strevaluationid();
        evaluation.m_strUserIDCreate = rspEvaluation.struseridcreate();
        evaluation.m_strUserIDCheck = rspEvaluation.struseridcheck();
        evaluation.m_dTotalScore = rspEvaluation.dtotalscore();
        evaluation.m_uiCheckStatus = rspEvaluation.uicheckstatus();
        evaluation.m_strCreateDate = rspEvaluation.strcreatedate();

        m_storeEvaluationList.push_back(evaluation);
    }
}

void PassengerFlowProtoHandler::AddRemotePatrolStoreReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddRemotePatrolStoreReq_T);

    auto patrol = message.mutable_reqvalue()->mutable_addremotepatrolstorereq_value()->mutable_patrolstore();
    patrol->set_strpatrolid(m_patrolStore.m_strPatrolID);
    patrol->set_struserid(m_patrolStore.m_strUserID);
    patrol->set_strdeviceid(m_patrolStore.m_strDeviceID);
    patrol->set_strstoreid(m_patrolStore.m_strStoreID);
    patrol->set_strpatroldate(m_patrolStore.m_strPatrolDate);
    patrol->set_uipatrolresult(m_patrolStore.m_uiPatrolResult);
    patrol->set_strdescription(m_patrolStore.m_strDescription);
    patrol->set_strcreatedate(m_patrolStore.m_strCreateDate);

    for (auto it = m_patrolStore.m_strPatrolPictureList.begin(), end = m_patrolStore.m_strPatrolPictureList.end(); it != end; ++it)
    {
        patrol->add_strpatrolpicture(*it);
    }
}

void PassengerFlowProtoHandler::AddRemotePatrolStoreReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);

    auto patrol = message.reqvalue().addremotepatrolstorereq_value().patrolstore();
    m_patrolStore.m_strPatrolID = patrol.strpatrolid();
    m_patrolStore.m_strUserID = patrol.struserid();
    m_patrolStore.m_strDeviceID = patrol.strdeviceid();
    m_patrolStore.m_strStoreID = patrol.strstoreid();
    m_patrolStore.m_strPatrolDate = patrol.strpatroldate();
    m_patrolStore.m_uiPatrolResult = patrol.uipatrolresult();
    m_patrolStore.m_strDescription = patrol.strdescription();
    m_patrolStore.m_strCreateDate = patrol.strcreatedate();

    for (int i = 0, sz = patrol.strpatrolpicture_size(); i < sz; ++i)
    {
        m_patrolStore.m_strPatrolPictureList.push_back(patrol.strpatrolpicture(i));
    }
}

void PassengerFlowProtoHandler::AddRemotePatrolStoreRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::AddRemotePatrolStoreRsp_T);
    message.mutable_rspvalue()->mutable_addremotepatrolstorersp_value()->set_strpatrolid(m_strPatrolID);
}

void PassengerFlowProtoHandler::AddRemotePatrolStoreRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);
    m_strPatrolID = message.rspvalue().addremotepatrolstorersp_value().strpatrolid();
}

void PassengerFlowProtoHandler::DeleteRemotePatrolStoreReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteRemotePatrolStoreReq_T);

    auto req = message.mutable_reqvalue()->mutable_deleteremotepatrolstorereq_value();
    req->set_struserid(m_strUserID);
    req->set_strpatrolid(m_strPatrolID);
}

void PassengerFlowProtoHandler::DeleteRemotePatrolStoreReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().deleteremotepatrolstorereq_value();
    m_strUserID = req.struserid();
    m_strPatrolID = req.strpatrolid();
}

void PassengerFlowProtoHandler::DeleteRemotePatrolStoreRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::DeleteRemotePatrolStoreRsp_T);

    message.mutable_rspvalue()->mutable_deleteremotepatrolstorersp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::DeleteRemotePatrolStoreRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().deleteremotepatrolstorersp_value().strvalue();
}

void PassengerFlowProtoHandler::ModifyRemotePatrolStoreReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyRemotePatrolStoreReq_T);

    auto req = message.mutable_reqvalue()->mutable_modifyremotepatrolstorereq_value();
    req->set_struserid(m_strUserID);

    auto patrol = req->mutable_patrolstore();
    patrol->set_strpatrolid(m_patrolStore.m_strPatrolID);
    patrol->set_struserid(m_patrolStore.m_strUserID);
    patrol->set_strdeviceid(m_patrolStore.m_strDeviceID);
    patrol->set_strstoreid(m_patrolStore.m_strStoreID);
    patrol->set_strpatroldate(m_patrolStore.m_strPatrolDate);
    patrol->set_uipatrolresult(m_patrolStore.m_uiPatrolResult);
    patrol->set_strdescription(m_patrolStore.m_strDescription);
    patrol->set_strcreatedate(m_patrolStore.m_strCreateDate);

    for (auto it = m_patrolStore.m_strPatrolPictureList.begin(), end = m_patrolStore.m_strPatrolPictureList.end(); it != end; ++it)
    {
        patrol->add_strpatrolpicture(*it);
    }
}

void PassengerFlowProtoHandler::ModifyRemotePatrolStoreReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().modifyremotepatrolstorereq_value();
    m_strUserID = req.struserid();

    auto patrol = req.patrolstore();
    m_patrolStore.m_strPatrolID = patrol.strpatrolid();
    m_patrolStore.m_strUserID = patrol.struserid();
    m_patrolStore.m_strDeviceID = patrol.strdeviceid();
    m_patrolStore.m_strStoreID = patrol.strstoreid();
    m_patrolStore.m_strPatrolDate = patrol.strpatroldate();
    m_patrolStore.m_uiPatrolResult = patrol.uipatrolresult();
    m_patrolStore.m_strDescription = patrol.strdescription();
    m_patrolStore.m_strCreateDate = patrol.strcreatedate();

    for (int i = 0, sz = patrol.strpatrolpicture_size(); i < sz; ++i)
    {
        m_patrolStore.m_strPatrolPictureList.push_back(patrol.strpatrolpicture(i));
    }
}

void PassengerFlowProtoHandler::ModifyRemotePatrolStoreRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ModifyRemotePatrolStoreRsp_T);

    message.mutable_rspvalue()->mutable_modifyremotepatrolstorersp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::ModifyRemotePatrolStoreRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().modifyremotepatrolstorersp_value().strvalue();
}

void PassengerFlowProtoHandler::QueryRemotePatrolStoreInfoReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryRemotePatrolStoreInfoReq_T);

    auto req = message.mutable_reqvalue()->mutable_queryremotepatrolstoreinforeq_value();
    req->set_struserid(m_strUserID);
    req->set_strpatrolid(m_strPatrolID);
}

void PassengerFlowProtoHandler::QueryRemotePatrolStoreInfoReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);

    auto req = message.reqvalue().queryremotepatrolstoreinforeq_value();
    m_strUserID = req.struserid();
    m_strPatrolID = req.strpatrolid();
}

void PassengerFlowProtoHandler::QueryRemotePatrolStoreInfoRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryRemotePatrolStoreInfoRsp_T);

    auto patrol = message.mutable_rspvalue()->mutable_queryremotepatrolstoreinforsp_value()->mutable_patrolstore();
    patrol->set_strpatrolid(m_patrolStore.m_strPatrolID);
    patrol->set_struserid(m_patrolStore.m_strUserID);
    patrol->set_strdeviceid(m_patrolStore.m_strDeviceID);
    patrol->set_strstoreid(m_patrolStore.m_strStoreID);
    patrol->set_strpatroldate(m_patrolStore.m_strPatrolDate);
    patrol->set_uipatrolresult(m_patrolStore.m_uiPatrolResult);
    patrol->set_strdescription(m_patrolStore.m_strDescription);
    patrol->set_strcreatedate(m_patrolStore.m_strCreateDate);

    for (auto it = m_patrolStore.m_strPatrolPictureList.begin(), end = m_patrolStore.m_strPatrolPictureList.end(); it != end; ++it)
    {
        patrol->add_strpatrolpicture(*it);
    }
}

void PassengerFlowProtoHandler::QueryRemotePatrolStoreInfoRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    auto patrol = message.rspvalue().queryremotepatrolstoreinforsp_value().patrolstore();
    m_patrolStore.m_strPatrolID = patrol.strpatrolid();
    m_patrolStore.m_strUserID = patrol.struserid();
    m_patrolStore.m_strDeviceID = patrol.strdeviceid();
    m_patrolStore.m_strStoreID = patrol.strstoreid();
    m_patrolStore.m_strPatrolDate = patrol.strpatroldate();
    m_patrolStore.m_uiPatrolResult = patrol.uipatrolresult();
    m_patrolStore.m_strDescription = patrol.strdescription();
    m_patrolStore.m_strCreateDate = patrol.strcreatedate();

    for (int i = 0, sz = patrol.strpatrolpicture_size(); i < sz; ++i)
    {
        m_patrolStore.m_strPatrolPictureList.push_back(patrol.strpatrolpicture(i));
    }
}

void PassengerFlowProtoHandler::QueryAllRemotePatrolStoreReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllRemotePatrolStoreReq_T);

    auto req = message.mutable_reqvalue()->mutable_queryallremotepatrolstorereq_value();
    req->set_struserid(m_strUserID);
    req->set_strstoreid(m_strStoreID);
    req->set_strbegindate(m_strBeginDate);
    req->set_strenddate(m_strEndDate);
    req->set_uibeginindex(m_uiBeginIndex);
}

void PassengerFlowProtoHandler::QueryAllRemotePatrolStoreReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);

    auto req = message.reqvalue().queryallremotepatrolstorereq_value();
    m_strUserID = req.struserid();
    m_strStoreID = req.strstoreid();
    m_strBeginDate = req.strbegindate();
    m_strEndDate = req.strenddate();
    m_uiBeginIndex = req.uibeginindex();
}

void PassengerFlowProtoHandler::QueryAllRemotePatrolStoreRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryAllRemotePatrolStoreRsp_T);

    auto rsp = message.mutable_rspvalue()->mutable_queryallremotepatrolstorersp_value();
    for (auto it = m_patrolStoreList.begin(), end = m_patrolStoreList.end(); it != end; ++it)
    {
        auto patrol = rsp->add_patrolstore();
        patrol->set_strpatrolid(it->m_strPatrolID);
        patrol->set_struserid(it->m_strUserID);
        patrol->set_strdeviceid(it->m_strDeviceID);
        patrol->set_strstoreid(it->m_strStoreID);
        patrol->set_strpatroldate(it->m_strPatrolDate);
        patrol->set_uipatrolresult(it->m_uiPatrolResult);
        patrol->set_strdescription(it->m_strDescription);
        patrol->set_strcreatedate(it->m_strCreateDate);

        for (auto pic = it->m_strPatrolPictureList.begin(), end = it->m_strPatrolPictureList.end(); pic != end; ++pic)
        {
            patrol->add_strpatrolpicture(*pic);
        }
    }
}

void PassengerFlowProtoHandler::QueryAllRemotePatrolStoreRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    auto rsp = message.rspvalue().queryallremotepatrolstorersp_value();
    for (int i = 0, sz = rsp.patrolstore_size(); i < sz; ++i)
    {
        auto rspPatrol = rsp.patrolstore(i);
        PassengerFlowProtoHandler::RemotePatrolStore patrol;
        patrol.m_strPatrolID = rspPatrol.strpatrolid();
        patrol.m_strUserID = rspPatrol.struserid();
        patrol.m_strDeviceID = rspPatrol.strdeviceid();
        patrol.m_strStoreID = rspPatrol.strstoreid();
        patrol.m_strPatrolDate = rspPatrol.strpatroldate();
        patrol.m_uiPatrolResult = rspPatrol.uipatrolresult();
        patrol.m_strDescription = rspPatrol.strdescription();
        patrol.m_strCreateDate = rspPatrol.strcreatedate();

        for (int i = 0, sz = rspPatrol.strpatrolpicture_size(); i < sz; ++i)
        {
            patrol.m_strPatrolPictureList.push_back(rspPatrol.strpatrolpicture(i));
        }

        m_patrolStoreList.push_back(patrol);
    }
}

void PassengerFlowProtoHandler::ImportPOSDataReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ImportPOSDataReq_T);

    auto req = message.mutable_reqvalue()->mutable_importposdatareq_value();
    req->set_struserid(m_strUserID);
    req->set_strstoreid(m_strStoreID);
    req->set_uiorderamount(m_uiOrderAmount);
    req->set_uigoodsamount(m_uiGoodsAmount);
    req->set_ddealamount(m_dDealAmount);
    req->set_strdealdate(m_strDealDate);
}

void PassengerFlowProtoHandler::ImportPOSDataReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().importposdatareq_value();
    m_strUserID = req.struserid();
    m_strStoreID = req.strstoreid();
    m_uiOrderAmount = req.uiorderamount();
    m_uiGoodsAmount = req.uigoodsamount();
    m_dDealAmount = req.ddealamount();
    m_strDealDate = req.strdealdate();
}

void PassengerFlowProtoHandler::ImportPOSDataRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ImportPOSDataRsp_T);

    message.mutable_rspvalue()->mutable_importposdatarsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::ImportPOSDataRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().importposdatarsp_value().strvalue();
}

void PassengerFlowProtoHandler::QueryCustomerFlowStatisticReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryCustomerFlowStatisticReq_T);

    auto req = message.mutable_reqvalue()->mutable_querycustomerflowstatisticreq_value();
    req->set_struserid(m_strUserID);
    req->set_strstoreid(m_strStoreID);
    req->set_strbegindate(m_strBeginDate);
    req->set_strenddate(m_strEndDate);
    req->set_uitimeprecision(m_uiTimePrecision);
}

void PassengerFlowProtoHandler::QueryCustomerFlowStatisticReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().querycustomerflowstatisticreq_value();
    m_strUserID = req.struserid();
    m_strStoreID = req.strstoreid();
    m_strBeginDate = req.strbegindate();
    m_strEndDate = req.strenddate();
    m_uiTimePrecision = req.uitimeprecision();
}

void PassengerFlowProtoHandler::QueryCustomerFlowStatisticRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::QueryCustomerFlowStatisticRsp_T);

    message.mutable_rspvalue()->mutable_querycustomerflowstatisticrsp_value()->set_strchartdata(m_strChartData);
}

void PassengerFlowProtoHandler::QueryCustomerFlowStatisticRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strChartData = message.rspvalue().querycustomerflowstatisticrsp_value().strchartdata();
}

void PassengerFlowProtoHandler::ReportCustomerFlowDataReq::Serializer(CustomerFlowMessage &message) const
{
    Request::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ReportCustomerFlowDataReq_T);

    auto req = message.mutable_reqvalue()->mutable_reportcustomerflowdatareq_value();
    req->set_strdeviceid(m_strDeviceID);

    SerializeRawCustomerFlowList(m_customerFlowList, req->mutable_customerflow());
}

void PassengerFlowProtoHandler::ReportCustomerFlowDataReq::UnSerializer(const CustomerFlowMessage &message)
{
    Request::UnSerializer(message);
    auto req = message.reqvalue().reportcustomerflowdatareq_value();
    m_strDeviceID = req.strdeviceid();

    UnSerializeRawCustomerFlowList(m_customerFlowList, req.customerflow());
}

void PassengerFlowProtoHandler::ReportCustomerFlowDataRsp::Serializer(CustomerFlowMessage &message) const
{
    Response::Serializer(message);
    message.set_type(CustomerFlow::Interactive::Message::CustomerFlowMsgType::ReportCustomerFlowDataRsp_T);

    message.mutable_rspvalue()->mutable_reportcustomerflowdatarsp_value()->set_strvalue(m_strValue);
}

void PassengerFlowProtoHandler::ReportCustomerFlowDataRsp::UnSerializer(const CustomerFlowMessage &message)
{
    Response::UnSerializer(message);

    m_strValue = message.rspvalue().reportcustomerflowdatarsp_value().strvalue();
}

