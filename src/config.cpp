#include <string>

#include "config.h"

Config::Config(bool daemon = DEFAULT_DAEMON, int port = DEFAULT_PORT, int maxConnections = DEFAULT_MAX_CONNECTIONS, std::string host = DEFAULT_HOST, std::string rpcAuth = DEFAULT_RPC_AUTH, std::string configdir = DEFAULT_CONFIG_DIR):
    daemon(daemon),
    port(port),
    maxConnections(maxConnections),
    host(host),
    rpcAuth(rpcAuth),
    configdir(configdir)
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

Config createConfig(int argv, char* argc[])
{
    Config conf();

    return conf;
}