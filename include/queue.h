#ifndef _QUEUE_H_
#define _QUEUE_H_
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct _queue_node_t {
    struct _queue_node_t *next;
    struct _queue_node_t *prev;
    void *data;
} queue_node_t;

typedef struct _queue_t {
    pthread_mutex_t * queue_mutex;
    queue_node_t *head;
    queue_node_t *tail;
    int size;
} queue_t;


queue_t *queue_new();
void queue_free(queue_t *queue, void (*free_func)(void *));
void queue_push(queue_t *queue, void *data);
void *queue_pop(queue_t *queue);
void *queue_peek(queue_t *queue);
int queue_size(queue_t *queue);

#endif
