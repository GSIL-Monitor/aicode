#include "FileHandler.h"
#include <dirent.h>
#include <boost/algorithm/string.hpp>  
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scope_exit.hpp>
#include "file_api.h"
#include "json/json.h"
#include "zipfile/ZipUtility.h"
#include "util.h"
#include "MD5.h"
#include "mime_types.h"
#include "LogRLD.h"

std::string CreateUUID();

struct PValueBlock
{
    void* handle;
    void* writer;
};

void FileHandler::BlockCB(const char* buff, int len, int range_start, int range_end, int range_size, int filesize, bool first_block, const char* md5, const char* serverfileid, void* pValue)
{
    PValueBlock* p = static_cast<PValueBlock*>(pValue);
    if (p == NULL)
        return;

    FileHandler* handle = static_cast<FileHandler*>(p->handle);
    if (handle == NULL)
        return;

    handle->WriteBlock(buff, len, range_start, range_end, range_size, filesize, first_block, md5, serverfileid, p->writer);
}

FileHandler::FileHandler(ConfigSt& cfg) : m_cfg(cfg)
{

}

FileHandler::~FileHandler()
{

}

bool FileHandler::Init()
{
    m_strHost = m_cfg.GetItem("file_server.ip");
    if (0 == m_strHost.size())
    {
        LOG_ERROR_RLD("file_server.ip not config");
        return false;
    }

    m_strPort = m_cfg.GetItem("file_server.port");
    if (0 == m_strPort.size())
    {
        LOG_ERROR_RLD("file_server.port not config");
        return false;
    }

    boost::system::error_code e;
    std::string strFileName = m_cfg.GetItem("sys.mime_types_file");
    if (0 == strFileName.size() || !boost::filesystem::exists(strFileName, e) || boost::filesystem::is_directory(strFileName, e))
    {
        LOG_ERROR_RLD("sys.mime_types_file not config");
        return false;
    }

    if (!load_mime_types(strFileName.c_str()))
    {
        LOG_ERROR_RLD("load_mime_types falied");
        return false;
    }

    std::string strBatchDownload = m_cfg.GetItem("sys.batch_download_max_num");
    if (0 == strBatchDownload.size())
    {
        strBatchDownload = "1000";
        LOG_ERROR_RLD("sys.batch_download_max_num not config");
    }

    m_uiBatchaDownload = atoi(strBatchDownload.c_str());

    std::string strMaxDownloadSize = m_cfg.GetItem("sys.batch_download_max_size");
    if (0 == strMaxDownloadSize.size())
    {
        strMaxDownloadSize = "1073741824";
        LOG_ERROR_RLD("sys.batch_download_max_size not config");
    }

    m_ulMaxDownloadSize = atoi(strMaxDownloadSize.c_str());

    ZipUtility::SetMaxZipFileSize(m_ulMaxDownloadSize);

    std::string strTimeout = m_cfg.GetItem("file_server.timeout");
    if (0 == strTimeout.size())
    {
        strTimeout = "120";
        LOG_ERROR_RLD("file_server.timeout not config");
    }

    m_uiTimeout = atoi(strTimeout.c_str());

    m_strDownloadPath = m_cfg.GetItem("dir.download_save_path");
    if (0 == m_strDownloadPath.size() || !boost::filesystem::exists(m_strDownloadPath, e) || !boost::filesystem::is_directory(m_strDownloadPath, e))
    {
        LOG_ERROR_RLD("dir.download_save_path not config");
        return false;
    }

    LOG_INFO_RLD("FileHandler Init success...");
    return true;
}

