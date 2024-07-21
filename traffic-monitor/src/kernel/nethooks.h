
#include "../common/protocol.h"

// defined in trafficmon.c
IPStatus network4_request(struct in_addr src, struct in_addr dst);

// defined in net.c
void init_nethooks(void);
void exit_nethooks(void);