// Uncomment this to debug:
//#define IP_VERSION 6

#include "iplookup.h"
#include "debug.h"

// we don't need these right now
#undef DEBUG_ENTER
#undef DEBUG_EXIT

#ifdef __KERNEL__
#else
// in user-mode, we execute everything in a single thread

#define mutex_lock(x)
#define mutex_unlock(x)
#define KERN_ERR ""
#define printk(x...) printf(x)
#define DEFINE_MUTEX(x)
#define READ_ONCE(x) x
#define WRITE_ONCE(x, y) x = y
#define cmpxchg(ptr, old, new) (((*(ptr)) = new), old)
#define smp_wmb()
#define smp_rmb()
#endif

#ifdef __KERNEL__ 
static DEFINE_MUTEX(data_mutex);
#endif

struct StatusFrame
{
    IP_ADDR_T src;
    IP_ADDR_T dst;
    IPStatus status;
    bool used;
    int16_t slot;
    struct list_head *head;
};

#define EMPTY -1
#define TOMBSTONE -2

#define Q_BITS 9
#define Q_MASK ((1 << Q_BITS) - 1)
#define H_BITS (Q_BITS + 2)
#define H_MASK ((1 << H_BITS) - 1)
#define H2_MASK (((1 << 15) - 1) & ~Q_MASK)
#define ENTRY_MASK (((1 << 16) - 1) & ~Q_MASK)
#define TOMBSTONE_THRESHOLD (1 << (H_BITS - 2))

/*
 * Hash Table Entry Structure:
 * - Lower Q_BITS bits:         index in the status queue
 * - Next (15 - Q_BITS) bits:   some non-hash bits of the ip address
 * - Most significant bit:      1 if the entry is empty or a tombstone
 */

static struct StatusFrame status_queue[1 << Q_BITS] = {[0 ... Q_MASK] = {.slot = EMPTY}};
static int status_queue_index = 0;
static int16_t hash_table[1 << H_BITS] = {[0 ... H_MASK] = EMPTY};
static int tombstone_count = 0;

static inline int16_t ipv6_hash(struct in6_addr ipv6)
{
    int16_t hash = 0;
    for (int i = 0; (i & 128) == 0; i += 16)
    {
        hash ^= ipv6.s6_addr[i];
    }
    return hash & H_MASK;
}

struct lookup_data
{
    int16_t hash;
    int16_t h2;
    int16_t slot;
    int16_t *entry;
    struct StatusFrame *status_frame;
};

#define find_status(l, src_, dst_, on_found)                                           \
    l.hash = IP_HASH(src_, dst_);                                                      \
    l.h2 = IP_H2(src_, dst_);                                                          \
    l.slot = l.hash;                                                                   \
    while (*(l.entry = &hash_table[l.slot]) != EMPTY)                                  \
    {                                                                                  \
        if ((*l.entry & ENTRY_MASK) == l.h2 &&                                         \
            IP_EQ((l.status_frame = &status_queue[(*l.entry) & Q_MASK])->src, src_) && \
            IP_EQ(l.status_frame->dst, dst_))                                          \
        {                                                                              \
            on_found break;                                                            \
        }                                                                              \
        l.slot++;                                                                      \
        l.slot &= H_MASK;                                                              \
    }

// atomically grab the queue from the status frame and prevent other threads from modifying it
static struct list_head *grab_queue(struct StatusFrame *frame)
{
    struct list_head *head_new = (struct list_head *)1; /* 1 means overwritten */
    struct list_head *head;
    struct list_head *old;
    do
    {
        old = READ_ONCE(frame->head);
        head = cmpxchg(&frame->head, old, head_new);
    } while (head != old);
    return head;
}

// precondition: find_status must have been called, and *l.entry must be EMPTY
// postcondition: l.status_frame is a new, tracked status frame with src, dst, slot, and used set.
// the status & queue_no must be set by the caller
#define allocate_frame(l, src_, dst_)                                                  \
    /* we need to find an unallocated status frame -- possibly replacing an old one */ \
    while (l.status_frame = &status_queue[status_queue_index],                         \
           l.status_frame->used && l.status_frame->slot > EMPTY)                       \
    {                                                                                  \
        l.status_frame->used = 0;                                                      \
        status_queue_index++;                                                          \
        status_queue_index &= Q_MASK;                                                  \
    }                                                                                  \
    /* if we are replacing an old status frame, we need to update the table entry */   \
    int16_t old_slot = l.status_frame->slot;                                           \
    if (old_slot > EMPTY)                                                              \
    {                                                                                  \
        hash_table[old_slot] = TOMBSTONE;                                              \
        tombstone_count++;                                                             \
        if (l.status_frame->status == Pending)                                         \
        {                                                                              \
            DISPATCH_QUEUE(grab_queue(l.status_frame), Pending);                       \
        }                                                                              \
    }                                                                                  \
    /* set up the frame */                                                             \
    l.status_frame->slot = l.slot;                                                     \
    l.status_frame->used = 1;                                                          \
    WRITE_ONCE(l.status_frame->src, src_);                                             \
    WRITE_ONCE(l.status_frame->dst, dst_);                                             \
    /* set up the table entry */                                                       \
    *l.entry = l.h2 | status_queue_index;                                              \
    /* increment the status queue index */                                             \
    status_queue_index++;                                                              \
    status_queue_index &= Q_MASK;

