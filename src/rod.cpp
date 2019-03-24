#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#define sleep(n) Sleep(n)
#else
#include <unistd.h>
#include <signal.h>
#endif

#include "server.h"
#include "logger.h"
#include "zmqserver.h"
#include "config.h"
#include "option.h"

Server *serverRPC;
ZMQServer *blockZMQServer;
ZMQServer *txZMQServer;

void sigHandler(int s)
{
	std::cout << std::endl;

	switch (s)
	{
// This could have wrong signals
#ifndef _WIN32
	case SIGABRT:
	case SIGINT:
	case SIGKILL:
	case SIGSTOP:
	case SIGQUIT:
	case SIGTERM:
#else
	case IDABORT:
	case SIF_ALL:
	case MCI_STOP:
	case WM_QUIT:
#endif
		serverRPC->setRunning(false);
		blockZMQServer->setRunning(false);
		txZMQServer->setRunning(false);
		while (!serverRPC->isStopped() && !blockZMQServer->isStopped() && !txZMQServer->isStopped());
		sleep(3);
		exit(1);
	default:
		std::cout << "Caught signal " << s << std::endl;
	}
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
		std::cout << "Lightning Rod allows users to service as a full node for others that cannot run one" << std::endl << std::endl;

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

	logDebug("Current Config:\n\n" + config.toString());

#ifndef _WIN32
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = sigHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);
#else
	signal(WM_QUIT, sigHandler);
#endif

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