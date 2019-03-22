#ifndef SERVER_H
#define SERVER_H

#include "rpcconnection.h"

class Server
{
private:
  RPCConnection *rpc;
  bool running = false;
  bool stopped = true;

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
  bool isStopped()
  {
    return this->stopped;
  }
  void setStopped(const bool s)
  {
    this->stopped = s;
  }
};

void handleConnection(const int sock, RPCConnection rpc);
std::string parseHTTPRequest(const char *buffer);

#endif