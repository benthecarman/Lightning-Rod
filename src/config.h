#ifndef Config_h
#define Config_h

class Config
{
    bool daemon = false;
    int port = 8331;
    int maxConnections = 16;
    std::string host = "http://127.0.0.1:8332/";
    std::string rpcAuth = "user:pass";
    std::string configdir = "config.cfg";

    public:
        Config();
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

#endif