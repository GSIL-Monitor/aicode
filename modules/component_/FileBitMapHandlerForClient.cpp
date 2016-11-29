#include "FileBitMapHandlerForClient.h"
#include <memory.h>

#define BYTE_BIT_NUM (8*sizeof(char))
const unsigned int FileBitMapHandlerForClient::BLOCK_FULL = 1;
const unsigned int FileBitMapHandlerForClient::BLOCK_EMPTY = 0;

FileBitMapHandlerForClient::FileBitMapHandlerForClient(const std::string &strFileGUID, const unsigned int uiFileSize, const unsigned int uiBlkSize, 
    const unsigned int uiBlkNum, const char *pSaveFilePath) : m_strFileGUID(strFileGUID), m_uiBlkSize(uiBlkSize)
{
    m_pFileBitMap = GenerateEmptyFileBitMap(uiFileSize, uiBlkSize, uiBlkNum, m_uiFileBitMapSize);
    m_uiFileBitMapBitSize = uiBlkNum;

    if (NULL != pSaveFilePath)
    {
        m_strFileSavePath = pSaveFilePath;
    }

}

FileBitMapHandlerForClient::FileBitMapHandlerForClient(const std::string &strFileGUID, const unsigned char *pFileBitMapBuffer, 
    const unsigned int uiLen, const unsigned int uiFileBitMapBitSize, const char *pSaveFilePath) : m_strFileGUID(strFileGUID), 
    m_uiFileBitMapSize(uiLen), m_uiFileBitMapBitSize(uiFileBitMapBitSize), m_uiBlkSize(0)
{
    m_pFileBitMap = NULL;
    if ((uiLen *BYTE_BIT_NUM) < uiFileBitMapBitSize)
    {
        return;
    }

    m_pFileBitMap = new unsigned char[uiLen];
    if (NULL == m_pFileBitMap)
    {
        return;
    }
    
    memcpy(m_pFileBitMap, pFileBitMapBuffer, uiLen);

    if (NULL != pSaveFilePath)
    {
        m_strFileSavePath = pSaveFilePath;
    }
}

FileBitMapHandlerForClient::FileBitMapHandlerForClient(const char *pLoadFilePath, const char *pSaveFilePath) : m_pFileBitMap(NULL), 
    m_uiFileBitMapSize(0), m_uiFileBitMapBitSize(0), m_uiBlkSize(0)
{
    if (NULL != pSaveFilePath)
    {
        m_strFileSavePath = pSaveFilePath;
    }

    FILE *fp = NULL;

    fp = fopen(pLoadFilePath, "rb");
    if (NULL == fp)
    {
        return;
    }

    fread(&m_uiBlkSize, sizeof(m_uiBlkSize), 1, fp);

    unsigned int uiFileGUIDSize = 0;    
    fread(&uiFileGUIDSize, sizeof(uiFileGUIDSize), 1, fp);
    if (0 == uiFileGUIDSize)
    {
        fclose(fp);
        return;
    }
    char *pFileGUID = new char[uiFileGUIDSize];    
    fread(pFileGUID, uiFileGUIDSize, 1, fp);

    m_strFileGUID.assign(pFileGUID, uiFileGUIDSize);
    delete[] pFileGUID;
    pFileGUID = NULL;

    fread(&m_uiFileBitMapBitSize, sizeof(m_uiFileBitMapBitSize), 1, fp);
    fread(&m_uiFileBitMapSize, sizeof(m_uiFileBitMapSize), 1, fp);

    m_pFileBitMap = new unsigned char[m_uiFileBitMapSize];
    if (NULL == m_pFileBitMap)
    {
        m_uiFileBitMapBitSize = 0;
        m_uiFileBitMapSize = 0;
        fclose(fp);
        return;
    }

    fread(m_pFileBitMap, m_uiFileBitMapSize, 1, fp);
    fclose(fp);
}


FileBitMapHandlerForClient::~FileBitMapHandlerForClient(void)
{
    delete[] m_pFileBitMap;
    m_pFileBitMap = NULL;
}

unsigned char * FileBitMapHandlerForClient::GenerateEmptyFileBitMap(const unsigned int uiFileSize, const unsigned int uiBlkSize, 
    const unsigned int uiBlkNum, unsigned int &uiFileBitMapSize)
{

    if (uiFileSize > (uiBlkNum *uiBlkSize)) //Òì³£Çé¿ö
    {
        return NULL;
    }

    unsigned int uiBitMapSize = uiBlkNum / BYTE_BIT_NUM;
    if (0 != uiBlkNum % BYTE_BIT_NUM)
    {
        uiBitMapSize++;
    }

    unsigned char *pBitMap = new unsigned char[uiBitMapSize];
    if (NULL == pBitMap)
    {
        return NULL;
    }

    memset(pBitMap, 0, uiBitMapSize);

    uiFileBitMapSize = uiBitMapSize;
    

    return pBitMap;

}

