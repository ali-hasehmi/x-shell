#include <stdio.h>
#include <stdlib.h>

#include "xtcpsocket.h"
#include "xterminal.h"
#include "xfragment.h"
#include "xshell.h"
#include <errno.h>
#include <string.h>
#include "xclient-list.h"

#define LISTEN_PORT 10100
#define BACKLOG_LIMIT 6

xclient_list_t list;
int isProgramRunning = 1;
xtcpsocket_t server_socket;

void cleanup()
{
    printf("[~] cleanUp starting...\n");
    xtcpsocket_close(&server_socket);
    xclient_list_destroy(&list);
    xterminal_reset();
    printf("[+] cleanUp finished.\n");
}

void print_ascii_art()
{
    const char *xshell_art = 
        "               _          _ _ \n"
        "__  __     ___| |__   ___| | |\n"
        "\\ \\/ /____/ __| '_ \\ / _ \\ | |\n"
        " >  <_____\\__ \\ | | |  __/ | |\n"
        "/_/\\_\\    |___/_| |_|\\___|_|_|\n\n";

    printf("%s", xshell_art);
}
void *accept_worker(void *arg)
{
    xtcpsocket_t *server_socket = (xtcpsocket_t *)arg;
    xtcpsocket_t client_socket;
    xclient_t tmp_client;
    xclient_create(&tmp_client, &client_socket, "null", "null");
    while (isProgramRunning)
    {
        if (xtcpsocket_accept(server_socket, &client_socket) == -1)
        {
            fprintf(stderr,
                    "[!] xtcpsocket_accept() failed\n\r");
            break;
        }
        tmp_client.socket = client_socket;
        xclient_list_push_back(&list, &tmp_client);
    }
    xtcpsocket_close(server_socket);
}
int main()
{

    if (freopen("./xshell.log", "w", stderr) == NULL)
    {
        fprintf(stderr,
                "[!] freopen() faild to open xshell.log: %s\r\n",
                strerror(errno));
    }
    // Register clean up for exiting
    atexit(&cleanup);
    xclient_list_create(&list);
    if (xterminal_init() == -1)
    {
        fprintf(stderr,
                "[!] xterminal_init() faild to save init terminal state\r\n");
    }

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

    pthread_t p;
    pthread_create(&p, NULL, &accept_worker, (void *)&server_socket);
    char command[256];
    print_ascii_art();
    do
    {
        printf("> ");
        scanf("%s", command);
        if (!strcmp(command, "list"))
        {
            xclient_list_print(&list);
        }
    } while (strcmp(command, "exit"));
    isProgramRunning = 0;
    // pthread_join(p, NULL);
    //  xtcpsocket_t client_socket;
    //  if (xtcpsocket_accept(&server_socket, &client_socket) == -1)
    //  {
    //      fprintf(stderr,
    //              "[!] xtcpsocket_accept() failed\n\r");
    //      exit(EXIT_FAILURE);
    //  }

    // // REQUEST A NEW SHELL SESSION

    // if (xterminal_enable_raw_mode() == -1)
    // {
    //     fprintf(stderr,
    //             "[!] xterminal_enable_raw_mode() failed\n\r");
    //     // xterminal_reset();
    //     exit(EXIT_FAILURE);
    // }

    // if (xshell_start(&client_socket, REQUESTER) == -1)
    // {
    //     fprintf(stderr,
    //             "[!] xshell_start() failed\n\r");
    //     // xterminal_reset();
    //     exit(EXIT_FAILURE);
    // }
    // fprintf(stderr, "[+] xshell started successfully!\n");

    // if (xshell_wait() == -1)
    // {
    //     fprintf(stderr,
    //             "[!] xshell_wait() failed\n\r");
    //     // xterminal_reset();
    //     exit(EXIT_FAILURE);
    // }
    // fprintf(stderr, "[+] xshell session waite finished!\n");

    // if (xshell_finish() == -1)
    // {
    //     fprintf(stderr,
    //             "[!] xshell_finish() failed\n\r");
    //     // xterminal_reset();

    //     exit(EXIT_FAILURE);
    // }
    // if (xterminal_disable_raw_mode() == -1)
    // {
    //     fprintf(stderr,
    //             "[!] xterminal_disable_raw_mode() failed\n\r");
    //     exit(EXIT_FAILURE);
    // }
    // fprintf(stderr, "[+] xshell finished succesffuly finished!\n");
    // // xterminal_reset();
    return 0;
}