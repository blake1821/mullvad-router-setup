#include "test-context.h"
#include <queue>
#include <map>
#include <set>
#include "test-generator.h"
extern "C"
{
#include "../../common/iplookup.h"
}
#include "../util.h"

using namespace std;

static VirtualMessageHandler *handler;
map<uint64_t, Connect4Payload> pending_connects;
queue<Query4Payload> pending_queries;
set<uint64_t> rules_it_queried;
map<uint64_t, Query4Payload> expected_queries;
int mallocs = 0;

void enqueue_Query4(struct Query4Payload *payload)
{
    pending_queries.push(*payload);

    // delete from expected queries, if it exists
    auto key = get_ip_pair_key(payload->src, payload->dst);
    if (expected_queries.find(key) != expected_queries.end())
    {
        expected_queries.erase(key);
    }
    rules_it_queried.insert(key);
}

void dispatch_queue4(struct list_head *head, IPStatus status)
{
    while (head != NULL)
    {
        Connect4Payload *payload = (Connect4Payload *)head->data;
        uint64_t key = get_conn_key(*payload);
        if (pending_connects.find(key) != pending_connects.end())
        {
            handler->handle_verdict(*payload, status);
            pending_connects.erase(key);
        }
        else
        {
            throw runtime_error("Key not found in pending connects");
        }
        struct list_head *old = head;
        head = head->next;
        delete old;
        mallocs--;
    }
}

class UserTestContext : public TestContext
{
private:
    queue<Connect4Payload> connection_queue;

public:
    void send_packet(struct Connect4Payload &payload) override
    {
        connection_queue.push(payload);
    }

    void send_status(struct SetStatus4Payload &payload) override
    {
        rules_it_queried.insert(get_ip_pair_key(payload.src, payload.dst));
        on_SetStatus4(&payload, 1);
    }

    void start_reading(VirtualMessageHandler *handler) override
    {
        ::handler = handler;
    }

    void stop_reading() override
    {
        finish_batch();
    }

    void finish_batch() override
    {

        while (!connection_queue.empty() || !pending_queries.empty())
        {
            // simulate the packet hook
            int packets = min(rand() % 10, (int)connection_queue.size());
            for (int i = 0; i < packets; i++)
            {
                Connect4Payload conn = connection_queue.front();
                connection_queue.pop();

                if (rules_it_queried.find(get_ip_pair_key(conn.src, conn.dst)) == rules_it_queried.end())
                {
                    expected_queries[get_ip_pair_key(conn.src, conn.dst)] = Query4Payload{conn.src, conn.dst};
                }

                uint16_t queue_no;
                IPStatus status = get_ipv4_status(conn.src, conn.dst, &queue_no);

                if (status == Pending)
                {
                    uint64_t key = get_conn_key(conn);
                    pending_connects[key] = conn;

                    list_head *head = new list_head();
                    head->data = &pending_connects[key];
                    mallocs++;
                    if (!enqueue_ipv4(head, queue_no, conn.src, conn.dst))
                        throw runtime_error("Failed to enqueue ipv4");
                }
                else
                {
                    handler->handle_verdict(conn, status);
                }
            }

            // simulate the query handler
            int queries = min(rand() % 10, (int)pending_queries.size());
            for (int i = 0; i < queries; i++)
            {
                Query4Payload payload = pending_queries.front();
                pending_queries.pop();
                handler->handle_query(payload);
            }
        }

        assert(mallocs == 0);
        assert(expected_queries.empty());
        assert(pending_connects.empty());
        assert(pending_queries.empty());
    }

    void debug() override
    {
    }
};

TestContext *new_user_test_context()
{
    return new UserTestContext();
}