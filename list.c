#include "list.h"

list_t *list_init() {
    list_t *list = (list_t *)malloc(sizeof(list_t));
    if (list == NULL) {
        return NULL;
    }
    list->data = NULL;
    list->next = list;
    list->prev = list;
    return list;
}

int list_add(list_t *list, void *data) {
    list_t *last = list->prev;
    list_t *new_node = (list_t *)malloc(sizeof(list_t));
    if (new_node == NULL) {
        return -1;
    }
    new_node->data = data;
    last->next = new_node;
    new_node->prev = last;
    new_node->next = list;
    list->prev = new_node;
    return 0;
}

int list_remove(list_t *list, list_t *node, void (*free_func)(void *)) {
    if (list == NULL || node == NULL) {
        return -1;
    }
    node->prev->next = node->next;
    node->next->prev = node->prev;
    if (free_func != NULL) {
        free_func(node->data);
    }
    free(node);
    return 0;
}

int list_destroy(list_t *list, void (*free_func)(void *)) {
    if (list == NULL) {
        return -1;
    }
    list_t *current = list->next;
    while (current != list) {
        list_t *tmp = current;
        current = current->next;
        if (free_func != NULL) {
            free_func(tmp->data);
        }
        free(tmp);
    }
    free(list);
    return 0;
}