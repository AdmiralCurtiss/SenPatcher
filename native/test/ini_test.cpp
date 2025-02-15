#include <string>
#include <string_view>

#include "gtest/gtest.h"

#include "util/ini.h"

TEST(Ini, WithMultiLineComments) {
    static constexpr const char INI[] =
        "; start\n"
        "; multiline\n"
        "[Section]\n"
        "; after\n"
        "; section\n"
        "Key1=Value1\n"
        "; after\n"
        "; key1\n"
        "Key2 = Value2\n"
        "; end\n"
        "; of\n"
        "; file\n";

    HyoutaUtils::Ini::IniFile ini;
    ASSERT_TRUE(ini.ParseExternalMemory(INI, sizeof(INI) - 1));

    const auto values = ini.GetValues();
    ASSERT_EQ(4, values.size());

    EXPECT_EQ("Section", values[0].Section);
    EXPECT_EQ("", values[0].Key);
    EXPECT_EQ("", values[0].Value);
    EXPECT_EQ("; start\n; multiline", values[0].Comment);

    EXPECT_EQ("Section", values[1].Section);
    EXPECT_EQ("Key1", values[1].Key);
    EXPECT_EQ("Value1", values[1].Value);
    EXPECT_EQ("; after\n; section", values[1].Comment);

    EXPECT_EQ("Section", values[2].Section);
    EXPECT_EQ("Key2", values[2].Key);
    EXPECT_EQ("Value2", values[2].Value);
    EXPECT_EQ("; after\n; key1", values[2].Comment);

    EXPECT_EQ("", values[3].Section);
    EXPECT_EQ("", values[3].Key);
    EXPECT_EQ("", values[3].Value);
    EXPECT_EQ("; end\n; of\n; file", values[3].Comment);

    return;
}

TEST(Ini, MalformedSection) {
    static constexpr const char INI1[] =
        "Section]\n"
        "Key1=Value1\n";
    static constexpr const char INI2[] =
        "[Section\n"
        "Key1=Value1\n";
    static constexpr const char INI3[] =
        "[Section]3\n"
        "Key1=Value1\n";
    static constexpr const char INI4[] =
        "4[Section]\n"
        "Key1=Value1\n";

    {
        HyoutaUtils::Ini::IniFile ini;
        EXPECT_FALSE(ini.ParseExternalMemory(INI1, sizeof(INI1) - 1));
    }
    {
        HyoutaUtils::Ini::IniFile ini;
        EXPECT_FALSE(ini.ParseExternalMemory(INI2, sizeof(INI2) - 1));
    }
    {
        HyoutaUtils::Ini::IniFile ini;
        EXPECT_FALSE(ini.ParseExternalMemory(INI3, sizeof(INI3) - 1));
    }
    {
        HyoutaUtils::Ini::IniFile ini;
        EXPECT_FALSE(ini.ParseExternalMemory(INI4, sizeof(INI4) - 1));
    }

    return;
}

TEST(Ini, MalformedKeyValue) {
    static constexpr const char INI1[] =
        "[Section]\n"
        "Key1Value1\n";
    static constexpr const char INI2[] =
        "[Section]\n"
        "=Value1\n";
    static constexpr const char INI3[] =
        "[Section]\n"
        "Key1=\n";
    static constexpr const char INI4[] =
        "[Section]\n"
        "=\n";

    {
        HyoutaUtils::Ini::IniFile ini;
        EXPECT_FALSE(ini.ParseExternalMemory(INI1, sizeof(INI1) - 1));
    }
    {
        HyoutaUtils::Ini::IniFile ini;
        EXPECT_FALSE(ini.ParseExternalMemory(INI2, sizeof(INI2) - 1));
    }
    {
        // empty value is okay
        HyoutaUtils::Ini::IniFile ini;
        EXPECT_TRUE(ini.ParseExternalMemory(INI3, sizeof(INI3) - 1));
        const auto* kvp = ini.FindValue("Section", "Key1");
        EXPECT_TRUE(kvp != nullptr);
        if (kvp != nullptr) {
            EXPECT_EQ("", kvp->Value);
        }
    }
    {
        HyoutaUtils::Ini::IniFile ini;
        EXPECT_FALSE(ini.ParseExternalMemory(INI4, sizeof(INI4) - 1));
    }

    return;
}
