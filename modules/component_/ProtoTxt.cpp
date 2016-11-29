#include "ProtoTxt.h"
#include "CommonUtility.h"

#ifdef WIN32
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

ProtoTxt::ProtoTxt()
{
}


ProtoTxt::~ProtoTxt()
{
}

char *ProtoTxt::Pack(const std::string &strSrcID, const std::string &strDstID, const std::string &strType, const char *pContentBuffer, 
    const unsigned int uiContentBufferLen, unsigned int &uiTotalLen, const bool IsNeedEncode)
{
    //RG,Len,src,dst,type,body,crc crc是指"src,dst,type,body"
    //RG,40,storage_upload_11111111111111111,0,0,0,0

    //content base64 encode
    std::string strMsgContent;
    const std::string &strContentEncoded = IsNeedEncode ? 
        Encode64((const unsigned char*)pContentBuffer, uiContentBufferLen) : 
        strMsgContent.assign(pContentBuffer, uiContentBufferLen);

    const char *pContentBufferEncoded = strContentEncoded.data();
    const unsigned int uiContentBufferLenEncoded = strContentEncoded.size();
    //LOG_INFO_RLD("=====uiContentBufferLenEncoded====: " << uiContentBufferLenEncoded);

    unsigned int uiSrcIDLen = strSrcID.length();
    unsigned int uiDestIDLen = strDstID.length();
    unsigned int uiTypeLen = strType.length();

    unsigned int uiToBeCrcBufferLen = (uiSrcIDLen + 1 + uiDestIDLen + 1 + uiTypeLen + 1 + uiContentBufferLenEncoded);

    //LOG_INFO_RLD("=====uiToBeCrcBufferLen====: " << uiToBeCrcBufferLen);

    unsigned short usCrc = 0;
    if (1)
    {
        char *pTobeCrcBuffer = new char[uiToBeCrcBufferLen + 1];
        snprintf(pTobeCrcBuffer, uiToBeCrcBufferLen + 1, "%s,%s,%s,%s", strSrcID.c_str(), strDstID.c_str(), strType.c_str(), pContentBufferEncoded);
        usCrc = crc16(pTobeCrcBuffer, uiToBeCrcBufferLen);
        delete[] pTobeCrcBuffer;
        pTobeCrcBuffer = NULL;
    }

    char cCovert[256] = { 0 };
    snprintf(cCovert, sizeof(cCovert), "%u", (unsigned int)usCrc);

    std::string strCrc = cCovert; //boost::lexical_cast<std::string>(usCrc);
    unsigned int uiPartEndLen = uiToBeCrcBufferLen + 1 + strCrc.length();

    //LOG_INFO_RLD("=====uiPartEndLen====: " << uiPartEndLen << " ===strCrc===" << strCrc.c_str());

    memset(cCovert, 0, sizeof(cCovert));
    snprintf(cCovert, sizeof(cCovert), "%u", uiPartEndLen);
    std::string strPartEndLen = cCovert; //boost::lexical_cast<std::string>(uiPartEndLen);
    unsigned int uiAllLen = 3 + strPartEndLen.length() + 1 + uiPartEndLen;

    //LOG_INFO_RLD("=====strPartEndLen====: " << strPartEndLen << " ===uiAllLen===" << uiAllLen);

    char *pAllBuffer = new char[uiAllLen + 1];
    memset(pAllBuffer, 0, (uiAllLen + 1));

    //注意，这里snprintf最后一个字节会默认填充0，所以这里需要加一
    //snprintf(pAllBuffer, uiAllLen + 1, "RG,%s,%s,%s,%s,%s,%s", strPartEndLen.c_str(), strSrcID.c_str(), strDstID.c_str(), strType.c_str(), pContentBufferEncoded, strCrc.c_str());
    snprintf(pAllBuffer, uiAllLen + 1, "RG,%s,%s,%s,%s,", strPartEndLen.c_str(), strSrcID.c_str(), strDstID.c_str(), strType.c_str());
    
    unsigned int uiPos1 = 7 + strPartEndLen.size() + strSrcID.size() + strDstID.size() + strType.size();
    memcpy(pAllBuffer + uiPos1, pContentBufferEncoded, uiContentBufferLenEncoded);
    std::string strTmp(",");
    memcpy(pAllBuffer + uiPos1 + uiContentBufferLenEncoded, strTmp.data(), strTmp.size());
    memcpy(pAllBuffer + uiPos1 + uiContentBufferLenEncoded + 1, strCrc.data(), strCrc.size());


    uiTotalLen = uiAllLen;

    return pAllBuffer;
}

