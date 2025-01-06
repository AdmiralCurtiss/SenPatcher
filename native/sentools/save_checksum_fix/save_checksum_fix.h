#pragma once

#include <string>
#include <string_view>

#include "util/result.h"

namespace SenTools {
enum class SaveChecksumFixResult { Success, AlreadyCorrect };

HyoutaUtils::Result<SaveChecksumFixResult, std::string> SaveChecksumFix(std::string_view source,
                                                                        std::string_view target);
} // namespace SenTools
