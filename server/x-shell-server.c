#include <stdio.h>
#include <stdlib.h>

#include "xtcpsocket.h"
#include "xterminal.h"
#include "xfragment.h"
#include "xshell.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "xclient-list.h"
#include "xrequest.h"

#include "../config.h"

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

void print_progress_bar(double _progress)
{
    int numofhash = _progress * PROGRESS_BAR_SIZE;
    putchar('[');
    for (int i = 0; i < PROGRESS_BAR_SIZE; ++i)
    {
        if (numofhash > 0)
        {
            putchar('#');
            numofhash--;
        }
        else
        {
            putchar(' ');
        }
    }
    putchar(']');
    printf(" %.2f %%", _progress * 100);
    putchar('\r');
    fflush(stdout);
}

void *progress_worker(void *arg)
{
    xfile_t *f = arg;
    do
    {
        usleep(1000);
        print_progress_bar((double)f->x_rel_size / (double)f->x_size);
    } while (f->x_rel_size < f->x_size);
    putchar('\n');
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
int  exec_close_command()
{

    uint16_t cid;
    scanf("%hu", &cid);
    xclient_t *target_client = xclient_list_find(&list, cid);
    if (target_client == NULL)
    {
        printf("[!] Client with %hu id didn't find!\r\n", cid);
        return -1;
    }
    xclient_list_remove(&list, cid);
    return 0;
}
void print_help()
{
    const char *xshell_help =
        "\n  list                                    list all connected clients\n"
        "  shell <cid>                             try to establish a shell session with client with <cid> id\n"
        "  download <cid> <CFILE> <SAVELOCATION>   download the CFILE from the client with <cid> id and save it to SAVELOCATION\n"
        "  upload <cid> <HFILE> <SAVELOCATION>     upload the HFILE from the Host to client with <cid> id and save it to SAVELOCATION\n"
        "  close <cid>                             close the connection to client with <cid> id\n"
        "  help                                    show programs options\n"
        "  rename <cid> <newname>                  change the local name of client with <cid> id to <newname>\n"
        "  clear                                   clear screen\n"
        "  exit                                    exit the x-shell\n\n";

    printf("%s", xshell_help);
}
void *accept_worker(void *arg)
{
    xtcpsocket_t *server_socket = (xtcpsocket_t *)arg;
    xtcpsocket_t client_socket;
    xclient_t tmp_client;
    xrequest_t req;
    xclient_create(&tmp_client, NULL, "null", "null");
    while (isProgramRunning)
    {
        if (xtcpsocket_accept(server_socket, &client_socket) == -1)
        {
            fprintf(stderr,
                    "[!] xtcpsocket_accept() failed\n\r");
            break;
        }
        tmp_client.socket = client_socket;
        xtcpsocket_init_communication(&tmp_client.socket);
        xrequest_recv(&tmp_client.socket, &req);
        if (req.xr_flag == XRFLAG_INIT)
        {
            xclient_create(&tmp_client, NULL, req.xr_hostname, req.xr_logname);
            req.xr_flag = XRFlAG_ACK_INIT;
            xrequest_send(&tmp_client.socket, &req);
        }
        xclient_list_push_back(&list, &tmp_client);
    }
    xtcpsocket_close(server_socket);
}
int exec_rename_command()
{
    uint16_t cid;
    scanf("%hu", &cid);
    xclient_t *target_client = xclient_list_find(&list, cid);
    if (target_client == NULL)
    {
        printf("[!] Client with %hu id didn't find!\r\n", cid);
        return -1;
    }
    char buff[LOCALNAME_MAX_LEN];
    scanf("%s", buff);
    if (xclient_set_localname(target_client, buff))
    {
        printf("[!] failed to change the client local name\r\n");
        return -1;
    }
    return 0;
}
int exec_shell_command()
{
    xrequest_t req;
    req.xr_flag = XRFLAG_SHELL;
    uint16_t cid;
    scanf("%hu", &cid);
    xclient_t *target_client = xclient_list_find(&list, cid);
    if (target_client == NULL)
    {
        printf("[!] Client with %hu id didn't find!\r\n", cid);
        return -1;
    }
    // // REQUEST A NEW SHELL SESSION
    xrequest_send(&target_client->socket, &req);

    if (xterminal_enable_raw_mode() == -1)
    {
        fprintf(stderr,
                "[!] xterminal_enable_raw_mode() failed\n\r");
        // xterminal_reset();
        return -1;
    }

    if (xshell_start(&target_client->socket, REQUESTER) == -1)
    {
        fprintf(stderr,
                "[!] xshell_start() failed\n\r");
        // xterminal_reset();
        return -1;
    }
    // fprintf(stderr, "[+] xshell started successfully!\n");

    if (xshell_wait() == -1)
    {
        fprintf(stderr,
                "[!] xshell_wait() failed\n\r");
        // xterminal_reset();
        return -1;
    }
    // fprintf(stderr, "[+] xshell session waite finished!\n");

    if (xshell_finish() == -1)
    {
        fprintf(stderr,
                "[!] xshell_finish() failed\n\r");
        // xterminal_reset();

        return -1;
    }
    if (xterminal_disable_raw_mode() == -1)
    {
        fprintf(stderr,
                "[!] xterminal_disable_raw_mode() failed\n\r");
        return -1;
    }
    // fprintf(stderr, "[+] xshell finished succesffuly finished!\n");
    xterminal_reset();
    return 0;
}
int exec_download_command()
{
    xrequest_t req;
    req.xr_flag = XRFLAG_FILE_SHARING;
    uint16_t cid;
    scanf("%hu", &cid);
    xclient_t *target_client = xclient_list_find(&list, cid);
    if (target_client == NULL)
    {
        printf("[!] Client with %hu id isn't found!\n", cid);
        fflush(stdin);
        return -1;
    }

    xfile_t file;
    char client_file_path[256];
    char save_location[256];
    scanf("%s", client_file_path);
    scanf("%s", save_location);
    if (xfopen(save_location, &file, XF_CREAT | XF_EXCL | XF_WRONLY))
    {
        printf("[!] failed to open %s: %s\r\n", save_location, strerror(errno));
        return -1;
    }
    // Request file sharing from client
    xrequest_send(&target_client->socket, &req);
    if (make_fhandshake(&target_client->socket, client_file_path, &file, XFMODE_DOWNLOAD) == -1)
    {
        printf("[!] failed to handshake\r\n");
        return -1;
    }
    pthread_t p;
    pthread_create(&p, NULL, &progress_worker, &file);
    if (xfile_recv(&target_client->socket, &file) == -1)
    {
        printf("[!] an error occured\r\n");
        return -1;
    }
    pthread_join(p, NULL);
    return 0;
}
int exec_upload_command()
{
    xrequest_t req;
    req.xr_flag = XRFLAG_FILE_SHARING;
    uint16_t cid;
    scanf("%hu", &cid);
    xclient_t *target_client = xclient_list_find(&list, cid);
    if (target_client == NULL)
    {
        printf("[!] Client with %hu id isn't found!\n", cid);
        return -1;
    }

    xfile_t file;
    char file_path[256];
    char client_save_location[256];
    scanf("%s", file_path);
    scanf("%s", client_save_location);
    if (xfopen(file_path, &file, XF_RDONLY))
    {
        printf("[!] failed to open %s: %s\r\n", file_path, strerror(errno));
        return -1;
    }

    xrequest_send(&target_client->socket, &req);

    if (make_fhandshake(&target_client->socket, client_save_location, &file, XFMODE_UPLOAD) == -1)
    {
        printf("[!] failed to handshake\r\n");
        return -1;
    }
    pthread_t p;
    pthread_create(&p, NULL, &progress_worker, &file);
    if (xfile_send(&target_client->socket, &file) == -1)
    {
        printf("[!] an error occured\r\n");
        return -1;
    }
    pthread_join(p, NULL);
}
int main()
{

    if (freopen("./xshell.log", "w", stderr) == NULL)
    {
        fprintf(stderr,
                "[!] freopen() faild to open xshell.log: %s\r\n",
                strerror(errno));
    }

    print_ascii_art();

    // Register clean up for exiting
    atexit(&cleanup);
    xclient_list_create(&list);
    if (xterminal_init() == -1)
    {
        fprintf(stderr,
                "[!] xterminal_init() faild to save init terminal state\r\n");
    }
    printf("[+] terminal is initialized\n");

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
    printf("[+] server is listening on %d\n" ,SERVER_PORT);

    pthread_t p;
    if(pthread_create(&p, NULL, &accept_worker, (void *)&server_socket)){
        fprintf(stderr,
                "[!] pthread_create() failed\n\r");
                exit(EXIT_FAILURE);
    }
    printf("[+] workers are running\n");
    char command[256];
    while (true)
    {
        printf("> ");
        scanf("%s", command);
        if (!strcmp(command, "list"))
        {
            xclient_list_print(&list);
        }
        else if (!strcmp(command, "help"))
        {
            print_help();
        }
        else if (!strcmp(command, "shell"))
        {
            exec_shell_command();
        }
        else if (!strcmp(command, "clear"))
        {
            system("clear || cls");
            print_ascii_art();
        }
        else if (!strcmp(command, "exit"))
        {
            break;
        }
        else if (!strcmp(command, "download"))
        {
            exec_download_command();
        }
        else if (!strcmp(command, "upload"))
        {
            exec_upload_command();
        }
        else if (!strcmp(command, "rename"))
        {
            exec_rename_command();
        }
        else if (!strcmp(command, "close"))
        {
            exec_close_command();
        }
        else
        {
            printf("[!] Invalid Command!\nuse help command to list all available commands\n");
        }
    }
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
