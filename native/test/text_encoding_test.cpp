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
    EXPECT_EQ(utf8.size(), HyoutaUtils::TextUtils::MeasureUtf16ToUtf8(utf16.data(), utf16.size()));
    EXPECT_EQ(sjis.size(),
              HyoutaUtils::TextUtils::MeasureUtf16ToShiftJis(utf16.data(), utf16.size()));
    EXPECT_EQ(utf16.size(), HyoutaUtils::TextUtils::MeasureUtf8ToUtf16(utf8.data(), utf8.size()));
    EXPECT_EQ(sjis.size(), HyoutaUtils::TextUtils::MeasureUtf8ToShiftJis(utf8.data(), utf8.size()));
    EXPECT_EQ(utf8.size(), HyoutaUtils::TextUtils::MeasureShiftJisToUtf8(sjis.data(), sjis.size()));
    EXPECT_EQ(utf16.size(),
              HyoutaUtils::TextUtils::MeasureShiftJisToUtf16(sjis.data(), sjis.size()));

    std::string utf8_from_utf16(utf8.size(), '\0');
    std::string utf8_from_sjis(utf8.size(), '\0');
    std::u16string utf16_from_utf8(utf16.size(), char16_t(0));
    std::u16string utf16_from_sjis(utf16.size(), char16_t(0));
    std::string sjis_from_utf8(sjis.size(), '\0');
    std::string sjis_from_utf16(sjis.size(), '\0');
    EXPECT_EQ(utf8.size(),
              HyoutaUtils::TextUtils::ConvertUtf16ToUtf8(
                  utf8_from_utf16.data(), utf8_from_utf16.size(), utf16.data(), utf16.size()));
    EXPECT_EQ(sjis.size(),
              HyoutaUtils::TextUtils::ConvertUtf16ToShiftJis(
                  sjis_from_utf16.data(), sjis_from_utf16.size(), utf16.data(), utf16.size()));
    EXPECT_EQ(utf16.size(),
              HyoutaUtils::TextUtils::ConvertUtf8ToUtf16(
                  utf16_from_utf8.data(), utf16_from_utf8.size(), utf8.data(), utf8.size()));
    EXPECT_EQ(sjis.size(),
              HyoutaUtils::TextUtils::ConvertUtf8ToShiftJis(
                  sjis_from_utf8.data(), sjis_from_utf8.size(), utf8.data(), utf8.size()));
    EXPECT_EQ(utf8.size(),
              HyoutaUtils::TextUtils::ConvertShiftJisToUtf8(
                  utf8_from_sjis.data(), utf8_from_sjis.size(), sjis.data(), sjis.size()));
    EXPECT_EQ(utf16.size(),
              HyoutaUtils::TextUtils::ConvertShiftJisToUtf16(
                  utf16_from_sjis.data(), utf16_from_sjis.size(), sjis.data(), sjis.size()));
    EXPECT_EQ(utf8, utf8_from_utf16);
    EXPECT_EQ(utf8, utf8_from_sjis);
    EXPECT_EQ(utf16, utf16_from_utf8);
    EXPECT_EQ(utf16, utf16_from_sjis);
    EXPECT_EQ(sjis, sjis_from_utf8);
    EXPECT_EQ(sjis, sjis_from_utf16);

    EXPECT_EQ(utf8, HyoutaUtils::TextUtils::Utf16ToUtf8(utf16.data(), utf16.size()));
    EXPECT_EQ(sjis, HyoutaUtils::TextUtils::Utf16ToShiftJis(utf16.data(), utf16.size()));
    EXPECT_EQ(utf16, HyoutaUtils::TextUtils::Utf8ToUtf16(utf8.data(), utf8.size()));
    EXPECT_EQ(sjis, HyoutaUtils::TextUtils::Utf8ToShiftJis(utf8.data(), utf8.size()));
    EXPECT_EQ(utf8, HyoutaUtils::TextUtils::ShiftJisToUtf8(sjis.data(), sjis.size()));
    EXPECT_EQ(utf16, HyoutaUtils::TextUtils::ShiftJisToUtf16(sjis.data(), sjis.size()));
}

