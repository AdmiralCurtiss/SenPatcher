#pragma once

#include <filesystem>
#include <optional>

#include "p3a/pack.h"

namespace SenPatcher {
std::optional<SenPatcher::P3APackData>
    P3APackDataFromJsonFile(const std::filesystem::path& jsonPath);
} // namespace SenPatcher
