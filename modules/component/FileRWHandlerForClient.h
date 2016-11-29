#ifndef FILERW_HANDLER_CLIENT
#define FILERW_HANDLER_CLIENT

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
