#pragma once

#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "util/hash/sha1.h"
#include "util/logger.h"

namespace SenLib::ModLoad {
struct LoadedP3AData;
struct LoadedPkaData;
} // namespace SenLib::ModLoad

namespace SenLib::TX {
bool CreateSwitchScriptArchive(HyoutaUtils::Logger& logger,
                               std::string_view dataDirSwitch,
                               std::string_view baseDirPc,
                               SenLib::ModLoad::LoadedP3AData& vanillaP3As,
                               SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
                               std::span<const std::string_view> pkaPrefixes);
SenPatcher::P3APackFile* FindAlreadyPackedFile(std::vector<SenPatcher::P3APackFile>& packFiles,
                                               std::string_view path,
                                               size_t size,
                                               const HyoutaUtils::Hash::SHA1& hash);
} // namespace SenLib::TX
