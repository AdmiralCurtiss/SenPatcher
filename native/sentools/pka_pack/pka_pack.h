#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

#include "util/result.h"

namespace SenTools {
enum class PackPkaResult { Success };

struct PackPkaPkgInfo {
    std::string Path;
    std::string NameInPka;
};

HyoutaUtils::Result<PackPkaResult, std::string>
    PackPka(std::string_view target,
            std::span<const PackPkaPkgInfo> sourcePkgs,
            std::span<const std::string_view> referencedPkaPaths,
            std::optional<uint32_t> recompressFlags,
            size_t desiredThreadCount);
} // namespace SenTools
