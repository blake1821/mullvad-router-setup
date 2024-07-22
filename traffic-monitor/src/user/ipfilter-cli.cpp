#include "ipfilter.h"

IPFilter *ip_filter;

void sigint_handler(int signo)
{
    ip_filter->kill();
    exit(0);
}

int main(int argc, char *argv[])
{

    class CliHandler : FilterHandler
    {
        void handle_connection(Connect4Payload &conn, bool allowed)
        {
            char src_str[128];
            char dst_str[128];
            inet_ntop(AF_INET, &conn.src, src_str, 128);
            inet_ntop(AF_INET, &conn.dst, dst_str, 128);

            cout << "Connection from " << src_str << " to " << dst_str << " is " << (allowed ? "allowed" : "blocked") << endl;
        }
    };

    CliHandler handler;

    IPFilter ip_filter("wg0", (FilterHandler *)&handler);
    ::ip_filter = &ip_filter;

    signal(SIGINT, sigint_handler);

    while (!done)
    {
        cout << "Enter source and destination IP addresses: " << endl;
        cout << "Source: ";
        char src_str[100];
        cin >> src_str;
        struct in_addr src;
        if (inet_pton(AF_INET, src_str, &src) <= 0)
        {
            cerr << "Invalid IP address: " << src_str << endl;
            return 1;
        }

        cout << "Destination: ";
        char dst_str[100];
        cin >> dst_str;
        struct in_addr dst;
        if (inet_pton(AF_INET, dst_str, &dst) <= 0)
        {
            cerr << "Invalid IP address: " << dst_str << endl;
            return 1;
        }

        cout << "Allow? (Y/n) ";
        char response[10];
        cin >> response;
        bool allow = response[0] == 'Y' || response[0] == 'y';

        SetStatus4Payload payload = {
            .src = src,
            .dst = dst,
            .status = allow ? Allowed : Blocked};

        vector<SetStatus4Payload> rules{payload};
        ip_filter.add_rules(rules);
    }

    return 0;
}