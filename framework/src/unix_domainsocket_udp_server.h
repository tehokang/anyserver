#ifndef __UNIX_DOMAIN_SOCKET_UDP_SERVER_H__
#define __UNIX_DOMAIN_SOCKET_UDP_SERVER_H__

#include "anyserver.h"
#include <sys/epoll.h>

namespace anyserver
{

class UnixDomainSocketUdpServer : public AnyServer
{
public:
    UnixDomainSocketUdpServer(
            const string name, const string bind, const unsigned int max_client=200);
    virtual ~UnixDomainSocketUdpServer();

protected:
    virtual bool init() override;
    virtual void __deinit__() override;
    virtual bool start() override;
    virtual void stop() override;

private:
    /**
     * @note http://www.joinc.co.kr/w/Site/Network_Programing/AdvancedComm/epoll24
     */
    static void* communication_thread(void *argv);
    pthread_t m_communication_thread;

    bool m_run_thread;
    int m_server_fd;
};

} // end of namespace

#endif
