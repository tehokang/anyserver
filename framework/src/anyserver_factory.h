#ifndef __ANYSERVER_FACTORY_H__
#define __ANYSERVER_FACTORY_H__

#include "anyserver.h"
#include "anyserver_configuration.h"

#include <list>
#include <memory>

using namespace std;

namespace anyserver
{

class IAnyServerFactoryListener
{
public:
    virtual void onClientConnected(size_t server_id, size_t client_id) = 0;
    virtual void onClientDisconnected(size_t server_id, size_t client_id) = 0;
    virtual void onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len) = 0;
};

class AnyServerFactory : public IAnyServerListener
{
public:
    AnyServerFactory();
    virtual ~AnyServerFactory();

    virtual bool init(const string config_file);
    virtual bool start();
    virtual void stop();
    virtual void showClientList();
    virtual bool sendToClient(size_t server_id, size_t client_id, char *msg, unsigned int msg_len);

    virtual void addEventListener(IAnyServerFactoryListener *listener)
    {
        m_server_listeners.push_back(listener);
    }

    virtual void removeEventListener(IAnyServerFactoryListener *listener)
    {
        for ( list<IAnyServerFactoryListener*>::iterator it = m_server_listeners.begin();
                it!=m_server_listeners.end(); ++it )
        {
            if ( listener == (*it) )
            {
                m_server_listeners.erase(it);
                break;
            }
        }
    }

    virtual void onClientConnected(size_t server_id, size_t client_id) override;
    virtual void onClientDisconnected(size_t server_id, size_t client_id) override;
    virtual void onReceived(size_t server_id, size_t client_id, char *msg, unsigned int msg_len) override;
protected:
    virtual void __deinit__();

private:
    AnyServerConfiguration *m_anyserver_configuration;
    const string m_config_file;

    typedef shared_ptr<AnyServer> AnyServerPtr;
    typedef list<AnyServerPtr> AnyServerList;
    AnyServerList m_servers;

    list<IAnyServerFactoryListener*> m_server_listeners;
};

} // end of namespace

#endif
