#include "FileBitMapHandler.h"

#define BYTE_BIT_NUM (8*sizeof(char))
const boost::uint32_t FileBitMapHandler::BLOCK_FULL = 1;
const boost::uint32_t FileBitMapHandler::BLOCK_EMPTY = 0;

FileBitMapHandler::FileBitMapHandler(const std::string &strFileGUID, const boost::uint32_t uiFileSize, const boost::uint32_t uiBlkSize, 
    const boost::uint32_t uiBlkNum, const char *pSaveFilePath) : m_strFileGUID(strFileGUID), m_uiBlkSize(uiBlkSize), 
    m_fpSaved(NULL)
{
    m_pFileBitMap = GenerateEmptyFileBitMap(uiFileSize, uiBlkSize, uiBlkNum, m_uiFileBitMapSize);
    m_uiFileBitMapBitSize = uiBlkNum;

    if (NULL != pSaveFilePath)
    {
        m_strFileSavePath = pSaveFilePath;
    }

}

FileBitMapHandler::FileBitMapHandler(const std::string &strFileGUID, const unsigned char *pFileBitMapBuffer, 
    const boost::uint32_t uiLen, const boost::uint32_t uiFileBitMapBitSize, const char *pSaveFilePath) : m_strFileGUID(strFileGUID), 
    m_uiFileBitMapSize(uiLen), m_uiFileBitMapBitSize(uiFileBitMapBitSize), m_uiBlkSize(0), m_fpSaved(NULL)
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

