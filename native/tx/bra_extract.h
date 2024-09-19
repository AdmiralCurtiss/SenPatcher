#pragma once

#include <cstdint>

#include "util/endian.h"

namespace SenPatcher {
bool ExtractAndDecompressBraFile(char* uncompressedDataBuffer,
                                 uint32_t uncompressedDataLength,
                                 const char* compressedDataBuffer,
                                 uint32_t compressedDataLength);
} // namespace SenPatcher
