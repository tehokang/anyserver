#include "websocket_tcp_server.h"

#define MAX_ECHO_PAYLOAD 1024

namespace anyserver
{
struct lws_context *WebSocketTcpServer::m_context = nullptr;
struct lws_vhost *WebSocketTcpServer::m_vhost = nullptr;

WebSocketTcpServer::WebSocketTcpServer(
        const string name, const string bind, const bool tcp, const unsigned int max_client)
    : BaseServerImpl(name, bind, tcp, max_client)
    , m_lws_protocols(nullptr)
{
    LOG_DEBUG("\n");

    lws_set_log_level(
            LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO |
            LLL_DEBUG | LLL_PARSER | LLL_HEADER | LLL_EXT | LLL_CLIENT | LLL_LATENCY,
            __log__);
}

WebSocketTcpServer::~WebSocketTcpServer()
{
    LOG_DEBUG("\n");
    __deinit__();

    if ( m_context )
    {
        lws_context_destroy(m_context);
        m_context = nullptr;
    }
    SAFE_FREE(m_lws_protocols);
}

void WebSocketTcpServer::addProtocols(list<string> protocols)
{
    LOG_DEBUG("protocols.size : %d \n", protocols.size());
    m_lws_protocols = (struct lws_protocols*)
            malloc(sizeof(struct lws_protocols)*(BASIC_PROTOCOL+protocols.size()));

    unsigned int protocol_id = 0;
    /**
     * @note None protocol request will patch from callback_websocket
     */
    m_lws_protocols[HTTP].name = "http-only";
    m_lws_protocols[HTTP].callback = __callback_websocket__;
    m_lws_protocols[HTTP].user = (void*)(m_lws_protocols[HTTP].name);
    m_lws_protocols[HTTP].per_session_data_size = strlen(m_lws_protocols[HTTP].name);
    m_lws_protocols[HTTP].rx_buffer_size = MAX_ECHO_PAYLOAD;
    m_lws_protocols[HTTP].id = protocol_id++;

    /**
     * @note "test" protocol
     */
    m_lws_protocols[TEST].name = "test";
    m_lws_protocols[TEST].callback = __callback_websocket__;
    m_lws_protocols[TEST].user = (void*)(m_lws_protocols[TEST].name);
    m_lws_protocols[TEST].per_session_data_size = strlen(m_lws_protocols[TEST].name);
    m_lws_protocols[TEST].rx_buffer_size = MAX_ECHO_PAYLOAD;
    m_lws_protocols[TEST].id = protocol_id++;

    int index = TEST+1;
    for ( list<string>::iterator it=protocols.begin();
            it!=protocols.end(); ++it, index++)
    {
        m_lws_protocols[index].name = (*it).data();
        m_lws_protocols[index].callback = __callback_websocket__;
        m_lws_protocols[index].user = (void*)(m_lws_protocols[index].name);
        m_lws_protocols[index].per_session_data_size = strlen(m_lws_protocols[index].name);
        m_lws_protocols[index].rx_buffer_size = MAX_ECHO_PAYLOAD;
        m_lws_protocols[index].id = protocol_id++;
    }

    m_lws_protocols[index].name = nullptr;
    m_lws_protocols[index].callback = nullptr;
    m_lws_protocols[index].user = nullptr;
    m_lws_protocols[index].per_session_data_size = 0;
    m_lws_protocols[index].rx_buffer_size = 0;
    m_lws_protocols[index].id = protocol_id;
}

bool WebSocketTcpServer::init()
{
    LOG_DEBUG("\n");
    memset(&m_context_create_info, 0x00, sizeof(m_context_create_info));

    m_context_create_info.port = stoi(m_bind);
    m_context_create_info.iface = nullptr;
    m_context_create_info.protocols = m_lws_protocols;
    m_context_create_info.extensions = nullptr;
    m_context_create_info.keepalive_timeout = 60;
    m_context_create_info.timeout_secs = 5;
    m_context_create_info.gid = -1;
    m_context_create_info.uid = -1;

    if ( m_security )
    {
        m_context_create_info.vhost_name = "localhost";
        m_context_create_info.ssl_private_key_filepath = m_private_key_file.data();
        m_context_create_info.ssl_private_key_password = m_private_key_password.data();
        m_context_create_info.ssl_cert_filepath = m_cert_file.data();
        m_context_create_info.ssl_ca_filepath = m_ca_file.data();
        m_context_create_info.options =
                LWS_SERVER_OPTION_REDIRECT_HTTP_TO_HTTPS
                | LWS_SERVER_OPTION_VALIDATE_UTF8
                | LWS_SERVER_OPTION_ALLOW_NON_SSL_ON_SSL_PORT
                | LWS_SERVER_OPTION_PEER_CERT_NOT_REQUIRED
                | LWS_SERVER_OPTION_EXPLICIT_VHOSTS
                | LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
        /**
         * @warning LWS_SERVER_OPTION_ALLOW_NON_SSL_ON_SSL_PORT has a bug
         * When client request ws(non-secure), server emit so much many logs (reason 34,35,36)
         * via callback_websocket()
         */

        m_context_create_info.ssl_cipher_list =
                "ECDHE-ECDSA-AES256-GCM-SHA384:"
                "ECDHE-RSA-AES256-GCM-SHA384:"
                "DHE-RSA-AES256-GCM-SHA384:"
                "ECDHE-RSA-AES256-SHA384:"
                "HIGH:!aNULL:!eNULL:!EXPORT:"
                "!DES:!MD5:!PSK:!RC4:!HMAC_SHA1:"
                "!SHA1:!DHE-RSA-AES128-GCM-SHA256:"
                "!DHE-RSA-AES128-SHA256:"
                "!AES128-GCM-SHA256:"
                "!AES128-SHA256:"
                "!DHE-RSA-AES256-SHA256:"
                "!AES256-GCM-SHA384:"
                "!AES256-SHA256";
    }
    else
    {
        m_context_create_info.ssl_private_key_filepath = nullptr;
        m_context_create_info.ssl_cert_filepath = nullptr;
        m_context_create_info.ssl_ca_filepath = nullptr;
        m_context_create_info.options = LWS_SERVER_OPTION_EXPLICIT_VHOSTS;
    }

    m_context_create_info.server_string = m_name.data();
    m_context_create_info.fd_limit_per_thread = m_max_client;
    m_context_create_info.user = this;

    m_context = lws_create_context(&m_context_create_info);
    if ( nullptr == m_context )
    {
        LOG_ERROR("context has nullptr \n");
        return false;
    }

    m_vhost = lws_create_vhost(m_context, &m_context_create_info);
    if ( nullptr == m_vhost )
    {
        LOG_ERROR("vhost_context has nullptr \n");
        return false;
    }
    return true;
}

bool WebSocketTcpServer::start()
{
    LOG_DEBUG("\n");
    if ( 0 != pthread_create(
            &m_websocket_thread, NULL, WebSocketTcpServer::__websocket_thread__, (void*)this) )
    {
        LOG_ERROR("Failed to create thread \n");
        return false;
    }
    return true;
}

void WebSocketTcpServer::stop()
{
    LOG_DEBUG("\n");
    m_run_thread = false;
    {
        int status = 0;
        pthread_join(m_websocket_thread, (void**)&status);
    }
}

bool WebSocketTcpServer::sendToClient(size_t client_id, string protocol, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("\n");
    auto client = static_pointer_cast<WebSocketTcpClientInfo>(findClientInfo(client_id));
    if ( client && strcmp(protocol.data(), client->getProtocol().data()) == 0 )
    {
        lws_write((struct lws *)client->getWsi(), (unsigned char*)msg, msg_len, LWS_WRITE_TEXT);
        return true;
    }
    return false;
}

bool WebSocketTcpServer::sendToClient(size_t client_id, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("\n");
    auto client = static_pointer_cast<WebSocketTcpClientInfo>(findClientInfo(client_id));
    if ( -1 == lws_write((struct lws *)client->getWsi(), (unsigned char*)msg, msg_len, LWS_WRITE_TEXT) )
    {
        return false;
    }
    return true;
}

void WebSocketTcpServer::__deinit__()
{
    LOG_DEBUG("\n");
    stop();
}

void WebSocketTcpServer::__log__(int level, const char *line)
{
    switch( level )
    {
        case LLL_ERR : LOG_ERROR("%s", line); break;
        case LLL_WARN: LOG_WARNING("%s", line); break;
        case LLL_NOTICE:
        case LLL_INFO: LOG_INFO("%s", line); break;
        case LLL_DEBUG:
        case LLL_PARSER:
        case LLL_HEADER:
        case LLL_EXT:
        case LLL_CLIENT:
        case LLL_LATENCY: LOG_DEBUG("%s", line) break;
        default:
            break;
    }
}

void* WebSocketTcpServer::__websocket_thread__(void *arg)
{
    LOG_DEBUG("\n");
    WebSocketTcpServer *server = static_cast<WebSocketTcpServer*>(arg);
    server->m_run_thread = true;

    while ( server->m_run_thread )
    {
        lws_service(server->m_context, 50);
    }
    return nullptr;
}

string WebSocketTcpServer::__getProtocolFromHeader__(struct lws *wsi)
{
    string protocol(64, '\0');
    lws_hdr_copy(wsi, (char*)protocol.data(), protocol.length(), WSI_TOKEN_GET_URI);
    return __replaceAll__(protocol, "/", "");
}

string WebSocketTcpServer::__replaceAll__(string &str, const string& from, const string& to){
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

int WebSocketTcpServer::__callback_websocket__(struct lws *wsi,
        enum lws_callback_reasons reason,
        void *user, void *in, size_t len)
{
    if ( nullptr == m_context ) return 0;

    WebSocketTcpServer *server = static_cast<WebSocketTcpServer*>(lws_context_user(m_context));

    if ( server->m_run_thread == false ) return 0;

    switch ( reason )
    {
        case LWS_CALLBACK_WSI_CREATE:
            break;
        case LWS_CALLBACK_ESTABLISHED:
            {
                LOG_DEBUG("LWS_CALLBACK_ESTABLISHED \n");
                string protocol = server->__getProtocolFromHeader__(wsi);
                LOG_DEBUG("user : %s \n", protocol.data());

                int client_fd = lws_get_socket_fd(wsi);
                struct sockaddr_in clientaddr;
                socklen_t peeraddrlen = sizeof(clientaddr);
                if ( 0 > getpeername(client_fd, (struct sockaddr*)&clientaddr, &peeraddrlen) )
                {
                    perror("getpeername error ");
                }

                size_t client_id = server->addClientInfo(ClientInfoPtr(
                    new WebSocketTcpClientInfo(client_fd, &clientaddr, wsi, protocol)));
                NOTIFY_CLIENT_CONNECTED(server->m_server_id, client_id);
            }
            break;
        case LWS_CALLBACK_RECEIVE:
            LOG_DEBUG("LWS_CALLBACK_RECEIVE \n");
            {
                LOG_DEBUG("received data: %s\n", (char *) in);
                int client_fd = lws_get_socket_fd(wsi);
                auto client = static_pointer_cast<WebSocketTcpClientInfo>(server->findClientInfo(client_fd));
                NOTIFY_SERVER_RECEIVED_FROM_PROTOCOL(
                        server->m_server_id, client->getClientId(), (char*)in, len, client->getProtocol());
            }
            break;
        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
            LOG_DEBUG("LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION \n");
            break;
        case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
            LOG_DEBUG("LWS_CALLBACK_WS_PEER_INITIATED_CLOSE \n");
            break;
        case LWS_CALLBACK_CLOSED:
            LOG_DEBUG("LWS_CALLBACK_CLOSED \n");
            {
                int client_fd = lws_get_socket_fd(wsi);
                size_t client_id = server->removeClientInfo(client_fd);
                NOTIFY_CLIENT_DISCONNECTED(server->m_server_id, client_id);
            }
            break;
        case LWS_CALLBACK_PROTOCOL_INIT:
            LOG_DEBUG("LWS_CALLBACK_PROTOCOL_INIT \n");
            break;
        case LWS_CALLBACK_PROTOCOL_DESTROY:
            LOG_DEBUG("LWS_CALLBACK_PROTOCOL_DESTROY \n");
            break;
        case LWS_CALLBACK_GET_THREAD_ID: /* to be silent */
            break;
        case LWS_CALLBACK_DEL_POLL_FD:
            LOG_DEBUG("LWS_CALLBACK_DEL_POLL_FD \n");
            {
                int client_fd = lws_get_socket_fd(wsi);
                size_t client_id = server->removeClientInfo(client_fd);
                NOTIFY_CLIENT_DISCONNECTED(server->m_server_id, client_id);
            }
            break;
        default:
            LOG_WARNING("Unhandled callback reason [%d] \n", reason);
            break;
    }
    return 0;
}

} // end of namespace
