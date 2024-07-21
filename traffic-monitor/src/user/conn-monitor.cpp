#include "trafficmon.h"

int main(){
    Trafficmon trafficmon;
    struct Connect4Payload buffer[ReadProps<Connect4>::MaxPayloadCount];
    
    // enable
    vector<SetNfEnabledPayload> set_nf_enabled_payloads;
    set_nf_enabled_payloads.push_back((struct SetNfEnabledPayload){
        .enabled = true,
        .outgoing_dev_name = "wg0"
    });
    trafficmon.write_messages<SetNfEnabled>(set_nf_enabled_payloads);

    while(true){
        int count = trafficmon.read_messages<Connect4>(buffer);
        for(int i = 0; i < count; i++){
            printf("Received Connect4 message with ipv4: %s\n", inet_ntoa(buffer[i].dst));
        }
    }

}