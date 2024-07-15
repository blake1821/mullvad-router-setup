
#include <iostream>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
extern "C" {
    #include "../common/protocol.h"
}
#include <string.h>
#include <vector>
#include <deque>
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
    template<>                           \
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

    void handle(int rv)
    {
        if (rv < 0)
        {
            throw runtime_error("Read or write returned -1");
        }
    }

public:
    Trafficmon()
    {
        char filename[128] = "/proc/";
        strcat(filename, TRAFFICMON_PROC_FILE);

        fd = open(filename, O_RDWR);
        if (fd == -1)
        {
            cerr << "Failed to open file: " << filename << endl;
            exit(1);
        }
    }

    void read_messages(deque<ReadMessage> &messages)
    {

        handle(read(fd, &read_header, sizeof(read_header)));
        payload_size = get_read_payload_size(read_header.type);
        handle(read(fd, temp_buffer, read_header.count * payload_size));

        for (int i = 0; i < read_header.count; i++)
        {
            message.type = read_header.type;
            message.payload = *(ReadMessagePayload *)(temp_buffer + i * payload_size);
            messages.push_back(message);
        }
    }

    template <WriteMessageType T>
    void write_messages(deque<typename Props<T>::Payload> &messages)
    {
        //static_assert(is_enum<WriteMessageType>::value, "T must be a WriteMessageType");
        write_header.type = T;
        int count = min(MAX_MESSAGE_SIZE / sizeof(typename Props<T>::Payload), messages.size());
        write_header.count = count;

        for (int i = 0; i < count; i++)
        {
            ((typename Props<T>::Payload *)temp_buffer)[i] = messages.front();
            messages.pop_front();
        }

        handle(write(fd, &write_header, sizeof(write_header)));
        handle(write(fd, temp_buffer, count * sizeof(typename Props<T>::Payload)));
    }

    ~Trafficmon()
    {
        close(fd);
    }
};

int main(int argc, char *argv[])
{

    Trafficmon trafficmon;
    deque<ReadMessage> read_messages;

    deque<SetStatus4Payload> set_4_payloads;

    while (true)
    {

        cout << "Reading a message..." << endl;
        
        trafficmon.read_messages(read_messages);
        assert(read_messages.size());


        while (!read_messages.empty())
        {
            auto message = read_messages.front();
            read_messages.pop_front();

            string response;
            bool allow;
            char ip_addr_str[128];
            SetStatus4Payload response_payload;
            Query4Payload &payload = message.payload.Query4;

            switch(message.type){
                case ReadMessageType::Connect4:
                    throw runtime_error("I didn't put this in yet");
                    break;
                case ReadMessageType::Connect6:
                    throw runtime_error("We don't support IPv6 yet");
                    break;
                case ReadMessageType::Query4:
                    inet_ntop(AF_INET, &payload.ipv4, ip_addr_str, 128);
                    cout << "New request for: " << ip_addr_str << endl;
                    cout << "Allow? (Y/n) ";

                    cin >> response;
                    
                    allow = response.size() > 0 && (response[0] == 'Y' || response[0] == 'y');

                    response_payload = {
                        .ipv4 = payload.ipv4,
                        .status = allow ? Allowed : Blocked
                    };

                    set_4_payloads.push_back(response_payload);
                    trafficmon.write_messages<SetStatus4>(set_4_payloads);
                    
                    break;
                case ReadMessageType::Query6:
                    throw runtime_error("We don't support IPv6 yet");
                    break;
            }
        }
    }

    return 0;
}