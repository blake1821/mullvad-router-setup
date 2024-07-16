#include "util.h"

Barrier::Barrier(int count)
{
    pthread_barrier_init(&barrier, NULL, count);
}

void Barrier::wait()
{
    pthread_barrier_wait(&barrier);
}

Barrier::~Barrier()
{
    pthread_barrier_destroy(&barrier);
}