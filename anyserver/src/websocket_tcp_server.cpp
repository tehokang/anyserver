#include "websocket_tcp_server.h"

namespace anyserver
{
struct lws_context *WebSocketTcpServer::m_context = nullptr;
WebSocketTcpServer::WebSocketTcpServer(
        const string name, const string bind, const bool tcp, const unsigned int max_client)
    : BaseServer(name, bind, tcp, max_client)
    , m_run_thread(false)
{
    LOG_DEBUG("\n");
    /**
     * @note None protocol request will patch from callback_websocket
     */
    m_protocols[HTTP].name = "http-only";
    m_protocols[HTTP].callback = callback_websocket;
    m_protocols[HTTP].user = this;
    m_protocols[HTTP].per_session_data_size = 0;
    m_protocols[HTTP].rx_buffer_size = 0;

    /**
     * @note Specific protocol request will also patch from callback_websocket.
     */
    m_protocols[WEBSOCKET_PROTOCOL_A].name = "protocol_a";
    m_protocols[WEBSOCKET_PROTOCOL_A].callback = callback_websocket;
    m_protocols[WEBSOCKET_PROTOCOL_A].user = this;
    m_protocols[WEBSOCKET_PROTOCOL_A].per_session_data_size = 0;
    m_protocols[WEBSOCKET_PROTOCOL_A].rx_buffer_size = 0;

    m_protocols[DUMMY].name = nullptr;
    m_protocols[DUMMY].callback = nullptr;
    m_protocols[DUMMY].user = nullptr;
    m_protocols[DUMMY].per_session_data_size = 0;
    m_protocols[DUMMY].rx_buffer_size = 0;

    lws_set_log_level(
            LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO |
            LLL_DEBUG | LLL_PARSER | LLL_HEADER | LLL_EXT | LLL_CLIENT | LLL_LATENCY,
            __log__);
}

WebSocketTcpServer::~WebSocketTcpServer()
{
    LOG_DEBUG("\n");
    __deinit__();
    lws_context_destroy(m_context);
}

bool WebSocketTcpServer::init()
{
    LOG_DEBUG("\n");
    memset(&m_context_create_info, 0x00, sizeof(m_context_create_info));

    m_context_create_info.port = stoi(m_bind);
    m_context_create_info.iface = nullptr;
    m_context_create_info.protocols = m_protocols;
    m_context_create_info.extensions = nullptr;
    m_context_create_info.keepalive_timeout = 60;
    m_context_create_info.ssl_private_key_filepath = nullptr;
    m_context_create_info.ssl_cert_filepath = nullptr;
    m_context_create_info.server_string = m_name.data();
    m_context_create_info.options = 0;
    m_context_create_info.fd_limit_per_thread = m_max_client;
    m_context_create_info.user = this;

    m_context = lws_create_context(&m_context_create_info);
    if ( nullptr == m_context )
    {
        LOG_ERROR("context has nullptr \n");
        return false;
    }
    return true;
}

bool WebSocketTcpServer::start()
{
    LOG_DEBUG("\n");
    if ( 0 != pthread_create(
            &m_websocket_thread, NULL, WebSocketTcpServer::websocket_thread, (void*)this) )
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

void* WebSocketTcpServer::websocket_thread(void *arg)
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

int WebSocketTcpServer::callback_websocket(struct lws *wsi,
        enum lws_callback_reasons reason,
        void *user, void *in, size_t len)
{
    if ( nullptr == m_context ) return 0;

    WebSocketTcpServer *server = static_cast<WebSocketTcpServer*>(lws_context_user(m_context));
    size_t &server_id = server->m_server_id;

    switch ( reason )
    {
        case LWS_CALLBACK_ESTABLISHED:
            LOG_DEBUG("LWS_CALLBACK_ESTABLISHED \n");
            {
                int client_fd = lws_get_socket_fd(wsi);
                struct sockaddr_in clientaddr;
                socklen_t peeraddrlen = sizeof(clientaddr);
                if ( 0 > getpeername(client_fd, (struct sockaddr*)&clientaddr, &peeraddrlen) )
                {
                    perror("getpeername error ");
                }

                size_t client_id = server->addClientInfo(ClientInfoPtr(new WebSocketTcpClientInfo(client_fd, &clientaddr, wsi)));
                NOTIFY_CLIENT_CONNECTED(server_id, client_id);
            }
            break;
        case LWS_CALLBACK_RECEIVE:
            LOG_DEBUG("LWS_CALLBACK_RECEIVE \n");
            {
                LOG_DEBUG("received data: %s\n", (char *) in);
                int client_fd = lws_get_socket_fd(wsi);
                ClientInfoPtr client = server->findClientInfo(client_fd);
                NOTIFY_SERVER_RECEIVED(server_id, client->getClientId(), (char*)in, len);
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
                NOTIFY_CLIENT_DISCONNECTED(server_id, client_id);
            }
            break;
        case LWS_CALLBACK_PROTOCOL_INIT:
            LOG_DEBUG("LWS_CALLBACK_PROTOCOL_INIT \n");
            break;
        case LWS_CALLBACK_PROTOCOL_DESTROY:
            LOG_DEBUG("LWS_CALLBACK_PROTOCOL_DESTROY \n");
            break;
        default:
//            LOG_WARNING("Unhandled callback reason [%d] \n", reason);
            break;
    }
    return 0;
}

} // end of namespace
