#pragma once
#include "../common/protocol.h"
#include "readqueue.h"

// defined in trafficmon.c
#define ENTRY(name) void on_##name(PAYLOAD_T(name) *payloads, int count);
WRITE_MESSAGES
#undef ENTRY

// defined in procfile.c
void init_procfile(void);
void exit_procfile(void);