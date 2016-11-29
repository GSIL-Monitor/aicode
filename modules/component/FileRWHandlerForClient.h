#ifndef FILERW_HANDLER_CLIENT
#define FILERW_HANDLER_CLIENT

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 创建日期：2014-11-25
// 作    者：尹宾
// 修改日期：
// 修 改 者：
// 修改说明：
// 类 摘 要：文件读写处理器
// 详细说明：
// 附加说明：
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <string>
#include <stdio.h>


class FileRWHandlerForClient
{
public:
    FileRWHandlerForClient(const char *pFilePath, const unsigned int uiFileSize, const unsigned int uiBlkSize, 
        const unsigned int uiBlkNum);
    ~FileRWHandlerForClient(void);

    bool Init();

    bool ReadBlk(const unsigned int uiBlkID, char *pBlkBuffer, const unsigned int uiBufferSize, unsigned int &uiSize);

    bool WriteBlk(const unsigned int uiBlkID, const char *pBlkBuffer, const unsigned int uiBufferSize);

    void Close();

    const std::string &GetFilePath();

private:
    FileRWHandlerForClient( const FileRWHandlerForClient& );
    FileRWHandlerForClient& operator=( const FileRWHandlerForClient& );

private:
    std::string m_strFilePath;
    unsigned int m_uiFileSize;
    unsigned int m_uiBlkSize;
    unsigned int m_uiBlkNum;
        
    FILE *m_fp;
};


#endif
