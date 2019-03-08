CC=g++
CFLAGS=-w -pthread -DBOOST_LOG_DYN_LINK
ENDFLAGS=-lcurl -lboost_filesystem -lboost_system -lboost_log -lboost_thread

default: rod.o

rod.o: src/rod.cpp src/config.cpp src/rpcconnection.cpp src/server.cpp depends/mongoose.c src/logger.cpp
	$(CC) $(CFLAGS) src/rod.cpp src/config.cpp src/server.cpp src/rpcconnection.cpp depends/mongoose.c src/logger.cpp -o rod.o -I depends $(ENDFLAGS)

test: test.cpp
	$(CC) $(CFLAGS) test.cpp src/rpcconnection.cpp -o test.o $(ENDFLAGS)

clean:
	$(RM) *.o
	$(RM) src/*.o