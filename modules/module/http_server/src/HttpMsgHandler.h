#ifndef FILE_HANDLER
#define FILE_HANDLER

#include "NetComm.h"
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include "FCGIManager.h"

/************************************************************************/
/*����ʵ����Ϣ�������ϸ�ڣ���FCGIManagerע�ᴦ������
 *���а���ʵ�ʵ�ҵ����Ϣ�������磺���û�ע�����Ϣ���ܣ�
 *���ýӿ�ʵ���û�ע���ҵ������*/
/************************************************************************/

class HttpMsgHandler : public boost::noncopyable
{
public:
    typedef struct _ParamInfo
    {
        std::string m_strRemoteAddress;
        std::string m_strRemotePort;
        unsigned int m_uiShakehandOfChannelInterval;
        std::string m_strSelfID;
        unsigned int m_uiCallFuncTimeout;
        unsigned int m_uiThreadOfWorking;
    } ParamInfo;

    HttpMsgHandler(const ParamInfo &parminfo);
    ~HttpMsgHandler();

    void RegisterUserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    void UnRegisterUserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    void QueryUserInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    void UserLoginHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    void UserLogoutHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    void ConfigInfoHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    void AddDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    void DeleteDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    void ModifyDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    void QueryDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    void QueryDevicesOfUserHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    void QueryUsersOfDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    void SharingDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    void CancelSharedDeviceHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);



private:
    void WriteMsg(const std::map<std::string, std::string> &MsgMap, MsgWriter writer, const bool blResult = true, boost::function<void(void*)> PostFunc = NULL);
    
private:
    ParamInfo m_ParamInfo;
    
    static const std::string SUCCESS_CODE;
    static const std::string SUCCESS_MSG;
    static const std::string FAILED_CODE;
    static const std::string FAILED_MSG;



};


#endif
