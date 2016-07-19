#include "unix_domainsocket_tcp_server.h"
#include "anymacro.h"

#include <unistd.h>
#include <sys/un.h>

#define EPOLL_SIZE      20

namespace anyserver
{

UnixDomainSocketTcpServer::UnixDomainSocketTcpServer(
        const string name, const string bind, const unsigned int max_client)
    : AnyServer(name, bind, max_client)
    , m_events(nullptr)
    , m_run_thread(false)
    , m_epoll_fd(0)
    , m_server_fd(0)
{
    LOG_DEBUG("\n");
}

UnixDomainSocketTcpServer::~UnixDomainSocketTcpServer()
{
    LOG_DEBUG("\n");
    __deinit__();
    SAFE_FREE(m_events);
}

bool UnixDomainSocketTcpServer::init()
{
    LOG_DEBUG("\n");

    m_events = (struct epoll_event *)malloc(sizeof(*m_events) * EPOLL_SIZE);
    if ( 0 > (m_epoll_fd = epoll_create(m_max_client)) )
    {
        perror("epoll_create error");
        return 1;
    }

    if (access(m_bind.data(), F_OK) == 0)
    {
        unlink(m_bind.data());
    }

    m_server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( -1 == m_server_fd )
    {
        perror("socket error :");
        close(m_server_fd);
        return false;
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, m_bind.data());
    if ( -1 == bind (m_server_fd, (struct sockaddr *)&addr, sizeof(addr)) )
    {
        close(m_server_fd);
        return false;
    }

    if ( -1 == listen(m_server_fd, 5) )
    {
        perror("listen error : ");
        return false;
    }

    m_ev.events = EPOLLIN;
    m_ev.data.fd = m_server_fd;
    if ( 0 > epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_server_fd, &m_ev) )
    {
        perror("epoll_ctl, adding listenfd\n");
        return false;
    }

    return true;
}

void UnixDomainSocketTcpServer::__deinit__()
{
    LOG_DEBUG("\n");
    stop();
}

bool UnixDomainSocketTcpServer::start()
{
    LOG_DEBUG("\n");

    if ( 0 != pthread_create(
            &m_epoll_thread, NULL, UnixDomainSocketTcpServer::epoll_thread, (void*)this) )
    {
        LOG_ERROR("Failed to create thread \n");
        return false;
    }
    return true;
}

void UnixDomainSocketTcpServer::stop()
{
    LOG_DEBUG("\n");
    m_run_thread = false;
}

void* UnixDomainSocketTcpServer::epoll_thread(void *argv)
{
    LOG_DEBUG("\n");

    UnixDomainSocketTcpServer *server = static_cast<UnixDomainSocketTcpServer*>(argv);
    server->m_run_thread = true;
    int trigger_count = 0;
    int client_fd = 0;
    struct sockaddr_un clientaddr;
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
        for ( int i = 0; i < trigger_count; i++ )
        {
            if ( events[i].data.fd == server_fd )
            {
                client_fd = accept(server_fd, (struct sockaddr *)&clientaddr, &clilen);
                ev.events = EPOLLIN;
                ev.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

                list<IAnyServerListener*> listeners = server->m_listeners;
                for ( list<IAnyServerListener*>::iterator it = listeners.begin();
                        it!=listeners.end(); ++it )
                {
                    IAnyServerListener *listener = (*it);
                    listener->onClientConnected(client_fd, "localhost", server->m_bind);
                }
            }
            else
            {
                memset(buffer, 0x00, sizeof(buffer));
                int readn = read(events[i].data.fd, buffer, sizeof(buffer));
                if ( 0 >= readn )
                {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, events);
                    close(events[i].data.fd);

                    list<IAnyServerListener*> listeners = server->m_listeners;
                    for ( list<IAnyServerListener*>::iterator it = listeners.begin();
                            it!=listeners.end(); ++it )
                    {
                        IAnyServerListener *listener = (*it);
                        listener->onClientDisconnected(events[i].data.fd);
                    }
                }
                else
                {
                    list<IAnyServerListener*> listeners = server->m_listeners;
                    for ( list<IAnyServerListener*>::iterator it = listeners.begin();
                            it!=listeners.end(); ++it )
                    {
                        IAnyServerListener *listener = (*it);
                        listener->onReceive(events[i].data.fd, buffer, readn);
#ifdef CONFIG_ECHO_RESPONSE
                        /**
                         * Test echo
                         */
                        write(events[i].data.fd, buffer, readn);
#endif
                    }
                }
            }
        }
    }
    return nullptr;
}

}
