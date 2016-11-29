#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "MD5.h"
#include "md5_helper.h"

//#define _UNIT_TEST_

std::string Md5Helper::Md5buf(const void* buf, int len)
{
	unsigned char md[16] = {0};
	unsigned char md5sum[64] = {0};
    unsigned char* p = (unsigned char*)buf;
	MD5_CTX ctx;
	MD5_Init(&ctx);
    for(int i=0; i<len; i+=MD5_STEP_SIZE)
    {
    	MD5_Update(&ctx, p+i, i+MD5_STEP_SIZE > len ? len-i : MD5_STEP_SIZE);
    }
	MD5_Finish(md, &ctx);

	BCD2ASCII(md, sizeof(md), md5sum);

	return (char*)md5sum;

}

std::string Md5Helper::Md5file(const char* filename)
{
    FILE *fp = fopen(filename,"r");
    if(!fp) return "";
    /*
	int fd = open(filename, O_RDONLY);
	if(fd < 0)
	{
		printf("open file: %s faile. %s\n", filename, strerror(errno));
		return "";
	}*/

	unsigned char buf[MD5_STEP_SIZE] = {0};
	int file_len = 0;
	unsigned char md[16] = {0};
	unsigned char md5sum[64] = {0};

	MD5_CTX ctx;
	MD5_Init(&ctx);
	
	while(1)
	{
		int len = 0;
		//len = read(fd, buf, sizeof(buf));
        len = fread(buf,1,sizeof(buf), fp);
		file_len += len;
		MD5_Update(&ctx, buf, len);
		if(len < (int)sizeof(buf))
			break;
	}
	MD5_Finish(md, &ctx);

	BCD2ASCII(md, sizeof(md), md5sum);
    fclose(fp);

	return (char*)md5sum;
}

#ifdef _UNIT_TEST_
int main(int argc, char* argv[])
{

	if(argc < 2)
	{
		printf("usage:\t%s <filename>\n", argv[0]);
		return -1;
	}
	unsigned char fbuf[65536] = {0};
	int len = 0;
	
	int fd = open(argv[1], O_RDONLY);
	if(fd < 0)
	{
		printf("open file: %s faile. %s\n", argv[1], strerror(errno));
		return -1;
	}
	len = read(fd, fbuf, sizeof(fbuf));
	
	std::string bufmd5 = Md5Helper::Md5buf(fbuf, len);

	std::string strmd5 = Md5Helper::Md5file(argv[1]);
	printf("file: %s's md5sum: %s md5bufsum: %s\n", argv[1], strmd5.c_str(), bufmd5.c_str());
	return 0;
}
#endif
