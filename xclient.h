#ifndef X_CLIENT_H
#define X_CLIENT_H

#include "xtcpsocket.h"

#define HOSTNAME_MAX_LEN 64
#define LOGNAME_MAX_LEN 64
#define LOCALNAME_MAX_LEN 64

typedef struct xclient
{
    xtcpsocket_t socket;
    uint16_t id;
    char hostname[HOSTNAME_MAX_LEN];
    char logname[LOGNAME_MAX_LEN];
    char localname[LOCALNAME_MAX_LEN];
} xclient_t;

int xclient_create(xclient_t *_client, const xtcpsocket_t *_socket, const char *_hostname, const char *_logname);
void xclient_print(const xclient_t *_client);

#endif // X_CLIENT_H