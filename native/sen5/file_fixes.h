#pragma once

#include <string_view>

#include "util/logger.h"

namespace SenPatcher {
struct Logger;
struct P3APackFile;
} // namespace SenPatcher

namespace SenLib::Sen5 {
bool CreateAssetPatchIfNeeded(SenPatcher::Logger& logger, std::string_view baseDir);
}
