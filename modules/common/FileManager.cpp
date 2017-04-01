#include "FileManager.h"
#include "FileRWHandler.h"
#include "LogRLD.h"
#include "boost/filesystem.hpp"
#include "boost/lexical_cast.hpp"
#include "CommonUtility.h"

FileManager::FileManager(const std::string &strPath, const unsigned int uiSubDirNum, const bool InitClean) :
m_strPath(strPath), m_uiSubDirNum(uiSubDirNum), m_InitClean(InitClean), m_uiBlockSize(1)
{
}

FileManager::~FileManager()
{
}

void FileManager::SetBlockSize(const unsigned int uiBlockSize)
{
    m_uiBlockSize = uiBlockSize;
}

bool FileManager::OpenFile(const std::string &strFileName, std::string &strFileID)
{
    std::string strStoragePath;  
    if (!GetStoragePath(strStoragePath, strFileID))
    {
        LOG_ERROR_RLD("Get path of storage failed.");
        return false;
    }

    strFileID += '_';
    strFileID += strFileName;

    strStoragePath += '/';
    strStoragePath += strFileID;
    
    LOG_INFO_RLD("Get file path is " << strStoragePath << " and file id is " << strFileID);

    if (!AddFileHandler(strFileID, strStoragePath))
    {
        LOG_ERROR_RLD("Add file handler failed and file id is " << strFileID << " and path is " << strStoragePath);
        return false;
    }

    return true;

}

bool FileManager::OpenFile(const std::string &strFileID)
{
    if (!strFileID.empty() && std::string::npos == strFileID.find("/"))
    {
        LOG_ERROR_RLD("Open file failed because file is invalid: " << strFileID);
        return false;
    }

    if (strFileID.empty())
    {
        LOG_ERROR_RLD("File id is empty.");
        return false;
    }
    
    std::string strFileIDTmp = strFileID;
    std::string strStoragePath;
    if (!GetStoragePath(strStoragePath, strFileIDTmp))
    {
        LOG_ERROR_RLD("Get path of storage failed.");
        return false;
    }

    LOG_INFO_RLD("Get file path is " << strStoragePath);

    if (!AddFileHandler(strFileID, strStoragePath))
    {
        LOG_ERROR_RLD("Add file handler failed and file id is " << strFileID << " and path is " << strStoragePath);
        return false;
    }

    return true;

}

bool FileManager::WriteBuffer(const std::string &strFileID, const char *pBuffer, const unsigned int uiBufferSize, const unsigned int uiBlockID)
{
    FileHdr fh;
    {
        boost::unique_lock<boost::mutex> lock(m_FileMapMutex);
        auto itFind = m_FileMap.find(strFileID);
        if (m_FileMap.end() == itFind)
        {
            LOG_ERROR_RLD("Not found file handler and file id is " << strFileID);
            return false;
        }
        
        fh.m_fd = itFind->second.m_fd;
        fh.m_uiBlockCountInc = itFind->second.m_uiBlockCountInc;
        fh.m_FileMutex = itFind->second.m_FileMutex;
    }

    boost::unique_lock<boost::mutex> lock(*fh.m_FileMutex);
    if (!fh.m_fd->WriteBlk(uiBlockID, pBuffer, uiBufferSize))
    {
        LOG_ERROR_RLD("Block write failed and block id is " << uiBlockID);
        return false;
    }

    return true;

}

bool FileManager::ReadBuffer(const std::string &strFileID, char *pBuffer, const unsigned int uiBufferSize, unsigned int &uiReadSize, const unsigned int uiBlockID)
{
    FileHdr fh;
    {
        boost::unique_lock<boost::mutex> lock(m_FileMapMutex);
        auto itFind = m_FileMap.find(strFileID);
        if (m_FileMap.end() == itFind)
        {
            LOG_ERROR_RLD("Not found file handler and file id is " << strFileID);
            return false;
        }

        fh.m_fd = itFind->second.m_fd;
        fh.m_uiBlockCountInc = itFind->second.m_uiBlockCountInc;
        fh.m_FileMutex = itFind->second.m_FileMutex;
    }

    boost::unique_lock<boost::mutex> lock(*fh.m_FileMutex);
    
    if (!fh.m_fd->ReadBlk(uiBlockID, pBuffer, uiBufferSize, uiReadSize))
    {
        LOG_ERROR_RLD("File read buffer failed.");
        return false;
    }

    return true;

}

