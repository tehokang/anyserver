#include "inet_domainsocket_server.h"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define EPOLL_SIZE      20

namespace anyserver
{

InetDomainSocketServer::InetDomainSocketServer(
        const string name, const string bind, const unsigned int max_client)
    : AnyServer(name, bind, max_client)
{

}

InetDomainSocketServer::~InetDomainSocketServer()
{

}

bool InetDomainSocketServer::init()
{
    struct sockaddr_in addr, clientaddr;
    struct eph_comm *conn;
    int sfd;
    int cfd;
    int clilen;
    int flags = 1;
    int n, i;
    int readn;
    struct epoll_event ev,*events;

    int efd;
    char buf_in[256];

    events = (struct epoll_event *)malloc(sizeof(*events) * EPOLL_SIZE);
    if ((efd = epoll_create(100)) < 0)
    {
        perror("epoll_create error");
        return 1;
    }

    clilen = sizeof(clientaddr);
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1)
    {
        perror("socket error :");
        close(sfd);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(stoi(bind));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind (sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        close(sfd);
        return false;
    }
    listen(sfd, 5);

    ev.events = EPOLLIN;
    ev.data.fd = sfd;
    epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &ev);

    return true;
}

void InetDomainSocketServer::deinit()
{

}

bool InetDomainSocketServer::start()
{
    return true;
}

void InetDomainSocketServer::stop()
{

}

}
