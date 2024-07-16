#include "test-kernel.h"

Barrier finished_barrier(2);
bool done = false;
void graceful_signal_handler(int signo)
{
    done = true;
}

void exit_thread(int signo)
{
    finished_barrier.wait();
    pthread_exit(NULL);
}

void KernelTestSession::run_filter()
{
    queue<ReadMessage> read_messages;
    queue<SetStatus4Payload> write_messages;

    while (true)
    {
        signal(SIGINT, graceful_signal_handler);
        // bug: if a signal is caught right here, we will hang forever
        trafficmon.read_messages(read_messages);

        while (!read_messages.empty())
        {
            auto msg = read_messages.front();
            read_messages.pop();

            if (msg.type == ReadMessageType::Query4)
            {
                struct in_addr ipv4 = msg.payload.Query4.ipv4;

                write_messages.push(
                    {.ipv4 = ipv4,
                     .status = generator.query(ipv4)});
            }
            else
            {
                throw runtime_error("Unexpected message type");
            }
        }

        trafficmon.write_messages<WriteMessageType::SetStatus4>(write_messages);

        signal(SIGINT, exit_thread);
        if (done)
        {
            exit_thread(SIGINT);
        }
    }
}

KernelTestSession::KernelTestSession(int ip_count, TestMode mode) : generator(ip_count, mode) {}

/** Run the tests. Return the # of cache misses  */
int KernelTestSession::run(int num_rounds)
{
    signal(SIGINT, exit_thread);
    done = false;
    thread filter_thread = thread(&KernelTestSession::run_filter, this);

    for (int i = 0; i < num_rounds; i++)
    {
        tuple<struct in_addr, IPStatus> next = generator.next();
        IPStatus recvd_status = testnet.write_ip(get<0>(next));
        if (recvd_status != get<1>(next))
        {
            throw runtime_error("Unexpected status");
        }
    }

    // send a signal to the filter thread to stop
    pthread_kill(filter_thread.native_handle(), SIGINT);

    // memory barrier to ensure cache_misses is updated
    finished_barrier.wait();
    filter_thread.join();
    signal(SIGINT, SIG_DFL);

    return generator.get_cache_misses();
};