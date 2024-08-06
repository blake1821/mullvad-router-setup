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
#include "../trafficmon.h"
#include "../data/iprule.h"
#include "../data/connection.h"
#include "../data/query.h"

using namespace std;


class VirtualMessageHandler
{
public:
    virtual void handle_query(IPQuery &query) = 0;
    virtual void handle_verdict(Verdict &verdict) = 0;
};

class TestContext
{
public:
    virtual void send_packet(Connection &connection) = 0;
    virtual void send_status(IPRule &rule) = 0;

    // launch a thread to read packets
    // note: this does nothing in the user testnet
    virtual void start_reading(VirtualMessageHandler *handler) = 0;

    // interrupt the reading thread
    virtual void stop_reading() = 0;

    // return true if the batch is ok
    virtual void finish_batch() = 0;

    virtual void debug() = 0;
};

#ifdef TEST_NETHOOKS
TestContext *new_kernel_test_context(Trafficmon &trafficmon);
#endif
TestContext *new_user_test_context();
