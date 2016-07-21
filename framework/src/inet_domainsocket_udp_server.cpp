#include "inet_domainsocket_udp_server.h"
#include "anymacro.h"

#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

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
    __deinit__();
}

bool InetDomainSocketUdpServer::init()
{
    LOG_DEBUG("\n");

    struct sockaddr_in serveraddr;

    memset((char *) &serveraddr, 0x00, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)stoi(m_bind));

    m_server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    int optval = 1;
    setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR,
           (const void *)&optval , sizeof(int));

    if ( 0 > m_server_fd
            || 0 > bind(m_server_fd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) )
    {
        perror("socket/bind error ");
        return false;
    }
    return true;
}

void InetDomainSocketUdpServer::__deinit__()
{
    LOG_DEBUG("\n");
    stop();
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
    size_t &server_id = server->m_server_id;
    unsigned int clientlen = sizeof(clientaddr);

    while ( server->m_run_thread )
    {
        memset(buffer, 0x00, sizeof(buffer));
        int readn = recvfrom(server_fd, buffer, sizeof(buffer), 0,
                (struct sockaddr *) &clientaddr, &clientlen);
        if (readn < 0) perror("ERROR in recvfrom");

        struct hostent *hostp = gethostbyaddr(
                (const char *)&clientaddr.sin_addr.s_addr,
                sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        if (hostp == NULL) LOG_WARNING("ERROR on gethostbyaddr");

        char *hostaddrp = inet_ntoa(clientaddr.sin_addr);
        if (hostaddrp == NULL) LOG_WARNING("ERROR on inet_ntoa\n");

        size_t client_id = server->addClientInfo(ClientInfoPtr(new UdpClientInfo(&clientaddr)));
        list<IAnyServerListener*> listeners = server->m_listeners;
        for ( list<IAnyServerListener*>::iterator it = listeners.begin();
                it!=listeners.end(); ++it )
        {
            IAnyServerListener *listener = (*it);
            listener->onClientConnected(server_id, client_id);
        }

        LOG_DEBUG("server[0x%x] received %d bytes: %s from client[0x%x] %s (%s) \n",
                server_id, readn, buffer, client_id, hostp->h_name, hostaddrp);

        for ( list<IAnyServerListener*>::iterator it = listeners.begin();
                it!=listeners.end(); ++it )
        {
            IAnyServerListener *listener = (*it);
            listener->onReceived(server_id, client_id, buffer, readn);
        }
#ifdef CONFIG_TEST_ECHO_RESPONSE
        /**
         * Test echo
         */
        ClientInfoPtr client = server->findClientInfo(client_id);
        if ( 0 > sendto(server_fd, buffer, strlen(buffer), 0,
                (struct sockaddr *) &clientaddr, clientlen) )
        {
            perror("ERROR in sendto ");
        }

        client_id = server->removeClientInfo(client_id);
        for ( list<IAnyServerListener*>::iterator it = listeners.begin();
                it!=listeners.end(); ++it )
        {
            IAnyServerListener *listener = (*it);
            listener->onClientDisconnected(server_id, client_id);
        }
#endif
    }
    close(server_fd);
    return nullptr;
}

}
