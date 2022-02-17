#ifndef _ETHER_H_
#define _ETHER_H_
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <net/if.h> 
#include <string.h>

/*
 *  ___________________________________
 * | dmac | smac | ethertype | payload |
 * |______|______|___________|_________|
 * | 6    | 6    | 2         | 0-1500  |
 * |______|______|___________|_________|
 */

struct eth_hdr_t
{
    u_int8_t  dmac[6];
    u_int8_t  smac[6];
    u_int16_t ethertype;
    u_int8_t  payload[];
} __attribute__((packed));

#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_ARP  0x0806

int get_mac_hex_type(u_int8_t *mac, char *s);
int get_ethertype_hex_type(u_int16_t ethertype, char *s);
int get_mac_address(char *dev_name, u_int8_t *mac);

#endif // _ETHER_H_