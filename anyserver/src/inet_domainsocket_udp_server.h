#ifndef __INET_DOMAIN_SOCKET_UDP_SERVER_H__
#define __INET_DOMAIN_SOCKET_UDP_SERVER_H__

#include "base_server_impl.h"

namespace anyserver
{

class InetDomainSocketUdpServer : public BaseServerImpl
{
public:
    InetDomainSocketUdpServer(
            const string name, const string bind,
            const bool tcp, const unsigned int max_client=200);
    virtual ~InetDomainSocketUdpServer();

    virtual bool init() override;
    virtual bool start() override;
    virtual void stop() override;
    virtual bool sendToClient(size_t client_id, char *msg, unsigned int msg_len) override;

protected:
    virtual void __deinit__() override;

private:
    static void* __communication_thread__(void *arg);
    pthread_t m_communication_thread;

    int m_server_fd;
};

} // end of namespace

#endif
