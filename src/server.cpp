#include <string>
#include <cstring>
#include <thread>
#include <vector>
#include <map>
#include <algorithm>

#include <cryptopp/osrng.h>
#include <cryptopp/aes.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/secblock.h>
#include <cryptopp/oids.h>
#include <cryptopp/asn.h>

#include <arpa/inet.h>
#include <unistd.h>

#include "rpcconnection.h"
#include "config.h"
#include "server.h"
#include "logger.h"
#include "aes.h"

static const std::string HTTP_200 = "HTTP/1.1 200 OK\r\nConnection: close\r\n";
static const std::string HTTP_400 = "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n";
static const std::string HTTP_401 = "HTTP/1.1 401 Unauthorized\r\nConnection: close\r\n\r\n";
static const std::string HTTP_403 = "HTTP/1.1 403 Forbidden\r\nConnection: close\r\n\r\n";

static std::string getPeerIP(const int &sock);
void handleRequest(int sock, RPCConnection *rpc, std::string peerIP, bool isSpark);
size_t sendMessage(int sock, const std::string message, bool encryptMsg = false, std::string ip = "");
std::string readMessage(int sock, bool decryptMsg = false, std::string ip = "");
bool authenticateHeader(const std::string str, int sock, bool isSpark, std::string peerIP);
int authenticateData(const std::string data);
void handShakeClient(int sock, std::string peerIP, int keySize);
void blackListPeer(std::string ip);

struct PeerInfo
{
	int infractions = 0;
	int version = -1;
	CryptoPP::SecByteBlock key;
};

std::map<std::string, PeerInfo> peers;

Server::Server(const bool sparkServer) : sparkServer(sparkServer)
{
	this->rpc = new RPCConnection(config.getHost(), config.getRPCAuth());

	if (sparkServer)
	{
		// Generate key pair
		CryptoPP::OID CURVE = CryptoPP::ASN1::secp256k1();
		CryptoPP::AutoSeededX917RNG<CryptoPP::AES> rng;

		CryptoPP::ECDH<CryptoPP::ECP>::Domain dh(CURVE);
		CryptoPP::SecByteBlock priv(dh.PrivateKeyLength()), pub(dh.PublicKeyLength());
		dh.GenerateKeyPair(rng, priv, pub);

		config.setPrivkey(priv);
		config.setPubkey(pub);
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

	logInfo("Lightning Rod ready to accept " + serverType + " connections!");

	while (this->running)
	{
		newSock = accept(sock, (struct sockaddr *)&cli_addr, &c);

		if (newSock < 0)
		{
			logError("Error accepting connection (" + serverType + ")");
			continue;
		}

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
			logInfo("New peer! (" + serverType + ")");
			logDebug("Peer IP: " + peerIP);
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
		inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
	}
	else // IPv6
	{
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
		inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof(ipstr));
	}

	std::string ip(ipstr);

	return ip;
}

