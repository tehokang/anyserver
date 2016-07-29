#include "controller.h"
#include "macro.h"
#include "logger.h"

#include <csignal>

namespace anyserver
{

Controller::Controller(const string config_file)
    : m_anyserver_factory(nullptr)
    , m_argc(0)
    , m_argv(nullptr)
    , m_config_file(config_file)
    , m_listener(nullptr)
{
    LOG_DEBUG("\n");
}

Controller::Controller(int argc, char **argv)
    : m_anyserver_factory(nullptr)
    , m_argc(argc)
    , m_argv(argv)
    , m_listener(nullptr)
{
    LOG_DEBUG("\n");
}

Controller::~Controller()
{
    LOG_DEBUG("\n");
    __deinit__();

    SAFE_DELETE(m_anyserver_factory);
}

bool Controller::init()
{
    LOG_DEBUG("\n");
    m_anyserver_factory = new ServerFactory();
    RETURN_FALSE_IF_NULL(m_anyserver_factory);
    m_anyserver_factory->addEventListener(this);
    RETURN_FALSE_IF_FALSE(m_anyserver_factory->init(m_config_file));

    return true;
}

void Controller::__deinit__()
{
    LOG_DEBUG("\n");
    m_anyserver_factory->removeEventListener(nullptr);
}

bool Controller::start()
{
    LOG_DEBUG("\n");

    RETURN_FALSE_IF_FALSE(m_anyserver_factory->start());

    return true;
}

void Controller::stop()
{
    LOG_DEBUG("\n");
    m_anyserver_factory->stop();
}

void Controller::setLogLevel(bool debug, bool info, bool warn, bool error)
{
    LOG_DEBUG("\n");
    Logger::setLogLevel(info, debug, warn, error);
}

void Controller::onClientConnected(size_t server_id, size_t client_id)
{
    LOG_DEBUG("client[0x%x] connected to server[0x%x] \n", client_id, server_id);
}

void Controller::onClientDisconnected(size_t server_id, size_t client_id)
{
    LOG_DEBUG("client[0x%x] disconnected from server[0x%x] \n", client_id, server_id);
}

void Controller::onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("Received msg[client:0x%x, server:0x%x] : %s (length: %d) \n",
            client_id, server_id, msg, msg_len);
}

}
