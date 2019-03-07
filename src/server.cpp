#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>

#include <string>
#include <cstring>
#include <thread>

#include "mongoose.h"

#include "rpcconnection.h"
#include "config.h"
#include "server.h"

static void ev_handler(struct mg_connection *nc, int ev, void *p, void *r);

Server::Server(Config c) : cfg(c)
{
	this->rpc = new RPCConnection(this->cfg.getHost(), this->cfg.getRPCAuth());
}

bool Server::isRunning()
{
	return this->running;
}

void Server::start()
{
	printf("Testing connection\n");
	std::string test = this->rpc->execute();

	// TODO check for RPC warming up
	if (test.find("{\"result\":[],\"error\":null,\"id\":\"test\"}") != 0)
	{
		printf("Initial RPC Test failed, bitcoin-cli may not be running or your lightning rod is configured incorrectly.\n");
		if (this->cfg.isDebug())
			printf("%s\n", test.c_str());
		exit(1);
	}
	else
	{
		printf("Tests complete\n\n");
	}

	struct mg_mgr mgr;
	struct mg_connection *c;

	mg_mgr_init(&mgr, NULL);
	c = mg_bind(&mgr, std::to_string(this->cfg.getPort()).c_str(), ev_handler, (void *)this->rpc);
	mg_set_protocol_http_websocket(c);

	this->running = true;

	while (this->running)
	{
		mg_mgr_poll(&mgr, 1000);
	}

	mg_mgr_free(&mgr);
}

static void ev_handler(struct mg_connection *nc, int ev, void *p, void *r)
{
	if (ev == MG_EV_HTTP_REQUEST)
	{
		printf("Request recieved\n");

		RPCConnection *rpc = (RPCConnection *)r;
		struct http_message *message = (struct http_message *)p;

		std::string data(message->body.p, message->body.len);
		std::string msg(message->message.p, message->message.len);

		std::string sendString = rpc->execute(data);

		mg_send_head(nc, 200, sendString.length(), nullptr);
		mg_printf(nc, "%s", sendString.c_str());
		mg_send_http_chunk(nc, "", 0);
	}
}