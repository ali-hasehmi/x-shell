#include "xmessage_queue.h"

#include <stdio.h>

/*
    create a circular queue for xmessages
    return 0 on success
    return -1 on failure
*/
int xmessage_queue_create(xmessage_queue_t *_queue)
{
    if (pthread_mutex_init(&_queue->queue_mutex, NULL))
    {
        return -1;
    }
    if (pthread_cond_init(&_queue->not_empty, NULL))
    {
        return -1;
    }
    if (pthread_cond_init(&_queue->not_full, NULL))
    {
        return -1;
    }
    _queue->front = -1;
    _queue->rear = -1;
    return 0;
}

/*
    destroyes the queue
    return 0 on success
    return -1 on failure
*/
int xmessage_queue_destroy(xmessage_queue_t *_queue)
{
    if (pthread_mutex_destroy(&_queue->queue_mutex))
    {
        return -1;
    }
    if (pthread_cond_destroy(&_queue->not_empty))
    {
        return -1;
    }
    if (pthread_cond_destroy(&_queue->not_full))
    {
        return -1;
    }
    return 0;
}

/*
    check if queue is empty
    return true in case of emptiness
    return false otehrwise
*/
bool xmessage_queue_isEmpty(xmessage_queue_t *_queue)
{
    return (_queue->rear == -1 && _queue->front == -1);
}

/*
    check if queue is full
    return true if queue if full
    return false otherwise
*/
bool xmessage_queue_isFull(xmessage_queue_t *_queue)
{
    return (_queue->front == ((_queue->rear + 1) % MAX_QUEUE_SIZE));
}

/*
    push a new message pointed by _msg to _queueu
    if queue is full return -1
    otherwise return 0
*/
int xmessage_queue_enqueue(xmessage_queue_t *_queue, xmessage_t *_msg)
{
    pthread_mutex_lock(&_queue->queue_mutex);
    while (xmessage_queue_isFull(_queue))
    {
        // wait for queue to have more space
        // but for now just return -1 and print the queue is full
        printf("Queue is Full! waiting to have space\n");
        pthread_cond_wait(&_queue->not_full, &_queue->queue_mutex);
        // return -1;
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
    pthread_cond_signal(&_queue->not_empty);
    pthread_mutex_unlock(&_queue->queue_mutex);
    return 0;
}

/*
    pop the front element from _queue and put it in _msg
    return -1 if queue is empty
    return 0 otherwise
*/
int xmessage_queue_dequeue(xmessage_queue_t *_queue, xmessage_t *_msg)
{
    pthread_mutex_lock(&_queue->queue_mutex);
    while (xmessage_queue_isEmpty(_queue))
    {
        // wait to have data in queue to return
        // but for now just return -1 and print in terminal
        printf("Queue is Empty! waiting to enqueue something...\n");
        pthread_cond_wait(&_queue->not_empty, &_queue->queue_mutex);
        // return -1;
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
    pthread_cond_signal(&_queue->not_full);
    pthread_mutex_unlock(&_queue->queue_mutex);

    return 0;
}