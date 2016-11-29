#ifndef _MD5_HELPER_H_
#define _MD5_HELPER_H_ 

#include <string>

class Md5Helper
{	
public:
    static std::string Md5buf(const void* buf, int len);
    static std::string Md5file(const char* filename);
private:
	static const int MD5_STEP_SIZE = 1024*16;
};
 
#endif
