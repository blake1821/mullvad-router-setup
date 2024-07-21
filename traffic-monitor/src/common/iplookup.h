#include "protocol.h"

// Defined in iplookup.c:

void init_iplookup(void);
void on_SetStatus4(struct SetStatus4Payload *payload, int n);
// TODO: void set_ipv6_status(struct SetStatus4Payload payload);
IPStatus get_ipv4_status(struct in_addr src, struct in_addr dst, uint16_t *queue_no);
bool enqueue_ipv4(struct list_head *head, uint16_t queue_no, struct in_addr src, struct in_addr dst);
int get_enqueued_ipv4(void);
// TODO: IPStatus get_ipv6_status(struct in6_addr ipv6);

// Defined in readqueue.c:
void enqueue_Query4(struct Query4Payload *payload);
// TODO: ipv6

// defined in nethooks.c
// note: these are both protected by the same lock
void dispatch_queue4(struct list_head *head, IPStatus status__possibly_pending);