bool FileBitMapHandlerForClient::SetBlkStatus(const unsigned int uiBlkID, const unsigned int uiBlkStatus)
{
    if (NULL == m_pFileBitMap)
    {
        return false;
    }

    if (uiBlkID > (m_uiFileBitMapSize * BYTE_BIT_NUM - 1) || uiBlkID >= m_uiFileBitMapBitSize)
    {
        return false;
    }

    if ((BLOCK_FULL != uiBlkStatus) && (BLOCK_EMPTY != uiBlkStatus))
    {
        return false;
    }

    unsigned int uiPosByte = (uiBlkID)/BYTE_BIT_NUM;
    unsigned int uiPosByteIn = (uiBlkID)%BYTE_BIT_NUM;
    
    if (uiPosByte > m_uiFileBitMapSize)
    {
        return false;
    }

    unsigned char *pFind = m_pFileBitMap + uiPosByte;

    unsigned char cValue = *pFind;

    unsigned char cPos = 0x80;
    cPos = cPos >> (uiPosByteIn);

    if (BLOCK_FULL == uiBlkStatus)
    {
        (*pFind) = cValue | cPos;
    }
    else if (BLOCK_EMPTY == uiBlkStatus)
    {
        cPos = ~cPos;
        (*pFind) = cValue&cPos;
    }    
    return true;
}

const unsigned char * FileBitMapHandlerForClient::GetFileBitMapBuffer(unsigned int &uiLen)
{
    if (NULL == m_pFileBitMap)
    {
        return NULL;
    }

    unsigned char *pBuffer = new unsigned char[m_uiFileBitMapSize];
    if (NULL == pBuffer)
    {
        return NULL;
    }

    memcpy(pBuffer, m_pFileBitMap, m_uiFileBitMapSize);

    uiLen = m_uiFileBitMapSize;

    return pBuffer;

}

bool FileBitMapHandlerForClient::GetBlkStatus(const unsigned int uiBlkID, unsigned int &uiBlkStatus)
{
    bool blRet = GetBlkStatusInner(uiBlkID, uiBlkStatus);
    return blRet;
}

bool FileBitMapHandlerForClient::GetBlkStatusInner(const unsigned int uiBlkID, unsigned int &uiBlkStatus)
{    
    if (NULL == m_pFileBitMap)
    {
        return false;
    }

    if (uiBlkID > (m_uiFileBitMapSize * BYTE_BIT_NUM - 1) || uiBlkID >= m_uiFileBitMapBitSize)
    {
        return false;
    }

    unsigned int uiPosByte = (uiBlkID)/BYTE_BIT_NUM;
    unsigned int uiPosByteIn = (uiBlkID)%BYTE_BIT_NUM;

    if (uiPosByte > m_uiFileBitMapSize)
    {
        return false;
    }

    unsigned char *pFind = m_pFileBitMap + uiPosByte;

    unsigned char cValue = *pFind;

    unsigned char cPos = 0x80;
    cPos = cPos >> (uiPosByteIn);


    cValue &= cPos;

    uiBlkStatus = cValue ? BLOCK_FULL : BLOCK_EMPTY;
    return true;
}

bool FileBitMapHandlerForClient::SaveFileBitMap(const char * pFileNameSaved)
{

    if ((NULL == pFileNameSaved) && (0 == m_strFileSavePath.size()))
    {
        return false;
    }

    if (NULL == m_pFileBitMap)
    {
        return false;
    }

    const char *pFileSaveUsed = (NULL == pFileNameSaved) ? m_strFileSavePath.c_str() : pFileNameSaved;

    FILE *fp = NULL;

    fp = fopen(pFileSaveUsed, "wb");
    if (NULL == fp)
    {
        return false;
    }
    
    const unsigned int uiFileGUIDSize = m_strFileGUID.size();
    fwrite(&m_uiBlkSize, sizeof(m_uiBlkSize), 1, fp);
    fwrite(&uiFileGUIDSize, sizeof(uiFileGUIDSize), 1, fp);
    fwrite(m_strFileGUID.data(), uiFileGUIDSize, 1, fp);

    fwrite(&m_uiFileBitMapBitSize, sizeof(m_uiFileBitMapBitSize), 1, fp);
    fwrite(&m_uiFileBitMapSize, sizeof(m_uiFileBitMapSize), 1, fp);
    fwrite(m_pFileBitMap, m_uiFileBitMapSize, 1, fp);
    fflush(fp);
    fclose(fp);

    return true;
}

bool FileBitMapHandlerForClient::AllFileBlkIsFull()
{
    if ((NULL == m_pFileBitMap) || (0 == m_uiFileBitMapSize) || (0 == m_uiFileBitMapBitSize))
    {
        return false;
    }

    unsigned int uiPosByte = (m_uiFileBitMapBitSize)/BYTE_BIT_NUM;
    unsigned int uiPosByteIn = (m_uiFileBitMapBitSize)%BYTE_BIT_NUM;
    
    for (unsigned int i = 0; i < uiPosByte; ++i)
    {
        if (0xFF != m_pFileBitMap[i])
        {
            return false;
        }
    }

    if (0 == uiPosByteIn)
    {
        return true;
    }

    unsigned int uiBegin = uiPosByte * BYTE_BIT_NUM;
    unsigned int uiEnd = uiPosByte * BYTE_BIT_NUM + uiPosByteIn - 1;

    for (unsigned int k = uiBegin; k <= uiEnd; ++k)
    {
        unsigned int uiBlkStatus = BLOCK_EMPTY;
        if (!GetBlkStatusInner(k, uiBlkStatus))
        {
            return false;
        }
        if (BLOCK_EMPTY == uiBlkStatus)
        {
            return false;
        }
    }

    return true;
}

void FileBitMapHandlerForClient::SetBlkSize(const unsigned int uiBlkSize)
{
    m_uiBlkSize = uiBlkSize;
}

unsigned int FileBitMapHandlerForClient::GetBlkSize()
{
    unsigned int uiRet = m_uiBlkSize;
    return uiRet;
}