void FileHandler::UploadHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    MsgInfoMap::iterator itFind;
    const std::string &strDevid = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find("devid"))) ? "" : itFind->second;

    if (strDevid.empty())
    {
        WriteMsg(writer, "devid is empty.", UPLOADFILE_FAILED);
        LOG_ERROR_RLD("Devid is empty.");
        return;
    }

    const std::string &strTicket = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find("ticket"))) ? "" : itFind->second;

    const std::string &strFileName = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find("uploadfile.name"))) ? "" : itFind->second;

    if (strFileName.empty())
    {
        WriteMsg(writer, "file name is empty.", UPLOADFILE_FAILED);
        LOG_ERROR_RLD("File name is empty.");
        return;
    }

    boost::system::error_code ec;

    const std::string &strFilePath = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find("uploadfile.path"))) ? "" : itFind->second;

    if (strFilePath.empty() || !boost::filesystem::exists(strFilePath, ec) || boost::filesystem::is_directory(strFilePath, ec))
    {
        char buff[256] = { 0 };
        snprintf(buff, sizeof(buff), "file path error, value(%s)", strFilePath.c_str());
        WriteMsg(writer, buff, UPLOADFILE_FAILED);
        LOG_ERROR_RLD(buff);
        return;
    }

    std::string strUploadMd5 = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find("md5"))) ? "" : itFind->second;
    std::transform(strUploadMd5.begin(), strUploadMd5.end(), strUploadMd5.begin(), ::tolower);

    char cMd5Buff[64] = { 0 };
    MD5_File_V2(strFilePath.c_str(), cMd5Buff);
    const std::string& strLocalFileMd5 = cMd5Buff;

    if (strUploadMd5.empty())
    {
        strUploadMd5 = strLocalFileMd5;
    }
    else if (strUploadMd5 != strLocalFileMd5)
    {
        char buff[256] = { 0 };
        snprintf(buff, sizeof(buff), "upload check md5 failed, client'md5(%s), http'md5(%s)", strUploadMd5.c_str(), strLocalFileMd5.c_str());
        WriteMsg(writer, buff, UPLOADFILE_FAILED);
        LOG_ERROR_RLD(buff);
        return;
    }

    const std::string &strExtdata = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find("extdata"))) ? "" : itFind->second;

    const std::string &strBusinessID = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find("businessid"))) ? "0" : itFind->second;

    const std::string &strSpeedUpload = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find("speedupload"))) ? "0" : itFind->second;

    const std::string &strSpeedUploadmodel = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find("speeduploadmodel"))) ? "0" : itFind->second;

    char busi_buffer[256] = { 0 };
    snprintf(busi_buffer, sizeof(busi_buffer), "{\"businessid\":%s,\"speedupload\":%s,\"speeduploadmodel\":%s}", 
        strBusinessID.c_str(), strSpeedUpload.c_str(), strSpeedUploadmodel.c_str());
    std::string strJsBusi = busi_buffer;

    char name_buffer[256] = { 0 };
    snprintf(name_buffer, sizeof(name_buffer), "{\"filename\":\"%s\",\"filepath\":\"%s\"}", strFileName.c_str(), strFilePath.c_str());
    std::string strJsFileName = name_buffer;

    char cFileID[256] = { 0 };
    int iRet = sync_upload_file(m_strHost.c_str(), atoi(m_strPort.c_str()), strDevid.c_str(), strTicket.c_str(), strJsFileName.c_str(), \
        strUploadMd5.c_str(), strExtdata.c_str(), strJsBusi.c_str(), cFileID, m_uiTimeout);

    LOG_INFO_RLD("FileHandler sync_upload_file result[" << iRet << "], host[" << m_strHost << ":" << m_strPort 
        << "], devid[" << strDevid << "], strTicket[" << strTicket << "], JsFileName[" << strJsFileName << "], md5[" << strUploadMd5 << "], extdata["
        << strExtdata << "], JsBusi[" << strJsBusi << "], FileID[" << cFileID << "]");

    if (E_SUCCESS != iRet && E_CLI_SPEED_UPLOAD_SUCCEED != iRet)
    {
        WriteMsg(writer, "upload failed.", UPLOADFILE_FAILED);
    }
    else
    {        
        WriteMsg(writer, cFileID, UPLOADFILE_SUCCESS);
    }

    boost::system::error_code e;
    boost::filesystem::remove(strFilePath, e);
}

void FileHandler::SpeedUploadHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    MsgInfoMap::iterator itFind;
    const std::string &strDevid = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find("devid"))) ? "" : itFind->second;

    if (strDevid.empty())
    {
        WriteMsg(writer, "devid is empty.", UPLOADFILE_FAILED);
        LOG_ERROR_RLD("Devid is empty.");
        return;
    }

    const std::string &strTicket = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find("ticket"))) ? "" : itFind->second;

    const std::string &strFileName = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find("filename"))) ? "" : itFind->second;

    if (strFileName.empty())
    {
        WriteMsg(writer, "file name is empty.", UPLOADFILE_FAILED);
        LOG_ERROR_RLD("File name is empty.");
        return;
    }

    const std::string &strFileSize = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find("filesize"))) ? "" : itFind->second;

    if (strFileSize.empty())
    {
        WriteMsg(writer, "file size is empty.", UPLOADFILE_FAILED);
        LOG_ERROR_RLD("File size is empty.");
        return;
    }

    const std::string &strMd5 = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find("md5"))) ? "" : itFind->second;

    char cFileID[256] = { 0 };
    int iRet = speed_upload_separate(m_strHost.c_str(), atoi(m_strPort.c_str()), strDevid.c_str(), strTicket.c_str(), strFileName.c_str(), atoi(strFileSize.c_str()), strMd5.c_str(), cFileID, m_uiTimeout);

    LOG_INFO_RLD("FileHandler speed_upload_separate result[" << iRet << "], host[" << m_strHost << ":" << m_strPort << "], devid[" 
        << strDevid << "], strTicket[" << strTicket << "], strFileSize[" << strFileSize << "], md5[" << strMd5 << "]");

    if (E_SUCCESS != iRet && E_CLI_SPEED_UPLOAD_SUCCEED != iRet)
    {
        WriteMsg(writer, "upload failed.", UPLOADFILE_FAILED);
    }
    else
    {
        WriteMsg(writer, cFileID, UPLOADFILE_SUCCESS);
    }
}