static void TestUtf8Utf16(std::string_view utf8, std::u16string_view utf16) {
    EXPECT_EQ(utf8.size(), HyoutaUtils::TextUtils::MeasureUtf16ToUtf8(utf16.data(), utf16.size()));
    EXPECT_EQ(utf16.size(), HyoutaUtils::TextUtils::MeasureUtf8ToUtf16(utf8.data(), utf8.size()));

    std::string utf8_from_utf16(utf8.size(), '\0');
    std::u16string utf16_from_utf8(utf16.size(), char16_t(0));
    EXPECT_EQ(utf8.size(),
              HyoutaUtils::TextUtils::ConvertUtf16ToUtf8(
                  utf8_from_utf16.data(), utf8_from_utf16.size(), utf16.data(), utf16.size()));
    EXPECT_EQ(utf16.size(),
              HyoutaUtils::TextUtils::ConvertUtf8ToUtf16(
                  utf16_from_utf8.data(), utf16_from_utf8.size(), utf8.data(), utf8.size()));
    EXPECT_EQ(utf8, utf8_from_utf16);
    EXPECT_EQ(utf16, utf16_from_utf8);

    EXPECT_EQ(utf8, HyoutaUtils::TextUtils::Utf16ToUtf8(utf16.data(), utf16.size()));
    EXPECT_EQ(utf16, HyoutaUtils::TextUtils::Utf8ToUtf16(utf8.data(), utf8.size()));
}

static void TestInvalidUtf16(std::u16string_view utf16) {
    std::array<char, 128> byteBuffer;
    EXPECT_EQ(HyoutaUtils::TextUtils::INVALID_LENGTH,
              HyoutaUtils::TextUtils::MeasureUtf16ToUtf8(utf16.data(), utf16.size()));
    EXPECT_EQ(HyoutaUtils::TextUtils::INVALID_LENGTH,
              HyoutaUtils::TextUtils::MeasureUtf16ToShiftJis(utf16.data(), utf16.size()));
    EXPECT_EQ(HyoutaUtils::TextUtils::INVALID_LENGTH,
              HyoutaUtils::TextUtils::ConvertUtf16ToUtf8(
                  byteBuffer.data(), byteBuffer.size(), utf16.data(), utf16.size()));
    EXPECT_EQ(HyoutaUtils::TextUtils::INVALID_LENGTH,
              HyoutaUtils::TextUtils::ConvertUtf16ToShiftJis(
                  byteBuffer.data(), byteBuffer.size(), utf16.data(), utf16.size()));
    EXPECT_EQ(std::nullopt, HyoutaUtils::TextUtils::Utf16ToUtf8(utf16.data(), utf16.size()));
    EXPECT_EQ(std::nullopt, HyoutaUtils::TextUtils::Utf16ToShiftJis(utf16.data(), utf16.size()));
}

static void TestInvalidUtf8(std::string_view utf8) {
    std::array<char, 128> byteBuffer;
    std::array<char16_t, 128> c16Buffer;
    EXPECT_EQ(HyoutaUtils::TextUtils::INVALID_LENGTH,
              HyoutaUtils::TextUtils::MeasureUtf8ToUtf16(utf8.data(), utf8.size()));
    EXPECT_EQ(HyoutaUtils::TextUtils::INVALID_LENGTH,
              HyoutaUtils::TextUtils::MeasureUtf8ToShiftJis(utf8.data(), utf8.size()));
    EXPECT_EQ(HyoutaUtils::TextUtils::INVALID_LENGTH,
              HyoutaUtils::TextUtils::ConvertUtf8ToUtf16(
                  c16Buffer.data(), c16Buffer.size(), utf8.data(), utf8.size()));
    EXPECT_EQ(HyoutaUtils::TextUtils::INVALID_LENGTH,
              HyoutaUtils::TextUtils::ConvertUtf8ToShiftJis(
                  byteBuffer.data(), byteBuffer.size(), utf8.data(), utf8.size()));
    EXPECT_EQ(std::nullopt, HyoutaUtils::TextUtils::Utf8ToUtf16(utf8.data(), utf8.size()));
    EXPECT_EQ(std::nullopt, HyoutaUtils::TextUtils::Utf8ToShiftJis(utf8.data(), utf8.size()));
}

