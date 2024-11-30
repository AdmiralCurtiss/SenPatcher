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
