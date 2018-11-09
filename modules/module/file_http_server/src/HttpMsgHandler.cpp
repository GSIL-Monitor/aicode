#include "HttpMsgHandler.h"
#include <boost/algorithm/string.hpp>  
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scope_exit.hpp>
#include "json/json.h"
#include "util.h"
#include "mime_types.h"
#include "LogRLD.h"
#include "InteractiveProtoHandler.h"
#include "CommMsgHandler.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include "boost/regex.hpp"
#include "ReturnCode.h"
#include "FileManager.h"


const std::string HttpMsgHandler::SUCCESS_CODE = "0";
const std::string HttpMsgHandler::SUCCESS_MSG = "Ok";
const std::string HttpMsgHandler::FAILED_CODE = "-1";
const std::string HttpMsgHandler::FAILED_MSG = "Inner failed";

const std::string HttpMsgHandler::UPLOAD_FILE_ACTION("upload_file");

const std::string HttpMsgHandler::DOWNLOAD_FILE_ACTION("download_file");

const std::string HttpMsgHandler::DELETE_FILE_ACTION("delete_file");

const std::string HttpMsgHandler::QUERY_FILE_ACTION("query_file");

HttpMsgHandler::HttpMsgHandler(const ParamInfo &parminfo):
m_ParamInfo(parminfo),
m_pInteractiveProtoHandler(new InteractiveProtoHandler)
{

}

HttpMsgHandler::~HttpMsgHandler()
{

}

void HttpMsgHandler::SetFileMgrGroupEx(FileMgrGroupEx *pFileMgrGex)
{
    m_pFileMgrGex = pFileMgrGex;
}

bool HttpMsgHandler::ParseMsgOfCompact(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter wr)
{
    auto itFind = pMsgInfoMap->find("compact_msg");
    if (pMsgInfoMap->end() != itFind)
    {
        const std::string &strValue = itFind->second;

        LOG_INFO_RLD("Compact msg is " << strValue);

        Json::Reader reader;
        Json::Value root;
        if (!reader.parse(strValue, root, false))
        {
            LOG_ERROR_RLD("Compact msg parse failed beacuse value parsed failed and value is " << strValue);
            return false;
        }

        if (!root.isObject())
        {
            LOG_ERROR_RLD("Compact msg parse failed beacuse json root parsed failed and value is " << strValue);
            return false;
        }

        Json::Value::Members members = root.getMemberNames();
        auto itBegin = members.begin();
        auto itEnd = members.end();
        while (itBegin != itEnd)
        {
            if (root[*itBegin].type() == Json::stringValue) //目前只支持string类型json字段
            {
                pMsgInfoMap->insert(MsgInfoMap::value_type(*itBegin, root[*itBegin].asString()));
            }

            ++itBegin;
        }

        pMsgInfoMap->erase(itFind);

    }

    return true;
}

bool HttpMsgHandler::UploadFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("filename");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("File name not found.");
        return blResult;
    }
    const std::string strFileName = itFind->second;
    
    itFind = pMsgInfoMap->find("fileid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("File id not found.");
        return blResult;
    }
    const std::string strFileID = itFind->second;

    LOG_INFO_RLD("Upload file info received and file name is " << strFileName << " and file id is " << strFileID);

    ////
    //std::string strUserID;
    //if (!RegisterUser(strUserName, strUserPwd, strType, strExtend, strAliasName, strEmail, strUserID))
    //{
    //    LOG_ERROR_RLD("Register user handle failed");
    //    return blResult;
    //}
    
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("fileid", strFileID));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::DownloadFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));

            this_->WriteMsg(ResultInfoMap, writer, blResult);
        }

    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("fileid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("File id not found.");
        return blResult;
    }
    const std::string strFileID = itFind->second;
    
    std::string strRange;
    itFind = pMsgInfoMap->find(FCGIManager::HTTP_RANGE);
    if (pMsgInfoMap->end() != itFind)
    {
        strRange = itFind->second;        
    }
    
    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Download file info received and file id is " << strFileID << " and file range is " << strRange);

    if (strRange.empty())
    {
        if (!DownloadFile(strFileID, writer))
        {
            LOG_ERROR_RLD("Download file handle failed");
            return blResult;
        }
    }
    else
    {
        if (!DownloadFileRange(strFileID, strRange, writer))
        {
            LOG_ERROR_RLD("Download file range handle failed");
            return blResult;
        }
    }

    blResult = true;

    return blResult;

}