void FileHandler::DeleteHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{

}

int FileHandler::GetRange(const std::string& strRange, int& nStart, int& nEnd)
{
    std::string::size_type pos1 = strRange.find('=');
    std::string::size_type pos2 = strRange.find('-');
    std::string::size_type pos3 = strRange.find(',');
    if (pos1 == std::string::npos || pos2 == std::string::npos || pos2 < pos1)
        return E_CLI_INVALID_PARAMS;
    if (pos3 != std::string::npos)
        LOG_ERROR_RLD("not support multi range, only fetch first range: " << strRange);

    const std::string& strStart = strRange.substr(pos1 + 1, pos2 - (pos1 + 1));
    const std::string& strEnd = (pos3 == std::string::npos) ? strRange.substr(pos2 + 1) : strRange.substr(pos2 + 1, pos3 - pos2 - 1);

    try
    {
        if (!strStart.empty())
        {
            nStart = boost::lexical_cast<int>(strStart);
        }
        else
        {
            nStart = INT_MAX;
        }

        if (!strEnd.empty())
        {
            nEnd = boost::lexical_cast<int>(strEnd);
        }
        else
        {
            nEnd = INT_MAX;
        }
    }
    catch (...)
    {
        LOG_ERROR_RLD("FileHandler GetRange throw exception: strStart[" << strStart << "], strEnd[" << strEnd << "]");
        return E_CLI_INVALID_PARAMS;
    }

    if (nStart < 0 || nEnd < 0)
        return E_CLI_INVALID_PARAMS;

    return E_SUCCESS;
}

void FileHandler::DownloadBlock(MsgWriter writer, const std::string& strDevid, const std::string& strTicket, const std::string& strFileid, const std::string& strRange)
{
    int nStart = 0;
    int nEnd = 0;
    if (E_SUCCESS != GetRange(strRange, nStart, nEnd))
    {
        WriteMsg(writer, "Range is invalid.", DOWNLOADFILE_FAILED);
        LOG_ERROR_RLD("Range is invalid, start:" << nStart << ", end: " << nEnd);
        return;
    }

    int filesize = 0;
    char md5[64] = {0};
    char serverfileid[1024] = {0};
    
    PValueBlock pValue;
    pValue.handle = this;
    pValue.writer = &writer;

    int iRet = download_file_block(m_strHost.c_str(), atoi(m_strPort.c_str()), strDevid.c_str(), strTicket.c_str(), strFileid.c_str(),
        m_strDownloadPath.c_str(), nStart, nEnd, &filesize, md5, serverfileid, &FileHandler::BlockCB, &pValue, m_uiTimeout);

    LOG_INFO_RLD("FileHandler download_file_block result[" << iRet << "], host[" << m_strHost << ":" << m_strPort << "], devid[" << strDevid << "], strTicket["
        << strTicket << "], strFileid[" << strFileid << "], range[" << nStart << "-" << nEnd << "], filesize[" << filesize << "], md5[" << md5 << "], serverfileid[" << serverfileid << "]");

    if (E_SUCCESS != iRet)
    {
        WriteMsg(writer, "download failed.", DOWNLOADFILE_FAILED);
    }
}

void FileHandler::WriteBlock(const char* buff, int len, int range_start, int range_end, int range_size, int filesize, bool first_block, const char* md5, const char* serverfileid, void* pValue)
{
    MsgWriter* writer = static_cast<MsgWriter*>(pValue);
    if (writer == NULL)
        return;

    if (first_block)
    {
        const std::string& strContentType = "application/octet-stream";
        const std::string& strHeadFileId = strlen(serverfileid) == 0 ? "" : GetServerFileId(serverfileid);

        char md5_extern[64] = { 0 };
        snprintf(md5_extern, sizeof(md5_extern), "FileMD5: %s\r\nMD5IsFromDB: 1\r\n", md5);

        char cBuffer[1024] = { 0 };
        snprintf(cBuffer, sizeof(cBuffer), "%sStatus: 206 Partial Content\r\nContent-Type: %s\r\nContent-Length: %d\r\nContent-Range: bytes %d-%d/%d\r\n%s\r\n",
            strHeadFileId.c_str(), strContentType.c_str(), range_size, range_start, range_end, filesize, (strlen(md5) == 0) ? "" : md5_extern);

        (*writer)(cBuffer, sizeof(cBuffer), MsgWriterModel::PRINT_MODEL);
        LOG_INFO_RLD("WriteCGI header\r\n[\r\n\r\n" << cBuffer << "]");
    }

    (*writer)(buff, len, MsgWriterModel::STREAM_MODEL);
}

