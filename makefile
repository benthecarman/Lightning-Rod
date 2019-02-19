CC=g++
CFLAGS=-w -pthread
ENDFLAGS=-lcurl

default: rod test

rod: src/rod.cpp
	$(CC) $(CFLAGS) src/rod.cpp src/rpcconnection.cpp src/server.cpp -o rod.o $(ENDFLAGS)

test: test.cpp
	$(CC) $(CFLAGS) test.cpp src/rpcconnection.cpp -o test.o $(ENDFLAGS)

clean:
	$(RM) *.o
	$(RM) src/*.o