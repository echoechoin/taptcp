#ifndef _IPV4_H_
#define _IPV4_H_

#include "ether.h"
#include "sys/types.h"
#include "skbuff.h"
#include "icmpv4.h"
#include "netdev.h"
#include "ether.h"

struct ipv4_hdr_t {
    u_int8_t ihl : 4;  // header length 以4个字节为一个单位
    u_int8_t version : 4;
    u_int8_t tos;                      // type of service
    u_int16_t len;                     // total length
    u_int16_t id;                      // identification 同一个ip报文的id相同
    u_int16_t flags : 3;               // reserved|DF|MF
    u_int16_t frag_offset : 13;        // fragment offset
    u_int8_t ttl;
    u_int8_t proto;                    // protocol
    u_int16_t csum;
    u_int32_t saddr;
    u_int32_t daddr;
    u_int8_t data[];
} __attribute__((packed));

#define IPV4 0x04

#define ICMPV4_PROTO 1
#define TCP_PROTO 6
#define UDP_PROTO 17

void ipv4_packet_debug(struct skbuff_t *skb);
u_int16_t checksum(void *addr, int count);
int ipv4_recv(struct skbuff_t *skb, struct netdev_t *dev);
struct ipv4_hdr_t *get_ipv4_hdr(struct skbuff_t *skb);

#endif
