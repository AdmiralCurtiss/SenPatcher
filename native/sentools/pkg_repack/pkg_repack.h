#pragma once

#include <span>
#include <string>
#include <string_view>

#include "util/result.h"

namespace SenTools {
enum class RepackPkgResult { Success };

HyoutaUtils::Result<RepackPkgResult, std::string> RepackPkg(std::string_view source,
                                                            std::string_view target);
} // namespace SenTools
