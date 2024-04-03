#include "xshell.h"

#include <pty.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <unistd.h>

static int sg_master_fd = -1; /*INITIAL VALUE,
 if this value didn't change it means this code is running on requesting side*/
static xs_state_t host_state;
static pthread_t read_thread;
static pthread_t write_thread;
static xtcpsocket_t *connection_socket;
static xfragment_t sending_fragment;
static xfragment_t recving_fragment;
static bool is_connected = true;

static int writeall(int _fd, const void *_buff, uint16_t *_buff_len)
{
    size_t total = 0,
           bytesleft = *_buff_len;
    ssize_t n;
    while (total < *_buff_len)
    {
        n = write(_fd, _buff + total, bytesleft);
        if (n == -1)
        {
            fprintf(stderr,
                    "[!] writeall() failed: %s\r\n",
                    strerror(errno));
            break;
        }
        total += n;
        bytesleft -= n;
    }
    *_buff_len = total;
    return n == -1 ? -1 : 0;
}

static void *write_worker(void *arg)
{

    /*
        Check For Host state
    */
    int tmp_fd;
    switch (host_state)
    {
    case REQUESTER:
        // Running on requesting side
        // write received data to stdout
        tmp_fd = STDOUT_FILENO;
        break;
    case RESPONDER:
        // Running of responding side
        // write received data to master fd
        tmp_fd = sg_master_fd;
        break;
    default:
        is_connected = false;
        fprintf(stderr,
                "[!] write_worker() failed: invalid host state\r\n");
        return NULL;
        break;
    }

    while (is_connected)
    {
        if (recv_xfragment(connection_socket, &recving_fragment) == -1)
        {
            printf("[~] write_worker():recv_xfragment() return -1\n");
            is_connected = false;
            return NULL;
        }

        if ((recving_fragment.f_flag == XFLAG_SFINISH) || (recving_fragment.f_flag == XFLAG_ACK_SFINISH))
        {
            printf("[~] write_worker(): detect finish fragment\n");
            is_connected = false;
            return NULL;
        }
       
        if (writeall(tmp_fd, recving_fragment.buffer, &(recving_fragment.buff_len)) == -1)
        {
            printf("[~] write_worker(): writeall() return -1\n");
            is_connected = false;
            return NULL;
        }
    }
    printf("[~] write_worker():is_connected is false\n");
}

static void *read_worker(void *arg)
{

    sending_fragment.f_flag = XFLAG_SHELL_DATA;
    /*
            Check For Host state
    */
    int tmp_fd;
    switch (host_state)
    {

    case REQUESTER:
        // Running on requesting side
        // read  data from stdin
        tmp_fd = STDIN_FILENO;
        break;

    case RESPONDER:
        // Running of responding side
        // write received data to master fd
        tmp_fd = sg_master_fd;
        break;

    default:
        is_connected = false;
        fprintf(stderr,
                "[!] read_worker() failed: invalid host state\r\n");
        return NULL;
        break;
    }

    while (is_connected)
    {
        if ((sending_fragment.buff_len = read(tmp_fd,
                                              sending_fragment.buffer,
                                              XFBUFF_SIZE)) == -1)
        {
            fprintf(stderr,
                    "[~] read_worker():read() failed: %s\r\n",
                    strerror(errno));
            is_connected = false;
            return NULL;
        }

        if (send_xfragment(connection_socket, &sending_fragment) == -1)
        {
            fprintf(stderr,
                    "[~] read_worker():send_xfragment() failed: %s\r\n",
                    strerror(errno));
            is_connected = false;
            return NULL;
        }
    }
}

int make_shandshake()
{
    xfragment_t init_frag;
    init_frag.buff_len = 0;
    init_frag.f_flag = XFLAG_SINIT;
    init_frag.f_offset = 0;
    init_frag.f_size = 0;
    if (send_xfragment(connection_socket, &init_frag) == -1)
    {
        fprintf(stderr,
                "[!] make_shandshake() failed: could not send initital fragment\r\n");
        return -1;
    }
    if (recv_xfragment(connection_socket, &init_frag) == -1)
    {
        fprintf(stderr,
                "[!] make_shandshake() failed: could not receive initital Ack fragment\r\n");
        return -1;
    }
    if (init_frag.f_flag == XFLAG_ACK_SINIT_FAILURE)
    {
        fprintf(stderr,
                "[!] make_xhandshake() failed: peer failed to create a new shell session\n\rerr: %s\r\n",
                init_frag.buffer);
        return -1;
    }
    if (init_frag.f_flag != XFLAG_ACK_SINIT_SUCCESS)
    {
        fprintf(stderr,
                "[!] make_shandshake() failed: receive fragment with invalid flag from peer\n\r");
        return -1;
    }
    printf("[+] New Shell Session Created!\n\r");
    return 0;
}

