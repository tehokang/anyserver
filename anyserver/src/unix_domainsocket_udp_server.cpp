#include "unix_domainsocket_udp_server.h"
#include "macro.h"

#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace anyserver
{

UnixDomainSocketUdpServer::UnixDomainSocketUdpServer(
        const string name, const string bind, const bool tcp, const unsigned int max_client)
    : BaseServer(name, bind, tcp, max_client)
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

    memset((char *) &serveraddr, 0x00, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    strcpy(serveraddr.sun_path, m_bind.data());

    m_server_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if ( 0 > m_server_fd
            || 0 > bind(m_server_fd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) )
    {
        perror("socket/bind error ");
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

bool UnixDomainSocketUdpServer::sendToClient(size_t client_id, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("\n");
    auto client = static_pointer_cast<UdpClientInfo>(findClientInfo(client_id));
    if ( 0 > sendto(m_server_fd, msg, msg_len, 0,
            (struct sockaddr*)client->getSockAddrUn(), sizeof(struct sockaddr_un)) )
    {
        LOG_ERROR("Failed to send a message \n");
        return false;
    }
    return true;
}

void* UnixDomainSocketUdpServer::communication_thread(void *arg)
{
    LOG_DEBUG("\n");

    UnixDomainSocketUdpServer *server = static_cast<UnixDomainSocketUdpServer*>(arg);
    server->m_run_thread = true;

    struct sockaddr_un clientaddr;
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
        if (readn < 0) perror("ERROR in recvfrom \n");

        size_t client_id = server->addClientInfo(ClientInfoPtr(new UdpClientInfo(&clientaddr)));

        NOTIFY_CLIENT_CONNECTED(server_id, client_id);

        LOG_DEBUG("server[0x%x] received %d bytes: %s from client[0x%x] %s  \n",
                server_id, readn, buffer, client_id, clientaddr.sun_path);
        NOTIFY_SERVER_RECEIVED(server_id, client_id, buffer, readn);

        client_id = server->removeClientInfo(client_id);
        NOTIFY_CLIENT_DISCONNECTED(server_id, client_id);

    }
    close(server_fd);
    return nullptr;
}

}
