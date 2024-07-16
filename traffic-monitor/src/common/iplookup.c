#ifdef __KERNEL__
#else
#define mutex_lock(x) 
#define mutex_unlock(x) 
#define printk(...)
#define NULL 0
#define DEFINE_MUTEX(x)
#define bool char
#define true 1
#define false 0
#endif

#include "iplookup.h"

DEFINE_MUTEX(ip4_lock);

struct StatusFrame4
{
    struct in_addr ipv4;
    IPStatus status;
    bool used;
    int16_t slot;
};

#define EMPTY -1
#define TOMBSTONE -2

#define Q_BITS 9
#define Q_MASK ((1 << Q_BITS) - 1)
#define H_BITS (Q_BITS + 2)
#define H_MASK ((1 << H_BITS) - 1)
#define H2_MASK (((1 << 15) - 1) & ~Q_MASK)
#define ENTRY_MASK (((1 << 16) - 1) & ~Q_MASK)
#define TOMBSTONE_THRESHOLD (1 << (H_BITS - 1))

/*
 * Hash Table Entry Structure:
 * - Lower Q_BITS bits:         index in the status queue
 * - Next (15 - Q_BITS) bits:   some non-hash bits of the ip address
 * - Most significant bit:      1 if the entry is empty or a tombstone
 */

struct StatusFrame4 ipv4_status_queue[1 << Q_BITS] = {[0 ... Q_MASK] = {.slot = EMPTY}};
int ipv4_status_queue_index = 0;
int16_t ipv4_hash_table[1 << H_BITS] = {[0 ... H_MASK] = EMPTY};
#define IPV4_HASH(ipv4) ((ipv4.s_addr ^ (ipv4.s_addr >> 16)) & H_MASK)
#define IPV4_H2(ipv4) (ipv4.s_addr & H2_MASK)
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

#define find_ipv4_status(l, ip, on_found)                                                         \
    l.hash = IPV4_HASH(ip);                                                                       \
    l.h2 = IPV4_H2(ip);                                                                           \
    l.slot = l.hash;                                                                              \
    while (*(l.entry = &ipv4_hash_table[l.slot]) != EMPTY)                                        \
    {                                                                                             \
        if ((*l.entry & ENTRY_MASK) == l.h2 &&                                                      \
            (l.status_frame = &ipv4_status_queue[(*l.entry) & Q_MASK])->ipv4.s_addr == ip.s_addr) \
        {                                                                                         \
            on_found break;                                                                       \
        }                                                                                         \
        l.slot++;                                                                                 \
        l.slot &= H_MASK;                                                                         \
    }

void ipv4_clean_up_tombstones(void)
{

    for (int i = 0; i <= H_MASK; i++)
    {
        ipv4_hash_table[i] = EMPTY;
    }

    ipv4_tombstone_count = 0;

    struct ipv4_lookup_data d;

    for (int i = 0; i <= Q_MASK; i++)
    {
        if (ipv4_status_queue[i].slot > EMPTY)
        {
            find_ipv4_status(d, ipv4_status_queue[i].ipv4, {
                printk(KERN_ERR "Shouldn't have found a status frame.\n");
                return;
            });
            *d.entry = d.h2 | i;
            ipv4_status_queue[i].slot = d.slot;
        }
    }
}

void set_ipv4_status(struct SetStatus4Payload *payloads, int n)
{

    mutex_lock(&ip4_lock);

    for (int i = 0; i < n; i++)
    {
        struct SetStatus4Payload payload = payloads[i];

        // get the hash of the ipv4 address
        struct ipv4_lookup_data l;

        // check if a status frame is already allocated for this address
        // if so, update the status and return
        find_ipv4_status(l, payload.ipv4, {
            l.status_frame->status = payload.status;
            l.status_frame->used = 1;
            goto outer;
        });

        // we need to find an unallocated status frame -- possibly replacing an old one
        while (l.status_frame = &ipv4_status_queue[ipv4_status_queue_index],
               l.status_frame->used && l.status_frame->slot > EMPTY)
        {
            ipv4_status_queue[ipv4_status_queue_index].used = 0;
            ipv4_status_queue_index++;
            ipv4_status_queue_index &= Q_MASK;
        }

        // if we are replacing an old status frame, we need to update the table entry
        int16_t old_slot = l.status_frame->slot;
        if (old_slot > EMPTY)
        {
            ipv4_hash_table[old_slot] = TOMBSTONE;
            ipv4_tombstone_count++;
        }

        // set up the frame
        l.status_frame->slot = l.slot;
        l.status_frame->status = payload.status;
        l.status_frame->used = 1;
        l.status_frame->ipv4 = payload.ipv4;

        // set up the table entry
        *l.entry = l.h2 | ipv4_status_queue_index;

        // increment the status queue index
        ipv4_status_queue_index++;
        ipv4_status_queue_index &= 1023;

    outer:
    }

    if (ipv4_tombstone_count > TOMBSTONE_THRESHOLD)
    {
        ipv4_clean_up_tombstones();
    }

    mutex_unlock(&ip4_lock);
}

// TODO: void set_ipv6_status(struct SetStatus4Payload payload);

IPStatus get_ipv4_status(struct in_addr ipv4)
{

    mutex_lock(&ip4_lock);

    // check if a status frame is already allocated for this address
    // if so, return the status

    IPStatus *status = NULL;

    struct ipv4_lookup_data d;
    find_ipv4_status(d, ipv4, {
        d.status_frame->used = 1;
        status = &d.status_frame->status;
    });

    mutex_unlock(&ip4_lock);

    if (status)
    {
        return *status;
    }

    // if no status frame is allocated, look it up
    struct SetStatus4Payload payload = query_ipv4(ipv4);
    set_ipv4_status(&payload, 1);
    return payload.status;
}

// TODO: IPStatus get_ipv6_status(struct in6_addr ipv6);

void init_iplookup(void)
{
    // do nothing for now
}