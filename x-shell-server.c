#include <stdio.h>
#include <stdlib.h>

#include "xtcpsocket.h"
#include "xterminal.h"
#include "xfragment.h"
#include "xshell.h"
#include <errno.h>
#include <string.h>

#define LISTEN_PORT 10100
#define BACKLOG_LIMIT 6

int main()
{

    if (freopen("./xshell.log", "w", stderr) == NULL)
    {
        fprintf(stderr,
                "[!] freopen() faild to open xshell.log: %s\r\n",
                strerror(errno));
    }

    if (xterminal_init() == -1)
    {
        fprintf(stderr,
                "[!] xterminal_init() faild to save init terminal state\r\n");
    }

    xtcpsocket_t server_socket;
    if (xtcpsocket_create(&server_socket, AF_INET, xst_server) == -1)
    {
        fprintf(stderr,
                "[!] xtcpsocket_create() failed\n\r");
        exit(EXIT_FAILURE);
    }

    if (xtcpsocket_bind(&server_socket, NULL, LISTEN_PORT) == -1)
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

    if (xterminal_enable_raw_mode() == -1)
    {
        fprintf(stderr,
                "[!] xterminal_enable_raw_mode() failed\n\r");
        // xterminal_reset();
        exit(EXIT_FAILURE);
    }

    if (xshell_start(&client_socket, REQUESTER) == -1)
    {
        fprintf(stderr,
                "[!] xshell_start() failed\n\r");
        // xterminal_reset();
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "[+] xshell started successfully!\n");

    if (xshell_wait() == -1)
    {
        fprintf(stderr,
                "[!] xshell_wait() failed\n\r");
        // xterminal_reset();
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "[+] xshell session waite finished!\n");

    if (xshell_finish() == -1)
    {
        fprintf(stderr,
                "[!] xshell_finish() failed\n\r");
        // xterminal_reset();

        exit(EXIT_FAILURE);
    }
    if (xterminal_disable_raw_mode() == -1)
    {
        fprintf(stderr,
                "[!] xterminal_disable_raw_mode() failed\n\r");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "[+] xshell finished succesffuly finished!\n");
    // xterminal_reset();
    return 0;
}