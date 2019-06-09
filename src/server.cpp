#include <string>
#include <cstring>
#include <thread>
#include <vector>
#include <map>
#include <algorithm>

#include <arpa/inet.h>
#include <unistd.h>

#include "rpcconnection.h"
#include "config.h"
#include "server.h"
#include "logger.h"

static std::string getPeerIP(const int &sock);
void handleRequest(int sock, RPCConnection *rpc, std::string peerIP, bool isSpark);
int authenticateData(const std::string data);
void blackListPeer(std::string ip);

struct PeerInfo
{
	int infractions = 0;
	int version = -1;
	long key = 0;
};

std::map<std::string, PeerInfo> peers;
Handshake handshake;

Server::Server(const bool sparkServer) : sparkServer(sparkServer)
{
	this->rpc = new RPCConnection(config.getHost(), config.getRPCAuth());

	if (sparkServer)
	{
		//generate key pair
	}
}

bool Server::testRPCConnection(const bool printWait)
{
	std::string test = this->rpc->testAvailable();

	if (test.empty())
	{
		logFatal("Failed initial RPC test, bitcoin-cli may not be running or your lightning rod is configured incorrectly.\n");
		return false;
	}
	else if (test.find("{\"result\":null,\"error\":{\"code\":-28,\"message\":\"") != std::string::npos)
	{
		if (printWait)
		{
			logDebug("Bitcoin RPC warming up, waiting...");
		}
		sleep(5);
		return this->testRPCConnection(false);
	}
	else if (test.find("{\"result\":[],\"error\":null,\"id\":\"test\"}") != std::string::npos)
	{
		logDebug("RPC Tests complete");
		return true;
	}

	logFatal("Unkown error when testing RPC connection");
	return false;
}

void Server::start()
{
	std::string serverType = this->sparkServer ? "Spark" : "RPC";

	logDebug(serverType + " server testing connection with the bitcoind RPC");

	if (!this->testRPCConnection(true))
	{
		exit(1);
	}

	int sock, newSock;
	socklen_t c;

	struct sockaddr_in serv_addr, cli_addr;

	//Create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0)
	{
		logFatal("ERROR creating socket in " + serverType + " server");
		exit(1);
	}

	memset((char *)&serv_addr, '0', sizeof(serv_addr));

	//Set necessary variables for connection
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(this->sparkServer ? config.getSparkPort() : config.getPort());

	int opt = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0)
	{
		logFatal("ERROR setsockopt(SO_REUSEADDR) failed in " + serverType + " server");
		exit(1);
	}

	if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		logFatal("ERROR binding socket in " + serverType + " server");
		exit(1);
	}

	listen(sock, SOMAXCONN);
	c = sizeof(cli_addr);

	this->running = true;
	this->stopped = false;

	logInfo("Lightning Rod ready to accept connections! (" + serverType + ")");

	while (this->running)
	{
		newSock = accept(sock, (struct sockaddr *)&cli_addr, &c);

		// Check connection's IP address
		std::string peerIP = getPeerIP(newSock);
		if (std::find(config.getIPBlackList().begin(), config.getIPBlackList().end(), peerIP) != config.getIPBlackList().end())
		{
			logWarning("Attempted connection from blocked IP (" + peerIP + ")");
			close(newSock);
			continue;
		}

		if (peers.find(peerIP) == peers.end())
		{
			PeerInfo pi;
			peers.insert(std::pair<std::string, PeerInfo>(peerIP, pi));
			logInfo("New peer!");
			logDebug("Peer IP: " + peerIP);
		}

		if (newSock < 0)
		{
			perror("ERROR accepting connection");
			exit(1);
		}

		std::thread handle(handleRequest, newSock, this->rpc, peerIP, this->sparkServer);
		handle.detach();
	}

	logInfo(serverType + " server shutdown");

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

	std::string ip(ipstr);

	return ip;
}

