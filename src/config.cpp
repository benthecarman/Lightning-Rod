#include <string>
#include <fstream>

#include <sys/socket.h>

#include "config.h"

void parseArgs(Config *, const int, char **);
void parseConfig(Config *);
void parseConfigLine(Config *cfg, const std::string &line, bool isArg);

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
    str += "daemon: " + std::to_string(this->daemon) + "\n";
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
        std::string tmp(argc[i]);

        if (tmp.find("--configdir=") == 0)
        {
            cfg->setConfigDir(tmp.substr(12));
        }
    }

    parseConfig(cfg);
    parseArgs(cfg, argv, argc);

    if (cfg->isDebug())
        printf("Config:\n\n%s\n", cfg->toString().c_str());

    return *cfg;
}

void parseArgs(Config *cfg, const int argv, char *argc[])
{
    int i;
    for (i = 1; i < argv; ++i)
    {
        std::string tmp(argc[i]);

        parseConfigLine(cfg, tmp, true);
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
            parseConfigLine(cfg, tmp, false);
        }
        configFile.close();
    }
    else
    {
        //TODO: Create sample config
        printf("No config file found, using defaults.\n");
    }
}

void parseConfigLine(Config *cfg, const std::string &line, bool isArg)
{
    if (line.find("#") == 0) // Is a comment
        return;

    std::string tmp = isArg ? line : "--" + line; //Make it easier to pasrse

    if (tmp.find("#") > 0) //Remove comment at end of line
        tmp = tmp.substr(0, tmp.find("#"));

    if (tmp.find("--port=") == 0 || tmp.find("--port =") == 0)
    {
        int sub = 7;
        if (tmp.find("--port = ") == 0)
            sub = 9;
        else if (tmp.find("--port =") == 0)
            sub = 8;

        std::string d = tmp.substr(sub);
        int x = -1;
        try
        {
            x = std::stoi(d);
        }
        catch (std::invalid_argument ignored)
        {
            printf("Invalid port number (%s), must be between 1024 and 65535\n", d.c_str());
            exit(1);
        }
        if (x <= 1024 || x > 65535)
        {
            printf("Invalid port number (%d), must be between 1024 and 65535\n", x);
            exit(1);
        }
        else
            cfg->setPort(x);
    }
    else if (tmp.find("--maxconnections=") == 0 || tmp.find("--maxconnections =") == 0)
    {
        int sub = 17;
        if (tmp.find("--maxconnections = ") == 0)
            sub = 19;
        else if (tmp.find("--maxconnections =") == 0)
            sub = 18;

        std::string d = tmp.substr(sub);
        int x = -1;
        try
        {
            x = std::stoi(d);
        }
        catch (std::invalid_argument ignored)
        {
            printf("Invalid max connections (%s), must be greater than 1 (0 for unlimited).\n", d.c_str());
            exit(1);
        }
        if (x < 0)
        {
            printf("Invalid max connections (%d), must be greater than 1 (0 for unlimited).\n", x);
            exit(1);
        }
        else if (x == 0)
        {
            cfg->setMaxConnections(SOMAXCONN);
        }
        else
            cfg->setMaxConnections(x);
    }
    else if (tmp.find("--host=") == 0 || tmp.find("--host =") == 0)
    {
        int sub = 7;
        if (tmp.find("--host = ") == 0)
            sub = 9;
        else if (tmp.find("--host =") == 0 || tmp.find("--host= ") == 0)
            sub = 8;

        cfg->setHost(tmp.substr(sub));
    }
    else if (tmp.find("--rpcauth=") == 0 || tmp.find("--rpcauth =") == 0)
    {
        int sub = 10;
        if (tmp.find("--rpcauth = ") == 0)
            sub = 12;
        else if (tmp.find("--rpcauth =") == 0 || tmp.find("--rpcauth= ") == 0)
            sub = 11;

        cfg->setRPCAuth(tmp.substr(sub));
    }
    else if (!isArg && (tmp.find("--daemon=") == 0 || tmp.find("--daemon =") == 0))
    {
        int sub = 9;
        if (tmp.find("--daemon = ") == 0)
            sub = 11;
        else if (tmp.find("--daemon =") == 0 || tmp.find("--daemon= ") == 0)
            sub = 10;

        std::string d = tmp.substr(sub);
        int x = -1;
        try
        {
            x = std::stoi(d);
        }
        catch (std::invalid_argument ignored)
        {
        }
        if (x == 1 || d.compare("true") == 0)
            cfg->setIsDaemon(true);
        else if (x == 0 || d.compare("false") == 0)
            cfg->setIsDaemon(false);
        else
        {
            printf("Unable to proccess daemon config option\n");
            exit(1);
        }
    }
    else if (isArg && (tmp.compare("--daemon") == 0 || tmp.compare("-d") == 0))
    {
        cfg->setIsDaemon(true);
    }
    else if (!isArg && (tmp.find("--debug=") == 0 || tmp.find("--debug =") == 0))
    {
        int sub = 8;
        if (tmp.find("--debug = ") == 0)
            sub = 10;
        else if (tmp.find("--debug =") == 0 || tmp.find("--debug= ") == 0)
            sub = 9;

        std::string d = tmp.substr(sub);
        int x = -1;
        try
        {
            x = std::stoi(d);
        }
        catch (std::invalid_argument ignored)
        {
        }
        if (x == 1 || d.compare("true") == 0)
            cfg->setDebug(true);
        else if (x == 0 || d.compare("false") == 0)
            cfg->setDebug(false);
        else
        {
            printf("Unable to proccess debug config option\n");
            exit(1);
        }
    }
    else if (isArg && (tmp.compare("--debug") == 0 || tmp.compare("-db") == 0))
    {
        cfg->setDebug(true);
    }
    else
    {
        printf("Unable to proccess config option %s\n", tmp.c_str());
    }
}