#ifndef _PRODUCT_MANAGER_
#define _PRODUCT_MANAGER_

#include "boost/noncopyable.hpp"
#include "ControlCenter.h"
#include <unordered_map>
#include <string>
#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "NetComm.h"
#include "DBInfoCacheManager.h"
#include "boost/atomic.hpp"
#include "SessionMgr.h"
#include "PassengerFlowProtoHandler.h"
#include "MessagePush_Getui.h"

#include "ProductService.h"
#include "OrderService.h"

using namespace Product::Service;

class MysqlImpl;

/************************************************************************/
/* 用户管理类，提供了管理用户所有相关的方法接口和实现。
 * 该类提供的方法会注册到到ControlCenter类中。
 * Author：尹宾
 * Date：2016-12-1*/
 /************************************************************************/
class ProductManager : virtual public ProductServiceIf, virtual public OrderServiceIf
{
public:

    static const unsigned int UNUSED_INPUT_UINT = 0xFFFFFFFF;

    static const int UNREAD_STATE = 0;
    static const int READ_STATE = 1;

    static const int EVENT_REMOTE_PATROL = 0;
    static const int EVENT_REGULAR_PATROL = 1;
    static const int EVENT_STORE_EVALUATION = 2;
    static const int EVENT_ALARM_CREATED = 3;
    static const int EVENT_ALARM_RECOVERD = 4;

    static std::string ALLOW_ACCESS;
    static std::string DISALLOW_ACCESS;

    typedef struct _ParamInfo
    {
        std::string m_strDBHost;
        std::string m_strDBPort;
        std::string m_strDBUser;
        std::string m_strDBPassword;
        std::string m_strDBName;
        std::string m_strMemAddress;
        std::string m_strMemPort;
        std::string m_strSessionTimeoutCountThreshold;
        std::string m_strDevSessionTimeoutCountThreshold;
        std::string m_strFileServerURL;
        std::string m_strGetIpInfoSite;
        std::string m_strMemAddressGlobal;
        std::string m_strMemPortGlobal;
        std::string m_strUserLoginMutex;
        std::string m_strUserAllowDiffTerminal;
        std::string m_strUserKickoutType;
        std::string m_strMasterNode;
        std::string m_strPushServerUrl;
        int m_iAuthExpire;
        std::string m_strAndroidAppID;
        std::string m_strAndroidAppKey;
        std::string m_strAndroidMasterSecret;
        std::string m_strIOSAppID;
        std::string m_strIOSAppKey;
        std::string m_strIOSMasterSecret;
        std::string m_strMessageTitle;
        std::string m_strMessageContentRegularPatrol;
        std::string m_strMessageContentRemotePatrol;
        std::string m_strMessageContentEvaluation;
        std::string m_strAlarmMessageTitle;
        std::string m_strAlarmMessageContentCreated;
    } ParamInfo;


    inline void SetParamInfo(const ParamInfo &pinfo)
    {
        m_ParamInfo = pinfo;
    };

    ProductManager(const ParamInfo &pinfo);
    virtual ~ProductManager();

    bool Init();

    virtual void AddProduct(AddProductRT& _return, const std::string& strSid, const std::string& strUserID, const ProductInfo& pdt);
    virtual void RemoveProduct(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID);
    virtual void ModifyProduct(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID, const ProductInfo& pdt);
    virtual void QueryProduct(QueryProductRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID);
    virtual void QueryAllProduct(QueryAllProductRT& _return, const std::string& strSid, const std::string& strUserID);
    virtual void AddProductProperty(AddProductPropertyRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID,
        const ProductProperty& pdtppt);
    virtual void RemoveProductProperty(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strPdtID,
        const std::string& strPdtpptID);


    virtual void AddOrd(AddOrdRT& _return, const std::string& strSid, const std::string& strUserID, const OrderInfo& ord);
    virtual void RemoveOrd(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID);
    virtual void ModifyOrd(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID, const OrderInfo& ord);
    virtual void QueryOrd(QueryOrdRT& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID);
    virtual void QueryAllOrd(QueryAllOrdRT& _return, const std::string& strSid, const std::string& strUserID, const QueryAllOrdParam& qryparam);
    virtual void AddOrdDetail(AddOrdDetailRT& _return, const std::string& strSid, const std::string& strUserID, const OrderDetail& orddt);
    virtual void RemoveOrdDetail(ProductRTInfo& _return, const std::string& strSid, const std::string& strUserID, const std::string& strOrdID, const std::string& strOrddtID);

    bool PreCommonHandler(int iCmd, const std::string &strSid, int *piRet);
    
private:

    bool PushMessage(const std::string &strTitle, const std::string &strContent, const std::string &strPayload, const std::string &strUserID);

    std::string CurrentTime();

    int Year(const std::string &strDate);

    int Month(const std::string &strDate);

    int Quarter(const std::string &strDate);

    int MonthDuration(const std::string &strBeginDate, const std::string &strEndDate);

    int QuarterDuration(const std::string &strBeginDate, const std::string &strEndDate);

    int TimePrecisionScale(const std::string &strDate, const unsigned int uiTimePrecision);

    bool QueryProductProperty(const std::string &strPdtID, std::vector<ProductProperty> &pdtpptlist);

    bool QueryOrderDetail(const std::string &strOrdID, std::vector<OrderDetail> &orddtlist, const std::string &strPdtID = "");
   

private:
    ParamInfo m_ParamInfo;

    Runner m_DBRuner;
    boost::shared_ptr<PassengerFlowProtoHandler> m_pProtoHandler;

    MysqlImpl *m_pMysql;
    DBInfoCacheManager m_DBCache;

    boost::atomic_uint64_t m_uiMsgSeq;

    SessionMgr m_SessionMgr;

    TimeOutHandler m_DBTimer;

    MessagePush_Getui m_pushGetuiIOS;
    MessagePush_Getui m_pushGetuiAndroid;
};



#endif