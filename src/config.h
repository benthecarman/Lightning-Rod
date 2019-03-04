#ifndef Config_h
#define Config_h

static const bool DEFAULT_DAEMON = false;
static const bool DEFAULT_DEBUG = false;
static const int DEFAULT_PORT = 8331;
static const int DEFAULT_MAX_CONNECTIONS = 16;
static const std::string DEFAULT_HOST = "http://127.0.0.1:8332/";
static const std::string DEFAULT_RPC_AUTH = "user:pass";
static const std::string DEFAULT_CONFIG_DIR = "conf.cfg";

class Config
{
    bool daemon;
    bool debug;
    int port;
    int maxConnections;
    std::string host;
    std::string rpcAuth;
    std::string configdir;

  public:
    Config(bool, bool, int, int, std::string, std::string, std::string);
    void setPort(const int);
    int getPort();
    bool isDebug();
    void setDebug(const bool);
    bool isDaemon();
    void setIsDaemon(const bool);
    void setMaxConnections(const int);
    int getMaxConnections();
    std::string getHost();
    void setHost(std::string const &);
    std::string getRPCAuth();
    void setRPCAuth(std::string const &);
    void setConfigDir(std::string const &);
    std::string getConfigDir();
    std::string toString();
};

Config createConfig(const int argv, char *argc[]);

void parseArgs(Config*, const int, char **);
void parseConfig(Config *);

#endif