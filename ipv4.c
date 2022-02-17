#include "ipv4.h"

struct iphdr {
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

u_int16_t checksum(void *addr, int count)
{
    register u_int32_t sum = 0;
    u_int16_t * ptr = addr;
    while( count > 1 )  {
        sum += * ptr++;
        count -= 2;
    }
    if( count > 0 )
        sum += * (u_int8_t *) ptr;
    while (sum>>16)
        sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}

