#include "json/json.h"
#include "anymacro.h"
#include "utilities.h"
#include "server_configuration.h"

#include <iostream>
#include <fstream>

namespace anyserver
{

ServerConfiguration::ServerInfo ServerConfiguration::m_server_info;
bool ServerConfiguration::init(string file)
{
    if ( Utilities::isExistFile(file) )
    {
        return __parse__(file);
    }
    return false;
}

bool ServerConfiguration::__parse__(string file)
{
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

    /**
     * @note Parse type of server supporting
     */
    const Json::Value types = root["type"];
    const Json::Value::Members server_kinds = types.getMemberNames();

    for(Json::Value::Members::const_iterator it=server_kinds.begin();
            it!=server_kinds.end(); ++it)
    {
        string name = (*it);

        list<ServerType> supported_servers = m_server_info.supported_servers;
        for ( list<ServerType>::iterator it=supported_servers.begin();
                it!=supported_servers.end(); ++it )
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

    /**
     * @note Parse version of server
     */

    /**
     * @note Parse copyright of server
     */

    return true;
}

} // end of namespace
