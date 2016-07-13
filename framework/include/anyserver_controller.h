#ifndef __ANYSERVER_CONTROLLER_H__
#define __ANYSERVER_CONTROLLER_H__

#include "anyserver_factory.h"

namespace anyserver
{

class AnyServerController : public IAnyServerListener
{
public:
    AnyServerController(const string config_file);
    AnyServerController(int argc, char **argv);
    virtual ~AnyServerController();

    virtual bool init();
    virtual void deinit();
    virtual bool start();
    virtual void stop();
    virtual void setLogLevel(bool debug, bool info, bool warn, bool error);

    virtual void onClientConnected(int fd, string ip_address) override;
    virtual void onClientDisconnected(int fd) override;
    virtual void onReceive(int fd) override;

private:
    AnyServerFactory *m_anyserver_factory;
    int m_argc;
    char **m_argv;
    string m_config_file;
};

} // end of namespace

#endif
