#include "trafficmon.h"

uint64_t get_ip_pair_key(struct in_addr src, struct in_addr dst)
{
    return (uint64_t)src.s_addr << 32 | dst.s_addr;
}

uint64_t get_conn_key(Connect4Payload &payload)
{
    return get_ip_pair_key(payload.src, payload.dst) ^ payload.dst_port ^ (payload.protocol << 16);
}

#ifdef TEST_NETHOOKS
void print_debug_response(DebugResponsePayload &response){
    cout << "Debug response: " << endl;
    #define ENTRY(name) DECLARATION_TF(name)
    #define DECLARATION(type, name, value) cout << "\t" << #name << ": " << response.name << endl;
    DEBUG_PARAMS
    #undef ENTRY
    #undef DECLARATION
}
#endif