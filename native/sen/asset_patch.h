#pragma once

#include <filesystem>
#include <functional>
#include <string_view>

#include "file_getter.h"
#include "logger.h"
#include "p3a/pack.h"
#include "sha1.h"

namespace SenLib {
void CreateArchiveIfNeeded(
    SenPatcher::Logger& logger,
    const std::filesystem::path& archivePath,
    const std::function<bool(SenPatcher::P3APackData& packData)>& collectAssets);
void CreateVideoIfNeeded(
    SenPatcher::Logger& logger,
    const std::filesystem::path& videoPath,
    const std::function<bool(SenPatcher::P3APackData& packData)>& collectAssets);
std::optional<SenPatcher::CheckedFileResult> GetCheckedFile(const std::filesystem::path& baseDir,
                                                            std::string_view path,
                                                            size_t size,
                                                            const SenPatcher::SHA1& hash);
} // namespace SenLib
