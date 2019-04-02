CC=g++
CFLAGS=-w -pthread -DBOOST_LOG_DYN_LINK
ENDFLAGS=-lcurl -lboost_filesystem -lboost_system -lboost_log -lboost_thread -lzmq

default: rod.o

rod.o: src/rod.cpp src/config.cpp src/option.cpp src/rpcconnection.cpp src/server.cpp src/logger.cpp src/zmqserver.cpp depends/mongoose.cpp src/config.h src/option.h src/rpcconnection.h src/server.h src/logger.h src/zmqserver.h depends/mongoose.hpp
	$(CC) $(CFLAGS) src/rod.cpp src/option.cpp src/config.cpp src/server.cpp src/rpcconnection.cpp src/logger.cpp src/zmqserver.cpp depends/mongoose.cpp -o rod.o -I depends $(ENDFLAGS)

test: test.cpp
	$(CC) $(CFLAGS) test.cpp src/rpcconnection.cpp -o test.o $(ENDFLAGS)

zmqtest: zmqtest.cpp
	$(CC) $(CFLAGS) zmqtest.cpp -o test.o $(ENDFLAGS)

clean:
	$(RM) *.o
	$(RM) src/*.o