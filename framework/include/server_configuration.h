#ifndef __SERVER_CONFIGURATION_H__
#define __SERVER_CONFIGURATION_H__

#include "json.h"

#include <string>
using namespace std;

class ServerConfiguration
{
public:
    static bool init(string file);

protected:
    typedef struct
    {
        enum
        {
            WEBSOCKET_TYPE,
            HTTP_TYPE,
            IDS_TYPE,
            UDS_TYPE
        } type;

        union
        {
            unsigned int port;
            string file;
        } target;
    } ServerType;

    typedef struct
    {
        ServerType type;
        string name;
        int max_clients;
        bool enable_security;
        string version;
        string copyright;
    } ServerInfo;

private:
    ServerConfiguration();
    ~ServerConfiguration();
};


#endif
