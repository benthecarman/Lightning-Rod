#include <string>
#include <vector>
#include <thread>
#include <cstring>

#include <unistd.h>

#include "server.h"
#include "logger.h"
#include "zmqserver.h"
#include "config.h"

void rpcServer()
{
	Server *s = new Server();
	s->start();
}

void rawBlockZMQServer()
{
	ZMQServer *zmq = new ZMQServer("rawblock", config.getZMQBlockHost(), config.getZMQBlockPort());
	zmq->start();
}

void rawTxZMQServer()
{
	ZMQServer *zmq = new ZMQServer("rawtx", config.getZMQTxHost(), config.getZMQTxPort());
	zmq->start();
}

int main(int argc, char *argv[])
{
	if (argc == 2 && (strcmp("--help", argv[1]) == 0 || strcmp("-h", argv[1]) == 0))
	{
		// TODO Help interface
		return 0;
	}

	createConfig(argc, argv);
    initLogger();

	if (config.isDaemon())
	{
		if (fork() == 0)
		{
		}
	}
	else
	{
		std::thread rpcThread(rpcServer);
		rpcThread.detach();

		if (config.getZMQEnabled())
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
	}

	return 0;
}