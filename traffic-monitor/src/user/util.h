#include <pthread.h>

class Barrier{
private:
    pthread_barrier_t barrier;

public:
    Barrier(int count){
        pthread_barrier_init(&barrier, NULL, count);
    }

    void wait(){
        pthread_barrier_wait(&barrier);
    }

    ~Barrier(){
        pthread_barrier_destroy(&barrier);
    }
};