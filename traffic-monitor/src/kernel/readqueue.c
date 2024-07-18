#include "readqueue.h"
#include "debug.h"
#include <linux/circ_buf.h>

#define BUFFER_SIZE (1 << 8)
#define BUFFER_MASK (BUFFER_SIZE - 1)

#define ENTRY(name)                                                                                           \
    PAYLOAD_T(name)                                                                                           \
    buffer_##name[BUFFER_SIZE];                                                                               \
    DECLARE_WAIT_QUEUE_HEAD(wait_queue_##name);                                                               \
    long buffer_head_##name = 0;                                                                              \
    long buffer_tail_##name = 0;                                                                              \
    void enqueue_##name(PAYLOAD_T(name) * payload)                                                            \
    {                                                                                                         \
        /* Normally this implementation would result in problems, but the constraints                         \
         * of this project make it safe to assume that the buffer will never be full. */                      \
        buffer_##name[buffer_head_##name] = *payload;                                                         \
        spin_lock(&wait_queue_##name.lock);                                                                   \
        smp_wmb();                                                                                            \
        buffer_head_##name = (buffer_head_##name + 1) & BUFFER_MASK;                                          \
        wake_up_locked(&wait_queue_##name);                                                                   \
        spin_unlock(&wait_queue_##name.lock);                                                                 \
    }                                                                                                         \
                                                                                                              \
    ssize_t read_##name(struct file *file, char *buffer, size_t size, loff_t *offset)                         \
    {                                                                                                         \
        spin_lock(&wait_queue_##name.lock);                                                                   \
        if (buffer_head_##name == buffer_tail_##name)                                                         \
        {                                                                                                     \
            if (wait_event_interruptible_locked(wait_queue_##name, buffer_head_##name != buffer_tail_##name)) \
            {                                                                                                 \
                spin_unlock(&wait_queue_##name.lock);                                                         \
                return -EINTR;                                                                                \
            }                                                                                                 \
        }                                                                                                     \
        int initial_tail = buffer_tail_##name;                                                                \
        int payload_count = min((int)MAX_PAYLOAD_COUNT(name),                                                 \
                                (int)CIRC_CNT(buffer_head_##name, buffer_tail_##name, BUFFER_SIZE));          \
        buffer_tail_##name = (buffer_tail_##name + payload_count) & BUFFER_MASK;                              \
        spin_unlock(&wait_queue_##name.lock);                                                                 \
                                                                                                              \
        int message_size = payload_count * sizeof(PAYLOAD_T(name));                                           \
                                                                                                              \
        if (initial_tail > buffer_tail_##name)                                                                \
        {                                                                                                     \
            int round_1_size = (BUFFER_SIZE - initial_tail) * sizeof(PAYLOAD_T(name));                        \
            copy_to_user(buffer, &buffer_##name[initial_tail], round_1_size);                                 \
            buffer += round_1_size;                                                                           \
            message_size -= round_1_size;                                                                     \
            initial_tail = 0;                                                                                 \
        }                                                                                                     \
        copy_to_user(buffer, &buffer_##name[initial_tail], message_size);                                     \
        return payload_count;                                                                                 \
    }
READ_MESSAGES
#undef ENTRY

void init_read_queue(void)
{
    // nothing to do yet
}

void exit_read_queue(void)
{
    // nothing to do yet
}
