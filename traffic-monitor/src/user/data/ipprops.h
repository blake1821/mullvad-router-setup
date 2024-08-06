#pragma once
extern "C"
{
#include "../../common/protocol.h"
#include "../../common/iplookup.h"
}
#include "ipaddress.h"

#define _IP_VERSION_ENUM(v) IPv##v,
enum class
    IPVersion
{
    APPLY(_IP_VERSION_ENUM, IP_VERSIONS)
};
#undef _IP_VERSION_ENUM

using enum IPVersion;

template <IPVersion>
struct IPProps;

#define _IP_VERSION_STRUCT(v)                                                                      \
    template <>                                                                                    \
    struct IPProps<IPv##v>                                                                         \
    {                                                                                              \
        using ConnectPayload = Connect##v##Payload;                                                \
        using QueryPayload = Query##v##Payload;                                                    \
        using SetStatusPayload = SetStatus##v##Payload;                                            \
        using RawAddress = __IP_ADDR_T(v);                                                         \
        using Address = IPv##v##Address;                                                           \
        static bool (*enqueue_packet)(struct list_head *, uint16_t,                                \
                                      RawAddress, RawAddress);                                     \
        static void (*on_set_status)(SetStatusPayload *, int);                                     \
    };                                                                                             

APPLY(_IP_VERSION_STRUCT, IP_VERSIONS)
#undef _IP_VERSION_STRUCT