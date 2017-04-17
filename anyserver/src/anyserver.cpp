#include "anyserver.h"
#include "anyserver_impl.h"
#include "server_factory.h"
#include "posix_signal_interceptor.h"

#include <unistd.h>
#include <csignal>

namespace anyserver
{
AnyServer* AnyServer::m_instance = nullptr;
AnyServer* AnyServer::getInstance(int argc, char **argv)
{
    if ( m_instance == nullptr )
    {
        m_instance = new AnyServerImpl(string(argv[1]));
    }
    return m_instance;
}

AnyServer* AnyServer::getInstance(string config_file)
{
    if ( m_instance == nullptr )
    {
        m_instance = new AnyServerImpl(config_file);
    }
    return m_instance;
}

BaseServer::BaseServer(const string name, const string bind, const bool tcp, const unsigned int max_client)
    : m_name(name)
    , m_bind(bind)
    , m_tcp(tcp)
    , m_max_client(max_client)
    , m_server_id(hash<string>()(m_name + bind + to_string(tcp)))
{

}

BaseServer::~BaseServer()
{

}


} // end of namespace
