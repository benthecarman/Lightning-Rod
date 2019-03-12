#include <string>
#include <vector>
#include <fstream>
#include <boost/filesystem.hpp>

#include "config.h"
#include "logger.h"

Config config;

void parseArgs(const int, char **);
void parseConfig();
void parseConfigLine(const std::string &, const bool);

Config::Config()
{
    this->daemon = DEFAULT_DAEMON;
    this->debug = DEFAULT_DEBUG;
    this->zmqEnabled = DEFAULT_ZMQ_ENABLED;
    this->port = DEFAULT_PORT;
    this->zmqBlockPort = DEFAULT_ZMQ_BLOCK_PORT;
    this->zmqTxPort = DEFAULT_ZMQ_TX_PORT;
    this->host = DEFAULT_HOST;
    this->zmqBlockHost = DEFAULT_ZMQ_BLOCK_HOST;
    this->zmqTxHost = DEFAULT_ZMQ_TX_HOST;
    this->rpcAuth = DEFAULT_RPC_AUTH;
    boost::filesystem::path path = boost::filesystem::path(getenv("HOME") + DEFAULT_CONFIG_DIR);
    this->configdir = path.string();
    this->logdir = DEFAULT_LOG_DIR;
    this->cmdWhiteList = DEFAULT_CMD_WHITE_LIST;
}

std::string Config::toString()
{
    std::string str;

    str += "Debug mode: " + std::to_string(this->debug) + "\n";
    str += "daemon: " + std::to_string(this->daemon) + "\n";
    str += "zmqEnabled: " + std::to_string(this->zmqEnabled) + "\n";
    str += "host: " + this->host + "\n";
    str += "port: " + std::to_string(this->port) + "\n";
    str += "rpcauth: " + this->rpcAuth + "\n";
    str += "configdir: " + this->configdir + "\n";

    if (this->zmqEnabled)
    {
        str += "\n";
        str += "zmqBlockPort: " + std::to_string(this->zmqBlockPort) + "\n";
        str += "zmqBlockHost: " + this->zmqBlockHost + "\n";
        str += "zmqTxPort: " + std::to_string(this->zmqTxPort) + "\n";
        str += "zmqTxHost: " + this->zmqTxHost + "\n";
    }

    return str;
}

void createConfig(const int argv, char *argc[])
{
    int i;
    for (i = 1; i < argv; ++i)
    {
        std::string tmp(argc[i]);

        if (tmp.find("--configdir=") == 0)
        {
            config.setConfigDir(tmp.substr(12));
        }
    }

    parseConfig();
    parseArgs(argv, argc);

    if (config.getZMQBlockPort() == config.getZMQTxPort())
    {
        logFatal("Config option: zmqblockport cannot be the same as zmqtxport");
    }
    if (config.getPort() == config.getZMQTxPort())
    {
        logFatal("Config option: port cannot be the same as zmqtxport");
    }
    if (config.getPort() == config.getZMQBlockPort())
    {
        logFatal("Config option: port cannot be the same as zmqblockport");
    }

    logDebug("Current Config:\n\n" + config.toString());
}

void parseArgs(const int argv, char *argc[])
{
    int i;
    for (i = 1; i < argv; ++i)
    {
        std::string tmp(argc[i]);

        parseConfigLine(tmp, true);
    }
}

void parseConfig()
{
    std::string tmp;
    std::ifstream configFile(config.getConfigDir());
    if (configFile.is_open())
    {
        while (getline(configFile, tmp))
        {
            parseConfigLine(tmp, false);
        }
        configFile.close();
    }
    else if (config.getConfigDir().find(DEFAULT_CONFIG_DIR) != std::string::npos)
    {
        logDebug("No config file found, using defaults.");
    }
    else
    {
        //TODO: Create sample config
        //logDebug("Created sample config file");
    }
}

