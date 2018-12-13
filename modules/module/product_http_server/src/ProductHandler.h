#ifndef PRODUCT_HANDLER
#define PRODUCT_HANDLER

#include "NetComm.h"
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include "FCGIManager.h"
#include "json/json.h"

#include "ProductClient.h"

/************************************************************************/
/*负责实现消息处理具体细节，向FCGIManager注册处理函数，
 *其中包括实际的业务消息处理，例如：将用户注册的信息汇总，
 *调用接口实现用户注册的业务动作。*/
/************************************************************************/


typedef boost::function<bool(const std::string &strBinaryContent, unsigned int uiRetCode)> RspFuncCommonT;


class ProductHandler;

class ProductHandler : public boost::noncopyable
{
public:
    static const std::string ADD_PRODUCT_ACTION;
    static const std::string REMOVE_PRODUCT_ACTION;
    static const std::string MOD_PRODUCT_ACTION;
    static const std::string QUERY_PRODUCT_ACTION;
    static const std::string QUERY_ALL_PRODUCT_ACTION;
    static const std::string ADD_PRODUCT_PROPERTY_ACTION;
    static const std::string REMOVE_PRODUCT_PROPERTY_ACTION;
    
    typedef struct _ParamInfo
    {
        std::string m_strRemoteAddress;
        std::string m_strRemotePort;
        unsigned int m_uiShakehandOfChannelInterval;
        std::string m_strSelfID;
        unsigned int m_uiCallFuncTimeout;
        unsigned int m_uiThreadOfWorking;
    } ParamInfo;

    ProductHandler(const ParamInfo &parminfo);
    ~ProductHandler();

    bool ParseMsgOfCompact(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool AddProductHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool RemoveProductHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool ModifyProductHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryProductHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool QueryAllProductHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    bool AddProductPropertyHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool RemoveProductPropertyHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);


    void WriteMsg(const std::map<std::string, std::string> &MsgMap, MsgWriter writer, const bool blResult = true, boost::function<void(void*)> PostFunc = NULL);
    
private:
    bool RspFuncCommonActionT(const std::string &strBinaryContent, unsigned int uiRetCode, int *piRetCode, RspFuncCommonT rspfunc);

    bool PreCommonHandler(const std::string &strMsgReceived, int &iRetCode);
    
private:
    bool ValidDatetime(const std::string &strDatetime, const bool IsTime = false);

    bool ValidNumber(const std::string &strNumber, const unsigned int uiCount = 1);

    template<typename T>
    bool ValidType(const std::string &strValue, T &ValueT);

    bool GetValueList(const std::string &strValue, std::list<std::string> &strValueList);

    bool GetValueFromList(const std::string &strValue, boost::function<bool(Json::Value &)> ParseFunc);

private:
    ParamInfo m_ParamInfo;
    
private:
    static const std::string SUCCESS_CODE;
    static const std::string SUCCESS_MSG;
    static const std::string FAILED_CODE;
    static const std::string FAILED_MSG;

};

#endif
