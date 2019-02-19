#include <string>
#include <cstring>

#include "src/rpcconnection.h"

int main(int argc, char *argv[])
{
    std::string port = "8331";
    if (argc > 1)
    {
        int i;
        for (i = 1; i < argc; ++i)
        {
            std::string tmp(argv[i], 0, strlen(argv[i]));

            if (tmp.find("--port=") == 0)
            {
                int pos = tmp.find("--port=");
                port = tmp.substr(pos + 7);
            }
        }
    }

    RPCConnection rpc("http://127.0.0.1:" + port + "/", "benLinuxPC828:fgCy5QaEgu&C&q$7");

    std::string rev = rpc.execute();

    printf("%s\n", rev.c_str());

    return 0;
}