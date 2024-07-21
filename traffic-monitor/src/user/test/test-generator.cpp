#include "test-generator.h"
#include "../util.h"
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

TestGenerator::TestGenerator(int src_count, int dst_count, int rule_count, TestMode mode) : mode(mode)
{
    vector<struct in_addr> src_addrs;
    vector<struct in_addr> dst_addrs;
    // fill up the vectors
    for (int i = 0; i < src_count; i++)
    {
        src_addrs.push_back(generate_random_ipv4());
    }
    for (int i = 0; i < dst_count; i++)
    {
        dst_addrs.push_back(generate_random_ipv4());
    }

    for (int i = 0; i < rule_count; i++)
    {
        // Randomly select a source and destination
        struct SetStatus4Payload payload = {
            .src = src_addrs[rand() % src_count],
            .dst = dst_addrs[rand() % dst_count],
            .status = generate_random_status()};
        
        uint64_t key = get_ip_pair_key(payload.src, payload.dst);
        if(ip_status.find(key) == ip_status.end()){
            // place the payload in the map
            ip_status[key] = payload.status;

            rules.push_back(payload);
        }

    }
}

struct SetStatus4Payload TestGenerator::next()
{
    struct SetStatus4Payload payload;
    if (mode == TestMode::RoundRobin)
    {
        payload = rules[rule_index++];
        if (rule_index == rules.size())
        {
            rule_index = 0;
        }
    }
    else
    {
        payload = rules[rand() % rules.size()];
    }

    return payload;
}

IPStatus TestGenerator::query(struct Query4Payload payload)
{
    cache_misses++;
    return ip_status[get_ip_pair_key(payload.src, payload.dst)];
}
