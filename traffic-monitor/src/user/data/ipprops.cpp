#include "ipprops.h"

#define _IP_VERSION_STRUCT(v)                                                                      \
    bool (*IPProps<IPv##v>::enqueue_packet)(list_head *, uint16_t,                                 \
                                            __IP_ADDR_T(v), __IP_ADDR_T(v)) = __ENQUEUE_PACKET(v); \
    void (*IPProps<IPv##v>::on_set_status)(SetStatus##v##Payload *, int) = __ON_SETSTATUS(v);

APPLY(_IP_VERSION_STRUCT, IP_VERSIONS)
#undef _IP_VERSION_STRUCT