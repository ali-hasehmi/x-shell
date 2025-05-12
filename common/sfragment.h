#ifndef _S_SFRAGMENT_H
#define _S_SFRAGMENT_H

#include <stdint.h>

#include "xtcpsocket.h"
#include <sys/ioctl.h>

#define SBUFF_SIZE 1024

#define XFLAG_SINIT 0
#define XFLAG_ACK_SINIT_SUCCESS 1
#define XFLAG_ACK_SINIT_FAILURE 2
#define XFLAG_SHELL_DATA 3
#define XFLAG_SFINISH 4
#define XFLAG_ACK_SFINISH 5

typedef struct sfragment
{
    uint16_t buff_len;     // Size of data buffer
    uint16_t s_flag;       // flags
    struct winsize s_ws;   // terminal window size
    char buff[SBUFF_SIZE]; // payload
} sfragment_t;

int send_sfragment( xtcpsocket_t *_socket,const sfragment_t *_fragment);

int recv_sfragment(xtcpsocket_t *_socket, sfragment_t *_fragment);
#endif