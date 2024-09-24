#include <cstdint>
#include <vector>

#include "gtest/gtest.h"

#include "sen/sen_script_patcher.h"

TEST(SenScriptPatcher, ShiftData) {
    // standard forwards shifts
    {
        std::vector<char> tmp = {{0, 1, 2, 3, 4}};
        SenLib::SenScriptPatcher patcher(tmp);
        EXPECT_TRUE(patcher.ShiftData(1, 4, 2));
        EXPECT_EQ(std::vector<char>({{0, 3, 1, 2, 4}}), tmp);
    }
    {
        std::vector<char> tmp = {{0, 1, 2, 3, 4}};
        SenLib::SenScriptPatcher patcher(tmp);
        EXPECT_TRUE(patcher.ShiftData(1, 4, 1));
        EXPECT_EQ(std::vector<char>({{0, 2, 3, 1, 4}}), tmp);
    }

    // standard backwards shifts
    {
        std::vector<char> tmp = {{0, 1, 2, 3, 4}};
        SenLib::SenScriptPatcher patcher(tmp);
        EXPECT_TRUE(patcher.ShiftData(2, 1, 2));
        EXPECT_EQ(std::vector<char>({{0, 2, 3, 1, 4}}), tmp);
    }
    {
        std::vector<char> tmp = {{0, 1, 2, 3, 4}};
        SenLib::SenScriptPatcher patcher(tmp);
        EXPECT_TRUE(patcher.ShiftData(3, 1, 1));
        EXPECT_EQ(std::vector<char>({{0, 3, 1, 2, 4}}), tmp);
    }

    // start-to-end
    {
        std::vector<char> tmp = {{0, 1, 2, 3, 4}};
        SenLib::SenScriptPatcher patcher(tmp);
        EXPECT_TRUE(patcher.ShiftData(0, 5, 2));
        EXPECT_EQ(std::vector<char>({{2, 3, 4, 0, 1}}), tmp);
    }
    {
        std::vector<char> tmp = {{0, 1, 2, 3, 4}};
        SenLib::SenScriptPatcher patcher(tmp);
        EXPECT_TRUE(patcher.ShiftData(0, 5, 3));
        EXPECT_EQ(std::vector<char>({{3, 4, 0, 1, 2}}), tmp);
    }

    // end-to-start
    {
        std::vector<char> tmp = {{0, 1, 2, 3, 4}};
        SenLib::SenScriptPatcher patcher(tmp);
        EXPECT_TRUE(patcher.ShiftData(3, 0, 2));
        EXPECT_EQ(std::vector<char>({{3, 4, 0, 1, 2}}), tmp);
    }
    {
        std::vector<char> tmp = {{0, 1, 2, 3, 4}};
        SenLib::SenScriptPatcher patcher(tmp);
        EXPECT_TRUE(patcher.ShiftData(2, 0, 3));
        EXPECT_EQ(std::vector<char>({{2, 3, 4, 0, 1}}), tmp);
    }

    // no-ops
    {
        std::vector<char> tmp = {{0, 1, 2, 3, 4}};
        SenLib::SenScriptPatcher patcher(tmp);
        EXPECT_TRUE(patcher.ShiftData(1, 4, 0));
        EXPECT_EQ(std::vector<char>({{0, 1, 2, 3, 4}}), tmp);
    }
    {
        std::vector<char> tmp = {{0, 1, 2, 3, 4}};
        SenLib::SenScriptPatcher patcher(tmp);
        EXPECT_TRUE(patcher.ShiftData(1, 1, 3));
        EXPECT_EQ(std::vector<char>({{0, 1, 2, 3, 4}}), tmp);
    }
    {
        std::vector<char> tmp = {{0, 1, 2, 3, 4}};
        SenLib::SenScriptPatcher patcher(tmp);
        EXPECT_TRUE(patcher.ShiftData(2, 4, 2));
        EXPECT_EQ(std::vector<char>({{0, 1, 2, 3, 4}}), tmp);
    }

    // invalid parameter combinations
    {
        std::vector<char> tmp = {{0, 1, 2, 3, 4}};
        SenLib::SenScriptPatcher patcher(tmp);

        // out of bounds
        EXPECT_FALSE(patcher.ShiftData(10, 4, 2));
        EXPECT_FALSE(patcher.ShiftData(1, 10, 2));
        EXPECT_FALSE(patcher.ShiftData(1, 4, 10));
        EXPECT_FALSE(patcher.ShiftData(4, 0, 3));

        // target inside source range
        EXPECT_FALSE(patcher.ShiftData(1, 2, 2));
        EXPECT_FALSE(patcher.ShiftData(1, 2, 3));
        EXPECT_FALSE(patcher.ShiftData(1, 3, 3));
    }
}
