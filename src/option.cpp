#include <string>
#include <vector>

#include "option.h"
#include "logger.h"

std::vector<Option> options;

Option::Option()
{
}

Option::Option(const std::string name, const std::string desc) : name(name),
                                                                 description(desc)
{
}

Option::Option(const std::string name, const std::string desc, const bool isBool) : name(name),
                                                                                    description(desc),
                                                                                    isBoolean(isBool)
{
}

Option::Option(const std::string name, const std::string desc, const std::string shortcut) : name(name),
                                                                                             description(desc),
                                                                                             shortcut(shortcut)
{
}

Option::Option(const std::string name, const std::string desc, const std::string shortcut, const bool isBool) : name(name),
                                                                                                                description(desc),
                                                                                                                shortcut(shortcut),
                                                                                                                isBoolean(isBool)
{
}

void registerOptions()
{
    options.push_back(Option(OPTION_DAEMON_NAME, "Run in the background, suppress stdout/stderr", OPTION_DAEMON_SHORTCUT, true));
    options.push_back(Option(OPTION_DEBUGLEVEL_NAME, "Logging level for all subsystems {trace, debug, info, warning, error, fatal}", OPTION_DEBUG_SHORTCUT));
    options.push_back(Option(OPTION_DISABLEZMQ_NAME, "Disable ZeroMQ Ports", OPTION_DISABLEZMQ_SHORTCUT, true));
    options.push_back(Option(OPTION_ZMQBLOCKPORT_NAME, "Port the Lightning Rod will be broadcasting blocks through ZeroMQ"));
    options.push_back(Option(OPTION_ZMQTXPORT_NAME, "Port the Lightning Rod will be broadcasting transactions through ZeroMQ"));
    options.push_back(Option(OPTION_PORT_NAME, "Port the Lightning Rod will be listening for connections (0 to disable)"));
    options.push_back(Option(OPTION_SPARKPORT_NAME, "Port the Lightning Rod will be listening for connections from sparks (0 to disable)"));
    options.push_back(Option(OPTION_HTTPAUTH_NAME, "Authentication creditentials clients must use to make requests"));
    options.push_back(Option(OPTION_HOST_NAME, "Specify how to connect to the bitcoind RPC server"));
    options.push_back(Option(OPTION_RPCAUTH_NAME, "Authentication credentials for the bitcoind RPC server"));
    options.push_back(Option(OPTION_CONFIGDIR_NAME, "Specify where the configuration file is located"));
    options.push_back(Option(OPTION_BLACKLISTIPDIR_NAME, "Specify where the automatically blacklisted IP addresses file is located"));
    options.push_back(Option(OPTION_LOGDIR_NAME, "Specify where log files will be stored"));
    options.push_back(Option(OPTION_BANTHRESHOLD_NAME, "Number of disallowed requests before peer is automatically blacklisted (-1 to disable)"));
    options.push_back(Option(OPTION_WHITELISTCMD_NAME, "Whitelist a RPC command to allow peers to use"));
    options.push_back(Option(OPTION_BLACKLISTCMD_NAME, "Blacklist a RPC command to block peers from using"));
    options.push_back(Option(OPTION_BLACKLISTIP_NAME, "Blacklist an IP address from connecting to your node"));
}

void registerSparkOptions()
{
    options.push_back(Option(OPTION_DAEMON_NAME, "Run in the background, suppress stdout/stderr", OPTION_DAEMON_SHORTCUT, true));
    options.push_back(Option(OPTION_DEBUGLEVEL_NAME, "Logging level for all subsystems {trace, debug, info, warning, error, fatal}", OPTION_DEBUG_SHORTCUT));
    options.push_back(Option(OPTION_PORT_NAME, "Port your lightning node is attempt to connect with (default: 8332)"));
    options.push_back(Option(OPTION_HOST_NAME, "IP address and port of specified Lightning Rod (ex: 5.5.5.5:8330"));
    options.push_back(Option(OPTION_HTTPAUTH_NAME, "Authentication creditentials for the Lightning Rod (optional)"));
    options.push_back(Option(OPTION_LOGDIR_NAME, "Specify where log files will be stored"));
    options.push_back(Option(OPTION_CONFIGDIR_NAME, "Specify where the configuration file is located"));
}
