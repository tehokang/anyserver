#include "anymacro.h"
#include "utilities/filesystem.h"
#include "anyserver_configuration.h"

#include <iostream>
#include <fstream>
#include <exception>

namespace anyserver
{

AnyServerConfiguration::AnyServerConfiguration()
{
    LOG_DEBUG("\n");
    m_server_kinds["websocket"] = ServerKinds::WEBSOCKET;
    m_server_kinds["http"] = ServerKinds::HTTP;
    m_server_kinds["inet_domainsocket"] = ServerKinds::INETDS;
    m_server_kinds["unix_domainsocket"] = ServerKinds::UNIXDS;
}

AnyServerConfiguration::~AnyServerConfiguration()
{
    LOG_DEBUG("\n");
}

bool AnyServerConfiguration::init(const string config_file)
{
    LOG_DEBUG("\n");
    if ( false == FileSystem::isExistFile(config_file) )
    {
        LOG_ERROR("Not found configuration file [%s] \n", config_file.data());
        return false;
    }
    return __parse__(config_file);
}

bool AnyServerConfiguration::__parse__(string file)
{
    LOG_DEBUG("\n");
    Json::Value root;
    Json::Reader reader;
    LOG_DEBUG("conf file : %s \n", file.data());

    try
    {
        ifstream config_file(file, ifstream::binary);
        if ( false == reader.parse(config_file, root, false) )
        {
            LOG_WARNING("Failed to parse configuration \n"
                        "%s \n", reader.getFormattedErrorMessages().data());
            return false;
        }

        m_configuration.name = root.get("name", "anyserver").asString();
        LOG_DEBUG("Server name : %s \n", m_configuration.name.data());

        __subparse_common__(root);
        __subparse_server_list__(root);
        __subparse_capabilities__(root);
        __subparse_log__(root);
    }
    catch(exception &e)
    {
        LOG_ERROR("Failed to load config [%s] \n", e.what());
        return false;
    }
    return true;
}

void AnyServerConfiguration::__subparse_server_list__(Json::Value &root)
{
    /**
     * @note Parse type of server supporting
     */
    const Json::Value server_list = root["server_list"];
    LOG_DEBUG("server count : %d \n", server_list.size());

    for ( int i=0; i<server_list.size(); i++ )
    {
        const Json::Value server = server_list[i];

        map<string, ServerKinds>::const_iterator it = m_server_kinds.find(server["type"].asString());
        if ( it != m_server_kinds.end() )
        {
            ServerInfoPtr new_server = ServerInfoPtr(new ServerInfo(it->first, it->second));
            new_server->enable = server["enable"].asBool();
            new_server->bind = server["bind"].asString();
            new_server->tcp = server["tcp"].asBool();

            LOG_DEBUG("server : %s, enable : %d, bind : %s, tcp : %d \n",
                    new_server->header.data(), new_server->enable,
                    new_server->bind.data(), new_server->tcp);

            m_configuration.server_infos.push_back(new_server);
        }
    }
}

void AnyServerConfiguration::__subparse_capabilities__(Json::Value &root)
{
    /**
      * @note Parse capabilities of server
      */
    const Json::Value capabilities = root["capabilities"];
    m_configuration.capabilities.max_client = capabilities["max_client"].asUInt();
    m_configuration.capabilities.enable_security = capabilities["security"].asBool();

    LOG_DEBUG("[Capabilities] \n");
    LOG_DEBUG("max_client : %d \n", m_configuration.capabilities.max_client);
    LOG_DEBUG("enable_security : %d \n", m_configuration.capabilities.enable_security);
}

void AnyServerConfiguration::__subparse_log__(Json::Value &root)
{
    /**
      * @note Parse log configuration
      */
    m_configuration.enable_log = root["log"]["enable"].asBool();
    m_configuration.log_file = root["log"]["file"].asString();
    LOG_DEBUG("enable_log : %d \n", m_configuration.enable_log);
    LOG_DEBUG("log_file : %s \n", m_configuration.log_file.data());
}

void AnyServerConfiguration::__subparse_common__(Json::Value &root)
{
    /**
     * @note Parse version of server
     */
    m_configuration.version = root["version"].asString();
    LOG_DEBUG("[Version] : %s \n", m_configuration.version.data());

    /**
     * @note Parse copyright of server
     */
    m_configuration.copyright = root["copyright"].asString();
    LOG_DEBUG("[Copyright] : %s \n", m_configuration.copyright.data());
}

} // end of namespace
