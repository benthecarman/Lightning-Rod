#include <string>
#include <cstring>
#include <thread>
#include <vector>
#include <algorithm>

#include <arpa/inet.h>
#include <unistd.h>

#include "rpcconnection.h"
#include "config.h"
#include "server.h"
#include "logger.h"

static std::string getPeerIP(const int &sock);
void handleRequest(int sock, RPCConnection *rpc, std::string peerIP);
int authenticateData(const std::string data);

std::vector<std::string> peers;

Server::Server()
{
	this->rpc = new RPCConnection(config.getHost(), config.getRPCAuth());
}

void Server::start()
{
	logDebug("Testing connection with the bitcoind RPC");
	std::string test = this->rpc->execute();

	// TODO check for RPC warming up
	if (test.find("{\"result\":[],\"error\":null,\"id\":\"test\"}") != 0)
	{
		logFatal("Failed initial RPC test, bitcoin-cli may not be running or your lightning rod is configured incorrectly.\n");
		exit(1);
	}
	else
	{
		logDebug("RPC Tests complete");
	}
	int sock, newSock;
	socklen_t c;

	struct sockaddr_in serv_addr, cli_addr;
	int n, pid;

	//Create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0)
	{
		perror("ERROR creating socket");
		exit(1);
	}

	memset((char *)&serv_addr, '0', sizeof(serv_addr));

	//Set necessary variables for connection
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(config.getPort());

	int opt = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0)
	{
		perror("ERROR setsockopt(SO_REUSEADDR) failed");
		exit(1);
	}

	if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("ERROR binding socket");
		exit(1);
	}

	listen(sock, 128);
	c = sizeof(cli_addr);

	this->running = true;
	this->stopped = false;

	logInfo("Lightning Rod ready to accept connections!");

	while (this->running)
	{
		newSock = accept(sock, (struct sockaddr *)&cli_addr, &c);

		// Check connection IP address
		std::string peerIP = getPeerIP(newSock);
		for (auto const &ip : config.getIPBlackList())
		{
			if (ip.compare(peerIP) == 0)
			{
				logWarning("Attempted connection from blocked IP (" + peerIP + ")");
				continue;
			}
		}
		if (std::find(peers.begin(), peers.end(), peerIP) == peers.end())
		{
			peers.push_back(peerIP);
			logInfo("New peer!");
			logDebug("Peer IP: " + peerIP);
		}

		if (newSock < 0)
		{
			perror("ERROR accepting connection");
			exit(1);
		}

		std::thread handle(handleRequest, newSock, this->rpc, peerIP);
		handle.detach();
	}

	logInfo("RPC Server shutdown");

	this->stopped = true;
}

static std::string getPeerIP(const int &sock)
{
	socklen_t len;
	struct sockaddr_storage addr;
	char ipstr[INET6_ADDRSTRLEN];

	len = sizeof addr;
	getpeername(sock, (struct sockaddr *)&addr, &len);

	if (addr.ss_family == AF_INET) // IPv4
	{
		struct sockaddr_in *s = (struct sockaddr_in *)&addr;
		inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
	}
	else // IPv6
	{
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
		inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
	}

	std::string ip(ipstr, 0, strlen(ipstr));
	return ip;
}

void handleRequest(int sock, RPCConnection *rpc, std::string peerIP)
{
	char buffer[1024] = {0};
	int readval = read(sock, buffer, 1024);
	if (readval < 0)
	{
		logError("Error reading message from " + peerIP);
		std::string sendString = "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n";

		send(sock, sendString.c_str(), sendString.length(), 0);

		close(sock);

		return;
	}
	std::string message(buffer);

	int pos = message.find("{\"");
	std::string data = "";

	if (pos != std::string::npos)
	{
		data = message.substr(pos);
	}

	int auth = authenticateData(data);

	if (auth != 0)
	{
		int f = data.find("\"id\":\"") + 5;
		std::string id = data.substr(f, data.find("\"", f) - f);

		f = data.find("\"method\":\"") + 10;
		std::string cmd = data.substr(f, data.find("\"", f) - f);

		if (auth == -1)
		{
			logWarning("Peer (" + peerIP + ") attempted non-whitelisted command (" + cmd + ")");
		}
		else if (auth == 1)
		{
			logWarning("Peer (" + peerIP + ") attempted blacklisted command (" + cmd + ")");
		}

		std::string result = "{\"result\":\"\",\"error\":\"invalid\",\"id\":" + id + "}";
		std::string header = "HTTP/1.1 403 Forbidden\r\nConnection: close\r\nContent-Length: " + std::to_string(result.length()) + "\r\n\r\n";

		std::string sendString = header + result;

		send(sock, sendString.c_str(), sendString.length(), 0);

		close(sock);

		return;
	}

	std::string result = rpc->execute(data);
	std::string header = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: " + std::to_string(result.length()) + "\r\n\r\n";

	std::string sendString = header + result;

	send(sock, sendString.c_str(), sendString.length(), 0);

	close(sock);
}

int authenticateData(const std::string data)
{
	/*
	 * Check whitelist then blacklist so commands are black list has priority
	 * Blacklist has priority for better protection
	 */
	bool whitelisted = false;
	for (auto const &cmd : config.getCommandWhiteList())
	{
		if (data.find("\"method\":\"" + cmd + "\"") != std::string::npos)
		{
			whitelisted = true;
			break;
		}
	}
	bool blacklisted = false;
	if (whitelisted && !config.getCommandBlackList().empty())
	{
		for (auto const &cmd : config.getCommandBlackList())
		{
			if (data.find("\"method\":\"" + cmd + "\"") != std::string::npos)
			{
				blacklisted = true;
				break;
			}
		}
	}

	if (whitelisted && !blacklisted) // Command is in whitelist and is not blacklisted
		return 0;
	if (!whitelisted && blacklisted) // Command is in blacklist and not whitelisted
		return 1;
	return -1; // Command is not in whitelist
}