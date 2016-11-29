#ifndef _CONTROL_CENTER_
#define _CONTROL_CENTER_

#include <string>
#include "boost/thread.hpp"
#include "ClientCommInterface.h"
#include "boost/shared_ptr.hpp"
#include "NetComm.h"
#include <map>
#include "boost/atomic.hpp"
#include "LogRLD.h"

class ProtoHandler;

class ControlCenter
{
public:
    typedef struct _ParamInfo
    {
        std::string strDBHost;
        std::string strDBPort;
        std::string strDBUser;
        std::string strDBPassword;
        std::string strDBName;

        std::string strRemoteAddress;
        std::string strRemotePort;
        unsigned int uiShakehandOfChannelInterval;

        std::string strSelfID;
        unsigned int uiSyncShakehandTimeoutCount;
        unsigned int uiSyncShakehandTimeout;
        unsigned int uiSyncAddressRspInvalidTimeout;
        unsigned int uiThreadOfWorking;

    } ParamInfo;

    ControlCenter(const ParamInfo &pinfo);
    ~ControlCenter();

    void Run(const bool isWaitRunFinished);

    void Stop();


    inline void SetParamInfo(const ParamInfo &pinfo)
    {
        m_ParamInfo = pinfo;
    };

private:    
    void ConnectCB(const boost::system::error_code &ec);
    void WriteCB(const boost::system::error_code &ec, void *pValue);
    void ReadCB(const boost::system::error_code &ec, std::list<ClientMsg> *pClientMsgList, void *pValue);    
    void Reconnect();

private:
    boost::shared_ptr<ProtoHandler> m_pProtoHandler;

    ParamInfo m_ParamInfo;

    boost::shared_ptr<ClientCommInterface> m_pClient;

    std::string m_strChannelUUID;

    Runner m_MsgHandlerRunner;

    struct SyncService
    {
        SyncService(){};
        ~SyncService(){ LOG_INFO_RLD("Syncservice session id " << m_strSID << " destroyed."); };

        boost::mutex m_SyncServiceMutex;

        std::string m_strSID;
        std::string m_strSyncServiceName;
        std::string m_strPassword;
        std::string m_strStorageIP;
        std::string m_strStoragePort;
        std::string m_strConfigInfo;
        
        enum SyncServiceStatus
        {
            IDLE = 0,
            BUSSY = 1,
            FAULT = 2
        };
        SyncServiceStatus m_Status;
        
        std::string m_strAddress;
        enum NodeType
        {
            Center = 0,
            Sync = 1
        };
        NodeType m_Type;
        std::string m_strAreaID;

        boost::shared_ptr<TimeOutHandler> m_pTMHander;
        boost::atomic_uint32_t m_uiTMTickCount;
        unsigned int m_uiMaxTickCount;

        void TimeOutCB(const boost::system::error_code &ec);

        typedef boost::function<void()> TMFunc;
        TMFunc m_TMFunc;
    };

    boost::mutex m_SyncServiceMapMutex;
    std::map<std::string, boost::shared_ptr<SyncService> > m_SyncServiceMap; //key is session id
    
    typedef boost::function<bool(const std::string &strData, const std::string &strSrcID)> MsgUnSerializer;
    std::map<int, MsgUnSerializer> m_MsgUnSerializerMap;

    TimeOutHandlerEx m_TMEX;


private:
    bool ReceiveMsgHandler(const std::string &strData, const std::string &strSrcID, void *pValue);
    void ReceiveMsgHandlerInner(MsgUnSerializer MsgUszr, const std::string &strData, const std::string &strSrcID, const int iMsgType, void *pValue);

    bool LoginReqMsgHandler(const std::string &strData, const std::string &strSrcID);
    bool LoginoutReqMsgHandler(const std::string &strData, const std::string &strSrcID);
    bool ConfigInfoReqMsgHandler(const std::string &strData, const std::string &strSrcID);
    bool ShakehandReqMsgHandler(const std::string &strData, const std::string &strSrcID);
    bool GetSyncAddressReqMsgHandler(const std::string &strData, const std::string &strSrcID);
    bool SyncFileListPendingReqMsgHandler(const std::string &strData, const std::string &strSrcID);
    bool ControlCMDReqMsgHandler(const std::string &strData, const std::string &strSrcID);
    bool GetFileInfoReqMsgHandler(const std::string &strData, const std::string &strSrcID);

    void AddSyncService(const std::string &strSID, boost::shared_ptr<SyncService> pSyncservice);
    void DeleteSyncService(const std::string &strSID);
    boost::shared_ptr<SyncService> FindSyncService(const std::string &strSID);

    std::string GetConfigInfo(const std::string &strSyncServiceName);


};

#endif
