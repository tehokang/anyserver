#ifndef __ANYSERVER_H__
#define __ANYSERVER_H__

#include "posix_signal_interceptor.h"

#include <string>
using namespace std;

namespace anyserver
{

class Controller;
class AnyServer
{
public:
    AnyServer(int argc, char **argv);
    AnyServer(string config_file);
    virtual ~AnyServer();

    bool init();
    bool start();
    void stop();
    bool isRun() { return m_run; };
private:
    void onReceivedPosixSignal(int signal_id);

    bool m_run;
    PosixSignalInterceptor posix_signal_interceptor;
    Controller *m_anyserver_controller;
};

} // end of namespace

#endif
