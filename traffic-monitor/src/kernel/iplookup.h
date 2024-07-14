#include "../common/protocol.h"

// Defined in iplookup.c:

void init_iplookup(void);
void set_ipv4_status(struct SetStatus4Payload *payload, int n);
// TODO: void set_ipv6_status(struct SetStatus4Payload payload);
IPStatus get_ipv4_status(struct in_addr ipv4);
// TODO: IPStatus get_ipv6_status(struct in6_addr ipv6);


// Defined in trafficmon.c:

struct SetStatus4Payload query_ipv4(struct in_addr ipv4);
// TODO: struct SetStatus6Payload query_ipv6(struct in_addr ipv6);