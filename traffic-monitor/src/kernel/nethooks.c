#include "nethooks.h"
#include "procfile.h"
#include "debug.h"
#include "../common/iplookup.h"

#ifdef TEST_NETHOOKS
#include "mock-nf.h"
#else
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter/nfnetlink.h>
#include <net/netfilter/nf_queue.h>
#include <net/netfilter/nf_conntrack.h>
#endif

static int queue_hook(struct nf_queue_entry *entry, unsigned int queuenum)
{
    bool enqueued = false;
    if (entry->skb->protocol == htons(ETH_P_IP))
    {
        struct iphdr *iph = ip_hdr(entry->skb);
        enqueued = ipv4_enqueue_packet(
            &entry->list,
            queuenum,
            (struct in_addr){.s_addr = iph->saddr},
            (struct in_addr){.s_addr = iph->daddr});
    }
    else if (entry->skb->protocol == htons(ETH_P_IPV6))
    {
        struct ipv6hdr *ip6h = ipv6_hdr(entry->skb);
        enqueued = ipv6_enqueue_packet(
            &entry->list,
            queuenum,
            ip6h->addrs.saddr,
            ip6h->addrs.daddr);
    }

    if (enqueued)
    {
        return 0;
    }
    else
    {
        debug_incr_enqueue_failures();
        nf_reinject(entry, NF_DROP);
        return 0;
    }
}

#define _DEFINE_DISPATCH_QUEUE(v)                                                           \
    void __DISPATCH_QUEUE(v)(struct list_head * head, IPStatus status)                      \
    {                                                                                       \
        unsigned int verdict;                                                               \
        DBG(bool pending = false);                                                          \
        switch (status)                                                                     \
        {                                                                                   \
        case Pending:                                                                       \
            DBG(pending = true);                                                            \
        case Blocked:                                                                       \
            verdict = NF_DROP;                                                              \
            break;                                                                          \
        case Allowed:                                                                       \
            verdict = NF_ACCEPT;                                                            \
            break;                                                                          \
        }                                                                                   \
        while (head != NULL)                                                                \
        {                                                                                   \
            DBG(if (pending) debug_incr_overflow_packets());                                \
            struct nf_queue_entry *entry = container_of(head, struct nf_queue_entry, list); \
            nf_reinject(entry, verdict);                                                    \
            head = head->next;                                                              \
        }                                                                                   \
    }
APPLY(_DEFINE_DISPATCH_QUEUE, IP_VERSIONS)
#undef _DEFINE_DISPATCH_QUEUE

static inline void translate_protocol(struct sk_buff *skb, uint8_t proto_in, ConnectProtocol *proto_out, uint16_t *port_out)
{
    union
    {
        struct tcphdr *tcph;
        struct udphdr *udph;
    } u;

    switch (proto_in)
    {
    case IPPROTO_TCP:
        u.tcph = tcp_hdr(skb);
        *proto_out = ProtoTCP;
        *port_out = ntohs(u.tcph->dest);
        break;
    case IPPROTO_UDP:
        u.udph = udp_hdr(skb);
        *proto_out = ProtoUDP;
        *port_out = ntohs(u.udph->dest);
        break;
    default:
        *proto_out = ProtoOther;
        *port_out = 0;
        return;
    }
}

static unsigned int nethook(void *priv,
                            struct sk_buff *skb,
                            const struct nf_hook_state *state)
{
    enum ip_conntrack_info ctinfo;
    struct nf_conn *ct = nf_ct_get(skb, &ctinfo);
    if (ct != NULL)
    {
        if (ctinfo == IP_CT_NEW && ct->mark != PRIVILEGED_MARK)
        {
            uint16_t queue_no;
            IPStatus status = -1;
            if (skb->protocol == htons(ETH_P_IP))
            {
                struct iphdr *iph = ip_hdr(skb);
                struct Connect4Payload conn_payload = {
                    .dst = {.s_addr = iph->daddr},
                    .src = {.s_addr = iph->saddr},
                };
                translate_protocol(skb, iph->protocol, &conn_payload.protocol, &conn_payload.dst_port);
                enqueue_Connect4(&conn_payload);

                status = __GET_IPSTATUS(4)(conn_payload.src, conn_payload.dst, &queue_no);
            }
            else if (skb->protocol == htons(ETH_P_IPV6))
            {
                struct ipv6hdr *ip6h = ipv6_hdr(skb);
                struct Connect6Payload conn_payload = {
                    .dst = ip6h->addrs.daddr,
                    .src = ip6h->addrs.saddr,
                };
                translate_protocol(skb, ip6h->nexthdr, &conn_payload.protocol, &conn_payload.dst_port);
                enqueue_Connect6(&conn_payload);

                status = __GET_IPSTATUS(6)(conn_payload.src, conn_payload.dst, &queue_no);
            }
            switch (status)
            {
            case Pending:
                return NF_QUEUE_NR(queue_no);
            case Blocked:
                return NF_DROP;
            case Allowed:
                break;
            }
        }
    }
    return NF_ACCEPT;
}

bool hooks_enabled = false;
static struct nf_hook_ops ipv4_nfho = {
    .hook = nethook,
    .hooknum = NF_INET_POST_ROUTING,
    .pf = PF_INET,
    .priority = NF_IP_PRI_MANGLE,
};
static struct nf_hook_ops ipv6_nfho = {
    .hook = nethook,
    .hooknum = NF_INET_POST_ROUTING,
    .pf = PF_INET6,
    .priority = NF_IP_PRI_MANGLE,
};

static void hook_drop(struct net *net)
{
    // do nothing??
}

static struct nf_queue_handler nfqh = {
    .outfn = queue_hook,
    .nf_hook_drop = hook_drop // ???
};

void on_SetNfEnabled(struct SetNfEnabledPayload *payloads, int count)
{
    if (count != 1)
    {
        my_debug("on_SetNfEnabled: invalid count %d\n", count);
        return;
    }
    if (payloads[0].enabled)
    {
        if (hooks_enabled)
        {
            my_debug("on_SetNfEnabled: hooks already enabled\n");
            return;
        }
        nf_register_net_hook(&init_net, &ipv4_nfho);
        nf_register_net_hook(&init_net, &ipv6_nfho);
        nf_register_queue_handler(&nfqh);
        hooks_enabled = true;
    }
    else
    {
        if (!hooks_enabled)
        {
            my_debug("on_SetNfEnabled: hooks already disabled\n");
            return;
        }
        nf_unregister_net_hook(&init_net, &ipv4_nfho);
        nf_unregister_net_hook(&init_net, &ipv6_nfho);
        nf_unregister_queue_handler();
        hooks_enabled = false;
    }
}

void init_nethooks(void)
{
}

void exit_nethooks(void)
{
    if (hooks_enabled)
    {
        nf_unregister_net_hook(&init_net, &ipv4_nfho);
        nf_unregister_net_hook(&init_net, &ipv6_nfho);
        nf_unregister_queue_handler();
        hooks_enabled = false;
    }
}