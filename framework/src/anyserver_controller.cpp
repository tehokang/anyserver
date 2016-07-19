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
            std::bind1st(std::mem_fun(&AnyServerController::onReceivedPosixSignal),this));

    m_anyserver_factory = new AnyServerFactory();
    RETURN_FALSE_IF_NULL(m_anyserver_factory);
    m_anyserver_factory->setEventListener(this);
    RETURN_FALSE_IF_FALSE(m_anyserver_factory->init(m_config_file));

    return true;
}

void AnyServerController::__deinit__()
{
    LOG_DEBUG("\n");
    m_anyserver_factory->setEventListener(nullptr);
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

void AnyServerController::onClientConnected(int fd, string ip_address, int port)
{
    LOG_DEBUG("\n");
    LOG_DEBUG("client [fd:%d] connected from %s:%d \n", fd, ip_address.data(), port);
}

void AnyServerController::onClientConnected(int fd, string ip_address, string bind)
{
    LOG_DEBUG("\n");
    LOG_DEBUG("client [fd:%d] connected from %s:%s \n", fd, ip_address.data(), bind.data());
}

void AnyServerController::onClientDisconnected(int fd)
{
    LOG_DEBUG("\n");
}

void AnyServerController::onReceive(int fd, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("\n");
    LOG_DEBUG("[TCP] Received msg[fd:%d] : %s (length: %d) \n", fd, msg, msg_len);
}

void AnyServerController::onReceive(int sfd, struct sockaddr *client_addr, char *msg, unsigned int msg_len)
{
    LOG_DEBUG("\n");
    LOG_DEBUG("[UDP] Received msg[fd:%d] : %s (length: %d) \n", sfd, msg, msg_len);
}

}
