#include "FileRWHandlerForClient.h"
#include <string.h>


FileRWHandlerForClient::FileRWHandlerForClient(const char *pFilePath, const unsigned int uiFileSize, const unsigned int uiBlkSize, 
    const unsigned int uiBlkNum) : m_strFilePath(pFilePath), m_uiFileSize(uiFileSize), 
                                                         m_uiBlkSize(uiBlkSize), m_uiBlkNum(uiBlkNum), m_fp(NULL)
{
}



FileRWHandlerForClient::~FileRWHandlerForClient(void)
{
    Close();
}

bool FileRWHandlerForClient::ReadBlk(const unsigned int uiBlkID, char *pBlkBuffer, const unsigned int uiBufferSize, unsigned int &uiSize)
{

    if (NULL == m_fp)
    {
        return false;
    }

    if ( 0 != fseek(m_fp, (uiBlkID * m_uiBlkSize), SEEK_SET))
    {
        return false;
    }

    uiSize = fread(pBlkBuffer, 1, uiBufferSize, m_fp);

    return true;
}

bool FileRWHandlerForClient::WriteBlk(const unsigned int uiBlkID, const char *pBlkBuffer, const unsigned int uiBufferSize)
{
    if (NULL == m_fp)
    {
        return false;
    }

    if ( 0 != fseek(m_fp, (uiBlkID * m_uiBlkSize), SEEK_SET))
    {
        return false;
    }

    fwrite(pBlkBuffer, uiBufferSize, 1, m_fp);
    fflush(m_fp);

    return true;
}

bool FileRWHandlerForClient::Init()
{

    m_fp = fopen(m_strFilePath.c_str(), "r");
    if (NULL == m_fp)
    {
        //file not exist
        m_fp = fopen(m_strFilePath.c_str(), "w+b");
        if (NULL == m_fp)
        {
            return false;
        }
    }
    else
    {
        //file exist
        fclose(m_fp);
        m_fp = fopen(m_strFilePath.c_str(), "r+b");
        if (NULL == m_fp)
        {
            return false;
        }
    }

    return true;
}

void FileRWHandlerForClient::Close()
{
    if (NULL != m_fp)
    {
        fclose(m_fp);
        m_fp = NULL;
    }
}

const std::string &FileRWHandlerForClient::GetFilePath()
{
    return m_strFilePath;
}
