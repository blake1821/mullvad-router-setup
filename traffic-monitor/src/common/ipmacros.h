#include "macro-utils.h"

// info for automatic type generation
#define IP_VERSIONS 4, 6
#define IP_READ_MESSAGE_TYPES Query, Connect
#define IP_WRITE_MESSAGE_TYPES SetStatus

// hardcoded ipv4 defs
#define IPV4_ADDR_T struct in_addr
#define IPV4_HASH(src, dst) ((dst.s_addr ^ (dst.s_addr >> 16) ^ src.s_addr) & H_MASK)
#define IPV4_H2(src, dst) (dst.s_addr & H2_MASK)
#define IPV4_EQ(a, b) (a.s_addr == b.s_addr)

// hardcoded ipv6 defs
#define IPV6_HASH(src, dst) ipv6_hash(src) ^ ipv6_hash(dst)
#define IPV6_ADDR_T struct in6_addr
#define IPV6_H2(src, dst) (dst.s6_addr[0] & H2_MASK)
#define IPV6_SIZE sizeof((struct in6_addr){0}.s6_addr)
#define IPV6_EQ(a, b) (memcmp(a.s6_addr, b.s6_addr, IPV6_SIZE) == 0)

// lookup ip-version-specific definitions
#define __IP_ADDR_T(x) IPV##x##_ADDR_T
#define __IP_HASH(x) IPV##x##_HASH
#define __IP_H2(x) IPV##x##_H2
#define __IP_EQ(x) IPV##x##_EQ

#define __GET_IPSTATUS(x) ipv##x##_get_status
#define __DISPATCH_QUEUE(x) ipv##x##_dispatch_queue
#define __ENQUEUE_PACKET(x) ipv##x##_enqueue_packet
#define __GET_ENQUEUED_COUNT(x) ipv##x##_get_enqueued_count
#define __IP_RESET(x) ipv##x##_reset

#define __ON_SETSTATUS(x) on_SetStatus##x
#define __ENQUEUE_QUERY(x) enqueue_Query##x

// definitions specific to the ip version:
#ifdef IP_VERSION

// ip utils
#define IP_ADDR_T VCALL(__IP_ADDR_T, IP_VERSION)
#define IP_HASH(src, dst) VCALL(__IP_HASH, IP_VERSION)(src, dst)
#define IP_H2(src, dst) VCALL(__IP_H2, IP_VERSION)(src, dst)
#define IP_EQ(a, b) VCALL(__IP_EQ, IP_VERSION)(a, b)

// defined in iplookup
#define GET_IPSTATUS VCALL(__GET_IPSTATUS, IP_VERSION)
#define DISPATCH_QUEUE VCALL(__DISPATCH_QUEUE, IP_VERSION)
#define ENQUEUE_PACKET VCALL(__ENQUEUE_PACKET, IP_VERSION)
#define GET_ENQUEUED_COUNT VCALL(__GET_ENQUEUED_COUNT, IP_VERSION)
#define IP_RESET VCALL(__IP_RESET, IP_VERSION)

// refs to automatically defined types & functions
#define SETSTATUS_PAYLOAD_T VCAT(VCAT(struct SetStatus, IP_VERSION), Payload)
#define QUERY_PAYLOAD_T VCAT(VCAT(struct Query, IP_VERSION), Payload)
#define ON_SETSTATUS VCAT(on_SetStatus, IP_VERSION)
#define ENQUEUE_QUERY VCAT(enqueue_Query, IP_VERSION)

#endif

////////////////////////////////////////
//          AUTO-GENERATED TYPES
////////////////////////////////////////

// message types
#define MAKE_ENTRY(v, t) ENTRY(t##v)

#define IP_READ_MESSAGES PAIRAPPLY(MAKE_ENTRY, (IP_READ_MESSAGE_TYPES), IP_VERSIONS)
#define IP_WRITE_MESSAGES PAIRAPPLY(MAKE_ENTRY, (IP_WRITE_MESSAGE_TYPES), IP_VERSIONS)

// message structs
#define DECLARE_CONNECT_STRUCT(v) \
    struct Connect##v##Payload    \
    {                             \
        uint16_t dst_port;        \
        ConnectProtocol protocol; \
        __IP_ADDR_T(v)            \
        src;                      \
        __IP_ADDR_T(v)            \
        dst;                      \
    };

#define DECLARE_QUERY_STRUCT(v) \
    struct Query##v##Payload    \
    {                           \
        __IP_ADDR_T(v)          \
        src;                    \
        __IP_ADDR_T(v)          \
        dst;                    \
    };

#define DECLARE_SETSTATUS_STRUCT(v) \
    struct SetStatus##v##Payload    \
    {                               \
        __IP_ADDR_T(v)              \
        src;                        \
        __IP_ADDR_T(v)              \
        dst;                        \
        IPStatus status;            \
    };
