#pragma once
#include <iostream>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
extern "C"
{
#include "../../common/protocol.h"
}
#include <string.h>

using namespace std;

class TestNet
{
private:
    int fd;

public:
    TestNet();
    ~TestNet();
    IPStatus write_ip(struct in_addr &addr);
};
