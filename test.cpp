#include <string>

#include "src/rpcconnection.h"

int main(int argc, char *argv[])
{
    RPCConnection rpc("http://127.0.0.1:8331/", "benLinuxPC828:fgCy5QaEgu&C&q$7");

    std::string rev = rpc.execute();

    printf("%s\n", rev.c_str());

    return 0;
}