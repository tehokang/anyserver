#ifndef __WEBSOCKET_TCP_SERVER_H__
#define __WEBSOCKET_TCP_SERVER_H__

#include "anyserver.h"
#include "libwebsockets.h"

namespace anyserver
{

class WebSocketTcpServer : public AnyServer
{
public:
    WebSocketTcpServer(const string name, const string bind, const unsigned int max_client=200);
    virtual ~WebSocketTcpServer();

    virtual bool init() override;
    virtual bool start() override;
    virtual void stop() override;
protected:
    virtual void __deinit__() override;

    static void* websocket_thread(void *argv);
    pthread_t m_websocket_thread;

    enum { HTTP, WEBSOCKET, DUMMY, MAX_SERVER };
private:
    static int callback_http(struct lws *wsi,
            enum lws_callback_reasons reason,
            void *user, void *in, size_t len);

    static int callback_websocket(struct lws *wsi,
            enum lws_callback_reasons reason,
            void *user, void *in, size_t len);

    static struct lws_context *m_context;

    bool m_run_thread;
    struct lws_protocols m_protocols[MAX_SERVER];
    struct lws_context_creation_info m_context_create_info;

};

} // end of namespace

#endif
