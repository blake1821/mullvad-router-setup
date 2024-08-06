#include "ipfilter.h"
#include "util.h"

#define _DECLARE_QUERY_HANDLER(v)                                             \
    void IPFilter::handle(Query##v##Payload *payloads, int count)             \
    {                                                                         \
        vector<SetStatus##v##Payload> responses;                              \
        rules_mutex.lock();                                                   \
        for (int i = 0; i < count; i++)                                       \
        {                                                                     \
            IPv##v##Address src(payloads[i].src);                             \
            IPv##v##Address dst(payloads[i].dst);                             \
            responses.push_back((SetStatus##v##Payload){                      \
                .src = payloads[i].src,                                       \
                .dst = payloads[i].dst,                                       \
                .status = filter##v.is_allowed(src, dst) ? Allowed : Blocked, \
            });                                                               \
        }                                                                     \
        rules_mutex.unlock();                                                 \
        trafficmon.write_messages<SetStatus##v>(responses);                   \
    }
APPLY(_DECLARE_QUERY_HANDLER, IP_VERSIONS)
#undef _DECLARE_QUERY_HANDLER

#define _CONNECT_HANDLER(v)                                              \
    void IPFilter::handle(Connect##v##Payload *payloads, int count)      \
    {                                                                    \
        vector<Verdict> verdicts;                                        \
        rules_mutex.lock();                                              \
        for (int i = 0; i < count; i++)                                  \
        {                                                                \
            IPv##v##Address src(payloads[i].src);                        \
            IPv##v##Address dst(payloads[i].dst);                        \
            verdicts.push_back(Verdict(IPv##v##Connection(               \
                                           src,                          \
                                           dst,                          \
                                           payloads[i].dst_port,         \
                                           payloads[i].protocol),        \
                                       filter##v.is_allowed(src, dst))); \
        }                                                                \
        rules_mutex.unlock();                                            \
        for (int i = 0; i < count; i++)                                  \
        {                                                                \
            Verdict verdict = verdicts[i];                               \
            handler->handle_verdict(verdict);                            \
        }                                                                \
    }
APPLY(_CONNECT_HANDLER, IP_VERSIONS)
#undef _CONNECT_HANDLER

void IPFilter::set_enabled(bool enabled)
{
    struct SetNfEnabledPayload payload = {
        .enabled = enabled,
    };
    strcpy(payload.outgoing_dev_name, ifname.c_str());
    trafficmon.write_message<SetNfEnabled>(payload);
}

IPFilter::IPFilter(string ifname, FilterHandler *handler)
    : handler(handler)
{
#define _START_READER_THREAD(message) \
    thr_##message.start(this, trafficmon);
    APPLY(_START_READER_THREAD, IPFilterReadMessages)
#undef _START_READER_THREAD
    clear_rules();
    set_enabled(true);
}

void IPFilter::add_rules(vector<IPRule> &rules)
{
    vector<SetStatus4Payload> responses4;
    vector<SetStatus6Payload> responses6;

    rules_mutex.lock();
    for (auto &rule : rules)
    {
        match(IPRule, rule, r)
        (IPv4Rule, {
            filter4.add_rule(r);
            responses4.push_back(r.to_set_status_payload());
        }),
        (IPv6Rule, {
            filter6.add_rule(r);
            responses6.push_back(r.to_set_status_payload());
        }));
    }
    rules_mutex.unlock();
    trafficmon.write_messages<SetStatus4>(responses4);
    trafficmon.write_messages<SetStatus6>(responses6);
}

void IPFilter::clear_rules()
{
    rules_mutex.lock();
    filter4.clear_rules();
    filter6.clear_rules();
    rules_mutex.unlock();
    ResetPayload reset_payload = {.reset = true};
    trafficmon.write_message<Reset>(reset_payload);
}

void IPFilter::kill()
{
    set_enabled(false);
#define _STOP_READER_THREAD(message) \
    thr_##message.kill();
    APPLY(_STOP_READER_THREAD, IPFilterReadMessages)
#undef _STOP_READER_THREAD
}

void IPFilter::join()
{
#define _STOP_READER_THREAD(message) \
    thr_##message.join();
    APPLY(_STOP_READER_THREAD, IPFilterReadMessages)
#undef _STOP_READER_THREAD
}