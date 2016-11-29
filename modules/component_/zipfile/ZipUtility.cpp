#include "ZipUtility.h"
#include "boost/filesystem.hpp"
#include "zip.h"
#include "boost/cstdint.hpp"
#include "uuid.h"
#include <stdio.h>
#include <list>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp> 
#include <boost/date_time/posix_time/posix_time.hpp>  
#include <boost/date_time/time.hpp>

static std::string Lex(unsigned char *pInValue, const boost::uint32_t uiSize)
{
    char cTmp[32] = { 0 };
    std::string strTmp;
    for (boost::uint32_t i = 0; i < uiSize; ++i)
    {
#ifdef _WIN32
        _snprintf(cTmp, sizeof(cTmp), "%02x", pInValue[i]);
#else
        snprintf(cTmp, sizeof(cTmp), "%02x", pInValue[i]);
#endif
        strTmp += cTmp;
        memset(cTmp, 0, sizeof(cTmp));
    }

    return strTmp;
}

boost::thread_specific_ptr<unsigned int> ZipUtility::sm_uiMaxZipFileSize;
boost::thread_specific_ptr<unsigned int> ZipUtility::sm_uiFileSize;

ZipUtility::ZipUtility()
{
}


ZipUtility::~ZipUtility()
{
}

unsigned int ZipUtility::CompressFileFromDir(const std::string &strFileDir, std::string &strFileZip, const unsigned int uiCompressLevel)
{
    unsigned int uiRetCode = ZIP_INNER_FAILED;

    boost::filesystem::path FileDirPath(strFileDir);

    boost::system::error_code ec;
    if (!boost::filesystem::exists(FileDirPath, ec))
    {
        return uiRetCode;
    }
    
    if (!boost::filesystem::is_directory(FileDirPath, ec))
    {
        return uiRetCode;
    }

    if (boost::filesystem::is_empty(FileDirPath, ec))
    {
        return uiRetCode;
    }

    std::list<std::string> SrcFileList;    
    boost::filesystem::recursive_directory_iterator itBegin(FileDirPath);
    boost::filesystem::recursive_directory_iterator itEnd;
    while (itBegin != itEnd)
    {
        if (boost::filesystem::is_regular_file(itBegin->status()))
        {
            printf("Handle file : %s\r\n", itBegin->path().string().c_str());

            SrcFileList.push_back(itBegin->path().string());
        }

        ++itBegin;
    }

    uuid_t uu;
    uuid_generate(uu);
    const std::string &strUUID = Lex((unsigned char *)uu, sizeof(uu));
    std::string strUUIDZIP = strUUID + ".zip";
    boost::filesystem::path ZipOutFile = FileDirPath / strUUIDZIP;

    zipFile zf = zipOpen64(ZipOutFile.string().c_str(), 0);

    if (zf == NULL)
    {
        printf("zipOpenNewFileInZip failed\r\n");
        return uiRetCode;
    }

    if (NULL == sm_uiFileSize.get())
    {
        sm_uiFileSize.reset(new unsigned int(0));
    }

    *sm_uiFileSize = 0; //reset current file size

    bool blRet = true;
    auto itBegin2 = SrcFileList.begin();
    auto itEnd2 = SrcFileList.end();
    while (itBegin2 != itEnd2)
    {
        printf("Compress file : %s\r\n", itBegin2->c_str());

        if (ZIP_COMPRESS_SUCCESS != (uiRetCode = CompressFileToZip(zf, *itBegin2, ZipOutFile.string().c_str(), uiCompressLevel)))
        {
            blRet = false;
            break;
        }

        ++itBegin2;
    }
    
    if (ZIP_OK != zipClose(zf, (const char*)NULL))
    {
        printf("zipClose failed\r\n");
        blRet = false;
        uiRetCode = ZIP_INNER_FAILED;
    }

    if (!blRet)
    {
        boost::filesystem::remove(ZipOutFile, ec);        
    }
    else
    {
        strFileZip = ZipOutFile.string();
    }

    return uiRetCode;
}

void ZipUtility::SetMaxZipFileSize(const unsigned int uiSize)
{
    if (0 == uiSize)
    {
        return;
    }
    
    if (NULL == sm_uiMaxZipFileSize.get())
    {
        sm_uiMaxZipFileSize.reset(new unsigned int(0));
    }

    *sm_uiMaxZipFileSize = uiSize;
}

