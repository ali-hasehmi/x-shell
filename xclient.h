#ifndef X_CLIENT_H
#define X_CLIENT_H

#include "xtcpsocket.h"

#define HOSTNAME_MAX_LEN 64
#define LOGNAME_MAX_LEN 64
#define NAME_MAX_LEN 64

typedef struct xclient
{
    xtcpsocket_t socket;
    uint16_t id;
    char hostname[HOSTNAME_MAX_LEN];
    char logname[LOGNAME_MAX_LEN];
    char localname[NAME_MAX_LEN];
} xclient_t;

#endif // X_CLIENT_H