#include "queue.h"

queue_t *queue_new()
{
    queue_t *queue = (queue_t *)malloc(sizeof(queue_t));
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    queue->queue_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(queue->queue_mutex, NULL);
    return queue;
}
void queue_free(queue_t *queue, void (*free_func)(void *))
{
    pthread_mutex_lock(queue->queue_mutex);
    if (queue == NULL)
        return;
    queue_node_t *node = queue->head;
    while (node != NULL) {
        queue_node_t *next = node->next;
        if (free_func != NULL)
            free_func(node->data);
        free(node);
        node = next;
    }
    free(queue);
    queue = NULL;
}

void queue_push(queue_t *queue, void *data)
{
    pthread_mutex_lock(queue->queue_mutex);

    queue_node_t *node = (queue_node_t *)malloc(sizeof(queue_node_t));
    node->data = data;
    node->next = NULL;
    node->prev = NULL;
    if (queue->head == NULL) {
        queue->head = node;
        queue->tail = node;
    } else {
        queue->tail->next = node;
        node->prev = queue->tail;
        queue->tail = node;
    }
    queue->size++;
    pthread_mutex_unlock(queue->queue_mutex);
}
void *queue_pop(queue_t *queue)
{
    pthread_mutex_lock(queue->queue_mutex);
    if (queue == NULL)
        return NULL;
    if (queue->head == NULL)
        return NULL;
    queue_node_t *node = queue->head;
    queue->head = node->next;
    if (queue->head != NULL)
        queue->head->prev = NULL;
    else
        queue->tail = NULL;
    queue->size--;
    void *data = node->data;
    free(node);
    pthread_mutex_unlock(queue->queue_mutex);
    return data;
}
void *queue_peek(queue_t *queue)
{
    if (queue == NULL)
        return NULL;
    if (queue->head == NULL)
        return NULL;
    return queue->head->data;
}
int queue_size(queue_t *queue)
{
    if (queue == NULL)
        return -1;
    return queue->size;
}