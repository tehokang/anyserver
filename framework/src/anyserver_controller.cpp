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
            std::bind1st(std::mem_fun(&AnyServerController::onReceivedPosixSignal),this));

    m_anyserver_factory = new AnyServerFactory();
    RETURN_FALSE_IF_NULL(m_anyserver_factory);
    RETURN_FALSE_IF_FALSE(m_anyserver_factory->init(m_config_file));

    return true;
}

void AnyServerController::__deinit__()
{
    LOG_DEBUG("\n");
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

void AnyServerController::onClientConnected(int fd, string ip_address)
{
    LOG_DEBUG("\n");
}

void AnyServerController::onClientDisconnected(int fd)
{
    LOG_DEBUG("\n");
}

void AnyServerController::onReceive(int fd)
{
    LOG_DEBUG("\n");
}

}
