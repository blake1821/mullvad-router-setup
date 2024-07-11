#include <linux/hashtable.h>
#include "iplookup.h"
// These methods are not thread-safe
// Use a lock in the main file

DEFINE_HASHTABLE(ipv4_table, 10);
DEFINE_HASHTABLE(ipv6_table, 10);

void add_ipv4(struct in_addr ipv4, IPStatus status){
    struct iplookup_entry *entry = kmalloc(sizeof(struct iplookup_entry), GFP_KERNEL);
    entry->ipv4 = ipv4;
    entry->status = status;
    hash_add(ipv4_table, &entry->node, ipv4.s_addr);
}



void init_iplookup(){

}