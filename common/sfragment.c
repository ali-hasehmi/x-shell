#include "sfragment.h"

#include <stdlib.h>
#include <string.h>

int send_sfragment(xtcpsocket_t *_socket,const sfragment_t *_fragment)
{
    xmessage_t msg;
    msg.xm_f = XMESSAGE_SHELL;
    msg.xm_len = 2 * sizeof(uint16_t) + sizeof(struct winsize) + _fragment->buff_len;
    msg.xm_d = malloc(msg.xm_len);
    if (msg.xm_d == NULL)
    {
        return -1;
    }
    memcpy(msg.xm_d, _fragment, msg.xm_len);
    xmessage_queue_enqueue(_socket->outgoing_message_queue, &msg);
    return 0;
}

int recv_sfragment( xtcpsocket_t *_socket, sfragment_t *_fragment)
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
