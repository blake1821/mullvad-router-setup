#include <queue>
#include <unordered_map>
#include <unordered_set>
#include "test-context.h"
#include "test-generator.h"
extern "C"
{
#include "../../common/iplookup.h"
}
#include "../util.h"

using namespace std;

class UserTestContext : public TestContext
{
private:
    queue<Connection> connection_queue;
    VirtualMessageHandler *handler;
    unordered_map<uint64_t, Connection> pending_connects;
    queue<IPQuery> pending_queries;
    unordered_set<uint64_t> rules_it_queried;
    unordered_set<uint64_t> expected_queries;
    int mallocs = 0;

public:
    void send_packet(Connection &connection) override
    {
        connection_queue.push(connection);
    }

    void send_status(IPRule &rule) override
    {
        rules_it_queried.insert(rule.base().get_key());
        match(IPRule, rule, v)
        (IPv4Rule, {
            SetStatus4Payload payload = v.to_set_status_payload();
            on_SetStatus4(&payload, 1);
        }),
        (IPv6Rule, {
            SetStatus6Payload payload = v.to_set_status_payload();
            on_SetStatus6(&payload, 1);
        }));
    }

    void start_reading(VirtualMessageHandler *handler) override
    {
        this->handler = handler;
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
                auto conn = connection_queue.front();
                connection_queue.pop();

                if (rules_it_queried.find(conn.base().get_key()) == rules_it_queried.end())
                {
                    expected_queries.insert(conn.base().get_key());
                }

                uint16_t queue_no;
                IPStatus status;
                match(Connection, conn, v)
                (IPv4Connection, {
                    status = ipv4_get_status(v.src.addr, v.dst.addr, &queue_no);
                }),
                (IPv6Connection, {
                    status = ipv6_get_status(v.src.addr, v.dst.addr, &queue_no);
                }));

                if (status == Pending)
                {
                    uint64_t key = conn.base().get_conn_key();
                    pending_connects.emplace(key, conn);

                    list_head *head = new list_head();
                    head->data = &pending_connects.at(key);
                    mallocs++;

                    match(Connection, conn, v)
                    (IPv4Connection, {
                        if (!ipv4_enqueue_packet(head, queue_no, v.src.addr, v.dst.addr))
                            throw runtime_error("Failed to enqueue ipv4");
                    }),
                    (IPv6Connection, {
                        if (!ipv6_enqueue_packet(head, queue_no, v.src.addr, v.dst.addr))
                            throw runtime_error("Failed to enqueue ipv6");
                    }));
                }
                else
                {
                    Verdict verdict(conn, status == Allowed);
                    handler->handle_verdict(verdict);
                }
            }

            // simulate the query handler
            int queries = min(rand() % 10, (int)pending_queries.size());
            for (int i = 0; i < queries; i++)
            {
                IPQuery query = pending_queries.front();
                pending_queries.pop();
                handler->handle_query(query);
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

    void enqueue_query(IPQuery &query)
    {
        pending_queries.push(query);

        // delete from expected queries, if it exists
        auto key = query.base().get_key();
        if (expected_queries.find(key) != expected_queries.end())
        {
            expected_queries.erase(key);
        }
        rules_it_queried.insert(key);
    }

    void dispatch_queue(IPVersion version, struct list_head *head, IPStatus status)
    {
        while (head != NULL)
        {
            Connection &conn = *(Connection *)head->data;
            uint64_t key = conn.base().get_conn_key();
            if (pending_connects.find(key) != pending_connects.end())
            {
                Verdict verdict(conn, status == Allowed);
                handler->handle_verdict(verdict);
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
};

UserTestContext *the_context;

TestContext *new_user_test_context()
{
    return the_context = new UserTestContext();
}

void enqueue_Query4(struct Query4Payload *payload)
{
    IPQuery query = IPv4Query(payload->src, payload->dst);
    the_context->enqueue_query(query);
}

void enqueue_Query6(struct Query6Payload *payload)
{
    IPQuery query = IPv6Query(payload->src, payload->dst);
    the_context->enqueue_query(query);
}

void ipv4_dispatch_queue(struct list_head *head, IPStatus status)
{
    the_context->dispatch_queue(IPv4, head, status);
}

void ipv6_dispatch_queue(struct list_head *head, IPStatus status)
{
    the_context->dispatch_queue(IPv6, head, status);
}