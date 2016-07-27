#include "unix_domainsocket_tcp_server.h"
#include "anymacro.h"

#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>

#define EPOLL_SIZE      20

namespace anyserver
{

UnixDomainSocketTcpServer::UnixDomainSocketTcpServer(
        const string name, const string bind, const bool tcp, const unsigned int max_client)
    : AnyServer(name, bind, tcp, max_client)
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
        perror("epoll_create error ");
        return false;
    }

    if (access(m_bind.data(), F_OK) == 0)
    {
        unlink(m_bind.data());
    }

    m_server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( -1 == m_server_fd )
    {
        perror("socket error ");
        close(m_server_fd);
        return false;
    }

    struct sockaddr_un serveraddr;
    serveraddr.sun_family = AF_UNIX;
    strcpy(serveraddr.sun_path, m_bind.data());

    m_ev.events = EPOLLIN;
    m_ev.data.fd = m_server_fd;

    if ( 0 > bind (m_server_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr))
            || 0 > listen(m_server_fd, 5)
            ||  0 > epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_server_fd, &m_ev) )
    {
        perror("bind/listen/epoll_ctl, adding listenfd ");
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

bool UnixDomainSocketTcpServer::sendToClient(size_t client_id, char *msg, unsigned int msg_len)
{
    auto client = static_pointer_cast<UnixTcpClientInfo>(findClientInfo(client_id));
    if ( -1 == write(client->getFd(), msg, msg_len) )
    {
        return false;
    }
    return true;
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
    size_t &server_id = server->m_server_id;

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

                size_t client_id = server->addClientInfo(ClientInfoPtr(new UnixTcpClientInfo(client_fd, &clientaddr)));

                NOTIFY_CLIENT_CONNECTED(server_id, client_id);
            }
            else
            {
                memset(buffer, 0x00, sizeof(buffer));
                int readn = read(events[i].data.fd, buffer, sizeof(buffer));
                if ( 0 >= readn )
                {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, events);
                    close(events[i].data.fd);

                    size_t client_id = server->removeClientInfo(events[i].data.fd);
                    NOTIFY_CLIENT_DISCONNECTED(server_id, client_id);
                }
                else
                {
                    ClientInfoPtr client = server->findClientInfo(events[i].data.fd);
                    NOTIFY_SERVER_RECEIVED(server_id, client->getClientId(), buffer, readn);

#ifdef CONFIG_TEST_ECHO_RESPONSE
                    /**
                     * Test echo
                     */
                    ssize_t written = write(events[i].data.fd, buffer, readn);
                    (void)written;
#endif
                }
            }
        }
    }
    return nullptr;
}

}
