#include <string>

#include "rpcconnection.h"

int main(int argc, char *argv[])
{
    RPCConnection rpc("http://127.0.0.1:8332/", "benLinuxPC828:fgCy5QaEgu&C&q$7");

    printf("%s\n", rpc.execute("getbestblockhash", "[]").c_str());

    return 0;
}