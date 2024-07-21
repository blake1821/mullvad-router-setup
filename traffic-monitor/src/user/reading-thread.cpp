#include "reading-thread.h"
bool ThreadProps<Query4>::done = false;
#ifdef TEST_NETHOOKS
bool ThreadProps<TestVerdict4>::done = false;
#endif
bool ThreadProps<Connect4>::done = false;