#include "anyserver_impl.h"
#include "server_factory.h"
#include "posix_signal_interceptor.h"

#include <unistd.h>
#include <csignal>

namespace anyserver
{

AnyServerImpl::AnyServerImpl(string config_file)
    : m_config_file(config_file)
    , m_run(false)
    , m_posix_signal_interceptor(new PosixSignalInterceptor())
{
    LOG_DEBUG("\n");
}

AnyServerImpl::~AnyServerImpl()
{
    SAFE_DELETE(m_posix_signal_interceptor);
    ServerFactory::destroyInstance();
}

bool AnyServerImpl::init()
{
    LOG_DEBUG("\n");
    if ( m_server_factory == nullptr )
    {
        m_server_factory = ServerFactory::getInstance();
        RETURN_FALSE_IF_NULL(m_server_factory);
        m_server_factory->addEventListener(this);
        RETURN_FALSE_IF_FALSE(m_server_factory->init(m_config_file));

        std::vector<int> signal_ids;
        signal_ids.push_back(SIGINT);
        signal_ids.push_back(SIGQUIT);
        signal_ids.push_back(SIGTERM);
        signal_ids.push_back(SIGHUP);
    //    signal_ids.push_back(SIGPIPE);
        m_posix_signal_interceptor->HandleSignals(
                signal_ids,
                std::bind1st(std::mem_fun(&AnyServerImpl::onReceivedPosixSignal), this));
    }
    return true;
}

void AnyServerImpl::addEventListener(IAnyServerListener *listener)
{
    LOG_DEBUG("\n");
    m_listeners.push_back(listener);
}

void AnyServerImpl::removeEventListener(IAnyServerListener *listener)
{
    LOG_DEBUG("\n");
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

void AnyServerImpl::__deinit__()
{
    LOG_DEBUG("\n");
    m_server_factory->removeEventListener(this);
}

bool AnyServerImpl::start()
{
    LOG_DEBUG("\n");
    RETURN_FALSE_IF_FALSE(m_server_factory->start());
    return true;
}

void AnyServerImpl::stop()
{
    LOG_DEBUG("\n");
    m_server_factory->stop();
}

AnyServer::BaseServerList AnyServerImpl::getServerList()
{
    AnyServer::BaseServerList list;
    ServerFactory::BaseServerImplList servers = m_server_factory->getServerList();
    for ( ServerFactory::BaseServerImplList::iterator it=servers.begin();
            it!=servers.end(); ++it )
    {
        ServerFactory::BaseServerPtr server_ptr = (*it);
        BaseServerPtr server = BaseServerPtr(server_ptr.get());
        list.push_back(server);
    }
    return list;
}

void AnyServerImpl::sendToServer(size_t server_id, string protocol, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("\n");
    m_server_factory->sendToServer(server_id, protocol, msg, msg_len);
}

void AnyServerImpl::sendToServer(size_t server_id, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("\n");
    m_server_factory->sendToServer(server_id, msg, msg_len);
}

bool AnyServerImpl::sendToClient(size_t server_id, size_t client_id, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("\n");
    return m_server_factory->sendToClient(server_id, client_id, msg, msg_len);
}

void AnyServerImpl::onClientConnected(size_t server_id, size_t client_id)
{
    for ( list<IAnyServerListener*>::iterator it = m_listeners.begin();
             it!=m_listeners.end(); ++it )
    {
        IAnyServerListener *listener = (*it);
        listener->onClientConnected(server_id, client_id);
    }
}

void AnyServerImpl::onClientDisconnected(size_t server_id, size_t client_id)
{
    for ( list<IAnyServerListener*>::iterator it = m_listeners.begin();
             it!=m_listeners.end(); ++it )
    {
        IAnyServerListener *listener = (*it);
        listener->onClientDisconnected(server_id, client_id);
    }
}

void AnyServerImpl::onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len)
{
    for ( list<IAnyServerListener*>::iterator it = m_listeners.begin();
             it!=m_listeners.end(); ++it )
    {
        IAnyServerListener *listener = (*it);
        listener->onReceive(server_id, client_id, msg, msg_len);
    }
}

void AnyServerImpl::onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len, string protocol)
{
    for ( list<IAnyServerListener*>::iterator it = m_listeners.begin();
             it!=m_listeners.end(); ++it )
    {
        IAnyServerListener *listener = (*it);
        listener->onReceive(server_id, client_id, msg, msg_len, protocol);
    }
}

void AnyServerImpl::onReceivedPosixSignal(int signal_id)
{
    for ( list<IAnyServerListener*>::iterator it = m_listeners.begin();
             it!=m_listeners.end(); ++it )
    {
        IAnyServerListener *listener = (*it);
        listener->onReceivedSystemSignal(signal_id);
    }
    m_run = false;
}

} // end of namespace
