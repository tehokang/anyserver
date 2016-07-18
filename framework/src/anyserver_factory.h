#ifndef __ANYSERVER_FACTORY_H__
#define __ANYSERVER_FACTORY_H__

#include "anyserver.h"
#include "anyserver_configuration.h"

#include <list>
#include <memory>
using namespace std;

namespace anyserver
{

class AnyServerFactory
{
public:
    AnyServerFactory();
    virtual ~AnyServerFactory();

    virtual bool init(const string config_file);
    virtual bool start();
    virtual void stop();

    virtual void setEventListener(IAnyServerListener *listener)
    {
        m_server_listener = listener;
    }
protected:
    virtual void __deinit__();

private:
    AnyServerConfiguration *m_anyserver_configuration;
    const string m_config_file;

    typedef shared_ptr<AnyServer> AnyServerPtr;
    typedef list<AnyServerPtr> AnyServerList;
    AnyServerList m_servers;

    IAnyServerListener *m_server_listener;
};

} // end of namespace

#endif
