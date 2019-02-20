#include <stdlib.h>
#include <curl/curl.h>

#include <string>
#include <string.h>

#include "rpcconnection.h"

RPCConnection::RPCConnection(std::string url, std::string userpwd):
    url(url),
    userpwd(userpwd)
{
}

void RPCConnection::setURL(std::string url)
{
    this->url = url;
}

void RPCConnection::setUserPwd(std::string userpwd)
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
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if (res != 0)
    {
        printf("Failed global init ...\n");
        exit(1);
    }

    CURL *curl = curl_easy_init();
    struct curl_slist *headers = NULL;

    std::string s;

    if (curl)
    {
        headers = curl_slist_append(headers, "content-type: text/plain;");
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

// Used for testing 
std::string RPCConnection::execute()
{
    std::string data = "{\"jsonrpc\": \"1.0\", \"id\":\"curltest\",\"method\": \"getblockcount\", \"params\": [] }";
    return execute(data);
}