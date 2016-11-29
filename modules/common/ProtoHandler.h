#ifndef _PROTOE_HANDLER_
#define _PROTOE_HANDLER_

#include <string>
#include <list>
#include <map>
#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace TDFS
{
    namespace MESSAGE
    {
        class SyncMessage;
    }
}

using TDFS::MESSAGE::SyncMessage;

class ProtoHandler
{
public:
    ProtoHandler();
    ~ProtoHandler();

    enum MsgType
    {
        Init_T = 0,
        LoginReq_T = 10000, //登录
        LoginRsp_T = 10010,
        LoginoutReq_T = 10020, //登出
        LoginoutRsp_T = 10030,
        ConfigInfoReq_T = 10040, //配置
        ConfigInfoRsp_T = 10050,
        ShakehandReq_T = 10060, //握手
        ShakehandRsp_T = 10070,
        GetSyncAddressReq_T = 10080, //获取同步服务节点地址
        GetSyncAddressRsp_T = 10090,
        SyncFileListPendingReq_T = 10100, //待同步文件列表
        SyncFileListPendingRsp_T = 10110,
        ControlCMDReq_T = 10120, //控制命令
        ControlCMDRsp_T = 10130,
        GetFileInfoReq_T = 10140, //获取文件信息
        GetFileInfoRsp_T = 10150,
        FullSyncReq_T = 10160, //全量同步请求
        FullSyncRsp_T = 10170,
        FullSyncConform_T = 10180, //全量同步确认
        ActiveReportingReq_T = 10190, //主动上报请求
        ActiveReportingRsp_T = 10200
    };

    enum SyncServiceStatus
    {
        IDLE = 0,
        BUSSY = 1,
        FAULT = 2
    };

    struct Req
    {
        Req(){};
        virtual ~Req(){};
 
        MsgType m_MsgType;
        unsigned long long m_uiMsgSeq;
        std::string m_strSID;

