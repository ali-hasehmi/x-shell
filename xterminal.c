#include "xterminal.h"
#include <termio.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

static struct termios
    init_terminal_state,
    save_terminal_state,
    current_terminal_state;

static bool is_in_raw_mode = false;

int xterminal_init()
{
    // store the initial attributes of the terminal
    if (tcgetattr(0, &init_terminal_state) == -1)
    {
        fprintf(stderr,
                "[!] tcgetattr() failed: %s\r\n",
                strerror(errno));
        return -1;
    }
    current_terminal_state = init_terminal_state;

    // Register the xterminal_reset at exiting
    // Make sure terminal setting after this program wil
    // set back to initial state
    atexit(&xterminal_reset);
    return 0;
}

void xterminal_reset()
{
    if (tcsetattr(0, TCSANOW, &init_terminal_state) == -1)
    {
        fprintf(stderr,
                "[!] tcsetattr() failed: %s\r\n",
                strerror(errno));
    }
    
}

int xterminal_disable_buffering()
{
    current_terminal_state.c_lflag &= ~ICANON;
    if (tcsetattr(0, TCSANOW, &current_terminal_state) == -1)
    {
        fprintf(stderr,
                "[!] tcsetattr() failed: %s\r\n",
                strerror(errno));
        return -1;
    }
    return 0;
}

int xterminal_disable_echoing()
{
    current_terminal_state.c_lflag &= ~ECHO;
    if (tcsetattr(0, TCSANOW, &current_terminal_state) == -1)
    {
        fprintf(stderr,
                "[!] tcsetattr() failed: %s\r\n",
                strerror(errno));
        return -1;
    }
    return 0;
}

int xterminal_enable_echoing()
{
    current_terminal_state.c_lflag |= ECHO;
    if (tcsetattr(0, TCSANOW, &current_terminal_state) == -1)
    {
        fprintf(stderr,
                "[!] tcsetattr() failed: %s\r\n",
                strerror(errno));
        return -1;
    }
    return 0;
}

int xterminal_enable_buffering()
{
    current_terminal_state.c_lflag |= ICANON;
    if (tcsetattr(0, TCSANOW, &current_terminal_state) == -1)
    {
        fprintf(stderr,
                "[!] tcsetattr() failed: %s\r\n",
                strerror(errno));
        return -1;
    }
    return 0;
}

int xterminal_enable_raw_mode()
{
    if (is_in_raw_mode)
    {
        // Already in raw mode
        // not an error really
        return 0;
    }
    save_terminal_state = current_terminal_state;

    current_terminal_state.c_iflag |= IGNPAR;
    current_terminal_state.c_iflag &= ~(ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXANY | IXOFF);
#ifdef IUCLC
    current_terminal_state.c_iflag &= ~IUCLC;
#endif
    current_terminal_state.c_lflag &= ~(ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHONL);
#ifdef IEXTEN
    current_terminal_state.c_lflag &= ~IEXTEN;
#endif
    current_terminal_state.c_oflag &= ~OPOST;
    current_terminal_state.c_cc[VMIN] = 1;
    current_terminal_state.c_cc[VTIME] = 0;
    if (tcsetattr(fileno(stdin), TCSADRAIN, &current_terminal_state) == -1)
    {
        perror("[!] xterminal_enable_raw_mode():tcsetattr failed");
        return -1;
    }

    is_in_raw_mode = true;
}

int xterminal_disable_raw_mode()
{
    if (!is_in_raw_mode)
    {
        // Already not in raw mode
        // not an error really
        return 0;
    }

    if (tcsetattr(STDIN_FILENO, TCSADRAIN, &save_terminal_state) == -1)
    {
        perror("[!] xterminal_disable_raw_mode():tcsetattr() failed");
        return -1;
    }
    is_in_raw_mode = false;
    return 0;
}
