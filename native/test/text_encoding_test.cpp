#include <span>
#include <string>
#include <string_view>

#include "gtest/gtest.h"

#include "util/text.h"

static std::string StringFromBytes(std::span<const char> bytes) {
    std::string s;
    s.reserve(bytes.size());
    for (size_t i = 0; i < bytes.size(); ++i) {
        s.push_back(bytes[i]);
    }
    return s;
}

static std::u16string U16StrFromBytes(std::span<const char> bytes) {
    if ((bytes.size() % 2) != 0) {
        throw "invalid utf16 bytes";
    }

    std::u16string s;
    s.reserve(bytes.size() / 2);
    for (size_t i = 0; i < bytes.size(); i += 2) {
        char16_t c = char16_t(uint16_t(uint8_t(bytes[i])) | (uint16_t(uint8_t(bytes[i + 1])) << 8));
        s.push_back(c);
    }
    return s;
}

static void
    TestUtf8Utf16ShiftJis(std::string_view utf8, std::u16string_view utf16, std::string_view sjis) {
    EXPECT_EQ(utf8, HyoutaUtils::TextUtils::Utf16ToUtf8(utf16.data(), utf16.size()));
    EXPECT_EQ(sjis, HyoutaUtils::TextUtils::Utf16ToShiftJis(utf16.data(), utf16.size()));
    EXPECT_EQ(utf16, HyoutaUtils::TextUtils::Utf8ToUtf16(utf8.data(), utf8.size()));
    EXPECT_EQ(sjis, HyoutaUtils::TextUtils::Utf8ToShiftJis(utf8.data(), utf8.size()));
    EXPECT_EQ(utf8, HyoutaUtils::TextUtils::ShiftJisToUtf8(sjis.data(), sjis.size()));
    EXPECT_EQ(utf16, HyoutaUtils::TextUtils::ShiftJisToUtf16(sjis.data(), sjis.size()));
}

static void TestUtf8Utf16(std::string_view utf8, std::u16string_view utf16) {
    EXPECT_EQ(utf8, HyoutaUtils::TextUtils::Utf16ToUtf8(utf16.data(), utf16.size()));
    EXPECT_EQ(utf16, HyoutaUtils::TextUtils::Utf8ToUtf16(utf8.data(), utf8.size()));
}

static void TestInvalidUtf16(std::u16string_view utf16) {
    EXPECT_EQ(std::nullopt, HyoutaUtils::TextUtils::Utf16ToUtf8(utf16.data(), utf16.size()));
    EXPECT_EQ(std::nullopt, HyoutaUtils::TextUtils::Utf16ToShiftJis(utf16.data(), utf16.size()));
}

static void TestInvalidUtf8(std::string_view utf8) {
    EXPECT_EQ(std::nullopt, HyoutaUtils::TextUtils::Utf8ToUtf16(utf8.data(), utf8.size()));
    EXPECT_EQ(std::nullopt, HyoutaUtils::TextUtils::Utf8ToShiftJis(utf8.data(), utf8.size()));
}

static void TestInvalidShiftJis(std::string_view sjis) {
    EXPECT_EQ(std::nullopt, HyoutaUtils::TextUtils::ShiftJisToUtf8(sjis.data(), sjis.size()));
    EXPECT_EQ(std::nullopt, HyoutaUtils::TextUtils::ShiftJisToUtf16(sjis.data(), sjis.size()));
}

