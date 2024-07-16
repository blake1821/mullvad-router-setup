#pragma once
#include <vector>
#include <map>
#include "../../common/protocol.h"

using namespace std;

enum class TestMode{
    RoundRobin,
    Random
};

class TestGenerator {
private:
    int ip_count;
    vector<struct in_addr> ip_addrs;
    map<in_addr_t, IPStatus> ip_status;
    int cache_misses = 0;
    TestMode mode;
    int ip_index = 0;

public:
    TestGenerator(int ip_count, TestMode mode);
    int get_cache_misses(){
        return cache_misses;
    }

    tuple<struct in_addr, IPStatus> next();
    IPStatus query(struct in_addr ipv4);
};