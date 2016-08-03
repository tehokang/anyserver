#ifndef __INTERFACE_CONTROLLER_H__
#define __INTERFACE_CONTROLLER_H__

#include <cstddef>
using namespace std;

namespace anyserver
{

class IControllerListener
{
public:
    virtual void onReceivedSystemSignal(int signal) = 0;
    virtual void onClientConnected(size_t server_id, size_t client_id) = 0;
    virtual void onClientDisconnected(size_t server_id, size_t client_id) = 0;
    virtual void onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len) = 0;
};

} // end of namespace

#endif
