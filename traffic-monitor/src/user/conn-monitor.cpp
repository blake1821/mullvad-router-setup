#include "trafficmon.h"

int main(){
    Trafficmon trafficmon;
    struct Connect4Payload buffer[ReadProps<Connect4>::MaxPayloadCount];
    
    // enable
    SetNfEnabledPayload set_nf_enabled_payload = {
        .enabled = true,
    };
    trafficmon.write_message<SetNfEnabled>(set_nf_enabled_payload);

    while(true){
        int count = trafficmon.read_messages<Connect4>(buffer);
        for(int i = 0; i < count; i++){
            printf("Received Connect4 message with ipv4: %s\n", inet_ntoa(buffer[i].dst));
        }
    }

}