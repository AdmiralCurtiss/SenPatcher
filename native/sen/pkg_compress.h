#pragma once

#include <cstdint>
#include <memory>

#include "util/endian.h"

namespace SenLib {
struct PkgFile;

// sets the UncompressedSize, CompressedSize, Flags, and Data fields of pkgFile on success
bool CompressPkgFile(std::unique_ptr<char[]>& dataBuffer,
                     PkgFile& pkgFile,
                     const char* uncompressedData,
                     uint32_t uncompressedLength,
                     uint32_t flags,
                     HyoutaUtils::EndianUtils::Endianness e);
} // namespace SenLib
