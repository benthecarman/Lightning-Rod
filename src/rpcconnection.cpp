#include <stdlib.h>
#include <curl/curl.h>

#include <string>
#include <string.h>

#include "rpcconnection.h"
#include "logger.h"

RPCConnection::RPCConnection(const std::string url, const std::string userpwd) : url(url),
                                                                                 userpwd(userpwd)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
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

std::string RPCConnection::execute(std::string data)
{
    CURL *curl = curl_easy_init();
    struct curl_slist *headers = NULL;

    std::string s;

    if (curl)
    {
        headers = curl_slist_append(headers, "Connection: close");
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

    free(headers);

    return s;
}

std::string RPCConnection::testAvailable()
{
    std::string data = "{\"method\":\"echo\",\"params\":[],\"id\":\"test\"}";
    return execute(data);
}