void handleRequest(int sock, RPCConnection *rpc, std::string peerIP, bool isSpark)
{
	std::string message = readMessage(sock, isSpark, peerIP);

	if (message.empty())
	{
		logError("Error getting message data from " + peerIP);

		sendMessage(sock, HTTP_400, isSpark, peerIP);

		close(sock);
		return;
	}
	else if (message == "rekeyed")
	{
		return;
	}

	if (!authenticateHeader(message, sock, isSpark, peerIP))
		return;

	int pos = message.find("{\"");

	if (pos == std::string::npos)
	{
		logError("Error getting message data from " + peerIP);

		sendMessage(sock, HTTP_400, isSpark, peerIP);

		close(sock);
		return;
	}

	std::string data = message.substr(pos);

	int authData = authenticateData(data);

	// Disallowed commands attempted
	if (authData != 0)
	{
		int f = data.find("\"method\":\"") + 10;
		std::string cmd = data.substr(f, data.find("\"", f) - f);

		if (authData == -1)
		{
			logWarning("Peer (" + peerIP + ") attempted non-whitelisted command (" + cmd + ")");
		}
		else if (authData == 1)
		{
			logWarning("Peer (" + peerIP + ") attempted blacklisted command (" + cmd + ")");
		}

		sendMessage(sock, HTTP_403, isSpark, peerIP);

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

	std::string header = HTTP_200 + "Content-Length: " + std::to_string(result.length()) + "\r\n\r\n";

	std::string sendString = header + result;

	size_t sent = sendMessage(sock, sendString, isSpark, peerIP);

	close(sock);
}

std::string readMessage(int sock, bool isSpark, std::string ip)
{
	byte iv[CryptoPP::AES::BLOCKSIZE];
	int payloadLength = 1024;
	if (isSpark)
	{
		read(sock, &payloadLength, sizeof(payloadLength));
		payloadLength = ntohl(payloadLength);

		if (payloadLength < 0)
		{
			handShakeClient(sock, ip, -payloadLength);

			return "rekeyed";
		}
		read(sock, iv, sizeof(iv));
	}

	char buffer[payloadLength] = {0};
	size_t r = read(sock, buffer, sizeof(buffer));
	if (r < 0)
	{
		logError("Error reading message from " + ip);

		sendMessage(sock, HTTP_400, isSpark, ip);

		close(sock);
		return NULL;
	}

	std::string message(buffer, r);

	// Decrypt message
	if (isSpark)
	{
		if (message.length() <= 32)
		{
			logWarning("Error decrypting message, did not recieve all data");
			return NULL;
		}

		CryptoPP::SecByteBlock vec(iv, sizeof(iv));
		std::string hash = message.substr(0, 32);
		std::string cipher = message.substr(32);
		AESPacket p = {cipher, hash, vec};
		return decrypt(peers.at(ip).key.data(), p);
	}

	return message;
}

size_t sendMessage(int sock, const std::string message, bool encryptMsg, std::string ip)
{
	size_t ret = 0;

	// Encrypt message
	if (encryptMsg && ip.length() > 1)
	{
		AESPacket p = encrypt(peers.at(ip).key.data(), message);
		std::string hashAndCipher = p.hash + p.cipher;
		int payloadLength = hashAndCipher.length();

		ret += send(sock, &payloadLength, sizeof(payloadLength), 0);
		ret += send(sock, p.iv.data(), p.iv.size(), 0);
		ret += send(sock, hashAndCipher.c_str(), hashAndCipher.length(), 0);
	}
	else
	{
		ret += send(sock, message.c_str(), message.length(), 0);
	}

	return ret;
}

void handShakeClient(int sock, std::string peerIP, int keySize)
{
	// Get key and version
	unsigned short version = -1;
	size_t bytes_received = read(sock, &version, sizeof(version));
	if (bytes_received <= 0)
	{
		logError("Error receiving data from client");
		close(sock);
		return;
	}

	version = ntohs(version);

	byte clientPub[keySize];

	read(sock, &clientPub, sizeof(clientPub));

	CryptoPP::SecByteBlock cp(clientPub, keySize);

	peers.at(peerIP).version = version;

	version = htons(0);
	keySize = htonl(config.getPubKey().size());

	// Send public key and version
	send(sock, &keySize, sizeof(keySize), 0);
	send(sock, &version, sizeof(version), 0);
	send(sock, config.getPubKey().data(), config.getPubKey().size(), 0);
	close(sock);

	// Generate session key
	CryptoPP::OID CURVE = CryptoPP::ASN1::secp256k1();
	CryptoPP::ECDH<CryptoPP::ECP>::Domain dh(CURVE);

	CryptoPP::SecByteBlock shared(dh.AgreedValueLength());
	dh.Agree(shared, config.getPrivKey(), cp);

	peers.at(peerIP).key = shared;
}

bool authenticateHeader(const std::string str, int sock, bool isSpark, std::string peerIP)
{
	if (config.hasHttpAuth() && str.find("Authorization: Basic " + config.getHttpAuthEncoded()) == std::string::npos) // Invalid HTTP Credentials
	{
		logWarning("Peer (" + peerIP + ") attempted to connect with invalid credentials");

		sendMessage(sock, HTTP_403, isSpark, peerIP);

		close(sock);

		if (config.getBanThreshold() >= 0 && ++peers.at(peerIP).infractions > config.getBanThreshold())
		{
			blackListPeer(peerIP);
		}

		return false;
	}

	return true;
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

void blackListPeer(std::string ip)
{
	config.blacklistIP(ip);
	logWarning("Peer (" + ip + ") has reached the ban threshold, peer is now blacklisted");

	writeBlacklistedPeer(ip);
}
