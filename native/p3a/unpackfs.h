#pragma once

#include <filesystem>

namespace SenPatcher {
bool UnpackP3A(const std::filesystem::path& archivePath,
               const std::filesystem::path& extractPath,
               bool generateJson,
               bool noDecompression);
}
