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

DEFINE_MUTEX(ip4_lock);

struct StatusFrame4
{
    struct in_addr src;
    struct in_addr dst;
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

struct StatusFrame4 ipv4_status_queue[1 << Q_BITS] = {[0 ... Q_MASK] = {.slot = EMPTY}};
int ipv4_status_queue_index = 0;
int16_t ipv4_hash_table[1 << H_BITS] = {[0 ... H_MASK] = EMPTY};
#define IPV4_HASH(src, dst) ((dst.s_addr ^ (dst.s_addr >> 16) ^ src.s_addr) & H_MASK)
#define IPV4_H2(src, dst) (dst.s_addr & H2_MASK)
int ipv4_tombstone_count = 0;

inline int16_t ipv6_hash(struct in6_addr ipv6)
{
    int16_t hash = 0;
    for (int i = 0; (i & 128) == 0; i += 16)
    {
        hash ^= ipv6.s6_addr[i];
    }
    return hash & H_MASK;
}
#define IPV6_HASH(ipv6) ipv6_hash(ipv6)

struct ipv4_lookup_data
{
    int16_t hash;
    int16_t h2;
    int16_t slot;
    int16_t *entry;
    struct StatusFrame4 *status_frame;
};

#define find_ipv4_status(l, src_, dst_, on_found)                                                    \
    l.hash = IPV4_HASH(src_, dst_);                                                                  \
    l.h2 = IPV4_H2(src_, dst_);                                                                      \
    l.slot = l.hash;                                                                                 \
    while (*(l.entry = &ipv4_hash_table[l.slot]) != EMPTY)                                           \
    {                                                                                                \
        if ((*l.entry & ENTRY_MASK) == l.h2 &&                                                       \
            (l.status_frame = &ipv4_status_queue[(*l.entry) & Q_MASK])->src.s_addr == src_.s_addr && \
            l.status_frame->dst.s_addr == dst_.s_addr)                                               \
        {                                                                                            \
            on_found break;                                                                          \
        }                                                                                            \
        l.slot++;                                                                                    \
        l.slot &= H_MASK;                                                                            \
    }

// atomically grab the queue from the status frame and prevent other threads from modifying it
struct list_head *grab_queue(struct StatusFrame4 *frame)
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

// precondition: find_ipv4_status must have been called, and *l.entry must be EMPTY
// postcondition: l.status_frame is a new, tracked status frame with src, dst, slot, and used set.
// the status & queue_no must be set by the caller
#define allocate_frame(l, src_, dst_)                                                  \
    /* we need to find an unallocated status frame -- possibly replacing an old one */ \
    while (l.status_frame = &ipv4_status_queue[ipv4_status_queue_index],               \
           l.status_frame->used && l.status_frame->slot > EMPTY)                       \
    {                                                                                  \
        l.status_frame->used = 0;                                                      \
        ipv4_status_queue_index++;                                                     \
        ipv4_status_queue_index &= Q_MASK;                                             \
    }                                                                                  \
    /* if we are replacing an old status frame, we need to update the table entry */   \
    int16_t old_slot = l.status_frame->slot;                                           \
    if (old_slot > EMPTY)                                                              \
    {                                                                                  \
        ipv4_hash_table[old_slot] = TOMBSTONE;                                         \
        ipv4_tombstone_count++;                                                        \
        if (l.status_frame->status == Pending)                                         \
        {                                                                              \
            dispatch_queue4(grab_queue(l.status_frame), Pending);                      \
        }                                                                              \
    }                                                                                  \
    /* set up the frame */                                                             \
    l.status_frame->slot = l.slot;                                                     \
    l.status_frame->used = 1;                                                          \
    WRITE_ONCE(l.status_frame->src, src_);                                             \
    WRITE_ONCE(l.status_frame->dst, dst_);                                             \
    /* set up the table entry */                                                       \
    *l.entry = l.h2 | ipv4_status_queue_index;                                         \
    /* increment the status queue index */                                             \
    ipv4_status_queue_index++;                                                         \
    ipv4_status_queue_index &= Q_MASK;

void ipv4_clean_up_tombstones(void)
{

    memset(ipv4_hash_table, 0xFF, sizeof(ipv4_hash_table));

    struct in_addr src;
    struct in_addr dst;
    int16_t slot;

    for (int q_index = 0; q_index != (1 << Q_BITS); q_index++)
    {
        if (ipv4_status_queue[q_index].slot > EMPTY)
        {
            src = ipv4_status_queue[q_index].src;
            dst = ipv4_status_queue[q_index].dst;
            slot = IPV4_HASH(src, dst);
            while (ipv4_hash_table[slot] != EMPTY)
            {
                slot++;
                slot &= H_MASK;
            }
            ipv4_hash_table[slot] = IPV4_H2(src, dst) | q_index;
            ipv4_status_queue[q_index].slot = slot;
        }
    }

    ipv4_tombstone_count = 0;
}

void on_SetStatus4(struct SetStatus4Payload *payloads, int n)
{
    mutex_lock(&ip4_lock);

    for (int i = 0; i < n; i++)
    {
        struct SetStatus4Payload payload = payloads[i];

        // get the hash of the ipv4 address
        struct ipv4_lookup_data l;

        // check if a status frame is already allocated for this address
        // if so, update the status and return
        find_ipv4_status(l, payload.src, payload.dst, {
            if (l.status_frame->status == Pending)
            {
                dispatch_queue4(grab_queue(l.status_frame), payload.status);
            }
            l.status_frame->status = payload.status;
            l.status_frame->used = 1;
            goto outer;
        });

        allocate_frame(l, payload.src, payload.dst);
        l.status_frame->status = payload.status;

        if (ipv4_tombstone_count > TOMBSTONE_THRESHOLD)
        {
            ipv4_clean_up_tombstones();
        }
    outer:
    }

    mutex_unlock(&ip4_lock);
}

// TODO: void set_ipv6_status(struct SetStatus4Payload payload);

IPStatus get_ipv4_status(struct in_addr src, struct in_addr dst, uint16_t *queue_no)
{

    mutex_lock(&ip4_lock);

    // check if a status frame is already allocated for this address
    // if so, return the status

    bool found = false;
    IPStatus status;
    struct ipv4_lookup_data d;

    find_ipv4_status(d, src, dst, {
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
        if (ipv4_tombstone_count > TOMBSTONE_THRESHOLD)
        {
            ipv4_clean_up_tombstones();
            d.entry = &ipv4_hash_table[d.status_frame->slot];
        }
        mutex_unlock(&ip4_lock);
        struct Query4Payload query = {src, dst};
        enqueue_Query4(&query);
    }
    else
    {
        mutex_unlock(&ip4_lock);
    }

    *queue_no = *d.entry & Q_MASK;

    return status;
}

bool enqueue_ipv4(struct list_head *node, uint16_t queue_no, struct in_addr expected_src, struct in_addr expected_dst)
{
    struct StatusFrame4 *frame = &ipv4_status_queue[queue_no];

    struct list_head *head;
    struct list_head *old_head;
    do
    {
        old_head = READ_ONCE(frame->head);
        smp_rmb();
        struct in_addr src = READ_ONCE(frame->src);
        struct in_addr dst = READ_ONCE(frame->dst);
        if (src.s_addr != expected_src.s_addr || dst.s_addr != expected_dst.s_addr)
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

void init_iplookup(void)
{
}

int get_enqueued_ipv4(void)
{
    mutex_lock(&ip4_lock);
    int count = 0;
    for (int i = 0; i < (1 << Q_BITS); i++)
    {
        if (ipv4_status_queue[i].slot >= 0)
        {
            struct list_head *head = ipv4_status_queue[i].head;
            while(head > (struct list_head*) 1){
                head = head->next;
                count++;
            }
        }
    }
    mutex_unlock(&ip4_lock);
    return count;
}