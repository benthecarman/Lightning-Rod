CC=g++
CFLAGS=-w -pthread
ENDFLAGS=-lcurl

default: rod.o

rod.o: src/rod.cpp
	$(CC) $(CFLAGS) src/rod.cpp src/config.cpp src/server.cpp src/rpcconnection.cpp depends/mongoose.c -o rod.o -I depends $(ENDFLAGS)

test.o: test.cpp
	$(CC) $(CFLAGS) test.cpp src/rpcconnection.cpp -o test.o $(ENDFLAGS)

clean:
	$(RM) *.o
	$(RM) src/*.o