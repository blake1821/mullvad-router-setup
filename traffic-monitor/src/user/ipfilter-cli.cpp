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
        void handle_verdict(Verdict &verdict)
        {
            cout << verdict.to_string() << endl;
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

        vector<IPRule> rules{IPv4Rule(
            IPv4Address(src),
            IPv4Address(dst),
            allow)};

        ip_filter.add_rules(rules);
    }

    return 0;
}