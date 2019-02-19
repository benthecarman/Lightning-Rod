#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>

#include <string>
#include <cstring>
#include <thread>

#include "rpcconnection.h"
#include "server.h"

Server::Server(RPCConnection r, int port):
    rpc(r),
    port(port)
{
}

void Server::setRPC(RPCConnection rpc)
{
    this->rpc = rpc;
}

RPCConnection Server::getRPC()
{
    return this->rpc;
}

void Server::setPort(int port)
{
    this->port = port;
}

int Server::getPort()
{
    return this->port;
}

bool Server::isRunning()
{
    return this->running;
}

void Server::start()
{
    this->running = true;

    int sock, newSock;
	socklen_t c;

	struct sockaddr_in serv_addr, cli_addr;

	//Create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0)
	{
		perror("ERROR creating socket");
		exit(1);
	}

	memset((char *)&serv_addr, '\0', sizeof(serv_addr));

	//Set necessary variables for connection
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(this->port);

	int optVal = 1;

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&optVal, sizeof(int)) < 0)
	{
		perror("ERROR setsockopt(SO_REUSEADDR) failed");
		exit(1);
	}

	if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("ERROR binding socket");
		exit(1);
	}

	listen(sock, SOMAXCONN);
	c = sizeof(cli_addr);

	printf("Able to start accepting connections\n");
	signal(SIGCHLD, SIG_IGN);

	int count = 0;
	while (this->running)
	{
		newSock = accept(sock, (struct sockaddr *)&cli_addr, &c);
		printf("\nNew request!\n");

		if (newSock < 0)
		{
			perror("ERROR accepting connection");
			exit(1);
		}

		std::thread t(handleConnection, newSock, this->rpc);
		t.detach();
		printf("Number of requests served: %d\n", ++count);
	}
}

void handleConnection(int sock, RPCConnection rpc)
{
    char buffer[1024] = {0};
    int valread = read(sock, buffer, 1024);

    std::string data = parseHTTPRequest(buffer);

	std::string sendString = rpc.execute(data);

    int s = send(sock, sendString.c_str(), sendString.length(), 0);
    printf("\nMessage sent!\n");

    close(sock);
}

std::string parseHTTPRequest(char buffer[1024])
{
    std::string b(buffer, 0, strlen(buffer));

    int pos = b.find("{\"jsonrpc\":");

    if (pos != std::string::npos)
    {
		return b.substr(pos);
    }

    return "";
}