void handleRequest(int sock, RPCConnection *rpc, std::string peerIP, bool isSpark)
{
	if (isSpark && peers.at(peerIP).version == -1) // New peer, need to handshake
	{
		//get key and version
		Handshake client;
		size_t bytes_received = recv(sock, &client, sizeof(Handshake), MSG_CMSG_CLOEXEC);
		// TODO: handle 0 recv

		client.version = ntohs(client.version);
		client.pubkey = ntohl(client.pubkey);

		peers.at(peerIP).version = client.version;

		handshake.version = 0;
		handshake.pubkey = 999; // Random number for testing

		//send public key and version
		sendto(sock, (const Handshake *)&handshake, sizeof(handshake), MSG_CONFIRM, NULL, 0);

		//generate session key

		close(sock);
		return;
	}

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

	if (isSpark)
	{
		// Decrypt message
	}

	if (config.hasHttpAuth() && message.find("Authorization: Basic " + config.getHttpAuthEncoded()) == std::string::npos) // Invalid HTTP Credentials
	{
		logWarning("Peer (" + peerIP + ") attempted to connect with invalid credentials");
		std::string sendString = "HTTP/1.1 401 Unauthorized\r\nConnection: close\r\n\r\n";

		send(sock, sendString.c_str(), sendString.length(), 0);

		close(sock);

		if (config.getBanThreshold() >= 0 && ++peers.at(peerIP).infractions > config.getBanThreshold())
		{
			blackListPeer(peerIP);
		}

		return;
	}

	int pos = message.find("{\"");

	if (pos == std::string::npos)
	{
		logError("Error getting message data from " + peerIP);
		std::string sendString = "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n";
		send(sock, sendString.c_str(), sendString.length(), 0);

		close(sock);
		return;
	}

	std::string data = message.substr(pos);

	int authData = authenticateData(data);

	// Disallowed commands attempted
	if (authData != 0)
	{
		int f = data.find("\"id\":\"") + 5;
		std::string id = data.substr(f, data.find("\"", f) - f);

		f = data.find("\"method\":\"") + 10;
		std::string cmd = data.substr(f, data.find("\"", f) - f);

		if (authData == -1)
		{
			logWarning("Peer (" + peerIP + ") attempted non-whitelisted command (" + cmd + ")");
		}
		else if (authData == 1)
		{
			logWarning("Peer (" + peerIP + ") attempted blacklisted command (" + cmd + ")");
		}

		std::string result = "{\"result\":\"\",\"error\":\"invalid\",\"id\":" + id + "}";
		std::string header = "HTTP/1.1 403 Forbidden\r\nConnection: close\r\nContent-Length: " + std::to_string(result.length()) + "\r\n\r\n";

		std::string sendString = header + result;

		send(sock, sendString.c_str(), sendString.length(), 0);

		close(sock);

		if (config.getBanThreshold() >= 0 && ++peers.at(peerIP).infractions > config.getBanThreshold())
		{
			blackListPeer(peerIP);
		}

		return;
	}

	std::string result = rpc->execute(data);

	int c = result.find("\"error\":{\"code\":");
	if (c != std::string::npos)
	{
		c += 16;
		int m = data.find("\"method\":\"") + 10;
		std::string cmd = data.substr(m, data.find("\"", m) - m);
		std::string code = result.substr(c, result.find(",\"", c) - c);

		logWarning("Command (" + cmd + ") returned error code " + code + ", there may be a problem with your bitcoin node");
	}

	std::string header = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: " + std::to_string(result.length()) + "\r\n\r\n";

	std::string sendString = header + result;

	send(sock, sendString.c_str(), sendString.length(), 0);

	close(sock);
}

void blackListPeer(std::string ip)
{
	config.blacklistIP(ip);
	logWarning("Peer (" + ip + ") has reached the ban threshold, peer is now blacklisted");

	writeBlacklistedPeer(ip);
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
