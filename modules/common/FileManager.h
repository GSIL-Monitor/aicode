#ifndef __FILE_MGR__
#define __FILE_MGR__

#include "NetComm.h"
#include <map>

class FileRWHandler;

typedef boost::function<bool(const char *, const unsigned int, const unsigned int, const unsigned int)> ReadFileCB;

class FileManager
{
public:
    FileManager(const std::string &strPath, const unsigned int uiSubDirNum, const bool InitClean = true);
    ~FileManager();

    void SetBlockSize(const unsigned int uiBlockSize);

    bool OpenFile(const std::string &strFileName, std::string &strFileID);

    bool OpenFile(const std::string &strFileID, unsigned int &uiFileSize);

    bool WriteBuffer(const std::string &strFileID, const char *pBuffer, const unsigned int uiBufferSize, const unsigned int uiBlockID);

    bool ReadBuffer(const std::string &strFileID, char *pBuffer, const unsigned int uiBufferSize, unsigned int &uiReadSize, const unsigned int uiBlockID);

    void CloseFile(const std::string &strFileID);

    bool ReadFile(const std::string &strFileID, ReadFileCB rfcb);

private:

    bool GetStoragePath(std::string &strOutputPath, std::string &strFileID);

    bool AddFileHandler(const std::string &strFileID, const std::string &strFilePath);

private:

    std::string m_strPath;
    unsigned int m_uiSubDirNum;
    bool m_InitClean;
    unsigned int m_uiBlockSize;
        
    struct FileHdr
    {
        boost::shared_ptr<FileRWHandler> m_fd;
        unsigned int m_uiBlockCountInc;
        boost::shared_ptr<boost::mutex> m_FileMutex;
    };

    boost::mutex m_FileMapMutex;
    std::map<std::string, FileHdr> m_FileMap;
    
};

#endif
