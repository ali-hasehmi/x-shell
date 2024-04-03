#ifndef X_SHELL_H
#define X_SHELL_H

#include "xfragment.h"

typedef enum
{
    REQUESTER,
    RESPONDER,
} xs_state_t;
/*
    making handshake to requst creating a New Shell session from Peer
    return 0 on success
    otherwise return -1
*/
int make_shandshake();

/*
    making handshake to respond to Peer for creating a New Shell session
    return 0 on success
    otherwise return -1
*/
int make_shandshake_d();

static int xshell_init();


/*
    store essential data about shell connection
    make handshake base on state
    create shell and create reliable connection channel
    between requester and responder
    return 0 on sucess
    return -1 on failure
*/
int xshell_start(const xtcpsocket_t *_socket, xs_state_t _state);

int xshell_finish();

/*
    wait for shell session until it finishes
*/
int xshell_wait();

#endif