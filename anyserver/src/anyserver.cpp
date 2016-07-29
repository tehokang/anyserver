#include "anyserver.h"
#include "controller.h"
#include <unistd.h>
#include <csignal>

namespace anyserver
{

AnyServer::AnyServer(int argc, char **argv)
	: m_anyserver_controller(new Controller(argv[1]))
   	, m_run(false)
{
	m_anyserver_controller->setLogLevel(true, true, true, true);
};

AnyServer::AnyServer(string config_file)
	: m_anyserver_controller(new Controller(config_file))
	, m_run(false)
{
	m_anyserver_controller->setLogLevel(true, true, true, true);
}

AnyServer::~AnyServer()
{
	if ( m_anyserver_controller )
	{
		delete m_anyserver_controller;
		m_anyserver_controller = nullptr;
	}
};

bool AnyServer::init()
{
    std::vector<int> signal_ids;
    signal_ids.push_back(SIGINT);
    signal_ids.push_back(SIGQUIT);
    signal_ids.push_back(SIGTERM);
    signal_ids.push_back(SIGHUP);
    signal_ids.push_back(SIGPIPE);
    posix_signal_interceptor.HandleSignals(
            signal_ids,
            std::bind1st(std::mem_fun(&AnyServer::onReceivedPosixSignal), this));

	return m_anyserver_controller->init();
}

void AnyServer::onReceivedPosixSignal(int signal_id)
{
    m_run = false;
}

bool AnyServer::start()
{
	if ( m_anyserver_controller->start() )
	{
		m_run = true;
		return true;
	}
	return false;
}

void AnyServer::stop()
{
	m_anyserver_controller->stop();
}

} // end of namespace
