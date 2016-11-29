#ifndef FILEBITMAPHANDLER
#define FILEBITMAPHANDLER

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

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <string>
#include <stdio.h>

class FileBitMapHandler : public boost::noncopyable
{
public:
    FileBitMapHandler(const std::string &strFileGUID, const boost::uint32_t uiFileSize, const boost::uint32_t uiBlkSize, 
        const boost::uint32_t uiBlkNum, const char *pSaveFilePath = NULL);
    
    FileBitMapHandler(const std::string &strFileGUID, const unsigned char *pFileBitMapBuffer, const boost::uint32_t uiLen,
        const boost::uint32_t uiFileBitMapBitSize, const char *pSaveFilePath = NULL);

    FileBitMapHandler(const char *pLoadFilePath, const char *pSaveFilePath = NULL);
    
    ~FileBitMapHandler(void);

    bool SetBlkStatus(const boost::uint32_t uiBlkID, const boost::uint32_t uiBlkStatus);

    bool GetBlkStatus(const boost::uint32_t uiBlkID, boost::uint32_t &uiBlkStatus);

    const unsigned char * GetFileBitMapBuffer(boost::uint32_t &uiLen);

    bool SaveFileBitMap(const char * pFileNameSaved = NULL);

    bool AllFileBlkIsFull();

    void SetBlkSize(const boost::uint32_t uiBlkSize);

    boost::uint32_t GetBlkSize();

    void Close();

private:
    unsigned char * GenerateEmptyFileBitMap(const boost::uint32_t uiFileSize, const boost::uint32_t uiBlkSize, 
        const boost::uint32_t uiBlkNum, boost::uint32_t &uiFileBitMapSize); //根据参数生成位图内容

    bool GetBlkStatusInner(const boost::uint32_t uiBlkID, boost::uint32_t &uiBlkStatus);

public:
    const static boost::uint32_t BLOCK_FULL;
    const static boost::uint32_t BLOCK_EMPTY;

private:
    std::string m_strFileGUID;

    unsigned char * m_pFileBitMap;
    boost::uint32_t m_uiFileBitMapSize;
    boost::uint32_t m_uiFileBitMapBitSize;

    //boost::mutex m_MutexForBitMap;

    std::string m_strFileSavePath;

    boost::uint32_t m_uiBlkSize;

    FILE *m_fpSaved;

};


#endif
