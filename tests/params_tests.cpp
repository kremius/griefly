#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Params.h"

using ::testing::internal::CaptureStderr;
using ::testing::internal::GetCapturedStderr;
using ::testing::HasSubstr;

TEST(Params, Basics)
{
    ParamsHolder params;

    const int ARGC = 3;
    const char* argv_data[ARGC] = {"-var", "var2=text", "var3=3"};

    // argv should be mutable
    char** argv = new char*[ARGC];
    for (int i = 0; i < ARGC; ++i)
    {
        int length = strlen(argv_data[i]);
        argv[i] = new char[length];
        strcpy(argv[i], argv_data[i]);
    }

    params.ParseParams(3, argv);
    ASSERT_TRUE(params.GetParamBool("-var"));
    ASSERT_TRUE(params.GetParamBool("var2"));
    ASSERT_TRUE(params.GetParamBool("var3"));
    ASSERT_FALSE(params.GetParamBool("do_not_exist"));

    ASSERT_EQ(params.GetParam<QString>("-var"), "");
    ASSERT_EQ(params.GetParam<int>("-var"), 0);
    ASSERT_EQ(params.GetParam<QString>("var2"), "text");
    ASSERT_EQ(params.GetParam<QString>("var3"), "3");
    ASSERT_EQ(params.GetParam<int>("var3"), 3);

    {
        CaptureStderr();
        ASSERT_EQ(params.GetParam<QString>("donot"), "");
        std::string output = GetCapturedStderr();
        ASSERT_THAT(output, HasSubstr("Cannot find param"));
        ASSERT_THAT(output, HasSubstr("donot"));
    }
    {
        CaptureStderr();
        ASSERT_EQ(params.GetParam<int>("donot"), 0);
        std::string output = GetCapturedStderr();
        ASSERT_THAT(output, HasSubstr("Cannot find param"));
        ASSERT_THAT(output, HasSubstr("donot"));
    }

    for (int i = 0; i < ARGC; ++i)
    {
        delete[] argv[i];
    }
    delete[] argv;
}
