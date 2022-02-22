#include "tcp.h"

static struct tcp_hdr_t *get_tcp_hdr(struct skbuff_t *skb)
{
    struct ipv4_hdr_t *ipv4_hdr = get_ipv4_hdr(skb);
    return (struct tcp_hdr_t *)((u_int8_t *)ipv4_hdr + ipv4_hdr->ihl * 4);
}

int tcp_recv(struct skbuff_t *skb, struct netdev_t *dev)
{
    struct ipv4_hdr_t *ipv4_hdr = (struct ipv4_hdr_t *)get_ipv4_hdr(skb);
    struct tcp_hdr_t *tcp_hdr = (struct tcp_hdr_t *)get_tcp_hdr(skb);
    printf(">>> tcp_recv\n");
    ether_packet_debug(skb);
    ipv4_packet_debug(skb);
    tcp_packet_debug(skb);
    if (tcp_checksum(ipv4_hdr, tcp_hdr) != 0) {
        printf(">>> TCP segment checksum did not match, dropping\n");
        return 1;
    }

    if (tcp_hdr->flags & TCP_FLAG_SYN) {
        tcp_hdr->flags |= TCP_FLAG_ACK;
        tcp_hdr->ack_seq = htonl(ntohl(tcp_hdr->seq) + 1);
        tcp_hdr->seq = htonl(12345678);
        memcpy(tcp_hdr->data+12, tcp_hdr->data+8, 4);
        return tcp_send(skb, dev);
    }

    free(skb);
    return 1;
    
}

int tcp_checksum(struct ipv4_hdr_t *ipv4_hdr, struct tcp_hdr_t *tcp_hdr)
{
    u_int16_t ip_len = ntohs(ipv4_hdr->len);
    u_int16_t iphdr_len = ipv4_hdr->ihl * 4;
    u_int16_t tcp_len = ip_len - iphdr_len;
    struct ip_pseudo_hdr_t *ip_pseudo_hdr = (struct ip_pseudo_hdr_t *)malloc(sizeof(struct ip_pseudo_hdr_t) + tcp_len);

    ip_pseudo_hdr->saddr = ipv4_hdr->saddr;
    ip_pseudo_hdr->daddr = ipv4_hdr->daddr;
    ip_pseudo_hdr->zero = 0;
    ip_pseudo_hdr->proto = TCP_PROTO;
    ip_pseudo_hdr->len = htons(tcp_len);

    memcpy(ip_pseudo_hdr->data, tcp_hdr, tcp_len);

    tcp_hdr->csum = checksum(ip_pseudo_hdr, sizeof(struct ip_pseudo_hdr_t) + tcp_len);
    free(ip_pseudo_hdr);
    return tcp_hdr->csum;
}

int tcp_send(struct skbuff_t *skb, struct netdev_t *dev)
{

    struct ipv4_hdr_t *ipv4_hdr = (struct ipv4_hdr_t *)get_ipv4_hdr(skb);
    struct tcp_hdr_t *tcp_hdr = (struct tcp_hdr_t *) ipv4_hdr->data;
    u_int16_t ip_len = ntohs(ipv4_hdr->len);
    u_int16_t iphdr_len = ipv4_hdr->ihl * 4;
    u_int16_t tcp_len = ip_len - iphdr_len;
    struct ip_pseudo_hdr_t *ip_pseudo_hdr = (struct ip_pseudo_hdr_t *)malloc(sizeof(struct ip_pseudo_hdr_t) + tcp_len);

    uint16_t tmpport = tcp_hdr->sport;
    tcp_hdr->sport = tcp_hdr->dport;
    tcp_hdr->dport = tmpport;

    ip_pseudo_hdr->saddr = ipv4_hdr->daddr;
    ip_pseudo_hdr->daddr = ipv4_hdr->saddr;
    ip_pseudo_hdr->zero = 0;
    ip_pseudo_hdr->proto = TCP_PROTO;
    ip_pseudo_hdr->len = htons(tcp_len);

    
    tcp_hdr->csum = 0;

    memcpy(ip_pseudo_hdr->data, tcp_hdr, tcp_len);
    tcp_hdr->csum = checksum(ip_pseudo_hdr, sizeof(struct ip_pseudo_hdr_t) + tcp_len);
    tcp_hdr->csum = tcp_hdr->csum;
    free(ip_pseudo_hdr);
    
    printf(">>> tcp_send\n");
    tcp_packet_debug(skb);

    skb->len = sizeof(struct eth_hdr_t) + ip_len;
    return ipv4_send(skb, dev);

}

char *get_tcp_flags(u_int8_t flag, char *s, int len)
{
    char *tmp = s;
    if (flag & TCP_FLAG_FIN) {
        tmp += sprintf(tmp, "F");
    }
    if (flag & TCP_FLAG_SYN) {
        tmp += sprintf(tmp, "S");
    }
    if (flag & TCP_FLAG_RST) {
        tmp += sprintf(tmp, "R");
    }
    if (flag & TCP_FLAG_PSH) {
        tmp += sprintf(tmp, "P");
    }
    if (flag & TCP_FLAG_ACK) {
        tmp += sprintf(tmp, "A");
    }
    if (flag & TCP_FLAG_URG) {
        tmp += sprintf(tmp, "U");
    }
    if (flag & TCP_FLAG_ECE) {
        tmp += sprintf(tmp, "E");
    }
    if (flag & TCP_FLAG_CWR) {
        tmp += sprintf(tmp, "C");
    }
        
    return s;
}

void tcp_packet_debug(struct skbuff_t *skb)
{
    struct ipv4_hdr_t *ipv4_hdr = (struct ipv4_hdr_t *)get_ipv4_hdr(skb);
    struct tcp_hdr_t *tcp_hdr = (struct tcp_hdr_t *)get_tcp_hdr(skb);

    char tcp_packet_debug_cmd[2048] = {0};
    char tcp_flags[32] = {0};
    snprintf(tcp_packet_debug_cmd, 2048 - 1,
        "python3 draw_table.py 10 sport dport seq ack rsvd flags window_size csum urg_ptr data"
        " %u %u %u %u %u %s %u %u %u %s",
        ntohs(tcp_hdr->sport),
        ntohs(tcp_hdr->dport),
        ntohl(tcp_hdr->seq),
        ntohl(tcp_hdr->ack_seq),
        tcp_hdr->rsvd,
        get_tcp_flags(tcp_hdr->flags, tcp_flags, 32),
        tcp_hdr->win,
        ntohs(tcp_hdr->csum),
        ntohs(tcp_hdr->urp),
        "'...'"
    );
    system(tcp_packet_debug_cmd);
}