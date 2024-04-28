#pragma once

#include <cstddef>
#include <memory>

#include "util/file.h"

namespace SenLib {
struct PkaHeader;
struct PkgHeader;

bool ConvertPkaToSinglePkg(PkgHeader& pkg,
                           std::unique_ptr<char[]>& dataBuffer,
                           const PkaHeader& pka,
                           size_t index,
                           SenPatcher::IO::File& file);
} // namespace SenLib
