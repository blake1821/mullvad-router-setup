#pragma once
#include "trafficmon.h"
#include "reading-thread.h"
#include <mutex>
#include <map>
#include <unordered_set>
#include "data/connection.h"
#include "data/iprule.h"

class FilterHandler
{
public:
    virtual void handle_verdict(Verdict &verdict) = 0;
};

template <IPVersion V>
class MonoIPFilter
{
private:
    unordered_set<IPRuleConcrete<V>, IPPairHashFunction> allowed;

public:
    using T = typename IPProps<V>::Address;
    void add_rule(IPRuleConcrete<V> &rule)
    {
        if (rule.is_allowed())
        {
            allowed.insert(rule);
        }
        else
        {
            IPRuleConcrete<V> rule2(rule.src, rule.dst, true);
            allowed.erase(rule);
        }
    }

    void clear_rules()
    {
        allowed.clear();
    }

    bool is_allowed(T src, T dst)
    {
        auto it = allowed.find(IPRuleConcrete<V>(src, dst, true));
        return it != allowed.end();
    }
};

#define IPFilterReadMessages PAIRAPPLYCOMMA(CONCAT, (IP_VERSIONS), Query, Connect)

#define _MAKE_PAYLOAD_HANDLER(T) \
protected                        \
    PayloadHandler<T>
class IPFilter : APPLYCOMMA(_MAKE_PAYLOAD_HANDLER, IPFilterReadMessages)
{
#undef _MAKE_PAYLOAD_HANDLER

private:
    Trafficmon trafficmon;

#define _DECLARE_IP_READER_THREAD(message) \
    ReadingThread<message> thr_##message;  \
    void handle(PAYLOAD_T(message) * payloads, int count);
    APPLY(_DECLARE_IP_READER_THREAD, IPFilterReadMessages)
#undef _DECLARE_IP_READER_THREAD

    MonoIPFilter<IPv4> filter4;
    MonoIPFilter<IPv6> filter6;
    mutex rules_mutex;
    FilterHandler *handler;
    string ifname;

protected:
    void set_enabled(bool enabled);

public:
    IPFilter(string ifname, FilterHandler *handler);
    void add_rules(vector<IPRule> &rules);
    void clear_rules();
    void kill();
    void join();
};