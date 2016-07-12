#include "server_configuration.h"
#include "anylogger.h"

using namespace anyserver;

int main(int argc, char **argv)
{
    AnyLogger::setLogLevel(true, true, true, true);

    ServerConfiguration::init(argv[1]);

    return 0;
}
