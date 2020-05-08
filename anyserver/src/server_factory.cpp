#include "macro.h"
#include "server_factory.h"
#include "configuration.h"
#include "websocket_tcp_server.h"
#include "http_tcp_server.h"
#include "inet_domainsocket_tcp_server.h"
#include "inet_domainsocket_udp_server.h"
#include "unix_domainsocket_tcp_server.h"
#include "unix_domainsocket_udp_server.h"

namespace anyserver
{
ServerFactory *ServerFactory::m_instance = nullptr;
ServerFactory* ServerFactory::getInstance()
{
    if ( m_instance == nullptr )
    {
        m_instance = new ServerFactory();
    }
    return m_instance;
}

void ServerFactory::destroyInstance()
{
    if ( m_instance != nullptr )
    {
        SAFE_DELETE(m_instance);
    }
}

ServerFactory::ServerFactory()
    : m_configuration(new Configuration())
{
    LOG_DEBUG("\n");
}

ServerFactory::~ServerFactory()
{
    LOG_DEBUG("\n");
    __deinit__();

    SAFE_DELETE(m_configuration);
    m_servers.clear();
}

bool ServerFactory::init(const string config_file)
{
    LOG_DEBUG("\n");
    /**
     * make configuration
     */
    RETURN_FALSE_IF_FALSE(m_configuration->init(config_file));

    /**
     * @todo create servers and save the server into list
     */
    typedef Configuration::JsonConfiguration JsonConfiguration;
    const JsonConfiguration configuration = m_configuration->getJsonConfiguration();

    Logger::setLogLevel(
            configuration.log.enable_info,
            configuration.log.enable_debug,
            configuration.log.enable_warn,
            configuration.log.enable_error,
            configuration.log.enable_filewrite,
            configuration.log.filesize,
            configuration.log.directory,
            configuration.name);

    RETURN_FALSE_IF_FALSE(__check_restrict_configuration__(configuration));
    RETURN_FALSE_IF_FALSE(__create_servers__(configuration));

    /**
     * @todo initialize servers of list
     */
    for ( BaseServerImplList::iterator it=m_servers.begin(); it!=m_servers.end(); ++it )
    {
        BaseServerPtr server = (*it);
        if ( false == server->init() )
        {
            LOG_ERROR("server[%s] failed to initialize \n", server->getName().data());
            return false;
        }
    }

    return true;
}

void ServerFactory::__deinit__()
{
    for ( BaseServerImplList::iterator it=m_servers.begin(); it!=m_servers.end(); ++it )
    {
        BaseServerPtr server = (*it);
        server->removeEventListener(this);
    }
    LOG_DEBUG("\n");
}

bool ServerFactory::start()
{
    LOG_DEBUG("\n");
    for ( BaseServerImplList::iterator it=m_servers.begin(); it!=m_servers.end(); ++it )
    {
        BaseServerPtr server = (*it);
        if ( false == server->start() )
        {
            LOG_ERROR("server[%s] failed to start \n", server->getName().data());
            return false;
        }
    }
    return true;
}

void ServerFactory::stop()
{
    LOG_DEBUG("\n");
    for ( BaseServerImplList::iterator it=m_servers.begin(); it!=m_servers.end(); ++it )
    {
        BaseServerPtr server = (*it);
        if ( nullptr != server )
        {
            server->stop();
        }
    }
}

void ServerFactory::onClientConnected(size_t server_id, size_t client_id)
{
    LOG_DEBUG("\n");
    showClientList();

    LOG_DEBUG("client[0x%x] connected to server[0x%x] \n", client_id, server_id);
    for ( list<IServerFactoryListener*>::iterator it = m_server_listeners.begin();
             it!=m_server_listeners.end(); ++it )
    {
        IServerFactoryListener *listener = (*it);
        listener->onClientConnected(server_id, client_id);
    }
}

void ServerFactory::onClientDisconnected(size_t server_id, size_t client_id)
{
    LOG_DEBUG("\n");
    showClientList();

    LOG_DEBUG("client[0x%x] disconnected from server[0x%x] \n", client_id, server_id);
    for ( list<IServerFactoryListener*>::iterator it = m_server_listeners.begin();
             it!=m_server_listeners.end(); ++it )
    {
        IServerFactoryListener *listener = (*it);
        listener->onClientDisconnected(server_id, client_id);
    }
}

void ServerFactory::onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("server[0x%x] get message[%s] (len=%d) from client[0x%x] \n", server_id, msg, msg_len, client_id);

