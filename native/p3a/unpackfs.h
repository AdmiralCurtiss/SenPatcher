#pragma once

#include <filesystem>
#include <string_view>

namespace SenPatcher {
bool UnpackP3A(const std::filesystem::path& archivePath,
               const std::filesystem::path& extractPath,
               std::string_view pathFilter,
               bool generateJson,
               bool noDecompression);
}
