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

template <WriteMessageType>
struct WriteProps;
#define ENTRY(name)                                                 \
    template <>                                                     \
    struct WriteProps<name>                                         \
    {                                                               \
        using Payload = PAYLOAD_T(name);                            \
        static const int MaxPayloadCount = MAX_PAYLOAD_COUNT(name); \
    };
WRITE_MESSAGES
#undef ENTRY

#define ENTRY(name) + 1
constexpr int WRITE_MESSAGE_COUNT = 0 + WRITE_MESSAGES;
#undef ENTRY

template <ReadMessageType>
struct ReadProps;
#define ENTRY(name)                                                 \
    template <>                                                     \
    struct ReadProps<name>                                          \
    {                                                               \
        using Payload = PAYLOAD_T(name);                            \
        static const int MaxPayloadCount = MAX_PAYLOAD_COUNT(name); \
    };
READ_MESSAGES
#undef ENTRY

#define ENTRY(name) + 1
constexpr int READ_MESSAGE_COUNT = 0 + READ_MESSAGES;
#undef ENTRY

class Trafficmon
{
private:
    int write_fd[WRITE_MESSAGE_COUNT];
    int read_fd[READ_MESSAGE_COUNT];

    void handle(int rv);

public:
    Trafficmon();

    /** Precondition: messages is empty */
    template <ReadMessageType T, size_t N>
    inline int read_messages(typename ReadProps<T>::Payload (&buffer)[N]){
        static_assert(N >= ReadProps<T>::MaxPayloadCount, "Buffer passed to read_messages() is too small.");
        return read(read_fd[T], buffer, 0);
    }

    template <WriteMessageType T>
    void write_messages(vector<typename WriteProps<T>::Payload> &messages)
    {
        int count;
        int start = 0;
        while ((count = min(MAX_MESSAGE_SIZE / sizeof(typename WriteProps<T>::Payload), messages.size() - start)))
        {
            handle(write(write_fd[T], &messages[start], count));
            start += count;
        }
    }

    ~Trafficmon();
};
