#include "FCGIManager.h"
#include <algorithm>
#include "util.h"
#include "LogRLD.h"

FCGIManager::FCGIManager(ConfigSt& cfg, const unsigned int uiRunTdNum) : m_cfg(cfg), m_uiTdNum(uiRunTdNum), m_MsgLoopRunner(uiRunTdNum)
{
    FCGX_Init();

    m_ParseFuncMap.insert(std::map<std::string, ParseMsgFunc>::value_type("upload_file", boost::bind(&FCGIManager::ParseMsgOfUpload, this, _1, _2)));
    m_ParseFuncMap.insert(std::map<std::string, ParseMsgFunc>::value_type("speed_upload_file", boost::bind(&FCGIManager::ParseMsgOfSpeedUpload, this, _1, _2)));
    m_ParseFuncMap.insert(std::map<std::string, ParseMsgFunc>::value_type("delete_file", boost::bind(&FCGIManager::ParseMsgOfDelete, this, _1, _2)));
    m_ParseFuncMap.insert(std::map<std::string, ParseMsgFunc>::value_type("download_file", boost::bind(&FCGIManager::ParseMsgOfDownload, this, _1, _2)));

}

FCGIManager::~FCGIManager()
{

}

void FCGIManager::Run(bool isWaitRunFinished /*= false*/)
{
    
    for (unsigned int i = 0; i < m_uiTdNum; ++i)
    {
        m_MsgLoopRunner.Post(boost::bind(&FCGIManager::FCGILoopHandler, this));
    }
    
    m_MsgLoopRunner.Run(isWaitRunFinished);

}



void FCGIManager::SetUploadMsgHandler(MsgHandler msghdr)
{
    m_MsgHandlerMap.insert(std::map<std::string, MsgHandler>::value_type("upload_file", msghdr));
}

void FCGIManager::SetSpeedUploadMsgHandler(MsgHandler msghdr)
{
    m_MsgHandlerMap.insert(std::map<std::string, MsgHandler>::value_type("speed_upload_file", msghdr));
}

void FCGIManager::SetDeleteMsgHandler(MsgHandler msghdr)
{
    m_MsgHandlerMap.insert(std::map<std::string, MsgHandler>::value_type("delete_file", msghdr));
}

void FCGIManager::SetDownloadMsgHandler(MsgHandler msghdr)
{
    m_MsgHandlerMap.insert(std::map<std::string, MsgHandler>::value_type("download_file", msghdr));
}

const std::string FCGIManager::QUERY_STRING = "QUERY_STRING";

const std::string FCGIManager::REQUEST_METHOD = "REQUEST_METHOD";

const std::string FCGIManager::REQUEST_URI = "REQUEST_URI";

const std::string FCGIManager::CGI_NAME = "CGI_NAME";

const std::string FCGIManager::ACTION = "ACTION";

const std::string FCGIManager::CONTENT_LENGTH = "CONTENT_LENGTH";

const std::string FCGIManager::CONTENT_TYPE = "CONTENT_TYPE";

const std::string FCGIManager::HTTP_RANGE = "HTTP_RANGE";

