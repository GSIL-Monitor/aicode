#ifndef __DUMP_HEX_H__
#define __DUMP_HEX_H__

#include <string>
#include <stdio.h>
#ifdef WIN32
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif
std::string DumpHex2String(const void* buf, unsigned len);


#endif