static void TestInvalidShiftJis(std::string_view sjis) {
    std::array<char, 128> byteBuffer;
    std::array<char16_t, 128> c16Buffer;
    EXPECT_EQ(HyoutaUtils::TextUtils::INVALID_LENGTH,
              HyoutaUtils::TextUtils::MeasureShiftJisToUtf8(sjis.data(), sjis.size()));
    EXPECT_EQ(HyoutaUtils::TextUtils::INVALID_LENGTH,
              HyoutaUtils::TextUtils::MeasureShiftJisToUtf16(sjis.data(), sjis.size()));
    EXPECT_EQ(HyoutaUtils::TextUtils::INVALID_LENGTH,
              HyoutaUtils::TextUtils::ConvertShiftJisToUtf8(
                  byteBuffer.data(), byteBuffer.size(), sjis.data(), sjis.size()));
    EXPECT_EQ(HyoutaUtils::TextUtils::INVALID_LENGTH,
              HyoutaUtils::TextUtils::ConvertShiftJisToUtf16(
                  c16Buffer.data(), c16Buffer.size(), sjis.data(), sjis.size()));
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
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xe2, (char)0x80, (char)0x95}}),
        U16StrFromBytes({{(char)0x15, (char)0x20}}),
        StringFromBytes({{(char)0x81, (char)0x5c}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xef, (char)0xbc, (char)0xbc}}),
        U16StrFromBytes({{(char)0x3c, (char)0xff}}),
        StringFromBytes({{(char)0x81, (char)0x5f}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xef, (char)0xbd, (char)0x9e}}),
        U16StrFromBytes({{(char)0x5e, (char)0xff}}),
        StringFromBytes({{(char)0x81, (char)0x60}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xe2, (char)0x88, (char)0xa5}}),
        U16StrFromBytes({{(char)0x25, (char)0x22}}),
        StringFromBytes({{(char)0x81, (char)0x61}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xef, (char)0xbc, (char)0x8d}}),
        U16StrFromBytes({{(char)0x0d, (char)0xff}}),
        StringFromBytes({{(char)0x81, (char)0x7c}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xef, (char)0xbf, (char)0xa0}}),
        U16StrFromBytes({{(char)0xe0, (char)0xff}}),
        StringFromBytes({{(char)0x81, (char)0x91}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xef, (char)0xbf, (char)0xa1}}),
        U16StrFromBytes({{(char)0xe1, (char)0xff}}),
        StringFromBytes({{(char)0x81, (char)0x92}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xef, (char)0xbf, (char)0xa2}}),
        U16StrFromBytes({{(char)0xe2, (char)0xff}}),
        StringFromBytes({{(char)0x81, (char)0xca}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xe2, (char)0x89, (char)0x92}}),
        U16StrFromBytes({{(char)0x52, (char)0x22}}),
        StringFromBytes({{(char)0x81, (char)0xe0}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xe2, (char)0x89, (char)0xa1}}),
        U16StrFromBytes({{(char)0x61, (char)0x22}}),
        StringFromBytes({{(char)0x81, (char)0xdf}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xe2, (char)0x88, (char)0xab}}),
        U16StrFromBytes({{(char)0x2b, (char)0x22}}),
        StringFromBytes({{(char)0x81, (char)0xe7}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xe2, (char)0x88, (char)0x91}}),
        U16StrFromBytes({{(char)0x11, (char)0x22}}),
        StringFromBytes({{(char)0x87, (char)0x94}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xe2, (char)0x88, (char)0x9a}}),
        U16StrFromBytes({{(char)0x1a, (char)0x22}}),
        StringFromBytes({{(char)0x81, (char)0xe3}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xe2, (char)0x8a, (char)0xa5}}),
        U16StrFromBytes({{(char)0xa5, (char)0x22}}),
        StringFromBytes({{(char)0x81, (char)0xdb}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xe2, (char)0x88, (char)0xa0}}),
        U16StrFromBytes({{(char)0x20, (char)0x22}}),
        StringFromBytes({{(char)0x81, (char)0xda}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xe2, (char)0x88, (char)0xb5}}),
        U16StrFromBytes({{(char)0x35, (char)0x22}}),
        StringFromBytes({{(char)0x81, (char)0xe6}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xe2, (char)0x88, (char)0xa9}}),
        U16StrFromBytes({{(char)0x29, (char)0x22}}),
        StringFromBytes({{(char)0x81, (char)0xbf}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xe2, (char)0x88, (char)0xaa}}),
        U16StrFromBytes({{(char)0x2a, (char)0x22}}),
        StringFromBytes({{(char)0x81, (char)0xbe}})
    );
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0xe3, (char)0x83, (char)0xbb}}),
        U16StrFromBytes({{(char)0xfb, (char)0x30}}),
        StringFromBytes({{(char)0x81, (char)0x45}})
    );

    // all single-byte shift-jis characters
    TestUtf8Utf16ShiftJis(
        StringFromBytes({{(char)0x00, (char)0x01, (char)0x02, (char)0x03, (char)0x04, (char)0x05, (char)0x06, (char)0x07, (char)0x08, (char)0x09, (char)0x0a, (char)0x0b, (char)0x0c, (char)0x0d, (char)0x0e, (char)0x0f, (char)0x10, (char)0x11, (char)0x12, (char)0x13, (char)0x14, (char)0x15, (char)0x16, (char)0x17, (char)0x18, (char)0x19, (char)0x1a, (char)0x1b, (char)0x1c, (char)0x1d, (char)0x1e, (char)0x1f, (char)0x20, (char)0x21, (char)0x22, (char)0x23, (char)0x24, (char)0x25, (char)0x26, (char)0x27, (char)0x28, (char)0x29, (char)0x2a, (char)0x2b, (char)0x2c, (char)0x2d, (char)0x2e, (char)0x2f, (char)0x30, (char)0x31, (char)0x32, (char)0x33, (char)0x34, (char)0x35, (char)0x36, (char)0x37, (char)0x38, (char)0x39, (char)0x3a, (char)0x3b, (char)0x3c, (char)0x3d, (char)0x3e, (char)0x3f, (char)0x40, (char)0x41, (char)0x42, (char)0x43, (char)0x44, (char)0x45, (char)0x46, (char)0x47, (char)0x48, (char)0x49, (char)0x4a, (char)0x4b, (char)0x4c, (char)0x4d, (char)0x4e, (char)0x4f, (char)0x50, (char)0x51, (char)0x52, (char)0x53, (char)0x54, (char)0x55, (char)0x56, (char)0x57, (char)0x58, (char)0x59, (char)0x5a, (char)0x5b, (char)0x5c, (char)0x5d, (char)0x5e, (char)0x5f, (char)0x60, (char)0x61, (char)0x62, (char)0x63, (char)0x64, (char)0x65, (char)0x66, (char)0x67, (char)0x68, (char)0x69, (char)0x6a, (char)0x6b, (char)0x6c, (char)0x6d, (char)0x6e, (char)0x6f, (char)0x70, (char)0x71, (char)0x72, (char)0x73, (char)0x74, (char)0x75, (char)0x76, (char)0x77, (char)0x78, (char)0x79, (char)0x7a, (char)0x7b, (char)0x7c, (char)0x7d, (char)0x7e, (char)0x7f, (char)0xc2, (char)0x80, (char)0xef, (char)0xbd, (char)0xa1, (char)0xef, (char)0xbd, (char)0xa2, (char)0xef, (char)0xbd, (char)0xa3, (char)0xef, (char)0xbd, (char)0xa4, (char)0xef, (char)0xbd, (char)0xa5, (char)0xef, (char)0xbd, (char)0xa6, (char)0xef, (char)0xbd, (char)0xa7, (char)0xef, (char)0xbd, (char)0xa8, (char)0xef, (char)0xbd, (char)0xa9, (char)0xef, (char)0xbd, (char)0xaa, (char)0xef, (char)0xbd, (char)0xab, (char)0xef, (char)0xbd, (char)0xac, (char)0xef, (char)0xbd, (char)0xad, (char)0xef, (char)0xbd, (char)0xae, (char)0xef, (char)0xbd, (char)0xaf, (char)0xef, (char)0xbd, (char)0xb0, (char)0xef, (char)0xbd, (char)0xb1, (char)0xef, (char)0xbd, (char)0xb2, (char)0xef, (char)0xbd, (char)0xb3, (char)0xef, (char)0xbd, (char)0xb4, (char)0xef, (char)0xbd, (char)0xb5, (char)0xef, (char)0xbd, (char)0xb6, (char)0xef, (char)0xbd, (char)0xb7, (char)0xef, (char)0xbd, (char)0xb8, (char)0xef, (char)0xbd, (char)0xb9, (char)0xef, (char)0xbd, (char)0xba, (char)0xef, (char)0xbd, (char)0xbb, (char)0xef, (char)0xbd, (char)0xbc, (char)0xef, (char)0xbd, (char)0xbd, (char)0xef, (char)0xbd, (char)0xbe, (char)0xef, (char)0xbd, (char)0xbf, (char)0xef, (char)0xbe, (char)0x80, (char)0xef, (char)0xbe, (char)0x81, (char)0xef, (char)0xbe, (char)0x82, (char)0xef, (char)0xbe, (char)0x83, (char)0xef, (char)0xbe, (char)0x84, (char)0xef, (char)0xbe, (char)0x85, (char)0xef, (char)0xbe, (char)0x86, (char)0xef, (char)0xbe, (char)0x87, (char)0xef, (char)0xbe, (char)0x88, (char)0xef, (char)0xbe, (char)0x89, (char)0xef, (char)0xbe, (char)0x8a, (char)0xef, (char)0xbe, (char)0x8b, (char)0xef, (char)0xbe, (char)0x8c, (char)0xef, (char)0xbe, (char)0x8d, (char)0xef, (char)0xbe, (char)0x8e, (char)0xef, (char)0xbe, (char)0x8f, (char)0xef, (char)0xbe, (char)0x90, (char)0xef, (char)0xbe, (char)0x91, (char)0xef, (char)0xbe, (char)0x92, (char)0xef, (char)0xbe, (char)0x93, (char)0xef, (char)0xbe, (char)0x94, (char)0xef, (char)0xbe, (char)0x95, (char)0xef, (char)0xbe, (char)0x96, (char)0xef, (char)0xbe, (char)0x97, (char)0xef, (char)0xbe, (char)0x98, (char)0xef, (char)0xbe, (char)0x99, (char)0xef, (char)0xbe, (char)0x9a, (char)0xef, (char)0xbe, (char)0x9b, (char)0xef, (char)0xbe, (char)0x9c, (char)0xef, (char)0xbe, (char)0x9d, (char)0xef, (char)0xbe, (char)0x9e, (char)0xef, (char)0xbe, (char)0x9f}}),
        U16StrFromBytes({{(char)0x00, (char)0x00, (char)0x01, (char)0x00, (char)0x02, (char)0x00, (char)0x03, (char)0x00, (char)0x04, (char)0x00, (char)0x05, (char)0x00, (char)0x06, (char)0x00, (char)0x07, (char)0x00, (char)0x08, (char)0x00, (char)0x09, (char)0x00, (char)0x0a, (char)0x00, (char)0x0b, (char)0x00, (char)0x0c, (char)0x00, (char)0x0d, (char)0x00, (char)0x0e, (char)0x00, (char)0x0f, (char)0x00, (char)0x10, (char)0x00, (char)0x11, (char)0x00, (char)0x12, (char)0x00, (char)0x13, (char)0x00, (char)0x14, (char)0x00, (char)0x15, (char)0x00, (char)0x16, (char)0x00, (char)0x17, (char)0x00, (char)0x18, (char)0x00, (char)0x19, (char)0x00, (char)0x1a, (char)0x00, (char)0x1b, (char)0x00, (char)0x1c, (char)0x00, (char)0x1d, (char)0x00, (char)0x1e, (char)0x00, (char)0x1f, (char)0x00, (char)0x20, (char)0x00, (char)0x21, (char)0x00, (char)0x22, (char)0x00, (char)0x23, (char)0x00, (char)0x24, (char)0x00, (char)0x25, (char)0x00, (char)0x26, (char)0x00, (char)0x27, (char)0x00, (char)0x28, (char)0x00, (char)0x29, (char)0x00, (char)0x2a, (char)0x00, (char)0x2b, (char)0x00, (char)0x2c, (char)0x00, (char)0x2d, (char)0x00, (char)0x2e, (char)0x00, (char)0x2f, (char)0x00, (char)0x30, (char)0x00, (char)0x31, (char)0x00, (char)0x32, (char)0x00, (char)0x33, (char)0x00, (char)0x34, (char)0x00, (char)0x35, (char)0x00, (char)0x36, (char)0x00, (char)0x37, (char)0x00, (char)0x38, (char)0x00, (char)0x39, (char)0x00, (char)0x3a, (char)0x00, (char)0x3b, (char)0x00, (char)0x3c, (char)0x00, (char)0x3d, (char)0x00, (char)0x3e, (char)0x00, (char)0x3f, (char)0x00, (char)0x40, (char)0x00, (char)0x41, (char)0x00, (char)0x42, (char)0x00, (char)0x43, (char)0x00, (char)0x44, (char)0x00, (char)0x45, (char)0x00, (char)0x46, (char)0x00, (char)0x47, (char)0x00, (char)0x48, (char)0x00, (char)0x49, (char)0x00, (char)0x4a, (char)0x00, (char)0x4b, (char)0x00, (char)0x4c, (char)0x00, (char)0x4d, (char)0x00, (char)0x4e, (char)0x00, (char)0x4f, (char)0x00, (char)0x50, (char)0x00, (char)0x51, (char)0x00, (char)0x52, (char)0x00, (char)0x53, (char)0x00, (char)0x54, (char)0x00, (char)0x55, (char)0x00, (char)0x56, (char)0x00, (char)0x57, (char)0x00, (char)0x58, (char)0x00, (char)0x59, (char)0x00, (char)0x5a, (char)0x00, (char)0x5b, (char)0x00, (char)0x5c, (char)0x00, (char)0x5d, (char)0x00, (char)0x5e, (char)0x00, (char)0x5f, (char)0x00, (char)0x60, (char)0x00, (char)0x61, (char)0x00, (char)0x62, (char)0x00, (char)0x63, (char)0x00, (char)0x64, (char)0x00, (char)0x65, (char)0x00, (char)0x66, (char)0x00, (char)0x67, (char)0x00, (char)0x68, (char)0x00, (char)0x69, (char)0x00, (char)0x6a, (char)0x00, (char)0x6b, (char)0x00, (char)0x6c, (char)0x00, (char)0x6d, (char)0x00, (char)0x6e, (char)0x00, (char)0x6f, (char)0x00, (char)0x70, (char)0x00, (char)0x71, (char)0x00, (char)0x72, (char)0x00, (char)0x73, (char)0x00, (char)0x74, (char)0x00, (char)0x75, (char)0x00, (char)0x76, (char)0x00, (char)0x77, (char)0x00, (char)0x78, (char)0x00, (char)0x79, (char)0x00, (char)0x7a, (char)0x00, (char)0x7b, (char)0x00, (char)0x7c, (char)0x00, (char)0x7d, (char)0x00, (char)0x7e, (char)0x00, (char)0x7f, (char)0x00, (char)0x80, (char)0x00, (char)0x61, (char)0xff, (char)0x62, (char)0xff, (char)0x63, (char)0xff, (char)0x64, (char)0xff, (char)0x65, (char)0xff, (char)0x66, (char)0xff, (char)0x67, (char)0xff, (char)0x68, (char)0xff, (char)0x69, (char)0xff, (char)0x6a, (char)0xff, (char)0x6b, (char)0xff, (char)0x6c, (char)0xff, (char)0x6d, (char)0xff, (char)0x6e, (char)0xff, (char)0x6f, (char)0xff, (char)0x70, (char)0xff, (char)0x71, (char)0xff, (char)0x72, (char)0xff, (char)0x73, (char)0xff, (char)0x74, (char)0xff, (char)0x75, (char)0xff, (char)0x76, (char)0xff, (char)0x77, (char)0xff, (char)0x78, (char)0xff, (char)0x79, (char)0xff, (char)0x7a, (char)0xff, (char)0x7b, (char)0xff, (char)0x7c, (char)0xff, (char)0x7d, (char)0xff, (char)0x7e, (char)0xff, (char)0x7f, (char)0xff, (char)0x80, (char)0xff, (char)0x81, (char)0xff, (char)0x82, (char)0xff, (char)0x83, (char)0xff, (char)0x84, (char)0xff, (char)0x85, (char)0xff, (char)0x86, (char)0xff, (char)0x87, (char)0xff, (char)0x88, (char)0xff, (char)0x89, (char)0xff, (char)0x8a, (char)0xff, (char)0x8b, (char)0xff, (char)0x8c, (char)0xff, (char)0x8d, (char)0xff, (char)0x8e, (char)0xff, (char)0x8f, (char)0xff, (char)0x90, (char)0xff, (char)0x91, (char)0xff, (char)0x92, (char)0xff, (char)0x93, (char)0xff, (char)0x94, (char)0xff, (char)0x95, (char)0xff, (char)0x96, (char)0xff, (char)0x97, (char)0xff, (char)0x98, (char)0xff, (char)0x99, (char)0xff, (char)0x9a, (char)0xff, (char)0x9b, (char)0xff, (char)0x9c, (char)0xff, (char)0x9d, (char)0xff, (char)0x9e, (char)0xff, (char)0x9f, (char)0xff}}),
        StringFromBytes({{(char)0x00, (char)0x01, (char)0x02, (char)0x03, (char)0x04, (char)0x05, (char)0x06, (char)0x07, (char)0x08, (char)0x09, (char)0x0a, (char)0x0b, (char)0x0c, (char)0x0d, (char)0x0e, (char)0x0f, (char)0x10, (char)0x11, (char)0x12, (char)0x13, (char)0x14, (char)0x15, (char)0x16, (char)0x17, (char)0x18, (char)0x19, (char)0x1a, (char)0x1b, (char)0x1c, (char)0x1d, (char)0x1e, (char)0x1f, (char)0x20, (char)0x21, (char)0x22, (char)0x23, (char)0x24, (char)0x25, (char)0x26, (char)0x27, (char)0x28, (char)0x29, (char)0x2a, (char)0x2b, (char)0x2c, (char)0x2d, (char)0x2e, (char)0x2f, (char)0x30, (char)0x31, (char)0x32, (char)0x33, (char)0x34, (char)0x35, (char)0x36, (char)0x37, (char)0x38, (char)0x39, (char)0x3a, (char)0x3b, (char)0x3c, (char)0x3d, (char)0x3e, (char)0x3f, (char)0x40, (char)0x41, (char)0x42, (char)0x43, (char)0x44, (char)0x45, (char)0x46, (char)0x47, (char)0x48, (char)0x49, (char)0x4a, (char)0x4b, (char)0x4c, (char)0x4d, (char)0x4e, (char)0x4f, (char)0x50, (char)0x51, (char)0x52, (char)0x53, (char)0x54, (char)0x55, (char)0x56, (char)0x57, (char)0x58, (char)0x59, (char)0x5a, (char)0x5b, (char)0x5c, (char)0x5d, (char)0x5e, (char)0x5f, (char)0x60, (char)0x61, (char)0x62, (char)0x63, (char)0x64, (char)0x65, (char)0x66, (char)0x67, (char)0x68, (char)0x69, (char)0x6a, (char)0x6b, (char)0x6c, (char)0x6d, (char)0x6e, (char)0x6f, (char)0x70, (char)0x71, (char)0x72, (char)0x73, (char)0x74, (char)0x75, (char)0x76, (char)0x77, (char)0x78, (char)0x79, (char)0x7a, (char)0x7b, (char)0x7c, (char)0x7d, (char)0x7e, (char)0x7f, (char)0x80, (char)0xa1, (char)0xa2, (char)0xa3, (char)0xa4, (char)0xa5, (char)0xa6, (char)0xa7, (char)0xa8, (char)0xa9, (char)0xaa, (char)0xab, (char)0xac, (char)0xad, (char)0xae, (char)0xaf, (char)0xb0, (char)0xb1, (char)0xb2, (char)0xb3, (char)0xb4, (char)0xb5, (char)0xb6, (char)0xb7, (char)0xb8, (char)0xb9, (char)0xba, (char)0xbb, (char)0xbc, (char)0xbd, (char)0xbe, (char)0xbf, (char)0xc0, (char)0xc1, (char)0xc2, (char)0xc3, (char)0xc4, (char)0xc5, (char)0xc6, (char)0xc7, (char)0xc8, (char)0xc9, (char)0xca, (char)0xcb, (char)0xcc, (char)0xcd, (char)0xce, (char)0xcf, (char)0xd0, (char)0xd1, (char)0xd2, (char)0xd3, (char)0xd4, (char)0xd5, (char)0xd6, (char)0xd7, (char)0xd8, (char)0xd9, (char)0xda, (char)0xdb, (char)0xdc, (char)0xdd, (char)0xde, (char)0xdf}})
    );

    // weird shift-jis things that don't roundtrip
    auto StrShiftJisToUtf16 = [](const std::string& s) {
        return HyoutaUtils::TextUtils::ShiftJisToUtf16(s.data(), s.size());
    };
    EXPECT_EQ(U16StrFromBytes({{(char)0x52, (char)0x22}}), StrShiftJisToUtf16(StringFromBytes({{(char)0x87, (char)0x90}})));
    EXPECT_EQ(U16StrFromBytes({{(char)0x61, (char)0x22}}), StrShiftJisToUtf16(StringFromBytes({{(char)0x87, (char)0x91}})));
    EXPECT_EQ(U16StrFromBytes({{(char)0x2b, (char)0x22}}), StrShiftJisToUtf16(StringFromBytes({{(char)0x87, (char)0x92}})));
    EXPECT_EQ(U16StrFromBytes({{(char)0x1a, (char)0x22}}), StrShiftJisToUtf16(StringFromBytes({{(char)0x87, (char)0x95}})));
    EXPECT_EQ(U16StrFromBytes({{(char)0xa5, (char)0x22}}), StrShiftJisToUtf16(StringFromBytes({{(char)0x87, (char)0x96}})));
    EXPECT_EQ(U16StrFromBytes({{(char)0x20, (char)0x22}}), StrShiftJisToUtf16(StringFromBytes({{(char)0x87, (char)0x97}})));
    EXPECT_EQ(U16StrFromBytes({{(char)0x35, (char)0x22}}), StrShiftJisToUtf16(StringFromBytes({{(char)0x87, (char)0x9a}})));
    EXPECT_EQ(U16StrFromBytes({{(char)0x29, (char)0x22}}), StrShiftJisToUtf16(StringFromBytes({{(char)0x87, (char)0x9b}})));
    EXPECT_EQ(U16StrFromBytes({{(char)0x2a, (char)0x22}}), StrShiftJisToUtf16(StringFromBytes({{(char)0x87, (char)0x9c}})));

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
        StringFromBytes({{(char)0x81, (char)0xb0}}) // this is a tilde in some non-windows shift-jis variants
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

