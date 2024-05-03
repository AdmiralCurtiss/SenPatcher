#pragma once

#include <vector>

#include "util/stream.h"

namespace HyoutaUtils::Bps {
void ApplyPatchToStream(HyoutaUtils::Stream::ReadStream& source,
                        HyoutaUtils::Stream::ReadStream& patch,
                        std::vector<char>& target);
} // namespace HyoutaUtils::Bps
