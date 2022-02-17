#include "ipv4.h"

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

int ipv4_process(int fd, struct eth_hdr_t *eth_hdr)
{
    struct ipv4_hdr_t *ipv4_hdr = (struct ipv4_hdr_t *) (eth_hdr->payload);
    if (ipv4_hdr->version != 4) {
        printf("unsupported ipv4 version: %d\n", ipv4_hdr->version);
        return -1;
    }
    // 不支持可选参数
    if (ipv4_hdr->ihl != 5) {
        printf("unsupported ipv4 ihl: %d\n", ipv4_hdr->ihl);
        return -1;
    }



}

