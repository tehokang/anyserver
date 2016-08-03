#include "anyserver.h"
#include "controller.h"
#include "posix_signal_interceptor.h"

#include <unistd.h>
#include <csignal>

namespace anyserver
{

AnyServer::AnyServer(int argc, char **argv)
    : m_controller(new Controller(argv[1]))
    , m_run(false)
{
    m_controller->setListener(this);
};

AnyServer::AnyServer(string config_file)
    : m_controller(new Controller(config_file))
    , m_run(false)
{
    m_controller->setListener(this);
}

AnyServer::~AnyServer()
{
    if ( m_controller )
    {
        delete m_controller;
        m_controller = nullptr;
    }
}

bool AnyServer::init()
{
    return m_controller->init();
}

void AnyServer::__deinit__()
{
    RETURN_IF_NULL(m_controller);
    m_controller->setListener(nullptr);
}

bool AnyServer::start()
{
    if ( m_controller->start() )
    {
        m_run = true;
        return true;
    }
    return false;
}

void AnyServer::stop()
{
    m_controller->stop();
}

void AnyServer::onReceivedSystemSignal(int signal)
{
    for ( list<IAnyServerListener*>::iterator it = m_listeners.begin();
             it!=m_listeners.end(); ++it )
    {
        IAnyServerListener *listener = (*it);
        listener->onReceivedSystemSignal(signal);
    }

    m_run = false;
}

void AnyServer::onClientConnected(size_t server_id, size_t client_id)
{
    for ( list<IAnyServerListener*>::iterator it = m_listeners.begin();
             it!=m_listeners.end(); ++it )
    {
        IAnyServerListener *listener = (*it);
        listener->onClientConnected(server_id, client_id);
    }
}

void AnyServer::onClientDisconnected(size_t server_id, size_t client_id)
{
    for ( list<IAnyServerListener*>::iterator it = m_listeners.begin();
             it!=m_listeners.end(); ++it )
    {
        IAnyServerListener *listener = (*it);
        listener->onClientDisconnected(server_id, client_id);
    }
}

void AnyServer::onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len)
{
    for ( list<IAnyServerListener*>::iterator it = m_listeners.begin();
             it!=m_listeners.end(); ++it )
    {
        IAnyServerListener *listener = (*it);
        listener->onReceive(server_id, client_id, msg, msg_len);
    }
}


} // end of namespace