unsigned int ZipUtility::CompressFileToZip(void *pZipFile, const std::string &strSrcFile, const std::string &strDstZipFile, const unsigned int uiCompressLevel)
{
    unsigned int uiRetCode = ZIP_INNER_FAILED;

    boost::filesystem::path SrcFile(strSrcFile);

    zip_fileinfo FileInfo;
    memset(&FileInfo, 0, sizeof(FileInfo));

    boost::gregorian::date Today = boost::gregorian::day_clock::local_day();
    boost::posix_time::ptime TodayTime = boost::posix_time::second_clock::local_time();
        
    FileInfo.tmz_date.tm_year = boost::lexical_cast<unsigned int>(Today.year());
    FileInfo.tmz_date.tm_mon = boost::lexical_cast<unsigned int>(Today.month().as_number()) - 1;
    FileInfo.tmz_date.tm_mday = boost::lexical_cast<unsigned int>(Today.day());
    FileInfo.tmz_date.tm_hour = boost::lexical_cast<unsigned int>(TodayTime.time_of_day().hours());
    FileInfo.tmz_date.tm_min = boost::lexical_cast<unsigned int>(TodayTime.time_of_day().minutes());
    FileInfo.tmz_date.tm_sec = boost::lexical_cast<unsigned int>(TodayTime.time_of_day().seconds());
    FileInfo.dosDate = 0;

    //printf("FileInfo.tmz_date.tm_mon is %u\r\n", boost::lexical_cast<unsigned int>(Today.month().as_number()));
    
    if (zipOpenNewFileInZip((zipFile)pZipFile, SrcFile.filename().string().c_str(), &FileInfo, NULL, 0, NULL, 0, NULL, 
        (uiCompressLevel != 0) ? Z_DEFLATED : 0, (9 > uiCompressLevel ) ? uiCompressLevel : 9) != ZIP_OK)
    {
        printf("zipOpenNewFileInZip failed\r\n");
        return uiRetCode;
    }

    boost::system::error_code ec;
    unsigned int uiSrcFileSize = boost::filesystem::file_size(SrcFile, ec);
    if (ec)
    {
        return uiRetCode;
    }

    if (NULL == sm_uiMaxZipFileSize.get())
    {
        sm_uiMaxZipFileSize.reset(new unsigned int(1024 * 1024 * 1024));
    }

    if (NULL == sm_uiFileSize.get())
    {
        sm_uiFileSize.reset(new unsigned int(0));
    }

    *sm_uiFileSize = *sm_uiFileSize + uiSrcFileSize;
    if (*sm_uiFileSize >= *sm_uiMaxZipFileSize)
    {
        printf("Max zip file size reached, %u, %u\r\n", *sm_uiFileSize, *sm_uiMaxZipFileSize);
        uiRetCode = ZIP_FILESIZE_LIMITED_FAILED;
        return uiRetCode;
    }

    const unsigned int ui10M = 1024 * 1024 * 10;
    unsigned int uiBufferSize = 0;
    if (ui10M > uiSrcFileSize)
    {
        uiBufferSize = uiSrcFileSize;
    }
    else
    {
        uiBufferSize = ui10M;
    }

    char *pBuffer = new char[uiBufferSize];
    
    bool blRet = true;
    unsigned int uiReadCount = 0;
    unsigned int uiTotalCount = 0;

    FILE *fp = fopen(strSrcFile.c_str(), "r");
    if (NULL == fp)
    {
        //file not exist
        return uiRetCode;
    }

    while (1)
    {
        uiReadCount = fread(pBuffer, 1, uiBufferSize, fp);
        uiTotalCount += uiReadCount;

        if (zipWriteInFileInZip(pZipFile, pBuffer, uiBufferSize) < 0)
        {
            printf("zipWriteInFileInZip failed\r\n");
            blRet = false;
            break;
        }

        if (uiTotalCount >= uiSrcFileSize)
        {
            break;
        }
    }

    fclose(fp);

    delete[] pBuffer;
    pBuffer = NULL;

    if (ZIP_OK != zipCloseFileInZip(pZipFile))
    {
        printf("zipCloseFileInZip failed\r\n");
        blRet = false;
    }

    if (blRet)
    {
        uiRetCode = ZIP_COMPRESS_SUCCESS;
    }
    
    return uiRetCode;
}


