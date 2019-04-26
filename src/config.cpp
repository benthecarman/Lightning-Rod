#include <string>
#include <vector>
#include <fstream>
#include <boost/filesystem.hpp>

#include "config.h"
#include "option.h"
#include "logger.h"

Config config;

void parseArgs(const int, char **);
void parseConfig();
void processConfigLine(const std::string &, const bool);
void createSampleConfigFile();

Config::Config()
{
    this->daemon = DEFAULT_DAEMON;
    this->debug = DEFAULT_DEBUG;
    this->disablezmq = DEFAULT_ZMQ_DISABLED;
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

    str += "Debug mode: " + std::string(this->debug ? "true" : "false") + "\n";
    str += "daemon: " + std::string(this->daemon ? "true" : "false") + "\n";
    str += "disablezmq: " + std::string(this->disablezmq ? "true" : "false") + "\n";
    str += "host: " + this->host + "\n";
    str += "port: " + std::to_string(this->port) + "\n";
    str += "rpcauth: " + this->rpcAuth + "\n";
    str += "configdir: " + this->configdir + "\n";

    if (!this->disablezmq)
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
}

void parseArgs(const int argv, char *argc[])
{
    int i;
    for (i = 1; i < argv; ++i)
    {
        std::string tmp(argc[i]);

        processConfigLine(tmp, true);
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
            processConfigLine(tmp, false);
        }
        configFile.close();
    }
    else if (config.getConfigDir().find(DEFAULT_CONFIG_DIR) == std::string::npos)
    {
        logError("No config file found at specified location");
        exit(1);
    }
    else // No config file found and user did not give a location
    {
        createSampleConfigFile();
        logInfo("Created sample config file");
    }
}

