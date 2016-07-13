#ifndef __ANYSERVER_H__
#define __ANYSERVER_H__

#include <string>
#include <list>
using namespace std;

namespace anyserver
{

class IAnyServerListener
{
    virtual void onClientConnected(int fd, string ip_address) = 0;
    virtual void onClientDisconnected(int fd) = 0;
    virtual void onReceive(int fd) = 0;
};

class AnyServer
{
public:
    AnyServer(const string name, const string bind, const unsigned int max_client);
    virtual ~AnyServer();

    void addEventListener(IAnyServerListener *listener);
    void removeEventListener(IAnyServerListener *listener);

    virtual bool init() = 0;
    virtual void deinit() = 0;
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
