#include "trafficmon.h"

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