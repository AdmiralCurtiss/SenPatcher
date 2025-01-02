#pragma once

#include <string>
#include <string_view>

#include "util/result.h"

namespace SenTools {
enum class DecompressType1Result { Success };

HyoutaUtils::Result<DecompressType1Result, std::string> DecompressType1(std::string_view source,
                                                                        std::string_view target);
} // namespace SenTools
