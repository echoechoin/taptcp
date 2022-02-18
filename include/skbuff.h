#ifndef SKBUFF_H_
#define SKBUFF_H_
#include "list.h"
#include <sys/types.h>
#include <string.h>

#define BUFSIZE 1600

struct skbuff_t {
    list_t list;
    struct rtentry *rt;
    struct netdev *dev;
    int refcnt;
    u_int16_t protocol;
    u_int32_t len;
    u_int32_t dlen;
    u_int32_t seq;
    u_int32_t end_seq;
    u_int8_t *end;
    u_int8_t *head;
    u_int8_t *data;
    u_int8_t *payload;
};

struct skbuff_t *skb_alloc(u_int32_t size);
void skb_free(struct skbuff_t *skb);
u_int8_t *skb_push(struct skbuff_t *skb, u_int32_t len);
u_int8_t *skb_head(struct skbuff_t *skb);
void *skb_reserve(struct skbuff_t *skb, u_int32_t len);
void skb_reset_header(struct skbuff_t *skb);

#endif
