#pragma once
#include "ipprops.h"

class IPPair
{
public:
    virtual const IPAddressBase *get_src() const = 0;
    virtual const IPAddressBase *get_dst() const = 0;
    virtual uint64_t get_key() const = 0;
};

template <IPVersion V>
class IPPairConcrete
{
public:
    using T = typename IPProps<V>::Address;
    const T src;
    const T dst;

    IPPairConcrete(T src, T dst) : src(src), dst(dst) {}
};

// put this in all classes that inherit from IPPairConcrete
#define IMPLEMENT_IPPAIR                                              \
    const IPAddressBase *get_src() const override                     \
    {                                                                 \
        return &(IPPairConcrete<V>::src);                             \
    }                                                                 \
    const IPAddressBase *get_dst() const override                     \
    {                                                                 \
        return &(IPPairConcrete<V>::dst);                             \
    }                                                                 \
    inline uint64_t get_key() const override                          \
    {                                                                 \
        return (((uint64_t)IPPairConcrete<V>::src.get_key()) << 32) ^ \
               IPPairConcrete<V>::dst.get_key();                      \
    }

class IPPairHashFunction
{
public:
    size_t operator()(const IPPair &pair) const
    {
        return pair.get_key();
    }
};
