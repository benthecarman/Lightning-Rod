#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <thread>
#include <cstring>
#include <cstdlib>

#include <unistd.h>
#include <signal.h>

#include "server.h"
#include "logger.h"
#include "client.h"
#include "config.h"
#include "option.h"

#define RUNNING_DIR_DAEMON "/tmp"

Client *client;

void sigHandler(int s)
{
	std::cout << std::endl;

	switch (s)
	{
	// This could have wrong signals
	case SIGABRT:
	case SIGINT:
	case SIGKILL:
	case SIGSTOP:
	case SIGQUIT:
	case SIGTERM:
		exit(0);
	default:
		logInfo("Caught signal: " + s);
	}
}

// This was taken from internet, can probably be improved
void daemonize()
{
	int i;
	if (getppid() == 1)
		return;
	i = fork();
	if (i < 0)
		exit(1);
	if (i > 0)
		exit(0);

	setsid();
	for (i = getdtablesize(); i >= 0; --i)
		close(i);
	chdir(RUNNING_DIR_DAEMON);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGHUP, sigHandler);
	signal(SIGTERM, sigHandler);
}

void startClient()
{
	client = new Client();
	client->start();
}

int main(int argc, char *argv[])
{
	registerSparkOptions();

	if (argc == 2 && (strcmp("--help", argv[1]) == 0 || strcmp("-h", argv[1]) == 0))
	{
		std::cout << "Usage: lightning rod" << std::endl;
		std::cout << "Lightning Rod allows users to service as a full node for others that cannot run one" << std::endl
				  << std::endl;

		for (auto &opt : options)
		{
			std::string names = "--" + opt.getName();
			if (opt.hasShortcut())
				names += "|-" + opt.getShortcut();
			std::cout << std::setw(25) << std::left << names;
			std::cout << std::setw(40) << opt.getDescription();
			std::cout << std::endl;
		}
		return 0;
	}

	config.setSpark(true);

	createConfig(argc, argv);
	initLogger();

	if (config.isDaemon())
	{
		daemonize();
	}

	logDebug("Current Config:\n\n" + config.toString());

	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = sigHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);

	startClient();

	while (true)
		;

	return 0;
}
