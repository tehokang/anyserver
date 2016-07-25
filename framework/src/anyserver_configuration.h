#ifndef __ANYSERVER_CONFIGURATION_H__
#define __ANYSERVER_CONFIGURATION_H__

#include <string>
#include <vector>
#include <list>
#include <memory>
#include <map>

#include "json/json.h"
using namespace std;

namespace anyserver
{

class AnyServerConfiguration
{
public:
    AnyServerConfiguration();
    virtual ~AnyServerConfiguration();
    bool init(const string config_file);

    typedef enum
    {
        WEBSOCKET,
        INETDS,
        UNIXDS,
        HTTP,
        SERVER_KINDS_NUM
    } ServerKinds;

    class ServerInfo
    {
    public:
        ServerInfo(const string _header, const ServerKinds _kinds)
            : header(_header), bind(""), enable(false)
            , kinds(_kinds), tcp(true), max_client(200)
        {
        }
        string header;
        string bind;
        bool enable;
        bool tcp;
        unsigned int max_client;
        ServerKinds kinds;
    };
    typedef shared_ptr<ServerInfo> ServerInfoPtr;
    typedef list<ServerInfoPtr> ServerInfoList;

    class Capabilities
    {
    public:
        Capabilities()
            :  enable_security(false)
        {

        }

        bool enable_security;
        list<string> m_ssl_cert_list;
        list<string> m_ssl_private_key_list;
        list<string> m_ssl_ca_list;
    };

    class Configuration
    {
    public:
        Configuration()
            : name(""), enable_log(true), log_file(""), version(""), copyright("")
        {
        };
        ~Configuration(){};

        string name;
        ServerInfoList server_infos;
        Capabilities capabilities;
        bool enable_log;
        string log_file;
        string version;
        string copyright;
    };

    const Configuration& getConfiguration() { return m_configuration; };
    map<string, ServerKinds> getSupportedServer() { return m_server_kinds; };
protected:
    virtual bool __parse__(const string config_file);
    virtual void __subparse_server_list__(Json::Value &root);
    virtual void __subparse_capabilities__(Json::Value &root);
    virtual void __subparse_log__(Json::Value &root);
    virtual void __subparse_common__(Json::Value &root);

    Configuration m_configuration;
    /**
     * @todo How to make constant type
     */
    map<string, ServerKinds> m_server_kinds;

};

} // end of nameserver

#endif
