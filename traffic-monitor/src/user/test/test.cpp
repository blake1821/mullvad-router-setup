#include "testnet.h"
#include "../trafficmon.h"
#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <signal.h>
#include "../util.h"
#include "test-generator.h"
#include "test-kernel.h"
#include "test-user.h"

using namespace std;

#define TestSession UserTestSession

int main(){
    srand(time(NULL));

    auto start_time = chrono::system_clock::now();

    TestSession test_session(10, TestMode::Random);
    cout << "1k Runs. Cache misses: " << test_session.run(1000) << endl;
    cout << "(We expect this to be exactly 10)" << endl;

    TestSession test_session2(100, TestMode::Random);
    cout << "100k Runs. Cache misses: " << test_session2.run(100000) << endl;
    cout << "(We expect this to be exactly 100)" << endl;

    TestSession test_session3(2000, TestMode::RoundRobin);
    cout << "100k Runs. Cache misses: " << test_session3.run(100000) << endl;
    cout << "(We expect this to be like 100000)" << endl;
    
    TestSession test_session4(500, TestMode::RoundRobin);
    cout << "1M Runs. Cache misses: " << test_session4.run(1000000) << endl;
    cout << "(We expect this to be 500)" << endl;

    TestSession test_session5(600, TestMode::Random);
    cout << "1M Runs. Cache misses: " << test_session4.run(1000000) << endl;
    cout << "(We expect this to be big)" << endl;


    auto end_time = chrono::system_clock::now();
    auto ellapsed_time = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
    cout << "Finished in " << ellapsed_time << "ms" << endl;

    // before optimization: Takes roughly 1800ms (Big variance)
    
    return 0;
    
}