    for ( list<IServerFactoryListener*>::iterator it = m_server_listeners.begin();
             it!=m_server_listeners.end(); ++it )
    {
        IServerFactoryListener *listener = (*it);
        listener->onReceive(server_id, client_id, msg, msg_len);
    }

    RETURN_IF_NULL(m_configuration);
    if ( true == m_configuration->getJsonConfiguration().enable_echo_test )
    {
        sendToClient(server_id, client_id, msg, msg_len);
    }
}

void ServerFactory::onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len, string protocol)
{
    LOG_DEBUG("server[0x%x] get message[%s] (len=%d) from client[0x%x] \n", server_id, msg, msg_len, client_id);

    for ( list<IServerFactoryListener*>::iterator it = m_server_listeners.begin();
             it!=m_server_listeners.end(); ++it )
    {
        IServerFactoryListener *listener = (*it);
        listener->onReceive(server_id, client_id, msg, msg_len, protocol);
    }
}

void ServerFactory::showClientList()
{
    LOG_DEBUG("\n");
    for ( BaseServerImplList::iterator it=m_servers.begin();
            it!=m_servers.end(); ++it )
    {
        BaseServerPtr server = (*it);
        LOG_DEBUG("[Server : %s] \n", server->getName().data());
        BaseServerImpl::ClientInfoList client_list = server->getClientInfoList();
        int i=0;
        for ( BaseServerImpl::ClientInfoList::iterator it=client_list.begin();
                it!=client_list.end(); ++it )
        {
            BaseServerImpl::ClientInfoPtr client = (*it);
            //LOG_DEBUG("[%d] Client : 0x%x \n", ++i, client->getClientId());
        }
        LOG_DEBUG("Client connected : %d \n", i);
        LOG_DEBUG("\n");
    }
}

void ServerFactory::sendToServer(size_t server_id, string protocol, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("server_id : 0x%x \n", server_id);
    for ( BaseServerImplList::iterator it=m_servers.begin();
            it!=m_servers.end(); ++it )
    {
        BaseServerPtr server = (*it);
        if ( server_id == server->getId() )
        {
            BaseServerImpl::ClientInfoList client_list = server->getClientInfoList();
            for ( BaseServerImpl::ClientInfoList::iterator it=client_list.begin();
                    it!=client_list.end(); ++it )
            {
                LOG_DEBUG("Found server to send message : 0x%x (msg : %s)\n", server_id, msg);
                server->sendToClient((*it)->getClientId(), protocol, msg, msg_len);
            }
        }
    }
}

void ServerFactory::sendToServer(size_t server_id, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("server_id : 0x%x \n", server_id);
    for ( BaseServerImplList::iterator it=m_servers.begin();
            it!=m_servers.end(); ++it )
    {
        BaseServerPtr server = (*it);
        if ( server_id == server->getId() )
        {
            BaseServerImpl::ClientInfoList client_list = server->getClientInfoList();
            for ( BaseServerImpl::ClientInfoList::iterator it=client_list.begin();
                    it!=client_list.end(); ++it )
            {
                LOG_DEBUG("Found server to send message : 0x%x (msg : %s)\n", server_id, msg);
                server->sendToClient((*it)->getClientId(), msg, msg_len);
            }
        }
    }
}

