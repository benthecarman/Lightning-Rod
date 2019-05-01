#ifndef RPCCONNECTION_H
#define RPCCONNECTION_H

class RPCConnection
{
private:
  std::string url, userpwd;

public:
  RPCConnection(const std::string, const std::string);
  void setURL(const std::string);
  std::string getURL();
  void setUserPwd(const std::string);
  std::string getUserPwd();
  std::string execute(const std::string);
  std::string testAvailable();
  void sendBack(const std::string, const std::string);
};

#endif