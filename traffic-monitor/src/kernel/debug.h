#pragma once
#include <linux/module.h>
#include "../common/protocol.h"
// #define PRINT_DEBUG_STATEMENTS

#ifdef TEST_NETHOOKS
#define my_debug(fmt, ...) printk(fmt, ##__VA_ARGS__)
#define DBG(x) x
#else
#define my_debug(fmt, ...)
#define DBG(x)
#endif
#define debug_incr_verdict_responses()
