#include "ether.h"

struct eth_hdr_t *get_eth_hdr(struct skbuff_t *skb) {
    struct eth_hdr_t *hdr = (struct eth_hdr_t *)skb_head(skb);
    return hdr;
}

int get_mac_hex_type(u_int8_t *mac, char *s) {
    int i;
    for (i = 0; i < 6; i++) {
        if (i == 5) {
            sprintf(s, "%02x", mac[i]);
            break;
        }
        sprintf(s, "%02x-", mac[i]);
        s += 3;
    }
    return 0;
}

int get_ethertype_hex_type(u_int16_t ethertype, char *s)
{
    sprintf(s, "0x%04x", ethertype);
    return 0;
}

int get_mac_address(char *dev_name, u_int8_t *mac) {
    struct ifreq ifreq;
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    strcpy(ifreq.ifr_name, dev_name);

    if (ioctl (sock, SIOCGIFHWADDR, &ifreq) < 0) {
        perror("ioctl");
        return -1;
    }
    memcpy(mac, ifreq.ifr_hwaddr.sa_data, 6);
    return 0;
}

void ether_packet_debug(struct skbuff_t *skb) {
    struct eth_hdr_t *hdr = get_eth_hdr(skb);
    char mac_addr[20] = {0};
    char ethertype[20] = {0};
    get_mac_hex_type(hdr->smac, mac_addr);
    printf("    ether_packet_debug: smac: %s\n", mac_addr);
    get_mac_hex_type(hdr->dmac, mac_addr);
    printf("    ether_packet_debug: dmac: %s\n", mac_addr);
    get_ethertype_hex_type(hdr->ethertype, ethertype);
    printf("    ether_packet_debug: ethertype: %s\n", ethertype);
}