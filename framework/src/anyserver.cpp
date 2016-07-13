#include "anyserver.h"

namespace anyserver
{

AnyServer::AnyServer(const string name, const string bind, const unsigned int max_client)
    : m_name(name)
    , m_bind(bind)
    , m_max_client(max_client)
    , m_security(false)
{

}

AnyServer::~AnyServer()
{
    m_listeners.clear();
}

void AnyServer::addEventListener(IAnyServerListener *listener)
{
    m_listeners.push_back(listener);
}

void AnyServer::removeEventListener(IAnyServerListener *listener)
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

}
