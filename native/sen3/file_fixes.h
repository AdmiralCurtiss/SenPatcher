#pragma once

#include <string_view>

#include "util/logger.h"

namespace SenLib::Sen3 {
bool CreateAssetPatchIfNeeded(HyoutaUtils::Logger& logger, std::string_view baseDir);
}
