#include "arp.h"
#include "client.h"
#include "event.h"

static struct arp_hdr_t *get_arp_hdr(struct skbuff_t *skb);
static int update_arp_table(struct arp_hdr_t *arp_hdr, struct arp_ipv4_t *arp_ipv4, struct netdev_t *dev);
static int arp_relpy(struct skbuff_t *skb, struct netdev_t *dev);
static struct skbuff_t *arp_alloc_skb();

int show_arp_table(struct netdev_t *dev)
{
    char tmp_mac[20] = {0};
    printf("arp table:\n");
    printf("%-15s%-15s\n", "ip", "mac");
    for (list_t *tmp = dev->arp_table->next; tmp != dev->arp_table; tmp = tmp->next) {
        struct arp_entity_t *arp_entity = (struct arp_entity_t *)tmp->data;
        get_mac_hex_type(arp_entity->mac, tmp_mac);
        char *ip = inet_ntoa(*(struct in_addr *)&arp_entity->ip);
        printf ("%s %s\n", ip, tmp_mac);
    }
    return 0;
}

int arp_recv(struct skbuff_t *skb, struct netdev_t *dev) {
    struct arp_hdr_t *arp_hdr;
    struct arp_ipv4_t *arp_ipv4;
    int merge_flag = 0;

    arp_hdr = get_arp_hdr(skb);
    arp_ipv4 = (struct arp_ipv4_t *) arp_hdr->data;
    
    if (ntohs(arp_hdr->hwtype) != HWTYPE_ETHERNET) {
        printf(">>> arp recv: unsupported hwtype(%x)", ntohs(arp_hdr->hwtype));
        goto drop_packet;
    }
    if (arp_hdr->hwsize != HWSIZE_ETHERNET) {
        printf(">>> arp recv: invalid hwsize %d in Ethernet\n", arp_hdr->hwsize);
        goto drop_packet;
    }
    if (ntohs(arp_hdr->protype) != ETHERTYPE_IPV4) {
        printf(">>> arp recv: unsupported protype(%x)", ntohs(arp_hdr->hwtype));
        goto drop_packet;
    }
    if (arp_hdr->prosize != PROSIZE_IPV4) {
        printf(">>> arp recv: invalid prosize %d in IPV4\n", arp_hdr->hwsize);
        goto drop_packet;
    }
    printf(">>> arp recv:\n");
    ether_packet_debug(skb);
    arp_packet_debug(skb);

    if (dev->addr == arp_ipv4->dip) {
        arp_relpy(skb, dev);
        return 0;
    } 
    update_arp_table(arp_hdr, arp_ipv4, dev);
    show_arp_table(dev);
    return 0;

drop_packet:
    skb_free(skb);
    return -1;
}

int arp_request(u_int32_t sip, u_int32_t dip, struct netdev_t *dev) {
    struct skbuff_t *skb;
    struct arp_hdr_t *arp_hdr;
    struct arp_ipv4_t *arp_ipv4;
    skb = arp_alloc_skb();
    if (skb == NULL) {
        return -1;
    }


}

static struct skbuff_t *arp_alloc_skb() {
    struct skbuff_t *skb;
    u_int32_t arp_skb_len = sizeof(struct arp_hdr_t) + sizeof(struct arp_ipv4_t) + sizeof(struct eth_hdr_t);
    skb = skb_alloc(arp_skb_len);
    skb_reserve(skb, arp_skb_len);
    skb->protocol = htons(ETHERTYPE_ARP);
}

static int update_arp_table(struct arp_hdr_t *arp_hdr, struct arp_ipv4_t *arp_ipv4, struct netdev_t *dev) {
    printf("update_arp_table...\n");
    for (list_t *tmp = dev->arp_table->next; tmp != dev->arp_table; tmp = tmp->next) {
        struct arp_entity_t *arp_entity = (struct arp_entity_t *)tmp->data;
        if (arp_entity->ip == arp_ipv4->sip) {
            for (int i = 0; i < 6; i++) {
                arp_entity->mac[i] = arp_ipv4->smac[i];
            }
            return 0;
        }
    }
    struct arp_entity_t *arp_entity = (struct arp_entity_t *)malloc(sizeof(struct arp_entity_t));
    if (arp_entity == NULL) {
        return -1;
    }
    for (int i = 0; i < 6; i++) {
        arp_entity->mac[i] = arp_ipv4->smac[i];
    }
    arp_entity->ip = arp_ipv4->sip;
    list_add(dev->arp_table, arp_entity);
    return 0;
}

