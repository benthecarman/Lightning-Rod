#include <string>

#include "rpcconnection.h"
#include "server.h"
#include "config.h"

int main(int argc, char *argv[])
{
	Config cfg = createConfig(argc, argv);

	RPCConnection* rpc = new RPCConnection(cfg.getHost(), "benLinuxPC828:fgCy5QaEgu&C&q$7");

	Server* s = new Server(*rpc, cfg.getPort());

	s->start();

	return 0;
}