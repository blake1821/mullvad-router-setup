#include "protocol.h"

// Defined in iplookup.c:
void init_iplookup(void);
void on_Reset(struct ResetPayload *payload, int n);

// Defined in each iplookup.<v>.c file:
#define DECLARE_IP_LOOKUP(v)                                                                                      \
    /* defined in iplookup. */                                                                                    \
    IPStatus __GET_IPSTATUS(v)(__IP_ADDR_T(v) src, __IP_ADDR_T(v) dst, uint16_t * queue_no);                      \
    bool __ENQUEUE_PACKET(v)(struct list_head * head, uint16_t queue_no, __IP_ADDR_T(v) src, __IP_ADDR_T(v) dst); \
    void __ON_SETSTATUS(v)(struct SetStatus##v##Payload * payload, int n);                                        \
    int __GET_ENQUEUED_COUNT(v)(void);                                                                            \
    void __IP_RESET(v)(void);                                                                                     \
    /* defined in nethooks.c */                                                                                   \
    void __DISPATCH_QUEUE(v)(struct list_head * head, IPStatus status__possibly_pending);                         \
    /* defined in readqueue.c */                                                                                  \
    void __ENQUEUE_QUERY(v)(struct Query##v##Payload * payload);

APPLY(DECLARE_IP_LOOKUP, IP_VERSIONS)