#include <string>
#include <zmq.hpp>

#include "logger.h"
#include "config.h"
#include "zmqserver.h"

ZMQServer::ZMQServer(const std::string topic, const std::string host, const int port) : topic(topic),
                                                                                        host(host),
                                                                                        port(port)
{
}

void ZMQServer::start()
{
    if (this->topic.empty())
    {
        logFatal("ZMQ Server Topic not set");
    }

    this->running = true;

    zmq::context_t pubContext(1);
    zmq::socket_t pubSocket(pubContext, ZMQ_PUB);
    std::string b = "tcp://*:" + std::to_string(this->port);
    pubSocket.bind(b);

    zmq::context_t subContext(1);
    zmq::socket_t subSocket(subContext, ZMQ_SUB);
    subSocket.connect(this->host);
    subSocket.setsockopt(ZMQ_SUBSCRIBE, this->topic.c_str(), 1);

    logInfo(this->topic + " ZMQ Server ready!");

    while (this->running)
    {
        zmq::message_t env;
        subSocket.recv(&env);
        std::string env_str = std::string(static_cast<char *>(env.data()), env.size());

        zmq::message_t msg;
        subSocket.recv(&msg);
        std::string msg_str = std::string(static_cast<char *>(msg.data()), msg.size());

        zmq::message_t env1(1);
        memcpy(env1.data(), env_str.c_str(), 1);
        zmq::message_t msg1(msg_str.size());
        memcpy(msg1.data(), msg_str.c_str(), msg_str.length());
        pubSocket.send(env1, ZMQ_SNDMORE);
        pubSocket.send(msg1);
    }
}