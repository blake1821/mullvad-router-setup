#include "debug.h"
#include "../common/iplookup.h"
#include "readqueue.h"

#ifdef TEST_NETHOOKS

atomic_t atomic_debug_verdict_responses = {
    .counter = 0};

void on_DebugRequest(struct DebugRequestPayload *payload, int n)
{
    if (n == 1)
    {

        // debug params. all must be defined here or the compiler will complain
        int debug_enqueued_ipv4;
        if (payload->avoid_locking)
        {
            debug_enqueued_ipv4 = 0;
        }
        else
        {
            debug_enqueued_ipv4 = get_enqueued_ipv4();
        }

        int debug_verdict_responses = atomic_read(&atomic_debug_verdict_responses);

        struct DebugResponsePayload response = {
#define ENTRY(f)
#define DECLARATION(type, name, value) .name = CONCAT(debug_, name),
            DEBUG_PARAMS
#undef ENTRY
#undef DECLARATION
        };
        debug_get_queue_sizes(&response);

        enqueue_DebugResponse(&response);
    }
}

/*
void debug_incr_verdict_responses(void)
{
    atomic_inc(&atomic_debug_verdict_responses);
}
*/

#else
#endif
