#ifndef _NETDEV_H_
#define _NETDEV_H_
#include "sys/types.h"
#include "list.h"
#include "arpa/inet.h"

struct netdev_t {
    u_int32_t addr;
    u_int8_t addr_len;
    u_int8_t hwaddr[6];
    u_int32_t mtu;

    list_t *arp_table;
};

struct netdev_t *netdev_alloc(char *addr, char *hwaddr, u_int32_t mtu);
void netdev_free(struct netdev_t *netdev);

#endif
