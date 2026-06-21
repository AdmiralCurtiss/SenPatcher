#pragma once

#include <string>
#include <string_view>

#include "util/result.h"

namespace SenTools {
enum class ExtractDirDatResult { Success };

HyoutaUtils::Result<ExtractDirDatResult, std::string> ExtractDirDat(std::string_view sourcePathDir,
                                                                    std::string_view sourcePathDat,
                                                                    std::string_view targetPath,
                                                                    bool generateJson);
} // namespace SenTools
