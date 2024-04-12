#include <stdio.h>
#include <stdlib.h>

#include "xshell.h"
#include "xfragment.h"
#include "xfile.h"

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

    xtcpsocket_init_communication(&client_socket);

    xfile_t file;
    int mode;
    if (make_fhandshake_d(&client_socket, &file, &mode) == -1)
    {
        fprintf(stderr,
                "[!] failed to make handshake!\r\n");
        exit(EXIT_FAILURE);
    }
    switch (mode)
    {
    case XFMODE_DOWNLOAD:
        printf("[+] DOWNLOAD command detected!\n");
        if (xfile_send(&client_socket, &file) == -1)
        {
            fprintf(stderr,
                    "[!] xfile_send() failed\n\r");
        }
        break;

    case XFMODE_UPLOAD:
        printf("[+] UPLOAD command detected!\n");
        if (xfile_recv(&client_socket, &file) == -1)
        {
            fprintf(stderr,
                    "[!] xfile_recv() failed\n\r");
        }
        break;
    default:
        fprintf(stderr,
                "[!] invalid mode\n\r");
        break;
    }
    printf("[+] operation was finished successfully!\n");
    // if (xshell_start(&client_socket, RESPONDER) == -1)
    // {
    //     fprintf(stderr,
    //             "[!] xshell_start() failed\n\r");
    //     exit(EXIT_FAILURE);
    // }

    // printf("[+] xshell started successfully!\n");

    // if (xshell_wait() == -1)
    // {
    //     fprintf(stderr,
    //             "[!] xshell_wait() failed\n\r");
    //     exit(EXIT_FAILURE);
    // }

    // printf("[+] xshell session waite finished!\n");

    // if (xshell_finish() == -1)
    // {
    //     fprintf(stderr,
    //             "[!] xshell_finish() failed\n\r");
    //     exit(EXIT_FAILURE);
    // }
    // printf("[+] xshell finished succesffuly finished!\n");
    getchar();
}
