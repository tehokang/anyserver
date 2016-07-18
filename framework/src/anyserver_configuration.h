#ifndef __ANYSERVER_CONFIGURATION_H__
#define __ANYSERVER_CONFIGURATION_H__

#include <string>
#include <vector>
#include <list>
using namespace std;

namespace anyserver
{

class AnyServerConfiguration
{
public:
    AnyServerConfiguration();
    virtual ~AnyServerConfiguration();
    bool init(const string config_file);

    typedef enum { NONE, WEBSOCKET, INETDS, UNIXDS, HTTP } ServerKinds;
    class ServerType
    {
    public:
        ServerType(const string _header, const ServerKinds _kinds)
            : header(_header), bind(""), enable(false), kinds(_kinds), tcp(true)
        {
        }
        string header;
        string bind;
        bool enable;
        bool tcp;
        ServerKinds kinds;
    };

    class Capabilities
    {
    public:
        Capabilities()
            : max_client(200), enable_security(false)
        {

        }
        unsigned int max_client;
        bool enable_security;
    };

    class ServerInfo
    {
    public:
        ServerInfo()
            : name(""), enable_log(true), log_file(""), version(""), copyright("")
        {
            server_types.push_back(ServerType("websocket", WEBSOCKET));
            server_types.push_back(ServerType("http", HTTP));
            server_types.push_back(ServerType("inet_domainsocket", INETDS));
            server_types.push_back(ServerType("unix_domainsocket", UNIXDS));
        };
        ~ServerInfo(){};

        string name;
        list<ServerType> server_types;
        Capabilities capabilities;
        bool enable_log;
        string log_file;
        string version;
        string copyright;
    };

    const ServerInfo& getServerInfo() { return m_server_info; };

protected:
    virtual bool __parse__(const string config_file);
    ServerInfo m_server_info;
    const string m_config_file;

};

} // end of nameserver

#endif
