#ifndef _ARP_H_
#define _ARP_H_
#include "ether.h"
#include "list.h"
#include "skbuff.h"
#include "netdev.h"
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

/*
 * arp 请求头
 *  _______________________________________________________ 
 * | hwtype | protype | hwsize | protosize | opcode | data |
 * |________|_________|________|___________|________|______|
 * | 2      | 2       | 1      | 1         | 2      | 0-20 |
 * |________|_________|________|___________|________|______|
 */

struct arp_hdr_t
{
    u_int16_t hwtype;
    u_int16_t protype;
    u_int8_t hwsize;
    u_int8_t prosize;
    u_int16_t opcode;
    u_int8_t data[];
} __attribute__((packed));

#define ARP_OPCODE_REQUEST 1
#define ARP_OPCODE_REPLY   2

#define HWTYPE_ETHERNET 0x0001

#define HWSIZE_ETHERNET 6
#define PROSIZE_IPV4 4

/*
 * ipv4 arp请求体
 *  ________________________________________________________________
 * | sndr_hwaddr | sndr_protoaddr | target_hwaddr | target_protoaddr |
 * |_____________|________________|_______________|__________________|
 * | 0-6         | 0-4            | 0-6           | 0-4              |
 * |_____________|________________|_______________|__________________|
 * 
 */

struct arp_ipv4_t
{
    u_int8_t smac[6];
    u_int32_t sip;
    u_int8_t dmac[6];
    u_int32_t dip;
} __attribute__((packed));

struct arp_entity_t
{
    u_int8_t mac[6];
    u_int32_t ip;
};

int arp_recv(struct skbuff_t *skb, struct netdev_t *dev);
int arp_request(u_int32_t sip, u_int32_t dip, struct netdev_t *dev);
int show_arp_table(struct netdev_t *dev);
void arp_packet_debug(struct skbuff_t *skb);

#endif