std::string FileHandler::GetExtName(const std::string& strFileName, const std::string& strFileId, const std::string& strLocalFileName) const
{
    std::string::size_type pos = strFileName.find_last_of('.');
    if (string::npos != pos)
    {
        return strFileName.substr(pos + 1);
    }

    pos = strFileId.find_last_of('.');
    if (string::npos != pos)
    {
        return strFileId.substr(pos + 1);
    }

    pos = strLocalFileName.find_last_of('.');
    if (string::npos != pos)
    {
        return strLocalFileName.substr(pos + 1);
    }

    return std::string("");
}

std::string FileHandler::GetServerFileId(const std::string& serverfileid, std::map<std::string, std::string>* mPairValues) const
{
    Json::Reader reader;
    Json::Value root;
    std::string http_fileid;
    if (reader.parse(serverfileid, root, false))
    {
        {
            const std::string& key = "createdate";
            if (root[key].isString())
            {
                if (mPairValues) mPairValues->insert(std::make_pair(key, root[key].asString()));
                http_fileid += key + ":" + root[key].asString() + "\r\n";
            }
        }
        {
            const std::string& key = "extend";
            if (root[key].isString())
            {
                if (mPairValues) mPairValues->insert(std::make_pair(key, root[key].asString()));
                http_fileid += key + ":" + root[key].asString() + "\r\n";
            }
        }
        {
            const std::string& key = "fileid";
            if (root[key].isString())
            {
                if (mPairValues) mPairValues->insert(std::make_pair(key, root[key].asString()));
                http_fileid += key + ":" + root[key].asString() + "\r\n";
            }
        }
        {
            const std::string& key = "filename";
            if (root[key].isString())
            {
                if (mPairValues) mPairValues->insert(std::make_pair(key, root[key].asString()));
                http_fileid += key + ":" + root[key].asString() + "\r\n";
            }
        }
    }

    return http_fileid;
}

int FileHandler::DoDownloadFile(const std::string& strDevid, const std::string& strTicket, const std::string& strFileId, stHttpFileInfo& httpinfo)
{
    char md5buff[64] = { 0 };
    char serverfileid[1024] = { 0 };

    int tmpFileSize = 0;
    httpinfo.errcode = sync_download_file(m_strHost.c_str(), atoi(m_strPort.c_str()), strDevid.c_str(), strTicket.c_str(), strFileId.c_str(), httpinfo.strLocalPath.c_str(), &tmpFileSize, md5buff, serverfileid, m_uiTimeout);
    
    LOG_INFO_RLD("FileHandler sync_download_file result[" << httpinfo.errcode << "], host[" << m_strHost << ":" << m_strPort << "], devid[" << strDevid << "], strTicket["
        << strTicket << "], strFileid[" << strFileId << "], strLocalPath[" << httpinfo.strLocalPath << "], uiFileSize[" << httpinfo.uiFileSize << "], md5[" << md5buff << "], serverfileid[" << serverfileid << "]");

    if (httpinfo.errcode == E_SUCCESS)
    {
        httpinfo.uiFileSize = tmpFileSize;
        httpinfo.filemd5 = md5buff;
        if (!httpinfo.filemd5.empty())
        {
            httpinfo.md5isfromdb = 1;
        }

        std::map<std::string, std::string> mPairValues;
        std::string s = GetServerFileId(serverfileid, &mPairValues);
        if (!s.empty() && !mPairValues.empty())
        {
            httpinfo.createdate = mPairValues["createdate"];
            httpinfo.extend = mPairValues["extend"];
            httpinfo.fileid = mPairValues["fileid"];
            httpinfo.filename = mPairValues["filename"];
        }
    }

    return httpinfo.errcode;
}

std::string FileHandler::CombineFileInfo(const stHttpFileInfo& httpinfo)
{
    std::string s;
    s += "createdate: ";
    s += httpinfo.createdate + "\r\n";
    s += "extend: ";
    s += httpinfo.extend + "\r\n";
    s += "fileid: ";
    s += httpinfo.fileid + "\r\n";
    s += "filename: ";
    s += httpinfo.filename + "\r\n";
    return s;
}

