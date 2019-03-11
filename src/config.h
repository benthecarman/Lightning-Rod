#ifndef Config_H
#define Config_H

#include <algorithm>

static const bool DEFAULT_DAEMON = false;
static const bool DEFAULT_DEBUG = false;
static const int DEFAULT_PORT = 8331;
static const std::string DEFAULT_HOST = "http://127.0.0.1:8332/";
static const std::string DEFAULT_RPC_AUTH = "user:pass";
static const std::string DEFAULT_CONFIG_DIR = "/.lightning-rod/conf.cfg";
static const std::string DEFAULT_LOG_DIR = "/.lightning-rod/logs/";

static const std::vector<std::string> DEFAULT_CMD_WHITE_LIST = {
    "echo",
    "getblockhash",
    "getblock",
    "estimatesmartfee",
    "getbestblockhash",
    "sendrawtransaction",
    "getblockcount",
    "gettxout"};

class Config
{
  bool daemon;
  bool debug;
  int port;
  std::string host;
  std::string rpcAuth;
  std::string configdir;
  std::string logdir;
  std::vector<std::string> cmdWhiteList;
  std::vector<std::string> cmdBlackList;
  std::vector<std::string> ipBlackList;

public:
  Config();

  bool isDaemon()
  {
    return this->daemon;
  }
  void setIsDaemon(const bool d)
  {
    this->daemon = d;
  }
  bool isDebug()
  {
    return this->debug;
  }
  void setDebug(const bool db)
  {
    this->debug = db;
  }
  int getPort()
  {
    return this->port;
  }
  void setPort(const int p)
  {
    this->port = p;
  }
  std::string getHost()
  {
    return this->host;
  }
  void setHost(std::string const &h)
  {
    this->host = h;
  }
  std::string getRPCAuth()
  {
    return this->rpcAuth;
  }
  void setRPCAuth(std::string const &auth)
  {
    this->rpcAuth = auth;
  }
  std::string getConfigDir()
  {
    return this->configdir;
  }
  void setConfigDir(std::string const &dir)
  {
    this->configdir = dir;
  }
  std::string getLogDir()
  {
    return this->logdir;
  }
  void setLogDir(std::string const &dir)
  {
    this->logdir = dir;
  }
  std::vector<std::string> getCommandWhiteList()
  {
    return this->cmdWhiteList;
  }
  void whitelistCommand(const std::string cmd)
  {
    if (std::find(this->cmdWhiteList.begin(), this->cmdWhiteList.end(), cmd) == this->cmdWhiteList.end())
      this->cmdWhiteList.push_back(cmd);
  }
  std::vector<std::string> getCommandBlackList()
  {
    return this->cmdBlackList;
  }
  void blacklistCommand(const std::string cmd)
  {
    if (std::find(this->cmdBlackList.begin(), this->cmdBlackList.end(), cmd) == this->cmdBlackList.end())
      this->cmdBlackList.push_back(cmd);
  }
  std::vector<std::string> getIPBlackList()
  {
    return this->ipBlackList;
  }
  void blacklistIP(const std::string ip)
  {
    if (std::find(this->ipBlackList.begin(), this->ipBlackList.end(), ip) == this->ipBlackList.end())
      this->ipBlackList.push_back(ip);
  }

  std::string toString();
};

extern Config config;

void createConfig(const int argv, char *argc[]);

#endif