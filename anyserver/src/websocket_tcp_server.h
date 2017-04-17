#ifndef __WEBSOCKET_TCP_SERVER_H__
#define __WEBSOCKET_TCP_SERVER_H__

#include "base_server_impl.h"
#include "libwebsockets.h"

namespace anyserver
{

class WebSocketTcpServer : public BaseServerImpl
{
public:
    WebSocketTcpServer(const string name, const string bind, const bool tcp, const unsigned int max_client=200);
    virtual ~WebSocketTcpServer();

    virtual bool init() override;
    virtual bool start() override;
    virtual void stop() override;
    virtual bool sendToClient(size_t client_id, string protocol, char *msg, unsigned int msg_len) override;
    virtual bool sendToClient(size_t client_id, char *msg, unsigned int msg_len) override;
    virtual void addProtocols(list<string> protocols);

    class WebSocketTcpClientInfo : public TcpClientInfo
    {
    public:
        WebSocketTcpClientInfo(int fd, struct sockaddr_in* sockaddr, void *wsi, string protocol)
            : TcpClientInfo(fd, sockaddr)
            , m_wsi(wsi)
            , m_protocol(protocol)
        {
        }
        void* getWsi() { return m_wsi; };
        string getProtocol() { return m_protocol; };
    protected:
        void *m_wsi;
        string m_protocol;
    };

protected:
    virtual void __deinit__() override;

    static void* websocket_thread(void *arg);
    pthread_t m_websocket_thread;

    enum Protocol
    {
        HTTP,
        TEST,
        DUMMY,
        BASIC_PROTOCOL
    };
private:
    string getProtocolFromHeader(struct lws *wsi);
    string replaceAll(string &str, const string& from, const string& to);

    static int callback_websocket(struct lws *wsi,
            enum lws_callback_reasons reason,
            void *user, void *in, size_t len);

    static void __log__(int level, const char *line);
    static struct lws_context *m_context;
    static struct lws_vhost* m_vhost;

    bool m_run_thread;
    struct lws_protocols *m_lws_protocols;
    struct lws_context_creation_info m_context_create_info;

};

} // end of namespace

#endif
