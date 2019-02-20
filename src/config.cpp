#include <string>
#include <cstring>

#include "src/rpcconnection.h"

int main(int argc, char *argv[])
{
    std::string port = "8331";
    if (argc > 1)
    {
        int i;
        for (i = 1; i < argc; ++i)
        {
            std::string tmp(argv[i], 0, strlen(argv[i]));

            if (tmp.find("--port=") == 0)
            {
                int pos = tmp.find("--port=");
                port = tmp.substr(pos + 7);
            }
        }
    }

    RPCConnection rpc("http://127.0.0.1:" + port + "/", "benLinuxPC828:fgCy5QaEgu&C&q$7");

    std::string rev = rpc.execute();

    printf("%s\n", rev.c_str());

    return 0;
}
#include <string>

#include "config.h"

Config::Config(bool daemon = DEFAULT_DAEMON,
               int port = DEFAULT_PORT,
               int maxConnections = DEFAULT_MAX_CONNECTIONS,
               std::string host = DEFAULT_HOST,
               std::string rpcAuth = DEFAULT_RPC_AUTH,
               std::string configdir = DEFAULT_CONFIG_DIR) : daemon(daemon),
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

void Config::setIsDaemon(const bool isDaemon)
{
    this->daemon = isDaemon;
}

int Config::getPort()
{
    return this->port;
}

void Config::setPort(const int p)
{
    this->port = p;
}

int Config::getMaxConnections()
{
    return this->maxConnections;
}

void Config::setMaxConnections(const int mc)
{
    this->maxConnections = mc;
}

std::string Config::getHost()
{
    return this->host;
}

void Config::setHost(std::string const &h)
{
    this->host = h;
}

std::string Config::getRPCAuth()
{
    return this->rpcAuth;
}

void Config::setRPCAuth(std::string const &auth)
{
    this->rpcAuth = auth;
}

std::string Config::getConfigDir()
{
    return this->configdir;
}

void Config::setConfigDir(std::string const &dir)
{
    this->configdir = dir;
}

Config createConfig(const int argv, char *argc[])
{
    Config* conf = new Config();

    parseArgs(conf, argv, argc);
    parseConfig(conf);

    return conf;
}

void parseArgs(Config *cfg, const int argv, char *argc[])
{
    
}

void parseConfig(Config *cfg)
{

}