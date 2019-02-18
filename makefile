CC=g++
CFLAGS=-w -pthread

default: test

test: test.cpp
	$(CC) $(CFLAGS) test.cpp -o test.o

clean:
	$(RM) *.o