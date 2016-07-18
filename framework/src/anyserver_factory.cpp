#include "anymacro.h"
#include "anyserver_factory.h"
#include "anyserver_configuration.h"
#include "inet_domainsocket_tcp_server.h"

namespace anyserver
{

AnyServerFactory::AnyServerFactory()
    : m_anyserver_configuration(new AnyServerConfiguration())
{
    LOG_DEBUG("\n");
}

AnyServerFactory::~AnyServerFactory()
{
    SAFE_DELETE(m_anyserver_configuration);
    m_servers.clear();
}

bool AnyServerFactory::init(const string config_file)
{
    LOG_DEBUG("\n");
    /**
     * make configuration
     */
    RETURN_FALSE_IF_FALSE(m_anyserver_configuration->init(config_file));

    /**
     * @todo create servers and save the server into list
     */
    typedef AnyServerConfiguration::ServerInfo SInfo;
    const SInfo server_info = m_anyserver_configuration->getServerInfo();

    typedef AnyServerConfiguration::ServerType SType;
    for ( list<SType>::const_iterator it=server_info.server_types.begin();
            it!=server_info.server_types.end(); ++it )
    {
        const SType &server_type = (*it);
        LOG_DEBUG("%s enable : %d \n", server_type.header.data(), server_type.enable);
        if ( true == server_type.enable )
        {
            switch ( server_type.kinds )
            {
                case AnyServerConfiguration::WEBSOCKET:
                    break;
                case AnyServerConfiguration::HTTP:
                    break;
                case AnyServerConfiguration::INETDS:
                    {
                        AnyServerPtr server = AnyServerPtr(
                                new InetDomainSocketTcpServer(
                                        server_type.header,
                                        server_type.bind,
                                        server_info.capabilities.max_client));
                        m_servers.push_back(server);
                    }
                    break;
                case AnyServerConfiguration::UNIXDS:
                    break;
                case AnyServerConfiguration::NONE:
                default:
                    LOG_DEBUG("Unknown and unsupported server : %s \n",
                            server_type.header.data());
                    break;
            }
        }
    }

    /**
     * @todo initialize servers of list
     */
    for ( AnyServerList::iterator it=m_servers.begin(); it!=m_servers.end(); ++it )
    {
        AnyServerPtr server = (*it);
        if ( false == server->init() )
        {
            LOG_ERROR("server[%s] failed to initialize \n", server->getName().data());
            return false;
        }
    }

    return true;
}

void AnyServerFactory::__deinit__()
{
    LOG_DEBUG("\n");
}

bool AnyServerFactory::start()
{
    LOG_DEBUG("\n");
    for ( AnyServerList::iterator it=m_servers.begin(); it!=m_servers.end(); ++it )
    {
        AnyServerPtr server = (*it);
        if ( false == server->start() )
        {
            LOG_ERROR("server[%s] failed to start \n", server->getName().data());
            return false;
        }
    }
    return true;
}

void AnyServerFactory::stop()
{
    LOG_DEBUG("\n");
}



} // end of namespace
