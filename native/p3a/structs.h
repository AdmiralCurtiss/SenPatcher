#pragma once

#include <array>
#include <cstdint>
#include <type_traits>

namespace SenPatcher {
enum class P3ACompressionType : uint64_t {
    None = 0,
    LZ4 = 1,
    ZSTD = 2,
    ZSTD_DICT = 3,
};

struct P3AHeader {
    std::array<char, 8> Magic;
    uint32_t Flags;
    uint32_t Version;
    uint64_t FileCount;
    uint64_t Hash;
};
static_assert(sizeof(P3AHeader) == 0x20);
static_assert(std::is_trivially_copyable_v<P3AHeader>);

static constexpr uint32_t P3AHeaderFlag_HasZstdDict = 1;

struct P3AFileInfo {
    std::array<char8_t, 0x100> Filename;
    P3ACompressionType CompressionType;
    uint64_t CompressedSize;
    uint64_t UncompressedSize;
    uint64_t Offset;
    uint64_t Hash;
};
static_assert(sizeof(P3AFileInfo) == 0x128);
static_assert(std::is_trivially_copyable_v<P3AFileInfo>);

struct P3ADictHeader {
    std::array<char, 8> Magic;
    uint64_t Length;
};
static_assert(sizeof(P3ADictHeader) == 0x10);
static_assert(std::is_trivially_copyable_v<P3ADictHeader>);
} // namespace SenPatcher
