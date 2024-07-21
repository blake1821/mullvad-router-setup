#pragma once
#include <vector>
#include <map>
#include "../../common/protocol.h"

using namespace std;

enum class TestMode
{
    RoundRobin,
    Random
};

class TestGenerator
{
private:
    vector<struct SetStatus4Payload> rules;
    map<uint64_t, IPStatus> ip_status;
    int cache_misses = 0;
    TestMode mode;
    int rule_index = 0;

public:
    TestGenerator(int src_count, int dst_count, int rule_count, TestMode mode);
    int get_cache_misses()
    {
        return cache_misses;
    }

    struct SetStatus4Payload next();
    IPStatus query(struct Query4Payload payload);
};