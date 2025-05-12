#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <pthread.h>
#include <termios.h>

static struct termios old, current;

/* Initialize new terminal i/o settings */
void initTermios(int echo)
{
    tcgetattr(0, &old);         /* grab old terminal i/o settings */
    current = old;              /* make new settings same as old settings */
    current.c_lflag &= ~ICANON; /* disable buffered i/o */
    if (echo)
    {
        current.c_lflag |= ECHO; /* set echo mode */
    }
    else
    {
        current.c_lflag &= ~ECHO; /* set no echo mode */
    }
    tcsetattr(0, TCSANOW, &current); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void)
{
    tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo)
{
    char ch;
    initTermios(echo);
    ch = getchar();
    resetTermios();
    return ch;
}

/* Read 1 character without echo */
char getch(void)
{
    return getch_(0);
}

/* Read 1 character with echo */
char getche(void)
{
    return getch_(1);
}

typedef struct helper
{
    int fd;
    bool permit;
    char buff[1024];
} helper_t;

void *writing_worker(void *_arg)
{
    helper_t *help = _arg;
    while (1)
    {
        if (help->permit)
        {
            int q = write(help->fd, help->buff, strlen(help->buff));
            // printf("writing_worker(): %dbyte written!\r\n", q);
            help->permit = false;
        }
    }
    return NULL;
}
void *reading_worker(void *_arg)
{
    helper_t *help = _arg;
    while (1)
    {

        //  printf("start reading\n\r");
        int q = read(help->fd, help->buff, sizeof(help->buff));
        help->permit = false;
        // printf("reading_worker(): %dbyte read!\r\n", q);
        write(STDOUT_FILENO, help->buff, q);
        help->permit = true;
    }
    return NULL;
}
int main()
{
    // FILE *f = freopen("main.txt", "w+", stdout);
    //  if (f == NULL)
    //  {
    //      printf("failed()\n");
    //      exit(1);
    //  }
    //  write(STDOUT_FILENO,"\e[1;97m goodlord",15);
    //   pipe[0] -> read
    //   pipe[1] -> write
    fprintf(stderr, "goodlord!\n");
    int parent_to_child[2];
    int child_to_parent[2];
    if (pipe(parent_to_child) == -1)
    {
        perror("pipe() failed");
        exit(1);
    }
    if (pipe(child_to_parent) == -1)
    {
        perror("pipe() failed");
        exit(1);
    }
    int pid = fork();
    if (pid == -1)
    {
        perror("pipe() failed");
        exit(1);
    }

    // Child Process
    if (pid == 0)
    {
        // printf("Child started...\n");
        close(child_to_parent[0]);
        close(parent_to_child[1]);
        dup2(parent_to_child[0], STDIN_FILENO);
        dup2(child_to_parent[1], STDOUT_FILENO);
        dup2(child_to_parent[1], STDERR_FILENO);
        char *const argv[] = {"/bin/bash", NULL};
        execve("/bin/bash", argv, NULL);
        perror("execve failed!");
        exit(1);
    }

    // parent process
    else
    {
        close(child_to_parent[1]);
        close(parent_to_child[0]);
        helper_t write_helper, read_helper;
        read_helper.permit = true;
        write_helper.permit = false;
        write_helper.fd = parent_to_child[1];
        read_helper.fd = child_to_parent[0];
        pthread_t write_worker, read_worker;
        pthread_create(&write_worker, NULL, &writing_worker, (void *)&write_helper);
        pthread_create(&read_worker, NULL, &reading_worker, (void *)&read_helper);
        printf("Parent started...\nChild id: %d\n", pid);
        char buff[1024];
        char command[256];
        do
        {
            usleep(1000);
            do
            {
            } while (!read_helper.permit);
            printf("> ");
            write_helper.buff[0] = getch();
            write_helper.buff[1] = '\0';
            write_helper.permit = true;
            // printf("%dB write to pipe\n", q);
            // int fd = open("res.txt", O_WRONLY | O_CREAT, 0644);
            // int s = read(child_to_parent[0], buff, sizeof(buff));
            // printf("%s", buff);
        } while (strcmp(write_helper.buff, "exit\n"));
    }
    return 0;
}