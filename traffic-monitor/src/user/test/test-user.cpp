#include "test-user.h"

TestGenerator generator(0, TestMode::Random);

struct SetStatus4Payload query_ipv4(struct in_addr ipv4)
{
    return (struct SetStatus4Payload){
        .ipv4 = ipv4,
        .status = generator.query(ipv4)};
}

UserTestSession::UserTestSession(int ip_count, TestMode mode)
{
    generator = TestGenerator(ip_count, mode);
}

/** Run the tests. Return the # of cache misses  */
int UserTestSession::run(int num_rounds)
{
    for (int i = 0; i < num_rounds; i++)
    {
        auto [ipv4, status] = generator.next();
        if (get_ipv4_status(ipv4) != status)
        {
            throw runtime_error("Invalid status");
        }
    }
    return generator.get_cache_misses();
}