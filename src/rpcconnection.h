#ifndef RPCConnection_H
#define RPCConnection_H

class RPCConnection
{
  std::string url, userpwd;

public:
  RPCConnection(std::string, std::string);
  void setURL(std::string);
  std::string getURL();
  void setUserPwd(std::string);
  std::string getUserPwd();
  std::string execute(std::string);
  std::string execute();
  void sendBack(std::string, std::string);
};

#endif