bool HttpMsgHandler::DeleteFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("fileid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("File id not found.");
        return blResult;
    }
    const std::string strFileID = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Delete file info received and file id is " << strFileID);

    auto pFileMgr = m_pFileMgrGex->GetFileMgr(strFileID);
    if (NULL == pFileMgr.get())
    {
        LOG_ERROR_RLD("Get file mgr failed and file id is " << strFileID);
        return blResult;
    }
    
    std::string strFileIDInner;
    if (!m_pFileMgrGex->GroupFileID2FileID(strFileID, strFileIDInner))
    {
        LOG_ERROR_RLD("Get file id by group file id failed and group file id is " << strFileID);
        return blResult;
    }

    if (!pFileMgr->DeleteFile(strFileIDInner))
    {
        LOG_ERROR_RLD("Delete file failed and file id is " << strFileID);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("fileid", strFileID));

    blResult = true;

    return blResult;
}

bool HttpMsgHandler::QueryFileHandler(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, MsgWriter writer)
{
    ReturnInfo::RetCode(ReturnInfo::INPUT_PARAMETER_TOO_LESS);

    bool blResult = false;
    std::map<std::string, std::string> ResultInfoMap;

    BOOST_SCOPE_EXIT(&writer, this_, &ResultInfoMap, &blResult)
    {
        LOG_INFO_RLD("Return msg is writed and result is " << blResult);

        if (!blResult)
        {
            ResultInfoMap.clear();
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", boost::lexical_cast<std::string>(ReturnInfo::RetCode())));
            ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", FAILED_MSG));
        }

        this_->WriteMsg(ResultInfoMap, writer, blResult);
    }
    BOOST_SCOPE_EXIT_END

    auto itFind = pMsgInfoMap->find("fileid");
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("File id not found.");
        return blResult;
    }
    const std::string strFileID = itFind->second;

    ReturnInfo::RetCode(boost::lexical_cast<int>(FAILED_CODE));

    LOG_INFO_RLD("Query file info received and file id is " << strFileID);

    auto pFileMgr = m_pFileMgrGex->GetFileMgr(strFileID);
    if (NULL == pFileMgr.get())
    {
        LOG_ERROR_RLD("Get file mgr failed and file id is " << strFileID);
        return blResult;
    }

    std::string strFileIDInner;
    if (!m_pFileMgrGex->GroupFileID2FileID(strFileID, strFileIDInner))
    {
        LOG_ERROR_RLD("Get file id by group file id failed and group file id is " << strFileID);
        return blResult;
    }

    FileManager::FileSTInfo fileinfo;
    if (!pFileMgr->QueryFile(strFileIDInner, fileinfo))
    {
        LOG_ERROR_RLD("Query file failed and file id is " << strFileID);
        return blResult;
    }

    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retcode", SUCCESS_CODE));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("retmsg", SUCCESS_MSG));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("name", fileinfo.m_strName));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("size", boost::lexical_cast<std::string>(fileinfo.m_uiSize)));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("md5", fileinfo.m_strMd5));
    ResultInfoMap.insert(std::map<std::string, std::string>::value_type("localpath", fileinfo.m_strLocalPath));
    
    blResult = true;

    return blResult;
}

