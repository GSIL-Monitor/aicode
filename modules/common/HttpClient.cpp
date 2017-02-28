#include "HttpClient.h"

HttpClient::HttpClient()
{
    curl_global_init(CURL_GLOBAL_ALL);
}


HttpClient::~HttpClient()
{
    curl_global_cleanup();
}

static size_t WriteToString(void *ptr, size_t size, size_t nmemb, void *stream)
{
    ((std::string *)stream)->append((char *)ptr, 0, size * nmemb);
    return size * nmemb;
}

int HttpClient::Get(const std::string &url, std::string &response)
{
    CURLcode ret;
    CURL* curl = curl_easy_init();
    if (NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
    /**
    * ������̶߳�ʹ�ó�ʱ�����ʱ��ͬʱ���߳�����sleep����wait�Ȳ�����
    * ������������ѡ�libcurl���ᷢ�źŴ�����wait�Ӷ����³����˳���
    */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    //curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);

    ret = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    return ret;
}

int HttpClient::Post(const std::string &url, const std::string &request, std::string &response)
{
    CURL *curl = curl_easy_init();
    if (curl == NULL)
    {
        return CURLE_FAILED_INIT;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request.length());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    //curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);

    int ret = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    return ret;
}

