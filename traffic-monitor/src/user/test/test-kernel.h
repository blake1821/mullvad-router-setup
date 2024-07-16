#include "testnet.h"
#include "../trafficmon.h"
#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <signal.h>
#include "../util.h"
#include "test-generator.h"

class KernelTestSession
{
private:
    Trafficmon trafficmon;
    TestNet testnet;
    TestGenerator generator;

    void run_filter();
    

public:
    KernelTestSession(int ip_count, TestMode mode);
    int run(int num_rounds);
};