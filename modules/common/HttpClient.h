#ifndef _HTTP_CLIENT_
#define _HTTP_CLIENT_

#include <string>
#include <map>

#include "curl/curl.h"


/*
* ʹ��libcurl����HTTP����֧��HTTP GET��POST��ʽ�������ݣ���֧�ֶ��̵߳��÷�������
*/
class HttpClient
{
public:
    HttpClient();
    ~HttpClient();

    int Get(const std::string &url, std::string &response);
    int Post(const std::string &url, const std::string &request, std::string &response);
    int PostForm(const std::string &url, const std::map<std::string, std::string> &reqFormMap, std::string &response);

};

#endif