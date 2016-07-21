#include "anymacro.h"
#include "anyserver_factory.h"
#include "anyserver_configuration.h"
#include "inet_domainsocket_tcp_server.h"
#include "inet_domainsocket_udp_server.h"
#include "unix_domainsocket_tcp_server.h"
#include "unix_domainsocket_udp_server.h"

namespace anyserver
{

AnyServerFactory::AnyServerFactory()
    : m_anyserver_configuration(new AnyServerConfiguration())
{
    LOG_DEBUG("\n");
}

AnyServerFactory::~AnyServerFactory()
{
    LOG_DEBUG("\n");
    __deinit__();

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
    typedef AnyServerConfiguration::Configuration SConfiguration;
    const SConfiguration configuration = m_anyserver_configuration->getConfiguration();

    typedef AnyServerConfiguration::ServerInfoPtr SInfo;
    for ( list<SInfo>::const_iterator it=configuration.server_infos.begin();
            it!=configuration.server_infos.end(); ++it )
    {
        const SInfo &server_info = (*it);
        LOG_DEBUG("%s enable : %d \n", server_info->header.data(), server_info->enable);
        if ( true == server_info->enable )
        {
            switch ( server_info->kinds )
            {
                case AnyServerConfiguration::WEBSOCKET:
                    break;
                case AnyServerConfiguration::HTTP:
                    break;
                case AnyServerConfiguration::INETDS:
                    {
                        AnyServerPtr server;
                        if ( server_info->tcp )
                        {
                            server = AnyServerPtr(
                                    new InetDomainSocketTcpServer(
                                            server_info->header,
                                            server_info->bind,
                                            configuration.capabilities.max_client));

                        }
                        else
                        {
                            server = AnyServerPtr(
                                    new InetDomainSocketUdpServer(
                                            server_info->header,
                                            server_info->bind,
                                            configuration.capabilities.max_client));
                        }
                        server->addEventListener(this);
                        m_servers.push_back(server);
                    }
                    break;
                case AnyServerConfiguration::UNIXDS:
                    {
                        AnyServerPtr server;
                        if ( server_info->tcp )
                        {
                            server = AnyServerPtr(
                                    new UnixDomainSocketTcpServer(
                                            server_info->header,
                                            server_info->bind,
                                            configuration.capabilities.max_client));

                        }
                        else
                        {
                            server = AnyServerPtr(
                                    new UnixDomainSocketUdpServer(
                                            server_info->header,
                                            server_info->bind,
                                            configuration.capabilities.max_client));
                        }
                        server->addEventListener(this);
                        m_servers.push_back(server);
                    }
                    break;
                default:
                    LOG_DEBUG("Unknown and unsupported server : %s \n",
                            server_info->header.data());
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
    for ( AnyServerList::iterator it=m_servers.begin(); it!=m_servers.end(); ++it )
    {
        AnyServerPtr server = (*it);
        server->removeEventListener(this);
    }
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
    for ( AnyServerList::iterator it=m_servers.begin(); it!=m_servers.end(); ++it )
    {
        AnyServerPtr server = (*it);
        if ( nullptr != server )
        {
            server->stop();
        }
    }
}

void AnyServerFactory::onClientConnected(size_t server_id, size_t client_id)
{
    LOG_DEBUG("client[0x%x] connected to server[0x%x] \n", client_id, server_id);
}

void AnyServerFactory::onClientDisconnected(size_t server_id, size_t client_id)
{
    LOG_DEBUG("client[0x%x] disconnected from server[0x%x] \n", client_id, server_id);
}

void AnyServerFactory::onReceived(size_t server_id, size_t client_id, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("server[0x%x] get message[%s] (len=%d) from client[0x%x] \n", server_id, msg, msg_len, client_id);

    for ( list<IAnyServerFactoryListener*>::iterator it = m_server_listeners.begin();
             it!=m_server_listeners.end(); ++it )
    {
        IAnyServerFactoryListener *listener = (*it);
    }
}

void AnyServerFactory::showClientList()
{
    for ( AnyServerList::iterator it=m_servers.begin();
            it!=m_servers.end(); ++it )
    {
        AnyServerPtr server = (*it);
        LOG_DEBUG("[Server : %s] \n", server->getName().data());
        AnyServer::ClientInfoList client_list = server->getClientInfoList();
        for ( AnyServer::ClientInfoList::iterator it=client_list.begin();
                it!=client_list.end(); ++it )
        {
            AnyServer::ClientInfoPtr client = (*it);
            LOG_DEBUG("Client : 0x%x \n", client->getClientId());
        }
        LOG_DEBUG("\n");
    }
}

} // end of namespace
