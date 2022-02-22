#include "icmpv4.h"

void icmpv4_packet_debug(struct skbuff_t *skb)
{
    struct eth_hdr_t *eth_hdr = get_eth_hdr(skb);
    struct ipv4_hdr_t *ipv4_hdr = get_ipv4_hdr(skb);
    struct icmp_v4_t *icmp_v4 = (struct icmp_v4_t *)(((void *)ipv4_hdr) + ipv4_hdr->ihl * 4);
    char draw_table_cmd[2048] = {0};
    if (icmp_v4->type == ICMPV4_TYPE_ECHO_REQUEST || icmp_v4->type == ICMPV4_TYPE_ECHO_REPLY) {
        struct icmp_v4_echo_reply_t *tmp = (struct icmp_v4_echo_reply_t *)icmp_v4->data;
        snprintf(draw_table_cmd, 2048 - 1,
            "python3 draw_table.py 6 type code checksum id seq data %d %d %d %d %d %s",
            icmp_v4->type,
            icmp_v4->code,
            ntohs(icmp_v4->csum),
            ntohs(tmp->id),
            ntohs(tmp->seq),
            "'...'"
        );
    } else if (icmp_v4->type == ICMPV4_TYPE_DEST_UNREACH) {
        struct icmp_v4_dst_unreachable *tmp = (struct icmp_v4_dst_unreachable *)icmp_v4->data;
        snprintf(draw_table_cmd, 2048 - 1,
            "python3 draw_table.py 6 type code checksum unused len var data %d %d %d %d %d %d %s",
            icmp_v4->type,
            icmp_v4->code,
            ntohs(icmp_v4->csum),
            tmp->unused,
            tmp->len,
            ntohs(tmp->var),
            "'...'"
        );
    } else {
        snprintf(draw_table_cmd, 2048 - 1,
            "python3 draw_table.py 5 type code checksum '...' %d %d %d %s",
            icmp_v4->type,
            icmp_v4->code,
            ntohs(icmp_v4->csum),
            "'...'"
        );
    }
    system(draw_table_cmd);
}

int icmpv4_echo_reply(struct skbuff_t *skb, struct netdev_t *dev) {
    struct eth_hdr_t *eth_hdr = get_eth_hdr(skb);
    struct ipv4_hdr_t *ipv4_hdr = get_ipv4_hdr(skb);
    struct icmp_v4_t *icmp_v4 = (struct icmp_v4_t *)(((void *)ipv4_hdr) + ipv4_hdr->ihl * 4);
    struct icmp_v4_echo_reply_t *echo_reply = (struct icmp_v4_echo_reply_t *)icmp_v4->data;

    icmp_v4->type = ICMPV4_TYPE_ECHO_REPLY;
    icmp_v4->code = 0;
    icmp_v4->csum = 0;

    echo_reply->id = echo_reply->id;
    echo_reply->seq = echo_reply->seq;
    
    icmp_v4->csum = checksum(icmp_v4, ntohs(ipv4_hdr->len) - ipv4_hdr->ihl * 4);
    printf(">>> reply icmpv4 echo request\n");
    icmpv4_packet_debug(skb);

    skb->len = ntohs(ipv4_hdr->len) + sizeof(struct eth_hdr_t);
    return ipv4_send(skb, dev);
}

int icmpv4_recv(struct skbuff_t *skb, struct netdev_t *dev)
{
    struct ipv4_hdr_t *ipv4_hdr = (struct ipv4_hdr_t *)get_ipv4_hdr(skb);
    struct icmp_v4_t *icmp_v4 = (struct icmp_v4_t *)ipv4_hdr->data;

    printf(">>> icmpv4_recv:\n");
    ether_packet_debug(skb);
    ipv4_packet_debug(skb);
    icmpv4_packet_debug(skb);

    switch (icmp_v4->type) {
    case ICMPV4_TYPE_ECHO_REQUEST:
        if (icmpv4_echo_reply(skb, dev) != 0) {
            printf("icmpv4_recv: echo reply error\n");
            return -1;
        }
        return 0;
    case ICMPV4_TYPE_DEST_UNREACH:
        printf(">>> ICMPv4: destination unreachable\n");
        break;
    default:
        printf("ICMPv4 do not support other type currently\n");
        break;
    }
    return -1;
}