void FCGIManager::FCGILoopHandler()
{
    FCGX_Request	*pRequest = NULL;

    while (true)
    {
        pRequest = new FCGX_Request;

        FCGX_InitRequest(pRequest, 0, 0);

        //FCGX_Detach( pRequest );
        //pthread_mutex_lock(&accept_mutex);

        int ret = FCGX_Accept_r(pRequest);
        //pthread_mutex_unlock(&accept_mutex);

        if (0 > ret)
        {
            //释放资源
            FCGX_Detach(pRequest);
            FCGX_Finish_r(pRequest);
            delete pRequest;
            pRequest = NULL;
        }

        ParseAndHandleMsg(pRequest);

        FCGX_Finish_r(pRequest);
        delete pRequest;
        pRequest = NULL;
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
    boost::shared_ptr<MsgInfoMap> pMsgInfoMap(new MsgInfoMap);

    std::string strQueryStr = FCGX_GetParam(QUERY_STRING.c_str(), pRequest->envp);
    std::string strAction;
    getURIRequestData(strQueryStr.c_str(), ACTION.c_str(), strAction);
        
    std::string strMethod = FCGX_GetParam(REQUEST_METHOD.c_str(), pRequest->envp);
    std::string strRequestUri = FCGX_GetParam(REQUEST_URI.c_str(), pRequest->envp);
    std::string strCgiName;

    if (!strRequestUri.empty())
    {
        string::size_type iPos = 0;

        if (string::npos == (iPos = strRequestUri.find_first_of('?')))
        {
            LOG_ERROR_RLD("Request url error : " << strRequestUri);
            return;
        }
        else
        {
            std::string	sTmpBuf;
            sTmpBuf = strRequestUri.substr(0, iPos);
            if (string::npos == (iPos = sTmpBuf.find_last_of('/')))
            {
                LOG_ERROR_RLD("Request url error : " << strRequestUri);
                return;
            }
            strCgiName = sTmpBuf.substr(iPos + 1);
        }
    }

    std::string strContentLen = FCGX_GetParam(CONTENT_LENGTH.c_str(), pRequest->envp);

    std::string strContentType = FCGX_GetParam(CONTENT_TYPE.c_str(), pRequest->envp);

    std::string strHttpRange = FCGX_GetParam(HTTP_RANGE.c_str(), pRequest->envp);

    if (strAction != "")      pMsgInfoMap->insert(MsgInfoMap::value_type(ACTION, strAction));
    if (strQueryStr != "")    pMsgInfoMap->insert(MsgInfoMap::value_type(QUERY_STRING, strQueryStr));
    if (strMethod != "")      pMsgInfoMap->insert(MsgInfoMap::value_type(REQUEST_METHOD, strMethod));
    if (strRequestUri != "")  pMsgInfoMap->insert(MsgInfoMap::value_type(REQUEST_URI, strRequestUri));
    if (strCgiName != "")     pMsgInfoMap->insert(MsgInfoMap::value_type(CGI_NAME, strCgiName));
    if (strContentLen != "")  pMsgInfoMap->insert(MsgInfoMap::value_type(CONTENT_LENGTH, strContentLen));
    if (strContentType != "") pMsgInfoMap->insert(MsgInfoMap::value_type(CONTENT_TYPE, strContentType));
    if (strHttpRange != "")   pMsgInfoMap->insert(MsgInfoMap::value_type(HTTP_RANGE, strHttpRange));
    
    std::map<std::string, ParseMsgFunc>::iterator itFind;
    if (m_ParseFuncMap.end() == (itFind = m_ParseFuncMap.find(strAction)))
    {
        LOG_ERROR_RLD("Not found parse msg handler, action error: " << strAction);
        return;
    }

    itFind->second(pMsgInfoMap, pRequest); //由具体业务再次解析得到具体的业务参数

    auto itBegin = pMsgInfoMap->begin();
    auto itEnd = pMsgInfoMap->end();
    while (itBegin != itEnd)
    {
        LOG_INFO_RLD("Param info: key=[" << itBegin->first << "],value=[" << itBegin->second << "]");
        ++itBegin;
    }

    
    //根据具体业务调用对应的Handler来进行处理。
    std::map<std::string, MsgHandler>::iterator itFindHandler;
    if (m_MsgHandlerMap.end() == (itFindHandler = m_MsgHandlerMap.find(strAction)))
    {
        LOG_ERROR_RLD("Not found action handler, action error: " << strAction);
        return;
    }

    itFindHandler->second(pMsgInfoMap, boost::bind(&FCGIManager::MsgWrite, this, _1, _2, _3, pRequest));
        
}

void FCGIManager::ParseMsgOfUpload(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, FCGX_Request *pRequest)
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

    if (string::npos == (iPos = strContendType.find("boundary=")))
    {
        LOG_ERROR_RLD("Contend type " << strContendType << " error");
        return;
    }

    sTmpBuf = strContendType.substr(iPos + 9);

    if (string::npos == (iPos = sTmpBuf.find(";")))
    {
        strBoundary = sTmpBuf;
    }
    else
    {
        strBoundary = sTmpBuf.substr(0, iPos);
    }

    strBoundary = "--" + strBoundary;

    //根据boundary，获取其他各个参数
    char	szKey[256] = { 0 };
    //char	szFileName[512];
    char	szLineBuf[1024] = { 0 };
    char	*p = NULL;
    char	*p1 = NULL;
    char	*p2 = NULL;

    int		iLineLen = 0;

    bool	bIsValue = false;
    bool	bIsHead = false;

    string	sBoundaryEnd;
    string	sTmpData;

    //m_sBoundry		=	"--" + m_sBoundry;
    sBoundaryEnd += strBoundary + "--";

    memset(szLineBuf, 0x00, sizeof(szLineBuf));

    while (FCGX_GetLine(szLineBuf, sizeof(szLineBuf) - 1, pRequest->in))
    {
        trim(szLineBuf);
        iLineLen = strlen(szLineBuf);
        if (0 == iLineLen)
        {
            memset(szLineBuf, 0x00, sizeof(szLineBuf));
            if (bIsHead)
            {
                bIsValue = true;
                bIsHead = false;
            }
            continue;
        }
        p = szLineBuf;

        if (strBoundary == p)
        {
            if (bIsValue)
            {
                std::transform(szKey, szKey + strlen(szKey), szKey, ::tolower);
                pMsgInfoMap->insert(MsgInfoMap::value_type(szKey, sTmpData));

                sTmpData = "";
                bIsValue = false;
            }
            memset(szLineBuf, 0x00, sizeof(szLineBuf));
            continue;
        }

        if (!strncasecmp(p, "Content-Disposition:", 20))  // title
        {
            if (NULL == (p1 = strstr(p, "name=\"")))
            {
                return;
            }

            p1 += 6;

            if (NULL == (p2 = strchr(p1, '\"')))
            {
                return;
            }

            memset(szKey, 0x00, sizeof(szKey));
            snprintf(szKey, p2 - p1 + 1, p1);

            //bIsValue = true;
            bIsHead = true;
        }
        else   //value
        {
            if (bIsValue)
            {
                sTmpData += p;
            }
        }

        memset(szLineBuf, 0x00, sizeof(szLineBuf));
    }

    //确保businessid值一定存在
    auto itFindBusiness = pMsgInfoMap->find("businessid");
    if (pMsgInfoMap->end() == itFindBusiness)
    {
        pMsgInfoMap->insert(MsgInfoMap::value_type("businessid", "0"));

    }

    pMsgInfoMap->insert(MsgInfoMap::value_type("speedupload", "1"));
    pMsgInfoMap->insert(MsgInfoMap::value_type("speeduploadmodel", "1"));
}

void FCGIManager::ParseMsgOfSpeedUpload(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, FCGX_Request *pRequest)
{
    ParseMsgOfDownload(pMsgInfoMap, pRequest);  //解析规则和download相同，GET方式参数
}

void FCGIManager::ParseMsgOfDelete(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, FCGX_Request *pRequest)
{

}

void FCGIManager::ParseMsgOfDownload(boost::shared_ptr<MsgInfoMap> pMsgInfoMap, FCGX_Request *pRequest)
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
