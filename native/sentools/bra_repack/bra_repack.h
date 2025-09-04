#pragma once

#include <span>
#include <string>
#include <string_view>

#include "util/result.h"

namespace SenTools {
enum class RepackBraResult { Success };

HyoutaUtils::Result<RepackBraResult, std::string> RepackBra(std::string_view source,
                                                            std::string_view target);
} // namespace SenTools
