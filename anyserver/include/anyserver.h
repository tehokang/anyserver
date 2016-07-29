#ifndef __ANYSERVER_H__
#define __ANYSERVER_H__

#include <string>
using namespace std;

namespace anyserver
{
class PosixSignalInterceptor;
class Controller;
class AnyServer
{
public:
    /**
     * @brief AnyServer argv[1] has to include configuration path
     * @param argc
     * @param argv
     */
    AnyServer(int argc, char **argv);
    AnyServer(string config_file);
    virtual ~AnyServer();

    /**
     * @brief initialize servers that has defined in config file
     * @return return true if succeed else return false.
     */
    bool init();

    /**
     * @brief start servers and the servers will be listening
     * @return return true if succeed else return false
     */
    bool start();

    /**
     * @brief stop servers which has started
     */
    void stop();

    /**
     * @brief To check if servers is working out at this moment
     * @return return true if servers work out successfully
     */
    bool isRun() { return m_run; };
private:
    /**
     * @brief To handle posix signal(SIGPIPE, SIGXXX and so on)
     * @param signal_id
     */
    void onReceivedPosixSignal(int signal_id);

    bool m_run;
    PosixSignalInterceptor *m_posix_signal_interceptor;
    Controller *m_anyserver_controller;
};

} // end of namespace

#endif
