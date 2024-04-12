#include "xclient-list.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef struct xclient_list_node
{
    struct xclient_list_node *next;
    xclient_t data;
    // xclient_node_t *prev;
} xclient_list_node_t;

static uint16_t bit_used_ids[(UINT16_MAX + 1) / 8] = {0};

uint16_t generate_random_unique_id()
{
    uint16_t generated_id;
    uint16_t inx;
    do
    {
        generated_id = rand() % UINT16_MAX + 1;
        inx = (uint16_t)ceil(((double)(generated_id)) / ((double)8));
    } while ((bit_used_ids[inx] >> (inx * 8 - generated_id)) & 1);
    (bit_used_ids[inx] |= 1 << (inx * 8 - generated_id));
    return generated_id;
}

void free_random_unique_id(uint16_t _id)
{
    uint16_t inx = (uint16_t)ceil(((double)(_id)) / ((double)8));
    (bit_used_ids[inx] &= ~(1 << (inx * 8 - _id)));
}

void clear_bit_used_ids()
{
    memset(bit_used_ids, 0, (UINT16_MAX + 1) / 8);
}
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
int xclient_list_push_back(xclient_list_t *_list, xclient_t *_client)
{
    /*
        create a new node
    */
    xclient_list_node_t *new_node = (xclient_list_node_t *)malloc(sizeof(xclient_list_node_t));
    if (new_node == NULL)
    {
        fprintf(stderr,
                "[!] xclient_list_push_back() failed: cannot allocate memory!\r\n");
        return -1;
    }
    _client->id = generate_random_unique_id();
    new_node->data = *_client;
    new_node->next = NULL;

    if (_list->head == NULL)
    {
        _list->head = new_node;
        return 0;
    }

    xclient_list_node_t *currNode = _list->head;

    while (currNode->next)
    {
        currNode = currNode->next;
    }
    currNode->next = new_node;

    return 0;
}

/*
    push the _client at the beggining of the _list
    return 0 on success
    return -1 on failure
*/
int xclient_list_push_front(xclient_list_t *_list, xclient_t *_client)
{
    /*
        create a new node
    */
    xclient_list_node_t *new_node = (xclient_list_node_t *)malloc(sizeof(xclient_t));
    if (new_node == NULL)
    {
        fprintf(stderr,
                "[!] xclient_list_push_front() failed: cannot allocate memory!\r\n");
        return -1;
    }
    _client->id = generate_random_unique_id();
    new_node->data = *_client;
    new_node->next = _list->head;
    _list->head = new_node;
    return 0;
}

/*
    Delete the last element in _list
    return 0 on success
    return 1 if list is empty
*/
int xclient_list_pop_back(xclient_list_t *_list)
{
    /* if list is empty*/
    if (_list->head == NULL)
    {
        return 0;
    }

    if (_list->head->next == NULL)
    {
        free(_list->head);
        _list->head = NULL;
        return 0;
    }

    xclient_list_node_t *curr = _list->head,
                        *prev;
    while (curr->next)
    {
        prev = curr;
        curr = curr->next;
    }
    free(curr);
    prev->next = NULL;
    return 0;
}

/*
    Delete the first element in _list
    return 0 on success
    return 1 if list is empty
*/
int xclient_list_pop_front(xclient_list_t *_list)
{
    if (_list->head == NULL)
    {
        // List is empty
        return 1;
    }
    xclient_list_node_t *tmp = _list->head->next;
    free(_list->head);
    _list->head = tmp;
    return 0;
}

/*
    find a client in the list by its id
    return pointer to client on success
    return null if client doesn't exist in the list
*/
xclient_t *xclient_list_find(xclient_list_t *_list, uint16_t _client_id)
{
    for (xclient_list_node_t *curr = _list->head;
         curr != NULL;
         curr = curr->next)
    {
        if (curr->data.id == _client_id)
        {
            return &(curr->data);
        }
    }
    return NULL;
}

/*
    Delete a Client with given id
    return 0 on success
    return 1 if user with given id doesn't exist
*/
int xclient_list_remove(xclient_list_t *_list, uint16_t _client_id)
{
    xclient_list_node_t *prevNode = NULL,
                        *currNode = _list->head;
    while (currNode)
    {
        if (currNode->data.id == _client_id)
        {
            break;
        }
        prevNode = currNode;
        currNode = currNode->next;
    }
    if (currNode == NULL)
    {
        // client with given id doesn't exist
        return 1;
    }
    if (prevNode == NULL)
    {
        _list->head = currNode->next;
    }
    else
    {
        prevNode->next = currNode->next;
    }
    xtcpsocket_close(&currNode->data.socket);
    free(currNode);
    free_random_unique_id(_client_id);
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
        xtcpsocket_close(&curr->data.socket);
        free(curr);
        curr = next;
    }
    return 0;
}

void xclient_list_print(const xclient_list_t *_list)
{
    printf("\n%-7s%-32s%-30s%-30s\n", "#id",
           "#username",
           "#hostname",
           "#localname");
    for (xclient_list_node_t *currNode = _list->head;
         currNode != NULL;
         currNode = currNode->next)
    {
        xclient_print(&currNode->data);
    }
    putchar('\n');
}
