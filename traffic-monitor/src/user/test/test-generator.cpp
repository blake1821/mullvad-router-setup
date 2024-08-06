#include "test-generator.h"
#include "../util.h"
#include <math.h>

template<typename A>
A generate_random_ip();

template<>
IPv4Address generate_random_ip()
{
    struct in_addr ipAddr;
    ipAddr.s_addr = rand() % 0xFFFFFFFF;
    return ipAddr;
}

template <>
IPv6Address generate_random_ip()
{
    struct in6_addr ipAddr;
    for (int i = 0; i < 16; i++)
    {
        ipAddr.s6_addr[i] = rand() % 256;
    }
    return ipAddr;
}


inline bool generate_random_status()
{
    return (rand() % 2 == 0);
}

template<IPVersion V>
void TestGenerator::initialize(int src_count, int dst_count, int rule_count)
{
    vector<typename IPProps<V>::Address> src_addrs;
    vector<typename IPProps<V>::Address> dst_addrs;
    // fill up the vectors
    for (int i = 0; i < src_count; i++)
    {
        src_addrs.push_back(generate_random_ip<typename IPProps<V>::Address>());
    }
    for (int i = 0; i < dst_count; i++)
    {
        dst_addrs.push_back(generate_random_ip<typename IPProps<V>::Address>());
    }

    for (int i = 0; i < rule_count; i++)
    {
        // Randomly select a source and destination
        IPRule rule = IPRuleConcrete<V>(
            src_addrs[rand() % src_count],
            dst_addrs[rand() % dst_count],
            generate_random_status());
        
        uint64_t key = rule.base().get_key();
        if(ip_status.find(key) == ip_status.end()){
            ip_status[key] = rule.base().is_allowed();
            rules.push_back(rule);
        }

    }
}


TestGenerator::TestGenerator(IPVersion version, int src_count, int dst_count, int rule_count, TestMode mode) : mode(mode)
{
    if(version == IPv4)
    {
        initialize<IPv4>(src_count, dst_count, rule_count);
    }
    else
    {
        initialize<IPv6>(src_count, dst_count, rule_count);
    }
}

IPRule TestGenerator::next()
{
    if (mode == TestMode::RoundRobin)
    {
        IPRule rule = rules[rule_index++];
        if (rule_index == rules.size())
        {
            rule_index = 0;
        }
        return rule;
    }
    else
    {
        return rules[rand() % rules.size()];
    }

}

bool TestGenerator::is_allowed(IPQuery &query)
{
    cache_misses++;
    return ip_status[query.base().get_key()];
}
