#include "macro.h"
#include "filesystem.h"
#include "configuration.h"

#include <iostream>
#include <fstream>
#include <exception>

namespace anyserver
{

Configuration::Configuration()
{
    LOG_DEBUG("\n");
    m_server_kinds["websocket"] = ServerKinds::WEBSOCKET;
    m_server_kinds["http"] = ServerKinds::HTTP;
    m_server_kinds["inet_domainsocket"] = ServerKinds::INETDS;
    m_server_kinds["unix_domainsocket"] = ServerKinds::UNIXDS;
}

Configuration::~Configuration()
{
    LOG_DEBUG("\n");
}

bool Configuration::init(const string config_file)
{
    LOG_DEBUG("\n");
    if ( false == FileSystem::isExistFile(config_file) )
    {
        LOG_ERROR("Not found configuration file [%s] \n", config_file.data());
        return false;
    }
    return __parse__(config_file);
}

bool Configuration::__parse__(string file)
{
    LOG_DEBUG("\n");
    Json::Value root;
    Json::Reader reader;
    LOG_DEBUG("config file : %s \n", file.data());

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
        __subparse_test__(root);
    }
    catch(exception &e)
    {
        LOG_ERROR("Failed to load config [%s] \n", e.what());
        return false;
    }
    return true;
}

void Configuration::__subparse_server_list__(Json::Value &root)
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
            ServerInfoPtr new_server = ServerInfoPtr(new ServerInfo(string(it->first), it->second));
            new_server->enable = server["enable"].asBool();
            new_server->bind = server["bind"].asString();
            new_server->tcp = server["tcp"].asBool();
            new_server->max_client = server["max_client"].asUInt();

            LOG_DEBUG("server : %s, enable : %d, bind : %s, tcp : %d, max : %d \n",
                    new_server->header.data(), new_server->enable,
                    new_server->bind.data(), new_server->tcp,
                    new_server->max_client);

            if ( false == server["protocols"].isNull() )
            {
                for( int i=0; i<server["protocols"].size(); i++ )
                {
                    new_server->protocols.push_back(server["protocols"][i].asString());
                    LOG_DEBUG("pushed a protocol : %s \n", server["protocols"][i].asString().data());
                }
            }

            m_configuration.server_infos.push_back(new_server);
        }
        else
        {
            LOG_WARNING("Unknown and unsupported server : %s \n", server["type"].asString().data());
            LOG_WARNING("anyserver support types of following server \n");
            for (std::map<string,ServerKinds>::iterator it =m_server_kinds.begin();
                    it!=m_server_kinds.end(); ++it)
            {
                LOG_WARNING("- \"%s\" \n", (it->first).data());
            }
        }
    }
}

void Configuration::__subparse_capabilities__(Json::Value &root)
{
    /**
      * @note Parse capabilities of server
      */
    const Json::Value capabilities = root["capabilities"];
    m_configuration.capabilities.enable_security = capabilities["security"].asBool();

    LOG_DEBUG("[Capabilities] \n");

    LOG_DEBUG("enable_security : %d \n", m_configuration.capabilities.enable_security);

    m_configuration.capabilities.ssl_cert = capabilities["ssl_cert_file"].asString();
    m_configuration.capabilities.ssl_private_key = capabilities["ssl_private_key_file"].asString();
    m_configuration.capabilities.ssl_ca = capabilities["ssl_ca_file"].asString();

    LOG_DEBUG("ssl_cert : %s \n", m_configuration.capabilities.ssl_cert.data());
    LOG_DEBUG("ssl_private_key : %s \n", m_configuration.capabilities.ssl_private_key.data());
    LOG_DEBUG("ssl_ca : %s \n", m_configuration.capabilities.ssl_ca.data());
}

void Configuration::__subparse_log__(Json::Value &root)
{
    /**
     * @note Parse log configuration
     */
    m_configuration.log.enable_debug = root["log"]["enable_debug"].asBool();
    m_configuration.log.enable_info = root["log"]["enable_info"].asBool();
    m_configuration.log.enable_error = root["log"]["enable_error"].asBool();
    m_configuration.log.enable_warn = root["log"]["enable_warn"].asBool();
    m_configuration.log.enable_filewrite = root["log"]["enable_filewrite"].asBool();
    m_configuration.log.directory = root["log"]["directory"].asString();
    m_configuration.log.filesize = root["log"]["filesize"].asUInt();

    LOG_DEBUG("enable_log_debug : %d \n", m_configuration.log.enable_debug);
    LOG_DEBUG("enable_log_info : %d \n", m_configuration.log.enable_info);
    LOG_DEBUG("enable_log_error : %d \n", m_configuration.log.enable_error);
    LOG_DEBUG("enable_log_warn : %d \n", m_configuration.log.enable_warn);
    LOG_DEBUG("enable_filewrite : %d \n", m_configuration.log.enable_filewrite);
    LOG_DEBUG("log_directory : %s \n", m_configuration.log.directory.data());
    LOG_DEBUG("filesize : %d \n", m_configuration.log.filesize);
}

void Configuration::__subparse_test__(Json::Value &root)
{
    /**
     * @note Parse test object
     */
    m_configuration.enable_echo_test = root["test"]["echo"].asBool();
    LOG_DEBUG("enable echo test : %d \n", m_configuration.enable_echo_test);
}

void Configuration::__subparse_common__(Json::Value &root)
{
    /**
     * @note Parse copyright of server
     */
    m_configuration.copyright = root["copyright"].asString();
    LOG_DEBUG("[Copyright] \n");
    LOG_DEBUG("copy right : %s \n", m_configuration.copyright.data());
}

} // end of namespace
