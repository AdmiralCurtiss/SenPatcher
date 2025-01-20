#pragma once

#include <optional>
#include <string>

namespace SenTools::CommonPaths {
std::optional<std::string> GetSavedGamesFolder();
std::optional<std::string> GetLocalAppDataFolder();
std::optional<std::string> GetLocalSenPatcherGuiSettingsFolder();
} // namespace SenTools::CommonPaths
