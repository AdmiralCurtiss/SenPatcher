#pragma once

#include <string_view>

#include "logger.h"

namespace SenPatcher {
struct Logger;
struct P3APackFile;
} // namespace SenPatcher

namespace SenLib::Sen3 {
bool CreateAssetPatchIfNeeded(SenPatcher::Logger& logger, std::string_view baseDir);
}
