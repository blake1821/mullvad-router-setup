#include "../trafficmon.h"

/*
int main(int argc, char *argv[])
{

    Trafficmon trafficmon;
    Query4Payload query_payloads[ReadProps<Query4>::MaxPayloadCount];

    while (true)
    {
        cout << "Reading a message..." << endl;
        
        int count = trafficmon.read_messages<Query4>(query_payloads);
        assert(count);

        for(int i = 0; i < count; i++){
            Query4Payload &payload = query_payloads[i];
            char src_str[128];
            char dst_str[128];
            inet_ntop(AF_INET, &payload.src, src_str, 128);
            inet_ntop(AF_INET, &payload.dst, dst_str, 128);

            cout << "New request for: " << src_str << " -> " << dst_str << endl;
            cout << "Allow? (Y/n) ";

            string response;
            cin >> response;
            
            bool allow = response.size() > 0 && (response[0] == 'Y' || response[0] == 'y');

            SetStatus4Payload response_payload = {
                .src = payload.src,
                .dst = payload.dst,
                .status = allow ? Allowed : Blocked
            };

            trafficmon.write_message<SetStatus4>(response_payload);
        }
    }

    return 0;
}
*/

