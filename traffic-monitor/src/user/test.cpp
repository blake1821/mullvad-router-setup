#include "testnet.h"
#include "trafficmon.h"
#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <signal.h>
#include "util.h"

using namespace std;

struct in_addr generate_random_ipv4() {
    struct in_addr ipAddr;

    // Generate a random IP address. Each part of the IP can be a number between 0 and 255
    unsigned long addr = rand() % 0xFFFFFFFF;
    ipAddr.s_addr = htonl(addr); // Convert to network byte order

    return ipAddr;
}

IPStatus generate_random_status(){
    return (rand() % 2 == 0) ? Allowed : Blocked;
}

enum class TestMode{
    RoundRobin,
    Random
};


Barrier finished_barrier(2);
bool done = false;
void signal_handler(int){
    done = true;
}


class TestSession{
private:
    Trafficmon trafficmon;
    TestNet testnet;
    int ip_count;
    vector<struct in_addr> ip_addrs;
    map<in_addr_t, IPStatus> ip_status;
    int cache_misses = 0;


    void run_filter(){
        queue<ReadMessage> read_messages;
        queue<SetStatus4Payload> write_messages;

        while(true){
            if(done || trafficmon.read_messages(read_messages))
                break;
            while(!read_messages.empty()){
                auto msg = read_messages.front();
                read_messages.pop();

                if(msg.type == ReadMessageType::Query4){
                    cache_misses++;
                    struct in_addr ipv4 = msg.payload.Query4.ipv4;

                    write_messages.push(
                        {
                            .ipv4=ipv4,
                            .status=ip_status[ipv4.s_addr]
                        }
                    );
                }else{
                    throw runtime_error("Unexpected message type");
                }
            }

            trafficmon.write_messages<WriteMessageType::SetStatus4>(write_messages);
        }
        finished_barrier.wait();
    }

    inline void test_ip(struct in_addr ipv4){
        IPStatus status = testnet.write_ip(ipv4);
        if(status != ip_status[ipv4.s_addr]){
            throw runtime_error("Unexpected status");
        }
    }


public:
    TestSession(int ip_count){
        for(int i = 0; i < ip_count; i++){
            struct in_addr ipv4 = generate_random_ipv4();
            ip_addrs.push_back(ipv4);
            ip_status[ipv4.s_addr] = generate_random_status();
        }
    }

    /** Run the tests. Return the # of cache misses  */
    int run(TestMode test_mode, int num_rounds){
        done = false;
        signal(SIGINT, signal_handler);
        thread filter_thread = thread(&TestSession::run_filter, this);

        if(test_mode == TestMode::RoundRobin){
            int ip_index = 0;
            for(int i = 0; i < num_rounds; i++){
                auto ip = ip_addrs[ip_index++];
                if(ip_index == ip_addrs.size()){
                    ip_index = 0;
                }
                test_ip(ip);
            }
        }else{
            for(int i = 0; i < num_rounds; i++){
                test_ip(ip_addrs[rand() % ip_addrs.size()]);
            }
        }
        
        // send a signal to the filter thread to stop
        pthread_kill(filter_thread.native_handle(), SIGINT);
        
        // memory barrier to ensure cache_misses is updated
        finished_barrier.wait();
        filter_thread.join();
        signal(SIGINT, SIG_DFL);
        
        return cache_misses;
    }
};

int main(){
    srand(time(NULL));
    TestSession test_session(10);
    cout << "1k Runs. Cache misses: " << test_session.run(TestMode::Random, 1000) << endl;
    cout << "(We expect this to be exactly 10)" << endl;

    TestSession test_session2(100);
    cout << "100k Runs. Cache misses: " << test_session2.run(TestMode::Random, 100000) << endl;
    cout << "(We expect this to be exactly 100)" << endl;

    TestSession test_session3(2000);
    cout << "100k Runs. Cache misses: " << test_session3.run(TestMode::RoundRobin, 100000) << endl;
    cout << "(We expect this to be like 100000)" << endl;
    
    TestSession test_session4(1000);
    cout << "1M Runs. Cache misses: " << test_session4.run(TestMode::RoundRobin, 1000000) << endl;
    cout << "(We expect this to be 1000)" << endl;
    
    return 0;
    
}