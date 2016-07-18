#ifndef __ANYSERVER_H__
#define __ANYSERVER_H__

#include <string>
#include <list>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

#define CONFIG_ECHO_RESPONSE

namespace anyserver
{

class IAnyServerListener
{
public:
    /**
     * @brief Tcp client connection/disconnection/receiving
     * @param fd
     * @param ip_address
     * @param port
     */
    virtual void onClientConnected(int fd, string ip_address, int port) = 0;
    virtual void onClientDisconnected(int fd) = 0;
    virtual void onReceive(int fd, char *msg, unsigned int msg_len) = 0;

    /**
     * @brief Udp client receiving (except for connection/disconnection)
     * @param sfd
     * @param ip_address
     * @param port
     */
    virtual void onReceive(int sfd, struct sockaddr *client_addr, char *msg, unsigned int msg_len) = 0;
};

class AnyServer
{
public:
    AnyServer(const string name, const string bind, const unsigned int max_client);
    virtual ~AnyServer();

    void addEventListener(IAnyServerListener *listener);
    void removeEventListener(IAnyServerListener *listener);

    virtual bool init() = 0;
    virtual void __deinit__() = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;

    virtual void enableSecurity() { m_security = true; };
    virtual void disableSecurity() { m_security = false; };

    const unsigned int getMaxClient() { return m_max_client; };
    const string getName() { return m_name; };
protected:
    list<IAnyServerListener*> m_listeners;
    const unsigned int m_max_client;
    bool m_security;
    const string m_bind;
    const string m_name;
};

} // end of namespace



#endif
