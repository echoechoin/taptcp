#include <arpa/inet.h>
#include "tuntap.h"
#include "ether.h"
#include "arp.h"
#include "skbuff.h"
#include "ipv4.h"
#include "event.h"
#include "queue.h"
#include "pthread.h"

#define SERVER_IP "192.168.233.2"
#define SERVER_PORT 8888

#define CLIENT_IP "192.168.233.2"
#define CLIENT_PORT 8889

int tapfd;
int listener;
queue_t *listen_queue;
queue_t *tapfd_queue;
struct event *listen_event_wr;

int run_server(int argc, char **argv);
void data_transfer_socket_read(evutil_socket_t sockfd, short event_type, void *arg);
void data_transfer_socket_write(evutil_socket_t sockfd, short event_type, void *arg);