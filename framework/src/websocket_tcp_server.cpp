#include "websocket_tcp_server.h"

namespace anyserver
{
struct lws_context *WebSocketTcpServer::m_context = nullptr;
WebSocketTcpServer::WebSocketTcpServer(const string name, const string bind, const unsigned int max_client)
    : AnyServer(name, bind, max_client)
    , m_run_thread(false)
{
    LOG_DEBUG("\n");
    m_protocols[HTTP].name = "http-only";
    m_protocols[HTTP].callback = callback_http;
    m_protocols[HTTP].user = this;
    m_protocols[HTTP].per_session_data_size = 0;
    m_protocols[HTTP].rx_buffer_size = 0;

    m_protocols[WEBSOCKET].name = "protocol";
    m_protocols[WEBSOCKET].callback = callback_websocket;
    m_protocols[WEBSOCKET].user = this;
    m_protocols[WEBSOCKET].per_session_data_size = 0;
    m_protocols[WEBSOCKET].rx_buffer_size = 0;

    m_protocols[DUMMY].name = nullptr;
    m_protocols[DUMMY].callback = nullptr;
    m_protocols[DUMMY].user = nullptr;
    m_protocols[DUMMY].per_session_data_size = 0;
    m_protocols[DUMMY].rx_buffer_size = 0;
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
    m_context_create_info.port = stoi(m_bind);
    m_context_create_info.iface = nullptr;
    m_context_create_info.protocols = m_protocols;
    m_context_create_info.extensions = lws_get_internal_extensions();
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

void WebSocketTcpServer::__deinit__()
{
    LOG_DEBUG("\n");
    stop();
}

void* WebSocketTcpServer::websocket_thread(void *argv)
{
    LOG_DEBUG("\n");
    WebSocketTcpServer *server = static_cast<WebSocketTcpServer*>(argv);
    server->m_run_thread = true;

    while ( server->m_run_thread )
    {
        lws_service(server->m_context, 50);
    }
    return nullptr;
}

int WebSocketTcpServer::callback_http(struct lws *wsi,
        enum lws_callback_reasons reason,
        void *user, void *in, size_t len)
{
    return 0;
}

int WebSocketTcpServer::callback_websocket(struct lws *wsi,
        enum lws_callback_reasons reason,
        void *user, void *in, size_t len)
{
    LOG_DEBUG("\n");
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

                size_t client_id = server->addClientInfo(ClientInfoPtr(new WebSocketTcpClientInfo(client_fd, &clientaddr)));

                list<IAnyServerListener*> listeners = server->m_listeners;
                for ( list<IAnyServerListener*>::iterator it = listeners.begin();
                        it!=listeners.end(); ++it )
                {
                    IAnyServerListener *listener = (*it);
                    listener->onClientConnected(server_id, client_id);
                }
            }
            break;
        case LWS_CALLBACK_RECEIVE:
            LOG_DEBUG("LWS_CALLBACK_RECEIVE \n");
            {
                LOG_DEBUG("received data: %s\n", (char *) in);
                int client_fd = lws_get_socket_fd(wsi);
                ClientInfoPtr client = server->findClientInfo(client_fd);
                list<IAnyServerListener*> listeners = server->m_listeners;
                for ( list<IAnyServerListener*>::iterator it = listeners.begin();
                        it!=listeners.end(); ++it )
                {
                    IAnyServerListener *listener = (*it);
                    listener->onReceived(server_id, client->getClientId(), (char*)in, len);
                }
#ifdef CONFIG_TEST_ECHO_RESPONSE
                /**
                 * Test echo
                 */
                lws_write(wsi, (unsigned char*)in, len, LWS_WRITE_TEXT);
#endif
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

                list<IAnyServerListener*> listeners = server->m_listeners;
                for ( list<IAnyServerListener*>::iterator it = listeners.begin();
                        it!=listeners.end(); ++it )
                {
                    IAnyServerListener *listener = (*it);
                    listener->onClientDisconnected(server_id, client_id);
                }
            }
            break;
        case LWS_CALLBACK_PROTOCOL_INIT:
            LOG_DEBUG("LWS_CALLBACK_PROTOCOL_INIT \n");
            break;
        default:
            LOG_WARNING("Unhandled callback reason [%d] \n", reason);
            break;
    }
    return 0;
}

} // end of namespace
