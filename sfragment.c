#include "sfragment.h"

#include <stdlib.h>
#include <string.h>

int sfragment_send(const sfragment_t *_fragment, xtcpsocket_t *_socket)
{
    xmessage_t msg;
    msg.xm_f = XMESSAGE_SHELL;
    msg.xm_len = 2 * sizeof(uint16_t) + sizeof(struct winsize) + _fragment->size;
    msg.xm_d = malloc(msg.xm_len);
    if (msg.xm_d == NULL)
    {
        return -1;
    }
    memcpy(msg.xm_d, _fragment, msg.xm_len);
    xmessage_queue_enqueue(_socket->outgoing_message_queue, &msg);
    return 0;
}

int sfragment_recv(const sfragment_t *_fragment, xtcpsocket_t *_socket)
{
    xmessage_t msg;
    xmessage_queue_dequeue(_socket->incoming_message_queue, &msg);
    if (msg.xm_f != XMESSAGE_SHELL)
    {
        return -1;
    }
    memcpy(_fragment,msg.xm_d,msg.xm_len);
    free(msg.xm_d);
    return 0;
}
