#include <string>
#include <thread>
#include <cstring>

#include <unistd.h>

#include "server.h"
#include "config.h"

int main(int argc, char *argv[])
{
	if (argc == 2 && (strcmp("--help", argv[1]) == 0 || strcmp("-h", argv[1]) == 0))
	{
		// TODO Help interface
		return 0;
	}

	createConfig(argc, argv);

	Server *s = new Server();

	if (config.isDaemon())
	{
		if (fork() == 0)
			s->start();
	}
	else
	{
		s->start();
	}

	return 0;
}