void arp_packet_debug(struct skbuff_t *skb) 
{
    struct arp_hdr_t *arp_hdr;
    struct arp_ipv4_t *arp_ipv4;
    arp_hdr = get_arp_hdr(skb);
    arp_ipv4 = (struct arp_ipv4_t *) arp_hdr->data;
    char tmpip[64];
    char draw_table_cmd[2048] = {0};
    snprintf(draw_table_cmd, 2048 - 1, 
        "python3 draw_table.py 9 hwtype hwsize protype prosize opcode smac sip dmac dip "
        "%s %d %s %d %s %02x:%02x:%02x:%02x:%02x:%02x %s %02x:%02x:%02x:%02x:%02x:%02x %s",
        ntohs(arp_hdr->hwtype) == HWTYPE_ETHERNET ? "Ethernet" : "unknown",
        arp_hdr->hwsize,
        ntohs(arp_hdr->protype) == ETHERTYPE_IPV4 ? "IPV4" : "unknown",
        arp_hdr->prosize,
        ntohs(arp_hdr->opcode) == ARP_OPCODE_REQUEST ? "ARP_REQUEST" : "ARP_REPLY",
        arp_ipv4->smac[0], arp_ipv4->smac[1], arp_ipv4->smac[2], arp_ipv4->smac[3], arp_ipv4->smac[4], arp_ipv4->smac[5],
        inet_ntop(AF_INET, &arp_ipv4->sip, tmpip, 64),
        arp_ipv4->dmac[0], arp_ipv4->dmac[1], arp_ipv4->dmac[2], arp_ipv4->dmac[3], arp_ipv4->dmac[4], arp_ipv4->dmac[5],
        inet_ntop(AF_INET, &arp_ipv4->dip, tmpip, 64)
    );
    system(draw_table_cmd);
}
    


static int arp_relpy(struct skbuff_t *skb, struct netdev_t *dev) {
    struct arp_hdr_t *arp_hdr;
    struct arp_ipv4_t *arp_ipv4;
    arp_hdr = (struct arp_hdr_t *)get_arp_hdr(skb);

    skb_reserve(skb, sizeof(struct arp_hdr_t) + sizeof(struct arp_ipv4_t) + sizeof(struct eth_hdr_t));
    skb_push(skb, sizeof(struct arp_ipv4_t));
    arp_ipv4 = (struct arp_ipv4_t *)skb->data;

    memcpy(arp_ipv4->dmac, arp_ipv4->smac, 6);
    memcpy(arp_ipv4->smac, dev->hwaddr, 6);

    arp_ipv4->dip = arp_ipv4->sip;
    arp_ipv4->sip = dev->addr;

    arp_hdr->hwtype = htons(HWTYPE_ETHERNET);
    arp_hdr->protype = htons(ETHERTYPE_IPV4);
    arp_hdr->hwsize = HWSIZE_ETHERNET;
    arp_hdr->prosize = PROSIZE_IPV4;
    arp_hdr->opcode = htons(ARP_OPCODE_REPLY);

    skb_push(skb, sizeof(struct eth_hdr_t) + sizeof(struct arp_hdr_t));

    struct eth_hdr_t *eth_hdr = (struct eth_hdr_t *)skb->data;
    memcpy(eth_hdr->dmac, arp_ipv4->dmac, 6);
    memcpy(eth_hdr->smac, dev->hwaddr, 6);
    eth_hdr->ethertype = htons(ETHERTYPE_ARP);

    printf(">>> arp reply:\n");
    ether_packet_debug(skb);
    arp_packet_debug(skb);

    queue_push(listen_queue, skb);
    event_add(listen_event_wr, NULL);
    return 0;
}

static int is_target_ip(struct arp_ipv4_t *arp_ipv4, struct netdev_t *dev) {
    return !(arp_ipv4->dip == dev->addr);
}

static struct arp_hdr_t *get_arp_hdr(struct skbuff_t *skb) {
    return (struct arp_hdr_t *)(skb_head(skb) + sizeof(struct eth_hdr_t));
}