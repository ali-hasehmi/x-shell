#include "xclient.h"
#include <stdio.h>
#include <string.h>

/*
    filled _client fields with given arugments
    return 0 on success
    return -1 on failure
*/
int xclient_create(xclient_t *_client, const xtcpsocket_t *_socket, const char *_hostname, const char *_logname)
{
    _client->socket = *_socket;
    _client->id = 0; // means, the client hasn't any id yet
    if (strncpy(_client->hostname, _hostname, HOSTNAME_MAX_LEN) == NULL)
    {
        return -1;
    }
    if (strncpy(_client->logname, _logname, LOGNAME_MAX_LEN) == NULL)
    {
        return -1;
    }
    if (strncpy(_client->localname, _logname, LOCALNAME_MAX_LEN) == NULL)
    {
        return -1;
    }
    return 0;
}

int xclient_set_localname(xclient_t *_client, const char *_new_local_name)
{
    if (strncpy(_client->localname, _new_local_name, LOCALNAME_MAX_LEN) == NULL)
    {
        return -1;
    }
    return 0;
}

void xclient_print(const xclient_t *_client)
{
    printf("%-15hu%-30s%-30s%-30s\n",
           _client->id,
           _client->logname,
           _client->hostname,
           _client->localname);
}
