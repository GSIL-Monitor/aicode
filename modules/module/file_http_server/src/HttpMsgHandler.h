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

class FileManager;
class InteractiveProtoHandler;

class HttpMsgHandler : public boost::noncopyable
{
public:
    static const std::string UPLOAD_FILE_ACTION;
    static const std::string DOWNLOAD_FILE_ACTION;

    static const std::string REGISTER_USER_ACTION;
    static const std::string USER_SHAKEHAND_ACTION;
    
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

    void SetFileMgr(boost::shared_ptr<FileManager> pFileMgr);

    bool UploadFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    bool DownloadFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    void WriteMsg(const std::map<std::string, std::string> &MsgMap, MsgWriter writer, const bool blResult = true, boost::function<void(void*)> PostFunc = NULL);
    
    void WriteMsg(MsgWriter writer, const char *pBuffer, const unsigned int uiBufferSize, const bool IsNeedWriteHead, const std::string &strHeaderMsg);

private:
    bool DownloadFile(const std::string &strFileID, MsgWriter writer);

private:
    ParamInfo m_ParamInfo;
    boost::shared_ptr<InteractiveProtoHandler> m_pInteractiveProtoHandler;

    boost::shared_ptr<FileManager> m_pFileMgr;

private:
    static const std::string SUCCESS_CODE;
    static const std::string SUCCESS_MSG;
    static const std::string FAILED_CODE;
    static const std::string FAILED_MSG;

};

#endif
