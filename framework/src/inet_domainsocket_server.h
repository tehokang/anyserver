#ifndef __INET_DOMAIN_SOCKET_SERVER_H__
#define __INET_DOMAIN_SOCKET_SERVER_H__

#include "anyserver.h"

namespace anyserver
{

class InetDomainSocketServer : public AnyServer
{
public:
    InetDomainSocketServer(
            const string name, const string bind, const unsigned int max_client=200);
    virtual ~InetDomainSocketServer();

protected:
    virtual bool init() override;
    virtual void deinit() override;
    virtual bool start() override;
    virtual void stop() override;
};

} // end of namespace

#endif
