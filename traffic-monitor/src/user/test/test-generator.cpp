#include "test-generator.h"
#include <math.h>

struct in_addr generate_random_ipv4()
{
    struct in_addr ipAddr;

    // Generate a random IP address. Each part of the IP can be a number between 0 and 255
    unsigned long addr = rand() % 0xFFFFFFFF;
    ipAddr.s_addr = htonl(addr); // Convert to network byte order

    return ipAddr;
}

IPStatus generate_random_status()
{
    return (rand() % 2 == 0) ? Allowed : Blocked;
}

TestGenerator::TestGenerator(int ip_count, TestMode mode) : mode(mode)
{
    for (int i = 0; i < ip_count; i++)
    {
        struct in_addr ipv4 = generate_random_ipv4();
        ip_addrs.push_back(ipv4);
        ip_status[ipv4.s_addr] = generate_random_status();
    }
}

tuple<struct in_addr, IPStatus> TestGenerator::next()
{
    struct in_addr ip;
    if (mode == TestMode::RoundRobin)
    {
        ip = ip_addrs[ip_index++];
        if (ip_index == ip_addrs.size())
        {
            ip_index = 0;
        }
    }
    else
    {
        ip = ip_addrs[rand() % ip_addrs.size()];
    }

    return tuple(ip, ip_status[ip.s_addr]);
}


IPStatus TestGenerator::query(struct in_addr ipv4){
    cache_misses++;
    return ip_status[ipv4.s_addr];
}

