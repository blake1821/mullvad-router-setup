#pragma once
#include <vector>
#include <map>
#include "../../common/protocol.h"
#include "../data/iprule.h"
#include "../data/query.h"

using namespace std;

enum class TestMode
{
    RoundRobin,
    Random
};

class TestGenerator
{
private:
    vector<IPRule> rules;
    map<uint64_t, bool> ip_status;
    int cache_misses = 0;
    TestMode mode;
    int rule_index = 0;

    template<IPVersion V>
    void initialize(int src_count, int dst_count, int rule_count);

public:
    TestGenerator(IPVersion version, int src_count, int dst_count, int rule_count, TestMode mode);
    int get_cache_misses()
    {
        return cache_misses;
    }

    IPRule next();
    bool is_allowed(IPQuery &query);
};