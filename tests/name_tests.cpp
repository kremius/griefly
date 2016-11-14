#include <gtest/gtest.h>

#include "core/Names.h"
#include "core/SyncRandom.h"

TEST(Names, Basics)
{
    SyncRandom random;
    Names names(&random);
    for (int i = 0; i < 20; ++i)
    {
        QString name = names.GetMaleName();
        ASSERT_FALSE(name.isEmpty());
    }
}