void parseConfigLine(const std::string &line, const bool isArg)
{
    if (line.find("#") == 0 || line.find(";") == 0 || line.length() <= 1) // Is a comment
        return;

    std::string tmp = isArg ? line : "--" + line; //Make it easier to parse

    if (tmp.find("#") > 0) //Remove comment at end of line
        tmp = tmp.substr(0, tmp.find("#"));
    if (tmp.find(";") > 0) //Remove comment at end of line
        tmp = tmp.substr(0, tmp.find(";"));

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
            logError("Invalid port number (" + d + "), must be between 1024 and 65535");
            exit(1);
        }
        if (x <= 1024 || x > 65535)
        {
            logError("Invalid port number (" + std::to_string(x) + "), must be between 1024 and 65535");
            exit(1);
        }
        else
            config.setPort(x);
    }
    else if (tmp.find("--zmqblockport=") == 0 || tmp.find("--zmqblockport =") == 0)
    {
        int sub = 15;
        if (tmp.find("--zmqblockport = ") == 0)
            sub = 17;
        else if (tmp.find("--zmqblockport =") == 0)
            sub = 16;

        std::string d = tmp.substr(sub);
        int x = -1;
        try
        {
            x = std::stoi(d);
        }
        catch (std::invalid_argument ignored)
        {
            logError("Invalid zmqblockport number (" + d + "), must be between 1024 and 65535");
            exit(1);
        }
        if (x <= 1024 || x > 65535)
        {
            logError("Invalid zmqblockport number (" + std::to_string(x) + "), must be between 1024 and 65535");
            exit(1);
        }
        else
            config.setZMQBlockPort(x);
    }
    else if (tmp.find("--zmqtxport=") == 0 || tmp.find("--zmqtxport =") == 0)
    {
        int sub = 12;
        if (tmp.find("--zmqtxport = ") == 0)
            sub = 14;
        else if (tmp.find("--zmqtxport =") == 0)
            sub = 13;

        std::string d = tmp.substr(sub);
        int x = -1;
        try
        {
            x = std::stoi(d);
        }
        catch (std::invalid_argument ignored)
        {
            logError("Invalid zmqtxport number (" + d + "), must be between 1024 and 65535");
            exit(1);
        }
        if (x <= 1024 || x > 65535)
        {
            logError("Invalid zmqtxport number (" + std::to_string(x) + "), must be between 1024 and 65535");
            exit(1);
        }
        else
            config.setZMQTxPort(x);
    }
    else if (tmp.find("--host=") == 0 || tmp.find("--host =") == 0)
    {
        int sub = 7;
        if (tmp.find("--host = ") == 0)
            sub = 9;
        else if (tmp.find("--host =") == 0 || tmp.find("--host= ") == 0)
            sub = 8;

        config.setHost(tmp.substr(sub));
    }
    else if (tmp.find("--zmqblockhost=") == 0 || tmp.find("--zmqblockhost =") == 0)
    {
        int sub = 15;
        if (tmp.find("--zmqblockhost = ") == 0)
            sub = 17;
        else if (tmp.find("--zmqblockhost =") == 0 || tmp.find("--zmqblockhost= ") == 0)
            sub = 16;

        config.setZMQBlockHost(tmp.substr(sub));
    }
    else if (tmp.find("--zmqtxhost=") == 0 || tmp.find("--zmqtxhost =") == 0)
    {
        int sub = 12;
        if (tmp.find("--zmqtxhost = ") == 0)
            sub = 14;
        else if (tmp.find("--zmqtxhost =") == 0 || tmp.find("--zmqtxhost= ") == 0)
            sub = 13;

        config.setZMQTxHost(tmp.substr(sub));
    }
    else if (tmp.find("--rpcauth=") == 0 || tmp.find("--rpcauth =") == 0)
    {
        int sub = 10;
        if (tmp.find("--rpcauth = ") == 0)
            sub = 12;
        else if (tmp.find("--rpcauth =") == 0 || tmp.find("--rpcauth= ") == 0)
            sub = 11;

        config.setRPCAuth(tmp.substr(sub));
    }
    else if (tmp.find("--whitelistcmd=") == 0 || tmp.find("--whitelistcmd =") == 0)
    {
        int sub = 15;
        if (tmp.find("--whitelistcmd = ") == 0)
            sub = 17;
        else if (tmp.find("--whitelistcmd =") == 0 || tmp.find("--whitelistcmd= ") == 0)
            sub = 16;

        config.whitelistCommand(tmp.substr(sub));
    }
    else if (tmp.find("--blacklistcmd=") == 0 || tmp.find("--blacklistcmd =") == 0)
    {
        int sub = 15;
        if (tmp.find("--blacklistcmd = ") == 0)
            sub = 17;
        else if (tmp.find("--blacklistcmd =") == 0 || tmp.find("--blacklistcmd= ") == 0)
            sub = 16;

        config.blacklistCommand(tmp.substr(sub));
    }
    else if (tmp.find("--blacklistip=") == 0 || tmp.find("--blacklistip =") == 0)
    {
        int sub = 14;
        if (tmp.find("--blacklistip = ") == 0)
            sub = 16;
        else if (tmp.find("--blacklistip =") == 0 || tmp.find("--blacklistip= ") == 0)
            sub = 15;

        config.blacklistIP(tmp.substr(sub));
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
            config.setIsDaemon(true);
        else if (x == 0 || d.compare("false") == 0)
            config.setIsDaemon(false);
        else
        {
            logError("Unable to proccess daemon config option");
            exit(1);
        }
    }
    else if (isArg && (tmp.compare("--daemon") == 0 || tmp.compare("-d") == 0))
    {
        config.setIsDaemon(true);
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
            config.setDebug(true);
        else if (x == 0 || d.compare("false") == 0)
            config.setDebug(false);
        else
        {
            logError("Unable to proccess debug config option");
            exit(1);
        }
    }
    else if (isArg && (tmp.compare("--debug") == 0 || tmp.compare("-db") == 0))
    {
        config.setDebug(true);
    }
    else if (tmp.find("--zmqenabled=") == 0 || tmp.find("--zmqenabled =") == 0)
    {
        int sub = 13;
        if (tmp.find("--zmqenabled = ") == 0)
            sub = 15;
        else if (tmp.find("--zmqenabled =") == 0 || tmp.find("--zmqenabled= ") == 0)
            sub = 14;

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
            config.setZMQEnabled(true);
        else if (x == 0 || d.compare("false") == 0)
            config.setZMQEnabled(false);
        else
        {
            logError("Unable to proccess debug config option");
            exit(1);
        }
    }
    else
    {
        logError("Unable to proccess config option " + line);
        exit(1);
    }
}