#include "ZipUtility.h"
#include "boost/filesystem.hpp"
#include "uuid.h"
#include <stdio.h>
#include "boost/lexical_cast.hpp"
#include <string>

std::string ConvertCharValueToLex(unsigned char *pInValue, const boost::uint32_t uiSize)
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

int main(int argc, char* argv[])
{
    uuid_t uu;
    uuid_generate(uu);
    const std::string &strUUID = ConvertCharValueToLex((unsigned char *)uu, sizeof(uu));
    
    boost::filesystem::path CurrentPath = boost::filesystem::current_path();

    boost::filesystem::path DirPath = CurrentPath / strUUID;

    boost::system::error_code e;
    bool blRet = boost::filesystem::create_directories(DirPath, e);
    if (e)
    {
        printf("Create dir error, %s\r\n", e.message().c_str());
        return 0;
    }
    
    char Buffer[1024 * 1024] = { 0 };
    for (int i = 0; i < 6; i++)
    {
        boost::filesystem::path FilePath = DirPath / boost::lexical_cast<std::string>(i);

        FILE *m_fp = fopen(FilePath.string().c_str(), "w+b");
        if (NULL == m_fp)
        {
            printf("Create file error.\r\n");
            return 0;
        }

        memset(Buffer, i, sizeof(Buffer));

        fwrite(Buffer, 1, sizeof(Buffer), m_fp);

        fclose(m_fp);
    }

    
    return 1;
}