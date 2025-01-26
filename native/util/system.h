#pragma once

#include <optional>
#include <string>

namespace HyoutaUtils::Sys {
// Try to get the directory the currently running executable is in.
// This is NOT the same as the current working directory!
std::optional<std::string> GetCurrentExecutableDirectory() noexcept;
} // namespace HyoutaUtils::Sys
