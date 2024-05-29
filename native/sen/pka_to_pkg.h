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
                           HyoutaUtils::IO::File& file,
                           const PkaHeader* referencedPka,
                           HyoutaUtils::IO::File* referencedFile);
} // namespace SenLib
