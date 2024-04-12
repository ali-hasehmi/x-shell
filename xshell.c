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
#include <sys/ioctl.h>
#include <signal.h>

static int sg_master_fd = -1; /*INITIAL VALUE,
 if this value didn't change it means this code is running on requesting side*/
static xs_state_t host_state;
static pthread_t read_thread;
static pthread_t write_thread;
static struct winsize curr_win_size;
static xtcpsocket_t *connection_socket;
static sfragment_t sending_fragment;
static sfragment_t recving_fragment;
static bool is_connected = true;

void handle_sigwinch(int signal)
{
    //printf("handle_sigwinch called\n");
    // struct winsize ws;
    //  Get the new terminal size
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &curr_win_size) == -1)
    {
        perror("[!]handle_sigwinch():ioctl()");
    }
    // printf("Terminal resized: rows=%d, cols=%d\n", ws.ws_row, ws.ws_col);
}

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
        if (recv_sfragment(connection_socket, &recving_fragment) == -1)
        {
            printf("[~] write_worker():recv_xfragment() return -1\n");
            is_connected = false;
            return NULL;
        }

        if (memcmp(&recving_fragment.s_ws, &curr_win_size, sizeof(struct winsize)) && (host_state == RESPONDER))
        {
            fprintf(stderr,"[~] pseudo terminal size changed!\n");
            memcpy(&curr_win_size, &recving_fragment.s_ws, sizeof(struct winsize));
            if (ioctl(sg_master_fd, TIOCSWINSZ, &curr_win_size) == -1)
            {
                perror("ioctl(TIOCSWINSZ)");
            }
        }
        if ((recving_fragment.s_flag == XFLAG_SFINISH) || (recving_fragment.s_flag == XFLAG_ACK_SFINISH))
        {
            fprintf(stderr,"[~] write_worker(): detect finish fragment\n");
            is_connected = false;
            return NULL;
        }

        if (writeall(tmp_fd, recving_fragment.buff, &(recving_fragment.buff_len)) == -1)
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

    sending_fragment.s_flag = XFLAG_SHELL_DATA;
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
        ssize_t n;
        if ((n = read(tmp_fd,
                      sending_fragment.buff,
                      SBUFF_SIZE)) == -1)
        {
            fprintf(stderr,
                    "[~] read_worker():read() failed: %s\r\n",
                    strerror(errno));
            is_connected = false;
            return NULL;
        }
        sending_fragment.buff_len = n;
        memcpy(&sending_fragment.s_ws, &curr_win_size, sizeof(struct winsize));
        // printf("[~] read_worker():read(): %huB read!\n\r", sending_fragment.buff_len);

        if (send_sfragment(connection_socket, &sending_fragment) == -1)
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
    struct sigaction sa;
    sa.sa_handler = handle_sigwinch;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    // Install signal handler for SIGWINCH
    if (sigaction(SIGWINCH, &sa, NULL) == -1)
    {
        perror("sigaction");
        // exit(EXIT_FAILURE);
    }

    sfragment_t init_frag;
    init_frag.buff_len = 0;
    init_frag.s_flag = XFLAG_SINIT;
    // Get window size
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &curr_win_size) == -1)
    {
        fprintf(stderr,
                "[!] make_shandshake(): ioctl() failed: %s\r\n", strerror(errno));
        return -1;
    }
    memcpy(&init_frag.s_ws, &curr_win_size, sizeof(struct winsize));
    if (send_sfragment(connection_socket, &init_frag) == -1)
    {
        fprintf(stderr,
                "[!] make_shandshake() failed: could not send initital fragment\r\n");
        return -1;
    }
    if (recv_sfragment(connection_socket, &init_frag) == -1)
    {
        fprintf(stderr,
                "[!] make_shandshake() failed: could not receive initital Ack fragment\r\n");
        return -1;
    }
    if (init_frag.s_flag == XFLAG_ACK_SINIT_FAILURE)
    {
        fprintf(stderr,
                "[!] make_xhandshake() failed: peer failed to create a new shell session\n\rerr: %s\r\n",
                init_frag.buff);
        return -1;
    }
    if (init_frag.s_flag != XFLAG_ACK_SINIT_SUCCESS)
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
    sfragment_t init_frag;
    if (recv_sfragment(connection_socket, &init_frag))
    {
        fprintf(stderr,
                "[!] make_shandshake_d() failed: could not receive initital fragment\r\n");
        return -1;
    }
    if (init_frag.s_flag != XFLAG_SINIT)
    {
        fprintf(stderr,
                "[!] make_shandshake_d() failed: receive fragment with invalid flag\r\n");
        return -1;
    }

    if (xshell_init(&init_frag.s_ws) == -1)
    {
        snprintf(init_frag.buff, SBUFF_SIZE, "failed to create shell: %s\r\n", strerror(errno));
        init_frag.buff_len = strlen(init_frag.buff) + 1;
        init_frag.s_flag = XFLAG_ACK_SINIT_FAILURE;
        if (send_sfragment(connection_socket, &init_frag) == -1)
        {
            fprintf(stderr,
                    "[!] make_shandshake_d():send_xfragment() failed: cannot send ACK fragment\r\n");
        }
        return -1;
    }

    // printf("something in between!\n");
    init_frag.s_flag = XFLAG_ACK_SINIT_SUCCESS;
    init_frag.buff_len = 0;
    if (send_sfragment(connection_socket, &init_frag) == -1)
    {
        fprintf(stderr,
                "[!] make_shandshake_d():send_xfragment failed: cannot send ACK fragment\r\n");
    }
    return 0;
}

static int xshell_init(const struct winsize *_ws)
{
    int pid;
    if ((pid = forkpty(&sg_master_fd, NULL, NULL, _ws)) == -1)
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
    sfragment_t finish_frag;
    switch (host_state)
    {
    case REQUESTER:
        // Do something Later
        return 0;
        break;

    case RESPONDER:
        finish_frag.buff_len = 0;
        finish_frag.s_flag = XFLAG_SFINISH;
        break;

    default:
        fprintf(stderr,
                "[!] xshell_finish() failed: invalid state\r\n");
        break;
    }

    if (send_sfragment(connection_socket, &finish_frag) == -1)
    {
        fprintf(stderr,
                "[!] xshell_finish():send_xfragment failed: cannot send finish fragment\r\n");
        return -1;
    }
    printf("[+] xshell_finish() finish fragment sends successfully\n\r");
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
