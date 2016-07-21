#ifndef __INET_DOMAIN_SOCKET_UDP_SERVER_H__
#define __INET_DOMAIN_SOCKET_UDP_SERVER_H__

#include "anyserver.h"

namespace anyserver
{

class InetDomainSocketUdpServer : public AnyServer
{
public:
    InetDomainSocketUdpServer(
            const string name, const string bind, const unsigned int max_client=200);
    virtual ~InetDomainSocketUdpServer();

    virtual bool init() override;
    virtual bool start() override;
    virtual void stop() override;

protected:
    virtual void __deinit__() override;

private:
    static void* communication_thread(void *argv);
    pthread_t m_communication_thread;

    bool m_run_thread;
    int m_server_fd;
};

} // end of namespace

#endif
