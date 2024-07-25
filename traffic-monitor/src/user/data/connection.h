#include "ipaddress.h"

class BaseConnection{
private:
    uint16_t dst_port;
    bool allowed;
public:
    BaseConnection(uint16_t dst_port, bool allowed) : dst_port(dst_port), allowed(allowed) {}
    virtual string to_string() = 0;
    virtual IPAddressBase *get_src() const = 0;
    virtual IPAddressBase *get_dst() const = 0;
    uint16_t get_dst_port(){
        return dst_port;
    }

    bool is_allowed(){
        return allowed;
    }
};

template<typename T>
class ConcreteConnection : public BaseConnection{
private:
    T src;
    T dst;
public:
    ConcreteConnection(T src, T dst, uint16_t dst_port, bool allowed) : src(src), dst(dst), BaseConnection(dst_port), allowed(allowed) {} 

    inline IPAddressBase *get_src() const override
    {
        return &src;
    }   

    inline IPAddressBase *get_dst() const override
    {
        return &dst;
    }

    string to_string() override{
        return src.to_string() + " -> " + dst.to_string() + " : " + to_string(dst_port);
    }
};

typedef ConcreteConnection<IPv4Address> IPv4Connection;
typedef ConcreteConnection<IPv6Address> IPv6Connection;

#define Connection BASED_UNION(BaseConnection, IPv4Connection, IPv6Connection)
DECLARE_UNION(Connection);