#include "testnet.h"

using namespace std;

// #define TEST_INTERFACE_PROC_FILE "/proc/test_interface"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Usage: " << argv[0] << " <ip_address>" << endl;
        return 1;
    }

    const char *ip_address_str = argv[1];
    struct in_addr addr;
    if (inet_pton(AF_INET, ip_address_str, &addr) <= 0)
    {
        cerr << "Invalid IP address: " << ip_address_str << endl;
        return 1;
    }

    TestNet test_net;

    IPStatus status = test_net.write_ip(addr);

    if (status == Allowed)
    {
        cout << "Allowed" << endl;
    }
    else
    {
        cout << "Blocked" << endl;
    }

    return 0;
}