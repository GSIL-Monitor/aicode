#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <string>
#include "util.h"

using namespace std;

bool is_clean_char(char ch)
{
    if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
        return true;

    return false;
}

void clean_line(char *line)
{
    char *pStart = line;
    while (*pStart != '\0' && is_clean_char(*pStart)) pStart++;

    char *pEnd = pStart + strlen(pStart);
    while ( (pEnd -1) >= pStart && is_clean_char(*(pEnd-1))) *--pEnd = '\0';

    if (pStart != line){
        int len = strlen(pStart);
        memmove(line, pStart, len);
        line[len] = 0x00;
    }
}

#define NON_NUM '0'
char Char2Num(char ch)
{
    if(ch>='0' && ch<='9')return (char)(ch-'0');
    if(ch>='a' && ch<='f')return (char)(ch-'a'+10);
    if(ch>='A' && ch<='F')return (char)(ch-'A'+10);
       
    return NON_NUM;   
}   

/************************************************  
 * 把字符串进行URL编码。  
 * 输入：  
 * str: 要编码的字符串  
 * strSize: 字符串的长度。这样str中可以是二进制数据  
 * result: 结果缓冲区的地址  
 * resultSize:结果地址的缓冲区大小(如果str所有字符都编码，该值为strSize*3)  
 * 返回值：  
 * >0: result中实际有效的字符长度，  
 * 0: 编码失败，原因是结果缓冲区result的长度太小  
 ************************************************/   
int URLEncode(const char* str, const int strSize, char* result, const int resultSize)
{   
    int i;   
    int j = 0; /* for result index */   
    char ch;   

    if ((str == NULL) || (result == NULL) || (strSize <= 0) || (resultSize <= 0)) {   
        return 0;   
    }   

    for (i=0; (i<strSize) && (j<resultSize); i++) {   
        ch = str[i];   
        if ((ch >= 'A') && (ch <= 'Z')) {   
            result[j++] = ch;   
        } else if ((ch >= 'a') && (ch <= 'z')) {   
            result[j++] = ch;   
        } else if ((ch >= '0') && (ch <= '9')) {   
            result[j++] = ch;   
        } else if(ch == ' '){   
            result[j++] = '+';   
        } else {   
            if (j + 3 < resultSize) {   
                sprintf(result+j, "%%%02X", (unsigned char)ch);   
                j += 3;   
            } else {   
                return 0;   
            }   
        }   
    }   

    result[j] = 0x00;   
    return j;   
}   


/************************************************  
 * 把字符串进行URL解码。  
 * 输入：  
 * str: 要解码的字符串  
 * strSize: 字符串的长度。  
 * result: 结果缓冲区的地址  
 * resultSize:结果地址的缓冲区大小，可以<=strSize  
 * 返回值：  
 * >0: result中实际有效的字符长度，  
 * 0: 解码失败，原因是结果缓冲区result的长度太小  
 ************************************************/   
int URLDecode(const char* str, const int strSize, char* result, const int resultSize) {   
    char ch, ch1, ch2;   
    int i;   
    int j = 0; /* for result index */   

    if ((str == NULL) || (result == NULL) || (strSize <= 0) || (resultSize <= 0)) {   
        return 0;   
    }   

    for (i=0; (i<strSize) && (j<resultSize); i++) {   
        ch = str[i];   
        switch (ch) {   
            case '+':   
                result[j++] = ' ';   
                break;   

            case '%':   
                if (i+2 < strSize) {   
                    ch1 = Char2Num(str[i+1]);   
                    ch2 = Char2Num(str[i+2]);   
                    if ((ch1 != NON_NUM) && (ch2 != NON_NUM)) {   
                        result[j++] = (char)((ch1<<4) | ch2);   

                        i += 2;   
                        break;   
                    }   
                }   

                /* goto default */   
            default:   
                result[j++] = ch;   
                break;   
        }   
    }   

    result[j] = 0x00;   
    return j;   
} 

