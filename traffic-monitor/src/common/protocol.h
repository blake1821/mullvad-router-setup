#pragma once

#include "global-vars.h"

#ifdef __KERNEL__
#include <net/inet_common.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
struct list_head
{
    struct list_head *next;
    void *data;
};
#endif
#include "ipmacros.h"

#define TRAFFICMON_PROC_FILE "trafficmon"
#define MAX_MESSAGE_SIZE 2000

// #define TEST_NETHOOKS

/*
Messaging protocol:
1. Open the file corresponding to the message type, for example, /proc/trafficmon/Connect4
2. Read/write using a buffer of size MAX_MESSAGE_SIZE
3. Read/write n payloads, and expect the size parameter or return value to be n
*/

#define DEFAULT_READ_MESSAGES \
    IP_READ_MESSAGES

#define DEFAULT_WRITE_MESSAGES \
    IP_WRITE_MESSAGES          \
    ENTRY(SetNfEnabled)        \
    ENTRY(Reset)

#ifdef TEST_NETHOOKS

// todo: fix DECLARATION(...)
#define DECLARATION_TF(name) DECLARATION(int, CONCAT(name, _queue_size), 0)
#define _IP_DEBUG_PARAMS(v) DECLARATION(int, ipv##v##_enqueued, 0)
#define DEBUG_PARAMS                       \
    DECLARATION(int, verdict_responses, 0) \
    DECLARATION(int, enqueue_failures, 0)  \
    DECLARATION(int, overflow_packets, 0)  \
    APPLY(_IP_DEBUG_PARAMS, IP_VERSIONS)   \
    DEFAULT_READ_MESSAGES

#define _VERDICT_MESSAGE(v) ENTRY(TestVerdict##v)
#define _TEST_PACKET_MESSAGE(v) ENTRY(TestPacket##v)
#define READ_MESSAGES                    \
    DEFAULT_READ_MESSAGES                \
    APPLY(_VERDICT_MESSAGE, IP_VERSIONS) \
    ENTRY(DebugResponse)
#define WRITE_MESSAGES                       \
    DEFAULT_WRITE_MESSAGES                   \
    APPLY(_TEST_PACKET_MESSAGE, IP_VERSIONS) \
    ENTRY(DebugRequest)

#else
#define READ_MESSAGES DEFAULT_READ_MESSAGES
#define WRITE_MESSAGES DEFAULT_WRITE_MESSAGES
#endif

typedef enum
{
    Allowed,
    Blocked,
    Pending
} IPStatus;

#define PROTOCOL_LIST TCP, UDP, Other
#define _PROTO_ENUM(name) Proto##name,
typedef enum
{
    APPLY(_PROTO_ENUM, PROTOCOL_LIST)
} ConnectProtocol;
#undef _PROTO_ENUM

// read() messages
APPLY(DECLARE_CONNECT_STRUCT, IP_VERSIONS)
APPLY(DECLARE_QUERY_STRUCT, IP_VERSIONS)

#ifdef TEST_NETHOOKS
#define _TEST_VERDICT_STRUCT(v)          \
    struct TestVerdict##v##Payload       \
    {                                    \
        struct Connect##v##Payload conn; \
        bool allowed;                    \
    };
APPLY(_TEST_VERDICT_STRUCT, IP_VERSIONS)
#undef _TEST_VERDICT_STRUCT

struct DebugResponsePayload
{
#define DECLARATION(type, name, value) type name;
#define ENTRY(name) DECLARATION_TF(name)
    DEBUG_PARAMS
#undef ENTRY
#undef DECLARATION
};

#endif

// write() messages
APPLY(DECLARE_SETSTATUS_STRUCT, IP_VERSIONS)

struct SetNfEnabledPayload
{
    bool enabled;
};

struct ResetPayload
{
    bool reset;
};

#ifdef TEST_NETHOOKS
#define _TEST_PACKET_STRUCT(v)           \
    struct TestPacket##v##Payload        \
    {                                    \
        struct Connect##v##Payload conn; \
    };
APPLY(_TEST_PACKET_STRUCT, IP_VERSIONS)
#undef _TEST_PACKET_STRUCT

struct DebugRequestPayload
{
    bool avoid_locking;
};
#endif

#define PAYLOAD_T(name) struct CONCAT(name, Payload)

typedef enum
{
#define ENTRY(name) name,
    READ_MESSAGES
#undef ENTRY
} ReadMessageType;

typedef enum
{
#define ENTRY(name) name,
    WRITE_MESSAGES
#undef ENTRY
} WriteMessageType;

#define MAX_PAYLOAD_COUNT(name) (MAX_MESSAGE_SIZE / sizeof(struct name##Payload))
