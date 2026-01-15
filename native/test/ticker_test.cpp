#include "gtest/gtest.h"

#include "util/ticker.h"

TEST(Ticker, Test) {
    HyoutaUtils::Ticker t;
    HyoutaUtils::InitializeTicker(t, 100, 1);
    EXPECT_EQ(3, HyoutaUtils::AdvanceTicker(t, 360));
    EXPECT_EQ(0, HyoutaUtils::AdvanceTicker(t, 35));
    EXPECT_EQ(1, HyoutaUtils::AdvanceTicker(t, 5));

    HyoutaUtils::InitializeTicker(t, 300, 200);
    EXPECT_EQ(0, HyoutaUtils::AdvanceTicker(t, 1));
    EXPECT_EQ(1, HyoutaUtils::AdvanceTicker(t, 1));
    EXPECT_EQ(1, HyoutaUtils::AdvanceTicker(t, 1));
    EXPECT_EQ(0, HyoutaUtils::AdvanceTicker(t, 1));
    EXPECT_EQ(1, HyoutaUtils::AdvanceTicker(t, 1));
    EXPECT_EQ(1, HyoutaUtils::AdvanceTicker(t, 1));

    HyoutaUtils::InitializeTicker(t, 10'000'000, 60);
    EXPECT_EQ(60, HyoutaUtils::AdvanceTicker(t, 10'000'000));
    EXPECT_EQ(6, HyoutaUtils::AdvanceTicker(t, 1'000'000));
    EXPECT_EQ(3, HyoutaUtils::AdvanceTicker(t, 500'000));
    EXPECT_EQ(2, HyoutaUtils::AdvanceTicker(t, 400'000));
    EXPECT_EQ(4, HyoutaUtils::AdvanceTicker(t, 600'000));

    return;
}
