#include "FCGIManager.h"
#include <algorithm>
#include "util.h"
#include "LogRLD.h"
#include <boost/scope_exit.hpp>
#include <boost/algorithm/string.hpp>
#include "FileManager.h"

static std::string FCGX_GETPARAM(const char *name, FCGX_ParamArray envp)
{
    char *pFCGIValue = FCGX_GetParam(name, envp);
    if (NULL == pFCGIValue)
    {
        return std::string("");
    }
    return std::string(pFCGIValue);
}
    

const std::string FCGIManager::QUERY_STRING = "QUERY_STRING";

const std::string FCGIManager::REQUEST_METHOD = "REQUEST_METHOD";

const std::string FCGIManager::REQUEST_URI = "REQUEST_URI";

const std::string FCGIManager::CGI_NAME = "CGI_NAME";

const std::string FCGIManager::ACTION = "ACTION";

const std::string FCGIManager::CONTENT_LENGTH = "CONTENT_LENGTH";

const std::string FCGIManager::CONTENT_TYPE = "CONTENT_TYPE";

const std::string FCGIManager::HTTP_RANGE = "HTTP_RANGE";

const std::string FCGIManager::REMOTE_ADDR = "REMOTE_ADDR";

const unsigned int FCGIManager::CGI_READ_BUFFER_SIZE = 1024;

FCGIManager::FCGIManager(const unsigned int uiRunTdNum) : m_uiTdNum(uiRunTdNum), m_MsgLoopRunner(uiRunTdNum), m_MsgHandleRunner(uiRunTdNum)
{
    int iRet = 0;
    if (0 != (iRet = FCGX_Init()))
    {
        LOG_ERROR_RLD("FCGI init failed and return code is " << iRet);
        return;
    }
    
    LOG_INFO_RLD("FCGI init success.");

    SetParseMsgFunc("POST", boost::bind(&FCGIManager::ParseMsgOfPost, this, _1, _2));
    SetParseMsgFunc("GET", boost::bind(&FCGIManager::ParseMsgOfGet, this, _1, _2));
    
}

FCGIManager::~FCGIManager()
{

}

void FCGIManager::Run(bool isWaitRunFinished /*= false*/)
{
    m_MsgHandleRunner.Run(false);

    for (unsigned int i = 0; i < m_uiTdNum; ++i)
    {
        m_MsgLoopRunner.Post(boost::bind(&FCGIManager::FCGILoopHandler, this));
    }
    
    m_MsgLoopRunner.Run(isWaitRunFinished);

}

void FCGIManager::SetUploadTmpPath(const std::string &strPath)
{
    m_strUploadTmpPath = strPath;
    m_pFileMgr.reset(new FileManager(strPath, 16));
    m_pFileMgr->SetBlockSize(CGI_READ_BUFFER_SIZE);

}

void FCGIManager::SetParseMsgFunc(const std::string &strKey, ParseMsgFunc fn)
{
    m_ParseFuncMap.insert(std::map<std::string, ParseMsgFunc>::value_type(strKey, fn));
}

void FCGIManager::SetMsgHandler(const std::string &strKey, MsgHandler msghdr)
{
    m_MsgHandlerMap.insert(std::map<std::string, MsgHandler>::value_type(strKey, msghdr));
}

void FCGIManager::SetMsgPreHandler(MsgHandler msghdr)
{
    m_MsgPreHandlerList.push_back(msghdr);
}

void FCGIManager::FCGILoopHandler()
{
    FCGX_Request	*pRequest = NULL;

    while (true)
    {
        pRequest = new FCGX_Request;
        int ret = FCGX_InitRequest(pRequest, 0, 0);

        if (0 != ret)
        {
            LOG_ERROR_RLD("FCGI init req error and return code is " << ret);
            delete pRequest;
            pRequest = NULL;
            continue;
        }

        ret = FCGX_Accept_r(pRequest);

        if (0 != ret)
        {
            //释放资源
            FCGX_Detach(pRequest);
            FCGX_Finish_r(pRequest);
            delete pRequest;
            pRequest = NULL;
            LOG_ERROR_RLD("FCGI accept error and return code is " << ret);
            continue;
        }

        m_MsgHandleRunner.Post(boost::bind(&FCGIManager::ParseAndHandleMsg, this, pRequest));
                
    }
}

