#include "base_server_impl.h"

#include "macro.h"

namespace anyserver
{

BaseServerImpl::BaseServerImpl(const string name, const string bind, const bool tcp, const unsigned int max_client)
    : BaseServer(name, bind, tcp, max_client)
    , m_security(false)
    , m_run_thread(false)
{
    LOG_DEBUG("[%s] server unique id : 0x%x \n", m_name.data(), m_server_id);
}

BaseServerImpl::~BaseServerImpl()
{
    m_listeners.clear();
}

void BaseServerImpl::addEventListener(IBaseServerListener *listener)
{
    m_listeners.push_back(listener);
}

void BaseServerImpl::removeEventListener(IBaseServerListener *listener)
{
    for ( list<IBaseServerListener*>::iterator it = m_listeners.begin();
            it!=m_listeners.end(); ++it )
    {
        if ( listener == (*it) )
        {
            m_listeners.erase(it);
            break;
        }
    }
}

size_t BaseServerImpl::addClientInfo(const BaseServerImpl::ClientInfoPtr client)
{
    m_client_list.push_back(client);
    return client->getClientId();
}

size_t BaseServerImpl::removeClientInfo(const int fd)
{
    for ( list<BaseServerImpl::ClientInfoPtr>::iterator it=m_client_list.begin();
            it!=m_client_list.end(); ++it )
    {
        auto *tcp_client = static_cast<BaseServerImpl::TcpClientInfo*>(((*it)).get());
        if ( fd == tcp_client->getFd() )
        {
            size_t client_id = tcp_client->getClientId();
            LOG_DEBUG("Removed a client in list [cid:0x%x] \n", client_id);
            m_client_list.remove(*it);
            LOG_DEBUG("Remaining client : %d \n", m_client_list.size());
            return client_id;
        }
    }
    return 0;
}

size_t BaseServerImpl::removeClientInfo(const size_t client_id)
{
    for ( list<BaseServerImpl::ClientInfoPtr>::iterator it=m_client_list.begin();
            it!=m_client_list.end(); ++it )
    {
        if ( client_id == (*it)->getClientId() )
        {
            LOG_DEBUG("Removed a client in list [cid:0x%x] \n", client_id);
            m_client_list.remove(*it);
            LOG_DEBUG("Remaining client : %d \n", m_client_list.size());
            return client_id;
        }
    }
    return 0;
}

const BaseServerImpl::ClientInfoPtr BaseServerImpl::findClientInfo(const int fd)
{
    for ( list<BaseServerImpl::ClientInfoPtr>::iterator it=m_client_list.begin();
            it!=m_client_list.end(); ++it )
    {
        auto *tcp_client = static_cast<TcpClientInfo*>(((*it)).get());
        if ( fd == tcp_client->getFd() )
        {
            return (*it);
        }
    }
    return nullptr;
}

const BaseServerImpl::ClientInfoPtr BaseServerImpl::findClientInfo(const size_t client_id)
{
    for ( list<BaseServerImpl::ClientInfoPtr>::iterator it=m_client_list.begin();
            it!=m_client_list.end(); ++it )
    {
        if ( client_id == (*it)->getClientId() )
        {
            return (*it);
        }
    }
    return nullptr;
}

} // end of namespace
