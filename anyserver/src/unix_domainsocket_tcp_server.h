#ifndef __UNIX_DOMAIN_SOCKET_TCP_SERVER_H__
#define __UNIX_DOMAIN_SOCKET_TCP_SERVER_H__

#include <sys/epoll.h>
#include "base_server_impl.h"

namespace anyserver
{

class UnixDomainSocketTcpServer : public BaseServerImpl
{
public:
    UnixDomainSocketTcpServer(
            const string name, const string bind,
            const bool tcp, const unsigned int max_client=200);
    virtual ~UnixDomainSocketTcpServer();

    virtual bool init() override;
    virtual bool start() override;
    virtual void stop() override;
    virtual bool sendToClient(size_t client_id, char *msg, unsigned int msg_len) override;

protected:
    virtual void __deinit__() override;

private:
    /**
     * @note http://www.joinc.co.kr/w/Site/Network_Programing/AdvancedComm/epoll24
     */
    static void* __epoll_thread__(void *arg);
    pthread_t m_epoll_thread;

    struct epoll_event m_ev;
    struct epoll_event *m_events;
    int m_epoll_fd;
    int m_server_fd;
};

} // end of namespace

#endif
