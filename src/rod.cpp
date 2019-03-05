#include <string>
#include <thread>

#include <unistd.h>

#include "server.h"
#include "config.h"

int main(int argc, char *argv[])
{
	Config cfg = createConfig(argc, argv);

	Server *s = new Server(cfg);

	if (cfg.isDaemon())
	{
		if (fork() == 0)
			s->start();
	}
	else
		s->start();

	return 0;
}