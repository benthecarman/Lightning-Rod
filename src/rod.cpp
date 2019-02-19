#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <signal.h>

#include <string>
#include <cstring>
#include <thread>

#include "rpcconnection.h"
#include "server.h"

int main(int argc, char *argv[])
{
	RPCConnection rpc("http://127.0.0.1:8332/", "benLinuxPC828:fgCy5QaEgu&C&q$7");

	int sock, newSock;
	socklen_t c;
	char buffer[1024];

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
	serv_addr.sin_port = htons(8331);

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
	while (1)
	{
		newSock = accept(sock, (struct sockaddr *)&cli_addr, &c);
		printf("\nNew client connected!\n");
		printf("Number of clients: %d\n", ++count);

		if (newSock < 0)
		{
			perror("ERROR accepting connection");
			exit(1);
		}

		std::thread t(handleConnection, newSock, rpc);
		t.detach();
	}

	return 0;
}