void FileHandler::WriteTargetFile(MsgWriter writer, const stHttpFileInfo& httpinfo)
{
    if (E_SUCCESS != httpinfo.errcode)
    {
        char msg[256] = { 0 };
        snprintf(msg, sizeof(msg), "download failed: %d", httpinfo.errcode);
        WriteMsg(writer, msg, DOWNLOADFILE_FAILED);
        LOG_ERROR_RLD(msg);
        return;
    }

    char filemd5[64] = { 0 };
    MD5_File_V2(httpinfo.strLocalPath.c_str(), filemd5);

    if (!httpinfo.filemd5.empty() && strncmp(filemd5, httpinfo.filemd5.c_str(), 32) != 0)
    {
        char msg[256] = { 0 };
        snprintf(msg, sizeof(msg), "download check md5 failed, download'md5(%s), local'md5(%s)\n", httpinfo.filemd5.c_str(), filemd5);
        WriteMsg(writer, msg, DOWNLOADFILE_FAILED);
        LOG_ERROR_RLD(msg);
    }
    else
    {
        const std::string& strExt = GetExtName(httpinfo.filename, httpinfo.fileid, httpinfo.uuid_name);
        const std::string& strContentType = strExt.empty() ? "application/octet-stream" : find_mime_type(strExt.c_str());
        const std::string& strHeadFileId = CombineFileInfo(httpinfo);

        char cBuffer[1024] = { 0 };
        snprintf(cBuffer, sizeof(cBuffer), "%sContent-disposition: attachment; filename=\"%s\"\r\n"
            "Content-Type: %s\r\nContent-Length: %lu\r\nFileMD5: %s\r\nMD5IsFromDB: %d\r\nStatus: 200 OK\r\n\r\n",
            strHeadFileId.c_str(), httpinfo.filename.empty() ? httpinfo.uuid_name.c_str() : httpinfo.filename.c_str(), 
            strContentType.c_str(), httpinfo.uiFileSize, filemd5, httpinfo.md5isfromdb);

        LOG_INFO_RLD("strExt[" << strExt << "], WriteCGI header\r\n[\r\n\r\n" << cBuffer << "]");

        writer(cBuffer, sizeof(cBuffer), MsgWriterModel::PRINT_MODEL);
        WriteFileToCgi(writer, httpinfo.strLocalPath, httpinfo.uiFileSize);
    }
}

std::string FileHandler::GetTargetFile(const std::string& strLocalPath, const std::vector<stHttpFileInfo>& vTargetFiles, int& errcode)
{
    std::string strContentMsg;
    for (auto it = vTargetFiles.begin(); vTargetFiles.end() != it; ++it)
    {
        const stHttpFileInfo& httpinfo = *it;

        char strContentBuff[1024] = { 0 };
        snprintf(strContentBuff, sizeof(strContentBuff), "********localname: %s********\r\n  filename: %s\r\n  createdate: %s\r\n  extend: %s\r\n  fileid: %s\r\n  FileMD5: %s\r\n  MD5IsFromDB: %d\r\n",
            httpinfo.uuid_name.c_str(), httpinfo.filename.c_str(), httpinfo.createdate.c_str(), httpinfo.extend.c_str(), httpinfo.fileid.c_str(), httpinfo.filemd5.c_str(), httpinfo.md5isfromdb);

        strContentMsg = strContentMsg + strContentBuff + "\r\n\r\n";
    }

    time_t t = time(0);
    char tmp[64] = { 0 };
    strftime(tmp, sizeof(tmp), "%Y%m%d%H%M%S", localtime(&t));
    std::string strLocalFileName = strLocalPath + "/summary_" + tmp + ".txt";
    int fd = open(strLocalFileName.c_str(), O_CREAT | O_WRONLY);
    if (fd < 0)
    {
        errcode = E_HTTP_OPEN_SUMMARY_FILE_FAILED;
        LOG_ERROR_RLD("open file[" << strLocalFileName << "] error, errno=" << errno << ", msg=" << strerror(errno));
        close(fd);
        return std::string("");
    }

    unsigned int len = write(fd, strContentMsg.c_str(), strContentMsg.length());
    if (len != strContentMsg.length())
    {
        errcode = E_HTTP_WRITE_SUMMARY_FILE_FAILED;
        LOG_ERROR_RLD("write file[" << strLocalFileName << "] error, errno=" << errno << ", msg=" << strerror(errno));
        close(fd);
        return std::string("");
    }
    close(fd);

    std::string strFileName;
    unsigned int uiResult = ZipUtility::CompressFileFromDir(strLocalPath, strFileName);
    if (uiResult != ZipUtility::ZIP_COMPRESS_SUCCESS)
    {
        errcode = uiResult + E_HTTP_ZIP_ERRORCODE_BASE;
        LOG_ERROR_RLD("ZipUtility CompressFileFromDir failed, strLocalPath[" << strLocalPath << "], error code[" << errcode << "]");
        return std::string("");
    }

    return strFileName;
}

