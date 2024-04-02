#ifndef X_TERMINAL_H
#define X_TERMINAL_H

/*
    Initalize Default terminal state
    return 0 on seccuess
    otherwise return -1;
*/
int xterminal_init();

/*
    reset teh Terminal attributes to state stored by calling xterminal_init
    return 0 on success
    otherwise return -1
*/
int xterminal_reset();

/*
    disable line buffering
    return 0 on success
    otherwise return -1
*/
int xterminal_disable_buffering();

/*
    disable echoing input keys
    reutrn 0 on success
    oterwise return -1
*/
int xterminal_disable_echoing();

/*
    enable echoing input keys
    reutrn 0 on success
    oterwise return -1
*/
int xterminal_enable_echoing();

/*
    enable line buffering
    reutrn 0 on success
    oterwise return -1
*/
int xterminal_enable_buffering();

#endif // X_TERMINAL_H