#include "KVAbort.h"

#include <gtest/gtest.h>

TEST(KvAbortDeathTest, Basics)
{
    ASSERT_DEATH(
    {
        KvAbort();
    }, "");
}
