#pragma once

#include <filesystem>
#include <functional>
#include <string_view>
#include <vector>

#include "file_getter.h"

namespace SenPatcher {
struct Logger;
struct P3APackFile;
} // namespace SenPatcher

namespace SenLib::Sen4 {
void CreateAssetPatchIfNeeded(SenPatcher::Logger& logger, const std::filesystem::path& baseDir);
}
