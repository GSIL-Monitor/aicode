#ifndef FILEBITMAPHANDLER_CLIENT
#define FILEBITMAPHANDLER_CLIENT

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 创建日期：2014-11-17
// 作    者：尹宾
// 修改日期：
// 修 改 者：
// 修改说明：
// 类 摘 要：文件位图处理器
// 详细说明：
// 附加说明：
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <string>
#include <stdio.h>

class FileBitMapHandlerForClient
{
public:
    FileBitMapHandlerForClient(const std::string &strFileGUID, const unsigned int uiFileSize, const unsigned int uiBlkSize, 
        const unsigned int uiBlkNum, const char *pSaveFilePath = NULL);
    
    FileBitMapHandlerForClient(const std::string &strFileGUID, const unsigned char *pFileBitMapBuffer, const unsigned int uiLen,
        const unsigned int uiFileBitMapBitSize, const char *pSaveFilePath = NULL);

    FileBitMapHandlerForClient(const char *pLoadFilePath, const char *pSaveFilePath = NULL);
    
    ~FileBitMapHandlerForClient(void);

    bool SetBlkStatus(const unsigned int uiBlkID, const unsigned int uiBlkStatus);

    bool GetBlkStatus(const unsigned int uiBlkID, unsigned int &uiBlkStatus);

    const unsigned char * GetFileBitMapBuffer(unsigned int &uiLen);

    bool SaveFileBitMap(const char * pFileNameSaved = NULL);

    bool AllFileBlkIsFull();

    void SetBlkSize(const unsigned int uiBlkSize);

    unsigned int GetBlkSize();

private:
    unsigned char * GenerateEmptyFileBitMap(const unsigned int uiFileSize, const unsigned int uiBlkSize, 
        const unsigned int uiBlkNum, unsigned int &uiFileBitMapSize); //根据参数生成位图内容

    bool GetBlkStatusInner(const unsigned int uiBlkID, unsigned int &uiBlkStatus);

    FileBitMapHandlerForClient( const FileBitMapHandlerForClient& );
    FileBitMapHandlerForClient& operator=( const FileBitMapHandlerForClient& );

public:
    const static unsigned int BLOCK_FULL;
    const static unsigned int BLOCK_EMPTY;

private:
    std::string m_strFileGUID;

    unsigned char * m_pFileBitMap;
    unsigned int m_uiFileBitMapSize;
    unsigned int m_uiFileBitMapBitSize;

    std::string m_strFileSavePath;

    unsigned int m_uiBlkSize;

};


#endif
