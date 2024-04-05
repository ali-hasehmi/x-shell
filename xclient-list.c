#include "xclient-list.h"

#include <stdio.h>
#include <stdlib.h>

/*
    create an empty newe client list
    return 0 on success
    return -1 on failure
*/
int xclient_list_create(xclient_list_t *_list)
{
    _list->head = NULL;
    //_list->tail = NULL;
    _list->size = 0;
    return 0;
}


/*
    push the _client to end of the list 
    return 0 on success
    reutrn -1 on failure
*/
int xclient_list_push_back(xclient_list_t *_list, const xclient_t *_client)
{
    /*
        create a new node
    */
    xclient_list_node_t *new_node = malloc(sizeof(xclient_t));
    if (new_node == NULL)
    {
        fprintf(stderr,
                "[!] xclient_list_push_back() failed: cannot allocate memory!\r\n");
        return -1;
    }
    new_node->data = *_client;
    new_node->next = NULL;

    xclient_list_node_t **tmp = &_list->head;
    while (*tmp)
    {
        tmp = &((*tmp)->next);
    }
    (*tmp)->next = new_node;
    return 0;
}

/*
    destroy a client list
    return 0 on success
    return -1 on failure
*/
int xclient_list_destroy(xclient_list_t *_list)
{
    xclient_list_node_t *next,
        *curr = _list->head;
    while (curr != NULL)
    {
        next = curr->next;
        free(curr);
        curr = next;
    }
    return 0;
}
