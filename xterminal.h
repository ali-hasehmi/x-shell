#ifndef X_TERMINAL_H
#define X_TERMINAL_H

int xterminal_init();
int xterminal_reset();
int xterminal_disable_buffering();
int xterminal_disable_echoing();
int xterminal_enable_echoing();
int xterminal_enable_buffering();

#endif // X_TERMINAL_H