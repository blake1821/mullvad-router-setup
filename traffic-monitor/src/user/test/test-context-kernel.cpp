#include "test-context.h"
#include "../reading-thread.h"
#include "../util.h"
#ifdef TEST_NETHOOKS

using namespace std;

class MessageAdapter : PayloadHandler<Query4>, PayloadHandler<Query6>, PayloadHandler<TestVerdict4>, PayloadHandler<TestVerdict6>
{
private:
    VirtualMessageHandler *handler;

public:
    void adapt(VirtualMessageHandler *handler)
    {
        this->handler = handler;
    }

    void handle(Query4Payload *payloads, int count) override
    {
        for (int i = 0; i < count; i++)
        {
            IPQuery query = IPv4Query(payloads[i].src, payloads[i].dst);
            handler->handle_query(query);
        }
    }

    void handle(Query6Payload *payloads, int count) override
    {
        for (int i = 0; i < count; i++)
        {
            IPQuery query = IPv6Query(payloads[i].src, payloads[i].dst);
            handler->handle_query(query);
        }
    }

    void handle(TestVerdict4Payload *payloads, int count) override
    {
        for (int i = 0; i < count; i++)
        {
            Verdict verdict = Verdict(
                IPv4Connection(payloads[i].conn.src, payloads[i].conn.dst, payloads[i].conn.dst_port, payloads[i].conn.protocol),
                payloads[i].allowed);
            handler->handle_verdict(verdict);
        }
    }

    void handle(TestVerdict6Payload *payloads, int count) override
    {
        for (int i = 0; i < count; i++)
        {
            Verdict verdict = Verdict(
                IPv6Connection(payloads[i].conn.src, payloads[i].conn.dst, payloads[i].conn.dst_port, payloads[i].conn.protocol),
                payloads[i].allowed);
            handler->handle_verdict(verdict);
        }
    }
};

class KernelTestContext : public TestContext
{
private:
    Trafficmon &trafficmon;
    ReadingThread<Query4> read_queries4_thread;
    ReadingThread<TestVerdict4> read_verdicts4_thread;
    ReadingThread<Query6> read_queries6_thread;
    ReadingThread<TestVerdict6> read_verdicts6_thread;
    MessageAdapter adapter;

public:
    KernelTestContext(Trafficmon &trafficmon)
        : trafficmon(trafficmon) {}

    void send_packet(Connection &conn) override
    {
        match(Connection, conn, v)
        (IPv4Connection, {
            struct TestPacket4Payload packet = {
                .conn = v.to_connect_payload(),
            };
            trafficmon.write_message<TestPacket4>(packet);
        }),
        (IPv6Connection, {
            struct TestPacket6Payload packet = {
                .conn = v.to_connect_payload(),
            };
            trafficmon.write_message<TestPacket6>(packet);
        }));
    }

    void send_status(IPRule &rule) override
    {
        match(IPRule, rule, v)
        (IPv4Rule, {
            struct SetStatus4Payload payload = v.to_set_status_payload();
            trafficmon.write_message<SetStatus4>(payload);
        }),
        (IPv6Rule, {
            struct SetStatus6Payload payload = v.to_set_status_payload();
            trafficmon.write_message<SetStatus6>(payload);
        }));
    }

    void start_reading(VirtualMessageHandler *handler) override
    {
        adapter.adapt(handler);
        read_verdicts4_thread.start((PayloadHandler<TestVerdict4> *)&adapter, trafficmon);
        read_queries4_thread.start((PayloadHandler<Query4> *)&adapter, trafficmon);
        read_verdicts6_thread.start((PayloadHandler<TestVerdict6> *)&adapter, trafficmon);
        read_queries6_thread.start((PayloadHandler<Query6> *)&adapter, trafficmon);
    }

    void stop_reading() override
    {
        // send a signal to the threads to stop
        read_queries4_thread.kill();
        read_verdicts4_thread.kill();
        read_queries6_thread.kill();
        read_verdicts6_thread.kill();

        // memory barrier to ensure cache_misses is updated
        read_queries4_thread.join();
        read_verdicts4_thread.join();
        read_queries6_thread.join();
        read_verdicts6_thread.join();
    }

    void finish_batch() override
    {
    }

    void debug() override
    {
        DebugRequestPayload req = {.avoid_locking = true};
        DebugResponsePayload debug_info = trafficmon.debug(req);
        print_debug_response(debug_info);
    }
};

class DummyHandler : VirtualMessageHandler, PayloadHandler<Connect4>, PayloadHandler<Connect6>
{
public:
    void handle_query(IPQuery &query) override
    {
    }

    void handle_verdict(Verdict &verdict) override
    {
    }

    void handle(Connect4Payload *payload, int n) override
    {
    }

    void handle(Connect6Payload *payload, int n) override
    {
    }
};

ReadingThread<Connect4> reader4_thread;
ReadingThread<Connect6> reader6_thread;
DummyHandler dummy;

TestContext *new_kernel_test_context(Trafficmon &trafficmon)
{
    KernelTestContext *context = new KernelTestContext(trafficmon);

    // clear out the buffers
    context->start_reading((VirtualMessageHandler *)&dummy);
    reader4_thread.start((PayloadHandler<Connect4> *)&dummy, trafficmon);
    reader6_thread.start((PayloadHandler<Connect6> *)&dummy, trafficmon);
    this_thread::sleep_for(chrono::milliseconds(1000));
    context->stop_reading();

    return context;
}

#endif