bool FileHandler::DownloadPathExam(MsgWriter writer, const std::string& strLocalPath)
{
    boost::system::error_code e;
    bool bResult = boost::filesystem::exists(strLocalPath, e);
    if (e && e.value() != boost::system::errc::no_such_file_or_directory)
    {
        std::string msg = "DownloadFile server exam directories failed: " + e.message();
        WriteMsg(writer, msg, DOWNLOADFILE_FAILED);
        LOG_ERROR_RLD(msg);
        return false;
    }

    if (bResult)
    {
        boost::filesystem::remove_all(strLocalPath, e);
        if (e)
        {
            std::string msg = "DownloadFile server remove directories failed: " + e.message();
            WriteMsg(writer, msg, DOWNLOADFILE_FAILED);
            LOG_ERROR_RLD(msg);
            return false;
        }
    }

    boost::filesystem::create_directories(strLocalPath, e); //创建临时目录
    if (e)
    {
        std::string msg = "DownloadFile server create directories failed: " + e.message();
        WriteMsg(writer, msg, DOWNLOADFILE_FAILED);
        LOG_ERROR_RLD(msg);
        return false;
    }

    return true;
}

bool FileHandler::RenameBatchDownFile(stHttpFileInfo& httpinfo, boost::system::error_code& e)
{
    int nFileIndex = 0;
    std::string strLocalPath = m_strDownloadPath + "/" + httpinfo.uuid_path;
    std::string strTmpFileName = httpinfo.filename.empty() ? httpinfo.uuid_name : httpinfo.filename;
    std::string strLocalFileName = strLocalPath + "/" + strTmpFileName;
    while (boost::filesystem::exists(strLocalFileName, e)
        && strTmpFileName != httpinfo.uuid_name) //文件名已存在，则进行重命名
    {
        char tmp[32] = { 0 };
        snprintf(tmp, sizeof(tmp), "[%d]", nFileIndex++);
        strTmpFileName = httpinfo.filename + tmp;
        strLocalFileName = strLocalPath + "/" + strTmpFileName;
    }

    boost::filesystem::rename(httpinfo.strLocalPath, strLocalFileName, e);
    if (e)
    {
        return false;
    }

    httpinfo.uuid_name = strTmpFileName;
    httpinfo.strLocalPath = strLocalFileName;
    return true;
}

stHttpFileInfo FileHandler::PackageTraget(const std::string& uuid_path, const std::vector<stHttpFileInfo>& vTargetFiles)
{
    boost::system::error_code e;
    stHttpFileInfo httpinfo;
    httpinfo.errcode = E_SUCCESS;
    std::string strLocalPath = m_strDownloadPath + "/" + uuid_path;
    httpinfo.strLocalPath = GetTargetFile(strLocalPath, vTargetFiles, httpinfo.errcode); //对目录的多个文件进行打包
    if (httpinfo.errcode == E_SUCCESS && httpinfo.strLocalPath != ""
        && boost::filesystem::exists(httpinfo.strLocalPath, e)
        && !boost::filesystem::is_directory(httpinfo.strLocalPath, e))
    {
        if (e)
        {
            httpinfo.errcode = E_HTTP_BATCH_DOWNLOAD_FAILED;
        }
        else
        {
            httpinfo.errcode = E_SUCCESS;
            httpinfo.uiFileSize = GetFilesize(httpinfo.strLocalPath.c_str());
            httpinfo.uuid_name = boost::filesystem::path(httpinfo.strLocalPath).filename().string();
            httpinfo.filename = httpinfo.uuid_name;
            httpinfo.uuid_path = uuid_path;

            //char filemd5[64] = { 0 };
            //MD5_File_V2(httpinfo.strLocalPath.c_str(), filemd5);
            //httpinfo.filemd5 = filemd5;

            time_t t = time(0);
            char tmp[64] = { 0 };
            strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&t));
            httpinfo.createdate = tmp;
        }
    }

    return httpinfo;
}

