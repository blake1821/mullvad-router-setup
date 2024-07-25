#include "ipfilter.h"
#include "util.h"

#define is_allowed(conn) \
    (allowed.find(get_ip_pair_key(conn.src, conn.dst)) != allowed.end())

void IPFilter::handle(Query4Payload *payloads, int count)
{
    vector<SetStatus4Payload> responses;

    rules_mutex.lock();
    for (int i = 0; i < count; i++)
    {
        responses.push_back((SetStatus4Payload){
            .src = payloads[i].src,
            .dst = payloads[i].dst,
            .status = is_allowed(payloads[i]) ? Allowed : Blocked,
        });
    }
    
    rules_mutex.unlock();

    trafficmon.write_messages<SetStatus4>(responses);
}

void IPFilter::handle(Connect4Payload *payloads, int count)
{
    bool connection_allowed[ReadProps<Connect4>::MaxPayloadCount];

    rules_mutex.lock();
    for (int i = 0; i < count; i++)
    {
        connection_allowed[i] = is_allowed(payloads[i]);
    }
    rules_mutex.unlock();

    for (int i = 0; i < count; i++)
    {
        handler->handle_connection(payloads[i], connection_allowed[i]);
    }
}

void IPFilter::set_enabled(bool enabled)
{
    struct SetNfEnabledPayload payload = {
        .enabled = enabled,
    };
    strcpy(payload.outgoing_dev_name, ifname.c_str());
    trafficmon.write_message<SetNfEnabled>(payload);
}

IPFilter::IPFilter(string ifname, FilterHandler *handler)
    : handler(handler)
{
    query_thr.start(this, trafficmon);
    connect_thr.start(this, trafficmon);
    clear_rules();
    set_enabled(true);
}

void IPFilter::add_rules(vector<SetStatus4Payload> &rules)
{
    rules_mutex.lock();
    for (auto &rule : rules)
    {
        uint64_t key = get_ip_pair_key(rule.src, rule.dst);
        if (rule.status == Allowed)
        {
            allowed.insert(key);
        }
        else
        {
            allowed.erase(key);
        }
    }
    rules_mutex.unlock();
    trafficmon.write_messages<SetStatus4>(rules);
}

void IPFilter::clear_rules()
{
    rules_mutex.lock();
    allowed.clear();
    rules_mutex.unlock();
    ResetPayload reset_payload = {
        .reset = true};
    trafficmon.write_message<Reset>(reset_payload);
}

void IPFilter::kill()
{
    set_enabled(false);
    query_thr.kill();
    connect_thr.kill();
}

void IPFilter::join()
{
    query_thr.join();
    connect_thr.join();
}