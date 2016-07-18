#ifndef __ANYSERVER_CONTROLLER_H__
#define __ANYSERVER_CONTROLLER_H__

#include "anyserver_factory.h"
#include "posix_signal_interceptor.h"

namespace anyserver
{

class IAnyServerControllerListener
{
public:
    virtual void onReceivedSystemSignal(int signal) = 0;
};

class AnyServerController : public IAnyServerListener
{
public:
    AnyServerController(const string config_file);
    AnyServerController(int argc, char **argv);
    virtual ~AnyServerController();

    void setListener(IAnyServerControllerListener *listener) { m_listener = listener; };

    virtual bool init();
    virtual bool start();
    virtual void stop();
    virtual void setLogLevel(bool debug, bool info, bool warn, bool error);

    virtual void onClientConnected(int fd, string ip_address, int port) override;
    virtual void onClientDisconnected(int fd) override;
    virtual void onReceive(int fd) override;
protected:
    void onReceivedPosixSignal(int signal_id);

    virtual void __deinit__();

private:
    PosixSignalInterceptor posix_signal_interceptor;
    IAnyServerControllerListener *m_listener;
    AnyServerFactory *m_anyserver_factory;
    int m_argc;
    char **m_argv;
    string m_config_file;
};

} // end of namespace

#endif
