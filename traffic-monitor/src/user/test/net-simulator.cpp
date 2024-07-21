#include "../reading-thread.h"

using namespace std;

int main(int argc, char *argv[])
{
#ifdef TEST_NETHOOKS

    Trafficmon trafficmon;

    class LocalHandler : VerdictHandler
    {
        void handle(ReadProps<TestVerdict4>::Payload &payload)
        {
            cout << "Received verdict: " << payload.allowed << endl;
        }
    };

    LocalHandler handler;
    ReadingThread<TestVerdict4> thread;
    thread.start((VerdictHandler *)&handler, trafficmon);

    while (true)
    {
        cout << "Enter source and destination IP addresses: " << endl;
        cout << "Source: ";
        char src_str[1000];
        cin >> src_str;
        struct in_addr src;
        if (inet_pton(AF_INET, src_str, &src) <= 0)
        {
            cerr << "Invalid IP address: " << src_str << endl;
            return 1;
        }

        cout << "Destination: ";
        char dst_str[1000];
        cin >> dst_str;
        struct in_addr dst;
        if (inet_pton(AF_INET, dst_str, &dst) <= 0)
        {
            cerr << "Invalid IP address: " << dst_str << endl;
            return 1;
        }

        vector<TestPacket4Payload> payloads;
        Connect4Payload conn = {
            .dst_port = 1000,
            .protocol = ProtoTcp,
            .src = src,
            .dst = dst,
        };
        payloads.push_back((TestPacket4Payload){conn});
        trafficmon.write_messages<TestPacket4>(payloads);
        // cout << "Wrote packet. Waiting for verdict..." << endl;
        sleep(1);
    }

#else
    cout << "This test is only available with TEST_NETHOOKS" << endl;
#endif

    return 0;
}