#include "xmessage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
    sends a message
    return 0 on success
    return -1 on failure
*/
int xmessage_send(const xtcpsocket_t *_socket, const xmessage_t *_xmsg)
{

    /*
        sends message len and flags
    */
    if (xtcpsocket_sendall(_socket,
                           _xmsg,
                           MESSAGE_HEADER_SIZE) < MESSAGE_HEADER_SIZE)
    {
        fprintf(stderr,
                "[!] send_xmessage():xtcpsocket_sendall() failed: %s\r\n",
                strerro(errno));
        return -1;
    }

    /*
        send message data
    */
    if (xtcpsocket_sendall(_socket,
                           _xmsg->xm_d,
                           _xmsg->xm_len) < MESSAGE_HEADER_SIZE)
    {
        fprintf(stderr,
                "[!] send_xmessage():xtcpsocket_sendall() failed: %s\r\n",
                strerro(errno));
        return -1;
    }

    fprintf(stderr, "[+] send_xmessage(): sends %luByte Successfuly!\n",
            MESSAGE_HEADER_SIZE + _xmsg->xm_len);

    return 0;
}

/*
    recvs a message
    return 0 on success
    return -1 on failure
*/
int xmessage_recv(const xtcpsocket_t *_socket, xmessage_t *_xmsg)
{
    /*
        receive message len and flags
    */
    if (xtcpsocket_recvall(_socket,
                           _xmsg,
                           MESSAGE_HEADER_SIZE) < MESSAGE_HEADER_SIZE)
    {
        fprintf(stderr,
                "[!] send_xmessage():xtcpsocket_sendall() failed: %s\r\n",
                strerro(errno));
        return -1;
    }

    /*
        Allocate sufficient memory for the message Data
    */
    _xmsg->xm_d = malloc(_xmsg->xm_len);
    if (_xmsg->xm_d == NULL)
    {
        fprintf(stderr,
                "[!] send_xmessage() failed: cannot allocate memory!\r\n");
        return -1;
    }

    /*
        receive message data
    */
    if (xtcpsocket_recvall(_socket,
                           _xmsg->xm_d,
                           _xmsg->xm_len) < _xmsg->xm_len)
    {
        fprintf(stderr,
                "[!] send_xmessage():xtcpsocket_sendall() failed: %s\r\n",
                strerro(errno));
        return -1;
    }

    fprintf(stderr, "[+] send_xmessage(): sends %luByte Successfuly!\n",
            MESSAGE_HEADER_SIZE + _xmsg->xm_len);

    return 0;
}
