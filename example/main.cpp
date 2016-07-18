#include "anyserver_controller.h"
#include <unistd.h>

using namespace anyserver;

bool g_run = false;

class ServerApplication : public IAnyServerControllerListener
{
public:
    ServerApplication(int argc, char **argv)
        : m_anyserver_controller(new AnyServerController(argv[1]))
        , m_run(false)
    {
        m_anyserver_controller->setLogLevel(true, true, true, true);
        m_anyserver_controller->setListener(this);
    };

    virtual ~ServerApplication()
    {
        if ( m_anyserver_controller )
        {
            delete m_anyserver_controller;
            m_anyserver_controller = nullptr;
        }
    };

    bool init()
    {
        return m_anyserver_controller->init();
    }

    bool start()
    {
        if ( m_anyserver_controller->start() )
        {
            m_run = true;
            return true;
        }
        return false;
    }

    void stop()
    {
        m_anyserver_controller->stop();
    }

    virtual void onReceivedSystemSignal(int signal)
    {
        printf("received signal : %d \n", signal);
        m_run = false;
    }

    bool isRun() { return m_run; };
private:
    bool m_run;

    AnyServerController *m_anyserver_controller;
};

/**
 * @brief Example server application
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv)
{
    ServerApplication example(argc, argv);

    if ( example.init() && example.start() )
    {
        while ( example.isRun() )
        {
            usleep(100000);
        }
        example.stop();
    }
    printf("Stopped example \n");
    return 0;
}
