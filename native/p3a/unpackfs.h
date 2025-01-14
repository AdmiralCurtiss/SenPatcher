#pragma once

#include <string>
#include <string_view>

#include "util/result.h"

namespace SenPatcher {
enum class UnpackP3AResult { Success };

HyoutaUtils::Result<UnpackP3AResult, std::string> UnpackP3A(std::string_view archivePath,
                                                            std::string_view extractPath,
                                                            std::string_view pathFilter,
                                                            bool generateJson,
                                                            bool noDecompression);
} // namespace SenPatcher
