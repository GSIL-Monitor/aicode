#include "HttpClient.h"
#include "openssl/crypto.h"
#include <pthread.h>

//保证libcurl在程序中只初始化一次
const int HttpClient::m_curlCode = Init();

pthread_mutex_t* HttpClient::m_lockArray = NULL;

HttpClient::HttpClient()
{

}

HttpClient::~HttpClient()
{
    //kill_locks();
    //curl_global_cleanup();
}

int HttpClient::Init()
{
    int ret = curl_global_init(CURL_GLOBAL_ALL);
    init_locks();

    return ret;
}

void HttpClient::lock_callback(int mode, int type, char *file, int line)
{
    (void)file;
    (void)line;
    if (mode & CRYPTO_LOCK)
    {
        pthread_mutex_lock(&(m_lockArray[type]));
    }
    else
    {
        pthread_mutex_unlock(&(m_lockArray[type]));
    }
}

unsigned long HttpClient::thread_id(void)
{
    unsigned long ret;

    ret = (unsigned long)pthread_self();
    return ret;
}

void HttpClient::init_locks(void)
{
    int i;

    m_lockArray = (pthread_mutex_t *)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
    for (i = 0; i < CRYPTO_num_locks(); i++)
    {
        pthread_mutex_init(&(m_lockArray[i]), NULL);
    }

    CRYPTO_set_id_callback((unsigned long(*)())thread_id);
    CRYPTO_set_locking_callback((void(*)())lock_callback);
}

void HttpClient::kill_locks(void)
{
    int i;

    CRYPTO_set_locking_callback(NULL);
    for (i = 0; i < CRYPTO_num_locks(); i++)
    {
        pthread_mutex_destroy(&(m_lockArray[i]));
    }

    OPENSSL_free(m_lockArray);
}

size_t HttpClient::WriteToString(void *ptr, size_t size, size_t nmemb, void *stream)
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
    * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
    * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
    */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 45);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

    ret = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    return ret;
}

/*HTTP头使用默认Content-Type:application/x-www-form-urlencoded*/
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
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 45);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

    int ret = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    return ret;
}

/*设置HTTP头使用Content-Type:multipart/form-data*/
int HttpClient::PostForm(const std::string &url, const std::map<std::string, std::string> &reqFormMap, std::string &response)
{
    CURL *curl = curl_easy_init();
    if (curl == NULL)
    {
        return CURLE_FAILED_INIT;
    }

    struct curl_slist *slist = NULL;
    struct curl_httppost* post = NULL;
    struct curl_httppost* last = NULL;

    slist = curl_slist_append(slist, "Content-Type:multipart/form-data");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 45);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

    for (auto &form : reqFormMap)
    {
        curl_formadd(&post, &last, CURLFORM_COPYNAME, form.first.c_str(), CURLFORM_COPYCONTENTS, form.second.c_str(), CURLFORM_END);
    }

    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

    int ret = curl_easy_perform(curl);

    curl_formfree(post);

    curl_slist_free_all(slist);

    curl_easy_cleanup(curl);

    return ret;
}

int HttpClient::HttpsGet(const std::string &url, const std::list<std::string> &strHeaderList, std::string &response)
{
    CURLcode ret;
    CURL* curl = curl_easy_init();
    if (NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }

    struct curl_slist *slist = NULL;
    for (auto &strHeader : strHeaderList)
    {
        slist = curl_slist_append(slist, strHeader.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

    /**
    * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
    * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
    */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 45);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

    ret = curl_easy_perform(curl);

    curl_slist_free_all(slist);

    curl_easy_cleanup(curl);

    return ret;
}

int HttpClient::HttpsPost(const std::string &url, const std::string &request, std::string &response)
{
    CURL *curl = curl_easy_init();
    if (curl == NULL)
    {
        return CURLE_FAILED_INIT;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request.length());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 45);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

    int ret = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    return ret;
}

int HttpClient::HttpsPostForm(const std::string &url, const std::map<std::string, std::string> &reqFormMap, std::string &response)
{
    CURL *curl = curl_easy_init();
    if (curl == NULL)
    {
        return CURLE_FAILED_INIT;
    }

    struct curl_slist *slist = NULL;
    struct curl_httppost* post = NULL;
    struct curl_httppost* last = NULL;

    slist = curl_slist_append(slist, "Content-Type:multipart/form-data");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 45);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

    for (auto &form : reqFormMap)
    {
        curl_formadd(&post, &last, CURLFORM_COPYNAME, form.first.c_str(), CURLFORM_COPYCONTENTS, form.second.c_str(), CURLFORM_END);
    }

    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

    int ret = curl_easy_perform(curl);

    curl_formfree(post);

    curl_slist_free_all(slist);

    curl_easy_cleanup(curl);

    return ret;
}

int HttpClient::HttpsPostJson(const std::string &url, const std::string auth, const std::string &request, std::string &response)
{
    CURL *curl = curl_easy_init();
    if (curl == NULL)
    {
        return CURLE_FAILED_INIT;
    }

    struct curl_slist *slist = NULL;

    slist = curl_slist_append(slist, "Content-Type:application/json");
    if (!auth.empty()) slist = curl_slist_append(slist, auth.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request.length());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 45);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

    int ret = curl_easy_perform(curl);

    curl_slist_free_all(slist);

    curl_easy_cleanup(curl);

    return ret;
}
