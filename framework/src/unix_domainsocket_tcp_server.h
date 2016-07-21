#ifndef __UNIX_DOMAIN_SOCKET_TCP_SERVER_H__
#define __UNIX_DOMAIN_SOCKET_TCP_SERVER_H__

#include "anyserver.h"
#include <sys/epoll.h>

namespace anyserver
{

class UnixDomainSocketTcpServer : public AnyServer
{
public:
    UnixDomainSocketTcpServer(
            const string name, const string bind, const unsigned int max_client=200);
    virtual ~UnixDomainSocketTcpServer();

    virtual bool init() override;
    virtual bool start() override;
    virtual void stop() override;
protected:
    virtual void __deinit__() override;

private:
    /**
     * @note http://www.joinc.co.kr/w/Site/Network_Programing/AdvancedComm/epoll24
     */
    static void* epoll_thread(void *argv);
    pthread_t m_epoll_thread;

    bool m_run_thread;
    struct epoll_event m_ev;
    struct epoll_event *m_events;
    int m_epoll_fd;
    int m_server_fd;
};

} // end of namespace

#endif
