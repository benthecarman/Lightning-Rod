CC=g++
CFLAGS=-w -pthread
ENDFLAGS=-lcurl

default: shelter

shelter: src/shelter.cpp
	$(CC) $(CFLAGS) src/shelter.cpp src/rpcconnection.cpp -o src/shelter.o $(ENDFLAGS)

clean:
	$(RM) *.o
	$(RM) src/*.o