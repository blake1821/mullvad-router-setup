#include "protocol.h"

// ensure each type is defined (don't call this)
void __payloads_exist_check(void)
{
#define ENTRY(name) struct CONCAT(name, Payload) __useless_##name;
    READ_MESSAGES
    WRITE_MESSAGES
#undef ENTRY
}

int get_read_payload_size(ReadMessageType type)
{
    switch (type)
    {
#define ENTRY(name) \
    case name:      \
        return sizeof(struct name##Payload);
        READ_MESSAGES
#undef ENTRY
    }
    return -1;
}

int get_write_payload_size(WriteMessageType type)
{
    switch (type)
    {
#define ENTRY(name) \
    case name:      \
        return sizeof(struct name##Payload);
        WRITE_MESSAGES
#undef ENTRY
    }
    return -1;
}
