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
    struct iphdr *iph = ip_hdr(entry->skb);
    bool enqueued = enqueue_ipv4(
        &entry->list,
        queuenum,
        (struct in_addr){.s_addr = iph->saddr},
        (struct in_addr){.s_addr = iph->daddr});
    
    if(enqueued)
    {
        return 0;
    }
    else
    {
        nf_reinject(entry, NF_DROP);
        return 0;
        // clean this up if it works
    }
}

void dispatch_queue4(struct list_head* head, IPStatus status__possibly_pending){
    unsigned int verdict;
    switch(status__possibly_pending){
        case Pending:
        case Blocked:
            verdict = NF_DROP;
            break;
        case Allowed:
            verdict = NF_ACCEPT;
            break;
    }
    while(head != NULL){
        struct nf_queue_entry *entry = container_of(head, struct nf_queue_entry, list);
        struct iphdr *iph = ip_hdr(entry->skb);
        nf_reinject(entry, verdict);
        head = head->next;
    }
}

static unsigned int hook4_func(void *priv,
                               struct sk_buff *skb,
                               const struct nf_hook_state *state)
{
    enum ip_conntrack_info ctinfo;
    struct nf_conn *ct = nf_ct_get(skb, &ctinfo);
    if (ct != NULL)
    {
        if (ctinfo == IP_CT_NEW)
        {
            struct iphdr *iph = ip_hdr(skb);
            struct Connect4Payload conn_payload = {
                .dst = {
                    .s_addr = iph->daddr},
                .src = {.s_addr = iph->saddr}};
            enqueue_Connect4(&conn_payload);

            uint16_t queue_no;
            IPStatus status = get_ipv4_status(conn_payload.src, conn_payload.dst, &queue_no);
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
static struct nf_hook_ops nfho = {
    .hook = hook4_func,
    .hooknum = NF_INET_POST_ROUTING,
    .pf = PF_INET,
    .priority = NF_IP_PRI_MANGLE,
};

static struct nf_queue_handler nfqh = {
    .outfn = queue_hook,
    .nf_hook_drop = NULL // ???
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
        struct net_device *dev = dev_get_by_name(&init_net, payloads[0].outgoing_dev_name);
        nfho.dev = dev;
        nf_register_net_hook(&init_net, &nfho);
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
        nf_unregister_net_hook(&init_net, &nfho);
        nf_unregister_queue_handler(); /* ??? */
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
        nf_unregister_net_hook(&init_net, &nfho);
        nf_unregister_queue_handler();
        hooks_enabled = false;
    }
}