TEST(TextEncoding, Utf8Utf16ShiftJis) {
    // clang-format off
    TestUtf8Utf16ShiftJis(
        StringFromBytes({}),
        U16StrFromBytes({}),
        StringFromBytes({})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{0}}),
        U16StrFromBytes({{0, 0}}),
        StringFromBytes({{0}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{0x0, 0xa}}),
        U16StrFromBytes({{0x0, 0x0, 0xa, 0x0}}),
        StringFromBytes({{0x0, 0xa}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{0x54, 0x65, 0x73, 0x74}}),
        U16StrFromBytes({{0x54, 0x00, 0x65, 0x00, 0x73, 0x00, 0x74, 0x00}}),
        StringFromBytes({{0x54, 0x65, 0x73, 0x74}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{0x3c, 0x78, 0x3e}}),
        U16StrFromBytes({{0x3c, 0x00, 0x78, 0x00, 0x3e, 0x00}}),
        StringFromBytes({{0x3c, 0x78, 0x3e}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xef, (char)0xbd, (char)0xa2, (char)0xe8, (char)0xbb, (char)0x8c, (char)0xe8, (char)0xb7, (char)0xa1, (char)0xef, (char)0xbd, (char)0xa3}}),
        U16StrFromBytes({{(char)0x62, (char)0xff, (char)0xcc, (char)0x8e, (char)0xe1, (char)0x8d, (char)0x63, (char)0xff}}),
        StringFromBytes({{(char)0xa2, (char)0x8b, (char)0x4f, (char)0x90, (char)0xd5, (char)0xa3}})
    );

    // windows codepage stuff, shift-jis has a few conflicting definitions, make sure we have the windows variant because that's what games tend to use
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0x5c}}),
        U16StrFromBytes({{(char)0x5c, (char)0x00}}),
        StringFromBytes({{(char)0x5c}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0x7e}}),
        U16StrFromBytes({{(char)0x7e, (char)0x00}}),
        StringFromBytes({{(char)0x7e}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xef, (char)0xbf, (char)0xa2, (char)0xe2, (char)0x88, (char)0xb5, (char)0xe2, (char)0x88, (char)0xb4, (char)0xe3, (char)0x8e, (char)0x8f}}),
        U16StrFromBytes({{(char)0xe2, (char)0xff, (char)0x35, (char)0x22, (char)0x34, (char)0x22, (char)0x8f, (char)0x33}}),
        StringFromBytes({{(char)0x81, (char)0xca, (char)0x81, (char)0xe6, (char)0x81, (char)0x88, (char)0x87, (char)0x73}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xe2, (char)0x91, (char)0xb2, (char)0xe2, (char)0x91, (char)0xb3, (char)0xe2, (char)0x85, (char)0xa0, (char)0xe3, (char)0x88, (char)0xb1, (char)0xef, (char)0xa8, (char)0x91, (char)0xe7, (char)0x80, (char)0xa8, (char)0xe9, (char)0xab, (char)0x99}}),
        U16StrFromBytes({{(char)0x72, (char)0x24, (char)0x73, (char)0x24, (char)0x60, (char)0x21, (char)0x31, (char)0x32, (char)0x11, (char)0xfa, (char)0x28, (char)0x70, (char)0xd9, (char)0x9a}}),
        StringFromBytes({{(char)0x87, (char)0x52, (char)0x87, (char)0x53, (char)0x87, (char)0x54, (char)0x87, (char)0x8a, (char)0xfa, (char)0xb1, (char)0xfb, (char)0x50, (char)0xfb, (char)0xfc}})
    );

    // utf16 with surrogate pair
    TestUtf8Utf16(
        StringFromBytes({{(char)0xf0, (char)0xa4, (char)0xad, (char)0xa2}}),
        U16StrFromBytes({{(char)0x52, (char)0xd8, (char)0x62, (char)0xdf}})
    );

    // invalid utf16 surrogate pairs
    TestInvalidUtf16(
        U16StrFromBytes({{(char)0x62, (char)0xdf, (char)0x52, (char)0xd8}})
    );
    TestInvalidUtf16(
        U16StrFromBytes({{(char)0x52, (char)0xd8}})
    );
    TestInvalidUtf16(
        U16StrFromBytes({{(char)0x62, (char)0xdf}})
    );
    TestInvalidUtf16(
        U16StrFromBytes({{(char)0x25, (char)0x25, (char)0x62, (char)0xdf, (char)0x25, (char)0x25}})
    );
    TestInvalidUtf16(
        U16StrFromBytes({{(char)0x25, (char)0x25, (char)0x52, (char)0xd8, (char)0x25, (char)0x25}})
    );

    // invalid utf8 strings
    TestInvalidUtf8(
        StringFromBytes({{(char)0x80}})
    );
    TestInvalidUtf8(
        StringFromBytes({{(char)0xc2}})
    );
    TestInvalidUtf8(
        StringFromBytes({{(char)0xc2, (char)0x00}})
    );
    TestInvalidUtf8(
        StringFromBytes({{(char)0xe0}})
    );
    TestInvalidUtf8(
        StringFromBytes({{(char)0xe0, (char)0xa0}})
    );
    TestInvalidUtf8(
        StringFromBytes({{(char)0xe0, (char)0x80, (char)0x80}})
    );
    TestInvalidUtf8(
        StringFromBytes({{(char)0xe1, (char)0x25, (char)0x80}})
    );
    TestInvalidUtf8(
        StringFromBytes({{(char)0xe1, (char)0x80, (char)0x25}})
    );
    TestInvalidUtf8(
        StringFromBytes({{(char)0xed, (char)0xa0, (char)0x80}})
    );
    TestInvalidUtf8(
        StringFromBytes({{(char)0xf0}})
    );
    TestInvalidUtf8(
        StringFromBytes({{(char)0xf0, (char)0x90}})
    );
    TestInvalidUtf8(
        StringFromBytes({{(char)0xf0, (char)0x90, (char)0x80}})
    );
    TestInvalidUtf8(
        StringFromBytes({{(char)0xf0, (char)0x80, (char)0x80, (char)0x80}})
    );
    TestInvalidUtf8(
        StringFromBytes({{(char)0xf1, (char)0x25, (char)0x80, (char)0x80}})
    );
    TestInvalidUtf8(
        StringFromBytes({{(char)0xf1, (char)0x80, (char)0x25, (char)0x80}})
    );
    TestInvalidUtf8(
        StringFromBytes({{(char)0xf1, (char)0x80, (char)0x80, (char)0x25}})
    );

    // invalid sjis strings
    TestInvalidShiftJis(
        StringFromBytes({{(char)0x81}})
    );
    TestInvalidShiftJis(
        StringFromBytes({{(char)0x81, (char)0x25}})
    );
    TestInvalidShiftJis(
        StringFromBytes({{(char)0x81, (char)0x7f}})
    );
    TestInvalidShiftJis(
        StringFromBytes({{(char)0x81, (char)0xfd}})
    );
    TestInvalidShiftJis(
        StringFromBytes({{(char)0xa0, (char)0x41}})
    );
    TestInvalidShiftJis(
        StringFromBytes({{(char)0xeb, (char)0x41}})
    );
    // clang-format on
}
