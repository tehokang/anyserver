#ifndef __INET_DOMAIN_SOCKET_SERVER_H__
#define __INET_DOMAIN_SOCKET_SERVER_H__

#include "anyserver.h"
#include <sys/epoll.h>

namespace anyserver
{

class InetDomainSocketServer : public AnyServer
{
public:
    InetDomainSocketServer(
            const string name, const string bind, const unsigned int max_client=200);
    virtual ~InetDomainSocketServer();

protected:
    virtual bool init() override;
    virtual void __deinit__() override;
    virtual bool start() override;
    virtual void stop() override;

private:
    static void* epoll_thread(void *argv);
    pthread_t _epoll_thread_;

    bool m_run_thread;
    struct epoll_event m_ev;
    struct epoll_event *m_events;
    int m_epoll_fd;
    int m_server_fd;
};

} // end of namespace

#endif
