#include "../trafficmon.h"
#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <signal.h>
#include "../util.h"
#include "test-generator.h"
#include "test-context.h"
#include <set>
extern "C"
{
#include "../../common/debug.h"
}
#include "../reading-thread.h"

#define KERNEL_TEST 1

using namespace std;

// Return number of cache misses
class TestSession : VirtualMessageHandler
{
private:
    TestContext *context;
    TestGenerator *generator;
    map<uint64_t, pair<IPStatus, bool>> conn_map;
    mutex conn_map_mutex;
    int sent_count;
    int recv_count;

public:
    void handle_query(Query4Payload &query) override
    {
        IPStatus status = generator->query(query);
        SetStatus4Payload payload = {
            .src = query.src,
            .dst = query.dst,
            .status = status};
        context->send_status(payload);
    }

    void handle_verdict(Connect4Payload &conn, IPStatus status) override
    {
        uint64_t key = get_conn_key(conn);
        conn_map_mutex.lock();
        auto it = conn_map.find(key);
        if (it != conn_map.end())
        {
            assert(it->second.first == status || status != Allowed);
            it->second.second = true;
            recv_count++;
        }else{
            assert(status != Allowed);
        }
        conn_map_mutex.unlock();
    }

    TestSession(TestContext *context, TestGenerator *generator) : context(context), generator(generator) {}

    void try_wait()
    {
        conn_map_mutex.lock();
        int last_count = -1;
        while (sent_count != recv_count && last_count != recv_count)
        {
            last_count = recv_count;
            conn_map_mutex.unlock();
            this_thread::sleep_for(chrono::milliseconds(100));
            conn_map_mutex.lock();
        }
        conn_map_mutex.unlock();
    }

    int run_test(
        int max_batch_size,
        int num_rounds)
    {
        conn_map.clear();
        sent_count = 0;
        recv_count = 0;
        context->start_reading(this);
        int rounds_left = num_rounds;
        while (rounds_left > 0)
        {
            int batch_size = min((rand() % max_batch_size) + 1, rounds_left);
            for (int i = 0; i < batch_size; i++)
            {
                Connect4Payload conn;
                SetStatus4Payload payload = generator->next();
                conn.dst = payload.dst;
                conn.dst_port = rand() % 65536;
                conn.protocol = ProtoTcp;
                conn.src = payload.src;
                conn_map_mutex.lock();
                conn_map[get_conn_key(conn)] = {payload.status, false};
                conn_map_mutex.unlock();
                _GLIBCXX_WRITE_MEM_BARRIER;
                context->send_packet(conn);
                sent_count++;
            }
            context->finish_batch();
            rounds_left -= batch_size;
        }
        try_wait();
        context->stop_reading();
        conn_map_mutex.lock();
        if (sent_count != recv_count)
        {
            cout << "!!!!" << endl;
            cout << "Mismatch between sent and received packets" << endl;
            cout << "send count: " << sent_count << ", recv count: " << recv_count << endl;
            context->debug();
        }
        for (auto [k, v] : conn_map)
        {
            assert(v.second);
        }
        conn_map_mutex.unlock();
        return generator->get_cache_misses();
    }
};

void run_test(
    TestContext *context,
    int test_session_count,
    int src_count,
    int dst_count,
    int rule_count,
    TestMode mode,
    int max_batch_size,
    int num_rounds,
    int expected_cache_misses)
{
    cout << "Running " << test_session_count << "x tests with \n\t"
         << src_count << " src ips, \n\t" << dst_count << " dst ips, \n\t"
         << rule_count << " rules, \n\tmode = " << (int)mode << ", \n\t"
         << max_batch_size << " >= rounds per batch, \n\t" << num_rounds << " rounds"
         << endl;
    for (int i = 0; i < test_session_count; i++)
    {
        TestGenerator generator(src_count, dst_count, rule_count, mode);
        TestSession session(context, &generator);
        int cache_misses = session.run_test(max_batch_size, num_rounds);
        cout << "Cache misses: " << cache_misses << ". (Expected " << expected_cache_misses << ")" << endl;
    }
}

int main()
{
    srand(time(NULL));
    init_debug(true);

    auto start_time = chrono::system_clock::now();

#if KERNEL_TEST
    Trafficmon trafficmon;
    TestContext *context = new_kernel_test_context(trafficmon);

#else
    TestContext *context = new_user_test_context();
#endif

    // theres a bug that happens for this test.
    run_test(
        context,          // TestContext
        1,                // test_session_count
        1,                // src_count
        1,                // dst_count
        1,                // rule_count
        TestMode::Random, // mode
        1,                // max_batch_size
        1,                // num_rounds
        1                 // expected_cache_misses
    );

    /* These take too long. Let's delete them for now.
    run_test(
        context,          // TestContext
        10,               // test_session_count
        1,                // src_count
        100,              // dst_count
        10,               // rule_count
        TestMode::Random, // mode
        5,                // max_batch_size
        1000,             // num_rounds
        10                // expected_cache_misses
    );
    */

    run_test(
        context,          // TestContext
        2,                // test_session_count
        3,                // src_count
        100,              // dst_count
        100,              // rule_count
        TestMode::Random, // mode
        15,               // max_batch_size
        10000,            // num_rounds
        200               // expected_cache_misses
    );

    run_test(
        context,              // TestContext
        2,                    // test_session_count
        4,                    // src_count
        1000,                 // dst_count
        1000,                 // rule_count
        TestMode::RoundRobin, // mode
        50,                   // max_batch_size
        100000,               // num_rounds
        100000                // expected_cache_misses
    );

    run_test(
        context,              // TestContext
        1,                    // test_session_count
        100,                  // src_count
        1000,                 // dst_count
        1000,                 // rule_count
        TestMode::RoundRobin, // mode
        1000,                 // max_batch_size
        1000000,              // num_rounds
        1000000               // expected_cache_misses
    );

    run_test(
        context,          // TestContext
        1,                // test_session_count
        100,              // src_count
        1000,             // dst_count
        580,              // rule_count
        TestMode::Random, // mode
        100,              // max_batch_size
        100000,           // num_rounds
        10000             // expected_cache_misses
    );

    auto end_time = chrono::system_clock::now();
    auto ellapsed_time = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
    cout << "Finished in " << ellapsed_time << "ms" << endl;

    delete context;
    return 0;
}