#ifndef CONFIG_H
#define CONFIG_H

#include <algorithm>
#include <cryptopp/secblock.h>

enum class DebugLevel
{
  trace,
  debug,
  info,
  warning,
  error,
  fatal
};

const std::string debugLevelStrings[] = {"trace", "debug", "info", "warning", "error", "fatal"};

static const DebugLevel DEFAULT_DEBUG_LEVEL = DebugLevel::info;
static const bool DEFAULT_DAEMON = false;
static const bool DEFAULT_ZMQ_DISABLED = false;
static const int DEFAULT_PORT = 8331;
static const int DEFAULT_SPARK_PORT = 8330;
static const int DEFAULT_ZMQ_BLOCK_PORT = 28330;
static const int DEFAULT_ZMQ_TX_PORT = 28331;
static const int DEFAULT_BAN_THRESHOLD = -1;
static const std::string DEFAULT_HTTP_AUTH;
static const std::string DEFAULT_HOST = "http://127.0.0.1:8332/";
static const std::string DEFAULT_ZMQ_BLOCK_HOST = "tcp://127.0.0.1:28332";
static const std::string DEFAULT_ZMQ_TX_HOST = "tcp://127.0.0.1:28333";
static const std::string DEFAULT_RPC_AUTH = "user:pass";
static const std::string DEFAULT_CONFIG_DIR = "/.lightning-rod/rod.cfg";
static const std::string DEFAULT_SPARK_CONFIG_DIR = "/.lightning-rod/spark.cfg";
static const std::string DEFAULT_BLACKLIST_IP_DIR = "/.lightning-rod/blacklisted-ips.txt";
static const std::string DEFAULT_LOG_DIR = "/.lightning-rod/logs/";

static const std::vector<std::string> DEFAULT_CMD_WHITE_LIST = {
    "echo",
    "estimatesmartfee",
    "getbestblockhash",
    "getblock",
    "getblockchaininfo",
    "getblockcount",
    "getblockhash",
    "getblockheader",
    "getnetworkinfo",
    "getrawtransaction",
    "gettxout",
    "sendrawtransaction"};

class Config
{
  bool spark = false;
  DebugLevel debugLevel;
  bool daemon;
  bool disablezmq;
  int version;
  int port;
  int sparkPort;
  int zmqBlockPort;
  int zmqTxPort;
  int banThreshold;
  std::string httpAuth;
  std::string httpAuthEncoded;
  std::string host;
  std::string zmqBlockHost;
  std::string zmqTxHost;
  std::string rpcAuth;
  std::string configdir;
  std::string blacklistipdir;
  std::string logdir;
  std::vector<std::string> cmdWhiteList;
  std::vector<std::string> cmdBlackList;
  std::vector<std::string> ipBlackList;
  std::vector<std::string> hosts;
  CryptoPP::SecByteBlock privkey, pubkey;

public:
  Config();

  bool isSpark()
  {
    return this->spark;
  }
  void setSpark(const bool s)
  {
    this->spark = s;
  }
  bool isDaemon()
  {
    return this->daemon;
  }
  void setIsDaemon(const bool d)
  {
    this->daemon = d;
  }
  DebugLevel getDebugLevel()
  {
    return this->debugLevel;
  }
  void setDebugLevel(const DebugLevel dbl)
  {
    this->debugLevel = dbl;
  }
  bool getDisableZMQ()
  {
    return this->disablezmq;
  }
  void setDisableZMQ(const bool z)
  {
    this->disablezmq = z;
  }
  int getPort()
  {
    return this->port;
  }
  void setPort(const int p)
  {
    this->port = p;
  }
  int getSparkPort()
  {
    return this->sparkPort;
  }
  void setSparkPort(const int sp)
  {
    this->sparkPort = sp;
  }
  int getZMQBlockPort()
  {
    return this->zmqBlockPort;
  }
  void setZMQBlockPort(const int p)
  {
    this->zmqBlockPort = p;
  }
  int getZMQTxPort()
  {
    return this->zmqTxPort;
  }
  void setBanThreshold(int bt)
  {
    this->banThreshold = bt;
  }
  int getBanThreshold()
  {
    return this->banThreshold;
  }
  void setZMQTxPort(const int p)
  {
    this->zmqTxPort = p;
  }
  bool hasHttpAuth()
  {
    return !this->httpAuth.empty();
  }
  CryptoPP::SecByteBlock getPubKey()
  {
    return this->pubkey;
  }
  void setPubkey(CryptoPP::SecByteBlock p)
  {
    this->pubkey = p;
  }
  CryptoPP::SecByteBlock getPrivKey()
  {
    return this->privkey;
  }
  void setPrivkey(CryptoPP::SecByteBlock p)
  {
    this->privkey = p;
  }
  void setHttpAuth(std::string const &);
  std::string getHttpAuth()
  {
    return this->httpAuth;
  }
  std::string getHttpAuthEncoded()
  {
    return this->httpAuthEncoded;
  }
  std::string getHost()
  {
    return this->host;
  }
  void setHost(std::string const &h)
  {
    if (this->spark)
      this->hosts.push_back(h);
    else
      this->host = h;
  }
  std::string getZMQBlockHost()
  {
    return this->zmqBlockHost;
  }
  void setZMQBlockHost(std::string const &z)
  {
    this->zmqBlockHost = z;
  }
  std::string getZMQTxHost()
  {
    return this->zmqTxHost;
  }
  void setZMQTxHost(std::string const &z)
  {
    this->zmqTxHost = z;
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
  std::string getBlacklistIPDir()
  {
    return this->blacklistipdir;
  }
  void setBlacklistIPDir(std::string const &dir)
  {
    this->blacklistipdir = dir;
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
  std::vector<std::string> getHosts()
  {
    return this->hosts;
  }

  bool isTxZMQValid()
  {
    return (this->zmqTxPort >= 1024 && this->zmqTxPort < 65535) && !this->zmqTxHost.empty();
  }
  bool isBlockZMQValid()
  {
    return (this->zmqBlockPort >= 1024 && this->zmqBlockPort < 65535) && !this->zmqBlockHost.empty();
  }

  bool rpcServerEnabled()
  {
    return this->port > 0;
  }
  bool sparkServerEnabled()
  {
    return this->sparkPort > 0;
  }

  std::string toString();
};

extern Config config;

void createConfig(const int argv, char *argc[]);

static std::string debugLevelToString(DebugLevel dbl)
{
  return debugLevelStrings[static_cast<int>(dbl)];
}

static DebugLevel stringToDebugLevel(std::string str);

void writeBlacklistedPeer(std::string);

#endif
