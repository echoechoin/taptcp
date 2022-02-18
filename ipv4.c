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

struct ipv4_hdr_t *get_ipv4_hdr(struct skbuff_t *skb)
{
    return (struct ipv4_hdr_t *)(skb_head(skb) + sizeof(struct eth_hdr_t));
}

void ipv4_packet_debug(struct skbuff_t *skb)
{
    struct ipv4_hdr_t *ipv4_hdr = get_ipv4_hdr(skb);
    char src_ip[16] = {0}, dst_ip[16] = {0};
    inet_ntop(AF_INET, &ipv4_hdr->saddr, src_ip, sizeof(src_ip));
    inet_ntop(AF_INET, &ipv4_hdr->daddr, dst_ip, sizeof(dst_ip));
    char draw_table_cmd[2048] = {0};
    snprintf(draw_table_cmd, 2048 - 1,
        "python3 draw_table.py 12 version ihl tos tot_len id flags frag_offset ttl proto csum saddr daddr "
        "%d %d %d %d %d %d %d %d %d %d %s %s",
        ipv4_hdr->version, 
        ipv4_hdr->ihl, 
        ipv4_hdr->tos, 
        ntohs(ipv4_hdr->len), 
        ipv4_hdr->id,
        ipv4_hdr->flags,
        ntohs(ipv4_hdr->frag_offset),
        ipv4_hdr->ttl,
        ipv4_hdr->proto,
        ntohs(ipv4_hdr->csum),
        src_ip, dst_ip
    );
    system(draw_table_cmd);
}

int ipv4_recv(struct skbuff_t *skb, struct netdev_t *dev)
{
    struct ipv4_hdr_t *ipv4_hdr = (struct ipv4_hdr_t *)get_ipv4_hdr(skb);
    if (ipv4_hdr->version != IPV4) {
        printf(">>> unsupported ipv4 version: %d\n", ipv4_hdr->version);
        goto drop_packet;
    }
    // 不支持可选参数
    if (ipv4_hdr->ihl < 5) {
        printf(">>> IPv4 header length must be at least 5: %d\n", ipv4_hdr->ihl);
        goto drop_packet;
    }

    if (ipv4_hdr->ttl == 0) {
        printf(">>> ttl is 0\n");
        // TODO: send icmp time exceeded
        goto drop_packet;
    }

    if (checksum(ipv4_hdr, ipv4_hdr->ihl * 4) != 0) {
        printf(">>> checksum error\n");
        goto drop_packet;
    }

    if (ipv4_hdr->flags != 0) {
        printf(">>> do not support fragmentation currently\n");
        goto drop_packet;
    }



    switch (ipv4_hdr->proto){
        case ICMPV4_PROTO:
            if (icmpv4_recv(skb, dev) != 0)
                goto drop_packet;
            break;
        case TCP_PROTO:
            // tcp_recv(skb, dev);
            break;
        case UDP_PROTO:
            // udp_recv(skb, dev);
            break;
        default:
            printf(">>> unsupported ipv4 protocol: %d\n", ipv4_hdr->proto);
            goto drop_packet;
    }
    return 0;


drop_packet:
    skb_free(skb);
    return -1;
}

