#ifndef CLIENT_H
#define CLIENT_H

#include <cryptopp/osrng.h>

class Client
{
private:
    bool running = false;
    bool stopped = true;
    int version = 0;

public:
    Client();
    void start();

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