#pragma once

#include <string>
#include <string_view>

#include "util/result.h"

namespace SenTools {
enum class CompressType1Result { Success };

HyoutaUtils::Result<CompressType1Result, std::string> CompressType1(std::string_view source,
                                                                    std::string_view target);
} // namespace SenTools
