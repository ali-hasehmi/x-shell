#include <stdio.h>
#include <stdlib.h>

#include "xshell.h"

#define SERVER_IP (char *)"127.0.0.1"
#define SERVER_PORT 10100

int main()
{
    xtcpsocket_t client_socket;
    if (xtcpsocket_create(&client_socket, AF_INET, xst_client) == -1)
    {
        fprintf(stderr,
                "[!] xtcpsocket_create() failed\n\r");
        exit(EXIT_FAILURE);
    }
    if (xtcpsocket_connect(&client_socket, SERVER_IP, SERVER_PORT) == -1)
    {
        fprintf(stderr,
                "[!] xtcpsocket_create() failed\n\r");
        exit(EXIT_FAILURE);
    }
    printf("[+] connected successfully to server!\n");

    if (xshell_start(&client_socket, RESPONDER) == -1)
    {
        fprintf(stderr,
                "[!] xshell_start() failed\n\r");
        exit(EXIT_FAILURE);
    }

    printf("[+] xshell started successfully!\n");

    if (xshell_wait() == -1)
    {
        fprintf(stderr,
                "[!] xshell_wait() failed\n\r");
        exit(EXIT_FAILURE);
    }

    printf("[+] xshell session waite finished!\n");

    if (xshell_finish() == -1)
    {
        fprintf(stderr,
                "[!] xshell_finish() failed\n\r");
        exit(EXIT_FAILURE);
    }
    printf("[+] xshell finished succesffuly finished!\n");
    getchar();
}