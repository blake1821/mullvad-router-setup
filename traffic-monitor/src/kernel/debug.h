#include <linux/module.h>
// #define PRINT_DEBUG_STATEMENTS

#ifdef PRINT_DEBUG_STATEMENTS
#define my_debug(fmt, ...) printk(fmt, ##__VA_ARGS__)
#else
#define my_debug(fmt, ...)
#endif
