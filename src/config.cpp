#include <string>
#include <cstring>
#include <fstream>

#include "config.h"

Config::Config(bool daemon = DEFAULT_DAEMON,
               bool debug = DEFAULT_DEBUG,
               int port = DEFAULT_PORT,
               int maxConnections = DEFAULT_MAX_CONNECTIONS,
               std::string host = DEFAULT_HOST,
               std::string rpcAuth = DEFAULT_RPC_AUTH,
               std::string configdir = DEFAULT_CONFIG_DIR) : daemon(daemon),
                                                             debug(debug),
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

void Config::setIsDaemon(const bool d)
{
    this->daemon = d;
}

bool Config::isDebug()
{
    return this->debug;
}

void Config::setDebug(const bool db)
{
    this->debug = db;
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

// For debugging
std::string Config::toString()
{
    std::string str;

    str += "Debug mode: " + std::to_string(this->debug) + "\n";
    str += "isDaemon: " + std::to_string(this->daemon) + "\n";
    str += "host: " + this->host + "\n";
    str += "port: " + std::to_string(this->port) + "\n";
    str += "maxconnections: " + std::to_string(this->maxConnections) + "\n";
    str += "rpcauth: " + this->rpcAuth + "\n";
    str += "configdir: " + this->configdir + "\n";

    return str;
}

Config createConfig(const int argv, char *argc[])
{
    Config *cfg = new Config();

    int i;
    for (i = 1; i < argv; ++i)
    {
        std::string tmp(argc[i], 0, strlen(argc[i]));

        if (tmp.find("--configdir=") == 0)
        {
            cfg->setConfigDir(tmp.substr(12));
        }
    }

    parseConfig(cfg);
    parseArgs(cfg, argv, argc);

    if (cfg->isDebug())
        printf("Config:\n\n%s\n", cfg->toString().c_str());

    return cfg;
}

void parseArgs(Config *cfg, const int argv, char *argc[])
{
    int i;
    for (i = 1; i < argv; ++i)
    {
        std::string tmp(argc[i], 0, strlen(argc[i]));

        if (tmp.find("--port=") == 0)
        {
            cfg->setPort(std::stoi(tmp.substr(7)));
        }
        else if (tmp.find("--maxconnections=") == 0)
        {
            cfg->setMaxConnections(std::stoi(tmp.substr(17)));
        }
        else if (tmp.find("--host=") == 0)
        {
            cfg->setHost(tmp.substr(7));
        }
        else if (tmp.find("--rpcauth=") == 0)
        {
            cfg->setRPCAuth(tmp.substr(10));
        }
        else if (tmp.compare("--debug") == 0 || tmp.compare("-db") == 0)
        {
            cfg->setDebug(true);
        }
        else if (tmp.compare("--daemon") == 0 || tmp.compare("-d") == 0)
        {
            cfg->setIsDaemon(true);
        }
        else
        {
            printf("Unable to proccess config option %s\n", tmp.c_str());
        }
    }
}

void parseConfig(Config *cfg)
{
    std::string tmp;
    std::ifstream configFile(cfg->getConfigDir());
    if (configFile.is_open())
    {
        while (getline(configFile, tmp))
        {
            if (tmp.find("port=") == 0)
            {
                cfg->setPort(std::stoi(tmp.substr(5)));
            }
            else if (tmp.find("maxconnections=") == 0)
            {
                cfg->setMaxConnections(std::stoi(tmp.substr(15)));
            }
            else if (tmp.find("host=") == 0)
            {
                cfg->setHost(tmp.substr(5));
            }
            else if (tmp.find("rpcauth=") == 0)
            {
                cfg->setRPCAuth(tmp.substr(8));
            }
            else if (tmp.find("daemon=") == 0)
            {
                std::string d = tmp.substr(7);
                if (std::stoi(d) == 1 || d.compare("true") == 0)
                    cfg->setIsDaemon(true);
                else if (std::stoi(d) == 0 || d.compare("false") == 0)
                    cfg->setIsDaemon(false);
                else
                    printf("unable to proccess daemon config option\n");
            }
            else
            {
                printf("Unable to proccess config option %s\n", tmp.c_str());
            }
        }
        configFile.close();
    }
    else
    {
        printf("No config file found, using defaults\n");
    }
}