int urlencode(char *code, char *dst)
{
	char c, c1, c2, c3;
	char hexval[5] = {0}; 
	int i, j;
	int length = strlen(code);
	
	
	hexval[0] = '0';
	hexval[1] = 'x';
	hexval[4] = '\0';
	
	i = 0;
	j = 0;
	while (i <= length - 3) {	// we lose 3 bytes but what the hell..
		c1 = code[i];
		c2 = code[i + 1];
		c3 = code[i + 2];
		
		if (c1 == '%' && 
		(((c2>='0') && (c2<='9')) || ((c2>='a') && (c2<='f')) || ((c2>='0') && (c2<= '9')) || ((c2>= 'A') && (c2<='Z'))) &&
		(((c3>='0') && (c3<='9')) || ((c3>='a') && (c3<='f')) || ((c3>='0') && (c3<= '9')) || ((c3>= 'A') && (c3<='Z'))))
		{
			hexval[2] = c2;
			hexval[3] = c3;
			c = (unsigned char) strtol(hexval, NULL, 0);
			i += 3;
		} else {
			c = c1;
			i++;
		}
		
		dst[j] = c;
		j++;
	}
	while (i < length){
	dst[j++] = code[i++];
	}
	
	return 0;
}

int getURIRequestData(const char *buff, const char *key, string &value) 
{
    const char *p = NULL;
    
    p = strcasestr(buff, key);
    if (p == NULL) return 1;

    p += strlen(key);
    if (*p != '='){
        return 2;
    }
    p++;
    
	const char *start = p; 
    for(; p < (buff + strlen(buff)); p++){
        if (*p == '&')
            break;
    }
    value.assign(start, p - start);

    return 0;
}

bool GetKeyValue(const char *buff, const char *key, string &value)
{
	const char *p = NULL;
	int flag = 0;
	int count = 0;
	
	//value = "";
	p = strstr(buff, key);
	if (p == NULL) return false;
	
	p += strlen(key);
	
	while(*p == ' ' || *p  == '\t') p++;
	if (*p == '\"'){
		p++;
		flag = 1;
	}
	
	const char *start = p;
	const char *end = buff+strlen(buff);
	for(; p < end; p++){
		if (flag == 1){
			if (*p == '\"')
				break;
		}
		else if (*p == '\t' || *p == ' ' || *p == '\r' || *p == '\n')
			break;
			
		count++;
	}
	
	value.assign(start, count);
	return true;
}


bool IsNum(const char* str)
{
	if( NULL == str ) return false;
	
    for( unsigned int i = 0; i < strlen(str); ++i )
    {
        if( ( str[i] < '0' ) || ( str[i] > '9' ) )
        {
            return false;
        }
    }

    return true;
}

char *trim( char *str )
{
        char *p = str;
        char *q = str;
 
        while (*q++){};

        q -= 2;

        while( p <= q && isspace(*q) )
        {
            q--;
        }
        while( p <= q && isspace(*p) )
        {
            p++;
        }
        while( p <= q )
        {
            *str++ = *p++;
        }
 
        *str = '\0';
 
        return str;
}

int HttpErrResponse(FCGX_Request *request, int http_rc, int rc, const char *msg)
{
	//FCGX_FPrintF(request->out, "HTTP/1.1 500 Not Found\r\n"
	FCGX_FPrintF(request->out, "Status: %d  Error\r\n"
		"Content-Type: text/html\r\n"
		"\r\n", http_rc);

	FCGX_FPrintF(request->out, "Result:%d<br>\r\n"
	"Message:%s\r\n", 
	rc, msg);

    return 0;
}

unsigned long GetFilesize(const char *path)
{
    unsigned long filesize = -1;
    struct stat statbuff;
    if (stat(path, &statbuff) < 0){
        return filesize;
    }
    else{
        filesize = statbuff.st_size;
    }
    return filesize;
}
