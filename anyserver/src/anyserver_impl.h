#ifndef __ANYSERVER_IMPL_H__
#define __ANYSERVER_IMPL_H__

#include "anyserver.h"
#include "server_factory.h"

#include <string>
#include <list>
using namespace std;

namespace anyserver
{

class PosixSignalInterceptor;
class AnyServerImpl : public AnyServer, public IServerFactoryListener
{
public:
    AnyServerImpl(string config_file);
    virtual ~AnyServerImpl();

    /**
     * @brief initialize servers that has defined in config file
     * @return return true if succeed else return false.
     */
    bool init() override;

    /**
     * @brief start servers and the servers will be listening
     * @return return true if succeed else return false
     */
    bool start() override;

    /**
     * @brief stop servers which has started
     */
    void stop() override;

    virtual BaseServerList getServerList() override;

    virtual bool isRun() { return m_run; };

    void sendToServer(size_t server_id, string protocol, char *msg, unsigned int msg_len) override;

    void sendToServer(size_t server_id, char *msg, unsigned int msg_len) override;

    bool sendToClient(size_t server_id, size_t client_id, char *msg, unsigned int msg_len) override;

    virtual void addEventListener(IAnyServerListener *listener) override;
    virtual void removeEventListener(IAnyServerListener *listener) override;

    virtual void onReceivedPosixSignal(int signal_id);
    virtual void onClientConnected(size_t server_id, size_t client_id) override;
    virtual void onClientDisconnected(size_t server_id, size_t client_id) override;
    virtual void onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len) override;
    virtual void onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len, string protocol) override;
protected:
    void __deinit__();

    bool m_run;
    string m_config_file;
    list<IAnyServerListener*> m_listeners;
    PosixSignalInterceptor *m_posix_signal_interceptor;
    ServerFactory *m_server_factory;
};

} // end of namespace

#endif
