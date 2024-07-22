#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <signal.h>
#include "trafficmon.h"

static thread_local bool done;

static void handle_signal(int signo)
{
    done = true;
}

// A class that reads messages of a certain type from the traffic monitor
// Do not use 2 instances of this class for the same message type
template <ReadMessageType T>
class ReadingThread
{
private:
    thread *thr;
    PayloadHandler<T> *handler;
    Trafficmon *trafficmon;

    void read_loop()
    {
        ::done = false;
        typename ReadProps<T>::Payload read_messages[ReadProps<T>::MaxPayloadCount];

        while (!::done)
        {
            // bug: if a signal is caught before we enter the read syscall, we will hang forever
            // luckily, this class is only used for testing
            int count = trafficmon->read_messages<T>(read_messages);
            if(count < 0)
                break;
            handler->handle(read_messages, count);
        }
    }

public:
    ReadingThread() {}

    void start(PayloadHandler<T> *handler, Trafficmon &trafficmon)
    {
        signal(SIGUSR1, handle_signal);
        this->handler = handler;
        this->trafficmon = &trafficmon;
        thr = new thread(&ReadingThread<T>::read_loop, this);
    }

    void kill()
    {
        pthread_kill(thr->native_handle(), SIGUSR1);
    }

    void join()
    {
        thr->join();
        delete thr;
    }
};
