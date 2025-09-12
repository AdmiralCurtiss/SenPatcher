#pragma once

#include <cstdint>
#include <string>

namespace SenTools {
struct BraFileInfo {
    // from the BRA footer
    uint32_t Unknown1 = 0;
    uint32_t UncompressedCrc32 = 0;
    uint32_t CompressedSize = 0;
    uint32_t UncompressedSize = 0;
    uint16_t PathLength = 0; // seems to be 4-byte aligned
    uint16_t Unknown3 = 0;
    uint32_t DataPosition = 0;

    // from the first 0x10 bytes of the individual file
    uint32_t FileHeader_UncompressedSize = 0;
    uint32_t FileHeader_CompressedSize = 0;
    uint32_t FileHeader_UncompressedCrc32 = 0;
    uint8_t FileHeader_CompressionType = 0;
    uint8_t FileHeader_Unknown4 = 0;
    uint16_t FileHeader_Unknown5 = 0;

    std::string Path;
};
} // namespace SenTools
