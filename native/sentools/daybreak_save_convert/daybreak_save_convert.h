#pragma once

#include <string>
#include <string_view>

#include "util/result.h"

namespace SenTools {
enum class DaybreakSaveConvertResult { Success };

HyoutaUtils::Result<DaybreakSaveConvertResult, std::string>
    DaybreakSaveConvert(std::string_view source, std::string_view target);
} // namespace SenTools
