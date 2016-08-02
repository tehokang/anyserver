#include "http_tcp_server.h"

namespace anyserver
{
struct lws_context *HttpTcpServer::m_context = nullptr;
HttpTcpServer::HttpTcpServer(
        const string name, const string bind, const bool tcp, const unsigned int max_client)
    : BaseServer(name, bind, tcp, max_client)
    , m_run_thread(false)
{
    LOG_DEBUG("\n");
    m_protocols[HTTP].name = "http-only";
    m_protocols[HTTP].callback = callback_http;
    m_protocols[HTTP].user = this;
    m_protocols[HTTP].per_session_data_size = 0;
    m_protocols[HTTP].rx_buffer_size = 0;

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
        m_context_create_info.options = 0;
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

bool HttpTcpServer::sendToClient(size_t client_id, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("\n");
    bool ret = true;
    auto client = static_pointer_cast<HttpTcpClientInfo>(findClientInfo(client_id));
    struct lws *wsi = static_cast<struct lws *>(client->m_wsi);

    unsigned char buffer[8*1024];
    memset(buffer, 0x00, sizeof(buffer));
    unsigned char *p = buffer + LWS_SEND_BUFFER_PRE_PADDING;
    unsigned char *end = p + sizeof(buffer) - LWS_SEND_BUFFER_PRE_PADDING;

    if ( lws_add_http_header_status(wsi, 200, &p, end) ||
         lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_SERVER,
                 (unsigned char *)"libwebsockets", 13, &p, end) ||
         lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE,
                 (unsigned char *)"text/plain", 10, &p, end) ||
         lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_ACCESS_CONTROL_ALLOW_ORIGIN,
                 (unsigned char *)"*", 2, &p, end) ||
         lws_add_http_header_content_length(wsi, msg_len, &p, end) ||
         lws_finalize_http_header(wsi, &p, end) ||
         -1 == lws_write(wsi, buffer + LWS_SEND_BUFFER_PRE_PADDING,
                 p - (buffer + LWS_SEND_BUFFER_PRE_PADDING), LWS_WRITE_HTTP_HEADERS) ||
         -1 == lws_write(wsi, (unsigned char*)msg, msg_len, LWS_WRITE_HTTP) )
    {
        LOG_ERROR("Failed to send a message \n");
        ret = false;
    }
    lws_http_transaction_completed(wsi);
    return ret;
}

void HttpTcpServer::__deinit__()
{
    LOG_DEBUG("\n");
    stop();
}

void HttpTcpServer::__log__(int level, const char *line)
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

void* HttpTcpServer::http_thread(void *arg)
{
    LOG_DEBUG("\n");
    HttpTcpServer *server = static_cast<HttpTcpServer*>(arg);
    server->m_run_thread = true;

    while ( server->m_run_thread )
    {
        lws_service(server->m_context, 50);
    }
    return nullptr;
}

/**
 * @ref https://goo.gl/bLpeLH
 */
int HttpTcpServer::callback_http(struct lws *wsi,
        enum lws_callback_reasons reason,
        void *user, void *in, size_t len)
{
    if ( nullptr == m_context ) return 0;

    HttpTcpServer *server = static_cast<HttpTcpServer*>(lws_context_user(m_context));
    size_t &server_id = server->m_server_id;
    static string req_body;

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
            /**
             * @note At this time, http_tcp_server support only POST message
             **/
            req_body = "";
            if ( 0 <  lws_hdr_total_length(wsi, WSI_TOKEN_POST_URI) )
            {
                LOG_DEBUG("Post message from client \n");
                int n=0;
                char arg[128] = {0x00, };
                while ( 0 < lws_hdr_copy_fragment(wsi, arg, sizeof(arg), WSI_TOKEN_HTTP_URI_ARGS, n) )
                {
                    LOG_DEBUG("arg[%d] : %s \n", n, arg);
                    n++;
                }
            }
            break;
        case LWS_CALLBACK_HTTP_BODY:
            LOG_DEBUG("LWS_CALLBACK_HTTP_BODY [in: %s, len: %d] \n", in, len);
            req_body += string((const char*)in, len);
            break;
        case LWS_CALLBACK_HTTP_BODY_COMPLETION:
            LOG_DEBUG("LWS_CALLBACK_HTTP_BODY_COMPLETION [in: %s] \n", req_body.data());
            {
                int client_fd = lws_get_socket_fd(wsi);
                ClientInfoPtr client = server->findClientInfo(client_fd);
                NOTIFY_SERVER_RECEIVED(server_id, client->getClientId(),
                        (char*)req_body.data(), req_body.length());
            }
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

} // end of namespace
