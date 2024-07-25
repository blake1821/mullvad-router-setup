#include "trafficmon.h"
#include "reading-thread.h"
#include <mutex>
#include <map>
#include <set>
#include "data/iprule.h"
#include "data/connection.h"


class FilterHandler
{
public:
    virtual void handle_connection(Connection &conn) = 0;
};


#define IPFilterReadMessages PAIRAPPLYCOMMA(CONCAT, (IP_VERSIONS), Query, Connect)

#define _MAKE_PAYLOAD_HANDLER(T) \
protected                        \
    PayloadHandler<T>
class IPFilter : APPLYCOMMA(_MAKE_PAYLOAD_HANDLER, IPFilterReadMessages)
{
#undef _MAKE_PAYLOAD_HANDLER

private:
    Trafficmon trafficmon;

#define _DECLARE_IP_READER_THREAD(message) \
    ReadingThread<message> thr_##message;  \
    void handle(PAYLOAD_T(message) * payloads, int count);
    APPLY(_DECLARE_IP_READER_THREAD, IPFilterReadMessages)
#undef _DECLARE_IP_READER_THREAD

#define _MAKE_IP_MEMBERS(version)   \
    set<uint64_t> allowed##version; \
    mutex rules_mutex##version;
    APPLY(_MAKE_IP_MEMBERS, IP_VERSIONS)
#undef _MAKE_IP_MEMBERS
    FilterHandler *handler;
    string ifname;

protected:
    void set_enabled(bool enabled);

public:
    IPFilter(string ifname, FilterHandler *handler);
    void add_rules(vector<SetStatus4Payload> &rules);
    void clear_rules();
    void kill();
    void join();
};