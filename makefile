CXX=g++
CXXFLAGS=-w -pthread -DBOOST_LOG_DYN_LINK
LDFLAGS=-lcurl -lboost_filesystem -lboost_system -lboost_log -lboost_thread -lzmq

default: lrod

lrod: src/rod.cpp src/config.cpp src/option.cpp src/rpcconnection.cpp src/server.cpp src/logger.cpp src/zmqserver.cpp src/config.h src/option.h src/rpcconnection.h src/server.h src/logger.h src/zmqserver.h
	$(CXX) $(CXXFLAGS) src/rod.cpp src/option.cpp src/config.cpp src/server.cpp src/rpcconnection.cpp src/logger.cpp src/zmqserver.cpp -o lrod $(LDFLAGS)

#PREFIX is environment variable, but if it is not set, then set default value
ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

install: lrod
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 755 lrod $(DESTDIR)$(PREFIX)/bin/

clean:
	$(RM) lrod
