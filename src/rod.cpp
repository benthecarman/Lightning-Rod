#include <string>

#include "rpcconnection.h"
#include "server.h"

int main(int argc, char *argv[])
{
	RPCConnection rpc("http://127.0.0.1:8332/", "benLinuxPC828:fgCy5QaEgu&C&q$7");

	Server s(rpc, 8331);

	s.start();

	return 0;
}