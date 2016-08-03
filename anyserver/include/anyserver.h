#ifndef __ANYSERVER_H__
#define __ANYSERVER_H__

#include "icontroller.h"
#include <string>
#include <list>
using namespace std;

namespace anyserver
{

class IAnyServerListener
{
public:
    virtual void onReceivedSystemSignal(int signal) = 0;
    virtual void onClientConnected(size_t server_id, size_t client_id) = 0;
    virtual void onClientDisconnected(size_t server_id, size_t client_id) = 0;
    virtual void onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len) = 0;
};

class Controller;
class AnyServer : public IControllerListener
{
public:
    /**
     * @brief AnyServer argv[1] has to include configuration path
     * @param argc
     * @param argv
     */
    AnyServer(int argc, char **argv);
    AnyServer(string config_file);
    virtual ~AnyServer();

    /**
     * @brief initialize servers that has defined in config file
     * @return return true if succeed else return false.
     */
    bool init();

    /**
     * @brief start servers and the servers will be listening
     * @return return true if succeed else return false
     */
    bool start();

    /**
     * @brief stop servers which has started
     */
    void stop();

    /**
     * @brief To check if servers is working out at this moment
     * @return return true if servers work out successfully
     */
    bool isRun() { return m_run; };

    virtual void addEventListener(IAnyServerListener *listener)
    {
        m_listeners.push_back(listener);
    }

    virtual void removeEventListener(IAnyServerListener *listener)
    {
        for ( list<IAnyServerListener*>::iterator it = m_listeners.begin();
                it!=m_listeners.end(); ++it )
        {
            if ( listener == (*it) )
            {
                m_listeners.erase(it);
                break;
            }
        }
    }

    virtual void onReceivedSystemSignal(int signal) override;
    virtual void onClientConnected(size_t server_id, size_t client_id) override;
    virtual void onClientDisconnected(size_t server_id, size_t client_id) override;
    virtual void onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len) override;

private:
    void __deinit__();

    bool m_run;
    Controller *m_controller;
    list<IAnyServerListener*> m_listeners;
};

} // end of namespace

#endif
