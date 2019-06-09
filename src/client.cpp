#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <thread>

#include "client.h"
#include "config.h"
#include "logger.h"

Client::Client()
{
    // generate key pair
}

void Client::start()
{
    int sockRod, sockLnd, newSock;
    struct sockaddr_in servAddrRod, servAddrLnd, cliAddr;
    struct hostent *server;
    socklen_t cli;

    sockRod = socket(AF_INET, SOCK_STREAM, 0);
    sockLnd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockRod < 0 || sockLnd < 0)
    {
        logFatal("Error creating socket");
        exit(1);
    }

    server = gethostbyname(config.getHost().c_str());

    if (server == NULL)
    {
        logFatal("Host does not exist or is not accepting connections");
        exit(1);
    }

    memset((char *)&servAddrRod, '0', sizeof(servAddrRod));
    memset((char *)&servAddrLnd, '0', sizeof(servAddrLnd));

    servAddrRod.sin_family = AF_INET;
    servAddrLnd.sin_family = AF_INET;
    servAddrLnd.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servAddrLnd.sin_port = htons(config.getPort());

    int opt = 1;
    if (setsockopt(sockLnd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0)
    {
        logFatal("Error, setsockopt(SO_REUSEADDR) failed");
        exit(1);
    }

    if (bind(sockLnd, (struct sockaddr *)&servAddrLnd, sizeof(servAddrLnd)) < 0)
    {
        logFatal("Error binding socket for lightning instance");
        exit(1);
    }

    listen(sockLnd, SOMAXCONN);
    cli = sizeof(cliAddr);

    bcopy((char *)server->h_addr, (char *)&servAddrRod.sin_addr.s_addr, server->h_length);

    servAddrRod.sin_port = htons(config.getSparkPort());

    int c = connect(sockRod, (struct sockaddr *)&servAddrRod, sizeof(servAddrRod));

    if (c < 0)
    {
        logFatal("Error connecting to host");
        exit(1);
    }

    int hostVersion = -1;

    Handshake h;
    h.version = htons(0);
    h.pubkey = htonl(402); // Random number for testing

    sendto(sockRod, (const Handshake *)&h, sizeof(h), MSG_CONFIRM, NULL, 0);

    Handshake host;
    recv(sockRod, &host, sizeof(Handshake), MSG_CMSG_CLOEXEC);

    host.version = ntohs(host.version);
    host.pubkey = ntohl(host.pubkey);

    close(sockRod);

    this->running = true;
    this->stopped = false;

    while (this->running)
    {
        // Receive message from lnd
        newSock = accept(sockLnd, (struct sockaddr *)&cliAddr, &cli);

        char buffer[1024] = {0};
        int r = read(newSock, buffer, 1024);

        // Connect to host and send
        sockRod = socket(AF_INET, SOCK_STREAM, 0);
        server = gethostbyname(config.getHost().c_str());
        memset((char *)&servAddrRod, '0', sizeof(servAddrRod));
        servAddrRod.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *)&servAddrRod.sin_addr.s_addr, server->h_length);
        servAddrRod.sin_port = htons(config.getSparkPort());
        c = connect(sockRod, (struct sockaddr *)&servAddrRod, sizeof(servAddrRod));

        int s = send(sockRod, buffer, strlen(buffer), 0);

        // Get response from host
        char buffer2[1024] = {0};
        r = read(sockRod, buffer2, 1024);

        // Send response back to lnd
        send(newSock, buffer2, strlen(buffer2), 0);

        close(newSock);
        close(sockRod);
    }

    close(sockLnd);

    logInfo("Shutting down");

    this->stopped = true;
}