bool HttpMsgHandler::DownloadFile(const std::string &strFileID, MsgWriter writer)
{
    std::string strExt;
    std::string::size_type pos = strFileID.find_last_of('.');
    if (string::npos != pos)
    {
        strExt = strFileID.substr(pos + 1);
    }

    const std::string& strContentType = strExt.empty() ? "application/octet-stream" : find_mime_type(strExt.c_str());

    auto FuncTmp = [&](const char *pBuffer, const unsigned int uiBufferSize, const unsigned int uiBlockStatus, const unsigned int uiFileSize) ->bool
    {
        if (0 == uiBlockStatus)
        {
            char cBuffer[1024] = { 0 };
            snprintf(cBuffer, sizeof(cBuffer), "Content-disposition: attachment; filename=\"%s\"\r\nContent-Type: %s\r\n"
                "Content-Length: %d\r\nStatus: 200 OK\r\n\r\n", strFileID.c_str(), strContentType.c_str(), uiFileSize);

            const std::string strHeader = cBuffer;

            WriteMsg(writer, pBuffer, uiBufferSize, true, strHeader);
        }
        else
        {
            WriteMsg(writer, pBuffer, uiBufferSize, false, "");
        }

        return true;
    };
    
    auto pFileMgr = m_pFileMgrGex->GetFileMgr(strFileID);
    if (NULL == pFileMgr.get())
    {
        LOG_ERROR_RLD("Get file mgr failed and file id is " << strFileID);
        return false;
    }

    std::string strFileIDInner;
    if (!m_pFileMgrGex->GroupFileID2FileID(strFileID, strFileIDInner))
    {
        LOG_ERROR_RLD("Get file id by group file id failed and group file id is " << strFileID);
        return false;
    }

    if (!pFileMgr->ReadFile(strFileIDInner, FuncTmp))
    {
        LOG_ERROR_RLD("Read file failed and file id is " << strFileID);
        return false;
    }

    return true;
}

bool HttpMsgHandler::DownloadFileRange(const std::string &strFileID, const std::string &strRange, MsgWriter writer)
{
    std::string strExt;
    std::string::size_type pos = strFileID.find_last_of('.');
    if (std::string::npos != pos)
    {
        strExt = strFileID.substr(pos + 1);
    }

    const std::string& strContentType = strExt.empty() ? "application/octet-stream" : find_mime_type(strExt.c_str());

    std::string::size_type p1 = strRange.find('=');
    if (std::string::npos == p1)
    {
        LOG_ERROR_RLD("File range info is invalid and value is " << strRange);
        return false;
    }

    std::string strBeginEnd = strRange.substr(p1 + 1);
    
    unsigned int uiBegin = 0;
    unsigned int uiEnd = 0;
    const unsigned int uiMax = 0xFFFFFFFF;
    try
    {
        if ('-' == strBeginEnd.front() && '-' != strBeginEnd.back()) //格式为 -xx
        {
            uiBegin = uiMax;
            std::string strEnd = strBeginEnd.substr(1);
            uiEnd = boost::lexical_cast<unsigned int>(strEnd);

        }
        else if ('-' != strBeginEnd.front() && '-' == strBeginEnd.back()) //格式为 xx-
        {
            std::string strBegin = strBeginEnd.substr(0, strBeginEnd.size() - 1);
            uiBegin = boost::lexical_cast<unsigned int>(strBegin);
            uiEnd = uiMax;

        }
        else if ('-' == strBeginEnd.front() && '-' == strBeginEnd.back()) //格式错误，只有字符 -
        {
            LOG_ERROR_RLD("File range info is invalid and value is " << strRange);
            return false;
        }
        else if ('-' != strBeginEnd.front() && '-' != strBeginEnd.back()) //格式为 xx-xx
        {
            std::string::size_type p2 = strBeginEnd.find('-');
            if (std::string::npos == p2)
            {
                LOG_ERROR_RLD("File range info is invalid and value is " << strBeginEnd);
                return false;
            }

            std::string strBegin = strBeginEnd.substr(0, p2);
            std::string strEnd = strBeginEnd.substr(p2 + 1);

            LOG_INFO_RLD("File range begin is " << strBegin << " and end is " << strEnd);

            uiBegin = boost::lexical_cast<unsigned int>(strBegin);
            uiEnd = boost::lexical_cast<unsigned int>(strEnd);
        }
        else
        {
            LOG_ERROR_RLD("File range info is valid and value is " << strRange);
            return false;
        }

    }
    catch (boost::bad_lexical_cast & e)
    {
        LOG_ERROR_RLD("File range info is invalid and error msg is " << e.what() << " and input is " << strRange);
        return false;
    }
    catch (...)
    {
        LOG_ERROR_RLD("File range info is invalid and input is " << strRange);
        return false;
    }

    std::string strCurrentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string::size_type pt = strCurrentTime.find('T');
    strCurrentTime.replace(pt, 1, std::string(" "));
        
    auto FuncTmp = [&](const char *pBuffer, const unsigned int uiBufferSize, const unsigned int uiBlockStatus, const unsigned int uiFileSize, 
        const unsigned int uiContentLen) ->bool
    {
        if (0 == uiBlockStatus)
        {
            std::string strContentRange = strBeginEnd + "/" + boost::lexical_cast<std::string>(uiFileSize);

            char cBuffer[2048] = { 0 };
            snprintf(cBuffer, sizeof(cBuffer), 
                "Status: 206 Partial Content\r\n"
                "Content-Type: %s\r\n"
                "Accept-Ranges: bytes\r\n"
                "Content-Length: %u\r\n"
                "ETag: \"%s\"\r\n"
                "Last-Modified: %s\r\n"
                "Content-Range: bytes %s\r\n"
                "Cache-Control: max-age=21600\r\n\r\n"                 
                , strContentType.c_str(),
                uiContentLen,
                strFileID.c_str(),
                strCurrentTime.c_str(),
                strContentRange.c_str()
                );

            const std::string strHeader = cBuffer;

            WriteMsg(writer, pBuffer, uiBufferSize, true, strHeader);
        }
        else
        {
            LOG_ERROR_RLD("File block status is error " << uiBlockStatus);
        }

        return true;
    };

    auto pFileMgr = m_pFileMgrGex->GetFileMgr(strFileID);
    if (NULL == pFileMgr.get())
    {
        LOG_ERROR_RLD("Get file mgr failed and file id is " << strFileID);
        return false;
    }

    std::string strFileIDInner;
    if (!m_pFileMgrGex->GroupFileID2FileID(strFileID, strFileIDInner))
    {
        LOG_ERROR_RLD("Get file id by group file id failed and group file id is " << strFileID);
        return false;
    }

    if (!pFileMgr->ReadFileRange(strFileIDInner, FuncTmp, uiBegin, uiEnd))
    {
        LOG_ERROR_RLD("Read file failed and file id is " << strFileID);
        return false;
    }

    return true;
}