TEST(TextEncoding, BufferSizes) {
    // test how we behave with too large or too small buffers
    // clang-format off
    std::string utf8 = StringFromBytes({{(char)0x6c,
                                         (char)0xc3, (char)0x9f,
                                         (char)0xe3, (char)0x82, (char)0xaa,
                                         (char)0xe6, (char)0xa3, (char)0xae,
                                         (char)0xf0, (char)0x9f, (char)0x8d, (char)0x8a}});
    std::u16string utf16 = U16StrFromBytes({{(char)0x6c, (char)0x00,
                                             (char)0xdf, (char)0x00,
                                             (char)0xaa, (char)0x30,
                                             (char)0xee, (char)0x68,
                                             (char)0x3c, (char)0xd8, (char)0x4a, (char)0xdf}});
    // clang-format on

    using namespace HyoutaUtils::TextUtils;
    EXPECT_EQ(13, MeasureUtf16ToUtf8(utf16.data(), utf16.size()));
    EXPECT_EQ(6, MeasureUtf8ToUtf16(utf8.data(), utf8.size()));

    // does not write past the end of the buffer, does not nullterminate
    {
        std::array<char, 16> b8{};
        std::array<char16_t, 8> b16{};
        b8.fill(0xfe);
        b16.fill(0xfefe);
        EXPECT_EQ(13, ConvertUtf16ToUtf8(b8.data(), b8.size(), utf16.data(), utf16.size()));
        EXPECT_EQ(6, ConvertUtf8ToUtf16(b16.data(), b16.size(), utf8.data(), utf8.size()));
        EXPECT_EQ(std::string_view(b8.data(), size_t(13)), utf8);
        EXPECT_EQ(std::u16string_view(b16.data(), size_t(6)), utf16);
        EXPECT_EQ((char)0xfe, b8[13]);
        EXPECT_EQ((char)0xfe, b8[14]);
        EXPECT_EQ((char)0xfe, b8[15]);
        EXPECT_EQ((char16_t)0xfefe, b16[6]);
        EXPECT_EQ((char16_t)0xfefe, b16[7]);
    }

    // if the output buffer is too small we *fail* and the output buffer contents are undefined.
    // tbqh this is kind of strange, i would expect it to convert as much as fits into the target
    // buffer, but the windows functions just do this so we have to define it like this too...
    {
        std::array<char, 5> b8{};
        std::array<char16_t, 5> b16{};
        b8.fill(0xfe);
        b16.fill(0xfefe);
        EXPECT_EQ(INVALID_LENGTH,
                  ConvertUtf16ToUtf8(b8.data(), b8.size(), utf16.data(), utf16.size()));
        EXPECT_EQ(INVALID_LENGTH,
                  ConvertUtf8ToUtf16(b16.data(), b16.size(), utf8.data(), utf8.size()));
#if 0
        EXPECT_EQ(std::string_view(b8.data(), size_t(3)), std::string_view(utf8).substr(0, 3));
        EXPECT_EQ(std::u16string_view(b16.data(), size_t(4)),
                  std::u16string_view(utf16).substr(0, 4));
        EXPECT_EQ((char)0xfe, b8[3]);
        EXPECT_EQ((char)0xfe, b8[4]);
        EXPECT_EQ((char16_t)0xfefe, b16[4]);
#endif
    }
}
