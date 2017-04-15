#ifndef _HTTP_CLIENT_
#define _HTTP_CLIENT_

#include <string>
#include <map>
#include <list>

#include "curl/curl.h"


/*
* ʹ��libcurl����HTTP��HTTPS����֧��GET��POST��ʽ�������ݣ���֧�ֶ��̵߳��÷�������
*/
class HttpClient
{
public:
    HttpClient();
    ~HttpClient();

    int Get(const std::string &url, std::string &response);
    int Post(const std::string &url, const std::string &request, std::string &response);
    int PostForm(const std::string &url, const std::map<std::string, std::string> &reqFormMap, std::string &response);

    int HttpsGet(const std::string &url, const std::list<std::string> &strHeaderList, std::string &response);
    int HttpsPost(const std::string &url, const std::string &request, std::string &response);
    int HttpsPostForm(const std::string &url, const std::map<std::string, std::string> &reqFormMap, std::string &response);

};

#endif