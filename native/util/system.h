#pragma once

#include <optional>
#include <string>

namespace HyoutaUtils::Sys {
// Try to get the directory the currently running executable is in.
// This is NOT the same as the current working directory!
std::optional<std::string> GetCurrentExecutableDirectory() noexcept;

// Get an environment variable. Returns nullopt if the variable is not set.
std::optional<std::string> GetEnvironmentVar(const char* key) noexcept;
} // namespace HyoutaUtils::Sys
