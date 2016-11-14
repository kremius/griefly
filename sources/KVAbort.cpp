#include "KVAbort.h"

#include <cstdlib>

#ifdef _BUILD_COVER
extern "C" void __gcov_flush();
void KvAbort()
{
    __gcov_flush();
    abort();
}
#else
void KvAbort()
{
    abort();
}
#endif // _BUILD_COVER
