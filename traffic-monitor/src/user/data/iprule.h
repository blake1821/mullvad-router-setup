#pragma once
#include "ippair.h"

class IPRuleBase : public IPPair
{
public:
    virtual const bool is_allowed() const = 0;

    string to_string() const
    {
        return get_src()->to_string() + " -> " + get_dst()->to_string() + " : " + (is_allowed() ? "Allowed" : "Blocked");
    }
};

template <IPVersion V>
class IPRuleConcrete : public IPRuleBase, public IPPairConcrete<V>
{
public:
    bool allowed;
    using T = typename IPProps<V>::Address;

    IPRuleConcrete(T src, T dst, bool allowed) : IPPairConcrete<V>(src, dst), IPRuleBase(), allowed(allowed) {}

    IMPLEMENT_IPPAIR

    inline const bool is_allowed() const override
    {
        return allowed;
    }

    inline bool operator==(const IPRuleConcrete &other) const
    {
        return IPPairConcrete<V>::src == other.src && IPPairConcrete<V>::dst == other.dst && allowed == other.allowed;
    }

    inline IPProps<V>::SetStatusPayload to_set_status_payload() const
    {
        return typename IPProps<V>::SetStatusPayload{
            .src = IPPairConcrete<V>::src.addr,
            .dst = IPPairConcrete<V>::dst.addr,
            .status = allowed ? Allowed : Blocked,
        };
    }
};

typedef IPRuleConcrete<IPv4> IPv4Rule;
typedef IPRuleConcrete<IPv6> IPv6Rule;

#define IPRule BASED_UNION(IPRuleBase, IPv4Rule, IPv6Rule)
DECLARE_UNION(IPRule);
