#include "tuntap.h"
#include "skbuff.h"
#include <event.h>
#include "queue.h"

#define SERVER_IP "192.168.233.2"
#define SERVER_PORT 8888

#define CLIENT_IP "192.168.233.2"
#define CLIENT_PORT 8889

int tapfd;
int listener;
queue_t *listen_queue;
queue_t *tapfd_queue;

struct event_base *evbase;
struct event *tapfd_event_wr;
struct event *listen_event_wr;

int run_client(int argc, char **argv);
void data_transfer_socket_read(evutil_socket_t sockfd, short event_type, void *arg);
void data_transfer_socket_write(evutil_socket_t sockfd, short event_type, void *arg);
void data_transfer_tapfd_read(evutil_socket_t tapfd, short event_type, void *arg);
void data_transfer_tapfd_write(evutil_socket_t tapfd, short event_type, void *arg);