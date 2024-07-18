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
    // open the proc files
    #define ENTRY(name) write_fd[WriteMessageType::name] = \
        open("/proc/" TRAFFICMON_PROC_FILE "/" #name, O_WRONLY);
    WRITE_MESSAGES
    #undef ENTRY

    #define ENTRY(name) read_fd[ReadMessageType::name] = \
        open("/proc/" TRAFFICMON_PROC_FILE "/" #name, O_RDONLY);
    READ_MESSAGES
    #undef ENTRY

    // check if the file descriptors are valid
    for(int i = 0; i < WRITE_MESSAGE_COUNT; i++){
        if(write_fd[i] < 0){
            throw runtime_error("Failed to open write file");
        }
    }
    for(int i = 0; i < READ_MESSAGE_COUNT; i++){
        if(read_fd[i] < 0){
            throw runtime_error("Failed to open read file");
        }
    }
}

Trafficmon::~Trafficmon()
{
    // close the proc files
    for(int i = 0; i < WRITE_MESSAGE_COUNT; i++){
        close(write_fd[i]);
    }

    for(int i = 0; i < READ_MESSAGE_COUNT; i++){
        close(read_fd[i]);
    }
}
