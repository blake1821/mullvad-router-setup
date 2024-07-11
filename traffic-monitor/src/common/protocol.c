#include "protocol.h"

// ensure each type is defined (don't call this)
void __payloads_exist_check(){
    #define ENTRY(name) CONCAT(name, Payload) __useless_##name;
    READ_MESSAGES
    WRITE_MESSAGES
    #undef ENTRY
}