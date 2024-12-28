#include <string>
#include <string_view>

#include "gtest/gtest.h"

#include "util/text.h"

TEST(TextGlob, GlobMatches) {
    EXPECT_TRUE(HyoutaUtils::TextUtils::GlobMatches("", ""));
    EXPECT_TRUE(HyoutaUtils::TextUtils::GlobMatches("", "*"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::GlobMatches("anything", "*"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::GlobMatches("abc", "abc"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::GlobMatches("abc", "a?c"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::GlobMatches("abc", "a*"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::GlobMatches("abc", "*c"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::GlobMatches("abc", "?b?"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::GlobMatches("abc", "*b*"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::GlobMatches("longer", "l????r"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::GlobMatches("longer", "lon*"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::GlobMatches("longer", "l*er"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::GlobMatches("longer", "?on*"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::GlobMatches("something else", "*thing*"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::GlobMatches("something else", "*ome*hin*els*"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::GlobMatches("something else", "*something else"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::GlobMatches("something else", "something else*"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::GlobMatches("something else", "*something else*"));

    EXPECT_FALSE(HyoutaUtils::TextUtils::GlobMatches("", "?"));
    EXPECT_FALSE(HyoutaUtils::TextUtils::GlobMatches("anything", ""));
    EXPECT_FALSE(HyoutaUtils::TextUtils::GlobMatches("abc", "ABC"));
    EXPECT_FALSE(HyoutaUtils::TextUtils::GlobMatches("abc", "?"));
    EXPECT_FALSE(HyoutaUtils::TextUtils::GlobMatches("abc", "??"));
    EXPECT_FALSE(HyoutaUtils::TextUtils::GlobMatches("abc", "??a"));
    EXPECT_FALSE(HyoutaUtils::TextUtils::GlobMatches("abc", "c??"));
    EXPECT_FALSE(HyoutaUtils::TextUtils::GlobMatches("abc", "a*x"));
    EXPECT_FALSE(HyoutaUtils::TextUtils::GlobMatches("abc", "*x*"));
    EXPECT_FALSE(HyoutaUtils::TextUtils::GlobMatches("longer", "lon?"));
    EXPECT_FALSE(HyoutaUtils::TextUtils::GlobMatches("longer", "l?ngr"));
    EXPECT_FALSE(HyoutaUtils::TextUtils::GlobMatches("something else", "*something  else*"));
}

TEST(TextGlob, CaseInsensitiveGlobMatches) {
    EXPECT_TRUE(HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches("anything", "*"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches("Abc", "ABC"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches("aBc", "AbC"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches("abC", "?B?"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches("abcDe", "*B*"));
    EXPECT_TRUE(HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches("abcdE", "*Bc*"));

    EXPECT_FALSE(HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches("", "?"));
    EXPECT_FALSE(HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches("anything", ""));
    EXPECT_FALSE(HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches("abc", "?"));
    EXPECT_FALSE(HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches("abc", "??"));
    EXPECT_FALSE(HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches("abc", "??a"));
    EXPECT_FALSE(HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches("abc", "c??"));
    EXPECT_FALSE(HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches("abc", "a*x"));
    EXPECT_FALSE(HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches("abc", "*x*"));
    EXPECT_FALSE(HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches("abc", "*X*"));
}
