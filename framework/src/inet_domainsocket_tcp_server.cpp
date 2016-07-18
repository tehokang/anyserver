#include "inet_domainsocket_tcp_server.h"
#include "anymacro.h"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define EPOLL_SIZE      20

namespace anyserver
{

InetDomainSocketTcpServer::InetDomainSocketTcpServer(
        const string name, const string bind, const unsigned int max_client)
    : AnyServer(name, bind, max_client)
    , m_events(nullptr)
    , m_run_thread(false)
    , m_epoll_fd(0)
    , m_server_fd(0)
{
    LOG_DEBUG("\n");
}

InetDomainSocketTcpServer::~InetDomainSocketTcpServer()
{
    LOG_DEBUG("\n");
    SAFE_FREE(m_events);
}

bool InetDomainSocketTcpServer::init()
{
    LOG_DEBUG("\n");

    struct sockaddr_in addr, clientaddr;
    struct eph_comm *conn;
    int clilen = sizeof(clientaddr);

    m_events = (struct epoll_event *)malloc(sizeof(*m_events) * EPOLL_SIZE);
    if ((m_epoll_fd = epoll_create(m_max_client)) < 0)
    {
        perror("epoll_create error");
        return 1;
    }

    m_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_server_fd == -1)
    {
        perror("socket error :");
        close(m_server_fd);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(stoi(m_bind));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind (m_server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        close(m_server_fd);
        return false;
    }
    listen(m_server_fd, 5);

    m_ev.events = EPOLLIN;
    m_ev.data.fd = m_server_fd;
    if ( 0 > epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_server_fd, &m_ev) )
    {
        perror("epoll_ctl, adding listenfd\n");
        return false;
    }

    return true;
}

void InetDomainSocketTcpServer::__deinit__()
{
    int status = 0;
    pthread_join(m_epoll_thread, reinterpret_cast<void **>(&status));
}

bool InetDomainSocketTcpServer::start()
{
    LOG_DEBUG("\n");

    if ( 0 != pthread_create(
            &m_epoll_thread, NULL, InetDomainSocketTcpServer::epoll_thread, (void*)this) )
    {
        LOG_ERROR("Failed to create thread \n");
        return false;
    }
    return true;
}

void InetDomainSocketTcpServer::stop()
{
    LOG_DEBUG("\n");
    m_run_thread = false;
}

void* InetDomainSocketTcpServer::epoll_thread(void *argv)
{
    LOG_DEBUG("\n");

    InetDomainSocketTcpServer *server = static_cast<InetDomainSocketTcpServer*>(argv);
    server->m_run_thread = true;
    int trigger_count = 0;
    int client_fd = 0;
    struct sockaddr_in clientaddr;
    unsigned int clilen = sizeof(clientaddr);

    struct epoll_event &ev = server->m_ev;
    struct epoll_event *events = server->m_events;
    int &epoll_fd = server->m_epoll_fd;
    int &server_fd = server->m_server_fd;

    enum { BUFFER_LENGTH = 2048 };
    char buffer[BUFFER_LENGTH] = {0, };

    while ( server->m_run_thread )
    {
        trigger_count = epoll_wait(epoll_fd, events, EPOLL_SIZE, -1);
        if (trigger_count)
        {
            perror("epoll wait error");
        }

        for (int i = 0; i < trigger_count; i++)
        {
            if ( events[i].data.fd == server_fd )
            {
                printf("Accept\n");
                client_fd = accept(server_fd, (struct sockaddr *)&clientaddr, &clilen);
                ev.events = EPOLLIN;
                ev.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

                list<IAnyServerListener*> listeners = server->m_listeners;
                for ( list<IAnyServerListener*>::iterator it = listeners.begin();
                        it!=listeners.end(); ++it )
                {
                    IAnyServerListener *listener = (*it);
                    listener->onClientConnected(client_fd,
                            inet_ntoa(clientaddr.sin_addr),
                            clientaddr.sin_port);
                }
            }
            else
            {
                memset(buffer, 0x00, BUFFER_LENGTH);
                int readn = read(events[i].data.fd, buffer, BUFFER_LENGTH);
                if (readn <= 0)
                {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, events);
                    close(events[i].data.fd);

                    list<IAnyServerListener*> listeners = server->m_listeners;
                    for ( list<IAnyServerListener*>::iterator it = listeners.begin();
                            it!=listeners.end(); ++it )
                    {
                        IAnyServerListener *listener = (*it);
                        listener->onClientDisconnected(client_fd);
                    }
                }
                else
                {
                    printf("read data %s\n", buffer);
                    write(events[i].data.fd, buffer, readn);
                }
            }
        }
    }
    return nullptr;
}

}
