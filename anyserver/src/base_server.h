#ifndef __BASE_SERVER_H__
#define __BASE_SERVER_H__

#include "macro.h"

#include <string>
#include <list>
#include <memory>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <functional>

using namespace std;

namespace anyserver
{

class IBaseServerListener
{
public:
    virtual void onClientConnected(size_t server_id, size_t client_id) = 0; // inet
    virtual void onClientDisconnected(size_t server_id, size_t client_id) = 0;
    virtual void onReceived(size_t server_id, size_t client_id, char *msg, unsigned int msg_len) = 0;
};

class BaseServer
{
public:
    BaseServer(const string name, const string bind, const bool tcp, const unsigned int max_client);
    virtual ~BaseServer();

    size_t getServerId() { return m_server_id; };
    void addEventListener(IBaseServerListener *listener);
    void removeEventListener(IBaseServerListener *listener);

    virtual bool init() = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool sendToClient(size_t client_id, char *msg, unsigned int msg_len) = 0;
    virtual void setCertification(bool security, string cert_file,
            string private_key_file, string private_key_password,
            string ca_file)
    {
        m_security = security;
        m_cert_file = cert_file;
        m_private_key_file = private_key_file;
        m_private_key_password = private_key_password;
        m_ca_file = ca_file;
    };

    const unsigned int getMaxClient() { return m_max_client; };
    const string getName() { return m_name; };

    class ClientInfo
    {
    public:
        ClientInfo(bool tcp)
            : m_tcp(tcp)
            , m_client_id(hash<size_t*>()(&m_client_id))
        {
            /* Nothing to do */
        };
        virtual ~ClientInfo()
        {
            /* Nothing to do */
        }
        size_t getClientId() { return m_client_id; };
    protected:
        const bool m_tcp;
        size_t m_client_id;
    };

    typedef shared_ptr<ClientInfo> ClientInfoPtr;
    typedef list<ClientInfoPtr> ClientInfoList;

    class TcpClientInfo : public ClientInfo
    {
    public:
        TcpClientInfo(int fd, struct sockaddr_in* sockaddr)
            : ClientInfo(true)
            , m_fd(fd)
        {
            memcpy(&m_sockaddr_in, sockaddr, sizeof(sockaddr));
            LOG_DEBUG("client[0x%x] New client [%s:%d] \n",
                    m_client_id, inet_ntoa(m_sockaddr_in.sin_addr), ntohs(m_sockaddr_in.sin_port));
        }

        TcpClientInfo(int fd, struct sockaddr_un* sockaddr)
            : ClientInfo(true)
            , m_fd(fd)
        {
            memcpy(&m_sockaddr_un, sockaddr, sizeof(sockaddr));
            LOG_DEBUG("client[0x%x] New client [%s] \n",
                    m_client_id, m_sockaddr_un.sun_path);
        }
        int getFd() { return m_fd; };
        struct sockaddr_in *getSockAddrIn() { return &m_sockaddr_in; };
        struct sockaddr_un *getSockAddrUn() { return &m_sockaddr_un; };
    protected:
        int m_fd;
        struct sockaddr_in m_sockaddr_in;
        struct sockaddr_un m_sockaddr_un;
    };

    class UdpClientInfo : public ClientInfo
    {
    public:
        UdpClientInfo(struct sockaddr_in* sockaddr)
            : ClientInfo(false)
        {
            memcpy(&m_sockaddr_in, sockaddr, sizeof(struct sockaddr_in));
            LOG_DEBUG("client[0x%x] New client [%s:%d] \n",
                    m_client_id, inet_ntoa(m_sockaddr_in.sin_addr), ntohs(m_sockaddr_in.sin_port));
        }

        UdpClientInfo(struct sockaddr_un* sockaddr)
            : ClientInfo(false)
        {
            memcpy(&m_sockaddr_un, sockaddr, sizeof(struct sockaddr_un));
            LOG_DEBUG("client[0x%x] New client [%s] \n",
                    m_client_id, m_sockaddr_un.sun_path);

        }
        struct sockaddr_in *getSockAddrIn() { return &m_sockaddr_in; };
        struct sockaddr_un *getSockAddrUn() { return &m_sockaddr_un; };
    protected:
        struct sockaddr_in m_sockaddr_in;
        struct sockaddr_un m_sockaddr_un;
    };

    size_t addClientInfo(const ClientInfoPtr client);
    size_t removeClientInfo(const int fd);
    size_t removeClientInfo(const size_t client_id);
    const ClientInfoPtr findClientInfo(const int fd);
    const ClientInfoPtr findClientInfo(const size_t client_id);
    const ClientInfoList getClientInfoList() { return m_client_list; };

protected:
    virtual void __deinit__() = 0;

    list<IBaseServerListener*> m_listeners;
    const unsigned int m_max_client;
    const bool m_tcp;
    bool m_security;
    const string m_bind;
    const string m_name;

    string m_cert_file;
    string m_private_key_file;
    string m_ca_file;
    string m_private_key_password;

    size_t m_server_id;
    ClientInfoList m_client_list;
};

#define NOTIFY_CLIENT_CONNECTED(sid, cid) \
    do { \
        list<IBaseServerListener*> listeners = server->m_listeners; \
        for ( list<IBaseServerListener*>::iterator it = listeners.begin(); \
                it!=listeners.end(); ++it ) \
        { \
            IBaseServerListener *listener = (*it); \
            listener->onClientConnected(sid, cid); \
        } \
    }while(0)

#define NOTIFY_CLIENT_DISCONNECTED(sid, cid) \
    do { \
        list<IBaseServerListener*> listeners = server->m_listeners; \
        for ( list<IBaseServerListener*>::iterator it = listeners.begin(); \
                it!=listeners.end(); ++it ) \
        { \
            IBaseServerListener *listener = (*it); \
            listener->onClientDisconnected(sid, cid); \
        } \
    }while(0)

#define NOTIFY_SERVER_RECEIVED(sid, cid, buf, buf_len) \
    do { \
        list<IBaseServerListener*> listeners = server->m_listeners; \
        for ( list<IBaseServerListener*>::iterator it = listeners.begin(); \
                it!=listeners.end(); ++it ) \
        { \
            IBaseServerListener *listener = (*it); \
            listener->onReceived(sid, cid, buf, buf_len); \
        } \
    }while(0)

} // end of namespace

#endif
