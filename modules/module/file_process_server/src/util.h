#ifndef UTIL_H
#define UTIL_H
#include <string>

#include "fcgiapp.h"


using namespace std;

#define TRIM(a) \
{ \
    int len=0; \
    while ((a)[len] && isspace((a)[len])) len++; \
    memmove((a),&(a)[len], strlen(&(a)[len])+1); \
    while ((len=strlen(a))>0 && isspace((a)[len-1])) (a)[len-1]='\0'; \
}

bool is_clean_char(char ch);
void clean_line(char *line);

int URLDecode(const char* str, const int strSize, char* result, const int resultSize);

char *trim( char *str );
bool IsNum(const char *str);
int HttpErrResponse(FCGX_Request *request, int http_rc, int rc, const char *msg);
int getURIRequestData(const char *buff, const char *key, string &value);
bool GetKeyValue(const char *buff, const char *key, string &value);
unsigned long GetFilesize(const char *path);


#endif
