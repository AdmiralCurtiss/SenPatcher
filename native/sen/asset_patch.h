#pragma once

#include <functional>
#include <string_view>
#include <vector>

#include "util/logger.h"
#include "p3a/pack.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

namespace SenLib {
bool CreateArchiveIfNeeded(
    SenPatcher::Logger& logger,
    std::string_view baseDir,
    std::string_view archivePath,
    const std::function<bool(SenPatcher::P3APackData& packData)>& collectAssets);
bool CreateVideoIfNeeded(SenPatcher::Logger& logger,
                         std::string_view baseDir,
                         std::string_view videoPath,
                         const std::function<bool(std::vector<char>& videoData)>& getVideo);
std::optional<SenPatcher::CheckedFileResult> GetCheckedFile(std::string_view baseDir,
                                                            std::string_view path,
                                                            size_t size,
                                                            const SenPatcher::SHA1& hash);
} // namespace SenLib