void FileHandler::DownloadFile(MsgWriter writer, const std::string& strDevid, const std::string& strTicket, const std::vector<std::string>& vFileids)
{
    const std::string uuid_path = CreateUUID();
    std::string strLocalPath = m_strDownloadPath + "/" + uuid_path;
    if (!DownloadPathExam(writer, strLocalPath))
    {
        LOG_ERROR_RLD("FileHandler DownloadPathExam failed.");
        return;
    }

    //删除临时目录
    boost::system::error_code e;
    BOOST_SCOPE_EXIT(&strLocalPath, &e)
    {
        boost::filesystem::remove_all(strLocalPath, e);
    }
    BOOST_SCOPE_EXIT_END

    unsigned int nFailedCount = 0;
    unsigned int bBatchDownload = (vFileids.size() > 1);
    unsigned long ulMaxDownloadSize = 0;
    std::vector<stHttpFileInfo> vTargetFiles;

    //下载单个文件以及多个文件
    for (auto it = vFileids.begin(); vFileids.end() != it; ++it)
    {
        stHttpFileInfo httpinfo;
        httpinfo.uuid_path = uuid_path;
        httpinfo.uuid_name = CreateUUID();
        httpinfo.strLocalPath = strLocalPath + "/" + httpinfo.uuid_name;

        const std::string& strFileId = *it;
        int nResult = DoDownloadFile(strDevid, strTicket, strFileId, httpinfo);
        if (E_SUCCESS != nResult)
        {
            nFailedCount++;
            LOG_ERROR_RLD("DoDownloadFile failed, strDevid[" << strDevid << "], strTicket[" << strTicket << "]");
        }

        if (bBatchDownload && E_SUCCESS == nResult)
        {
            //批量下载的文件总量检测，只检查批量文件，不检查单个文件
            ulMaxDownloadSize += httpinfo.uiFileSize;
            if (ulMaxDownloadSize > m_ulMaxDownloadSize)
            {
                char msg[256] = { 0 };
                snprintf(msg, sizeof(msg), "batch download failed, threshold size: %lu,  total size: %lu", m_ulMaxDownloadSize, ulMaxDownloadSize);
                WriteMsg(writer, msg, DOWNLOADFILE_FAILED);
                LOG_ERROR_RLD(msg);
                return;
            }

            //批量下载文件重命名
            if (!RenameBatchDownFile(httpinfo, e))
            {
                std::string msg = "batch download rename file failed: " + e.message();
                WriteMsg(writer, msg, DOWNLOADFILE_FAILED);
                LOG_ERROR_RLD(msg);
                return;
            }
        }

        vTargetFiles.push_back(httpinfo);
    }

    if (vTargetFiles.size() == nFailedCount && vTargetFiles.size() > 1)
    {
        std::string msg = "Batch download all target file failed.";
        WriteMsg(writer, msg, DOWNLOADFILE_FAILED);
        LOG_ERROR_RLD(msg);
        return;
    }

    const stHttpFileInfo& httpinfo = bBatchDownload ? PackageTraget(uuid_path, vTargetFiles) : vTargetFiles.front();
    WriteTargetFile(writer, httpinfo);
}

void FileHandler::DownloadHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    MsgInfoMap::iterator itFind;
    const std::string &strDevid = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find("devid"))) ? "" : itFind->second;

    if (strDevid.empty())
    {
        WriteMsg(writer, "devid is empty.", DOWNLOADFILE_FAILED);
        LOG_ERROR_RLD("Devid is empty.");
        return;
    }

    const std::string &strTicket = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find("ticket"))) ? "" : itFind->second;

    const std::string &strFileid = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find("fid"))) ? "" : itFind->second;

    if (strFileid.empty())
    {
        WriteMsg(writer, "fid is empty.", DOWNLOADFILE_FAILED);
        LOG_ERROR_RLD("fid is empty.");
        return;
    }

    std::string strFileName = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find("filename"))) ? "" : itFind->second;
    if (!strFileName.empty())
    {
        char szFileName[1024] = { 0 };
        URLDecode(strFileName.c_str(), strFileName.length(), szFileName, sizeof(szFileName));
        strFileName = szFileName;
        LOG_INFO_RLD("URLDecode filename[" << strFileName << "]");
    }

    const std::string &strRange = (pMsgInfoMap->end() == (itFind = pMsgInfoMap->find(FCGIManager::HTTP_RANGE))) ? "" : itFind->second;
    std::vector<std::string> vFileIds;
    boost::split(vFileIds, strFileid, boost::is_any_of("|"));
    if (strRange.empty() || vFileIds.size() > 1) //批量下载时候不支持range参数
    {
        if (vFileIds.size() > m_uiBatchaDownload)
        {
            char buff[1024] = {0};
            snprintf(buff, sizeof(buff), "batch download number no support, server deploy[%d], client download[%d].", m_uiBatchaDownload, (int)vFileIds.size());
            WriteMsg(writer, buff, DOWNLOADFILE_FAILED);
            LOG_ERROR_RLD(buff);
            return;
        }

        DownloadFile(writer, strDevid, strTicket, vFileIds);
    }
    else
    {
        DownloadBlock(writer, strDevid, strTicket, strFileid, strRange);
    }
}

void FileHandler::WriteMsg(MsgWriter writer, const std::string &strOutputMsg, const MsgHandleCode MsgCode)
{
    if (MsgCode == UPLOADFILE_FAILED || MsgCode == DOWNLOADFILE_FAILED)
    {
        std::string strErrMsg = "Status: 500  Error\r\nContent-Type: text/html\r\n\r\nResult:-1<br>\r\nMessage:";
        strErrMsg += strOutputMsg;
        strErrMsg += "\r\n";

        writer(strErrMsg.c_str(), strErrMsg.size(), MsgWriterModel::PRINT_MODEL);
    }
    else if (MsgCode == UPLOADFILE_SUCCESS)
    {        
        char cBuffer[256] = { 0 };
        snprintf(cBuffer, sizeof(cBuffer), 
            "Status: 200 OK\r\nContent-Type: text/html\r\n\r\nResult=0<br>\r\nFileID=%s<br>\r\nMessage=success<br>\r\n",
            strOutputMsg.c_str());
        writer(cBuffer, sizeof(cBuffer), MsgWriterModel::PRINT_MODEL);
    }
}

