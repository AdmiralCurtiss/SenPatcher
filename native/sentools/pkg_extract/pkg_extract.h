#pragma once

#include <string>
#include <string_view>

#include "util/result.h"

namespace SenTools {
enum class ExtractPkgResult { Success };

HyoutaUtils::Result<ExtractPkgResult, std::string>
    ExtractPkg(std::string_view source, std::string_view target, bool generateJson);
} // namespace SenTools
