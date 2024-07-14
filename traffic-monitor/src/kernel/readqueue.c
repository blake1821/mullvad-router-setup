#include "readqueue.h"

DEFINE_MUTEX(read_queue_lock);
DECLARE_WAIT_QUEUE_HEAD(read_wait_queue);

#define NODE_T(name) struct CONCAT(name, Node)
#define HEAD(name) CONCAT(name, _head)

// define a queue for each message type
#define ENTRY(name)          \
    NODE_T(name)             \
    {                        \
        PAYLOAD_T(name)      \
        payload;             \
        NODE_T(name) * next; \
    };                       \
    NODE_T(name) * HEAD(name) = NULL;

READ_MESSAGES
#undef ENTRY

// move all queued messages into the buffer and return the count
#define ENTRY(name)                                                         \
    int dequeue_##name(void)                                                \
    {                                                                       \
        int count = 0;                                                      \
        NODE_T(name) * temp;                                                \
        while (HEAD(name) != NULL && count < MAX_PAYLOAD_COUNT(name))       \
        {                                                                   \
            ((PAYLOAD_T(name) *)read_payload)[count] = HEAD(name)->payload; \
            temp = HEAD(name)->next;                                        \
            kfree(HEAD(name));                                              \
            HEAD(name) = temp;                                              \
            count++;                                                        \
        }                                                                   \
        return count;                                                       \
    }
READ_MESSAGES
#undef ENTRY

// an array of function pointers to the dequeue functions
// note that the order of the functions must match the order of the messages
int (*dequeue_functions[])(void) = {
#define ENTRY(name) dequeue_##name,
    READ_MESSAGES
#undef ENTRY
};
const int dequeue_functions_count = sizeof(dequeue_functions) / sizeof(dequeue_functions[0]);

// add a message to the queue and wake up the reader
#define ENTRY(name)                                                     \
    void enqueue_##name(PAYLOAD_T(name) payload)                        \
    {                                                                   \
        NODE_T(name) *node = kmalloc(sizeof(NODE_T(name)), GFP_KERNEL); \
        node->payload = payload;                                        \
        mutex_lock(&read_queue_lock);                                   \
        node->next = HEAD(name);                                        \
        HEAD(name) = node;                                              \
        spin_lock(&read_wait_queue.lock);                               \
        mutex_unlock(&read_queue_lock);                                 \
        wake_up_locked(&read_wait_queue);                               \
        spin_unlock(&read_wait_queue.lock);                             \
    }
READ_MESSAGES
#undef ENTRY

ReadMessageType next_type = 0;

/**
 * In a round-robin fashion, create a read message from a queue.
 * If no message is available, block until one is enqueued.
 */
void create_read_message(void)
{
    bool created = false;
    mutex_lock(&read_queue_lock);
    while (!created)
    {
        int i;
        for (i = 0; i < dequeue_functions_count && !created; i++)
        {
            int count;
            if ((count = dequeue_functions[next_type]()))
            {
                read_header.count = count;
                read_header.type = next_type;
                created = true;
            }
            next_type++;
            if (next_type == dequeue_functions_count)
                next_type = 0;
        }
        if (!created)
        {
            spin_lock(&read_wait_queue.lock);
            mutex_unlock(&read_queue_lock);
            wait_event_interruptible_locked(read_wait_queue, true);
            spin_unlock(&read_wait_queue.lock);
        }
    }
    mutex_unlock(&read_queue_lock);
}

void init_read_queue(void)
{
    // nothing to do yet
}

void exit_read_queue(void)
{
    for (int i = 0; i < dequeue_functions_count; i++)
    {
        // deallocate all remaining nodes
        while (dequeue_functions[i]())
            ;
    }
}
