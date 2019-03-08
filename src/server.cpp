#include <string>
#include <cstring>
#include <thread>
#include <vector>
#include <algorithm>

#include "mongoose.h"

#include "rpcconnection.h"
#include "config.h"
#include "server.h"
#include "logger.h"

void ev_handler(struct mg_connection *nc, int ev, void *p, void *r);

std::vector<std::string> peers;

Server::Server()
{
	this->rpc = new RPCConnection(config.getHost(), config.getRPCAuth());
}

bool Server::isRunning()
{
	return this->running;
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
		logInfo("Tests complete");
	}

	struct mg_mgr mgr;
	struct mg_connection *c;

	mg_mgr_init(&mgr, NULL);
	c = mg_bind(&mgr, std::to_string(config.getPort()).c_str(), ev_handler, (void *)this->rpc);
	mg_set_protocol_http_websocket(c);

	this->running = true;

	logInfo("Lightning Rod ready to accept connections!");
	while (this->running)
	{
		mg_mgr_poll(&mgr, 1000);
	}

	logInfo("Shutting down");

	mg_mgr_free(&mgr);
}

static std::string getPeerIP(const sock_t &sock)
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

	std::string *ip = new std::string(ipstr, 0, strlen(ipstr));
	return *ip;
}

void ev_handler(struct mg_connection *con, int ev, void *p, void *r)
{
	if (ev == MG_EV_HTTP_REQUEST)
	{
		std::string peerIP = getPeerIP(con->sock);
		if (std::find(peers.begin(), peers.end(), peerIP) == peers.end())
		{
			peers.push_back(peerIP);
			logInfo("New peer!");
		}

		RPCConnection *rpc = (RPCConnection *)r;
		struct http_message *message = (struct http_message *)p;

		std::string data(message->body.p, message->body.len);
		std::string msg(message->message.p, message->message.len);

		std::string sendString = rpc->execute(data);

		mg_send_head(con, 200, sendString.length(), nullptr);
		mg_printf(con, "%s", sendString.c_str());
		mg_send_http_chunk(con, "", 0);
	}
}