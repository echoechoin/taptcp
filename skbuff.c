#include "skbuff.h"

struct skbuff_t *skb_alloc(u_int32_t size)
{
    struct skbuff_t *skb = malloc(sizeof(struct skbuff_t));

    memset(skb, 0, sizeof(struct skbuff_t));
    skb->data = malloc(size);
    memset(skb->data, 0, size);

    skb->refcnt = 0;
    skb->head = skb->data;
    skb->end = skb->data + size;

    list_init(&skb->list);

    return skb;
}

void skb_free(struct skbuff_t *skb)
{
    if (skb->refcnt < 1) {
        free(skb->head);
        free(skb);
    }
}

void *skb_reserve(struct skbuff_t *skb, u_int32_t len)
{
    skb->data += len;

    return skb->data;
}

u_int8_t *skb_push(struct skbuff_t *skb, u_int32_t len)
{
    skb->data -= len;
    skb->len += len;

    return skb->data;
}

u_int8_t *skb_head(struct skbuff_t *skb)
{
    return skb->head;
}

void skb_reset_header(struct skbuff_t *skb)
{
    skb->data = skb->end - skb->dlen;
    skb->len = skb->dlen;
}
