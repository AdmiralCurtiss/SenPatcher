#pragma once

#include <cstdint>
#include <filesystem>

namespace SenPatcher {
enum class P3ACompressionType : uint64_t;
bool PackP3AFromDirectory(const std::filesystem::path& directoryPath,
                          const std::filesystem::path& archivePath,
                          P3ACompressionType desiredCompressionType,
                          const std::filesystem::path& dictPath = std::filesystem::path());
} // namespace SenPatcher
