#include "trafficmon.h"

uint64_t get_ip_pair_key(struct in_addr src, struct in_addr dst);
uint64_t get_conn_key(Connect4Payload &payload);

#ifdef TEST_NETHOOKS
void print_debug_response(DebugResponsePayload &response);
#endif