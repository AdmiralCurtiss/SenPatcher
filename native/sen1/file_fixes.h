#pragma once

#include <string_view>

#include "util/logger.h"

namespace SenLib::ModLoad {
struct LoadedP3AData;
}

namespace SenLib::Sen1 {
bool CreateAssetPatchIfNeeded(HyoutaUtils::Logger& logger,
                              std::string_view baseDir,
                              SenLib::ModLoad::LoadedP3AData& vanillaP3As);
}