        virtual void UnSerializer(const SyncMessage &SyncMsg);
        
        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct Rsp : Req
    {
        Rsp(){};
        virtual ~Rsp(){};

        int m_iRetcode;
        std::string m_strRetMsg;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct LoginReq : Req
    {

        std::string m_strSyncServiceName;
        std::string m_strPassword;
        std::string m_strStorageIP;
        std::string m_strStoragePort;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };
    
    struct LoginRsp : Rsp
    {

        std::string m_strLoginSID;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct LogoutReq : Req
    {

        std::string m_strValue;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct LogoutRsp :Rsp
    {

        std::string m_strValue;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct ConfigInfoReq : Req
    {

        std::string m_strSyncServiceName;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct ConfigInfoRsp : Rsp
    {

        std::string m_strConfigJson;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct ShakehandReq : Req
    {

        std::string m_strValue;

        SyncServiceStatus m_status;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct ShakehandRsp : Rsp
    {

        std::string m_strValue;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct Address
    {
        std::string strAddress;
        enum NodeType
        {
            Center = 0,
            Sync = 1
        };

        NodeType type;
        std::string AreaID;
        std::string m_strStorageIP;
        std::string m_strStoragePort;
        SyncServiceStatus status;
    };

    struct GetSyncAddressReq : Req
    {

        std::string m_strValue;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct GetSyncAddressRsp : Rsp
    {

        std::list<Address> m_AddressList;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct FileInfo
    {
        std::string strFileID;
        std::string strFileName;
        unsigned int uiFileSize;
        std::string strFileMD5;
        std::string strCreatedate;
    };

    struct SyncFileListPendingReq : Req
    {

        std::list<FileInfo> m_FileInfoList;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct SyncFileListPendingRsp : Rsp
    {

        std::list<FileInfo> m_FileInfoList;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct GetFileInfoReq : Req
    {

        std::string m_strFileID;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct GetFileInfoRsp : Rsp
    {

        FileInfo m_fileinfo;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct ControlCMDReq : Req
    {

        unsigned int m_uiCMD;
        std::string m_strCMDMsg;
        bool m_blNeedActiveReporting;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct ControlCMDRsp : Rsp
    {

        unsigned int m_uiCMD;
        bool m_blNeedActiveReporting;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct FullSyncReq : Req
    {
        std::string m_strValue;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct FullSyncRsp : Rsp
    {
        std::list<FileInfo> m_FileInfoList;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct FullSyncConform : Req
    {
        unsigned int m_uiFlag;
        std::list<FileInfo> m_FileInfoList;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct ActiveReportingReq : Req
    {
        unsigned int m_uiTotal;
        unsigned int m_uiCurrentCompleted;
        FileInfo m_CurrentCompletedFileInfo;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    struct ActiveReportingRsp : Rsp
    {
        std::string m_strValue;

        virtual void UnSerializer(const SyncMessage &SyncMsg);

        virtual void Serializer(SyncMessage &SyncMsg) const;
    };

    bool GetMsgType(const std::string &strData, MsgType &msgtype);

    bool SerializeReq(const Req &req, std::string &strOutput);
    bool UnSerializeReq(const std::string &strData, Req &req);
    
    bool SerializeRsp(const Rsp &rsp, std::string &strOutput);
    bool UnSerializeRsp(const std::string &strData, Rsp &rsp);

private:


    bool LoginReqSerializer(const Req &req, std::string &strOutput);
    bool LoginReqUnSerializer(const SyncMessage &SyncMsg, Req &req);
    bool LoginRspSerializer(const Req &rsp, std::string &strOutput);
    bool LoginRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp);
    
    bool LogoutReqSerializer(const Req &req, std::string &strOutput);
    bool LogoutReqUnSerializer(const SyncMessage &SyncMsg, Req &req);
    bool LogoutRspSerializer(const Req &rsp, std::string &strOutput);
    bool LogoutRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp);

    bool ConfigInfoReqSerializer(const Req &req, std::string &strOutput);
    bool ConfigInfoReqUnSerializer(const SyncMessage &SyncMsg, Req &req);
    bool ConfigInfoRspSerializer(const Req &rsp, std::string &strOutput);
    bool ConfigInfoRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp);

    bool ShakehandReqSerializer(const Req &req, std::string &strOutput);
    bool ShakehandReqUnSerializer(const SyncMessage &SyncMsg, Req &req);
    bool ShakehandRspSerializer(const Req &rsp, std::string &strOutput);
    bool ShakehandRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp);

    bool GetSyncAddressReqSerializer(const Req &req, std::string &strOutput);
    bool GetSyncAddressReqUnSerializer(const SyncMessage &SyncMsg, Req &req);
    bool GetSyncAddressRspSerializer(const Req &rsp, std::string &strOutput);
    bool GetSyncAddressRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp);

    bool SyncFileListPendingReqSerializer(const Req &req, std::string &strOutput);
    bool SyncFileListPendingReqUnSerializer(const SyncMessage &SyncMsg, Req &req);
    bool SyncFileListPendingRspSerializer(const Req &rsp, std::string &strOutput);
    bool SyncFileListPendingRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp);

    bool GetFileInfoReqSerializer(const Req &req, std::string &strOutput);
    bool GetFileInfoReqUnSerializer(const SyncMessage &SyncMsg, Req &req);
    bool GetFileInfoRspSerializer(const Req &rsp, std::string &strOutput);
    bool GetFileInfoRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp);

    bool ControlCMDReqSerializer(const Req &req, std::string &strOutput);
    bool ControlCMDReqUnSerializer(const SyncMessage &SyncMsg, Req &req);
    bool ControlCMDRspSerializer(const Req &rsp, std::string &strOutput);
    bool ControlCMDRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp);

    bool FullSyncReqSerializer(const Req &req, std::string &strOutput);
    bool FullSyncReqUnSerializer(const SyncMessage &SyncMsg, Req &req);
    bool FullSyncRspSerializer(const Req &rsp, std::string &strOutput);
    bool FullSyncRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp);

    bool FullSyncConformSerializer(const Req &req, std::string &strOutput);
    bool FullSyncConformUnSerializer(const SyncMessage &SyncMsg, Req &req);

    bool ActiveReportingReqSerializer(const Req &req, std::string &strOutput);
    bool ActiveReportingReqUnSerializer(const SyncMessage &SyncMsg, Req &req);
    bool ActiveReportingRspSerializer(const Req &rsp, std::string &strOutput);
    bool ActiveReportingRspUnSerializer(const SyncMessage &SyncMsg, Req &rsp);

private:    
    typedef boost::function<bool(const Req &req, std::string &strOutput)> Serializer;
    typedef boost::function<bool(const SyncMessage &SyncMsg, Req &req)> UnSerializer;

    struct SerializeHandler
    {
        Serializer Szr;
        UnSerializer UnSzr;
    };

    std::map<int, SerializeHandler> m_ReqAndRspHandlerMap;

};



#endif
