#ifndef __FILE_MGR__
#define __FILE_MGR__

#include "NetComm.h"
#include <map>
#include "boost/atomic.hpp"

class FileRWHandler;

typedef boost::function<bool(const char *, const unsigned int, const unsigned int, const unsigned int)> ReadFileCB;
typedef boost::function<bool(const char *, const unsigned int, const unsigned int, const unsigned int, const unsigned int)> ReadFileCBRange;

class FileManager
{
public:
    FileManager(const std::string &strPath, const unsigned int uiSubDirNum, const bool InitClean = true);
    ~FileManager();

    struct FileSTInfo
    {
        std::string m_strName;
        std::string m_strMd5;
        unsigned int m_uiSize;
        std::string m_strLocalPath;
    };

    void SetBlockSize(const unsigned int uiBlockSize);

    bool OpenFile(const std::string &strFileName, std::string &strFileID);

    bool OpenFile(const std::string &strFileID, unsigned int &uiFileSize, const std::string &strFileIDSeq = "", unsigned int uiBlockSize = 0);

    bool WriteBuffer(const std::string &strFileID, const char *pBuffer, const unsigned int uiBufferSize, const unsigned int uiBlockID);

    bool ReadBuffer(const std::string &strFileID, char *pBuffer, const unsigned int uiBufferSize, unsigned int &uiReadSize, const unsigned int uiBlockID);

    void CloseFile(const std::string &strFileID);

    bool ReadFile(const std::string &strFileID, ReadFileCB rfcb);

    bool ReadFileRange(const std::string &strFileID, ReadFileCBRange rfcb, unsigned int uiBegin, unsigned int uiEnd);

    bool DeleteFile(const std::string &strFileID);

    bool QueryFile(const std::string &strFileID, FileSTInfo &fileinfo);

    inline void SetID(const std::string &strID)
    {
        m_strID = strID;
    }

    inline const std::string &GetID()
    {
        return m_strID;
    };

private:

    bool GetStoragePath(std::string &strOutputPath, std::string &strFileID);

    bool AddFileHandler(const std::string &strFileID, const std::string &strFilePath, unsigned int uiBlockSize = 0);

    bool GetFileSize(const std::string &strStoragePath, unsigned int &uiFileSize);

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

    boost::atomic_uint64_t m_uiMsgSeq;
    
    std::string m_strID; //这里ID只会在初始化的时候设置一次，后面都是读取，所以这里不在加锁保护。
    
};

class FileMgrGroupEx
{
public:
    FileMgrGroupEx();
    ~FileMgrGroupEx();

    void AddFileMgr(const std::string &strGID, boost::shared_ptr<FileManager> pFileMgr);

    boost::shared_ptr<FileManager> GetFileMgr(const std::string &strGroupFileID); //根据定义好的文件id找到对应的对象。

    boost::shared_ptr<FileManager> GetFileMgr(); //根据内部选择规则来提供对应的对象。

    bool GroupFileID2FileID(const std::string &strGroupFileID, std::string &strFileID);

    bool FileID2GroupFileID(const std::string &strFileID, const std::string &strGID, std::string &strGroupFileID);


private:
    std::map<std::string, boost::shared_ptr<FileManager> > m_FileMgrMap;

};


#endif
