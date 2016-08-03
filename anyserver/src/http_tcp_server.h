#ifndef __HTTP_TCP_SERVER_H__
#define __HTTP_TCP_SERVER_H__

#include "base_server.h"
#include "libwebsockets.h"

namespace anyserver
{

class HttpTcpServer : public BaseServer 
{
public:
    HttpTcpServer(const string name, const string bind, const bool tcp, const unsigned int max_client=200);
    virtual ~HttpTcpServer();

    virtual bool init() override;
    virtual bool start() override;
    virtual void stop() override;
    virtual bool sendToClient(size_t client_id, char *msg, unsigned int msg_len) override;

    class HttpTcpClientInfo : public TcpClientInfo
    {
    public:
        HttpTcpClientInfo(int fd, struct sockaddr_in* sockaddr, void *wsi)
            : TcpClientInfo(fd, sockaddr)
            , m_wsi(wsi)
        {
        }
        void* getWsi() { return m_wsi; };
        void *m_wsi;
    };


protected:
    virtual void __deinit__() override;

    static void* http_thread(void *arg);
    pthread_t m_http_thread;

    enum { HTTP, DUMMY, MAX_SERVER };
private:
    static int callback_http(struct lws *wsi,
            enum lws_callback_reasons reason,
            void *user, void *in, size_t len);

    static void __log__(int level, const char *line);
    static struct lws_context *m_context;

    bool m_run_thread;
    struct lws_protocols m_protocols[MAX_SERVER];
    struct lws_context_creation_info m_context_create_info;

};

} // end of namespace

#endif
