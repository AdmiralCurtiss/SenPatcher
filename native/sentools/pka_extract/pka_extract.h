#pragma once

#include <span>
#include <string>
#include <string_view>

#include "util/result.h"

namespace SenTools {
enum class ExtractPkaResult { Success };

HyoutaUtils::Result<ExtractPkaResult, std::string>
    ExtractPka(std::string_view source,
               std::string_view target,
               std::span<const std::string> referencedPkaPaths);
} // namespace SenTools
