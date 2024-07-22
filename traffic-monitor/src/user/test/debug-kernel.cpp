#include "../util.h"

int main()
{
#ifdef TEST_NETHOOKS
    Trafficmon trafficmon;
    DebugRequestPayload request = {
        .avoid_locking = true
    };
    DebugResponsePayload response = trafficmon.debug(request);

    print_debug_response(response);
#else
    cout << "TEST_NETHOOKS not defined" << endl;
#endif
}