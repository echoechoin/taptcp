#include "ether.h"

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
    u_int8_t tmp[2] = {0};
    tmp[0] = (ethertype >> 8) & 0xff;
    tmp[1] = ethertype & 0xff; 
    sprintf(s, "0x%02x%02x", tmp[1], tmp[0]);
    return 0;
}

int get_mac_address(char *dev_name, u_int8_t *mac) {
    struct ifreq ifreq;
    int sock;
    printf("get mac address: %s\n", dev_name);
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    strcpy(ifreq.ifr_name, dev_name);

    if (ioctl (sock, SIOCGIFHWADDR, &ifreq) < 0) {
        perror("ioctl");
        return -1;
    }

    for (int i = 0; i < 6; i++) {
        mac[i] = ifreq.ifr_hwaddr.sa_data[i];
    }
    return 0;
}
