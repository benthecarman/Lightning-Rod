#ifndef OPTION_H
#define OPTION_H

#include <string>
static const std::string OPTION_DAEMON_SHORTCUT = "d";
static const std::string OPTION_DEBUG_SHORTCUT = "db";
static const std::string OPTION_DISABLEZMQ_SHORTCUT = "dzmq";

static const std::string OPTION_DAEMON_NAME = "daemon";
static const std::string OPTION_DEBUG_NAME = "debug";
static const std::string OPTION_DISABLEZMQ_NAME = "disablezmq";
static const std::string OPTION_ZMQBLOCKPORT_NAME = "zmqBlockPort";
static const std::string OPTION_ZMQTXPORT_NAME = "zmqTxPort";
static const std::string OPTION_HTTPAUTH_NAME = "httpauth";
static const std::string OPTION_PORT_NAME = "port";
static const std::string OPTION_HOST_NAME = "host";
static const std::string OPTION_RPCAUTH_NAME = "rpcauth";
static const std::string OPTION_CONFIGDIR_NAME = "configdir";
static const std::string OPTION_LOGDIR_NAME = "logdir";
static const std::string OPTION_WHITELISTCMD_NAME = "whitelistcmd";
static const std::string OPTION_BLACKLISTCMD_NAME = "blacklistcmd";
static const std::string OPTION_BLACKLISTIP_NAME = "blacklistip";

class Option
{
private:
    std::string name;
    std::string shortcut;
    std::string description;
    bool isBoolean = false;

public:
    Option();
    Option(const std::string, const std::string);
    Option(const std::string, const std::string, const bool);
    Option(const std::string, const std::string, const std::string);
    Option(const std::string, const std::string, const std::string, const bool);

    std::string getName()
    {
        return this->name;
    }
    void setName(const std::string n)
    {
        this->name = n;
    }
    std::string getShortcut()
    {
        return this->shortcut;
    }
    void setShortcut(const std::string s)
    {
        this->shortcut = s;
    }
    std::string getDescription()
    {
        return this->description;
    }
    void setDescription(const std::string d)
    {
        this->description = d;
    }
    bool isBool()
    {
        return this->isBoolean;
    }
    void setIsBool(const bool b)
    {
        this->isBoolean = b;
    }

    bool hasShortcut()
    {
        return !this->shortcut.empty();
    }
};

extern std::vector<Option> options;

void registerOptions();

#endif
