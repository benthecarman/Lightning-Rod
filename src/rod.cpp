#include <string>
#include <thread>

#include "rpcconnection.h"
#include "server.h"
#include "config.h"

void startServer(Server *s)
{
	s->start();
}

int main(int argc, char *argv[])
{
	Config cfg = createConfig(argc, argv);

	Server *s = new Server(cfg);

	if (cfg.isDaemon())
	{
		printf("d\n");
		std::thread t(startServer, s);
		t.detach();
		pthread_exit(NULL);
	}
	else
	{
		startServer(s);
	}

	return 0;
}