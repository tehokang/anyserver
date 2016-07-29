#ifndef __ANYSERVER_CONTROLLER_H__
#define __ANYSERVER_CONTROLLER_H__

#include "server_factory.h"

namespace anyserver
{

class IControllerListener
{
public:
    virtual void onReceivedSystemSignal(int signal) = 0;
};

class Controller : public IServerFactoryListener
{
public:
    Controller(const string config_file);
    Controller(int argc, char **argv);
    virtual ~Controller();

    void setListener(IControllerListener *listener) { m_listener = listener; };

    virtual bool init();
    virtual bool start();
    virtual void stop();
    virtual void setLogLevel(bool debug, bool info, bool warn, bool error);

    virtual void onClientConnected(size_t server_id, size_t client_id) override;
    virtual void onClientDisconnected(size_t server_id, size_t client_id) override;
    virtual void onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len) override;

protected:
    virtual void __deinit__();

private:
    IControllerListener *m_listener;
    ServerFactory *m_anyserver_factory;
    int m_argc;
    char **m_argv;
    string m_config_file;
};

} // end of namespace

#endif
