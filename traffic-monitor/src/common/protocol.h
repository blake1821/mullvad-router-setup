#pragma once

#ifdef __KERNEL__
#include <net/inet_common.h>
#endif

#define TRAFFICMON_PROC_FILE "trafficmon"
#define MAX_MESSAGE_SIZE 1024

#define CONCAT(x, y) x##y

/*
Messaging protocol:
1. Read/write the message type
2. Read/write the payload count, n, where n*payload size <= MAX_MESSAGE_SIZE
3. Read/write n payloads (all in one operation)
*/

#define READ_MESSAGES \
    ENTRY(Connect4) \
    ENTRY(Connect6) \
    ENTRY(Query4) \
    ENTRY(Query6)

#define WRITE_MESSAGES \
    ENTRY(SetStatus4) \
    ENTRY(SetStatus6)

enum IPStatus{
    Allowed,
    Blocked
};

// read() messages
struct Connect4Payload{
    struct in_addr ipv4;
};

struct Connect6Payload{
    struct in6_addr ipv6;
};

struct Query4Payload{
    struct in_addr ipv4;
};

struct Query6Payload{
    struct in6_addr ipv6;
};

// write() messages
struct SetStatus4Payload{
    struct in_addr ipv4;
    IPStatus status;
};

struct SetStatus6Payload{
    struct in6_addr ipv6;
    IPStatus status;
};

enum ReadMessageType{
    #define ENTRY(name) name,
    READ_MESSAGES
    #undef ENTRY
};

enum WriteMessageType{
    #define ENTRY(name) name,
    WRITE_MESSAGES
    #undef ENTRY
};
