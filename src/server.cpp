#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>

#include <string>
#include <cstring>
#include <thread>

#include "rpcconnection.h"
#include "config.h"
#include "server.h"

Server::Server(Config c) : cfg(c)
{
	this->rpc = new RPCConnection(this->cfg.getHost(), this->cfg.getRPCAuth());
	this->rpc = new RPCConnection(this->cfg.getHost(), this->cfg.getRPCAuth());
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
	serv_addr.sin_port = htons(this->cfg.getPort());

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

	listen(sock, this->cfg.getMaxConnections());
	c = sizeof(cli_addr);

	printf("Testing connection\n");
	std::string test = this->rpc->execute("{\"method\":\"echo\",\"params\":[],\"id\":1}");

	if (test.find("{\"result\":[],\"error\":null,\"id\":1}") != 0)
	{
		printf("Initial RPC Test failed, bitcoin-cli may not be running or your lightning rod is configured incorrectly.\n");
		exit(1);
	}
	else
	{
		printf("Tests complete\n\n");
	}

	printf("Able to start accepting connections\n");
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

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

		std::thread t(handleConnection, newSock, *(this->rpc));
		t.detach();
		printf("Number of requests served: %d\n", ++count);
	}
}

void handleConnection(int sock, RPCConnection rpc)
{
	char buffer[1024] = {0};
	int valread = read(sock, buffer, 1024);

	printf("\n%s\n", buffer);

	std::string data = parseHTTPRequest(buffer);

	std::string sendString = rpc.execute(data);

	bool socksend = true;

	if (socksend)
	{
		int s = send(sock, sendString.c_str(), sendString.length(), 0);
		printf("Sent: %s\n", sendString.c_str());
		printf("Sent l: %d\n", sendString.length());
		printf("Send: %d\n", s);
	}
	else
	{
		socklen_t len;
		struct sockaddr_storage addr;
		char ipstr[INET6_ADDRSTRLEN];
		int port;

		len = sizeof addr;
		getpeername(sock, (struct sockaddr *)&addr, &len);

		// deal with both IPv4 and IPv6:
		if (addr.ss_family == AF_INET)
		{
			struct sockaddr_in *s = (struct sockaddr_in *)&addr;
			port = ntohs(s->sin_port);
			inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
		}
		else // AF_INET6
		{
			struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
			port = ntohs(s->sin6_port);
			inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
		}

		std::string *ip = new std::string(ipstr, 0, strlen(ipstr));
		std::string url = *ip + ":" + std::to_string(port);
		rpc.sendBack(url, sendString);
	}

	printf("\nMessage sent!\n");
	printf("\n%s\n", sendString.c_str());

	close(sock);
}

std::string parseHTTPRequest(char buffer[1024])
{
	std::string b(buffer, 0, strlen(buffer));

	int pos = b.find("{\"");

	if (pos != std::string::npos)
	{
		return b.substr(pos);
	}

	return "";
}