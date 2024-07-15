#include <iostream>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
extern "C"
{
#include "../common/protocol.h"
}
#include <string.h>

using namespace std;

class TestNet
{
private:
    int fd;

public:
    TestNet()
    {
        char filename[128] = "/proc/";
        strcat(filename, TEST_INTERFACE_PROC_FILE);

        fd = open(filename, O_WRONLY);
        if (fd == -1)
        {
            cerr << "Failed to open file" << endl;
            exit(1);
        }
    }

    ~TestNet()
    {
        close(fd);
    }

    IPStatus write_ip(struct in_addr &addr)
    {
        IPStatus bytes_written = (IPStatus)write(fd, &addr, sizeof(addr));
        if (bytes_written == -1)
        {
            cerr << "Failed to write to test interface file" << endl;
            exit(1);
        }
        return bytes_written;
    }
};

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