#include <gtest/gtest.h>

#include "AutogenMetadata.h"

#include "core/FastIsType.h"

#include "core/objects/MainObject.h"
#include "core/objects/test/UnsyncGenerator.h"
#include "core/objects/DrawInt.h"

TEST(FastIsType, TwoArgs)
{
    int main_type = IMainObject::RT_ITEM_S();
    int unsync_type = UnsyncGenerator::RT_ITEM_S();
    int draw_type = IDraw::RT_ITEM_S();

    ASSERT_TRUE(FastIsType(main_type, main_type));
    ASSERT_TRUE(FastIsType(main_type, unsync_type));
    ASSERT_TRUE(FastIsType(unsync_type, unsync_type));
    ASSERT_TRUE(FastIsType(draw_type, draw_type));
    ASSERT_TRUE(FastIsType(main_type, draw_type));

    ASSERT_FALSE(FastIsType(unsync_type, main_type));
    ASSERT_FALSE(FastIsType(unsync_type, draw_type));
    ASSERT_FALSE(FastIsType(draw_type, unsync_type));
    ASSERT_FALSE(FastIsType(draw_type, main_type));
}

TEST(FastIsType, OneArg)
{
    int main_type = IMainObject::RT_ITEM_S();
    int unsync_type = UnsyncGenerator::RT_ITEM_S();
    int draw_type = IDraw::RT_ITEM_S();

    ASSERT_TRUE(FastIsType<IMainObject>(main_type));
    ASSERT_TRUE(FastIsType<IMainObject>(unsync_type));
    ASSERT_TRUE(FastIsType<UnsyncGenerator>(unsync_type));
    ASSERT_TRUE(FastIsType<IDraw>(draw_type));
    ASSERT_TRUE(FastIsType<IMainObject>(draw_type));

    ASSERT_FALSE(FastIsType<UnsyncGenerator>(main_type));
    ASSERT_FALSE(FastIsType<UnsyncGenerator>(draw_type));
    ASSERT_FALSE(FastIsType<IDraw>(unsync_type));
    ASSERT_FALSE(FastIsType<IDraw>(main_type));
}

TEST(FastIsType, CastTo)
{
    IMainObject main_type(nouse);
    UnsyncGenerator unsync_type(nouse);
    IDraw draw_type(nouse);

    IDraw* null_type = nullptr;

    ASSERT_FALSE(CastTo<IMainObject>(null_type));

    ASSERT_TRUE(CastTo<IMainObject>(&main_type));
    ASSERT_TRUE(CastTo<IMainObject>(&unsync_type));
    ASSERT_TRUE(CastTo<UnsyncGenerator>(&unsync_type));
    ASSERT_TRUE(CastTo<IDraw>(&draw_type));
    ASSERT_TRUE(CastTo<IMainObject>(&draw_type));

    ASSERT_FALSE(CastTo<UnsyncGenerator>(&main_type));
    ASSERT_FALSE(CastTo<UnsyncGenerator>(&draw_type));
    ASSERT_FALSE(CastTo<IDraw>(&unsync_type));
    ASSERT_FALSE(CastTo<IDraw>(&main_type));
}
