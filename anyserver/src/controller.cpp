#include "controller.h"
#include "icontroller.h"
#include "macro.h"
#include "logger.h"

#include <csignal>

namespace anyserver
{

Controller::Controller(const string config_file)
    : m_server_factory(nullptr)
    , m_argc(0)
    , m_argv(nullptr)
    , m_config_file(config_file)
    , m_listener(nullptr)
    , m_posix_signal_interceptor(new PosixSignalInterceptor())
{
    LOG_DEBUG("\n");
}

Controller::Controller(int argc, char **argv)
    : m_server_factory(nullptr)
    , m_argc(argc)
    , m_argv(argv)
    , m_listener(nullptr)
    , m_posix_signal_interceptor(new PosixSignalInterceptor())
{
    LOG_DEBUG("\n");
}

Controller::~Controller()
{
    LOG_DEBUG("\n");
    __deinit__();

    SAFE_DELETE(m_server_factory);
    SAFE_DELETE(m_posix_signal_interceptor);
}

bool Controller::init()
{
    LOG_DEBUG("\n");
    m_server_factory = new ServerFactory();
    RETURN_FALSE_IF_NULL(m_server_factory);
    m_server_factory->addEventListener(this);
    RETURN_FALSE_IF_FALSE(m_server_factory->init(m_config_file));

    std::vector<int> signal_ids;
    signal_ids.push_back(SIGINT);
    signal_ids.push_back(SIGQUIT);
    signal_ids.push_back(SIGTERM);
    signal_ids.push_back(SIGHUP);
//    signal_ids.push_back(SIGPIPE);
    m_posix_signal_interceptor->HandleSignals(
            signal_ids,
            std::bind1st(std::mem_fun(&Controller::onReceivedPosixSignal), this));

    return true;
}

void Controller::onReceivedPosixSignal(int signal_id)
{
    RETURN_IF_NULL(m_listener);
    m_listener->onReceivedSystemSignal(signal_id);
}

void Controller::__deinit__()
{
    LOG_DEBUG("\n");
    m_server_factory->removeEventListener(nullptr);
}

bool Controller::start()
{
    LOG_DEBUG("\n");

    RETURN_FALSE_IF_FALSE(m_server_factory->start());

    return true;
}

void Controller::stop()
{
    LOG_DEBUG("\n");
    m_server_factory->stop();
}

void Controller::onClientConnected(size_t server_id, size_t client_id)
{
    LOG_DEBUG("client[0x%x] connected to server[0x%x] \n", client_id, server_id);
    RETURN_IF_NULL(m_listener);
    m_listener->onClientConnected(server_id, client_id);
}

void Controller::onClientDisconnected(size_t server_id, size_t client_id)
{
    LOG_DEBUG("client[0x%x] disconnected from server[0x%x] \n", client_id, server_id);
    RETURN_IF_NULL(m_listener);
    m_listener->onClientDisconnected(server_id, client_id);
}

void Controller::onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("Received msg[client:0x%x, server:0x%x] : %s (length: %d) \n",
            client_id, server_id, msg, msg_len);
    RETURN_IF_NULL(m_listener);
    m_listener->onReceive(server_id, client_id, msg, msg_len);
}

}
