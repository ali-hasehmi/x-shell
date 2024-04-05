#ifndef _X_MESSAGE_H
#define _X_MESSAGE_H

#include <stdint.h>

#include "xtcpsocket.h"

#define MESSAGE_HEADER_SIZE (sizeof(uint32_t) + sizeof(xmflag_t))

typedef uint16_t xmflag_t;

typedef struct xmessage
{
    uint32_t xm_len; /* message Data Len*/
    xmflag_t xm_f;   /* message Flags */
    void *xm_d;      /* message actual Data*/
} xmessage_t;

int xmessage_send(const xtcpsocket_t *_socket, const xmessage_t *_xmsg);

int xmessage_recv(const xtcpsocket_t *_socket, xmessage_t *_xmsg);

#endif // _X_MESSAGE_H