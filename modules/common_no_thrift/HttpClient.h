#ifndef _HTTP_CLIENT_
#define _HTTP_CLIENT_

#include <string>
#include <map>
#include <list>

#include "curl/curl.h"


/*
* 使用libcurl发送HTTP、HTTPS请求，支持GET和POST方式发送数据，不支持多线程调用发送请求
*/
class HttpClient
{
public:
    HttpClient();
    ~HttpClient();

    static size_t WriteToString(void *ptr, size_t size, size_t nmemb, void *stream);

    int Get(const std::string &url, std::string &response);
    int Post(const std::string &url, const std::string &request, std::string &response);
    int PostForm(const std::string &url, const std::map<std::string, std::string> &reqFormMap, std::string &response);

    int HttpsGet(const std::string &url, const std::list<std::string> &strHeaderList, std::string &response);
    int HttpsPost(const std::string &url, const std::string &request, std::string &response);
    int HttpsPostForm(const std::string &url, const std::map<std::string, std::string> &reqFormMap, std::string &response);
    int HttpsPostJson(const std::string &url, const std::string auth, const std::string &request, std::string &response);

    static void lock_callback(int mode, int type, char *file, int line);

    static unsigned long thread_id(void);

    static void init_locks(void);

    static void kill_locks(void);

    static pthread_mutex_t *m_lockArray;


private:
    static int Init();

    static const int m_curlCode;
};

#endif