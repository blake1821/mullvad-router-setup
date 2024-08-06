#pragma once
#include "ippair.h"

class BaseQuery : public IPPair
{
};

template <IPVersion V>
class ConcreteQuery : public IPPairConcrete<V>, public BaseQuery
{
public:
    using T = typename IPProps<V>::Address;
    ConcreteQuery(T src, T dst) : BaseQuery(), IPPairConcrete<V>(src, dst) {}

    IMPLEMENT_IPPAIR

    bool operator==(const BaseQuery &other) const
    {
        return IPPairConcrete<V>::src == ((const ConcreteQuery<V> &)other).src &&
               IPPairConcrete<V>::dst == ((const ConcreteQuery<V> &)other).dst;
    }
};

typedef ConcreteQuery<IPv4> IPv4Query;
typedef ConcreteQuery<IPv6> IPv6Query;

#define IPQuery BASED_UNION(BaseQuery, IPv4Query, IPv6Query)
DECLARE_UNION(IPQuery);