void FCGIManager::MsgWrite(const char *pBuffer, const unsigned int uiSize, const unsigned int uiModel, FCGX_Request *pRequest)
{
    if (PRINT_MODEL == uiModel)
    {
        FCGX_FPrintF(pRequest->out, pBuffer);
    }
    else if (STREAM_MODEL == uiModel)
    {
        FCGX_PutStr(pBuffer, uiSize, pRequest->out);
    }
    else
    {
        LOG_ERROR_RLD("Unknown msg type that writing to fcgi, type is " << uiModel);
    }
}

void FCGIManager::ParseAndHandleMsg(FCGX_Request *pRequest)
{
    BOOST_SCOPE_EXIT(&pRequest, this_)
    {
        LOG_INFO_RLD("Parse msg and handle completed, request was freed.");
        FCGX_Finish_r(pRequest);
        delete pRequest;
        pRequest = NULL;
    }
    BOOST_SCOPE_EXIT_END


    boost::shared_ptr<MsgInfoMap> pMsgInfoMap(new MsgInfoMap);

    std::string strQueryStr = FCGX_GETPARAM(QUERY_STRING.c_str(), pRequest->envp);
    if (strQueryStr.empty())
    {
        LOG_ERROR_RLD("FCGI get query string is empty.");
        return;
    }

    std::string strAction;
    getURIRequestData(strQueryStr.c_str(), ACTION.c_str(), strAction);
            
    std::string strMethod = FCGX_GETPARAM(REQUEST_METHOD.c_str(), pRequest->envp);
    if (strMethod.empty())
    {
        LOG_ERROR_RLD("FCGI get method string is empty.");
        return;
    }

    std::string strRequestUri = FCGX_GETPARAM(REQUEST_URI.c_str(), pRequest->envp);
    if (strRequestUri.empty())
    {
        LOG_ERROR_RLD("FCGI get request url is empty.");
        return;
    }
    
    std::string strCgiName;
    string::size_type iPos = 0;
    if (string::npos == (iPos = strRequestUri.find_first_of('?')))
    {
        LOG_ERROR_RLD("Request url error : " << strRequestUri);
        return;
    }
    
    std::string	sTmpBuf;
    sTmpBuf = strRequestUri.substr(0, iPos);
    if (string::npos == (iPos = sTmpBuf.find_last_of('/')))
    {
        LOG_ERROR_RLD("Request url error : " << strRequestUri);
        return;
    }
    strCgiName = sTmpBuf.substr(iPos + 1);

    std::string strContentLen = FCGX_GETPARAM(CONTENT_LENGTH.c_str(), pRequest->envp);

    std::string strContentType = FCGX_GETPARAM(CONTENT_TYPE.c_str(), pRequest->envp);

    std::string strHttpRange = FCGX_GETPARAM(HTTP_RANGE.c_str(), pRequest->envp);

    std::string strRemoteAddr = FCGX_GETPARAM(REMOTE_ADDR.c_str(), pRequest->envp);

    if (strAction != "")      pMsgInfoMap->insert(MsgInfoMap::value_type(ACTION, strAction));
    if (strQueryStr != "")    pMsgInfoMap->insert(MsgInfoMap::value_type(QUERY_STRING, strQueryStr));
    if (strMethod != "")      pMsgInfoMap->insert(MsgInfoMap::value_type(REQUEST_METHOD, strMethod));
    if (strRequestUri != "")  pMsgInfoMap->insert(MsgInfoMap::value_type(REQUEST_URI, strRequestUri));
    if (strCgiName != "")     pMsgInfoMap->insert(MsgInfoMap::value_type(CGI_NAME, strCgiName));
    if (strContentLen != "")  pMsgInfoMap->insert(MsgInfoMap::value_type(CONTENT_LENGTH, strContentLen));
    if (strContentType != "") pMsgInfoMap->insert(MsgInfoMap::value_type(CONTENT_TYPE, strContentType));
    if (strHttpRange != "")   pMsgInfoMap->insert(MsgInfoMap::value_type(HTTP_RANGE, strHttpRange));
    if (strRemoteAddr != "")   pMsgInfoMap->insert(MsgInfoMap::value_type(REMOTE_ADDR, strRemoteAddr));


    //首先根据method来解析处理
    {
        std::map<std::string, ParseMsgFunc>::iterator itFind;
        if (m_ParseFuncMap.end() == (itFind = m_ParseFuncMap.find(strMethod)))
        {
            LOG_ERROR_RLD("Not found parse msg handler, method is " << strMethod);
            return;
        }

        itFind->second(pMsgInfoMap, pRequest);
    }

    //然后开始根据业务注册的action函数来继续解析
    {
        std::map<std::string, ParseMsgFunc>::iterator itFind;
        if (m_ParseFuncMap.end() != (itFind = m_ParseFuncMap.find(strAction)))
        {
            itFind->second(pMsgInfoMap, pRequest); //由具体业务再次解析得到具体的业务参数
        }        
    }

    auto itBegin = pMsgInfoMap->begin();
    auto itEnd = pMsgInfoMap->end();
    while (itBegin != itEnd)
    {
        LOG_INFO_RLD("Param info: key=[" << itBegin->first << "],value=[" << itBegin->second << "]");
        
        if (itBegin->second.empty())
        {
            LOG_ERROR_RLD("Param info value is empty and erase this key and value");
            pMsgInfoMap->erase(itBegin++);
            continue;
        }

        ++itBegin;
    }
            
    //根据具体业务调用对应的Handler来进行处理。
    std::map<std::string, MsgHandler>::iterator itFindHandler;
    if (m_MsgHandlerMap.end() == (itFindHandler = m_MsgHandlerMap.find(strAction)))
    {
        LOG_ERROR_RLD("Not found action handler, action error: " << strAction);
        return;
    }

    {
        auto itBegin = m_MsgPreHandlerList.begin();
        auto itEnd = m_MsgPreHandlerList.end();
        while (itBegin != itEnd)
        {
            if (!(*itBegin)(pMsgInfoMap, boost::bind(&FCGIManager::MsgWrite, this, _1, _2, _3, pRequest)))
            {
                LOG_ERROR_RLD("Receive msg prehandler failed.");
                return;
            }

            ++itBegin;
        }
    }

    itFindHandler->second(pMsgInfoMap, boost::bind(&FCGIManager::MsgWrite, this, _1, _2, _3, pRequest));
        
}

