#include "FileRWHandler.h"
//#include <boost/filesystem/operations.hpp>
//#include <boost/filesystem/path.hpp>
//#include <boost/filesystem/convenience.hpp>

FileRWHandler::FileRWHandler(const char *pFilePath, const boost::uint32_t uiFileSize, const boost::uint32_t uiBlkSize, 
    const boost::uint32_t uiBlkNum) : m_strFilePath(pFilePath), m_uiFileSize(uiFileSize), 
                                                         m_uiBlkSize(uiBlkSize), m_uiBlkNum(uiBlkNum), m_fp(NULL)
{
}



FileRWHandler::~FileRWHandler(void)
{
    Close();
}

bool FileRWHandler::ReadBlk(const boost::uint32_t uiBlkID, char *pBlkBuffer, const boost::uint32_t uiBufferSize, boost::uint32_t &uiSize)
{
    boost::unique_lock<boost::mutex> lock(m_Mutex);

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

bool FileRWHandler::WriteBlk(const boost::uint32_t uiBlkID, const char *pBlkBuffer, const boost::uint32_t uiBufferSize)
{
    //boost::unique_lock<boost::mutex> lock(m_Mutex);

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

bool FileRWHandler::Init()
{
    boost::unique_lock<boost::mutex> lock(m_Mutex);
        
#ifdef _WIN32
    if ((fopen_s(&m_fp, m_strFilePath.c_str(),"r"))!=0 || NULL == m_fp)
    {
        //file not exist
        if ((fopen_s(&m_fp, m_strFilePath.c_str(),"w+b"))!=0 || NULL == m_fp)
        {
            return false; //open failed
        }
    }
    else
    {
        //file exist
        fclose(m_fp);
        if ((fopen_s(&m_fp, m_strFilePath.c_str(),"r+b"))!=0 || NULL == m_fp)
        {
            return false; //open failed
        }

    }
#else
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

#endif
    return true;
}

void FileRWHandler::Close()
{
    boost::unique_lock<boost::mutex> lock(m_Mutex);

    if (NULL != m_fp)
    {
        fclose(m_fp);
        m_fp = NULL;
    }
}

const std::string &FileRWHandler::GetFilePath()
{
    return m_strFilePath;
}
