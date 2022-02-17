#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "tuntap.h"
#include "ether.h"
#include "arp.h"

#define BUFSIZE 2000

int main () {
    char dev[IFNAMSIZ] = "tap0";
    char *buf = malloc(BUFSIZE);
    char tmp_mac_addr[20] = {0};
    char tmp_ethertype[20] = {0};
    u_int8_t mac[6] = {0};
    // IFF_NO_PI is crucial: Do not add protocol information header to packets.
    //                       otherwise we end up with unnecessary packet infor-
    //                       mation prepended to the Ethernet frame.
    int fd = tun_alloc(dev, IFF_TAP | IFF_NO_PI);
    
    system("sudo bash ./uptap.sh");

    get_mac_address(dev, mac);
    get_mac_hex_type(mac, tmp_mac_addr);
    printf("%s's mac: %s\n", dev, tmp_mac_addr);

    arp_init(mac, inet_addr("192.168.200.100"));
    while(1) {
        if (read(fd, buf, BUFSIZE) < 0) {
            perror("read");
            return 1;
        }
        struct eth_hdr_t *hdr = (struct eth_hdr_t *) buf; 

        if (ntohs(hdr->ethertype) == ETHERTYPE_ARP) {
            arp_process(fd, hdr);
        }
    }
    
    
    return 0;
}