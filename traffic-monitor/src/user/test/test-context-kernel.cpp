#include "test-context.h"
#include "../reading-thread.h"
#include "../util.h"
#ifdef TEST_NETHOOKS

using namespace std;

class MessageAdapter : QueryHandler, VerdictHandler
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
        for(int i = 0; i < count; i++)
            handler->handle_query(payloads[i]);
    }

    void handle(TestVerdict4Payload *payloads, int count) override
    {
        for(int i = 0; i < count; i++)
            handler->handle_verdict(payloads[i].conn, payloads[i].allowed ? Allowed : Blocked);
    }
};

class KernelTestContext : public TestContext
{
private:
    Trafficmon &trafficmon;
    ReadingThread<Query4> read_queries_thread;
    ReadingThread<TestVerdict4> read_verdicts_thread;
    MessageAdapter adapter;

public:
    KernelTestContext(Trafficmon &trafficmon)
        : trafficmon(trafficmon) {}

    void send_packet(struct Connect4Payload &payload) override
    {
        struct TestPacket4Payload packet = {
            .conn = payload,
        };
        trafficmon.write_message<TestPacket4>(packet);
    }

    void send_status(struct SetStatus4Payload &payload) override
    {
        trafficmon.write_message<SetStatus4>(payload);
    }

    void start_reading(VirtualMessageHandler *handler) override
    {
        adapter.adapt(handler);
        read_verdicts_thread.start((VerdictHandler *)&adapter, trafficmon);
        read_queries_thread.start((QueryHandler *)&adapter, trafficmon);
    }

    void stop_reading() override
    {
        // send a signal to the threads to stop
        read_queries_thread.kill();
        read_verdicts_thread.kill();

        // memory barrier to ensure cache_misses is updated
        read_queries_thread.join();
        read_verdicts_thread.join();
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

class DummyHandler : VirtualMessageHandler, PayloadHandler<Connect4>
{
public:
    void handle_query(Query4Payload &payload) override
    {
    }

    void handle_verdict(Connect4Payload &payload, IPStatus status) override
    {
    }

    void handle(Connect4Payload *payload, int n) override
    {
    }
};

ReadingThread<Connect4> reader_thread;
DummyHandler dummy;

TestContext *new_kernel_test_context(Trafficmon &trafficmon)
{
    KernelTestContext *context = new KernelTestContext(trafficmon);

    // clear out the buffers
    context->start_reading((VirtualMessageHandler *)&dummy);
    reader_thread.start((PayloadHandler<Connect4> *)&dummy, trafficmon);
    this_thread::sleep_for(chrono::milliseconds(1000));
    context->stop_reading();

    return context;
}

#endif