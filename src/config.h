#ifndef Config_h
#define Config_h

static const bool DEFAULT_DAEMON = false;
static const int DEFAULT_PORT = 8331;
static const int DEFAULT_MAX_CONNECTIONS = 16;
static const std::string DEFAULT_HOST = "http://127.0.0.1:8332/";
static const std::string DEFAULT_RPC_AUTH = "user:pass";
static const std::string DEFAULT_CONFIG_DIR = "conf.cfg";

class Config
{
    bool daemon;
    int port;
    int maxConnections;
    std::string host;
    std::string rpcAuth;
    std::string configdir;

    public:
        Config(bool, int, int, std::string, std::string, std::string);
        void setPort(int);
        int getPort();
        bool isDaemon();
        void setIsDaemon(bool);
        void setMaxConnections(int);
        int getMaxConnections();
        std::string getHost();
        void setHost(std::string);
        std::string getRPCAuth();
        void setRPCAuth(std::string);
        void setConfigDir(std::string);
        std::string getConfigDir();
};

Config createConfig(int argv, char *argc[]);

#endif