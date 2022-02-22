#ifndef _TCP_H_
#define _TCP_H_
#include <sys/types.h>
#include "ipv4.h"
#include "skbuff.h"
#include "netdev.h"

struct tcp_hdr_t {
    u_int16_t sport;
    u_int16_t dport;
    u_int32_t seq;
    u_int32_t ack_seq;
    u_int8_t rsvd : 4;
    u_int8_t hl : 4;
    u_int8_t flags;
    u_int16_t win;
    u_int16_t csum;
    u_int16_t urp;
    u_int8_t data[];
} __attribute__((packed));

struct tcp_opt_t {
    u_int16_t options;
    u_int16_t mss;
    u_int8_t sack;
};

struct tcp_opt_mss {
    u_int8_t kind;
    u_int8_t len;
    u_int16_t mss;
} __attribute__((packed));

#define TCP_FLAG_FIN 0x01
#define TCP_FLAG_SYN 0x02
#define TCP_FLAG_RST 0x04
#define TCP_FLAG_PSH 0x08
#define TCP_FLAG_ACK 0x10
#define TCP_FLAG_URG 0x20
#define TCP_FLAG_ECE 0x40
#define TCP_FLAG_CWR 0x80

int tcp_recv(struct skbuff_t *skb, struct netdev_t *dev);
int tcp_send(struct skbuff_t *skb, struct netdev_t *dev);
int tcp_checksum(struct ipv4_hdr_t *ipv4_hdr, struct tcp_hdr_t *tcp_hdr);
void tcp_packet_debug(struct skbuff_t *skb);

#endif
