#include "inet_domainsocket_tcp_server.h"
#include "macro.h"

#include <unistd.h>
#include <arpa/inet.h>

#define EPOLL_SIZE      20

namespace anyserver
{

InetDomainSocketTcpServer::InetDomainSocketTcpServer(
        const string name, const string bind, const bool tcp, const unsigned int max_client)
    : BaseServerImpl(name, bind, tcp, max_client)
    , m_events(nullptr)
    , m_epoll_fd(0)
    , m_server_fd(0)
{
    LOG_DEBUG("\n");
}

InetDomainSocketTcpServer::~InetDomainSocketTcpServer()
{
    LOG_DEBUG("\n");
    __deinit__();
    SAFE_FREE(m_events);
}

bool InetDomainSocketTcpServer::init()
{
    LOG_DEBUG("\n");

    m_events = (struct epoll_event *)malloc(sizeof(*m_events) * EPOLL_SIZE);
    if ( 0 > (m_epoll_fd = epoll_create(m_max_client)) )
    {
        perror("epoll_create error");
        return false;
    }

    m_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if ( 0 > m_server_fd )
    {
        perror("socket error ");
        close(m_server_fd);
        return false;
    }

    int optval = 1;
    setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR,
           (const void *)&optval , sizeof(int));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(stoi(m_bind));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if ( -1 == bind (m_server_fd, (struct sockaddr *)&addr, sizeof(addr)) )
    {
        close(m_server_fd);
        perror("bind error ");
        return false;
    }
    listen(m_server_fd, 2*1024);

    m_ev.events = EPOLLIN;
    m_ev.data.fd = m_server_fd;
    if ( 0 > epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_server_fd, &m_ev) )
    {
        perror("epoll_ctl, adding listenfd ");
        return false;
    }

    return true;
}

void InetDomainSocketTcpServer::__deinit__()
{
    LOG_DEBUG("\n");
    stop();
}

bool InetDomainSocketTcpServer::start()
{
    LOG_DEBUG("\n");

    if ( 0 != pthread_create(
            &m_epoll_thread, NULL, InetDomainSocketTcpServer::__epoll_thread__, (void*)this) )
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

bool InetDomainSocketTcpServer::sendToClient(size_t client_id, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("\n");
    auto client = static_pointer_cast<TcpClientInfo>(findClientInfo(client_id));
    if ( -1 == write(client->getFd(), msg, msg_len) )
    {
        return false;
    }
    return true;
}

void* InetDomainSocketTcpServer::__epoll_thread__(void *arg)
{
    LOG_DEBUG("\n");

    InetDomainSocketTcpServer *server = static_cast<InetDomainSocketTcpServer*>(arg);
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
        for ( int i = 0; i < trigger_count; i++ )
        {
            if ( events[i].data.fd == server_fd )
            {
                client_fd = accept(server_fd, (struct sockaddr *)&clientaddr, &clilen);
                ev.events = EPOLLIN;
                ev.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

                size_t client_id = server->addClientInfo(ClientInfoPtr(new TcpClientInfo(client_fd, &clientaddr)));
                NOTIFY_CLIENT_CONNECTED(server->m_server_id, client_id);
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
                    NOTIFY_CLIENT_DISCONNECTED(server->m_server_id, client_id);
                }
                else
                {
                    ClientInfoPtr client = server->findClientInfo(events[i].data.fd);
                    NOTIFY_SERVER_RECEIVED(server->m_server_id, client->getClientId(), buffer, readn);
                }
            }
        }
    }
    close(server_fd);
    return nullptr;
}

}
