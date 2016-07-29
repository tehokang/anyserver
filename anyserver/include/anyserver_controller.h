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

class AnyServerController : public IAnyServerFactoryListener
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

    virtual void onClientConnected(size_t server_id, size_t client_id) override;
    virtual void onClientDisconnected(size_t server_id, size_t client_id) override;
    virtual void onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len) override;

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
