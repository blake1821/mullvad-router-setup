#pragma once
#include "ippair.h"

class BaseConnection: public IPPair
{
protected:
    uint16_t dst_port;
    ConnectProtocol proto;

public:
    BaseConnection(uint16_t dst_port, ConnectProtocol proto) : dst_port(dst_port), proto(proto){}
    virtual string to_string() = 0;
    uint16_t get_dst_port()
    {
        return dst_port;
    }

    uint64_t get_conn_key() const{
        return get_key() + (uint64_t)dst_port + (uint64_t)(proto << 16);
    }

};

template <IPVersion V>
class ConcreteConnection : public BaseConnection, public IPPairConcrete<V>
{
public:

    using T = typename IPProps<V>::Address;
    ConcreteConnection(T src, T dst, uint16_t dst_port, ConnectProtocol proto) : BaseConnection(dst_port, proto), IPPairConcrete<V>(src, dst) {}

    IMPLEMENT_IPPAIR

    string to_string() override
    {
        return IPPairConcrete<V>::src.to_string() + " -> " + IPPairConcrete<V>::dst.to_string() + " : " + std::to_string(dst_port);
    }

    inline IPProps<V>::ConnectPayload to_connect_payload() const
    {
        return typename IPProps<V>::ConnectPayload{
            .src = IPPairConcrete<V>::src.addr,
            .dst = IPPairConcrete<V>::dst.addr,
            .dst_port = dst_port,
            .protocol = proto,
        };
    }

};

typedef ConcreteConnection<IPv4> IPv4Connection;
typedef ConcreteConnection<IPv6> IPv6Connection;

#define Connection BASED_UNION(BaseConnection, IPv4Connection, IPv6Connection)
DECLARE_UNION(Connection);

class Verdict
{
private:
    Connection conn;
    bool allowed;
public:
    Verdict(Connection conn, bool allowed) : conn(conn), allowed(allowed) {}

    inline Connection get_connection()
    {
        return conn;
    }

    inline bool is_allowed()
    {
        return allowed;
    }

    string to_string()
    {
        return conn.base().to_string() + " " + (allowed ? "ALLOWED" : "DENIED");
    }
};

const Connection UNINITIALIZED_CONNECTION = IPv4Connection(IPv4Address("0.0.0.0"), IPv4Address("0.0.0.0"), 0, ProtoTCP);