void processConfigLine(const std::string &line, const bool isArg)
{
    if (line.find("#") == 0 || line.find(";") == 0 || line.length() <= 1) // Is a comment
        return;

    std::string str = isArg ? line : "--" + line; //Make it easier to parse

    if (str.find("#") > 0) //Remove comment at end of line
        str = str.substr(0, str.find("#"));
    if (str.find(";") > 0) //Remove comment at end of line
        str = str.substr(0, str.find(";"));

    for (auto &opt : options)
    {
        if (str.find("--" + opt.getName()) == 0)
        {
            if (isArg && opt.isBool())
            {
                if (opt.getName() == OPTION_DAEMON_NAME)
                {
                    config.setIsDaemon(true);
                }
                else if (opt.getName() == OPTION_DEBUG_NAME)
                {
                    config.setDebug(true);
                }
                else if (opt.getName() == OPTION_DISABLEZMQ_NAME)
                {
                    config.setDisableZMQ(true);
                }
            }
            else
            {
                int eq = str.find("= ");
                if (eq == std::string::npos)
                {
                    eq = str.find("=") - 1;
                }
                if (eq == std::string::npos) // Invalid string
                {
                    logError("Unable to proccess config option " + line);
                    exit(1);
                }
                std::string input = str.substr(eq + 2);

                if (opt.getName() == OPTION_DAEMON_NAME)
                {
                    int num = -1;
                    try
                    {
                        num = std::stoi(input);
                    }
                    catch (std::invalid_argument ignored)
                    {
                    }
                    if (num == 1 || input == "true")
                        config.setIsDaemon(true);
                    else if (num == 0 || input == "false")
                        config.setIsDaemon(false);
                    else
                    {
                        logError("Unable to proccess daemon config option: " + line);
                        exit(1);
                    }
                }
                else if (opt.getName() == OPTION_DEBUG_NAME)
                {
                    int num = -1;
                    try
                    {
                        num = std::stoi(input);
                    }
                    catch (std::invalid_argument ignored)
                    {
                    }
                    if (num == 1 || input == "true")
                        config.setDebug(true);
                    else if (num == 0 || input == "false")
                        config.setDebug(false);
                    else
                    {
                        logError("Unable to proccess debug config option: " + line);
                        exit(1);
                    }
                }
                else if (opt.getName() == OPTION_DISABLEZMQ_NAME)
                {
                    int num = -1;
                    try
                    {
                        num = std::stoi(input);
                    }
                    catch (std::invalid_argument ignored)
                    {
                    }
                    if (num == 1 || input == "true")
                        config.setDisableZMQ(true);
                    else if (num == 0 || input == "false")
                        config.setDisableZMQ(false);
                    else
                    {
                        logError("Unable to proccess disablezmq config option: " + line);
                        exit(1);
                    }
                }
                else if (opt.getName() == OPTION_ZMQBLOCKPORT_NAME)
                {
                    int num = -1;
                    try
                    {
                        num = std::stoi(input);
                    }
                    catch (std::invalid_argument ignored)
                    {
                        logError("Invalid zmqblockport number (" + input + "), must be between 1024 and 65535");
                        exit(1);
                    }
                    if (num <= 1024 || num > 65535)
                    {
                        logError("Invalid zmqblockport number (" + std::to_string(num) + "), must be between 1024 and 65535");
                        exit(1);
                    }
                    else
                    {
                        config.setZMQBlockPort(num);
                    }
                }
                else if (opt.getName() == OPTION_ZMQTXPORT_NAME)
                {
                    int num = -1;
                    try
                    {
                        num = std::stoi(input);
                    }
                    catch (std::invalid_argument ignored)
                    {
                        logError("Invalid zmqtxport number (" + input + "), must be between 1024 and 65535");
                        exit(1);
                    }
                    if (num <= 1024 || num > 65535)
                    {
                        logError("Invalid zmqtxport number (" + std::to_string(num) + "), must be between 1024 and 65535");
                        exit(1);
                    }
                    else
                    {
                        config.setZMQTxPort(num);
                    }
                }
                else if (opt.getName() == OPTION_PORT_NAME)
                {
                    int num = -1;
                    try
                    {
                        num = std::stoi(input);
                    }
                    catch (std::invalid_argument ignored)
                    {
                        logError("Invalid port number (" + input + "), must be between 1024 and 65535");
                        exit(1);
                    }
                    if (num <= 1024 || num > 65535)
                    {
                        logError("Invalid port number (" + std::to_string(num) + "), must be between 1024 and 65535");
                        exit(1);
                    }
                    else
                    {
                        config.setPort(num);
                    }
                }
                else if (opt.getName() == OPTION_HOST_NAME)
                {
                    config.setHost(input);
                }
                else if (opt.getName() == OPTION_RPCAUTH_NAME)
                {
                    config.setRPCAuth(input);
                }
                else if (opt.getName() == OPTION_CONFIGDIR_NAME)
                {
                    config.setConfigDir(input);
                }
                else if (opt.getName() == OPTION_LOGDIR_NAME)
                {
                    config.setLogDir(input);
                }
                else if (opt.getName() == OPTION_WHITELISTCMD_NAME)
                {
                    config.whitelistCommand(input);
                }
                else if (opt.getName() == OPTION_BLACKLISTCMD_NAME)
                {
                    config.blacklistCommand(input);
                }
                else if (opt.getName() == OPTION_BLACKLISTIP_NAME)
                {
                    config.blacklistIP(input);
                }
            }

            break;
        }
        else if (isArg && str == "-" + opt.getShortcut())
        {
            if (opt.getShortcut() == OPTION_DAEMON_SHORTCUT)
            {
                config.setIsDaemon(true);
            }
            else if (opt.getShortcut() == OPTION_DEBUG_SHORTCUT)
            {
                config.setDebug(true);
            }
            else if (opt.getShortcut() == OPTION_DISABLEZMQ_SHORTCUT)
            {
                config.setDisableZMQ(true);
            }

            break;
        }
    }
}

