#include "netdev.h"

struct netdev_t *netdev_alloc(char *addr, char *hwaddr, u_int32_t mtu)
{
    struct netdev_t *dev = malloc(sizeof(struct netdev_t));

    dev->addr = inet_addr(addr);

    sscanf(hwaddr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &dev->hwaddr[0],
                                                    &dev->hwaddr[1],
                                                    &dev->hwaddr[2],
                                                    &dev->hwaddr[3],
                                                    &dev->hwaddr[4],
                                                    &dev->hwaddr[5]);

    dev->addr_len = 6;
    dev->mtu = mtu;
    dev->arp_table = list_init();
    return dev;
}

void netdev_free(struct netdev_t *netdev)
{
    free(netdev);
    list_destroy(netdev->arp_table, free);
}