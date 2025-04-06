#pragma once

#include <string>
#include <string_view>

#include "util/result.h"

namespace SenTools {
enum class Daybreak2SaveConvertResult { Success };

HyoutaUtils::Result<Daybreak2SaveConvertResult, std::string>
    Daybreak2SaveConvert(std::string_view source, std::string_view target);
} // namespace SenTools
