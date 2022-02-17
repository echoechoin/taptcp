#ifndef _IPV4_H_
#define _IPV4_H_

#include "ether.h"
#include "sys/types.h"
// #include "icmpv4.h"

struct ipv4_hdr_t {
    u_int8_t version : 4;
    u_int8_t ihl : 4;
    u_int8_t tos;
    u_int16_t len;
    u_int16_t id;
    u_int16_t flags : 3;
    u_int16_t frag_offset : 13;
    u_int8_t ttl;
    u_int8_t proto;
    u_int16_t csum;
    u_int32_t saddr;
    u_int32_t daddr;
} __attribute__((packed));

u_int16_t checksum(void *addr, int count);
int ipv4_process(int fd, struct eth_hdr_t *eth_hdr);

#endif
