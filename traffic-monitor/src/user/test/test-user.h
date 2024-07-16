extern "C"
{
#include "../../common/iplookup.h"
}
#include "test-generator.h"
#include <stdexcept>

using namespace std;

class UserTestSession
{
private:

public:
    UserTestSession(int ip_count, TestMode mode);
    int run(int num_rounds);
};