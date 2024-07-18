#pragma once

#ifdef __KERNEL__
#include <net/inet_common.h>
#else
#include<netinet/in.h>
#endif

#define TRAFFICMON_PROC_FILE "trafficmon"
#define TEST_INTERFACE_PROC_FILE "test_interface"
#define MAX_MESSAGE_SIZE 1024

#define CONCAT(x, y) x##y

/*
Messaging protocol:
1. Open the file corresponding to the message type, for example, /proc/trafficmon/Connect4
2. Read/write using a buffer of size MAX_MESSAGE_SIZE
3. Read/write n payloads, and expect the size parameter or return value to be n
*/

#define READ_MESSAGES \
    ENTRY(Connect4) \
    ENTRY(Connect6) \
    ENTRY(Query4) \
    ENTRY(Query6)

#define WRITE_MESSAGES \
    ENTRY(SetStatus4) \
    ENTRY(SetStatus6)

typedef enum {
    Allowed,
    Blocked
} IPStatus;

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

#define PAYLOAD_T(name) struct CONCAT(name, Payload)


typedef enum {
    #define ENTRY(name) name,
    READ_MESSAGES
    #undef ENTRY
} ReadMessageType;

typedef enum{
    #define ENTRY(name) name,
    WRITE_MESSAGES
    #undef ENTRY
} WriteMessageType;

/* No more headers!

struct ReadHeader {
    ReadMessageType type;
    uint32_t count;
};

struct WriteHeader {
    WriteMessageType type;
    uint32_t count;
};
int get_read_payload_size(ReadMessageType type);
int get_write_payload_size(WriteMessageType type);
*/

#define MAX_PAYLOAD_COUNT(name) (MAX_MESSAGE_SIZE / sizeof(struct name##Payload))
