#include "representation/ViewInfo.h"

#include <gtest/gtest.h>

TEST(FramesetInfo, SettersAndGetters)
{
    ViewInfo::FramesetInfo frameset_info;
    ASSERT_EQ(frameset_info.GetSprite(), "");
    ASSERT_EQ(frameset_info.GetState(), "");
    ASSERT_EQ(frameset_info.GetAngle(), 0);
    ASSERT_EQ(frameset_info.GetShiftX(), 0);
    ASSERT_EQ(frameset_info.GetShiftY(), 0);

    frameset_info.SetSprite("something");
    ASSERT_EQ(frameset_info.GetSprite(), "something");
    ASSERT_EQ(frameset_info.GetState(), "");
    ASSERT_EQ(frameset_info.GetAngle(), 0);
    ASSERT_EQ(frameset_info.GetShiftX(), 0);
    ASSERT_EQ(frameset_info.GetShiftY(), 0);

    frameset_info.SetState("11112");
    ASSERT_EQ(frameset_info.GetSprite(), "something");
    ASSERT_EQ(frameset_info.GetState(), "11112");
    ASSERT_EQ(frameset_info.GetAngle(), 0);
    ASSERT_EQ(frameset_info.GetShiftX(), 0);
    ASSERT_EQ(frameset_info.GetShiftY(), 0);

    frameset_info.SetAngle(42);
    ASSERT_EQ(frameset_info.GetSprite(), "something");
    ASSERT_EQ(frameset_info.GetState(), "11112");
    ASSERT_EQ(frameset_info.GetAngle(), 42);
    ASSERT_EQ(frameset_info.GetShiftX(), 0);
    ASSERT_EQ(frameset_info.GetShiftY(), 0);

    frameset_info.SetShift(11, 22);
    ASSERT_EQ(frameset_info.GetSprite(), "something");
    ASSERT_EQ(frameset_info.GetState(), "11112");
    ASSERT_EQ(frameset_info.GetAngle(), 42);
    ASSERT_EQ(frameset_info.GetShiftX(), 11);
    ASSERT_EQ(frameset_info.GetShiftY(), 22);
}

TEST(FramesetInfo, IsSameSprites)
{
    ViewInfo::FramesetInfo frameset_info1;
    ASSERT_TRUE(ViewInfo::FramesetInfo::IsSameSprites(frameset_info1, frameset_info1));

    frameset_info1.SetSprite("sprite");
    frameset_info1.SetState("state");
    frameset_info1.SetAngle(42);
    frameset_info1.SetShift(420, 420);
    ASSERT_TRUE(ViewInfo::FramesetInfo::IsSameSprites(frameset_info1, frameset_info1));

    ViewInfo::FramesetInfo frameset_info2;
    ASSERT_FALSE(ViewInfo::FramesetInfo::IsSameSprites(frameset_info1, frameset_info2));

    frameset_info2.SetSprite("sprite");
    frameset_info2.SetState("state");
    frameset_info2.SetAngle(42);
    frameset_info2.SetShift(420, 420);
    ASSERT_TRUE(ViewInfo::FramesetInfo::IsSameSprites(frameset_info1, frameset_info2));    

    frameset_info2.SetSprite("sprite2");
    ASSERT_FALSE(ViewInfo::FramesetInfo::IsSameSprites(frameset_info1, frameset_info2));

    frameset_info2.SetSprite("sprite");
    frameset_info2.SetState("state2");
    ASSERT_FALSE(ViewInfo::FramesetInfo::IsSameSprites(frameset_info1, frameset_info2));

    frameset_info2.SetState("state");
    frameset_info2.SetAngle(43);
    ASSERT_FALSE(ViewInfo::FramesetInfo::IsSameSprites(frameset_info1, frameset_info2));

    frameset_info2.SetAngle(42);
    frameset_info2.SetShift(1, 420);
    ASSERT_FALSE(ViewInfo::FramesetInfo::IsSameSprites(frameset_info1, frameset_info2));

    frameset_info2.SetShift(420, 1);
    ASSERT_FALSE(ViewInfo::FramesetInfo::IsSameSprites(frameset_info1, frameset_info2));
}

