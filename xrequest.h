#ifndef _X_REQUEST_H
#define _X_REQUEST_H

#include "xtcpsocket.h"
#include <stdint.h>

#define XRFLAG_INIT 0
#define XRFlAG_ACK_INIT 1
#define XRFLAG_SHELL 2
#define XRFLAG_FILE_SHARING 3

#define XR_HOSTNAME_LEN 64
#define XR_LOGNAME_LEN 32

typedef struct xrequest
{
    uint8_t xr_flag;
    char xr_logname[XR_LOGNAME_LEN];
    char xr_hostname[XR_HOSTNAME_LEN];
} xrequest_t;

int xrequest_send(xtcpsocket_t *_socket, const xrequest_t *_req);
int xrequest_recv(xtcpsocket_t *_socket, xrequest_t *_req);

#endif // _X_REQUEST_H