void FCGIManager::ParseMsgOfPost(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, FCGX_Request *pRequest)
{
    auto itFind = pMsgInfoMap->find(CONTENT_TYPE);
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Not found content type");
        return;
    }

    //获得Boundary
    const std::string &strContendType = itFind->second;
    std::string::size_type	iPos = 0;
    std::string	sTmpBuf;
    std::string strBoundary;

    if (std::string::npos == (iPos = strContendType.find("boundary=")))
    {
        LOG_ERROR_RLD("Contend type " << strContendType << " error");
        return;
    }

    sTmpBuf = strContendType.substr(iPos + 9);

    if (std::string::npos == (iPos = sTmpBuf.find(";")))
    {
        strBoundary = sTmpBuf;
    }
    else
    {
        strBoundary = sTmpBuf.substr(0, iPos);
    }

    strBoundary = "--" + strBoundary;

    bool blNeedOpen = true;
    std::string strReadBuffer;
    std::string strKey;
    std::string strValue;
    char cReadBuffer[CGI_READ_BUFFER_SIZE] = { 0 };
    while (FCGX_GetLine(cReadBuffer, sizeof(cReadBuffer), pRequest->in))
    {
        strReadBuffer = cReadBuffer;

        LOG_INFO_RLD("FCGI get line is " << strReadBuffer);

        if (std::string::npos != strReadBuffer.find("Content-Disposition: form-data;"))
        {
            if ((std::string::npos != strReadBuffer.find("filename")) && (std::string::npos != strReadBuffer.find("name")))
            {
                std::string strFileName;
                std::size_t PosFind = 0;
                if (std::string::npos != (PosFind = strReadBuffer.find("filename=\"")))
                {
                    PosFind += 10;

                    std::size_t PosFind2 = strReadBuffer.find("\"", PosFind);
                    strFileName = strReadBuffer.substr(PosFind, (PosFind2 - PosFind));

                    boost::algorithm::trim_if(strFileName, boost::algorithm::is_any_of(" \n\r"));
                }

                if (strFileName.empty())
                {
                    LOG_ERROR_RLD("Upload file name is empty, so skipped and continue.");
                    continue;
                }

                pMsgInfoMap->insert(MsgInfoMap::value_type("filename", strFileName));

                //带有文件上传
                //
                // POST http://172.20.122.252/access.cgi?action=device_login HTTP/1.1
                //    User-Agent: curl/7.19.7 (x86_64-redhat-linux-gnu) libcurl/7.19.7 NSS/3.21 Basic ECC zlib/1.2.3 libidn/1.18 libssh2/1.4.2
                //    Host: 172.20.122.252
                //    Accept: * / *
                //    Connection: Keep - Alive
                //    Content - Length : 42517
                //    Expect : 100 - continue
                //    Content - Type : multipart / form - data; boundary = ----------------------------7393dbfddff5
                //
                //    ------------------------------7393dbfddff5
                //    Content - Disposition: form - data; name = "upfile"; filename = "spawn-fcgi"
                //    Content - Type: application / octet - stream

                //    文件开头 

                if (FCGX_GetLine(cReadBuffer, sizeof(cReadBuffer), pRequest->in) && (std::string::npos != std::string(cReadBuffer).find("octet-stream")))
                {
                    if (FCGX_GetLine(cReadBuffer, sizeof(cReadBuffer), pRequest->in))
                    {
                        //上传文件开始处
                        std::string strFileID;
                        if (blNeedOpen)
                        {                            
                            if (!m_pFileMgr->OpenFile(strFileName, strFileID))
                            {
                                LOG_ERROR_RLD("Open file failed and file name is " << strFileName);
                                continue;
                            }
                            blNeedOpen = false;

                            pMsgInfoMap->insert(MsgInfoMap::value_type("fileid", strFileID));
                        }

                        //
                        FileStreamFilter(strFileID, "\r\n" + strBoundary, pRequest);

                        ////
                        //std::size_t PosBoundary;
                        //std::string strTmp;
                        //unsigned int uiBlkID = 0;
                        //int iReadCount = 0;
                        //while ((iReadCount =FCGX_GetStr(cReadBuffer, sizeof(cReadBuffer), pRequest->in)))
                        //{
                        //    strTmp.assign(cReadBuffer, iReadCount);
                        //    if (std::string::npos != (PosBoundary = strTmp.find(strBoundary)))
                        //    {
                        //        const std::string &strEnd = strTmp.substr(0, strTmp.size() - (strBoundary.size() + 2 + 4)); //去掉\r\n和最后--字符
                        //        m_pFileMgr->WriteBuffer(strFileID, strEnd.c_str(), strEnd.length(), uiBlkID);
                        //        break;
                        //    }
                        //    else
                        //    {
                        //        m_pFileMgr->WriteBuffer(strFileID, cReadBuffer, iReadCount, uiBlkID);
                        //    }
                        //    
                        //    ++uiBlkID;
                        //}

                        m_pFileMgr->CloseFile(strFileID);                        
                    }
                }
                else
                {
                    LOG_ERROR_RLD("Content type is errror: " << cReadBuffer);
                }
                
                continue;
            }

            std::size_t PosFind = 0;
            if (std::string::npos != (PosFind = strReadBuffer.find("name=\"")))
            {
                PosFind += 6;

                std::size_t PosFind2 = strReadBuffer.find("\"", PosFind);
                strKey = strReadBuffer.substr(PosFind, (PosFind2 - PosFind));
                
                boost::algorithm::trim_if(strKey, boost::algorithm::is_any_of(" \n\r"));
            }

            if (FCGX_GetLine(cReadBuffer, sizeof(cReadBuffer), pRequest->in))
            {
                LOG_INFO_RLD("FCGI get line is " << cReadBuffer);
                if (FCGX_GetLine(cReadBuffer, sizeof(cReadBuffer), pRequest->in))
                {
                    LOG_INFO_RLD("FCGI get line2 is " << cReadBuffer);

                    strValue = cReadBuffer;

                    boost::algorithm::trim_if(strValue, boost::algorithm::is_any_of(" \n\r"));

                    pMsgInfoMap->insert(MsgInfoMap::value_type(strKey, strValue));

                    LOG_INFO_RLD("FCGI insert param map key is " << strKey << " and value is " << strValue);
                }
            }
        }
    }        
}

