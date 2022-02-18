#include "client.h"
#include "ether.h"
#include "arp.h"

void data_transfer_socket_read(evutil_socket_t sockfd, short event_type, void *arg)
{
    struct event_base *base = (struct event_base *)arg;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    char buf[BUFSIZE];
    int len = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&addr, &addr_len);
    if (len > 0) {
        printf(">>> read from sock fd: %d\n", len);
        struct skbuff_t *skb = skb_alloc(len);
        memcpy(skb->data, buf, len);
        skb->len = len;
        queue_push(tapfd_queue, skb);
        event_add(tapfd_event_wr, NULL);
    }
} 

void data_transfer_socket_write(evutil_socket_t sockfd, short event_type, void *arg)
{
    struct skbuff_t *skb = queue_pop(listen_queue);
    if (skb == NULL)
        return;
    struct sockaddr_in addr;
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    sendto(sockfd, skb->data, skb->len, 0, (struct sockaddr *)&addr, sizeof(addr));
    skb_free(skb);
    if (queue_size(listen_queue) > 0) event_add(listen_event_wr, NULL);
} 

void data_transfer_tapfd_read(evutil_socket_t tapfd, short event_type, void *arg)
{
    struct event_base *base = (struct event_base *)arg;
    char buffer[BUFSIZE] = {0};
    int len = read(tapfd, buffer, BUFSIZE);
    if (len > 0) {
        struct skbuff_t *skb = skb_alloc(len);
        memcpy(skb->data, buffer, len);
        skb->len = len;
        queue_push(listen_queue, skb);
        event_add(listen_event_wr, NULL);
    }
}

void data_transfer_tapfd_write(evutil_socket_t tapfd, short event_type, void *arg)
{
    struct event_base *base = (struct event_base *)arg;
    struct skbuff_t *skb = queue_pop(tapfd_queue);
    if (skb == NULL)
        return;
    int size = write(tapfd, skb->data, skb->len);
    printf(">>> write to tap fd: %d\n", size);
    skb_free(skb);
    if (queue_size(tapfd_queue) > 0) event_add(tapfd_event_wr, NULL);
}

int run_client(int argc, char **argv)
{
    char dev[IFNAMSIZ] = "tap_client";
    char *buf = malloc(BUFSIZE);

    tapfd = tun_alloc(dev, IFF_TAP | IFF_NO_PI);
    system("sudo bash ./uptap.sh tap_client 10.0.0.2/24");

    listener = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(CLIENT_PORT);
    addr.sin_addr.s_addr = inet_addr(CLIENT_IP);
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }

    listen_queue = queue_new();
    tapfd_queue = queue_new();

    struct event_base *evbase = event_base_new();
    struct event *listen_event_rd = event_new(evbase, listener, EV_READ|EV_PERSIST, data_transfer_socket_read, (void*)evbase);
    struct event *tapfd_event_rd = event_new(evbase, tapfd, EV_READ|EV_PERSIST, data_transfer_tapfd_read, (void*)evbase);
    tapfd_event_wr = event_new(evbase, tapfd, EV_WRITE, data_transfer_tapfd_write, evbase);
    listen_event_wr = event_new(evbase, listener, EV_WRITE, data_transfer_socket_write, evbase);

    event_add(listen_event_rd, NULL);
    event_add(tapfd_event_rd, NULL);

    event_base_dispatch(evbase); 
}