#include <string>

#include "config.h"

Config::Config()
{

}

bool Config::isDaemon()
{
    return this->daemon;
}

void Config::setIsDaemon(bool isDaemon)
{
    this->daemon = isDaemon;
}

int Config::getPort()
{
    return this->port;
}

void Config::setPort(int p)
{
    this->port = p;
}

int Config::getMaxConnections()
{
    return this->maxConnections;
}

void Config::setMaxConnections(int mc)
{
    this->maxConnections = mc;
}

std::string Config::getHost()
{
    return this->host;
}

void Config::setHost(std::string h)
{
    this->host = h;
}

std::string Config::getRPCAuth()
{
    return this->rpcAuth;
}

void Config::setRPCAuth(std::string auth)
{
    this->rpcAuth = auth;
}

std::string Config::getConfigDir()
{
    return this->configdir;
}

void Config::setConfigDir(std::string dir)
{
    this->configdir = dir;
}