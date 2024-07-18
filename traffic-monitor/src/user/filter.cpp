#include "trafficmon.h"

int main(int argc, char *argv[])
{

    Trafficmon trafficmon;
    Query4Payload query_payloads[ReadProps<Query4>::MaxPayloadCount];
    vector<SetStatus4Payload> set_4_payloads;

    while (true)
    {

        cout << "Reading a message..." << endl;
        
        int count = trafficmon.read_messages<Query4>(query_payloads);
        assert(count);


        for(int i = 0; i < count; i++){
            Query4Payload &payload = query_payloads[i];
            char ip_addr_str[128];
            inet_ntop(AF_INET, &payload.ipv4, ip_addr_str, 128);
            cout << "New request for: " << ip_addr_str << endl;
            cout << "Allow? (Y/n) ";

            string response;
            cin >> response;
            
            bool allow = response.size() > 0 && (response[0] == 'Y' || response[0] == 'y');

            SetStatus4Payload response_payload = {
                .ipv4 = payload.ipv4,
                .status = allow ? Allowed : Blocked
            };

            set_4_payloads.push_back(response_payload);
            trafficmon.write_messages<SetStatus4>(set_4_payloads);
            set_4_payloads.clear();
        }
    }

    return 0;
}