#include <iostream>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
extern "C" {
    #include "../common/protocol.h"
}
#include <string.h>

using namespace std;

// #define TEST_INTERFACE_PROC_FILE "/proc/test_interface"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <ip_address>" << endl;
        return 1;
    }

    const char* ip_address_str = argv[1];
    struct in_addr addr;
    if (inet_pton(AF_INET, ip_address_str, &addr) <= 0) {
        cerr << "Invalid IP address: " << ip_address_str << endl;
        return 1;
    }

    char filename[128] = "/proc/";
    strcat(filename, TEST_INTERFACE_PROC_FILE);

    int fd = open(filename, O_WRONLY);
    if (fd == -1) {
        cerr << "Failed to open file: " << filename << endl;
        return 1;
    }

    IPStatus bytes_written = (IPStatus) write(fd, &addr, sizeof(addr));
    if (bytes_written == -1) {
        cerr << "Failed to write to file" << endl;
        close(fd);
        return 1;
    }

    if(bytes_written == Allowed){
        cout << "Allowed" << endl;
    } else {
        cout << "Blocked" << endl;
    }

    close(fd);
    return 0;
}