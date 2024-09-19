#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string_view>

#include "util/file.h"

namespace SenPatcher {
// This is actually a subset of what is actually stored in the archive.
// See bra_extract.cpp in SenTools for more info.
struct BRAFileInfo {
    uint32_t CompressedSize = 0;
    uint32_t UncompressedSize = 0;
    uint32_t DataPosition = 0;
    std::array<char, 0x60> Path{};
};

struct BRA {
    uint64_t FileCount = 0;
    std::unique_ptr<SenPatcher::BRAFileInfo[]> FileInfo{};
    HyoutaUtils::IO::File FileHandle{};

    BRA();
    BRA(const BRA& other) = delete;
    BRA(BRA&& other);
    BRA& operator=(const BRA& other) = delete;
    BRA& operator=(BRA&& other);
    ~BRA();

    void Clear();
    bool Load(std::string_view path);

private:
    bool Load(HyoutaUtils::IO::File& f);
};
} // namespace SenPatcher
