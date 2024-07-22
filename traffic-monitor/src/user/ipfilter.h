#include "trafficmon.h"
#include "reading-thread.h"
#include <mutex>
#include <map>
#include <set>

class FilterHandler{
public:
    virtual void handle_connection(Connect4Payload &conn, bool allowed) = 0;
};

class IPFilter : protected PayloadHandler<Query4>, protected PayloadHandler<Connect4>
{
private:
    Trafficmon trafficmon;
    set<uint64_t> allowed;
    mutex rules_mutex;
    ReadingThread<Query4> query_thr;
    ReadingThread<Connect4> connect_thr;
    FilterHandler *handler;
    string ifname;
protected:
    void handle(Query4Payload *payloads, int count) override;
    void handle(Connect4Payload *payloads, int count) override;
    void set_enabled(bool enabled);

public:
    IPFilter(string ifname, FilterHandler *handler);
    void add_rules(vector<SetStatus4Payload> &rules);
    void clear_rules();
    void kill();
    void join();
};