bool ServerFactory::sendToClient(size_t server_id, size_t client_id, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("server_id : 0x%x \n", server_id);
    for ( BaseServerImplList::iterator it=m_servers.begin();
            it!=m_servers.end(); ++it )
    {
        BaseServerPtr server = (*it);
        if ( server_id == server->getId() )
        {
            LOG_DEBUG("Found server to send message : 0x%x (msg : %s)\n", server_id, msg);
            return server->sendToClient(client_id, msg, msg_len);
        }
    }
    return false;
}

#define ADD_SERVER(sinfo, TYPE, slist) \
    do { \
        server = BaseServerPtr( \
                new TYPE( \
                        sinfo->header, \
                        sinfo->bind, \
                        sinfo->tcp, \
                        sinfo->max_client)); \
        server->addEventListener(this); \
        slist.push_back(server); \
    } while(0)

bool ServerFactory::__check_restrict_configuration__(
        const Configuration::JsonConfiguration &configuration)
{
    /**
     * @warning Must check if security is enabled
     * If so, security server will have be only one server or
     * anyserver have to reject this configuration
     */
    typedef Configuration::ServerInfoPtr SInfo;
    if ( configuration.capabilities.enable_security )
    {
        int enable_server_count = 0;
        for ( list<SInfo>::const_iterator it=configuration.server_infos.begin();
                it!=configuration.server_infos.end(); ++it )
        {
            const SInfo &server_info = (*it);
            if ( server_info->enable ) enable_server_count++;
        }
        if ( 1 < enable_server_count )
        {
            LOG_ERROR("Please enable only one server to support "
                    "security type of server.\n"
                    "Currently openssl context couldn't create multiple in a process\n");
            return false;

        }
    }
    return true;
}

bool ServerFactory::__create_servers__(
        const Configuration::JsonConfiguration &configuration)
{
    typedef Configuration::ServerInfoPtr SInfo;
    for ( list<SInfo>::const_iterator it=configuration.server_infos.begin();
            it!=configuration.server_infos.end(); ++it )
    {
        const SInfo &server_info = (*it);
        LOG_DEBUG("%s enable : %d \n", server_info->header.data(), server_info->enable);
        if ( true == server_info->enable )
        {
            BaseServerPtr server;
            switch ( server_info->kinds )
            {
                case Configuration::WEBSOCKET:
                    {
                        if ( server_info->tcp )
                        {
                            ADD_SERVER(server_info, WebSocketTcpServer, m_servers);
                            auto websocket_server = static_pointer_cast<WebSocketTcpServer>(server);
                            websocket_server->addProtocols(server_info->protocols);
                            websocket_server->setCertification(
                                    configuration.capabilities.enable_security,
                                    configuration.capabilities.ssl_cert,
                                    configuration.capabilities.ssl_private_key,
                                    configuration.capabilities.ssl_private_key_password,
                                    configuration.capabilities.ssl_ca);
                        }
                        else
                        {
                            LOG_ERROR("Not support UDP websocket server yet \n");
                            return false;
                        }
                    }
                    break;
                case Configuration::HTTP:
                    {
                        if ( server_info->tcp )
                        {
                            ADD_SERVER(server_info, HttpTcpServer, m_servers);
                            auto http_server = static_pointer_cast<HttpTcpServer>(server);
                            http_server->setCertification(
                                    configuration.capabilities.enable_security,
                                    configuration.capabilities.ssl_cert,
                                    configuration.capabilities.ssl_private_key,
                                    configuration.capabilities.ssl_private_key_password,
                                    configuration.capabilities.ssl_ca);
                        }
                        else
                        {
                            LOG_ERROR("Not support UDP http server yet \n");
                            return false;
                        }
                    }
                    break;
                case Configuration::INETDS:
                    {
                        BaseServerPtr server;
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
                case Configuration::UNIXDS:
                    {
                        BaseServerPtr server;
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
    return true;
}

} // end of namespace