TEST(FramesetInfo, StreamOperators)
{
    ViewInfo::FramesetInfo frameset_info;
    frameset_info.SetSprite("sprite 1");
    frameset_info.SetState("state 2");
    frameset_info.SetAngle(84);
    frameset_info.SetShift(12, 13);

    std::stringstream str;
    str << frameset_info;

    ASSERT_EQ(str.str(), "8 sprite 1 7 state 2 84 12 13 ");

    str.str("8 sprite 2 7 state 3 -14 12 13");

    ViewInfo::FramesetInfo frameset_info2;
    str >> frameset_info2;
    ASSERT_EQ(frameset_info2.GetSprite(), "sprite 2");
    ASSERT_EQ(frameset_info2.GetState(), "state 3");
    ASSERT_EQ(frameset_info2.GetAngle(), -14);
    ASSERT_EQ(frameset_info2.GetShiftX(), 12);
    ASSERT_EQ(frameset_info2.GetShiftY(), 13);

    std::stringstream str2;
    str2 << frameset_info;
    ViewInfo::FramesetInfo frameset_info3;
    str2 >> frameset_info3;
    ASSERT_TRUE(ViewInfo::FramesetInfo::IsSameSprites(frameset_info, frameset_info3));
}

TEST(FramesetInfo, Hash)
{
    ViewInfo::FramesetInfo frameset_info;
    ASSERT_EQ(hash(frameset_info), 2);

    frameset_info.SetSprite("sprite 1");
    ASSERT_EQ(hash(frameset_info), 2014229692);

    frameset_info.SetState("state 2");
    ASSERT_EQ(hash(frameset_info), 230996800);

    frameset_info.SetAngle(84);
    ASSERT_EQ(hash(frameset_info), 230996884);

    frameset_info.SetShift(0, 13);
    ASSERT_EQ(hash(frameset_info), 230996897);

    frameset_info.SetShift(14, 13);
    ASSERT_EQ(hash(frameset_info), 230996911);
}

TEST(ViewInfo, AngleAndBaseFrameset)
{
    ViewInfo view_info;
    ASSERT_EQ(view_info.GetAngle(), 0);

    view_info.SetSprite("windows better than linux");
    ASSERT_EQ(view_info.GetBaseFrameset().GetSprite(), "windows better than linux");
    ASSERT_EQ(view_info.GetBaseFrameset().GetState(), "");
    ASSERT_EQ(view_info.GetAngle(), 0);

    view_info.SetState("lol jk");
    ASSERT_EQ(view_info.GetBaseFrameset().GetSprite(), "windows better than linux");
    ASSERT_EQ(view_info.GetBaseFrameset().GetState(), "lol jk");
    ASSERT_EQ(view_info.GetAngle(), 0);

    view_info.SetAngle(33);
    ASSERT_EQ(view_info.GetBaseFrameset().GetSprite(), "windows better than linux");
    ASSERT_EQ(view_info.GetBaseFrameset().GetState(), "lol jk");
    ASSERT_EQ(view_info.GetAngle(), 33);
}

