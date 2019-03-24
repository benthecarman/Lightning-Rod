#include <stdlib.h>
#ifdef _WIN32
#include <curl.h>
#else
#include <curl/curl.h>
#endif

#include <string>
#include <cstring>

#include "rpcconnection.h"
#include "logger.h"

RPCConnection::RPCConnection(const std::string url, const std::string userpwd) : url(url),
                                                                                 userpwd(userpwd)
{
}

void RPCConnection::setURL(const std::string url)
{
    this->url = url;
}

void RPCConnection::setUserPwd(const std::string userpwd)
{
    this->userpwd = userpwd;
}

std::string RPCConnection::getURL()
{
    return this->url;
}

std::string RPCConnection::getUserPwd()
{
    return this->userpwd;
}

size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s)
{
    size_t newLength = size * nmemb;
    try
    {
        s->append((char *)contents, newLength);
    }
    catch (std::bad_alloc &e)
    {
        return 0;
    }

    return newLength;
}

std::string RPCConnection::execute(const std::string data)
{
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if (res != 0)
    {
#ifdef LOGGER_ENABLED
        logWarning("Failed curl init, request failed");
#endif
    }

    CURL *curl = curl_easy_init();
    struct curl_slist *headers = NULL;

    std::string s;

    if (curl)
    {
        headers = curl_slist_append(headers, "Connection: close");
        headers = curl_slist_append(headers, "Accept:");
        headers = curl_slist_append(headers, "Content-Type:");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_URL, this->url.c_str());

        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)data.length());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        curl_easy_setopt(curl, CURLOPT_USERPWD, this->userpwd.c_str());

        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

        curl_easy_perform(curl);
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return s;
}

std::string RPCConnection::execute()
{
    std::string data = "{\"method\":\"echo\",\"params\":[],\"id\":\"test\"}";
    return execute(data);
}