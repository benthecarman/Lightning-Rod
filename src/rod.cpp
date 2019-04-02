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

Server *serverRPC;
ZMQServer *blockZMQServer;
ZMQServer *txZMQServer;

void sigHandler(int s)
{
	printf("\n");

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
		while (!serverRPC->isStopped() && !blockZMQServer->isStopped() && !txZMQServer->isStopped());
		sleep(3);
		exit(1);
	default:
		printf("Caught signal %d\n", s);
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
	if (argc == 2 && (strcmp("--help", argv[1]) == 0 || strcmp("-h", argv[1]) == 0))
	{
		// TODO Help interface
		return 0;
	}

	registerOptions();

	createConfig(argc, argv);
	initLogger();

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