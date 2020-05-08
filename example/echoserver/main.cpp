#include "anyserver.h"

#include <unistd.h>
#include <csignal>
#include <stdio.h>
#include <string>
#include <string.h>
#include <map>
using namespace std;
using namespace anyserver;

AnyServer *any_server;
AnyServer::BaseServerList server_list;

class AnyServerListener : public IAnyServerListener
{
    map<size_t, size_t> clients;
   
    virtual void onReceivedSystemSignal(int signal)
    {
        printf("[%s:%s:%d] received signal : %d \n",
                __FILE__, __FUNCTION__, __LINE__, signal);
        any_server->stop();
    }

    virtual void onClientConnected(size_t server_id, size_t client_id)
    {
        printf("[%s:%s:%d] sid : 0x%x, cid : 0x%x\n",
                __FILE__, __FUNCTION__, __LINE__,
                (unsigned int)server_id, (unsigned int)client_id);
        clients.insert(make_pair(client_id, server_id));
    }

    virtual void onClientDisconnected(size_t server_id, size_t client_id)
    {
        printf("[%s:%s:%d] sid : 0x%x, cid : 0x%x \n",
                __FILE__, __FUNCTION__, __LINE__,
                (unsigned int)server_id, (unsigned int)client_id);
        clients.erase(client_id);
    }

    virtual void onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len)
    {
        printf("client count : %u \n", clients.size());
        printf("[%s:%s:%d] sid : 0x%x, cid : 0x%x, msg : %s \n",
                __FILE__, __FUNCTION__, __LINE__,
                (unsigned int)server_id, (unsigned int)client_id, msg);
       
        int j=1; 
        printf("----------------------------\n"); 
        printf("%03d : ", j);
        for ( int i=0; i<msg_len; i++) {
            printf("%02x ", msg[i] & 0xff);
            if ( i%10 == 9 && i!=msg_len-1)
                printf("\n%03d : ", ++j);
        }
        printf("\n----------------------------\n"); 

        any_server->sendToClient(server_id, client_id, msg, msg_len);
    }

    virtual void onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len, string protocol)
    {
        printf("[%s:%s:%d] sid : 0x%x, cid : 0x%x, msg : %s, protocol from : %s \n",
                __FILE__, __FUNCTION__, __LINE__,
                (unsigned int)server_id, (unsigned int)client_id, msg, protocol.data());
        string target_protocol = "";

        if ( strcmp(protocol.data(), "sender") == 0 )
        {
            /**
            * @note Sender to Receiver
            */
            target_protocol = "receiver";
        }
        else if ( strcmp(protocol.data(), "receiver") == 0)
        {
            /**
            * @note Receiver to Sender
            */
            target_protocol = "sender";
        }

        for (AnyServer::BaseServerList::iterator it=server_list.begin();
                it!=server_list.end(); ++it)
        {
            AnyServer::BaseServerPtr server = (*it);
            any_server->sendToServer(server->getId(), target_protocol, msg, msg_len);
            printf("server name : %s, id : 0x%x \n", server->getName().data(), (unsigned int)server->getId());
        }
    }
};

int main(int argc, char **argv)
{
    AnyServerListener listener;
    any_server = AnyServer::getInstance(argc, argv);
    any_server->addEventListener(&listener);

    if ( any_server->init() && any_server->start() )
    {
        server_list = any_server->getServerList();
        for (AnyServer::BaseServerList::iterator it=server_list.begin();
                it!=server_list.end(); ++it)
        {
            AnyServer::BaseServerPtr server = (*it);
            printf("server name : %s, id : 0x%x \n",
                    server->getName().data(), (unsigned int)server->getId());
        }

        printf("any_server->isRun : %d \n", any_server->isRun());

        while ( any_server->isRun() )
        {
            usleep(100000);
        }
    }
    printf("Stopped example \n");
    return 0;
}
