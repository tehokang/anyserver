#include "anyserver_controller.h"

using namespace anyserver;

int main(int argc, char **argv)
{
    AnyServerController anyserver(argv[1]);
    anyserver.setLogLevel(true, true, true, true);

    if ( anyserver.init() && anyserver.start() )
    {
        printf("Started AnyServerController \n");
    }

    return 0;
}
