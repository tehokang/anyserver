#include "unix_domainsocket_udp_server.h"
#include "anymacro.h"

#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace anyserver
{

UnixDomainSocketUdpServer::UnixDomainSocketUdpServer(
        const string name, const string bind, const unsigned int max_client)
    : AnyServer(name, bind, max_client)
    , m_run_thread(false)
    , m_server_fd(0)
{
    LOG_DEBUG("\n");
}

UnixDomainSocketUdpServer::~UnixDomainSocketUdpServer()
{
    LOG_DEBUG("\n");
    __deinit__();
}

bool UnixDomainSocketUdpServer::init()
{
    LOG_DEBUG("\n");

    struct sockaddr_un serveraddr;
    int optval = 1;

    if ( 0 == access(m_bind.data(), F_OK) )
    {
        unlink(m_bind.data());
    }

    m_server_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if ( m_server_fd < 0 )
    {
        LOG_WARNING("ERROR opening socket");
        return false;
    }

    memset((char *) &serveraddr, 0x00, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    strcpy(serveraddr.sun_path, m_bind.data());

    if ( 0 > bind(m_server_fd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) )
    {
        LOG_WARNING("ERROR on binding");
        return false;
    }
    return true;
}

void UnixDomainSocketUdpServer::__deinit__()
{
    LOG_DEBUG("\n");
    stop();
}

bool UnixDomainSocketUdpServer::start()
{
    LOG_DEBUG("\n");

    if ( 0 != pthread_create(
            &m_communication_thread, NULL, UnixDomainSocketUdpServer::communication_thread, (void*)this) )
    {
        LOG_ERROR("Failed to create thread \n");
        return false;
    }
    return true;
}

void UnixDomainSocketUdpServer::stop()
{
    LOG_DEBUG("\n");
    m_run_thread = false;
}

void* UnixDomainSocketUdpServer::communication_thread(void *argv)
{
    LOG_DEBUG("\n");

    UnixDomainSocketUdpServer *server = static_cast<UnixDomainSocketUdpServer*>(argv);
    server->m_run_thread = true;

    struct sockaddr_un clientaddr;
    enum { BUFFER_LENGTH = 2048 };
    char buffer[BUFFER_LENGTH] = {0, };
    int &server_fd = server->m_server_fd;
    unsigned int clientlen = sizeof(clientaddr);

    while ( server->m_run_thread )
    {
        memset(buffer, 0x00, sizeof(buffer));
        int readn = recvfrom(server_fd, buffer, sizeof(buffer), 0,
                (struct sockaddr *) &clientaddr, &clientlen);
        if (readn < 0) LOG_WARNING("ERROR in recvfrom \n");

        printf( "client sun_path : %s\n", clientaddr.sun_path);

        list<IAnyServerListener*> listeners = server->m_listeners;
        for ( list<IAnyServerListener*>::iterator it = listeners.begin();
                it!=listeners.end(); ++it )
        {
            IAnyServerListener *listener = (*it);
            listener->onClientConnected(server_fd, "localhost", server->m_bind);
        }

        LOG_DEBUG("server received %d/%d bytes: %s\n", strlen(buffer), readn, buffer);

        for ( list<IAnyServerListener*>::iterator it = listeners.begin();
                it!=listeners.end(); ++it )
        {
            IAnyServerListener *listener = (*it);
            listener->onReceive(server_fd, (struct sockaddr*)&clientaddr, buffer, readn);
#ifdef CONFIG_ECHO_RESPONSE
            /**
             * Test echo
             */
            if ( 0 > sendto(server_fd, buffer, strlen(buffer), 0,
                    (struct sockaddr *) &clientaddr, clientlen) )
            {
                LOG_WARNING("ERROR in sendto \n");
                perror("fail to sendto ");
            }
#endif
        }

        for ( list<IAnyServerListener*>::iterator it = listeners.begin();
                it!=listeners.end(); ++it )
        {
            IAnyServerListener *listener = (*it);
            listener->onClientDisconnected(server_fd);
        }
    }
    close(server_fd);
    return nullptr;
}

}
