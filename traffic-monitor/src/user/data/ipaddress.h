#include "../../common/macro-utils.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
using namespace std;

class IPAddressBase
{
public:
    virtual uint32_t get_key() = 0;
    virtual string to_string() = 0;
    virtual bool operator==(const IPAddressBase &other) = 0;
};

class IPv4Address : public IPAddressBase
{
private:
    struct in_addr addr;

public:
    // delete the copy constructor and assignment operators
    IPv4Address& operator=(const IPv4Address& other) = delete;

    inline IPv4Address(struct in_addr addr) : addr(addr) {}
    inline uint32_t get_key() override
    {
        return addr.s_addr;
    }
    inline string to_string() override
    {
        return inet_ntoa(addr);
    }
    inline bool operator==(const IPAddressBase &other) override
    {
        return addr.s_addr == ((IPv4Address &)other).addr.s_addr;
    }
};

class IPv6Address : public IPAddressBase
{
private:
    struct in6_addr addr;

public:
    inline IPv6Address(struct in6_addr addr) : addr(addr) {}
    inline uint32_t get_key() override
    {
        return addr.s6_addr32[0] ^ addr.s6_addr32[1] ^ addr.s6_addr32[2] ^ addr.s6_addr32[3];
    }
    inline string to_string() override
    {
        char str[INET6_ADDRSTRLEN];
        return inet_ntop(AF_INET6, &addr, str, INET6_ADDRSTRLEN);
    }
    inline bool operator==(const IPAddressBase &other) override
    {
        return memcmp(addr.s6_addr, ((IPv6Address &)other).addr.s6_addr, sizeof(addr.s6_addr)) == 0;
    }
};
