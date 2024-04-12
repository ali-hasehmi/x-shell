#ifndef _S_SFRAGMENT_H
#define _S_SFRAGMENT_H

#include <stdint.h>

#include "xtcpsocket.h"
#include <sys/ioctl.h>

#define SBUFF_SIZE 1024

typedef struct sfragment
{
    uint16_t size;              // Size of data buffer
    uint16_t s_flag;            // flags
    struct winsize s_ws;        // terminal window size
    char data_buff[SBUFF_SIZE]; // payload
} sfragment_t;

int sfragment_send(const sfragment_t *_fragment, xtcpsocket_t *_socket);

int sfragment_recv(sfragment_t *_fragment, xtcpsocket_t *_socket);
#endif