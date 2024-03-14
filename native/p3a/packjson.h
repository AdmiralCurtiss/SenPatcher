#pragma once

#include "p3a/pack.h"

namespace SenPatcher {
bool PackP3AFromJsonFile(const std::filesystem::path& jsonPath,
                         const std::filesystem::path& archivePath);
} // namespace SenPatcher
