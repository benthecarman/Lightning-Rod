#ifndef SERVER_H
#define SERVER_H

#include "rpcconnection.h"
#include "config.h"

class Server
{
  Config cfg;
  RPCConnection *rpc;
  bool running = false;

public:
  Server(Config);
  void start();
  bool isRunning();
};

void handleConnection(int sock, RPCConnection rpc);
std::string parseHTTPRequest(char *buffer);

#endif