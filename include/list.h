#ifndef _LIST_H_
#define _LIST_H_
#include <stdio.h>
#include <stdlib.h>

typedef struct _list_t {
    void *data;
    struct _list_t *next;
    struct _list_t *prev;
} list_t;


list_t *list_init();
int list_add(list_t *list, void *data);
int list_remove(list_t *list, list_t *node, void *(free_func)(void *));
int list_destroy(list_t *list, void *(free_func)(void *));

#endif