static void clean_up_tombstones(void)
{

    memset(hash_table, 0xFF, sizeof(hash_table));

    IP_ADDR_T src;
    IP_ADDR_T dst;
    int16_t slot;

    for (int q_index = 0; q_index != (1 << Q_BITS); q_index++)
    {
        if (status_queue[q_index].slot > EMPTY)
        {
            src = status_queue[q_index].src;
            dst = status_queue[q_index].dst;
            slot = IP_HASH(src, dst);
            while (hash_table[slot] != EMPTY)
            {
                slot++;
                slot &= H_MASK;
            }
            hash_table[slot] = IP_H2(src, dst) | q_index;
            status_queue[q_index].slot = slot;
        }
    }

    tombstone_count = 0;
}

void ON_SETSTATUS(SETSTATUS_PAYLOAD_T *payloads, int n)
{
    mutex_lock(&data_mutex);

    for (int i = 0; i < n; i++)
    {
        SETSTATUS_PAYLOAD_T payload = payloads[i];

        // get the hash of the ip address
        struct lookup_data l;

        // check if a status frame is already allocated for this address
        // if so, update the status and return
        find_status(l, payload.src, payload.dst, {
            if (l.status_frame->status == Pending)
            {
                DISPATCH_QUEUE(grab_queue(l.status_frame), payload.status);
            }
            l.status_frame->status = payload.status;
            l.status_frame->used = 1;
            goto outer;
        });

        allocate_frame(l, payload.src, payload.dst);
        l.status_frame->status = payload.status;

        if (tombstone_count > TOMBSTONE_THRESHOLD)
        {
            clean_up_tombstones();
        }
    outer:
    }

    mutex_unlock(&data_mutex);
}

IPStatus GET_IPSTATUS(IP_ADDR_T src, IP_ADDR_T dst, uint16_t *queue_no)
{
    mutex_lock(&data_mutex);

    // check if a status frame is already allocated for this address
    // if so, return the status

    bool found = false;
    IPStatus status;
    struct lookup_data d;

    find_status(d, src, dst, {
        found = true;
        d.status_frame->used = 1;
        status = d.status_frame->status;
    });

    if (!found)
    {
        allocate_frame(d, src, dst);
        smp_wmb();
        WRITE_ONCE(d.status_frame->head, 0);
        status = d.status_frame->status = Pending;
        if (tombstone_count > TOMBSTONE_THRESHOLD)
        {
            clean_up_tombstones();
            d.entry = &hash_table[d.status_frame->slot];
        }
        mutex_unlock(&data_mutex);
        QUERY_PAYLOAD_T query = {src, dst};
        ENQUEUE_QUERY(&query);
    }
    else
    {
        mutex_unlock(&data_mutex);
    }

    *queue_no = *d.entry & Q_MASK;

    return status;
}

bool ENQUEUE_PACKET(struct list_head *node, uint16_t queue_no, IP_ADDR_T expected_src, IP_ADDR_T expected_dst)
{
    volatile struct StatusFrame *frame = &status_queue[queue_no];

    struct list_head *head;
    struct list_head *old_head;
    do
    {
        old_head = frame->head;
        smp_rmb();
        IP_ADDR_T src = frame->src;
        IP_ADDR_T dst = frame->dst;
        if (!IP_EQ(src, expected_src) || !IP_EQ(dst, expected_dst))
        {
            return false;
        }
        if (old_head == (struct list_head *)1)
        { // overwritten
            return false;
        }
        node->next = old_head;
        head = cmpxchg(&frame->head, old_head, node);
    } while (head != old_head);

    return true;
}

// TODO: IPStatus get_ipv6_status(struct in6_addr ipv6);

void IP_RESET()
{
    mutex_lock(&data_mutex);
    memset(hash_table, 0xFF, sizeof(hash_table));

    for (int q_index = 0; q_index != (1 << Q_BITS); q_index++)
    {
        if (status_queue[q_index].slot > EMPTY &&
            status_queue[q_index].status == Pending)
        {
            DISPATCH_QUEUE(grab_queue(&status_queue[q_index]), Blocked);
        }

        status_queue[q_index].slot = EMPTY;
    }

    tombstone_count = 0;
    mutex_unlock(&data_mutex);
}

int GET_ENQUEUED_COUNT(void)
{
    mutex_lock(&data_mutex);
    int count = 0;
    for (int i = 0; i < (1 << Q_BITS); i++)
    {
        if (status_queue[i].slot >= 0)
        {
            struct list_head *head = status_queue[i].head;
            while (head > (struct list_head *)1)
            {
                head = head->next;
                count++;
            }
        }
    }
    mutex_unlock(&data_mutex);
    return count;
}