TEST(ViewInfo, OverlaysAndUnderlays)
{
    ViewInfo view_info;
    ASSERT_EQ(view_info.GetOverlays().size(), 0);
    ASSERT_EQ(view_info.GetUnderlays().size(), 0);

    {
        ViewInfo::FramesetInfo& overlay
            = view_info.AddOverlay("bom bom", "bim bom");
        ASSERT_EQ(view_info.GetOverlays().size(), 1);
        ASSERT_EQ(view_info.GetUnderlays().size(), 0);
        ASSERT_EQ(view_info.GetOverlays()[0].GetSprite(), "bom bom");
        ASSERT_EQ(view_info.GetOverlays()[0].GetState(), "bim bom");
        ASSERT_EQ(&view_info.GetOverlays()[0], &overlay);
    }

    {
        ViewInfo::FramesetInfo& overlay
            = view_info.AddOverlay("bom bom2", "bim bom2");
        ASSERT_EQ(view_info.GetOverlays().size(), 2);
        ASSERT_EQ(view_info.GetUnderlays().size(), 0);
        ASSERT_EQ(view_info.GetOverlays()[1].GetSprite(), "bom bom2");
        ASSERT_EQ(view_info.GetOverlays()[1].GetState(), "bim bom2");
        ASSERT_EQ(&view_info.GetOverlays()[1], &overlay);
    }

    {
        ViewInfo::FramesetInfo& underlay
            = view_info.AddUnderlay("image", "state");
        ASSERT_EQ(view_info.GetOverlays().size(), 2);
        ASSERT_EQ(view_info.GetUnderlays().size(), 1);
        ASSERT_EQ(view_info.GetUnderlays()[0].GetSprite(), "image");
        ASSERT_EQ(view_info.GetUnderlays()[0].GetState(), "state");
        ASSERT_EQ(&view_info.GetUnderlays()[0], &underlay);
    }

    {
        ViewInfo::FramesetInfo& underlay
            = view_info.AddUnderlay("image2", "state2");
        ASSERT_EQ(view_info.GetOverlays().size(), 2);
        ASSERT_EQ(view_info.GetUnderlays().size(), 2);
        ASSERT_EQ(view_info.GetUnderlays()[1].GetSprite(), "image2");
        ASSERT_EQ(view_info.GetUnderlays()[1].GetState(), "state2");
        ASSERT_EQ(&view_info.GetUnderlays()[1], &underlay);
    }

    view_info.RemoveOverlays();
    ASSERT_EQ(view_info.GetOverlays().size(), 0);
    ASSERT_EQ(view_info.GetUnderlays().size(), 2);

    view_info.RemoveUnderlays();
    ASSERT_EQ(view_info.GetOverlays().size(), 0);
    ASSERT_EQ(view_info.GetUnderlays().size(), 0);
}

TEST(ViewInfo, IsSameFramesets)
{
    ViewInfo view_info;
    ASSERT_TRUE(ViewInfo::IsSameFramesets(view_info, view_info));

    view_info.SetAngle(10);
    view_info.SetSprite("sprite");
    view_info.SetState("state");
    view_info.AddOverlay("1", "1");
    view_info.AddUnderlay("2", "2");
    ASSERT_TRUE(ViewInfo::IsSameFramesets(view_info, view_info));

    ViewInfo view_info2;
    ASSERT_FALSE(ViewInfo::IsSameFramesets(view_info, view_info2));

    view_info2.SetAngle(11);
    view_info2.SetSprite("sprite");
    view_info2.SetState("state");
    ASSERT_FALSE(ViewInfo::IsSameFramesets(view_info, view_info2));

    view_info2.SetAngle(10);
    view_info2.SetSprite("sprite1");
    view_info2.SetState("state");
    ASSERT_FALSE(ViewInfo::IsSameFramesets(view_info, view_info2));

    view_info2.SetAngle(10);
    view_info2.SetSprite("sprite");
    view_info2.SetState("state1");
    ASSERT_FALSE(ViewInfo::IsSameFramesets(view_info, view_info2));

    view_info2.AddUnderlay("vvv", "wwww");
    ASSERT_FALSE(ViewInfo::IsSameFramesets(view_info, view_info2));

    view_info2.RemoveUnderlays();
    view_info2.AddUnderlay("2", "2");
    ASSERT_FALSE(ViewInfo::IsSameFramesets(view_info, view_info2));

    view_info2.SetAngle(10);
    view_info2.SetSprite("sprite");
    view_info2.SetState("state");
    ASSERT_FALSE(ViewInfo::IsSameFramesets(view_info, view_info2));

    view_info2.AddOverlay("vvv", "wwww");
    ASSERT_FALSE(ViewInfo::IsSameFramesets(view_info, view_info2));

    view_info2.RemoveOverlays();
    view_info2.AddOverlay("1", "1");
    ASSERT_TRUE(ViewInfo::IsSameFramesets(view_info, view_info2));

    view_info2.AddOverlay("111", "111");
    ASSERT_FALSE(ViewInfo::IsSameFramesets(view_info, view_info2));

    view_info2.RemoveOverlays();
    view_info2.AddOverlay("1", "1");
    view_info2.AddUnderlay("2", "2");
    ASSERT_FALSE(ViewInfo::IsSameFramesets(view_info, view_info2));

    view_info2.RemoveUnderlays();
    view_info2.AddUnderlay("2", "2");
    view_info2.SetAngle(88);
    ASSERT_FALSE(ViewInfo::IsSameFramesets(view_info, view_info2));

    view_info2.SetAngle(10);
    view_info2.SetState("spqr");
    ASSERT_FALSE(ViewInfo::IsSameFramesets(view_info, view_info2));

    view_info2.SetState("state");
    view_info2.RemoveUnderlays();
    view_info2.AddUnderlay("2fkds;a", "2fdjsl;kjf");
    ASSERT_FALSE(ViewInfo::IsSameFramesets(view_info, view_info2));
}