int make_shandshake_d()
{
    xfragment_t init_frag;
    if (recv_xfragment(connection_socket, &init_frag))
    {
        fprintf(stderr,
                "[!] make_shandshake_d() failed: could not receive initital fragment\r\n");
        return -1;
    }
    if (init_frag.f_flag != XFLAG_SINIT)
    {
        fprintf(stderr,
                "[!] make_shandshake_d() failed: receive fragment with invalid flag\r\n");
        return -1;
    }

    if (xshell_init() == -1)
    {
        snprintf(init_frag.buffer, XFBUFF_SIZE, "failed to create shell: %s\r\n", strerror(errno));
        init_frag.buff_len = strlen(init_frag.buffer) + 1;
        init_frag.f_flag = XFLAG_ACK_SINIT_FAILURE;
        if (send_xfragment(connection_socket, &init_frag) == -1)
        {
            fprintf(stderr,
                    "[!] make_shandshake_d():send_xfragment() failed: cannot send ACK fragment\r\n");
        }
        return -1;
    }

    // printf("something in between!\n");
    init_frag.f_flag = XFLAG_ACK_SINIT_SUCCESS;
    init_frag.buff_len = 0;
    if (send_xfragment(connection_socket, &init_frag) == -1)
    {
        fprintf(stderr,
                "[!] make_shandshake_d():send_xfragment failed: cannot send ACK fragment\r\n");
    }
    return 0;
}

static int xshell_init()
{
    int pid;
    if ((pid = forkpty(&sg_master_fd, NULL, NULL, NULL)) == -1)
    {
        fprintf(stderr, "[!] xshell_init():forkpty() failed: %s\r\n",
                strerror(errno));
        return -1;
    }

    /*
        Child Process
    */
    if (pid == 0)
    {

        // // create new session
        // if (setsid() == -1)
        // {
        //     perror("[!] setsid failed");
        //     return -1;
        // }

        // Execute bash shell
        if (execl("/bin/zsh", "/bin/zsh", NULL) == -1)
        {
            // execve("/bin/bash", arg, NULL);
            perror("[!] execl"); // This line will execute only if execl fails
            return -1;
        }
        return 0;
    }
    /*
        Parent Process
    */
    else
    {
        // printf(" fuckkkkk!\\\\n");
        return 0;
    }
}

int xshell_start(const xtcpsocket_t *_socket, xs_state_t _state)
{
    host_state = _state;
    connection_socket = _socket;
    switch (_state)
    {
    case REQUESTER:
        if (make_shandshake() == -1)
        {
            return -1;
        }
        break;

    case RESPONDER:
        if (make_shandshake_d() == -1)
        {
            return -1;
        }
        break;

    default:
        fprintf(stderr,
                "[!] xshell_start() failed: invalid state\r\n");
        break;
    }
    is_connected = true;
    if (pthread_create(&read_thread, NULL, &read_worker, NULL))
    {
        fprintf(stderr,
                "[!] xshell_start()pthread_create() failed: %s\r\n",
                strerror(errno));
        return -1;
    }
    if (pthread_create(&write_thread, NULL, &write_worker, NULL))
    {
        fprintf(stderr,
                "[!] xshell_start():pthread_create() failed: %s\r\n", strerror(errno));
        return -1;
    }
    return 0;
}

int xshell_finish()
{
    xfragment_t finish_frag;
    switch (host_state)
    {
    case REQUESTER:
        // Do something Later
        return 0;
        break;

    case RESPONDER:
        finish_frag.buff_len = 0;
        finish_frag.f_flag = XFLAG_FFINISH;
        break;

    default:
        fprintf(stderr,
                "[!] xshell_finish() failed: invalid state\r\n");
        break;
    }

    if (send_xfragment(connection_socket, &finish_frag) == -1)
    {
        fprintf(stderr,
                "[!] xshell_finish():send_xfragment failed: cannot send finish fragment\r\n");
        return -1;
    }
    sg_master_fd == -1;
    return 0;
}

int xshell_wait()
{
    printf("[~]xshell_wait() runs!\n");
    // pthread_join(write_thread, NULL);
    while (is_connected)
    {
        usleep(1000);
    }
}
