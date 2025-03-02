#pragma once

#include <functional>
#include <span>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"
#include "util/logger.h"

namespace SenLib::ModLoad {
struct LoadedP3AData;
struct LoadedPkaData;
struct LoadedPkaGroupData;
} // namespace SenLib::ModLoad

namespace SenLib {
bool CreateArchiveIfNeeded(
    HyoutaUtils::Logger& logger,
    std::string_view baseDir,
    std::string_view archivePath,
    std::string_view versionString,
    const std::function<bool(SenPatcher::P3APackData& packData)>& collectAssets);
bool CreateVideoIfNeeded(HyoutaUtils::Logger& logger,
                         std::string_view baseDir,
                         std::string_view videoPath,
                         const std::function<bool(std::vector<char>& videoData)>& getVideo);
std::optional<SenPatcher::CheckedFileResult>
    GetCheckedFile(std::string_view baseDir,
                   SenLib::ModLoad::LoadedP3AData& vanillaP3As,
                   SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
                   std::span<const std::string_view> pkaPrefixes,
                   SenLib::ModLoad::LoadedPkaGroupData* pkgsOfPrefix0File0,
                   std::string_view path,
                   size_t size,
                   const HyoutaUtils::Hash::SHA1& hash,
                   bool getPkgsFromPkaAsStub = false);
} // namespace SenLib