void createSampleConfigFile()
{
    boost::filesystem::path path = boost::filesystem::path(getenv("HOME") + DEFAULT_CONFIG_DIR + ".sample");

    std::ofstream samplecfg(path.string());

    // There might be a better way to do this
    samplecfg << "# Sample Lightining Rod Configuration File" << std::endl;
    samplecfg << "# Rename this file to conf.cfg to be used" << std::endl;
    samplecfg << "# All values are set to their default options" << std::endl;
    samplecfg << std::endl;
    samplecfg << "" << std::endl;
    samplecfg << "# Text following a '#' or a ';' will be considered comments and will be ignored" << std::endl;
    samplecfg << std::endl;
    samplecfg << "# The daemon options specifies if Lightning Rod will run in the background" << std::endl;
    samplecfg << "# while surpressing stdout/stderror, logging will still take place" << std::endl;
    samplecfg << "# Note: Can be enabled using \"1\" or \"true\" and disabled using \"0\" or \"false\"" << std::endl;
    samplecfg << "daemon = false" << std::endl;
    samplecfg << std::endl;
    samplecfg << "# The port option specifies what port your Lightning Rod will be listening on for" << std::endl;
    samplecfg << "# other users to connect with their lightning node" << std::endl;
    samplecfg << "port = 8331" << std::endl;
    samplecfg << std::endl;
    samplecfg << "# The host option specifies how to connect to your Bitcoin Core node's RPC server" << std::endl;
    samplecfg << "host = http://127.0.0.1:8332/" << std::endl;
    samplecfg << std::endl;
    samplecfg << "# The disablezmq option specifies if you want your Lightning Rod to relay ZMQ requests" << std::endl;
    samplecfg << "# Note: zmq must be enabled for lnd compatibility" << std::endl;
    samplecfg << "# Note: Can be enabled using \"1\" or \"true\" and disabled using \"0\" or \"false\"" << std::endl;
    samplecfg << "disablezmq = false" << std::endl;
    samplecfg << std::endl;
    samplecfg << "# The zmqblockhost option specifies how your Lighting Rod subscribes to your Bitcoin Core node's" << std::endl;
    samplecfg << "# raw block ZMQ server" << std::endl;
    samplecfg << "# Note: This should be the same as zmqpubrawblock in your bitcoin.conf" << std::endl;
    samplecfg << "zmqblockhost = tcp://127.0.0.1:28332" << std::endl;
    samplecfg << std::endl;
    samplecfg << "# The zmqtxhost option specifies how your Lighting Rod subscribes to your Bitcoin Core node's" << std::endl;
    samplecfg << "# raw tx ZMQ server" << std::endl;
    samplecfg << "# Note: This should be the same as zmqpubrawtx in your bitcoin.conf" << std::endl;
    samplecfg << "zmqtxhost = tcp://127.0.0.1:28333" << std::endl;
    samplecfg << std::endl;
    samplecfg << "# The zmqblockport option specifies what port you your Lighting Rod will be listening on" << std::endl;
    samplecfg << "# for other users to connect with their lightning node" << std::endl;
    samplecfg << "zmqblockport = 28330" << std::endl;
    samplecfg << std::endl;
    samplecfg << "# The zmqtxport option specifies what port you your Lighting Rod will be listening on" << std::endl;
    samplecfg << "# for other users to connect with their lightning node" << std::endl;
    samplecfg << "zmqtxport = 28331" << std::endl;
    samplecfg << std::endl;
    samplecfg << "# The rpcauth option specifies the rpc user and password for your Bitcoin Core node's RPC server" << std::endl;
    samplecfg << "# The format is rpcuser:rpcpassword" << std::endl;
    samplecfg << "# Note: It is highly recommended that you do not leave this as the default" << std::endl;
    samplecfg << "rpcauth = user:pass #CHANGE ME" << std::endl;
    samplecfg << std::endl;
    samplecfg << "# The debug option specifies whether or not to run in debug mode." << std::endl;
    samplecfg << "# With debug mode enabled you will recieve more information printed to the console." << std::endl;
    samplecfg << "# Note: Can be enabled using \"1\" or \"true\" and disabled using \"0\" or \"false\"" << std::endl;
    samplecfg << "debug = 0" << std::endl;
    samplecfg << std::endl;
    samplecfg << "# The logdir option specifies where your log files will be stored" << std::endl;
    samplecfg << ";logdir = ~/.lightning-rod/logs/" << std::endl;
    samplecfg << std::endl;
    samplecfg << "# The blacklistcmd option will blacklist the command given and will not allow a peer" << std::endl;
    samplecfg << "# to be passed to your Bitcoin Core node's RPC server" << std::endl;
    samplecfg << "# Note: This option can be specified multiple times." << std::endl;
    samplecfg << ";blacklistcmd=<cmd>" << std::endl;
    samplecfg << std::endl;
    samplecfg << "# The whitelistcmd option will whitelist the command given and will allow a peer" << std::endl;
    samplecfg << "# to be passed to your Bitcoin Core node's RPC server" << std::endl;
    samplecfg << "# Note: This option can be specified multiple times." << std::endl;
    samplecfg << ";whitelistcmd=<cmd>" << std::endl;
    samplecfg << std::endl;
    samplecfg << "# The blacklistip option will blacklist an IP address from connecting to your Lightning Rod" << std::endl;
    samplecfg << "# Note: IP addresses of peers can be found in your log files." << std::endl;
    samplecfg << "# Note: This option can be specified multiple times." << std::endl;
    samplecfg << ";blacklistip=<ip>" << std::endl;
    samplecfg << std::endl;

    samplecfg.close();
}