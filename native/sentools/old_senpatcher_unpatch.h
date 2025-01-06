#pragma once

#include <string_view>

namespace SenPatcher {
bool HasOldSenpatcherBackups(std::string_view gamepath, int sengame);
bool UnpatchGame(std::string_view gamepath, int sengame);
} // namespace SenPatcher
