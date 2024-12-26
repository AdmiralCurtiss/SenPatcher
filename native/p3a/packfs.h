#pragma once

#include <filesystem>
#include <optional>

#include "pack.h"

namespace SenPatcher {
std::optional<SenPatcher::P3APackData>
    P3APackDataFromDirectory(const std::filesystem::path& directoryPath,
                             uint32_t archiveVersion,
                             std::optional<P3ACompressionType> desiredCompressionType,
                             const std::filesystem::path& dictPath);
} // namespace SenPatcher