bool ProtoTxt::UnPackSingleOne(const char *pOrginalBuffer, const unsigned int uiLenOfOriginal, ClientMsg &Msg, const bool IsNeedValidCrc /*= false*/)
{
    if (NULL == pOrginalBuffer || 3 >= uiLenOfOriginal)
    {
        return false;
    }
    
    if ('R' != pOrginalBuffer[0] || 'G' != pOrginalBuffer[1] || ',' != pOrginalBuffer[2])
    {
        return false;
    }

    unsigned int uiStep = 0;
    uiStep += 3;

    const char *pBodyBegin = NULL;
    const char *pBodyEnd = NULL;
    unsigned int uiLen = 0;
    std::string strSrcID;
    std::string strDstID;
    std::string strType;
    std::string strValue;
    while (1)
    {
        char cValue = *(pOrginalBuffer + uiStep);
        if (',' != cValue)
        {
            strValue += cValue;
        }
        else
        {
            //break;
            uiLen = atoi(strValue.c_str());

            if (uiLenOfOriginal < (uiStep + 1 + uiLen))
            {
                //LOG_ERROR_RLD("Parse protocol buffer error, size of buffer is " << uiSize << " and protocol size is " << (uiStep + 1 + uiLen));
                return false;
            }

            const char *pTmp = pOrginalBuffer + uiStep + 1;
            unsigned int uiIDCn = 0;
            while (1)
            {
                if (0 == uiIDCn)
                {
                    strSrcID += *pTmp++;
                }
                else if (1 == uiIDCn)
                {
                    strDstID += *pTmp++;
                }
                else
                {
                    strType += *pTmp++;
                }

                if (',' == *pTmp)
                {
                    ++uiIDCn;
                    if (3 <= uiIDCn)
                    {
                        pBodyBegin = pTmp + 1;
                        break;
                    }
                    ++pTmp;
                }
            }
            break;
        }

        uiStep += 1;
        if (uiLenOfOriginal <= (uiStep + 1))
        {
            return false;
        }
    }

    uiStep += uiLen;

    //std::string strProto;
    //strProto.assign(pOrginalBuffer, (uiStep + 1));
    const char *pLastPos = pOrginalBuffer + uiLenOfOriginal - 1;
    while (1)
    {
        if (',' == *pLastPos--)
        {
            break;
        }
    }

    pBodyEnd = pLastPos + 1;

    if (0 >= (pBodyEnd - pBodyBegin))
    {
        return false;
    }

    unsigned int uiBodyLen = pBodyEnd - pBodyBegin;
    
    Msg.pContentBuffer.reset(new char[uiBodyLen]);
    Msg.strDstID = strDstID;
    Msg.strSrcID = strSrcID;
    Msg.strType = strType;
    Msg.uiContentBufferLen = uiBodyLen;

    memcpy(Msg.pContentBuffer.get(), pBodyBegin, uiBodyLen);

    return true;
}

bool ProtoTxt::UnPack(std::list<ClientMsg> &TxtMsgList, char *pBuffer, const unsigned int uiSize, std::size_t bytes_transferred,
    char *&pRemainBufferPos, unsigned int &uiRemainBufferSize)
{
    bool blRet = true;

    unsigned int uiReadPos = *(unsigned int*)pBuffer;

    uiReadPos += bytes_transferred;
    *(unsigned int*)pBuffer = uiReadPos;

    char *pUsedBuffer = pBuffer + sizeof(unsigned int);
    unsigned int uiUsedBufferSize = uiSize - sizeof(unsigned int);

    std::list<std::string> *pProtoList = new std::list < std::string >;
    std::list<std::string> *pDstIDList = new std::list < std::string >;

    unsigned int uiTmpSize = uiReadPos;
    char *pBufferNewPos = pUsedBuffer;
    while (1)
    {
        unsigned int uiFlag = 0;
        bool blRet = ParseProtocol(pBufferNewPos, uiTmpSize, pProtoList, uiFlag, pBufferNewPos, pDstIDList);
        if (uiFlag)
        {
            uiTmpSize = *(unsigned int *)(pBufferNewPos - sizeof(unsigned int));//*(boost::uint32_t*)pBuffer;

        }
        else
        {
            if (blRet)
            {
                *(unsigned int*)pBuffer = 0;
            }
            else
            {
                if (!pProtoList->empty())
                {
                    char *pStuff = new char[uiTmpSize];
                    memcpy(pStuff, pBufferNewPos, uiTmpSize);

                    memcpy(pUsedBuffer, pStuff, uiTmpSize);
                    *(unsigned int*)pBuffer = uiTmpSize;

                    delete[] pStuff;
                    pStuff = NULL;
                }
                else
                {
                    //LOG_INFO_RLD("Need continue to reading.");
                }

            }

            break;
        }
    }


    uiReadPos = *(unsigned int*)pBuffer;

    if (!pProtoList->empty())
    {

        char *pBf = pUsedBuffer + uiReadPos;
        unsigned int uiSz = uiUsedBufferSize - uiReadPos;

        pRemainBufferPos = pBf;
        uiRemainBufferSize = uiSz;

        ////
        //m_Func = boost::bind(&TCPSessionOfServer::AsyncRead, pSession, pBf, uiSz, m_uiAsyncReadTimeOut, (void *)pBuffer, 0);

        //m_ProxyHub.ProcessProtocol(boost::shared_ptr<std::list<std::string> >(pProtoList),
        //    boost::shared_ptr<std::list<std::string> >(pDstIDList),
        //    shared_from_this());
        //    
        auto itBegin = pProtoList->begin();
        auto itEnd = pProtoList->end();
        while (itBegin != itEnd)
        {
            ClientMsg tmsg;
            if (!UnPackSingleOne(itBegin->data(), itBegin->size(), tmsg))
            {
                blRet = false;
                break;
            }

            TxtMsgList.push_back(tmsg);

            ++itBegin;
        }
        
    }
    else
    {
        //delete pProtoList;
        //pProtoList = NULL;

        //delete pDstIDList;
        //pDstIDList = NULL;

        //UpdateWriteCallSnapshot();
        //pSession->AsyncRead((pUsedBuffer + uiReadPos), (uiUsedBufferSize - uiReadPos), m_uiAsyncReadTimeOut, (void *)pBuffer);
        // 

        pRemainBufferPos = pUsedBuffer + uiReadPos;
        uiRemainBufferSize = uiUsedBufferSize - uiReadPos;
    }

    delete pProtoList;
    pProtoList = NULL;

    delete pDstIDList;
    pDstIDList = NULL;

    return blRet;
}

