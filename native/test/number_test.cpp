#include <cstdint>
#include <optional>

#include "gtest/gtest.h"

#include "util/number.h"

TEST(NumberUtils, ParseInteger) {
    EXPECT_EQ(static_cast<uint32_t>(0), HyoutaUtils::NumberUtils::ParseUInt32("0"));
    EXPECT_EQ(static_cast<uint32_t>(1), HyoutaUtils::NumberUtils::ParseUInt32("1"));
    EXPECT_EQ(static_cast<uint32_t>(1), HyoutaUtils::NumberUtils::ParseUInt32("01"));
    EXPECT_EQ(static_cast<uint32_t>(1),
              HyoutaUtils::NumberUtils::ParseUInt32("0000000000000000000000000000000001"));
    EXPECT_EQ(static_cast<uint32_t>(4294967295u),
              HyoutaUtils::NumberUtils::ParseUInt32("4294967295"));
    EXPECT_EQ(static_cast<uint32_t>(4294967295u),
              HyoutaUtils::NumberUtils::ParseUInt32("0004294967295"));
    EXPECT_EQ(std::nullopt, HyoutaUtils::NumberUtils::ParseUInt32("-1"));
    EXPECT_EQ(std::nullopt, HyoutaUtils::NumberUtils::ParseUInt32("4294967296"));
    EXPECT_EQ(std::nullopt, HyoutaUtils::NumberUtils::ParseUInt32("1 "));
    EXPECT_EQ(std::nullopt, HyoutaUtils::NumberUtils::ParseUInt32(" 1"));
    EXPECT_EQ(std::nullopt, HyoutaUtils::NumberUtils::ParseUInt32(" 1 "));
    EXPECT_EQ(std::nullopt, HyoutaUtils::NumberUtils::ParseUInt32("a"));
    EXPECT_EQ(static_cast<uint32_t>(0), HyoutaUtils::NumberUtils::ParseUInt32("0x0"));
    EXPECT_EQ(static_cast<uint32_t>(0), HyoutaUtils::NumberUtils::ParseUInt32("0x0000"));
    EXPECT_EQ(static_cast<uint32_t>(1), HyoutaUtils::NumberUtils::ParseUInt32("0x1"));
    EXPECT_EQ(static_cast<uint32_t>(1), HyoutaUtils::NumberUtils::ParseUInt32("0x00000001"));
    EXPECT_EQ(static_cast<uint32_t>(0xa), HyoutaUtils::NumberUtils::ParseUInt32("0xa"));
    EXPECT_EQ(static_cast<uint32_t>(0xa), HyoutaUtils::NumberUtils::ParseUInt32("0xA"));
    EXPECT_EQ(static_cast<uint32_t>(0xa), HyoutaUtils::NumberUtils::ParseUInt32("0Xa"));
    EXPECT_EQ(static_cast<uint32_t>(0xa), HyoutaUtils::NumberUtils::ParseUInt32("0XA"));
    EXPECT_EQ(std::nullopt, HyoutaUtils::NumberUtils::ParseUInt32("0xg"));
    EXPECT_EQ(static_cast<uint32_t>(0x12ecfd), HyoutaUtils::NumberUtils::ParseUInt32("0x12eCFd"));
    EXPECT_EQ(static_cast<uint32_t>(4294967295u),
              HyoutaUtils::NumberUtils::ParseUInt32("0xFfFfFfFf"));
    EXPECT_EQ(std::nullopt, HyoutaUtils::NumberUtils::ParseUInt32("0x100000000"));
    EXPECT_EQ(static_cast<uint32_t>(0b11111), HyoutaUtils::NumberUtils::ParseUInt32("0b11111"));
    EXPECT_EQ(static_cast<uint32_t>(0b11111), HyoutaUtils::NumberUtils::ParseUInt32("0B11111"));
    EXPECT_EQ(static_cast<uint32_t>(0b11101), HyoutaUtils::NumberUtils::ParseUInt32("0b11101"));
    EXPECT_EQ(static_cast<uint32_t>(0b10111), HyoutaUtils::NumberUtils::ParseUInt32("0B10111"));
    EXPECT_EQ(static_cast<uint32_t>(4294967295u),
              HyoutaUtils::NumberUtils::ParseUInt32("0b11111111111111111111111111111111"));
    EXPECT_EQ(std::nullopt,
              HyoutaUtils::NumberUtils::ParseUInt32("0b100000000000000000000000000000000"));
    EXPECT_EQ(std::nullopt,
              HyoutaUtils::NumberUtils::ParseUInt32("0b111111111111111111111111111111111"));
    EXPECT_EQ(std::nullopt, HyoutaUtils::NumberUtils::ParseUInt32("0b2"));

    EXPECT_EQ(static_cast<int32_t>(0), HyoutaUtils::NumberUtils::ParseInt32("0"));
    EXPECT_EQ(static_cast<int32_t>(1), HyoutaUtils::NumberUtils::ParseInt32("1"));
    EXPECT_EQ(static_cast<int32_t>(1), HyoutaUtils::NumberUtils::ParseInt32("01"));
    EXPECT_EQ(static_cast<int32_t>(1),
              HyoutaUtils::NumberUtils::ParseInt32("0000000000000000000000000000000001"));
    EXPECT_EQ(static_cast<int32_t>(2147483647), HyoutaUtils::NumberUtils::ParseInt32("2147483647"));
    EXPECT_EQ(static_cast<int32_t>(2147483647),
              HyoutaUtils::NumberUtils::ParseInt32("0002147483647"));
    EXPECT_EQ(std::nullopt, HyoutaUtils::NumberUtils::ParseInt32("2147483648"));
    EXPECT_EQ(static_cast<int32_t>(-1), HyoutaUtils::NumberUtils::ParseInt32("-1"));
    EXPECT_EQ(static_cast<int32_t>(-2147483647),
              HyoutaUtils::NumberUtils::ParseInt32("-2147483647"));
    EXPECT_EQ(static_cast<int32_t>(-2147483647 - 1),
              HyoutaUtils::NumberUtils::ParseInt32("-2147483648"));
    EXPECT_EQ(std::nullopt, HyoutaUtils::NumberUtils::ParseInt32("-2147483649"));
    EXPECT_EQ(std::nullopt, HyoutaUtils::NumberUtils::ParseInt32("1 "));
    EXPECT_EQ(std::nullopt, HyoutaUtils::NumberUtils::ParseInt32(" 1"));
    EXPECT_EQ(std::nullopt, HyoutaUtils::NumberUtils::ParseInt32(" 1 "));
    EXPECT_EQ(std::nullopt, HyoutaUtils::NumberUtils::ParseInt32("a"));
    EXPECT_EQ(std::nullopt, HyoutaUtils::NumberUtils::ParseInt32("-0"));
    EXPECT_EQ(static_cast<int32_t>(0), HyoutaUtils::NumberUtils::ParseInt32("0x0"));
    EXPECT_EQ(static_cast<int32_t>(0), HyoutaUtils::NumberUtils::ParseInt32("0x0000"));
    EXPECT_EQ(static_cast<int32_t>(1), HyoutaUtils::NumberUtils::ParseInt32("0x1"));
    EXPECT_EQ(static_cast<int32_t>(1), HyoutaUtils::NumberUtils::ParseInt32("0x00000001"));
    EXPECT_EQ(static_cast<int32_t>(-1), HyoutaUtils::NumberUtils::ParseInt32("-0x1"));
    EXPECT_EQ(static_cast<int32_t>(-1), HyoutaUtils::NumberUtils::ParseInt32("-0x00000001"));
    EXPECT_EQ(static_cast<int32_t>(0xa), HyoutaUtils::NumberUtils::ParseInt32("0xa"));
    EXPECT_EQ(static_cast<int32_t>(0xa), HyoutaUtils::NumberUtils::ParseInt32("0xA"));
    EXPECT_EQ(static_cast<int32_t>(0xa), HyoutaUtils::NumberUtils::ParseInt32("0Xa"));
    EXPECT_EQ(static_cast<int32_t>(0xa), HyoutaUtils::NumberUtils::ParseInt32("0XA"));
    EXPECT_EQ(static_cast<int32_t>(-0xa), HyoutaUtils::NumberUtils::ParseInt32("-0xa"));
    EXPECT_EQ(static_cast<int32_t>(-0xa), HyoutaUtils::NumberUtils::ParseInt32("-0xA"));
    EXPECT_EQ(static_cast<int32_t>(-0xa), HyoutaUtils::NumberUtils::ParseInt32("-0Xa"));
    EXPECT_EQ(static_cast<int32_t>(-0xa), HyoutaUtils::NumberUtils::ParseInt32("-0XA"));
    EXPECT_EQ(std::nullopt, HyoutaUtils::NumberUtils::ParseInt32("0xg"));
    EXPECT_EQ(static_cast<int32_t>(0x12ecfd), HyoutaUtils::NumberUtils::ParseInt32("0x12eCFd"));
    EXPECT_EQ(static_cast<int32_t>(2147483647), HyoutaUtils::NumberUtils::ParseInt32("0x7fFfFfFf"));
    EXPECT_EQ(static_cast<int32_t>(-2147483647),
              HyoutaUtils::NumberUtils::ParseInt32("-0x7fFfFfFf"));
    EXPECT_EQ(static_cast<int32_t>(-2147483647 - 1),
              HyoutaUtils::NumberUtils::ParseInt32("-0x80000000"));
    EXPECT_EQ(static_cast<int32_t>(0b11111), HyoutaUtils::NumberUtils::ParseInt32("0b11111"));
    EXPECT_EQ(static_cast<int32_t>(0b11111), HyoutaUtils::NumberUtils::ParseInt32("0B11111"));
    EXPECT_EQ(static_cast<int32_t>(0b11101), HyoutaUtils::NumberUtils::ParseInt32("0b11101"));
    EXPECT_EQ(static_cast<int32_t>(0b10111), HyoutaUtils::NumberUtils::ParseInt32("0B10111"));
    EXPECT_EQ(static_cast<int32_t>(-0b11111), HyoutaUtils::NumberUtils::ParseInt32("-0b11111"));
    EXPECT_EQ(static_cast<int32_t>(-0b11111), HyoutaUtils::NumberUtils::ParseInt32("-0B11111"));
    EXPECT_EQ(static_cast<int32_t>(2147483647),
              HyoutaUtils::NumberUtils::ParseInt32("0b1111111111111111111111111111111"));
    EXPECT_EQ(static_cast<int32_t>(-2147483647),
              HyoutaUtils::NumberUtils::ParseInt32("-0b1111111111111111111111111111111"));
    EXPECT_EQ(static_cast<int32_t>(-2147483647 - 1),
              HyoutaUtils::NumberUtils::ParseInt32("-0b10000000000000000000000000000000"));
    EXPECT_EQ(std::nullopt,
              HyoutaUtils::NumberUtils::ParseInt32("-0b10000000000000000000000000000001"));
    EXPECT_EQ(std::nullopt,
              HyoutaUtils::NumberUtils::ParseInt32("0b10000000000000000000000000000000"));
    EXPECT_EQ(std::nullopt,
              HyoutaUtils::NumberUtils::ParseInt32("0b11111111111111111111111111111111"));
    EXPECT_EQ(std::nullopt, HyoutaUtils::NumberUtils::ParseInt32("0b2"));
}
