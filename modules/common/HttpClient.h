#ifndef _HTTP_CLIENT_
#define _HTTP_CLIENT_

#include <string>
#include <map>

#include "curl/curl.h"


/*
* 使用libcurl发送HTTP请求，支持HTTP GET和POST方式发送数据，不支持多线程调用发送请求
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