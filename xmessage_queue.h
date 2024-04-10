#ifndef X_MESSAGE_QUEUE_H
#define X_MESSAGE_QUEUE_H

#include "xmessage.h"
#include <stdbool.h>

#define MAX_QUEUE_SIZE 32

typedef struct xmessage_queue
{
    int front;
    int rear;
    xmessage_t buff[MAX_QUEUE_SIZE];
} xmessage_queue_t;

int xmessage_queue_create(xmessage_queue_t *_queue);
bool xmessage_queue_isEmpty(xmessage_queue_t *_queue);
bool xmessage_queue_isFull(xmessage_queue_t *_queue);
int xmessage_queue_enqueue(xmessage_queue_t *_queue, xmessage_t *_msg);
int xmessage_queue_dequeue(xmessage_queue_t *_queue, xmessage_t *_msg);

#endif // X_MESSAGE_QUEUE_H