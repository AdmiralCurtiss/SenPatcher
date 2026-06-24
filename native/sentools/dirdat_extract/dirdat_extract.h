#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

#include "util/result.h"

namespace SenTools::DirDat {
struct SingleFileDir {
    std::array<char, 12> Filename;
    uint32_t Unknown1;
    uint32_t CompressedSize;
    uint32_t Unknown3;
    uint32_t CompressedSizeCopy;
    uint32_t Unknown5;
    uint32_t OffsetInDat;
};
static_assert(sizeof(SingleFileDir) == 0x24);
static_assert(offsetof(SingleFileDir, CompressedSize) == 0x10);
static_assert(offsetof(SingleFileDir, OffsetInDat) == 0x20);

enum class ExtractDirDatResult { Success };

HyoutaUtils::Result<ExtractDirDatResult, std::string> ExtractDirDat(std::string_view sourcePathDir,
                                                                    std::string_view sourcePathDat,
                                                                    std::string_view targetPath,
                                                                    bool generateJson);
} // namespace SenTools::DirDat
