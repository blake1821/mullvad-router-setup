#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <signal.h>
#include "trafficmon.h"

// global variables -- one for each message type that can be read
template <ReadMessageType>
struct ThreadProps;

template <>
struct ThreadProps<Query4>
{
    static const int Signo = SIGUSR1;
    static bool done;
};

#ifdef TEST_NETHOOKS
template <>
struct ThreadProps<TestVerdict4>
{
    static const int Signo = SIGUSR2;
    static bool done;
};
#endif

template <>
struct ThreadProps<Connect4>
{
    static const int Signo = SIGURG;
    static bool done;
};

// A class that reads messages of a certain type from the traffic monitor
// Do not use 2 instances of this class for the same message type
template <ReadMessageType T>
class ReadingThread
{
private:
    thread *thr;
    PayloadHandler<T> *handler;
    Trafficmon *trafficmon;

    static void exit_thread(int signo)
    {
        pthread_exit(NULL);
    }

    static void handle_signal(int signo)
    {
        ThreadProps<T>::done = true;
    }

    void read_loop()
    {
        typename ReadProps<T>::Payload read_messages[ReadProps<T>::MaxPayloadCount];

        while (true)
        {
            signal(ThreadProps<T>::Signo, ReadingThread<T>::handle_signal);
            // bug: if a signal is caught before we enter the read syscall, we will hang forever
            int count = trafficmon->read_messages<T>(read_messages);

            for (int i = 0; i < count; i++)
            {
                handler->handle(read_messages[i]);
            }

            signal(ThreadProps<T>::Signo, exit_thread);
            if (ThreadProps<T>::done)
            {
                exit_thread(ThreadProps<T>::Signo);
            }
        }
    }

public:
    ReadingThread() {}

    void start(PayloadHandler<T> *handler, Trafficmon &trafficmon)
    {
        signal(ThreadProps<T>::Signo, exit_thread);
        this->handler = handler;
        this->trafficmon = &trafficmon;
        ThreadProps<T>::done = false;
        thr = new thread(&ReadingThread<T>::read_loop, this);
    }

    void kill()
    {
        pthread_kill(thr->native_handle(), ThreadProps<T>::Signo);
    }

    void join()
    {
        thr->join();
        signal(ThreadProps<T>::Signo, SIG_DFL);
        delete thr;
    }
};
