#include "xrequest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int xrequest_send(xtcpsocket_t *_socket, const xrequest_t *_req)
{
    xmessage_t msg;
    msg.xm_f = XMESSAGE_REQ;
    msg.xm_len = sizeof(xrequest_t);
    msg.xm_d = malloc(msg.xm_len);
    if (msg.xm_d == NULL)
    {
        fprintf(stderr,
               "[!] xrequest_send():malloc() failed: cannot allocate memory\r\n");
        return -1;
    }
    memcpy(msg.xm_d, _req, msg.xm_len);
    xmessage_queue_enqueue(_socket->outgoing_message_queue, &msg);
    return 0;
}

int xrequest_recv(xtcpsocket_t *_socket, xrequest_t *_req)
{
    xmessage_t msg;
    xmessage_queue_dequeue(_socket->incoming_message_queue, &msg);
    if(memcpy(_req, msg.xm_d, msg.xm_len)  == NULL){
        return -1;
    }
    free(msg.xm_d);
    return 0;
}
