#include "debug.h"
#include "../common/iplookup.h"
#include "readqueue.h"

#ifdef TEST_NETHOOKS

atomic_t atomic_debug_verdict_responses = {.counter = 0};
atomic_t atomic_debug_enqueue_failures = {.counter = 0};
atomic_t atomic_debug_overflow_packets = {.counter = 0};

void on_DebugRequest(struct DebugRequestPayload *payload, int n)
{
    if (n == 1)
    {

        // debug params. all must be defined here or the compiler will complain
        int debug_ipv4_enqueued;
        int debug_ipv6_enqueued;

        if (payload->avoid_locking)
        {
            debug_ipv4_enqueued = -1;
            debug_ipv6_enqueued = -1;
        }
        else
        {
            debug_ipv4_enqueued = __GET_ENQUEUED_COUNT(4)();
            debug_ipv6_enqueued = __GET_ENQUEUED_COUNT(6)();
        }

        int debug_verdict_responses = atomic_read(&atomic_debug_verdict_responses);
        int debug_enqueue_failures = atomic_read(&atomic_debug_enqueue_failures);
        int debug_overflow_packets = atomic_read(&atomic_debug_overflow_packets);
        atomic_set(&atomic_debug_verdict_responses, 0);
        atomic_set(&atomic_debug_enqueue_failures, 0);
        atomic_set(&atomic_debug_overflow_packets, 0);

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

void debug_incr_verdict_responses(void)
{
    atomic_inc(&atomic_debug_verdict_responses);
}

void debug_incr_enqueue_failures(void)
{
    atomic_inc(&atomic_debug_enqueue_failures);
}

void debug_incr_overflow_packets(void)
{
    atomic_inc(&atomic_debug_overflow_packets);
}

#else
#endif
