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
    /*strFileDir：待压缩的目录，内含多个或者一个文件
     *strFileZip：压缩生成的zip文件（包含全路径）
     *uiCompressLevel：压缩率，0~9，9最大，0不压缩
     **********************************************************************/
    static unsigned int CompressFileFromDir(const std::string &strFileDir, std::string &strFileZip, const unsigned int uiCompressLevel = 0);

    /************************************************************************/
    /*设置最大Zip文件大小，调用CompressFileFromDir前设置，
     *并且不支持线程安全， 默认值是1GB大小。                                                                      
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