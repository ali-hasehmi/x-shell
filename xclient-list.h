#ifndef _X_CLIENT_LIST_H
#define _X_CLIENT_LIST_H

#include "xclient.h"

typedef struct
{
    xclient_list_node_t *next;
    xclient_t data;
   // xclient_node_t *prev;
} xclient_list_node_t;

typedef struct
{
    xclient_list_node_t *head;
    size_t size;
} xclient_list_t;

int xclient_list_create(xclient_list_t *_list);
int xclient_list_push_back(xclient_list_t *_list, const xclient_t *_client);
int xclient_list_push_front(xclient_list_t *_list, const xclient_t *_client);
int xclient_list_pop_back(xclient_list_t *_list);
int xclient_list_pop_front(xclient_list_t *_list);
xclient_t *xclient_list_find(xclient_list_t *_list, uint16_t _client_id);
int xclient_list_remove(xclient_list_t *_list, uint16_t _client_id);
int xclient_list_destroy(xclient_list_t *_list);

#endif // _X_CLIENT_LIST_H