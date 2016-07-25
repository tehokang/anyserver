#include "http_tcp_server.h"

namespace anyserver
{
struct lws_context *HttpTcpServer::m_context = nullptr;
HttpTcpServer::HttpTcpServer(
        const string name, const string bind, const bool tcp, const unsigned int max_client)
    : AnyServer(name, bind, tcp, max_client)
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

HttpTcpServer::~HttpTcpServer()
{
    LOG_DEBUG("\n");
    __deinit__();
    lws_context_destroy(m_context);
}

bool HttpTcpServer::init()
{
    LOG_DEBUG("\n");
    memset(&m_context_create_info, 0x00, sizeof(m_context_create_info));

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

bool HttpTcpServer::start()
{
    LOG_DEBUG("\n");
    if ( 0 != pthread_create(
            &m_http_thread, NULL, HttpTcpServer::http_thread, (void*)this) )
    {
        LOG_ERROR("Failed to create thread \n");
        return false;
    }
    return true;
}

void HttpTcpServer::stop()
{
    LOG_DEBUG("\n");
    m_run_thread = false;
}

void HttpTcpServer::__deinit__()
{
    LOG_DEBUG("\n");
    stop();
}

void* HttpTcpServer::http_thread(void *argv)
{
    LOG_DEBUG("\n");
    HttpTcpServer *server = static_cast<HttpTcpServer*>(argv);
    server->m_run_thread = true;

    while ( server->m_run_thread )
    {
        lws_service(server->m_context, 50);
    }
    return nullptr;
}

/**
 * @ref https://gitlab.com/libwebsockets/libwebsockets/blob/6b5de70f4fb1eadac6730f3b4ecfe156bd38567a/test-server/test-server-http.c
 */
int HttpTcpServer::callback_http(struct lws *wsi,
        enum lws_callback_reasons reason,
        void *user, void *in, size_t len)
{
    if ( nullptr == m_context ) return 0;

    HttpTcpServer *server = static_cast<HttpTcpServer*>(lws_context_user(m_context));
    size_t &server_id = server->m_server_id;

    switch ( reason )
    {
        case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
            LOG_DEBUG("LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED \n");
            {
                int client_fd = lws_get_socket_fd(wsi);
                struct sockaddr_in clientaddr;
                socklen_t peeraddrlen = sizeof(clientaddr);
                if ( 0 > getpeername(client_fd, (struct sockaddr*)&clientaddr, &peeraddrlen) )
                {
                    perror("getpeername error ");
                }

                size_t client_id = server->addClientInfo(ClientInfoPtr(new HttpTcpClientInfo(client_fd, &clientaddr, wsi)));
                NOTIFY_CLIENT_CONNECTED(server_id, client_id);
            }
            break;
        case LWS_CALLBACK_CLOSED_HTTP:
            LOG_DEBUG("LWS_CALLBACK_CLOSED_HTTP \n");
            {
                int client_fd = lws_get_socket_fd(wsi);
                size_t client_id = server->removeClientInfo(client_fd);
                NOTIFY_CLIENT_DISCONNECTED(server_id, client_id);
            }
            break;
        case LWS_CALLBACK_HTTP:
            LOG_DEBUG("LWS_CALLBACK_HTTP [in: %s]\n", in);
            {
                int client_fd = lws_get_socket_fd(wsi);
                ClientInfoPtr client = server->findClientInfo(client_fd);
                NOTIFY_SERVER_RECEIVED(server_id, client->getClientId(), (char*)in, len);
            }
#ifdef CONFIG_TEST_ECHO_RESPONSE
            {
                /**
                 * Test echo
                 */
                char the_response[] = "Hello, World!";
                int strl = strlen(the_response);

                unsigned char *p;
                unsigned char *end;
                static unsigned char buffer[8*1024];

                p = buffer + LWS_SEND_BUFFER_PRE_PADDING;
                end = p + sizeof(buffer) - LWS_SEND_BUFFER_PRE_PADDING;

                if ( lws_add_http_header_status(wsi, 200, &p, end) ) return 1;
                if ( lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_SERVER,
                        (unsigned char *)"libwebsockets", 13, &p, end) ) return 1;
                if ( lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE,
                        (unsigned char *)"text/plain", 10, &p, end) ) return 1;
                if ( lws_add_http_header_content_length(wsi, strl, &p, end) ) return 1;
                if ( lws_finalize_http_header(wsi, &p, end) ) return 1;

                lws_write(wsi, buffer + LWS_SEND_BUFFER_PRE_PADDING,
                        p - (buffer + LWS_SEND_BUFFER_PRE_PADDING), LWS_WRITE_HTTP_HEADERS);
                lws_write(wsi, (unsigned char*)the_response,
                        strl, LWS_WRITE_HTTP);
            }
#endif
            break;
        case LWS_CALLBACK_HTTP_BODY:
            LOG_DEBUG("LWS_CALLBACK_HTTP_BODY [in: %s] \n", in);
            break;
        case LWS_CALLBACK_HTTP_BODY_COMPLETION:
            LOG_DEBUG("LWS_CALLBACK_HTTP_BODY_COMPLETION [in: %s] \n", in);
            break;
        case LWS_CALLBACK_HTTP_WRITEABLE:
            LOG_DEBUG("LWS_CALLBACK_HTTP_WRITEABLE \n");
            break;
        case LWS_CALLBACK_PROTOCOL_INIT:
            LOG_DEBUG("LWS_CALLBACK_PROTOCOL_INIT \n");
            break;
        case LWS_CALLBACK_GET_THREAD_ID:
            break;
        default:
            LOG_WARNING("Unhandled callback reason [%d] \n", reason);
            break;
    }
    return 0;
}

int HttpTcpServer::callback_websocket(struct lws *wsi,
        enum lws_callback_reasons reason,
        void *user, void *in, size_t len)
{
    return 0;
}

} // end of namespace
