#include "anyserver_controller.h"
#include "anymacro.h"
#include "anylogger.h"

#include <csignal>

namespace anyserver
{

AnyServerController::AnyServerController(const string config_file)
    : m_anyserver_factory(nullptr)
    , m_argc(0)
    , m_argv(nullptr)
    , m_config_file(config_file)
    , m_listener(nullptr)
{
    LOG_DEBUG("\n");
}

AnyServerController::AnyServerController(int argc, char **argv)
    : m_anyserver_factory(nullptr)
    , m_argc(argc)
    , m_argv(argv)
    , m_listener(nullptr)
{
    LOG_DEBUG("\n");
}

AnyServerController::~AnyServerController()
{
    LOG_DEBUG("\n");
    __deinit__();

    SAFE_DELETE(m_anyserver_factory);
}

bool AnyServerController::init()
{
    LOG_DEBUG("\n");
    std::vector<int> signal_ids;
    signal_ids.push_back(SIGINT);
    signal_ids.push_back(SIGQUIT);
    signal_ids.push_back(SIGTERM);
    signal_ids.push_back(SIGHUP);
    signal_ids.push_back(SIGPIPE);
    posix_signal_interceptor.HandleSignals(
            signal_ids,
            std::bind1st(std::mem_fun(&AnyServerController::onReceivedPosixSignal), this));

    m_anyserver_factory = new AnyServerFactory();
    RETURN_FALSE_IF_NULL(m_anyserver_factory);
    m_anyserver_factory->addEventListener(this);
    RETURN_FALSE_IF_FALSE(m_anyserver_factory->init(m_config_file));

    return true;
}

void AnyServerController::__deinit__()
{
    LOG_DEBUG("\n");
    m_anyserver_factory->removeEventListener(nullptr);
}

bool AnyServerController::start()
{
    LOG_DEBUG("\n");

    RETURN_FALSE_IF_FALSE(m_anyserver_factory->start());

    return true;
}

void AnyServerController::stop()
{
    LOG_DEBUG("\n");
    m_anyserver_factory->stop();
}

void AnyServerController::onReceivedPosixSignal(int signal_id)
{
    RETURN_IF_NULL(m_listener);
    m_listener->onReceivedSystemSignal(signal_id);
}

void AnyServerController::setLogLevel(bool debug, bool info, bool warn, bool error)
{
    LOG_DEBUG("\n");
    AnyLogger::setLogLevel(info, debug, warn, error);
}

void AnyServerController::onClientConnected(size_t server_id, size_t client_id)
{
    LOG_DEBUG("client[0x%x] connected to server[0x%x] \n", client_id, server_id);
}

void AnyServerController::onClientDisconnected(size_t server_id, size_t client_id)
{
    LOG_DEBUG("client[0x%x] disconnected from server[0x%x] \n", client_id, server_id);
}

void AnyServerController::onReceive(size_t server_id, size_t client_id, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("Received msg[client:0x%x, server:0x%x] : %s (length: %d) \n",
            client_id, server_id, msg, msg_len);
}

}
