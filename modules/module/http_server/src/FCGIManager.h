#ifndef FCGI_MANAGER
#define FCGI_MANAGER

#include "NetComm.h"
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include "fcgiapp.h"
#include "ConfigSt.h"

/************************************************************************/
/* FCGI管理器，负责将FCGI相关的信息解析出来，提供给业务使用，
 * 提供HTTP通道，供给业务进行写入HTTP消息（业务处理结果信息或者文件流信息）*/
/************************************************************************/

typedef boost::function<void(const char *, const unsigned int, const unsigned int)> MsgWriter;
typedef enum
{
   PRINT_MODEL = 0,
   STREAM_MODEL
} MsgWriterModel;

typedef std::map<std::string, std::string> MsgInfoMap;
typedef boost::function<void(boost::shared_ptr<MsgInfoMap>, MsgWriter)> MsgHandler;


class FCGIManager : public boost::noncopyable
{
public:
    FCGIManager(ConfigSt& cfg, const unsigned int uiRunTdNum);
    ~FCGIManager();

    void Run(bool isWaitRunFinished = false);

    void SetUploadMsgHandler(MsgHandler msghdr);
    void SetSpeedUploadMsgHandler(MsgHandler msghdr);
    void SetDeleteMsgHandler(MsgHandler msghdr);
    void SetDownloadMsgHandler(MsgHandler msghdr);


    static const std::string QUERY_STRING;
    static const std::string REQUEST_METHOD;
    static const std::string REQUEST_URI;
    static const std::string CGI_NAME;
    static const std::string ACTION;
    static const std::string CONTENT_LENGTH;
    static const std::string CONTENT_TYPE;
    static const std::string HTTP_RANGE;
    
private:
    void FCGILoopHandler();

    void MsgWrite(const char *pBuffer, const unsigned int uiSize, const unsigned int uiModel, FCGX_Request *pRequest);

    void ParseAndHandleMsg(FCGX_Request	*pRequest);

    void ParseMsgOfUpload(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, FCGX_Request *pRequest);

    void ParseMsgOfSpeedUpload(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, FCGX_Request *pRequest);

    void ParseMsgOfDelete(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, FCGX_Request	*pRequest);

    void ParseMsgOfDownload(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, FCGX_Request *pRequest);

private:
    typedef boost::function<void(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, FCGX_Request *pRequest)> ParseMsgFunc;

    ConfigSt& m_cfg;
    unsigned int m_uiTdNum;
    Runner m_MsgLoopRunner;

    std::map<std::string, ParseMsgFunc> m_ParseFuncMap;

    std::map<std::string, MsgHandler> m_MsgHandlerMap;
};


#endif
