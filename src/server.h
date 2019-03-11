#ifndef SERVER_H
#define SERVER_H

#include "rpcconnection.h"

class Server
{
private:
  RPCConnection *rpc;
  bool running = false;

public:
  Server();
  void start();
  bool isRunning()
  {
    return this->running;
  }
  void setRunning(const bool r)
  {
    this->running = r;
  }
  RPCConnection *getRPC()
  {
    return this->rpc;
  }
};

void handleConnection(const int sock, RPCConnection rpc);
std::string parseHTTPRequest(const char *buffer);

#endif