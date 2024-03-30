#pragma once

#include <filesystem>

#include "pack.h"

namespace SenPatcher {
bool PackP3AFromDirectory(const std::filesystem::path& directoryPath,
                          const std::filesystem::path& archivePath,
                          P3ACompressionType desiredCompressionType,
                          const std::filesystem::path& dictPath = std::filesystem::path());
bool PackP3A(const std::filesystem::path& archivePath, const P3APackData& packData);
} // namespace SenPatcher
