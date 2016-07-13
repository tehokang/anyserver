#include "anyserver_controller.h"
#include "anymacro.h"
#include "anylogger.h"

namespace anyserver
{

AnyServerController::AnyServerController(const string config_file)
    : m_anyserver_factory(nullptr)
    , m_argc(0)
    , m_argv(nullptr)
    , m_config_file(config_file)
{
    LOG_DEBUG("\n");
}

AnyServerController::AnyServerController(int argc, char **argv)
    : m_anyserver_factory(nullptr)
    , m_argc(argc)
    , m_argv(argv)
{
    LOG_DEBUG("\n");
}

AnyServerController::~AnyServerController()
{
    SAFE_DELETE(m_anyserver_factory);
}

bool AnyServerController::init()
{
    LOG_DEBUG("\n");
    m_anyserver_factory = new AnyServerFactory();
    RETURN_FALSE_IF_NULL(m_anyserver_factory);
    RETURN_FALSE_IF_FALSE(m_anyserver_factory->init(m_config_file));

    return true;
}

void AnyServerController::deinit()
{
    LOG_DEBUG("\n");
}

bool AnyServerController::start()
{
    LOG_DEBUG("\n");
    return true;
}

void AnyServerController::stop()
{
    LOG_DEBUG("\n");
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
