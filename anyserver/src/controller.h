#ifndef __ANYSERVER_CONTROLLER_H__
#define __ANYSERVER_CONTROLLER_H__

#include "server_factory.h"
#include "posix_signal_interceptor.h"

namespace anyserver
{
class IControllerListener;
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

    virtual void onClientConnected(size_t server_id, size_t client_id) override;
    virtual void onClientDisconnected(size_t server_id, size_t client_id) override;
    virtual void onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len) override;

protected:
    virtual void __deinit__();
    /**
     * @brief To handle posix signal(SIGPIPE, SIGXXX and so on)
     * @param signal_id
     */
    void onReceivedPosixSignal(int signal_id);

private:
    IControllerListener *m_listener;
    ServerFactory *m_server_factory;
    PosixSignalInterceptor *m_posix_signal_interceptor;
    int m_argc;
    char **m_argv;
    string m_config_file;
};

} // end of namespace

#endif
