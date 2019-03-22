#ifndef ZMQSERVER_H
#define ZMQSERVER_H

class ZMQServer
{
  private:
    std::string host;
    std::string topic;
    int port;
    bool running = false;
    bool stopped = true;

  public:
    ZMQServer(const std::string, const std::string, const int);
    void start();
    int getPort()
    {
        return this->port;
    }
    void setPort(const int p)
    {
        this->port = p;
    }
    std::string getHost()
    {
        return this->host;
    }
    void setHost(std::string const &h)
    {
        this->host = h;
    }
    std::string getTopic()
    {
        return this->topic;
    }
    void setTopic(std::string const &t)
    {
        this->topic = t;
    }
    bool isRunning()
    {
        return this->running;
    }
    void setRunning(const bool r)
    {
        this->running = r;
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

#endif