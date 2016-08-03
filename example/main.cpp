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

class AnyServerListener : public IAnyServerListener
{
    virtual void onReceivedSystemSignal(int signal)
    {
        printf("[%s:%s:%d] received signal : %d \n",
                __FILE__, __FUNCTION__, __LINE__, signal);

    }

    virtual void onClientConnected(size_t server_id, size_t client_id)
    {
        printf("[%s:%s:%d] sid : 0x%x, cid : 0x%x\n",
                __FILE__, __FUNCTION__, __LINE__,
                server_id, client_id);
    }

    virtual void onClientDisconnected(size_t server_id, size_t client_id)
    {
        printf("[%s:%s:%d] sid : 0x%x, cid : 0x%x \n",
                __FILE__, __FUNCTION__, __LINE__,
                server_id, client_id);
    }

    virtual void onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len)
    {
        printf("[%s:%s:%d] sid : 0x%x, cid : 0x%x, msg : %s \n",
                __FILE__, __FUNCTION__, __LINE__,
                server_id, client_id, msg);
    }
};

int main(int argc, char **argv)
{
    /**
     * @note AnyServer has to receive argv[1] that has configuration path
     */
    AnyServer server(argc, argv);
    AnyServerListener listener;
    server.addEventListener(&listener);

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