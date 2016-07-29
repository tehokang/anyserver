#include "anymacro.h"
#include "anyserver_factory.h"
#include "anyserver_configuration.h"
#include "websocket_tcp_server.h"
#include "http_tcp_server.h"
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

#define ADD_SERVER(sinfo, TYPE, slist) \
    do { \
        AnyServerPtr server = AnyServerPtr( \
                new TYPE( \
                        sinfo->header, \
                        sinfo->bind, \
                        sinfo->tcp, \
                        sinfo->max_client)); \
        server->addEventListener(this); \
        slist.push_back(server); \
    } while(0)

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
                    {
                        if ( server_info->tcp )
                        {
                            ADD_SERVER(server_info, WebSocketTcpServer, m_servers);
                        }
                        else
                        {
                            LOG_ERROR("Not support UDP websocket server yet \n");
                            return false;
                        }
                    }
                    break;
                case AnyServerConfiguration::HTTP:
                    {
                        if ( server_info->tcp )
                        {
                            ADD_SERVER(server_info, HttpTcpServer, m_servers);
                        }
                        else
                        {
                            LOG_ERROR("Not support UDP http server yet \n");
                            return false;
                        }
                    }
                    break;
                case AnyServerConfiguration::INETDS:
                    {
                        AnyServerPtr server;
                        if ( server_info->tcp )
                        {
                            ADD_SERVER(server_info, InetDomainSocketTcpServer, m_servers);
                        }
                        else
                        {
                            ADD_SERVER(server_info, InetDomainSocketUdpServer, m_servers);
                        }
                    }
                    break;
                case AnyServerConfiguration::UNIXDS:
                    {
                        AnyServerPtr server;
                        if ( server_info->tcp )
                        {
                            ADD_SERVER(server_info, UnixDomainSocketTcpServer, m_servers);
                        }
                        else
                        {
                            ADD_SERVER(server_info, UnixDomainSocketUdpServer, m_servers);
                        }
                    }
                    break;
                default:
                    {
                        LOG_DEBUG("Unknown and unsupported server : %s \n",
                                server_info->header.data());
                    }
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
    LOG_DEBUG("\n");
    showClientList();

    LOG_DEBUG("client[0x%x] connected to server[0x%x] \n", client_id, server_id);
    for ( list<IAnyServerFactoryListener*>::iterator it = m_server_listeners.begin();
             it!=m_server_listeners.end(); ++it )
    {
        IAnyServerFactoryListener *listener = (*it);
        listener->onClientConnected(server_id, client_id);
    }
}

void AnyServerFactory::onClientDisconnected(size_t server_id, size_t client_id)
{
    LOG_DEBUG("\n");
    showClientList();

    LOG_DEBUG("client[0x%x] disconnected from server[0x%x] \n", client_id, server_id);
    for ( list<IAnyServerFactoryListener*>::iterator it = m_server_listeners.begin();
             it!=m_server_listeners.end(); ++it )
    {
        IAnyServerFactoryListener *listener = (*it);
        listener->onClientDisconnected(server_id, client_id);
    }
}

void AnyServerFactory::onReceived(size_t server_id, size_t client_id, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("server[0x%x] get message[%s] (len=%d) from client[0x%x] \n", server_id, msg, msg_len, client_id);

    for ( list<IAnyServerFactoryListener*>::iterator it = m_server_listeners.begin();
             it!=m_server_listeners.end(); ++it )
    {
        IAnyServerFactoryListener *listener = (*it);
        listener->onReceive(server_id, client_id, msg, msg_len);
    }

    RETURN_IF_NULL(m_anyserver_configuration);
    if ( true == m_anyserver_configuration->getConfiguration().enable_echo_test )
    {
        sendToClient(server_id, client_id, msg, msg_len);
    }
}

void AnyServerFactory::showClientList()
{
    LOG_DEBUG("\n");
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

bool AnyServerFactory::sendToClient(size_t server_id, size_t client_id, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("server_id : 0x%x \n", server_id);
    for ( AnyServerList::iterator it=m_servers.begin();
            it!=m_servers.end(); ++it )
    {
        AnyServerPtr server = (*it);
        if ( server_id == server->getServerId() )
        {
            LOG_DEBUG("Found server to send message : 0x%x (msg : %s)\n", server_id, msg);
            return server->sendToClient(client_id, msg, msg_len);
        }
    }
    return false;
}

} // end of namespace
