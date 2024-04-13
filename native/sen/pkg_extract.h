#pragma once

#include <cstddef>
#include <memory>

#include "util/endian.h"

namespace SenLib {
struct PkgFile;

bool ExtractAndDecompressPkgFile(std::unique_ptr<char[]>& dataBuffer,
                                 size_t& dataLength,
                                 const PkgFile& pkgFile,
                                 HyoutaUtils::EndianUtils::Endianness e);
} // namespace SenLib