FileBitMapHandler::FileBitMapHandler(const char *pLoadFilePath, const char *pSaveFilePath) : m_pFileBitMap(NULL), 
    m_uiFileBitMapSize(0), m_uiFileBitMapBitSize(0), m_uiBlkSize(0), m_fpSaved(NULL)
{
    if (NULL != pSaveFilePath)
    {
        m_strFileSavePath = pSaveFilePath;
    }

    FILE *fp = NULL;
#ifdef _WIN32
    if ((fopen_s(&fp, pLoadFilePath,"rb"))!=0 || NULL == fp)
    {
        return;
    }
#else
    fp = fopen(pLoadFilePath, "rb");
    if (NULL == fp)
    {
        return;
    }
#endif

    fread(&m_uiBlkSize, sizeof(m_uiBlkSize), 1, fp);

    boost::uint32_t uiFileGUIDSize = 0;    
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


FileBitMapHandler::~FileBitMapHandler(void)
{
     //boost::unique_lock<boost::mutex> lock(m_MutexForBitMap);

    delete[] m_pFileBitMap;
    m_pFileBitMap = NULL;

    if (NULL != m_fpSaved)
    {
        fclose(m_fpSaved);
        m_fpSaved = NULL;
    }

}

unsigned char * FileBitMapHandler::GenerateEmptyFileBitMap(const boost::uint32_t uiFileSize, const boost::uint32_t uiBlkSize, 
    const boost::uint32_t uiBlkNum, boost::uint32_t &uiFileBitMapSize)
{

    if (uiFileSize > (uiBlkNum *uiBlkSize)) //Òì³£Çé¿ö
    {
        return NULL;
    }

    boost::uint32_t uiBitMapSize = uiBlkNum / BYTE_BIT_NUM;
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

bool FileBitMapHandler::SetBlkStatus(const boost::uint32_t uiBlkID, const boost::uint32_t uiBlkStatus)
{
    //boost::unique_lock<boost::mutex> lock(m_MutexForBitMap);

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

    boost::uint32_t uiPosByte = (uiBlkID)/BYTE_BIT_NUM;
    boost::uint32_t uiPosByteIn = (uiBlkID)%BYTE_BIT_NUM;
    
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

const unsigned char * FileBitMapHandler::GetFileBitMapBuffer(boost::uint32_t &uiLen)
{
    //boost::unique_lock<boost::mutex> lock(m_MutexForBitMap);

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

bool FileBitMapHandler::GetBlkStatus(const boost::uint32_t uiBlkID, boost::uint32_t &uiBlkStatus)
{
    //boost::unique_lock<boost::mutex> lock(m_MutexForBitMap);
    return GetBlkStatusInner(uiBlkID, uiBlkStatus);
}

bool FileBitMapHandler::GetBlkStatusInner(const boost::uint32_t uiBlkID, boost::uint32_t &uiBlkStatus)
{    
    //boost::unique_lock<boost::mutex> lock(m_MutexForBitMap);

    if (NULL == m_pFileBitMap)
    {
        return false;
    }

    if (uiBlkID > (m_uiFileBitMapSize * BYTE_BIT_NUM - 1) || uiBlkID >= m_uiFileBitMapBitSize)
    {
        return false;
    }

    boost::uint32_t uiPosByte = (uiBlkID)/BYTE_BIT_NUM;
    boost::uint32_t uiPosByteIn = (uiBlkID)%BYTE_BIT_NUM;

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

bool FileBitMapHandler::SaveFileBitMap(const char * pFileNameSaved)
{
    //boost::unique_lock<boost::mutex> lock(m_MutexForBitMap);

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
    if (NULL == m_fpSaved)
    {
#ifdef _WIN32
        if ((fopen_s(&fp, pFileSaveUsed, "wb")) != 0 || NULL == fp)
        {
            return false;
        }
#else
        fp = fopen(pFileSaveUsed, "wb");
        if (NULL == fp)
        {
            return false;
        }
#endif
        m_fpSaved = fp;
    }
    else
    {
        fp = m_fpSaved;

        if (0 != fseek(m_fpSaved, 0, SEEK_SET))
        {
            fclose(fp);
            return false;
        }
    }

    const boost::uint32_t uiFileGUIDSize = m_strFileGUID.size();
    fwrite(&m_uiBlkSize, sizeof(m_uiBlkSize), 1, fp);
    fwrite(&uiFileGUIDSize, sizeof(uiFileGUIDSize), 1, fp);
    fwrite(m_strFileGUID.data(), uiFileGUIDSize, 1, fp);

    fwrite(&m_uiFileBitMapBitSize, sizeof(m_uiFileBitMapBitSize), 1, fp);
    fwrite(&m_uiFileBitMapSize, sizeof(m_uiFileBitMapSize), 1, fp);
    fwrite(m_pFileBitMap, m_uiFileBitMapSize, 1, fp);
    fflush(fp);
    //fclose(fp);

    return true;
}

bool FileBitMapHandler::AllFileBlkIsFull()
{
    //boost::unique_lock<boost::mutex> lock(m_MutexForBitMap);

    if ((NULL == m_pFileBitMap) || (0 == m_uiFileBitMapSize) || (0 == m_uiFileBitMapBitSize))
    {
        return false;
    }

    boost::uint32_t uiPosByte = (m_uiFileBitMapBitSize)/BYTE_BIT_NUM;
    boost::uint32_t uiPosByteIn = (m_uiFileBitMapBitSize)%BYTE_BIT_NUM;
    
    for (boost::uint32_t i = 0; i < uiPosByte; ++i)
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

    boost::uint32_t uiBegin = uiPosByte * BYTE_BIT_NUM;
    boost::uint32_t uiEnd = uiPosByte * BYTE_BIT_NUM + uiPosByteIn - 1;

    for (boost::uint32_t k = uiBegin; k <= uiEnd; ++k)
    {
        boost::uint32_t uiBlkStatus = BLOCK_EMPTY;
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

void FileBitMapHandler::SetBlkSize(const boost::uint32_t uiBlkSize)
{
    //boost::unique_lock<boost::mutex> lock(m_MutexForBitMap);
    m_uiBlkSize = uiBlkSize;
}

boost::uint32_t FileBitMapHandler::GetBlkSize()
{
    //boost::unique_lock<boost::mutex> lock(m_MutexForBitMap);
    return m_uiBlkSize;
}

void FileBitMapHandler::Close()
{
    //boost::unique_lock<boost::mutex> lock(m_MutexForBitMap);

    if (NULL != m_fpSaved)
    {
        fclose(m_fpSaved);
        m_fpSaved = NULL;
    }
}