bool ProtoTxt::ParseProtocol(char *pBuffer, const unsigned int uiSize, std::list<std::string> *pstrProtoList,
    unsigned int &uiFlag, char *&pBufferNewPos, std::list<std::string> *pDstIDList)
{
    if (3 > uiSize)
    {
        //LOG_ERROR_RLD("Parse protocol buffer size not enought, size is " << uiSize);
        return false;
    }

    unsigned int uiStep = 0;
    if ('R' != *pBuffer || 'G' != *(pBuffer + 1))
    {
        //LOG_ERROR_RLD("Parse protocol buffer not begin with RG");
        return false;
    }

    uiStep += 3;
    if (uiSize <= (uiStep + 1))
    {
        //LOG_ERROR_RLD("Parse protocol buffer size not enought, size is " << uiSize);
        return false;
    }

    unsigned int uiLen = 0;
    std::string strSrcID;
    std::string strDstID;
    std::string strValue;
    while (1)
    {
        char cValue = *(pBuffer + uiStep);
        if (',' != cValue)
        {
            strValue += cValue;
        }
        else
        {
            //break;
            uiLen = atoi(strValue.c_str());

            if (uiSize < (uiStep + 1 + uiLen))
            {
                //LOG_ERROR_RLD("Parse protocol buffer error, size of buffer is " << uiSize << " and protocol size is " << (uiStep + 1 + uiLen));
                return false;
            }

            char *pTmp = pBuffer + uiStep + 1;
            unsigned int uiIDCn = 0;
            while (1)
            {
                if (0 == uiIDCn)
                {
                    strSrcID += *pTmp++;
                }
                else
                {
                    strDstID += *pTmp++;
                }

                if (',' == *pTmp)
                {
                    ++uiIDCn;
                    if (2 <= uiIDCn)
                    {
                        break;
                    }
                    ++pTmp;
                }
            }
            break;
        }

        uiStep += 1;
        if (uiSize <= (uiStep + 1))
        {
            //LOG_ERROR_RLD("Parse protocol buffer not enought, size is " << uiSize << " and protocol size is " << (uiStep + 1));
            return false;
        }
    }

    uiStep += uiLen;

    std::string strProto;
    strProto.assign(pBuffer, (uiStep + 1));

    //PreprocessProtoMsg(strProto, pDstIDList, strSrcID, strDstID);

    pstrProtoList->push_back(strProto);


    if (uiSize == (uiStep + 1))
    {
        //memset((pBuffer - sizeof(boost::uint32_t)), 0, BUFFER_SIZE);
        //printf("buffer clean.\n");
        return true;
    }

    ///////
    unsigned int uiTmpSize = uiSize - uiStep - 1;

    pBufferNewPos = pBuffer + uiStep + 1;
    *(unsigned int *)(pBufferNewPos - sizeof(unsigned int)) = uiTmpSize;

    uiFlag = 1;

    return false;
}

