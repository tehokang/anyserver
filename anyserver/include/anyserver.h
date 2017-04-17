#ifndef __ANYSERVER_H__
#define __ANYSERVER_H__

#include <string>
#include <list>
#include <memory>

using namespace std;

namespace anyserver
{

class IAnyServerListener
{
public:
    virtual void onReceivedSystemSignal(int signal) = 0;
    virtual void onClientConnected(size_t server_id, size_t client_id) = 0;
    virtual void onClientDisconnected(size_t server_id, size_t client_id) = 0;
    virtual void onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len) = 0;
    virtual void onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len, string protocol) = 0;
};

class BaseServer
{
public:
    BaseServer(const string name, const string bind, const bool tcp, const unsigned int max_client);
    virtual ~BaseServer();

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

    const unsigned int getMaxClient() { return m_max_client; };
    size_t getId() { return m_server_id; };
    const string getName() { return m_name; };
    const ClientInfoList getClientInfoList() { return m_client_list; };

protected:
    const bool m_tcp;
    const string m_bind;
    const string m_name;
    const unsigned int m_max_client;
    const size_t m_server_id;
    ClientInfoList m_client_list;
};

class AnyServer
{
public:
    static AnyServer* getInstance(int argc, char **argv);
    static AnyServer* getInstance(string config_file);
    /**
     * @brief initialize servers that has defined in config file
     * @return return true if succeed else return false.
     */
    virtual bool init() = 0;

    /**
     * @brief start servers and the servers will be listening
     * @return return true if succeed else return false
     */
    virtual bool start() = 0;

    /**
     * @brief stop servers which has started
     */
    virtual void stop() = 0;

    typedef shared_ptr<BaseServer> BaseServerPtr;
    typedef list<BaseServerPtr> BaseServerList;
    virtual BaseServerList getServerList() = 0;

    virtual void sendToServer(size_t server_id, string protocol, char *msg, unsigned int msg_len) = 0;

    virtual void sendToServer(size_t server_id, char *msg, unsigned int msg_len) = 0;

    virtual bool sendToClient(size_t server_id, size_t client_id, char *msg, unsigned int msg_len) = 0;

    /**
     * @brief To check if servers is working out at this moment
     * @return return true if servers work out successfully
     */
    bool isRun() { return m_run; };

    virtual void addEventListener(IAnyServerListener *listener) = 0;
    virtual void removeEventListener(IAnyServerListener *listener) = 0;
protected:
    virtual ~AnyServer() { };
    void __deinit__();

    bool m_run;
    BaseServerList m_server_list;

    static AnyServer* m_instance;

};

} // end of namespace

#endif
