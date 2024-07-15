#include "testnet.h"

using namespace std;

TestNet::TestNet()
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

TestNet::~TestNet()
{
    close(fd);
}

IPStatus TestNet::write_ip(struct in_addr &addr)
{
    IPStatus bytes_written = (IPStatus)write(fd, &addr, sizeof(addr));
    if (bytes_written == -1)
    {
        cerr << "Failed to write to test interface file" << endl;
        exit(1);
    }
    return bytes_written;
}
