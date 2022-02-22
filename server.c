#include "server.h"

char dev[IFNAMSIZ];
struct netdev_t *netdev;
void *process_eth_packet(void *data);

void data_transfer_socket_read(evutil_socket_t sockfd, short event_type, void *arg)
{
    struct event_base *base = (struct event_base *)arg;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    char buf[BUFSIZE];
    int len = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&addr, &addr_len);
    if (len > 0) {
        struct skbuff_t *skb = skb_alloc(len);
        memcpy(skb->data, buf, len);
        queue_push(tapfd_queue, skb);
    }
} 

void data_transfer_socket_write(evutil_socket_t sockfd, short event_type, void *arg)
{
    struct skbuff_t *skb = queue_pop(listen_queue);
    if (skb == NULL)
        return;
    struct sockaddr_in addr;
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(CLIENT_PORT);
    addr.sin_addr.s_addr = inet_addr(CLIENT_IP);
    int len = sendto(sockfd, skb->data, skb->len, 0, (struct sockaddr *)&addr, sizeof(addr));
    skb_free(skb);
    if (queue_size(listen_queue) > 0) event_add(listen_event_wr, NULL);
} 

int run_server(int argc, char **argv)
{
    strncpy(dev, "tap_server", IFNAMSIZ);
    tapfd = tun_alloc(dev, IFF_TAP | IFF_NO_PI);
    system("sudo bash ./uptap.sh tap_server");

    listener = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }

    listen_queue = queue_new();
    tapfd_queue = queue_new();

    struct event_base *evbase = event_base_new();
    struct event *listen_event_rd = event_new(evbase, listener, EV_READ|EV_PERSIST, data_transfer_socket_read, (void*)evbase);
    listen_event_wr = event_new(evbase, listener, EV_WRITE, data_transfer_socket_write, evbase);

    event_add(listen_event_rd, NULL);
    
    pthread_attr_t attr_detach;
    pthread_attr_init(&attr_detach);
    pthread_attr_setdetachstate(&attr_detach, PTHREAD_CREATE_DETACHED);

    pthread_t process_eth_packet_thread;
    pthread_create(&process_eth_packet_thread, &attr_detach, process_eth_packet, NULL);
    printf("create: process_eth_packet thread: %lu\n", process_eth_packet_thread);

    event_base_dispatch(evbase);
    return 0;
}

void *process_eth_packet(void *data) {
    char *buf = malloc(BUFSIZE);
    char tmp_mac_addr[20] = {0};
    char tmp_ethertype[20] = {0};
    u_int8_t mac[6] = {0};

    get_mac_address(dev, mac);
    get_mac_hex_type(mac, tmp_mac_addr);
    printf("get mac address: %s(%s)\n", dev, tmp_mac_addr);
    
    printf("start process packet... \n");
    netdev = netdev_alloc("10.0.0.1", tmp_mac_addr, 1500);
    
    while(1) {
        if (queue_size(tapfd_queue) == 0) {
            continue;
        }
        
        struct skbuff_t *skb = queue_pop(tapfd_queue);
        
        struct eth_hdr_t *hdr = get_eth_hdr(skb);
        // arp报文处理
        if (ntohs(hdr->ethertype) == ETHERTYPE_ARP) {
            arp_recv(skb, netdev);
        }

        // IP报文处理
        if (ntohs(hdr->ethertype) == ETHERTYPE_IPV4) {
            ipv4_recv(skb, netdev);
        }
    }
}
