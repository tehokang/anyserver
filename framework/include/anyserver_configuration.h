#ifndef __SERVER_CONFIGURATION_H__
#define __SERVER_CONFIGURATION_H__

#include <string>
#include <vector>
#include <list>
using namespace std;

namespace anyserver
{

class ServerConfiguration
{
public:
    static bool init(string file);

protected:
    static bool __parse__(string file);

    class ServerType
    {
    public:
        ServerType(const string _header)
        {
            header = _header;
        }
        string header;
        string bind;
        bool enable;
    };
    class WebSocket : public ServerType
    {
    public:
        WebSocket() : ServerType("websocket") { }
    };

    class Http : public ServerType
    {
    public:
        Http() : ServerType("http") { }
    };

    class InetDomainSocket : public ServerType
    {
    public:
        InetDomainSocket() : ServerType("inet_domainsocket") { }
    };

    class UnixDomainSocket : public ServerType
    {
    public:
        UnixDomainSocket() : ServerType("unix_domainsocket") { }
    };

    class ServerInfo
    {
    public:
        ServerInfo()
            : name(""), max_clients(200), enable_security(false), version(""), copyright("")
        {
            supported_servers.push_back(WebSocket());
            supported_servers.push_back(Http());
            supported_servers.push_back(InetDomainSocket());
            supported_servers.push_back(UnixDomainSocket());
        };
        ~ServerInfo(){};

        string name;

        list<ServerType> supported_servers;
        int max_clients;
        bool enable_security;
        string version;
        string copyright;
    };


    static ServerInfo m_server_info;


};

} // end of nameserver

#endif
