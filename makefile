CXX=@g++
CXXFLAGS=-w -pthread -DBOOST_LOG_DYN_LINK
LDFLAGS=-lcurl -lboost_filesystem -lboost_system -lboost_log -lboost_thread -lzmq

default: lrod spark

lrod: src/rod.cpp src/config.cpp src/option.cpp src/rpcconnection.cpp src/server.cpp src/logger.cpp src/base64.cpp src/zmqserver.cpp src/config.h src/option.h src/rpcconnection.h src/server.h src/logger.h src/base64.h src/zmqserver.h
	@echo CXX src/lrod.cpp
	$(CXX) $(CXXFLAGS) src/rod.cpp src/option.cpp src/config.cpp src/server.cpp src/rpcconnection.cpp src/logger.cpp src/base64.cpp src/zmqserver.cpp -o lrod $(LDFLAGS)

spark: src/spark.cpp src/client.cpp src/config.cpp src/option.cpp src/rpcconnection.cpp src/logger.cpp src/base64.cpp src/config.h src/client.h src/option.h src/rpcconnection.h src/logger.h src/base64.h
	@echo CXX src/spark.cpp
	$(CXX) $(CXXFLAGS) src/spark.cpp src/client.cpp src/option.cpp src/config.cpp src/rpcconnection.cpp src/logger.cpp src/base64.cpp -o spark $(LDFLAGS)

#PREFIX is environment variable, but if it is not set, then set default value
ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

install: lrod spark
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 755 lrod $(DESTDIR)$(PREFIX)/bin/
	install -m 755 spark $(DESTDIR)$(PREFIX)/bin/

clean:
	$(RM) lrod
	$(RM) spark
