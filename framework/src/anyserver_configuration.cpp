#include "json/json.h"
#include "anymacro.h"
#include "utilities/filesystem.h"
#include "anyserver_configuration.h"

#include <iostream>
#include <fstream>

namespace anyserver
{

AnyServerConfiguration::AnyServerConfiguration()
{
    LOG_DEBUG("\n");
}

AnyServerConfiguration::~AnyServerConfiguration()
{
    LOG_DEBUG("\n");
}

bool AnyServerConfiguration::init(const string config_file)
{
    LOG_DEBUG("\n");
    if ( Utilities::isExistFile(config_file) )
    {
        return __parse__(config_file);
    }
    return false;
}

bool AnyServerConfiguration::__parse__(string file)
{
    LOG_DEBUG("\n");
    Json::Value root;
    Json::Reader reader;
    LOG_DEBUG("conf file : %s \n", file.data());

    ifstream config_file(file, ifstream::binary);
    if ( false == reader.parse(config_file, root, false) )
    {
        LOG_WARNING("Failed to parse configuration \n"
                    "%s \n", reader.getFormattedErrorMessages().data());
        return false;
    }

    m_server_info.name = root.get("name", "anyserver").asString();
    LOG_DEBUG("[ServerName] : %s \n", m_server_info.name.data());

    /**
     * @note Parse type of server supporting
     */
    const Json::Value types = root["type"];
    const Json::Value::Members server_kinds = types.getMemberNames();

    LOG_DEBUG("[Type]\n");
    for(Json::Value::Members::const_iterator it=server_kinds.begin();
            it!=server_kinds.end(); ++it)
    {
        string name = (*it);

        list<ServerType> &server_types = m_server_info.server_types;
        for ( list<ServerType>::iterator it=server_types.begin();
                it!=server_types.end(); ++it )
        {
            ServerType &server = (*it);
            if ( 0 == name.compare(server.header) )
            {
                server.bind = root["type"][name]["bind"].asString();
                server.enable = root["type"][name]["enable"].asBool();

                LOG_DEBUG("server : %s, enable : %d, bind : %s \n",
                        server.header.data(),
                        server.enable,
                        server.bind.data());
            }
        }
    }

    /**
     * @note Parse capabilities of server
     */
    const Json::Value capabilities = root["capabilities"];
    m_server_info.capabilities.max_client = capabilities["max_client"].asUInt();
    m_server_info.capabilities.enable_security = capabilities["security"].asBool();
    m_server_info.capabilities.tcp = capabilities["tcp"].asBool();

    LOG_DEBUG("[Capabilities] \n");
    LOG_DEBUG("max_client : %d \n", m_server_info.capabilities.max_client);
    LOG_DEBUG("enable_security : %d \n", m_server_info.capabilities.enable_security);
    LOG_DEBUG("tcp : %d \n", m_server_info.capabilities.tcp);

    /**
     * @note Parse log configuration
     */
    m_server_info.enable_log = root["log"]["enable"].asBool();
    m_server_info.log_file = root["log"]["file"].asString();
    LOG_DEBUG("enable_log : %d \n", m_server_info.enable_log);
    LOG_DEBUG("log_file : %s \n", m_server_info.log_file.data());
    /**
     * @note Parse version of server
     */
    m_server_info.version = root["version"].asString();
    LOG_DEBUG("[Version] : %s \n", m_server_info.version.data());

    /**
     * @note Parse copyright of server
     */
    m_server_info.copyright = root["copyright"].asString();
    LOG_DEBUG("[Copyright] : %s \n", m_server_info.copyright.data());

    return true;
}

} // end of namespace
