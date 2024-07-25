#include "ipaddress.h"

class IPRuleBase
{
public:
    // delete the copy constructor and assignment operators
    IPRuleBase& operator=(const IPRuleBase& other) = delete;

    virtual IPAddressBase *get_src() const = 0;
    virtual IPAddressBase *get_dst() const = 0;
    virtual bool is_allowed() const = 0;

    string to_string() const
    {
        return get_src()->to_string() + " -> " + get_dst()->to_string() + " : " + (is_allowed() ? "Allowed" : "Blocked");
    }

    virtual bool modify(){

    }
};

template <typename T>
class IPRuleConcrete : public IPRuleBase
{
private:
    T src;
    T dst;
    bool allowed;

public:
    IPRuleConcrete(T src, T dst, bool allowed) : src(src), dst(dst), allowed(allowed) {}

    inline bool matches(T src_, T dst_)
    {
        return src == src_ && dst == dst_;
    }

    inline bool matches(IPRuleConcrete<T> &other)
    {
        return matches(other.src, other.dst);
    }

    inline uint64_t get_key()
    {
        return (src.get_key() << 32) ^ dst.get_key();
    }

    inline IPAddressBase *get_src() const override
    {
        return &src;
    }   

    inline IPAddressBase *get_dst() const override
    {
        return &dst;
    }

    inline bool is_allowed() const override
    {
        return allowed;
    }
};

typedef IPRuleConcrete<IPv4Address> IPv4Rule;
typedef IPRuleConcrete<IPv6Address> IPv6Rule;
#define IPRule BASED_UNION(IPRuleBase, IPv4Rule, IPv6Rule)
DECLARE_UNION(IPRule);

