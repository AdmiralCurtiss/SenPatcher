#pragma once

#include <cstddef>
#include <memory>

#include "sen/pka.h"
#include "util/file.h"

namespace SenLib {
struct PkaHeader;
struct PkgHeader;

struct ReferencedPka {
    HyoutaUtils::IO::File PkaFile;
    SenLib::PkaHeader PkaHeader;

    ReferencedPka() = default;
    ReferencedPka(const ReferencedPka& other) = delete;
    ReferencedPka(ReferencedPka&& other) = default;
    ReferencedPka& operator=(const ReferencedPka& other) = delete;
    ReferencedPka& operator=(ReferencedPka&& other) = default;
    ~ReferencedPka() = default;
};

bool ConvertPkaToSinglePkg(PkgHeader& pkg,
                           std::unique_ptr<char[]>& dataBuffer,
                           const PkaHeader& pka,
                           size_t index,
                           HyoutaUtils::IO::File& file,
                           std::vector<SenLib::ReferencedPka>& referencedPkas);
} // namespace SenLib
