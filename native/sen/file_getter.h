#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>
#include <vector>

#include "util/hash/sha1.h"

namespace SenPatcher {
struct CheckedFileResult {
    std::vector<char> Data;
    std::array<char, 0x100> Filename{};
};
using GetCheckedFileCallback =
    std::function<std::optional<CheckedFileResult>(std::string_view path,
                                                   size_t size,
                                                   const HyoutaUtils::Hash::SHA1& hash)>;
} // namespace SenPatcher
