#include "iplookup.h"

void init_iplookup(void){
    // nothing to do yet
}

void on_Reset(struct ResetPayload *payload, int n){
    if(n > 0){
        #define CALL_RESET(v) __IP_RESET(v)();
        APPLY(CALL_RESET, IP_VERSIONS)
    }
}
