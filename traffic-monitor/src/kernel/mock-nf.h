#pragma once

#include "../common/protocol.h"
#include "debug.h"

// This file contains mock definitions for the nf_* functions that simulate
// packet processing with test packets and verdicts that are produced and
// consumed by the user space test harness.

#ifdef TEST_NETHOOKS

struct sk_buff_
{
    int protocol;
    union
    {
        struct Connect4Payload conn4;
        struct Connect6Payload conn6;
    };
};
#define sk_buff sk_buff_

struct iphdr_
{
    unsigned int saddr;
    unsigned int daddr;
    uint8_t protocol;
};
#define iphdr iphdr_

struct ipv6addrs_
{
    struct in6_addr saddr;
    struct in6_addr daddr;
};

struct ipv6hdr_
{
    struct ipv6addrs_ addrs;
    uint8_t nexthdr;
};
#define ipv6hdr ipv6hdr_

struct nf_queue_entry_
{
    struct list_head list;
    struct sk_buff *skb;
};
#define nf_queue_entry nf_queue_entry_

struct queue_entry_and_skb
{
    struct nf_queue_entry entry;
    struct sk_buff skb;
};

static inline uint8_t inv_translate_protocol(ConnectProtocol proto_in)
{
    switch (proto_in)
    {
    case ProtoTCP:
        return IPPROTO_TCP;
    case ProtoUDP:
        return IPPROTO_UDP;
    default:
        return 21; // just a random number
    }
}

struct iphdr iph_;

struct iphdr *ip_hdr(struct sk_buff *skb){
    iph_ = (struct iphdr){
        .saddr = skb->conn4.src.s_addr,
        .daddr = skb->conn4.dst.s_addr,
        .protocol = inv_translate_protocol(skb->conn4.protocol)
    };
    return &iph_;
}

struct ipv6hdr ip6h_;
struct ipv6hdr *ipv6_hdr(struct sk_buff *skb){
    ip6h_ = (struct ipv6hdr){
        .addrs = {
            .saddr = skb->conn6.src,
            .daddr = skb->conn6.dst
        },
        .nexthdr = inv_translate_protocol(skb->conn6.protocol)
    };
    return &ip6h_;
}

struct tcphdr
{
    uint16_t dest;
};
struct tcphdr tcp_hdr_;
struct tcphdr *tcp_hdr(struct sk_buff *skb)
{
    if(skb->protocol == htons(ETH_P_IP)){
        tcp_hdr_.dest = htons(skb->conn4.dst_port);
    }else{
        tcp_hdr_.dest = htons(skb->conn6.dst_port);
    }
    return &tcp_hdr_;
}

#define udphdr tcphdr
#define udp_hdr tcp_hdr

void nf_reinject_(struct nf_queue_entry *entry, unsigned int verdict)
{
    struct TestVerdict4Payload payload4;
    struct TestVerdict6Payload payload6;
    bool allowed = verdict == NF_ACCEPT;

    switch (ntohs(entry->skb->protocol))
    {
    case ETH_P_IP:
        payload4.allowed = allowed;
        payload4.conn = entry->skb->conn4;
        enqueue_TestVerdict4(&payload4);
        break;
    case ETH_P_IPV6:
        payload6.allowed = allowed;
        payload6.conn = entry->skb->conn6;
        enqueue_TestVerdict6(&payload6);
        break;
    default:
        my_debug("nf_reinject: unknown protocol\n");
    }

    debug_incr_verdict_responses();

    kfree(container_of(entry, struct queue_entry_and_skb, entry));
}
#define nf_reinject nf_reinject_

struct nf_conn_
{
    int dummy;
};
#define nf_conn nf_conn_

struct nf_conn *nf_ct_get_(struct sk_buff *skb, enum ip_conntrack_info *ctinfo)
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
        chunk->skb.protocol = htons(ETH_P_IP);
        chunk->skb.conn4 = payloads[i].conn;
        chunk->entry.skb = &chunk->skb;
        chunk->entry.list.next = NULL;

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

void on_TestPacket6(struct TestPacket6Payload *payloads, int n)
{
    for (int i = 0; i < n; i++)
    {
        struct queue_entry_and_skb *chunk = kmalloc(sizeof(struct queue_entry_and_skb), GFP_KERNEL);
        chunk->skb.protocol = htons(ETH_P_IPV6);
        chunk->skb.conn6 = payloads[i].conn;
        chunk->entry.skb = &chunk->skb;
        chunk->entry.list.next = NULL;

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