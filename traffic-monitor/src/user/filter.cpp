#include "trafficmon.h"

int main(int argc, char *argv[])
{

    Trafficmon trafficmon;
    queue<ReadMessage> read_messages;
    queue<SetStatus4Payload> set_4_payloads;

    while (true)
    {

        cout << "Reading a message..." << endl;
        
        trafficmon.read_messages(read_messages);
        assert(read_messages.size());


        while (!read_messages.empty())
        {
            auto message = read_messages.front();
            read_messages.pop();

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

                    set_4_payloads.push(response_payload);
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