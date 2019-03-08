#ifndef Config_h
#define Config_h

static const bool DEFAULT_DAEMON = false;
static const bool DEFAULT_DEBUG = false;
static const int DEFAULT_PORT = 8331;
static const std::string DEFAULT_HOST = "http://127.0.0.1:8332/";
static const std::string DEFAULT_RPC_AUTH = "user:pass";
static const std::string DEFAULT_CONFIG_DIR = "/.lightning-rod/conf.cfg";
static const std::string DEFAULT_LOG_DIR = "/.lightning-rod/logs/";

class Config
{
  bool daemon;
  bool debug;
  int port;
  std::string host;
  std::string rpcAuth;
  std::string configdir;
  std::string logdir;

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
  std::string toString();
};

extern Config config;

void createConfig(const int argv, char *argc[]);

#endif