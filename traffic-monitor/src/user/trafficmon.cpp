#include "trafficmon.h"

using namespace std;

void Trafficmon::handle(int rv)
{
    if (rv < 0)
    {
        throw runtime_error("Read or write returned -1");
    }
}

Trafficmon::Trafficmon()
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

bool Trafficmon::read_messages(queue<ReadMessage> &messages)
{

    if(read(fd, &read_header, sizeof(read_header)) < 0){
        if(errno == EINTR){
            return true;
        }else{
            throw runtime_error("Failed to read header");
        }
    }
    payload_size = get_read_payload_size(read_header.type);
    handle(read(fd, temp_buffer, read_header.count * payload_size));

    for (int i = 0; i < read_header.count; i++)
    {
        message.type = read_header.type;
        message.payload = *(ReadMessagePayload *)(temp_buffer + i * payload_size);
        messages.push(message);
    }

    return false;
}

Trafficmon::~Trafficmon()
{
    close(fd);
}
