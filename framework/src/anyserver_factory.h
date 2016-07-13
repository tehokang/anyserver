#ifndef __ANYSERVER_FACTORY_H__
#define __ANYSERVER_FACTORY_H__

#include "anyserver.h"
#include "anyserver_configuration.h"

#include <list>
using namespace std;

namespace anyserver
{

class AnyServerFactory
{
public:
    AnyServerFactory();
    virtual ~AnyServerFactory();

    virtual bool init(const string config_file);
    virtual void deinit();
    virtual bool start();
    virtual void stop();

private:
    AnyServerConfiguration *m_anyserver_configuration;
    const string m_config_file;

    typedef shared_ptr<AnyServer> AnyServerPtr;
    typedef list<AnyServerPtr> AnyServerList;
    AnyServerList m_servers;
};

} // end of namespace

#endif
