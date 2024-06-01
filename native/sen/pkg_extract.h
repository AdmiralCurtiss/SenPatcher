#pragma once

#include <cstdint>

#include "util/endian.h"

namespace SenLib {
bool ExtractAndDecompressPkgFile(char* uncompressedDataBuffer,
                                 uint32_t uncompressedDataLength,
                                 const char* compressedDataBuffer,
                                 uint32_t compressedDataLength,
                                 uint32_t flags,
                                 HyoutaUtils::EndianUtils::Endianness e);
} // namespace SenLib
