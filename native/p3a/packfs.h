#pragma once

#include <filesystem>
#include <optional>

#include "pack.h"

namespace SenPatcher {
bool PackP3AFromDirectory(const std::filesystem::path& directoryPath,
                          const std::filesystem::path& archivePath,
                          std::optional<P3ACompressionType> desiredCompressionType,
                          const std::filesystem::path& dictPath,
                          size_t desiredThreadCount);
bool PackP3A(const std::filesystem::path& archivePath,
             const P3APackData& packData,
             size_t desiredThreadCount);
} // namespace SenPatcher
