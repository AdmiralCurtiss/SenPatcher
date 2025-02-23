#pragma once

#include <span>
#include <string>
#include <string_view>

#include "util/result.h"

namespace SenTools {
enum class ExtractPkgResult { Success };

HyoutaUtils::Result<ExtractPkgResult, std::string>
    ExtractPkg(std::string_view source,
               std::string_view target,
               std::span<const std::string> referencedPkaPaths,
               bool generateJson);
} // namespace SenTools