void FileManager::CloseFile(const std::string &strFileID)
{
    FileHdr fh;
    {
        boost::unique_lock<boost::mutex> lock(m_FileMapMutex);
        auto itFind = m_FileMap.find(strFileID);
        if (m_FileMap.end() == itFind)
        {
            LOG_ERROR_RLD("Not found file handler and file id is " << strFileID);
            return;
        }

        fh.m_fd = itFind->second.m_fd;
        fh.m_uiBlockCountInc = itFind->second.m_uiBlockCountInc;
        fh.m_FileMutex = itFind->second.m_FileMutex;
    }

    boost::unique_lock<boost::mutex> lock(*fh.m_FileMutex);
    fh.m_fd->Close();

    {
        boost::unique_lock<boost::mutex> lock(m_FileMapMutex);
        auto itFind = m_FileMap.find(strFileID);
        if (m_FileMap.end() == itFind)
        {
            LOG_ERROR_RLD("Not found file handler and file id is " << strFileID);
            return;
        }

        m_FileMap.erase(itFind);
    }
}


bool FileManager::GetStoragePath(std::string &strOutputPath, std::string &strFileID)
{
    boost::system::error_code e;
    boost::filesystem::path FilePath = m_strPath.empty() ? (boost::filesystem::current_path() / "FileStorage") : (boost::filesystem::path(m_strPath) / "FileStorage");

    if (m_InitClean)
    {
        boost::filesystem::remove_all(FilePath, e);
        LOG_INFO_RLD("Storage path was inited and path is " << FilePath.string() << " and result msg is " << e.message());
    }

    if (!boost::filesystem::exists(FilePath, e))
    {
        /*if (e)
        {
        LOG_ERROR_RLD("Find dir failed" << IDPath.string() << " error is " << e.message().c_str());
        return false;
        }*/

        boost::filesystem::create_directories(FilePath, e);

        if (e)
        {
            LOG_ERROR_RLD("Create dir failed" << FilePath.string() << " error is " << e.message().c_str());
            return false;
        }

        for (unsigned int i = 0; i < m_uiSubDirNum; ++i)
        {
            boost::filesystem::path idpath = FilePath / boost::lexical_cast<std::string>(i);
            boost::filesystem::create_directories(idpath, e);

            if (e)
            {
                LOG_ERROR_RLD("Create sub dir failed" << FilePath.string() << " error is " << e.message().c_str());
                return false;
            }

            LOG_INFO_RLD("Create sub dir " << idpath.string());
        }

        LOG_INFO_RLD("Dir init success.");
    }
    
    if (!strFileID.empty())
    {
        const std::string &strFilePathTmp = FilePath.string() + "/";
        
        strOutputPath = strFilePathTmp + strFileID;

        return true;
    }

    //随机分配存储目录，目前不限制单个目录下文件的数量。
    srand((unsigned)time(NULL));

    int iSubDirName = rand() % m_uiSubDirNum;

    boost::filesystem::path RandPath = FilePath / boost::lexical_cast<std::string>(iSubDirName);

    if (!boost::filesystem::exists(RandPath, e))
    {
        LOG_ERROR_RLD("Dir that was choised not exists and path is " << RandPath.string() << " error is " << e.message());
        return false;
    }

    strOutputPath = FilePath.string();

    strFileID = boost::lexical_cast<std::string>(iSubDirName) + "/";
    strFileID += CreateUUID();

    return true;
}

bool FileManager::AddFileHandler(const std::string &strFileID, const std::string &strFilePath)
{
    FileHdr fh;
    boost::unique_lock<boost::mutex> lock(m_FileMapMutex);
    auto itFind = m_FileMap.find(strFileID);
    if (m_FileMap.end() == itFind)
    {
        boost::shared_ptr<FileRWHandler> pFileHandler(new FileRWHandler(strFilePath.c_str(), 0, m_uiBlockSize, 0));

        if (!pFileHandler->Init())
        {
            LOG_ERROR_RLD("File handler inited failed.");
            return false;
        }

        fh.m_fd = pFileHandler;
        fh.m_uiBlockCountInc = 0;
        fh.m_FileMutex.reset(new boost::mutex());

        m_FileMap.insert(make_pair(strFileID, fh));
    }

    return true;
}

