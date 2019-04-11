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
#include "zmqserver.h"
#include "config.h"
#include "option.h"

#define RUNNING_DIR_DAEMON "/tmp"

Server *serverRPC;
ZMQServer *blockZMQServer;
ZMQServer *txZMQServer;

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
		serverRPC->setRunning(false);
		blockZMQServer->setRunning(false);
		txZMQServer->setRunning(false);
		while (!serverRPC->isStopped() && !blockZMQServer->isStopped() && !txZMQServer->isStopped())
			;
		sleep(3);
		exit(1);
	default:
		logInfo("Caught signal: " + s);
	}
}

void signal_handler(int sig)
{
	switch (sig)
	{
	case SIGHUP:
		logInfo("hangup signal caught");
		break;
	case SIGTERM:
		logInfo("terminate signal caught");
		exit(0);
		break;
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
	signal(SIGHUP, signal_handler);
	signal(SIGTERM, signal_handler);
}

void rpcServer()
{
	serverRPC = new Server();
	serverRPC->start();
}

void rawBlockZMQServer()
{
	blockZMQServer = new ZMQServer("rawblock", config.getZMQBlockHost(), config.getZMQBlockPort());
	blockZMQServer->start();
}

void rawTxZMQServer()
{
	txZMQServer = new ZMQServer("rawtx", config.getZMQTxHost(), config.getZMQTxPort());
	txZMQServer->start();
}
int main(int argc, char *argv[])
{
	registerOptions();

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

	std::thread rpcThread(rpcServer);
	rpcThread.detach();

	if (!config.getDisableZMQ())
	{
		if (config.isTxZMQValid())
		{
			std::thread zmqTxThread(rawTxZMQServer);
			zmqTxThread.detach();
		}
		if (config.isBlockZMQValid())
		{
			std::thread zmqBlockThread(rawBlockZMQServer);
			zmqBlockThread.detach();
		}
		else if (!config.isTxZMQValid())
		{
			logError("ZMQ Enabled but ZMQ Ports and Hosts are not configured correctly");
		}
	}

	while (true)
		;

	return 0;
}