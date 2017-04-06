#include "FileManager.h"
#include "FileRWHandler.h"
#include "LogRLD.h"
#include "boost/filesystem.hpp"
#include "boost/lexical_cast.hpp"
#include "CommonUtility.h"

FileManager::FileManager(const std::string &strPath, const unsigned int uiSubDirNum, const bool InitClean) :
m_uiSubDirNum(uiSubDirNum), m_InitClean(InitClean), m_uiBlockSize(0), m_uiMsgSeq(0)
{
    boost::system::error_code e;
    boost::filesystem::path FilePath = strPath.empty() ? (boost::filesystem::current_path() / "FileStorage") : (boost::filesystem::path(strPath) / "FileStorage");

    if (m_InitClean)
    {
        boost::filesystem::remove_all(FilePath, e);
        LOG_INFO_RLD("Storage path was inited and path is " << FilePath.string() << " and result msg is " << e.message());
    }
    
    m_strPath = FilePath.string();
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

bool FileManager::OpenFile(const std::string &strFileID, unsigned int &uiFileSize, const std::string &strFileIDSeq)
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

    {
        boost::filesystem::path FilePath(strStoragePath);

        boost::system::error_code ec;
        if (!boost::filesystem::exists(FilePath, ec))
        {
            LOG_ERROR_RLD("File not exist and path is " << strStoragePath);
            return false;
        }

        const unsigned int uiSize = (unsigned int)boost::filesystem::file_size(FilePath, ec);
        if (0 == uiSize)
        {
            LOG_ERROR_RLD("File size is zero.");
            return false;
        }

        uiFileSize = uiSize;
    }

    LOG_INFO_RLD("Get file path is " << strStoragePath << " and file id is " << strFileID << " and file size is " << uiFileSize
        << " and file id of seq is " << strFileIDSeq);

    if (!AddFileHandler((strFileIDSeq.empty() ? strFileID : strFileIDSeq), strStoragePath))
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


bool FileManager::ReadFile(const std::string &strFileID, ReadFileCB rfcb)
{
    const std::string &strFileIDSeq = boost::lexical_cast<std::string>(m_uiMsgSeq++) + "_"  + strFileID;
    
    unsigned int uiFileSize = 0;
    if (!OpenFile(strFileID, uiFileSize, strFileIDSeq))
    {
        LOG_ERROR_RLD("Open file failed and file id is " << strFileID);
        return false;
    }

    if (0 == m_uiBlockSize)
    {
        LOG_ERROR_RLD("File block size is zero");
        return false;
    }

    const unsigned int uiBlockSize = m_uiBlockSize;
    const unsigned int uiBlockCount = uiBlockSize >= uiFileSize ? 1 : uiFileSize / uiBlockSize + 1;

    unsigned int uiBlockStatus = 0; //0，第一个文件块，1，文件中间位置的文件块，2，文件最后文件块
    boost::shared_ptr<char> pReadBuffer(new char[uiBlockSize]);
    unsigned int uiReadSize = 0;
    for (unsigned int i = 0; i < uiBlockCount; ++i)
    {
        if (!ReadBuffer(strFileIDSeq, pReadBuffer.get(), uiBlockSize, uiReadSize, i))
        {
            LOG_ERROR_RLD("Read file buffer failed and file id is " << strFileID << " and current block id is " << i);
            return false;
        }

        if (!rfcb(pReadBuffer.get(), uiReadSize, uiBlockStatus, uiFileSize))
        {
            LOG_ERROR_RLD("Read file buffer callback error and file id is " << strFileID << " and current block id is " << i);
            return false;
        }

        switch (uiBlockStatus)
        {
        case 0:
            if (i + 1 < uiBlockCount)
            {
                uiBlockStatus = 1;
            }
            else
            {
                uiBlockStatus = 2;
            }
        	break;

        case 1:
            if (i + 1 >= uiBlockCount)
            {
                uiBlockStatus = 2;
            }
            break;

        case 2:  //已经是最后一个文件块，不用处理状态了
            break;

        default:
            LOG_ERROR_RLD("Unknown block status: " << uiBlockStatus);
            return false;
        }
    }

    CloseFile(strFileIDSeq);
    
    return true;
}

bool FileManager::DeleteFile(const std::string &strFileID)
{


    return true;
}

bool FileManager::GetStoragePath(std::string &strOutputPath, std::string &strFileID)
{
    boost::system::error_code e;
    boost::filesystem::path FilePath(m_strPath);

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

