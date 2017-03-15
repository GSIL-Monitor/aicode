#include "FCGIManager.h"
#include <algorithm>
#include "util.h"
#include "LogRLD.h"
#include <boost/scope_exit.hpp>
#include <boost/algorithm/string.hpp>

const std::string FCGIManager::QUERY_STRING = "QUERY_STRING";

const std::string FCGIManager::REQUEST_METHOD = "REQUEST_METHOD";

const std::string FCGIManager::REQUEST_URI = "REQUEST_URI";

const std::string FCGIManager::CGI_NAME = "CGI_NAME";

const std::string FCGIManager::ACTION = "ACTION";

const std::string FCGIManager::CONTENT_LENGTH = "CONTENT_LENGTH";

const std::string FCGIManager::CONTENT_TYPE = "CONTENT_TYPE";

const std::string FCGIManager::HTTP_RANGE = "HTTP_RANGE";

const std::string FCGIManager::REMOTE_ADDR = "REMOTE_ADDR";

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

    std::string strQueryStr = FCGX_GetParam(QUERY_STRING.c_str(), pRequest->envp);
    std::string strAction;
    getURIRequestData(strQueryStr.c_str(), ACTION.c_str(), strAction);
            
    std::string strMethod = FCGX_GetParam(REQUEST_METHOD.c_str(), pRequest->envp);
    std::string strRequestUri = FCGX_GetParam(REQUEST_URI.c_str(), pRequest->envp);
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

    std::string strContentLen = FCGX_GetParam(CONTENT_LENGTH.c_str(), pRequest->envp);

    std::string strContentType = FCGX_GetParam(CONTENT_TYPE.c_str(), pRequest->envp);

    std::string strHttpRange = FCGX_GetParam(HTTP_RANGE.c_str(), pRequest->envp);

    std::string strRemoteAddr = FCGX_GetParam(REMOTE_ADDR.c_str(), pRequest->envp);

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

    bool IsExistValueEmpty = false;
    auto itBegin = pMsgInfoMap->begin();
    auto itEnd = pMsgInfoMap->end();
    while (itBegin != itEnd)
    {
        LOG_INFO_RLD("Param info: key=[" << itBegin->first << "],value=[" << itBegin->second << "]");
        IsExistValueEmpty = itBegin->second.empty();
        ++itBegin;
    }

    if (IsExistValueEmpty)
    {
        LOG_ERROR_RLD("Empty param value is exist.");
        return;
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

    std::string strReadBuffer;
    std::string strKey;
    std::string strValue;
    char cReadBuffer[1024] = { 0 };
    while (FCGX_GetLine(cReadBuffer, sizeof(cReadBuffer), pRequest->in))
    {
        strReadBuffer = cReadBuffer;

        LOG_INFO_RLD("FCGI get line is " << strReadBuffer);

        if (std::string::npos != strReadBuffer.find("Content-Disposition: form-data;"))
        {
            if ((std::string::npos != strReadBuffer.find("filename")) && (std::string::npos != strReadBuffer.find("name")))
            {
                LOG_ERROR_RLD("Invalid read line " << strReadBuffer);
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
