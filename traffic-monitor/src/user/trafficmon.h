#pragma once
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
#include <vector>
#include <queue>
#include <type_traits>
#include <cassert>

using namespace std;

union ReadMessagePayload
{
#define ENTRY(name) PAYLOAD_T(name) name;
    READ_MESSAGES
#undef ENTRY
};

union WriteMessagePayload
{
#define ENTRY(name) PAYLOAD_T(name) name;
    WRITE_MESSAGES
#undef ENTRY
};

struct ReadMessage
{
    ReadMessageType type;
    ReadMessagePayload payload;
};

template <WriteMessageType>
struct Props;

#define ENTRY(name)                      \
    template <>                          \
    struct Props<name>                   \
    {                                    \
        using Payload = PAYLOAD_T(name); \
    };
WRITE_MESSAGES
#undef entry

class Trafficmon
{
private:
    int fd;
    // pulling these out of read_messages for performance reasons
    ReadHeader read_header;
    WriteHeader write_header;
    ssize_t payload_size;
    ReadMessage message;
    char temp_buffer[MAX_MESSAGE_SIZE];
    void handle(int rv);

public:
    Trafficmon();
    bool read_messages(queue<ReadMessage> &messages);
    template <WriteMessageType T>
    void write_messages(queue<typename Props<T>::Payload> &messages)
    {
        int count;
        write_header.type = T;
        while((count=min(MAX_MESSAGE_SIZE / sizeof(typename Props<T>::Payload), messages.size()))){
            write_header.count = count;

            for (int i = 0; i < count; i++)
            {
                ((typename Props<T>::Payload *)temp_buffer)[i] = messages.front();
                messages.pop();
            }

            handle(write(fd, &write_header, sizeof(write_header)));
            handle(write(fd, temp_buffer, count * sizeof(typename Props<T>::Payload)));
        }
    }

    ~Trafficmon();
};