void FileHandler::WriteFileToCgi(MsgWriter writer, const std::string& strFileName, unsigned long uiFileSize)
{
    int fd = open(strFileName.c_str(), O_RDONLY);
    if (fd < 0)
    {
        LOG_ERROR_RLD("open file[" << strFileName << "] error, errno=" << errno << ", msg=" << strerror(errno));
        close(fd);
        return;
    }

    unsigned long uiReadlen = 0;
    char buff[1024 * 8] = { 0 };
    while (uiReadlen < uiFileSize)
    {
        bzero(buff, sizeof(buff));
        int len = read(fd, buff, sizeof(buff)-1);
        if (0 > len)
        {
            LOG_ERROR_RLD("read file[" << strFileName << "] error, errno=" << errno << ", msg=" << strerror(errno));
            close(fd);
            return;
        }

        writer(buff, len, MsgWriterModel::STREAM_MODEL);
        uiReadlen += len;
    }
    close(fd);
}

bool FileHandler::UploadSecurityAuth()
{
    ////安全鉴权
    //std::string strPwd;
    //bool blRet = GetDevPwd(up_file_ptr->HttpUpFile().up_data()["devid"], strPwd);
    //if (!blRet)
    //{
    //    HttpErrResponse(up_file_ptr->m_request, 500, -1, "Get device password failed.");
    //    return;
    //}

    //if (m_strAuthEnable == std::string("1") && !strPwd.empty())
    //{
    //    CStr2Map &HttpInfoMap = up_file_ptr->HttpUpFile().up_data();
    //    std::string strAuthorization;
    //    auto itFind = HttpInfoMap.find("authorization");
    //    if (HttpInfoMap.end() == itFind)
    //    {
    //        LOG_ERROR_RLD("http authorization of uploading info not found, devid is " << HttpInfoMap["devid"]);
    //        HttpErrResponse(up_file_ptr->m_request, 500, -1, "Authorization string not found.");
    //        return;
    //    }

    //    strAuthorization = itFind->second;

    //    const std::string &strHttpMethod = up_file_ptr->HttpUpFile().GetHttpMethod();
    //    const std::string &strHttpReqURL = up_file_ptr->HttpUpFile().GetHttpReqURL();

    //    std::string strtCanonicalURI = strHttpReqURL.substr(0, (strHttpReqURL.find('?') + 1));


    //    std::list<std::string> keylist;
    //    keylist.push_back("access_token");
    //    keylist.push_back("version");
    //    keylist.push_back("signature_nonce");
    //    keylist.push_back("timestamp");

    //    std::string strCanonicalSignature;

    //    if (!GetCanonicalString(HttpInfoMap, keylist, false, strCanonicalSignature))
    //    {
    //        LOG_ERROR_RLD("get canonical string error of uploading, devid is " << HttpInfoMap["devid"]);
    //        HttpErrResponse(up_file_ptr->m_request, 500, -1, "Canonical param not found.");
    //        return;
    //    }

    //    keylist.clear();
    //    keylist.push_back("devid");
    //    //keylist.push_back("extdata");

    //    //keylist.push_back("filesize");
    //    //keylist.push_back("businessid");
    //    //keylist.push_back("md5");

    //    std::string strCanconicalParam;
    //    GetCanonicalString(HttpInfoMap, keylist, true, strCanconicalParam);

    //    SecurityAuth auth;
    //    auth.SeHttptMethod(strHttpMethod);
    //    auth.SetHost(m_strAuthHostName);
    //    auth.SetCanonicalURI(strtCanonicalURI);
    //    auth.SetCanonicalParam(strCanconicalParam);
    //    auth.SetCanonicalSignature(strCanonicalSignature);
    //    auth.SetAuthorization(strAuthorization);
    //    auth.SetSecretKey(strPwd);

    //    LOG_INFO_RLD("Authorization of uploading info is method = [" << strHttpMethod << "] host =[" << m_strAuthHostName << "] canonicalurl=[" << strtCanonicalURI
    //        << "] canconicalparam=[" << strCanconicalParam << "] canconicalsignature=[" << strCanonicalSignature << "] authorization=[" << strAuthorization << "]");

    //    std::string strOutAuthkey;
    //    if (0 != auth.Authenticate(strOutAuthkey))
    //    {
    //        LOG_ERROR_RLD("Authorization of uploading failed and output authkey is " << strOutAuthkey << ", devid is " << HttpInfoMap["devid"]);
    //        HttpErrResponse(up_file_ptr->m_request, 500, -1, "Authorization failed.");
    //        return;
    //    }

    //    LOG_INFO_RLD("Authorization of uploading successfully, devid is " << HttpInfoMap["devid"]);
    //}
    
    return true;

}
