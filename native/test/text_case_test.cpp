#include <string>
#include <string_view>

#include "gtest/gtest.h"

#include "util/text.h"

TEST(TextCase, ToLower) {
    EXPECT_EQ("", HyoutaUtils::TextUtils::ToLower(""));
    EXPECT_EQ("a", HyoutaUtils::TextUtils::ToLower("a"));
    EXPECT_EQ("a", HyoutaUtils::TextUtils::ToLower("A"));
    EXPECT_EQ("c:\\something", HyoutaUtils::TextUtils::ToLower("C:\\SomeThing"));
    EXPECT_EQ("\r\n\t", HyoutaUtils::TextUtils::ToLower("\r\n\t"));
    EXPECT_EQ("\xe2\x98\x85", HyoutaUtils::TextUtils::ToLower("\xe2\x98\x85"));
    EXPECT_EQ("\xef\xbd\x8d", HyoutaUtils::TextUtils::ToLower("\xef\xbd\x8d"));
    EXPECT_EQ("\xef\xbc\xad", HyoutaUtils::TextUtils::ToLower("\xef\xbc\xad"));
}

TEST(TextCase, ToUpper) {
    EXPECT_EQ("", HyoutaUtils::TextUtils::ToUpper(""));
    EXPECT_EQ("A", HyoutaUtils::TextUtils::ToUpper("a"));
    EXPECT_EQ("A", HyoutaUtils::TextUtils::ToUpper("A"));
    EXPECT_EQ("C:\\SOMETHING", HyoutaUtils::TextUtils::ToUpper("C:\\SomeThing"));
    EXPECT_EQ("\r\n\t", HyoutaUtils::TextUtils::ToUpper("\r\n\t"));
    EXPECT_EQ("\xe2\x98\x85", HyoutaUtils::TextUtils::ToUpper("\xe2\x98\x85"));
    EXPECT_EQ("\xef\xbd\x8d", HyoutaUtils::TextUtils::ToUpper("\xef\xbd\x8d"));
    EXPECT_EQ("\xef\xbc\xad", HyoutaUtils::TextUtils::ToUpper("\xef\xbc\xad"));
}

template<typename T>
static int Signum(T number) {
    if (number < 0) {
        return -1;
    }
    if (number > 0) {
        return 1;
    }
    return 0;
}

static void TestCompare(std::string_view lhs, std::string_view rhs) {
    auto lhs_lower = HyoutaUtils::TextUtils::ToLower(lhs);
    auto rhs_lower = HyoutaUtils::TextUtils::ToLower(rhs);
    EXPECT_TRUE(Signum(HyoutaUtils::TextUtils::CaseInsensitiveCompare(lhs, rhs))
                == Signum(lhs_lower.compare(rhs_lower)));
    EXPECT_TRUE(Signum(HyoutaUtils::TextUtils::CaseInsensitiveCompare(rhs, lhs))
                == Signum(rhs_lower.compare(lhs_lower)));
}

TEST(TextCase, Compare) {
    using HyoutaUtils::TextUtils::CaseInsensitiveCompare;
    TestCompare("", "");
    TestCompare("a", "a");
    TestCompare("a", "A");
    TestCompare("a", "b");
    TestCompare("a", "B");
    TestCompare("A", "b");
    TestCompare("A", "B");
    TestCompare("Aa", "a");
    TestCompare("A", "aA");
    TestCompare("left", "RIGHT");
    TestCompare("a_B", "aBc");
}
