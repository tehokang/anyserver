#ifndef __UNIX_DOMAIN_SOCKET_UDP_SERVER_H__
#define __UNIX_DOMAIN_SOCKET_UDP_SERVER_H__

#include "base_server.h"
#include <sys/epoll.h>

namespace anyserver
{

class UnixDomainSocketUdpServer : public BaseServer
{
public:
    UnixDomainSocketUdpServer(
            const string name, const string bind,
            const bool tcp, const unsigned int max_client=200);
    virtual ~UnixDomainSocketUdpServer();

    virtual bool init() override;
    virtual bool start() override;
    virtual void stop() override;
    virtual bool sendToClient(size_t client_id, char *msg, unsigned int msg_len) override;

    class UnixUdpClientInfo : public UdpClientInfo
    {
    public:
        UnixUdpClientInfo(struct sockaddr_in* sockaddr)
            : UdpClientInfo(sockaddr)
        {
        }

        UnixUdpClientInfo(struct sockaddr_un* sockaddr)
            : UdpClientInfo(sockaddr)
        {
        }
    };

protected:
    virtual void __deinit__() override;

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
