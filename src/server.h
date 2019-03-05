#ifndef SERVER_H
#define SERVER_H

#include "rpcconnection.h"
#include "config.h"

class Server
{
private:
  Config cfg;
  RPCConnection *rpc;
  bool running = false;

public:
  Server(Config);
  void start();
  bool isRunning();
};

void handleConnection(const int sock, RPCConnection rpc);
std::string parseHTTPRequest(const char *buffer);

#endif