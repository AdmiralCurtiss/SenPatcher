#pragma once

#include <string>
#include <string_view>

namespace HyoutaUtils::Ini {
struct IniFile;
struct IniWriter;
} // namespace HyoutaUtils::Ini

namespace SenTools {
struct GuiUserSettings {
    std::string Sen1Path;
    std::string Sen2Path;
    std::string Sen3Path;
    std::string Sen4Path;
    std::string Sen5Path;
    std::string TXPath;
};

bool LoadUserSettingsFromIni(GuiUserSettings& settings, std::string_view path);
bool LoadUserSettingsFromIni(GuiUserSettings& settings, const HyoutaUtils::Ini::IniFile& ini);
bool WriteUserSettingsToIni(const GuiUserSettings& settings, std::string_view path);
bool WriteUserSettingsToIni(const GuiUserSettings& settings, HyoutaUtils::Ini::IniWriter& ini);
} // namespace SenTools
