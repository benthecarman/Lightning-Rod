#ifndef SERVER_H
#define SERVER_H

#include "rpcconnection.h"

class Server
{
    RPCConnection rpc;
    int port;
    bool running = false;

  public:
    Server(RPCConnection, int);
    void start();

    void setRPC(RPCConnection);
    RPCConnection getRPC();
    void setPort(int);
    int getPort();
    bool isRunning();
};

void handleConnection(int sock, RPCConnection rpc);
std::string parseHTTPRequest(char *buffer);

#endif