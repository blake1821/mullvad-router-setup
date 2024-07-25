#pragma once

#include "../common/protocol.h"
#include "debug.h"

// please forgive this monstrosity. this should only be included by
// nethooks.c

#ifdef TEST_NETHOOKS

struct iphdr_
{
    unsigned int saddr;
    unsigned int daddr;
};
#define iphdr iphdr_

struct nf_queue_entry_
{
    struct list_head list;
    struct Connect4Payload *skb;
};
#define nf_queue_entry nf_queue_entry_

#define sk_buff Connect4Payload

struct queue_entry_and_skb
{
    struct nf_queue_entry entry;
    struct sk_buff skb;
};

#define ip_hdr(skb)                                                           \
    0;                                                                        \
    struct iphdr iph_ = {.saddr = skb->src.s_addr, .daddr = skb->dst.s_addr}; \
    iph = &iph_

void nf_reinject_(struct nf_queue_entry *entry, unsigned int verdict)
{
    struct TestVerdict4Payload payload;
    payload.conn = *entry->skb;

    switch (verdict)
    {
    case NF_DROP:
        payload.allowed = false;
        break;
    case NF_ACCEPT:
        payload.allowed = true;
        break;
    }

    enqueue_TestVerdict4(&payload);
    debug_incr_verdict_responses();

    kfree(container_of(entry, struct queue_entry_and_skb, entry));
}
#define nf_reinject nf_reinject_

struct nf_conn_
{
    int dummy;
};
#define nf_conn nf_conn_

struct nf_conn *nf_ct_get_(struct Connect4Payload *skb, enum ip_conntrack_info *ctinfo)
{
    *ctinfo = IP_CT_NEW;
    return (struct nf_conn *)1;
}
#define nf_ct_get nf_ct_get_

struct nf_hook_state_
{
    int dummy;
};
#define nf_hook_state nf_hook_state_

static unsigned int nethook(void *priv,
                            struct sk_buff *skb,
                            const struct nf_hook_state *state);

static int queue_hook(struct nf_queue_entry *entry, unsigned int queuenum);

void on_TestPacket4(struct TestPacket4Payload *payloads, int n)
{
    for (int i = 0; i < n; i++)
    {
        struct queue_entry_and_skb *chunk = kmalloc(sizeof(struct queue_entry_and_skb), GFP_KERNEL);
        chunk->skb = payloads[i].conn;
        chunk->entry.skb = &chunk->skb;

        unsigned int verdict = nethook(0, &chunk->skb, 0);
        if (verdict == NF_DROP || verdict == NF_ACCEPT)
        {
            nf_reinject(&chunk->entry, verdict);
        }
        else
        {
            queue_hook(&chunk->entry, (verdict >> 16) & 0xffff);
        }
    }
}

struct nf_hook_ops_
{
    unsigned int (*hook)(void *, struct sk_buff *, const struct nf_hook_state *);
    int hooknum;
    int pf;
    int priority;
    struct net_device *dev;
};

#define nf_hook_ops nf_hook_ops_

#define NF_IP_PRI_MANGLE 1

struct nf_queue_handler_
{
    int (*outfn)(struct nf_queue_entry_ *, unsigned int);
    void *nf_hook_drop;
};
#define nf_queue_handler nf_queue_handler_

#define dev_get_by_name(...) 0
#define nf_register_net_hook(...)
#define nf_register_queue_handler(...)
#define nf_unregister_net_hook(...)
#define nf_unregister_queue_handler(...)

#endif