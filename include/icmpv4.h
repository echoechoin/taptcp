#ifndef _ICMPV4_H_
#define _ICMPV4_H_
#include "skbuff.h"
#include "netdev.h"
#include "arp.h"
#include "ipv4.h"
#include "event.h"
#include "server.h"
#include "stdio.h"
#include "ipv4.h"

struct icmp_v4_t {
    u_int8_t type;
    u_int8_t code;
    u_int16_t csum;
    u_int8_t data[];
} __attribute__((packed));

struct icmp_v4_echo_reply_t { 
    u_int16_t id;
    u_int16_t seq;
    u_int8_t data[];
} __attribute__((packed));

struct icmp_v4_dst_unreachable {
    u_int8_t unused;
    u_int8_t len;
    u_int16_t var;
    u_int8_t data[];
} __attribute__((packed));

#define ICMPV4_TYPE_ECHO_REPLY 0
#define ICMPV4_TYPE_DEST_UNREACH 3
#define ICMPV4_TYPE_ECHO_REQUEST 8

int icmpv4_recv(struct skbuff_t *skb, struct netdev_t *dev);

#endif
