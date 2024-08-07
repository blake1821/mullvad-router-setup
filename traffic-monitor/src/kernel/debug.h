#pragma once
#include <linux/module.h>
#include "../common/protocol.h"

#ifdef TEST_NETHOOKS
#define my_debug(fmt, ...) printk(fmt, ##__VA_ARGS__)
#define DBG(x) x
void debug_incr_verdict_responses(void);
void debug_incr_enqueue_failures(void);
void debug_incr_overflow_packets(void);
#else
#define my_debug(fmt, ...)
#define DBG(x)
#define debug_incr_verdict_responses()
#define debug_incr_enqueue_failures()
#define debug_incr_overflow_packets()
#endif
