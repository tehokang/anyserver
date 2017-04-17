#ifndef __ANYSERVER_FACTORY_H__
#define __ANYSERVER_FACTORY_H__

#include "configuration.h"

#include <list>
#include <memory>
#include "base_server_impl.h"

using namespace std;

namespace anyserver
{

class IServerFactoryListener
{
public:
    virtual void onClientConnected(size_t server_id, size_t client_id) = 0;
    virtual void onClientDisconnected(size_t server_id, size_t client_id) = 0;
    virtual void onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len) = 0;
    virtual void onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len, string protocol) = 0;
};

class ServerFactory : public IBaseServerListener
{
public:
    static ServerFactory* getInstance();
    static void destroyInstance();

    virtual bool init(const string config_file);
    virtual bool start();
    virtual void stop();
    virtual void showClientList();

    virtual void sendToServer(size_t server_id, string protocol, char *msg, unsigned int msg_len);
    virtual void sendToServer(size_t server_id, char *msg, unsigned int msg_len);
    virtual bool sendToClient(size_t server_id, size_t client_id, char *msg, unsigned int msg_len);

    virtual void addEventListener(IServerFactoryListener *listener)
    {
        m_server_listeners.push_back(listener);
    }

    virtual void removeEventListener(IServerFactoryListener *listener)
    {
        for ( list<IServerFactoryListener*>::iterator it = m_server_listeners.begin();
                it!=m_server_listeners.end(); ++it )
        {
            if ( listener == (*it) )
            {
                m_server_listeners.erase(it);
                break;
            }
        }
    }

    typedef shared_ptr<BaseServerImpl> BaseServerPtr;
    typedef list<BaseServerPtr> BaseServerImplList;
    virtual BaseServerImplList getServerList() { return m_servers; }

    virtual void onClientConnected(size_t server_id, size_t client_id) override;
    virtual void onClientDisconnected(size_t server_id, size_t client_id) override;
    virtual void onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len) override;
    virtual void onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len, string protocol) override;
protected:
    virtual void __deinit__();
    bool __check_restrict_configuration__(const Configuration::JsonConfiguration &configuration);
    bool __create_servers__(const Configuration::JsonConfiguration &configuration);
private:
    ServerFactory();
    virtual ~ServerFactory();

    Configuration *m_configuration;
    const string m_config_file;


    BaseServerImplList m_servers;

    list<IServerFactoryListener*> m_server_listeners;
    static ServerFactory *m_instance;
};

} // end of namespace

#endif
