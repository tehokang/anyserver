#include "anyserver.h"
#include <unistd.h>
#include <csignal>
#include <stdio.h>
#include <string>
using namespace std;
using namespace anyserver;

bool g_run = false;

/**
 * @brief Example server application
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv)
{
    /**
     * @note AnyServer has to receive argv[1] that has configuration path
     */
    AnyServer server(argc, argv);

    if ( server.init() && server.start() )
    {
        while ( server.isRun() )
        {
            usleep(100000);
        }
        server.stop();
    }
    printf("Stopped example \n");
    return 0;
}
