#include "xtcpsocket.h"
#include <stdio.h>
#include <string.h>
#include "xfragment.h"
#include "xfile.h"
#include <stdlib.h>

void printbyte(const char *_buf, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        printf("%x ", _buf[i]);
    }
    putchar('\n');
}
int main()
{

    xtcpsocket_t socket;
    if (xtcpsocket_create(&socket, AF_INET, xst_server))
    {
        return -1;
    }
    if (xtcpsocket_bind(&socket, NULL, 0))
    {
        return -1;
    }
    char host_ip[20];
    uint16_t host_port;
    if (xtcpsocket_gethost(&socket, host_ip, 20, &host_port))
    {
        return -1;
    }
    if (xtcpsocket_listen(&socket, 6))
    {
        return -1;
    }
    printf("[+] listening on %s:%hu\n", host_ip, host_port);

    xtcpsocket_t client;
    while (xtcpsocket_accept(&socket, &client) != -1)
    {
        char ip[32];
        uint16_t port;
        xtcpsocket_getremote(&client, ip, 32, &port);
        printf("[+] %s:%hu connected!\n", ip, port);
        xfile_t file;
        if (make_xhandshake_d(&client, &file) == -1)
        {
            printf("[!] make_xhandshake_d() failed\n");
            exit(1);
        }
        xfragment_t frag;
        while (file.x_rel_size < file.x_size)
        {
            recv_xfragment(&client, &frag);
            write_fragment_to_file(&frag, &file);
        }
        printf("[+] file received Completely!\n");
        fflush(stdout);

        // printbyte(buff,512);
    }
    return 0;
}