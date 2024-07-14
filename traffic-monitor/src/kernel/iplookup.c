#ifdef __KERNEL__
#include <linux/hashtable.h>
#include <kunit/test-bug.h>
#endif

#include "iplookup.h"

DEFINE_MUTEX(ip4_lock);

struct StatusFrame{
    IPStatus status;
    bool used;
    int16_t ip_hash;
};

#define DEFINE_STATUS_QUEUE(name, bits)						\
	struct StatusFrame name[1 << (bits)] =					\
			{ [0 ... ((1 << (bits)) - 1)] = {.ip_hash = -1} }

DEFINE_STATUS_QUEUE(ipv4_status_queue, 10);
int ipv4_status_queue_index = 0;
DEFINE_STATUS_QUEUE(ipv6_status_queue, 10);
int ipv6_status_queue_index = 0;

struct IPv4TableEntry{
    struct in_addr ipv4;
    int16_t status_index;
    struct hlist_node node;
};

struct IPv6TableEntry{
    struct in6_addr ipv6;
    int16_t status_index;
    struct hlist_node node;
};

DEFINE_HASHTABLE(ipv4_table, 10);
DEFINE_HASHTABLE(ipv6_table, 10);

#define IPV4_HASH(ipv4) ((ipv4.s_addr ^ (ipv4.s_addr >> 16)) & 0x3FF)

inline int16_t ipv6_hash(struct in6_addr ipv6){
    int16_t hash = 0;
    for(int i = 0; (i & 128) == 0; i += 16){
        hash ^= ipv6.s6_addr[i];
    }
    return hash & 0x3FF;
}

#define IPV6_HASH(ipv6) ipv6_hash(ipv6)


void set_ipv4_status(struct SetStatus4Payload *payloads, int n){

    mutex_lock(&ip4_lock);

    for(int i = 0; i < n; i++){
        struct SetStatus4Payload payload = payloads[i];

        // get the hash of the ipv4 address
        int16_t hash = IPV4_HASH(payload.ipv4);

        // check if a status frame is already allocated for this address
        // if so, update the status and return
        struct IPv4TableEntry *entry;
        struct StatusFrame *status_frame;
        hlist_for_each_entry(entry, &ipv4_table[hash], node){
            if(entry->ipv4.s_addr == payload.ipv4.s_addr){
                status_frame = &ipv4_status_queue[entry->status_index];
                status_frame->status = payload.status;
                status_frame->used = 1;
                return;
            }
        }

        // we need to find an unallocated status frame -- possibly replacing an old one
        while(status_frame = &ipv4_status_queue[ipv4_status_queue_index],
                status_frame->ip_hash >-1 && status_frame->used){
            ipv4_status_queue[ipv4_status_queue_index].used = 0;
            ipv4_status_queue_index++;
            ipv4_status_queue_index &= 1023;
        }

        // if we are replacing an old status frame, we need to update the table entry
        int16_t old_hash = status_frame->ip_hash;
        if(old_hash > -1){
            struct IPv4TableEntry *old_entry;
            hlist_for_each_entry(old_entry, &ipv4_table[old_hash], node){
                if(old_entry->status_index == ipv4_status_queue_index){
                    break;
                }
            }
            hash_del(&old_entry->node);
            kfree(old_entry);
        }

        // set up the frame
        status_frame->ip_hash = hash;
        status_frame->status = payload.status;
        status_frame->used = 1;

        // set up the table entry
        entry = kmalloc(sizeof(struct IPv4TableEntry), GFP_KERNEL);
        entry->ipv4 = payload.ipv4;
        entry->status_index = ipv4_status_queue_index;
        hlist_add_head(&entry->node, &ipv4_table[hash]);

        // increment the status queue index
        ipv4_status_queue_index++;
        ipv4_status_queue_index &= 1023;
    }

    mutex_unlock(&ip4_lock);
}

// TODO: void set_ipv6_status(struct SetStatus4Payload payload);

IPStatus get_ipv4_status(struct in_addr ipv4){
    // get the hash of the ipv4 address
    int16_t hash = IPV4_HASH(ipv4);

    mutex_lock(&ip4_lock);

    // check if a status frame is already allocated for this address
    // if so, return the status
    struct IPv4TableEntry *entry;
    IPStatus *status = NULL;
    hlist_for_each_entry(entry, &ipv4_table[hash], node){
        if(entry->ipv4.s_addr == ipv4.s_addr){
            struct StatusFrame *status_frame = &ipv4_status_queue[entry->status_index];
            status_frame->used = 1;
            status = &status_frame->status;
        }
    }

    mutex_unlock(&ip4_lock);

    if(status){
        return *status;
    }

    // if no status frame is allocated, look it up
    struct SetStatus4Payload payload = query_ipv4(ipv4);
    set_ipv4_status(&payload, 1);
    return payload.status;
}

// TODO: IPStatus get_ipv6_status(struct in6_addr ipv6);

void init_iplookup(void){
    hash_init(ipv4_table);
    hash_init(ipv6_table);
}