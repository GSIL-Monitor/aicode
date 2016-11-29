#ifndef FILERW_HANDLER
#define FILERW_HANDLER

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// �������ڣ�2014-11-25
// ��    �ߣ�����
// �޸����ڣ�
// �� �� �ߣ�
// �޸�˵����
// �� ժ Ҫ���ļ���д������
// ��ϸ˵����
// ����˵����
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <string>
#include <stdio.h>


class FileRWHandler : public boost::noncopyable
{
public:
    FileRWHandler(const char *pFilePath, const boost::uint32_t uiFileSize, const boost::uint32_t uiBlkSize, 
        const boost::uint32_t uiBlkNum);
    ~FileRWHandler(void);

    bool Init();

    bool ReadBlk(const boost::uint32_t uiBlkID, char *pBlkBuffer, const boost::uint32_t uiBufferSize, boost::uint32_t &uiSize);

    bool WriteBlk(const boost::uint32_t uiBlkID, const char *pBlkBuffer, const boost::uint32_t uiBufferSize);

    void Close();

    const std::string &GetFilePath();

    boost::uint32_t GetFileSize()
    {
        return m_uiFileSize;
    };

private:
    std::string m_strFilePath;
    boost::uint32_t m_uiFileSize;
    boost::uint32_t m_uiBlkSize;
    boost::uint32_t m_uiBlkNum;

    boost::mutex m_Mutex;
    FILE *m_fp;
};


#endif
