#ifndef FILE_HANDLER
#define FILE_HANDLER

#include "NetComm.h"
#include <vector>
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include "FCGIManager.h"
#include "ConfigSt.h"

/************************************************************************/
/*负责实现文件处理具体细节，向FCGIManager注册处理函数，
 *其中包括上传、极速上传、删除、下载*/
/************************************************************************/

enum MsgHandleCode
{
    UPLOADFILE_FAILED = 0,
    UPLOADFILE_SUCCESS = 1,
    DOWNLOADFILE_FAILED = 2,
    DOWNLOADBLOCK_SUCCESS = 3,
    DOWNLOADFILE_SUCCESS = 4,
};

#define E_HTTP_ZIP_ERRORCODE_BASE          40000
#define E_HTTP_ZIP_INNER_FAILED            40001
#define E_HTTP_ZIP_FILESIZE_LIMITED_FAILED 40002

#define E_HTTP_BATCH_DOWNLOAD_FAILED       40011
#define E_HTTP_OPEN_SUMMARY_FILE_FAILED    40012
#define E_HTTP_WRITE_SUMMARY_FILE_FAILED   40013

struct stHttpFileInfo
{
    stHttpFileInfo() :errcode(0), md5isfromdb(0), uiFileSize(0) {}
    int errcode;
    int md5isfromdb;
    std::string filemd5;
    std::string createdate;
    std::string extend;
    std::string fileid;
    std::string filename;
    unsigned long uiFileSize;
    std::string uuid_name;
    std::string uuid_path;
    std::string strLocalPath; //with filename
};

class FileHandler : public boost::noncopyable
{
public:
    FileHandler(ConfigSt& cfg);
    ~FileHandler();

    bool Init();

    void UploadHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);
    void SpeedUploadHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);
    void DeleteHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);
    void DownloadHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer);

    static void BlockCB(const char* buff, int len, int range_start, int range_end, int range_size, int filesize, bool first_block, const char* md5, const char* serverfileid, void* pValue);
    
    void WriteBlock(const char* buff, int len, int range_start, int range_end, int range_size, int filesize, bool first_block, const char* md5, const char* serverfileid, void* pValue);

private:
    void WriteMsg(MsgWriter writer, const std::string &strOutputMsg, const MsgHandleCode MsgCode);
    void WriteFileToCgi(MsgWriter writer, const std::string& strFileName, unsigned long uiFileSize);

    int GetRange(const std::string& strRange, int& nStart, int& nEnd);
    void DownloadBlock(MsgWriter writer, const std::string& strDevid, const std::string& strTicket, const std::string& strFileid, const std::string& strRange);
    void DownloadFile(MsgWriter writer, const std::string& strDevid, const std::string& strTicket, const std::vector<std::string>& vFileids);

    std::string GetExtName(const std::string& strFileName, const std::string& strFileId, const std::string& strLocalFileName) const;

    std::string GetServerFileId(const std::string& serverfileid, std::map<std::string, std::string>* mPairValues = NULL) const;

    int DoDownloadFile(const std::string& strDevid, const std::string& strTicket, const std::string& strFileId, stHttpFileInfo& httpinfo);

    void WriteTargetFile(MsgWriter writer, const stHttpFileInfo& httpinfo);

    std::string CombineFileInfo(const stHttpFileInfo& httpinfo);

    std::string GetTargetFile(const std::string& strLocalPath, const std::vector<stHttpFileInfo>& vTargetFiles, int& errcode);

    stHttpFileInfo PackageTraget(const std::string& uuid_path, const std::vector<stHttpFileInfo>& vTargetFiles);

    bool RenameBatchDownFile(stHttpFileInfo& httpinfo, boost::system::error_code& e);

    bool DownloadPathExam(MsgWriter writer, const std::string& uuid_path);

private:
    bool UploadSecurityAuth();

private:
    ConfigSt&   m_cfg;
    std::string m_strHost;
    std::string m_strPort;
    unsigned int m_uiTimeout;
    unsigned int m_uiBatchaDownload;
    unsigned long m_ulMaxDownloadSize;
    std::string m_strDownloadPath;

};


#endif
