#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
using namespace std;

class IPAddressBase
{
public:
    virtual uint32_t get_key() const = 0;
    virtual string to_string() const = 0;
    virtual bool operator==(const IPAddressBase &other) const = 0;
};

class IPv4Address : public IPAddressBase
{
public:
    struct in_addr addr;

    inline IPv4Address(struct in_addr addr) : addr(addr) {}
    inline IPv4Address(string addr)
    {
        inet_pton(AF_INET, addr.c_str(), &this->addr);
    }
    inline uint32_t get_key() const override
    {
        return addr.s_addr;
    }
    inline string to_string() const override
    {
        return inet_ntoa(addr);
    }
    inline bool operator==(const IPAddressBase &other) const override
    {
        return addr.s_addr == ((IPv4Address &)other).addr.s_addr;
    }
};

class IPv6Address : public IPAddressBase
{
public:
    struct in6_addr addr;

    inline IPv6Address(struct in6_addr addr) : addr(addr) {}
    inline IPv6Address(string addr)
    {
        inet_pton(AF_INET6, addr.c_str(), &this->addr);
    }
    inline uint32_t get_key() const override
    {
        return addr.s6_addr32[0] ^ addr.s6_addr32[1] ^ addr.s6_addr32[2] ^ addr.s6_addr32[3];
    }
    inline string to_string() const override
    {
        char str[INET6_ADDRSTRLEN];
        return inet_ntop(AF_INET6, &addr, str, INET6_ADDRSTRLEN);
    }
    inline bool operator==(const IPAddressBase &other) const override
    {
        return memcmp(addr.s6_addr, ((IPv6Address &)other).addr.s6_addr, sizeof(addr.s6_addr)) == 0;
    }
};
