#pragma once

#include <functional>
#include <string_view>

#include "logger.h"
#include "p3a/pack.h"
#include "sen/file_getter.h"
#include "sha1.h"

namespace SenLib {
void CreateArchiveIfNeeded(
    SenPatcher::Logger& logger,
    std::string_view baseDir,
    std::string_view archivePath,
    const std::function<bool(SenPatcher::P3APackData& packData)>& collectAssets);
void CreateVideoIfNeeded(
    SenPatcher::Logger& logger,
    std::string_view baseDir,
    std::string_view videoPath,
    const std::function<bool(SenPatcher::P3APackData& packData)>& collectAssets);
std::optional<SenPatcher::CheckedFileResult> GetCheckedFile(std::string_view baseDir,
                                                            std::string_view path,
                                                            size_t size,
                                                            const SenPatcher::SHA1& hash);
} // namespace SenLib
