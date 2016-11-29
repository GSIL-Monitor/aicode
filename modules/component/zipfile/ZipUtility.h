#ifndef _ZIP_UTILITY_
#define _ZIP_UTILITY_

#include <string>
#include "boost/thread.hpp"

class ZipUtility
{
public:
    ZipUtility();
    ~ZipUtility();

    /************************************************************************/
    /*strFileDir����ѹ����Ŀ¼���ں��������һ���ļ�
     *strFileZip��ѹ�����ɵ�zip�ļ�������ȫ·����
     *uiCompressLevel��ѹ���ʣ�0~9��9���0��ѹ��
     **********************************************************************/
    static unsigned int CompressFileFromDir(const std::string &strFileDir, std::string &strFileZip, const unsigned int uiCompressLevel = 0);

    /************************************************************************/
    /*�������Zip�ļ���С������CompressFileFromDirǰ���ã�
     *���Ҳ�֧���̰߳�ȫ�� Ĭ��ֵ��1GB��С��                                                                      
    ************************************************************************/
    static void SetMaxZipFileSize(const unsigned int uiSize);

    static const unsigned int ZIP_COMPRESS_SUCCESS = 0;
    static const unsigned int ZIP_INNER_FAILED = 1;
    static const unsigned int ZIP_FILESIZE_LIMITED_FAILED = 2;

    
private:
    static unsigned int CompressFileToZip(void *pZipFile, const std::string &strSrcFile, const std::string &strDstZipFile, const unsigned int uiCompressLevel = 0);

private:
    static boost::thread_specific_ptr<unsigned int> sm_uiMaxZipFileSize;
    static boost::thread_specific_ptr<unsigned int> sm_uiFileSize;    
};

#endif