#pragma once

#include <vector>

#include "util/stream.h"

namespace HyoutaUtils::Bps {
void ApplyPatchToStream(SenLib::ReadStream& source,
                        SenLib::ReadStream& patch,
                        std::vector<char>& target);
} // namespace HyoutaUtils::Bps
