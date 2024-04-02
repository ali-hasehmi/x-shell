#include "xtcpsocket.h"
#include <stdio.h>
#include <string.h>
#include "xfragment.h"
#include <stdlib.h>

int main()
{

    char server_ip[16];
    uint16_t server_port;
    printf("IP: ");
    scanf("%s", server_ip);
    printf("port: ");
    scanf("%hu", &server_port);

    xtcpsocket_t socket;

    if (xtcpsocket_create(&socket, AF_INET, xst_client))
    {
        return -1;
    }

    if (xtcpsocket_connect(&socket, server_ip, server_port))
    {
        return -1;
    }
    printf("[+] connected successfully\n");
    char file_path[256];
    printf("file path: ");
    scanf("%s", file_path);

    xfile_t file;
    if (xfopen(file_path, &file, XF_RDONLY))
    {
        perror("[!] xfopen() failed: ");
        exit(1);
    }

    if (make_xhandshake(&socket, &file) == -1)
    {
        printf("[!] make_xhandshake() failed\n");
        exit(1);
    }
    xfragment_t frag;
    int i = 0;
    while (1)
    {
        if (write_file_to_fragment(&file, &frag) == -1)
        {
            perror("[!] write_file_to_fragment() failed: ");
            exit(1);
        }
        if (send_xfragment(&socket, &frag) == -1)
        {
            perror("[!] send_xfragment() failed: ");
            exit(1);
        }
        printf(" -%d- \n",++i);
        //printf("[+] %huByte send successfuly\n", frag.buff_len);
    }
    return 0;
}