void HttpMsgHandler::WriteMsg(const std::map<std::string, std::string> &MsgMap, MsgWriter writer, const bool blResult, boost::function<void(void*)> PostFunc)
{
    Json::Value jsBody;
    auto itBegin = MsgMap.begin();
    auto itEnd = MsgMap.end();
    while (itBegin != itEnd)
    {
        jsBody[itBegin->first] = itBegin->second;

        ++itBegin;
    }

    if (NULL != PostFunc)
    {
        PostFunc((void *)&jsBody);
    }

    //Json::FastWriter fastwriter;
    Json::StyledWriter stylewriter;
    const std::string &strBody = stylewriter.write(jsBody); //fastwriter.write(jsBody);//jsBody.toStyledString();

    //writer(strBody.c_str(), strBody.size(), MsgWriterModel::PRINT_MODEL);

    std::string strOutputMsg;
    if (!blResult)
    {
        strOutputMsg = "Status: 500  Error\r\nContent-Type: text/html\r\n\r\n";
    }
    else
    {
        strOutputMsg = "Status: 200 OK\r\nContent-Type: text/html\r\n\r\n";
    }

    strOutputMsg += strBody;
    strOutputMsg += "\r\n";

    writer(strOutputMsg.c_str(), strOutputMsg.size(), MsgWriterModel::PRINT_MODEL);

    //writer("Content-type: text/*\r\n\r\n", 0, MsgWriterModel::PRINT_MODEL);
    //writer("<title>FastCGI Hello! (C, fcgi_stdio library)</title>\n", 0, MsgWriterModel::PRINT_MODEL);

}

void HttpMsgHandler::WriteMsg(MsgWriter writer, const char *pBuffer, const unsigned int uiBufferSize, const bool IsNeedWriteHead, const std::string &strHeaderMsg)
{
    if (IsNeedWriteHead)
    {
        writer(strHeaderMsg.data(), strHeaderMsg.size(), MsgWriterModel::PRINT_MODEL);
        LOG_INFO_RLD("Write header\r\n[\r\n" << strHeaderMsg << "]");
    }

    writer(pBuffer, uiBufferSize, MsgWriterModel::STREAM_MODEL);
}

