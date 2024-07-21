#pragma once
#include <pthread.h>

class Barrier
{
private:
    pthread_barrier_t barrier;
public:
    Barrier(int count);
    void wait();
    ~Barrier();
};