TEST(ViewInfo, StreamOperators)
{
    ViewInfo view_info;  
    {
        std::stringstream str;
        str << view_info;
        ASSERT_EQ(str.str(), "0  0  0 0 0  0 0 0 ");
    }

    view_info.SetSprite("vhs");
    view_info.SetState("dead");
    view_info.SetAngle(11);
    view_info.AddOverlay("something", "someone").SetShift(1, 10);
    view_info.AddUnderlay("tree", "weed");
    {
        std::stringstream str;
        str << view_info;
        ASSERT_EQ(
            str.str(),
            "3 vhs 4 dead 0 0 0  1 4 tree 4 weed 0 0 0  1 "
            "9 something 7 someone 0 1 10  11 ");
    }

    {
        std::stringstream str("0  0  0 0 0  0 0 0 ");
        str >> view_info;
        ASSERT_EQ(view_info.GetAngle(), 0);
        ASSERT_EQ(view_info.GetBaseFrameset().GetSprite(), "");
        ASSERT_EQ(view_info.GetBaseFrameset().GetState(), "");
        ASSERT_EQ(view_info.GetOverlays().size(), 0);
        ASSERT_EQ(view_info.GetUnderlays().size(), 0);
    }

    {
        std::stringstream str(
            "4 vhsa 5 dea1d 0 0 0  1 4 tree 4 weed 0 0 0  1 "
            "8 smething 7 someone 0 1 10  11 ");
        str >> view_info;
        ASSERT_EQ(view_info.GetAngle(), 11);
        ASSERT_EQ(view_info.GetBaseFrameset().GetSprite(), "vhsa");
        ASSERT_EQ(view_info.GetBaseFrameset().GetState(), "dea1d");

        ASSERT_EQ(view_info.GetOverlays().size(), 1);
        ViewInfo::FramesetInfo frameset_info1 = view_info.GetOverlays()[0];
        ASSERT_EQ(frameset_info1.GetSprite(), "smething");
        ASSERT_EQ(frameset_info1.GetState(), "someone");
        ASSERT_EQ(frameset_info1.GetShiftX(), 1);
        ASSERT_EQ(frameset_info1.GetShiftY(), 10);

        ASSERT_EQ(view_info.GetUnderlays().size(), 1);
        ViewInfo::FramesetInfo frameset_info2 = view_info.GetUnderlays()[0];
        ASSERT_EQ(frameset_info2.GetSprite(), "tree");
        ASSERT_EQ(frameset_info2.GetState(), "weed");
    }
}

TEST(ViewInfo, Hash)
{
    ViewInfo view_info;
    ASSERT_EQ(hash(view_info), 2);

    view_info.SetAngle(10);
    ASSERT_EQ(hash(view_info), 12);

    view_info.SetSprite("sprite");
    ASSERT_EQ(hash(view_info), 2514701247);

    view_info.SetState("state");
    ASSERT_EQ(hash(view_info), 227251334);

    view_info.AddOverlay("1", "1");
    ASSERT_EQ(hash(view_info), 241341978);

    view_info.AddUnderlay("2", "2");
    ASSERT_EQ(hash(view_info), 834067624);
}






