#include "inet_domainsocket_udp_server.h"
#include "anymacro.h"

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace anyserver
{

InetDomainSocketUdpServer::InetDomainSocketUdpServer(
        const string name, const string bind, const unsigned int max_client)
    : AnyServer(name, bind, max_client)
    , m_run_thread(false)
    , m_server_fd(0)
{
    LOG_DEBUG("\n");
}

InetDomainSocketUdpServer::~InetDomainSocketUdpServer()
{
    LOG_DEBUG("\n");
}

bool InetDomainSocketUdpServer::init()
{
    LOG_DEBUG("\n");

    struct sockaddr_in serveraddr;
    int optval = 1;

    m_server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_server_fd < 0)
    {
        LOG_WARNING("ERROR opening socket");
        return false;
    }

    setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR,
           (const void *)&optval , sizeof(int));

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)stoi(m_bind));

    if (bind(m_server_fd, (struct sockaddr *) &serveraddr,
         sizeof(serveraddr)) < 0)
    {
        LOG_WARNING("ERROR on binding");
        return false;
    }

    return true;
}

void InetDomainSocketUdpServer::__deinit__()
{
    LOG_DEBUG("\n");
}

bool InetDomainSocketUdpServer::start()
{
    LOG_DEBUG("\n");

    if ( 0 != pthread_create(
            &m_communication_thread, NULL, InetDomainSocketUdpServer::communication_thread, (void*)this) )
    {
        LOG_ERROR("Failed to create thread \n");
        return false;
    }
    return true;
}

void InetDomainSocketUdpServer::stop()
{
    LOG_DEBUG("\n");
    m_run_thread = false;
}

void* InetDomainSocketUdpServer::communication_thread(void *argv)
{
    LOG_DEBUG("\n");

    InetDomainSocketUdpServer *server = static_cast<InetDomainSocketUdpServer*>(argv);
    server->m_run_thread = true;

    struct sockaddr_in clientaddr;
    enum { BUFFER_LENGTH = 2048 };
    char buffer[BUFFER_LENGTH] = {0, };
    int &server_fd = server->m_server_fd;
    unsigned int clientlen = sizeof(clientaddr);

    while ( server->m_run_thread )
    {
        bzero(buffer, sizeof(buffer));
        int readn = recvfrom(server_fd, buffer, sizeof(buffer), 0,
                (struct sockaddr *) &clientaddr, &clientlen);
        if (readn < 0) LOG_WARNING("ERROR in recvfrom");

        struct hostent *hostp = gethostbyaddr(
                (const char *)&clientaddr.sin_addr.s_addr,
                sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        if (hostp == NULL) LOG_WARNING("ERROR on gethostbyaddr");

        char *hostaddrp = inet_ntoa(clientaddr.sin_addr);
        if (hostaddrp == NULL) LOG_WARNING("ERROR on inet_ntoa\n");

        LOG_DEBUG("server received datagram from %s (%s)\n", hostp->h_name, hostaddrp);
        LOG_DEBUG("server received %d/%d bytes: %s\n", strlen(buffer), readn, buffer);

        list<IAnyServerListener*> listeners = server->m_listeners;
        for ( list<IAnyServerListener*>::iterator it = listeners.begin();
                it!=listeners.end(); ++it )
        {
            IAnyServerListener *listener = (*it);
            listener->onClientConnected(server_fd,
                    inet_ntoa(clientaddr.sin_addr),
                    clientaddr.sin_port);
        }

        for ( list<IAnyServerListener*>::iterator it = listeners.begin();
                it!=listeners.end(); ++it )
        {
            IAnyServerListener *listener = (*it);
            listener->onReceive(server_fd, (struct sockaddr*)&clientaddr, buffer, readn);
        }
#ifdef CONFIG_ECHO_RESPONSE
        /**
         * Test echo
         */
        if ( 0 > sendto(server_fd, buffer, strlen(buffer), 0,
                (struct sockaddr *) &clientaddr, clientlen) )
        {
            LOG_WARNING("ERROR in sendto");
        }
#endif
    }
    return nullptr;
}

}
