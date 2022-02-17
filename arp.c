#include "arp.h"

int arp_init(u_int8_t *mac, u_int32_t ip) {
    arp_table = list_init();
    if (arp_table == NULL) {
        return -1;
    }
    for(int i = 0; i < 6; i++) {
        mac_address[i] = mac[i];
    }
    ip_address = ip;
    return 0;
}

int is_target(struct arp_ipv4_t *arp_ipv4) {
    return !(arp_ipv4->dip == ip_address);
}

int show_arp_table()
{
    char tmp_mac[20] = {0};
    printf("arp table:\n");
    printf("%-15s%-15s\n", "ip", "mac");
    for (list_t *tmp = arp_table->next; tmp != arp_table; tmp = tmp->next) {
        struct arp_entity_t *arp_entity = (struct arp_entity_t *)tmp->data;
        get_mac_hex_type(arp_entity->mac, tmp_mac);
        char *ip = inet_ntoa(*(struct in_addr *)&arp_entity->ip);
        printf ("%s %s\n", ip, tmp_mac);
    }
    return 0;
}

int update_arp_table(struct arp_hdr_t *arp_hdr, struct arp_ipv4_t *arp_ipv4) {
    printf("update_arp_table...\n");
    for (list_t *tmp = arp_table->next; tmp != arp_table; tmp = tmp->next) {
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
    list_add(arp_table, arp_entity);
    return 0;
}

int arp_response(int fd, struct eth_hdr_t *eth_hdr) {
    printf("arp response...\n");
    struct arp_hdr_t *arp_hdr = (struct arp_hdr_t *)(eth_hdr->payload);
    struct arp_ipv4_t *arp_ipv4 = (struct arp_ipv4_t *)(arp_hdr->data);
    u_int8_t target_mac[6] = {0};
    for(int i = 0; i < 6; i++) {
        target_mac[i] = arp_ipv4->smac[i];
    }
    arp_hdr->opcode = htons(ARP_OPCODE_REPLY);
    for(int i = 0; i < 6; i++) {
        arp_ipv4->dmac[i] = target_mac[i];
        arp_ipv4->smac[i] = mac_address[i];
    }
    arp_ipv4->dip = arp_ipv4->sip;
    arp_ipv4->sip = ip_address;
    write(fd, eth_hdr, sizeof(struct eth_hdr_t) + sizeof(struct arp_hdr_t) + sizeof(struct arp_ipv4_t));
    return 0;
}

int arp_process(int fd, struct eth_hdr_t *eth_hdr) {
    struct arp_hdr_t *arp_hdr = (struct arp_hdr_t *)(eth_hdr->payload);
    struct arp_ipv4_t *arp_ipv4 = (struct arp_ipv4_t *)arp_hdr->data;
    int merge_flag = 0;
    printf("arp process...\n");
    if (ntohs(arp_hdr->hwtype) != HWTYPE_ETHERNET) {
        printf("unsupported hwtype: %x", ntohs(arp_hdr->hwtype));
        return -1;
    }
    if (ntohs(arp_hdr->hwsize != HWSIZE_ETHERNET)) {
        printf("arp request: invalid hwsize: %d in Ethernet\n", arp_hdr->hwsize);
        return -1;
    }
    if (ntohs(arp_hdr->protype) != ETHERTYPE_IPV4) {
        printf("unsupported protype: %x", ntohs(arp_hdr->hwtype));
        return -1;
    }
    if (arp_hdr->prosize != PROSIZE_IPV4) {
        printf("arp request: invalid prosize: %d in IPV4\n", arp_hdr->hwsize);
        return -1;
    }
    if (is_target(arp_ipv4) == 0) {
        arp_response(fd, eth_hdr);
        return 0;
    } 
    update_arp_table(arp_hdr, arp_ipv4);
    show_arp_table();
    return 0;
}