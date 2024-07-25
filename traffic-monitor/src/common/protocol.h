#pragma once

#ifdef __KERNEL__
#include <net/inet_common.h>
#else
#include <netinet/in.h>
#include <stdbool.h>
struct list_head
{
    struct list_head *next;
    void *data;
};
#endif
#include "ipmacros.h"

#define TRAFFICMON_PROC_FILE "trafficmon"
#define MAX_MESSAGE_SIZE 4096

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

#define DECLARATION_TF(name) DECLARATION(int, CONCAT(name, _queue_size), 0)
#define DEBUG_PARAMS                       \
    DECLARATION(int, verdict_responses, 0) \
    DECLARATION(int, enqueued_ipv4, 0)     \
    DEFAULT_READ_MESSAGES

#define READ_MESSAGES     \
    DEFAULT_READ_MESSAGES \
    ENTRY(TestVerdict4)   \
    ENTRY(DebugResponse)
#define WRITE_MESSAGES     \
    DEFAULT_WRITE_MESSAGES \
    ENTRY(TestPacket4)     \
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

typedef enum
{
    ProtoTcp,
    ProtoUdp,
    ProtoOther
} ConnectProtocol;

// read() messages
APPLY(DECLARE_CONNECT_STRUCT, IP_VERSIONS)
APPLY(DECLARE_QUERY_STRUCT, IP_VERSIONS)

#ifdef TEST_NETHOOKS
struct TestVerdict4Payload
{
    struct Connect4Payload conn;
    bool allowed;
};

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
    char outgoing_dev_name[32];
};

struct ResetPayload
{
    bool reset;
};

#ifdef TEST_NETHOOKS
struct TestPacket4Payload
{
    struct Connect4Payload conn;
};

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
