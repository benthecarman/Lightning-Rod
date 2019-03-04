#include <string>
#include <thread>

#include "rpcconnection.h"
#include "server.h"
#include "config.h"

void daemon(Server *s)
{
	s->start();
}

int main(int argc, char *argv[])
{
	Config cfg = createConfig(argc, argv);

	Server *s = new Server(cfg);

	if (cfg.isDaemon())
	{
		std::thread t(daemon, s);
		t.detach();
		pthread_exit(NULL);
	}
	else
		s->start();

	return 0;
}