void FCGIManager::ParseMsgOfGet(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, FCGX_Request *pRequest)
{
    auto itFind = pMsgInfoMap->find(QUERY_STRING);
    if (pMsgInfoMap->end() == itFind)
    {
        LOG_ERROR_RLD("Not found query_string type");
        return;
    }

    const std::string& strQueryString = itFind->second;
    std::vector<std::string> vDownloadParas;
    auto it_size_pre = 0;
    auto it_size = strQueryString.find('&'); 
    while (it_size != std::string::npos)
    {
        const std::string& strParas = strQueryString.substr(it_size_pre, it_size - it_size_pre);
        //LOG_INFO_RLD("Download Paras: " << strParas << ", it_size_pre: " << it_size_pre << ", it_size: " << it_size);
        vDownloadParas.push_back(strParas);
        it_size_pre = it_size + 1;
        it_size = strQueryString.find('&', it_size_pre);
    } 

    std::string last = strQueryString.substr(it_size_pre);
    if (last != "")
    {
        vDownloadParas.push_back(last);
    }

    for (auto itParas = vDownloadParas.begin(); itParas != vDownloadParas.end(); itParas++)
    {
        const std::string& strParams = *itParas;
        auto itFind = strParams.find('=');
        if (itFind != std::string::npos)
        {
            std::string strKey = strParams.substr(0, itFind);
            std::transform(strKey.begin(), strKey.end(), strKey.begin(), ::tolower);
            pMsgInfoMap->insert(MsgInfoMap::value_type(strKey, strParams.substr(itFind + 1)));
        }
    }
}

