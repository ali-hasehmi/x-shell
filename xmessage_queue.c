#include "xmessage_queue.h"

#include <stdio.h>

int xmessage_queue_create(xmessage_queue_t *_queue)
{
    _queue->front = -1;
    _queue->rear = -1;
    return 0;
}

bool xmessage_queue_isEmpty(xmessage_queue_t *_queue)
{
    return (_queue->rear == -1 && _queue->front == -1);
}

bool xmessage_queue_isFull(xmessage_queue_t *_queue)
{
    return (_queue->front == ((_queue->rear + 1) % MAX_QUEUE_SIZE));
}

int xmessage_queue_enqueue(xmessage_queue_t *_queue, xmessage_t *_msg)
{
    if (xmessage_queue_isFull(_queue))
    {
        // wait for queue to have more space
        // but for now just return -1 and print the queue is full
        printf("Queue is Full!\n");
        return -1;
    }
    if (xmessage_queue_isEmpty(_queue))
    {
        _queue->front = _queue->rear = 0;
    }
    else
    {
        _queue->rear = (_queue->rear + 1) % MAX_QUEUE_SIZE;
    }
    _queue->buff[_queue->rear] = *_msg;
    return 0;
}

int xmessage_queue_dequeue(xmessage_queue_t *_queue, xmessage_t *_msg)
{
    if (xmessage_queue_isEmpty(_queue))
    {
        // wait to have data in queue to return
        // but for now just return -1 and print in terminal
        printf("Queue is Empty!\n");
        return -1;
    }
    if (_msg != NULL)
    {
        *_msg = _queue->buff[_queue->front];
    }
    // if there is only one element in the queue
    if (_queue->front == _queue->rear)
    {
        _queue->front = _queue->rear = -1;
    }
    // there is more than one element
    else
    {
        _queue->front = (_queue->front + 1) % MAX_QUEUE_SIZE;
    }
    return 0;
}