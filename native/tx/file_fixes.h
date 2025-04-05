#pragma once

#include <span>
#include <string_view>

#include "util/logger.h"

namespace SenLib::ModLoad {
struct LoadedP3AData;
struct LoadedPkaData;
} // namespace SenLib::ModLoad

namespace SenLib::TX {
bool CreateAssetPatchIfNeeded(HyoutaUtils::Logger& logger,
                              std::string_view baseDir,
                              SenLib::ModLoad::LoadedP3AData& vanillaP3As,
                              SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
                              std::span<const std::string_view> pkaPrefixes,
                              bool isJp);
}