void FCGIManager::FileStreamFilter(const std::string &strFileID, const std::string &strFileterString, FCGX_Request *pRequest)
{
    unsigned int uiBlkID = 0;
    bool blFlagFinded = false;
    int iLoop = 0;
    std::string strNornalBuffer;
    std::string strFilterBuffer;
    int iReadChar = 0;
    while (-1 != (iReadChar = FCGX_GetChar(pRequest->in)))
    {
        if (strFileterString[iLoop] == (char)iReadChar)
        {
            strFilterBuffer += (char)iReadChar;
            if (strFilterBuffer == strFileterString)
            {
                blFlagFinded = true;
                break;
            }

            ++iLoop;
        }
        else
        {
            if (!strFilterBuffer.empty())
            {
                if (strNornalBuffer.empty())
                {
                    strNornalBuffer = strFilterBuffer;
                }
                else
                {
                    strNornalBuffer.append(strFilterBuffer);
                    //strNornalBuffer = strNornalBuffer + strFilterBuffer;
                }
                
                strFilterBuffer.clear();
            }
            strNornalBuffer += (char)iReadChar;

            iLoop = 0;

            if (strNornalBuffer.size() == CGI_READ_BUFFER_SIZE)
            {
                m_pFileMgr->WriteBuffer(strFileID, strNornalBuffer.c_str(), strNornalBuffer.length(), uiBlkID);
                strNornalBuffer.clear();

                ++uiBlkID;
            }            
        }
    }

    if (!strFilterBuffer.empty() && !blFlagFinded) //组装好正常buffer，包括了过滤字符
    {
        if (strNornalBuffer.empty())
        {
            strNornalBuffer = strFilterBuffer;
        }
        else
        {
            strNornalBuffer.append(strFilterBuffer);
            //strNornalBuffer = strNornalBuffer + strFilterBuffer;
        }

        strFilterBuffer.clear();
    }

    if (!strNornalBuffer.empty())
    {
        m_pFileMgr->WriteBuffer(strFileID, strNornalBuffer.c_str(), strNornalBuffer.length(), uiBlkID);
        strNornalBuffer.clear();
    }


}
