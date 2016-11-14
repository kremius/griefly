#include <gtest/gtest.h>

#include "core/objects/MainObject.h"
#include "core/Hashes.h"

TEST(HashesTest, IntegerNumber)
{
    ASSERT_EQ(hash(static_cast<unsigned int>(0)), 0);
    ASSERT_EQ(hash(1), 1);
    ASSERT_EQ(hash(42), 42);

    ASSERT_NE(hash(54), 53);

    unsigned int test = std::numeric_limits<unsigned int>::max();
    ASSERT_EQ(hash(test), test);
}

TEST(HashesTest, IdPtrOn)
{
    IdPtr<IMainObject> ptr = 0;
    ASSERT_EQ(hash(ptr), 0);

    ptr = 131;
    ASSERT_EQ(hash(ptr), 131);

    ptr = 1000000;
    ASSERT_EQ(hash(ptr), 1000000);
}

TEST(HashesTest, VectorIdPtrOn)
{
    std::vector<IdPtr<IMainObject>> v;
    v.push_back(0);
    v.push_back(1);
    v.push_back(2);
    ASSERT_EQ(hash(v), 8);

    v[0] = 4294967295;
    ASSERT_EQ(hash(v), 7);
}

TEST(HashesTest, String)
{
    QString s1("hello world");
    ASSERT_EQ(hash(s1), 1045060184);

    s1[4] = '\0';
    ASSERT_EQ(hash(s1), 2292011938);
}



