#include <sys/socket.h>
#include <unistd.h>

#include <string>

#include "rpcconnection.h"

void handleConnection(int sock, RPCConnection rpc)
{
    std::string sendString = rpc.execute();

    char buffer[1024] = {0};
    int valread = read(sock , buffer, 1024);

    printf("%s\n", buffer);

    int s = send(sock, sendString.c_str(), sendString.length(), 0);
    printf("\nMessage sent!\n");

    close(sock);
}