#pragma once

#include <functional>
#include <string>
#include <string_view>

#include "util/ini_writer.h"

namespace SenTools::GUI {
bool WriteGameSettingsIni(const std::function<void(HyoutaUtils::Ini::IniWriter& writer)>& callback,
                          const std::string& gameIniPath,
                          std::string_view defaultIniString);

// Returns a status message to be displayed to the user.
std::string
    PatchOrUnpatchGame(const std::function<void(HyoutaUtils::Ini::IniWriter& writer)>& callback,
                       const std::string& localDllPath,
                       const std::string& gameDllPath,
                       const std::string& gameIniPath,
                       std::string_view defaultIniString,
                       bool unpatch);
} // namespace SenTools::GUI
