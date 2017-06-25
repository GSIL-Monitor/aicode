#ifndef FCGI_MANAGER
#define FCGI_MANAGER

#include "NetComm.h"
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include "fcgiapp.h"


/************************************************************************/
/* FCGI管理器，负责将FCGI相关的信息解析出来，提供给业务使用，
 * 提供HTTP通道，供给业务进行写入HTTP消息（业务处理结果信息或者文件流信息）*/
/************************************************************************/

class FileManager;

typedef boost::function<void(const char *, const unsigned int, const unsigned int)> MsgWriter;
typedef enum
{
   PRINT_MODEL = 0,
   STREAM_MODEL
} MsgWriterModel;

typedef std::map<std::string, std::string> MsgInfoMap;
typedef boost::function<bool(boost::shared_ptr<MsgInfoMap>, MsgWriter)> MsgHandler;

typedef boost::function<void(boost::shared_ptr<MsgInfoMap>, FCGX_Request *)> ParseMsgFunc;

class FCGIManager : public boost::noncopyable
{
public:
    FCGIManager(const unsigned int uiRunTdNum);
    ~FCGIManager();

    void Run(bool isWaitRunFinished = false);

    void SetUploadTmpPath(const std::string &strPath);

    void SetParseMsgFunc(const std::string &strKey, ParseMsgFunc fn); //消息解析处理

    void SetMsgHandler(const std::string &strKey, MsgHandler msghdr); //消息处理

    void SetMsgPreHandler(MsgHandler msghdr); //消息预处理

    boost::shared_ptr<FileManager> GetFileMgr();
    
private:
    void FCGILoopHandler();

    void MsgWrite(const char *pBuffer, const unsigned int uiSize, const unsigned int uiModel, FCGX_Request *pRequest);

    void ParseAndHandleMsg(FCGX_Request	*pRequest);

    void ParseMsgOfPost(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, FCGX_Request *pRequest);

    void ParseMsgOfGet(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, FCGX_Request *pRequest);

    void FileStreamFilter(const std::string &strFileID,  const std::string &strFileterString, FCGX_Request *pRequest);

private:
    
    unsigned int m_uiTdNum;
    Runner m_MsgLoopRunner;
    Runner m_MsgHandleRunner;

    std::map<std::string, ParseMsgFunc> m_ParseFuncMap;

    std::map<std::string, MsgHandler> m_MsgHandlerMap;

    std::list<MsgHandler> m_MsgPreHandlerList;

    std::string m_strUploadTmpPath;

    boost::shared_ptr<FileManager> m_pFileMgr;

public:
    static const std::string QUERY_STRING;
    static const std::string REQUEST_METHOD;
    static const std::string REQUEST_URI;
    static const std::string CGI_NAME;
    static const std::string ACTION;
    static const std::string CONTENT_LENGTH;
    static const std::string CONTENT_TYPE;
    static const std::string HTTP_RANGE;
    static const std::string REMOTE_ADDR;

    static const unsigned int CGI_READ_BUFFER_SIZE;
};


#endif
