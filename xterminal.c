#include "xterminal.h"
#include <termio.h>
#include <stdio.h>
#include <errno.h>

static struct termios init_terminal, current_terminal;

int xterminal_init()
{
    // store the initial attributes of the terminal
    if (tcgetattr(0, &init_terminal) == -1)
    {
        fprintf(stderr,
                "[!] tcgetattr() failed: %s\r\n",
                strerr(errno));
        return -1;
    }
    current_terminal = init_terminal;
    return 0;
}

int xterminal_reset()
{
    if (tcsetattr(0, TCSANOW, &init_terminal) == -1)
    {
        fprintf(stderr,
                "[!] tcsetattr() failed: %s\r\n",
                strerr(errno));
        return -1;
    }
    return 0;
}

int xterminal_disable_buffering()
{
    current_terminal.c_lflag &= ~ICANON;
    if (tcsetattr(0, TCSANOW, &current_terminal) == -1)
    {
        fprintf(stderr,
                "[!] tcsetattr() failed: %s\r\n",
                strerr(errno));
        return -1;
    }
    return 0;
}

int xterminal_disable_echoing()
{
    current_terminal.c_lflag &= ~ECHO;
    if (tcsetattr(0, TCSANOW, &current_terminal) == -1)
    {
        fprintf(stderr,
                "[!] tcsetattr() failed: %s\r\n",
                strerr(errno));
        return -1;
    }
    return 0;
}

int xterminal_enable_echoing()
{
    current_terminal.c_lflag |= ECHO;
    if (tcsetattr(0, TCSANOW, &current_terminal) == -1)
    {
        fprintf(stderr,
                "[!] tcsetattr() failed: %s\r\n",
                strerr(errno));
        return -1;
    }
    return 0;
}

int xterminal_enable_buffering()
{
    current_terminal.c_lflag |= ICANON;
    if (tcsetattr(0, TCSANOW, &current_terminal) == -1)
    {
        fprintf(stderr,
                "[!] tcsetattr() failed: %s\r\n",
                strerr(errno));
        return -1;
    }
    return 0;
}
